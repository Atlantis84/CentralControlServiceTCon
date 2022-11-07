
#include "processor.h"

#include "spdlogwapper.hpp"
#include "../GlobDef.h"

#include "xmlrpc-c/base.h"
#include "xmlrpc-c/client.h"
#include "xmlrpc-c/client_global.h"
#include "xmlrpc-c/util.h"

#include "curl/curl.h"
#include "json/reader.h"
#include "json/writer.h"

#include <queue>
#include <mutex>
#include <string>
#include <thread>

struct upgrade_result {
	char type_[256];
	char line_[256];
	char machine_[256];
	int result_;
};
struct upgrade_result_tv {
	char type_[256];
	char line_[256];
	char machine_[256];
	int result_;
};
struct check_upgrade_postfields {
	std::string type_;
	std::string fields_;
	std::string buf_;
	uint32_t ts_;
};
struct check_upgrade_postfields_tv {
	std::string type_;
	std::string fields_;
	std::string buf_;
	uint32_t ts_;
};
static std::mutex mutex_;
static std::queue<upgrade_result> upgrade_result_;
static std::mutex check_upgrade_mutex_;
static std::deque<check_upgrade_postfields> check_upgrade_postfields_;

static std::mutex mutex_tv_;
static std::queue<upgrade_result_tv> upgrade_result_tv_;
static std::mutex check_upgrade_mutex_tv_;
static std::deque<check_upgrade_postfields_tv> check_upgrade_postfields_tv_;

static bool stop_check_upgrade_ = false;
static bool check_upgrade_stoped_ = false;
static bool stop_check_upgrade_tv_ = false;
static bool check_upgrade_stoped_tv_ = false;
static char center_url_[256] = { 0 };
static unsigned short center_port_ = 0;

// 定时请求中控换产信息(该线程在客户端有请求换产信息的时候才会去请求数据)
static void check_upgrade_proc();
static void check_upgrade_proc_tv();
// 通知中控，换产结果
static void notify_upgrade_result();
static void notify_upgrade_result_tv();

static size_t on_write_result_data(char *ptr, size_t size, size_t nmemb, void *userdata);
static size_t on_write_data(char *ptr, size_t size, size_t nmemb, void *userdata);

void start_check_upgrade(const char* url, unsigned short port)
{
	strcpy_s(center_url_, url);
	center_port_ = port;

	L_INFO("中控地址: {0}:{1}", center_url_, center_port_);

	stop_check_upgrade_ = check_upgrade_stoped_ = false;
	std::thread thr(check_upgrade_proc);
	thr.detach();

	std::thread thr1(notify_upgrade_result);
	thr1.detach();
}

void stop_check_upgrade()
{
	stop_check_upgrade_ = true;

	while (!check_upgrade_stoped_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}

static void check_upgrade_proc()
{
	//lock_it();
	static xmlrpc_env env;
	/* Initialize our error-handling environment. */
	xmlrpc_env_init(&env);

	std::string buffer;
	
	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, center_url_);
	curl_easy_setopt(curl, CURLOPT_PORT, center_port_);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_data);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	for (;;) {
		//L_WARN("test multiple threads---lilei!");
		if (stop_check_upgrade_) {
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(30));

		std::lock_guard<std::mutex> m(check_upgrade_mutex_);
		if (check_upgrade_postfields_.empty()) {
			//L_INFO("换产请求信息队列为空!");
			continue;
		}

		auto postfields = check_upgrade_postfields_.back();
		check_upgrade_postfields_.pop_back();
		do {
			if (GetTickCount() - postfields.ts_ < 5 * 1000) {
				break;
			}
			postfields.ts_ = GetTickCount();

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.fields_.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.fields_.size());

			buffer.clear();
			if (curl_easy_perform(curl) != CURLE_OK) {
				break;
			}
			buffer.append("\0");

			// 如果没有换产，中控不推送数据
			if (buffer.empty()) {
				L_INFO("当前没有换产请求信息!");
				break;
			}

// 			if (buffer == postfields.buf_) {
// 				break;
// 			}

			Json::Value root;
			Json::Reader reader;
			if (!reader.parse(buffer.c_str(), root)) {
				break;
			}

			if (root["code"].asString() != "OK") {
				break;
			}

			xmlrpc_value* array = xmlrpc_array_new(&env);

			Json::Value& data = root["data"];
			Json::Value& type = root["requestType"];
			if (!type.isString()) {
				break;
			}

			CleanClientEnv();

			const char* typeName = type.asCString();
			for (int i = 0; i < (int)data.size(); i++) {
				Json::Value& ctx = data[i];
				const char* areaName = ctx["areaName"].asCString();
				const char* machineName = ctx["machineName"].asCString();
				const char* boardStyle = ctx["boardStyle"].asCString();
				const char* machineStyleName = ctx["machineStyleName"].asCString();
				const char* halfMaterialNumber = ctx["halfMaterialNumber"].asCString();

				xmlrpc_value* resultP = xmlrpc_client_call(&env, get_db_server_url(), "proc.tcon_testplan_exist", "({s:s,s:s,s:s})", "boardStyle", boardStyle, "machineStyleName", machineStyleName, "halfMaterialNumber", halfMaterialNumber);
				if (env.fault_occurred) {
					L_WARN("proc.tcon_testplan_exist.{0}",env.fault_string);
					notify_result(typeName, areaName, machineName, -1);
					continue;
				}
				int ec;
				xmlrpc_decompose_value(&env, resultP, "{s:i,*}", "ec", &ec);
				xmlrpc_DECREF(resultP);

				
				if (env.fault_occurred || ec <= EC_SUCCESS) {
					L_WARN("fault_cde: {0}: size: {1}", env.fault_code, ec);
					notify_result(typeName, areaName, machineName, -1);
					continue;
				}

				//L_INFO("[{0}] = machineName: {1}, boardStyle: {2}, machineStyleName: {3}, halfMaterialNumber: {4}", i, machineName, boardStyle, machineStyleName, halfMaterialNumber);
				xmlrpc_value* d = xmlrpc_struct_new(&env);
				xmlrpc_struct_set_value(&env, d, "areaName", xmlrpc_string_new(&env, areaName));
				xmlrpc_struct_set_value(&env, d, "machineName", xmlrpc_base64_new(&env, strlen(machineName), (unsigned char*)machineName));
				xmlrpc_struct_set_value(&env, d, "boardStyle", xmlrpc_string_new(&env, boardStyle));
				xmlrpc_struct_set_value(&env, d, "machineStyleName", xmlrpc_string_new(&env, machineStyleName));
				xmlrpc_struct_set_value(&env, d, "halfMaterialNumber", xmlrpc_string_new(&env, halfMaterialNumber));
				xmlrpc_struct_set_value(&env, d, "stat", xmlrpc_int_new(&env, 1));

				xmlrpc_array_append_item(&env, array, d);
			}

			if (env.fault_occurred) {
				break;
			}

			xmlrpc_value* resultP = xmlrpc_client_call(&env, get_db_server_url(), "proc.update_recipe", "({s:s,s:i,s:A})", "testtype", typeName, "size", xmlrpc_array_size(&env, array), "data", array);
			xmlrpc_DECREF(array);
			if (env.fault_occurred) {
				continue;
			}

			xmlrpc_value* ecP;
			xmlrpc_struct_read_value(&env, resultP, "ec", &ecP);
			xmlrpc_DECREF(resultP);
			if (env.fault_occurred) {
				break;
			}
			int ec;
			xmlrpc_read_int(&env, ecP, &ec);
			if (env.fault_occurred || ec < 0) {
				break;
			}

			postfields.buf_ = buffer;
		} while (0);
		
		check_upgrade_postfields_.push_front(postfields);
	}

	check_upgrade_stoped_ = true;

	curl_easy_cleanup(curl);
}

void check_update_machine(const char* type)
{
	std::lock_guard<std::mutex> lock1(check_upgrade_mutex_);
	auto iter = std::find_if(check_upgrade_postfields_.begin(), check_upgrade_postfields_.end(),
		[type](const check_upgrade_postfields& c)
	{
		return c.type_ == type;
	});

	if (iter != check_upgrade_postfields_.end()) {
		return;
	}

	Json::Value root;
	root["reType"] = type;
	Json::Value requesetData;
	root["requesetData"].append(requesetData);
	Json::FastWriter writer;

	check_upgrade_postfields check;
	check.type_ = type;
	check.fields_ = writer.write(root);
	check.buf_ = "";
	check.ts_ = 0;
	check_upgrade_postfields_.push_back(check);
}

void notify_result(const char* type, const char* line, const char* machine, int result)
{
	std::lock_guard<std::mutex> lock(mutex_);
	upgrade_result up_result = { 0 };
	const char* p = strrchr(type, '_');
	if (!p) { // 非法请求
		return;
	}

	strncpy_s(up_result.type_, type, (p + 1) - type);
	strcpy_s(up_result.line_, line);
	strcpy_s(up_result.machine_, machine);
	up_result.result_ = result;

	upgrade_result_.push(up_result);
}

static void notify_upgrade_result()
{
	CURL* curl = curl_easy_init();
	char buffer[4096] = { 0 };

// 	curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.8.131");
// 	curl_easy_setopt(curl, CURLOPT_PORT, 8080);
	curl_easy_setopt(curl, CURLOPT_URL, center_url_);
	curl_easy_setopt(curl, CURLOPT_PORT, center_port_);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_result_data);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	Json::Reader reader;
	for (; !check_upgrade_stoped_;) {
		std::this_thread::sleep_for(std::chrono::milliseconds(300));

		std::lock_guard<std::mutex> lock(mutex_);
		if (upgrade_result_.empty()) {
			continue;
		}

		upgrade_result result = upgrade_result_.front();
		upgrade_result_.pop();

		char reType[200] = { 0 };
		Json::Value root;
		if (result.result_ != EC_SUCCESS) {
			sprintf_s(reType, "%s%s", result.type_, "NULL");
		}
		else {
			sprintf_s(reType, "%s%s", result.type_, "RESULT");
		}
		root["reType"] = reType;
		root["lineName"] = result.line_;
		root["machineName"] = result.machine_;
// 		root["result"] = result.result_;

		Json::FastWriter writer;
		std::string postfields = writer.write(root);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.size());

		L_TRACE(postfields.c_str());
		memset(buffer, 0, sizeof(buffer));
		if (curl_easy_perform(curl) != CURLE_OK) {
			continue;
		}

		L_DEBUG(buffer);
		if (!reader.parse(buffer, root)) {
			upgrade_result_.push(result);
			continue;
		}

		if (!root["code"].isString()) {
			upgrade_result_.push(result);
			continue;
		}
	}

	curl_easy_cleanup(curl);
}

static size_t on_write_result_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	char* buffer = (char*)userdata;
	int len = (int)strlen(buffer);
	memcpy_s(buffer + len, 4096 - len, ptr, size * nmemb);

	return size * nmemb;
}

static size_t on_write_data(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	std::string* buffer = (std::string*)userdata;
	buffer->append(ptr, size * nmemb);

	return size * nmemb;
}

void start_check_upgrade_tv(const char* url, unsigned short port)
{
	strcpy_s(center_url_, url);
	center_port_ = port;

	L_INFO("中控地址: {0}:{1}", center_url_, center_port_);

	stop_check_upgrade_tv_ = check_upgrade_stoped_tv_ = false;
	std::thread thrtv(check_upgrade_proc_tv);
	thrtv.detach();

	std::thread thrtv1(notify_upgrade_result_tv);
	thrtv1.detach();
}

void stop_check_upgrade_tv()
{
	stop_check_upgrade_tv_ = true;

	while (!check_upgrade_stoped_tv_) {
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}
static void check_upgrade_proc_tv()
{
	//lock_it();
	static xmlrpc_env env;
	/* Initialize our error-handling environment. */
	xmlrpc_env_init(&env);

	std::string buffer;

	CURL* curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, center_url_);
	curl_easy_setopt(curl, CURLOPT_PORT, center_port_);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_data);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	for (;;) {
		if (stop_check_upgrade_tv_) {
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(30));

		std::lock_guard<std::mutex> m(check_upgrade_mutex_tv_);
		if (check_upgrade_postfields_tv_.empty()) {
			//L_INFO("换产请求信息队列为空!");
			continue;
		}

		auto postfields = check_upgrade_postfields_tv_.back();
		check_upgrade_postfields_tv_.pop_back();
		do {
			if (GetTickCount() - postfields.ts_ < 5 * 1000) {
				break;
			}
			postfields.ts_ = GetTickCount();

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.fields_.c_str());
			curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.fields_.size());

			buffer.clear();
			if (curl_easy_perform(curl) != CURLE_OK) {
				break;
			}
			buffer.append("\0");

			// 如果没有换产，中控不推送数据
			if (buffer.empty()) {
				L_INFO("当前没有换产请求信息!");
				break;
			}

			Json::Value root;
			Json::Reader reader;
			if (!reader.parse(buffer.c_str(), root)) {
				break;
			}

			if (root["code"].asString() != "OK") {
				break;
			}

			xmlrpc_value* array = xmlrpc_array_new(&env);

			Json::Value& data = root["data"];
			Json::Value& type = root["requestType"];
			if (!type.isString()) {
				break;
			}

			CleanClientEnv();

			const char* typeName = type.asCString();
			for (int i = 0; i < (int)data.size(); i++) {
				Json::Value& ctx = data[i];
				const char* areaName = ctx["areaName"].asCString();//线体
				const char* machineName = ctx["machineName"].asCString();//机台
				const char* templatetype = ctx["boardStyle"].asCString();//板型
				const char* modulebom = ctx["machineStyleBom"].asCString();//机型BOM
				const char* materialno = ctx["componentNum"].asCString();//组件号
				wchar_t wcs_deviceno[256] = { 0 };
				char str[20] = { 0 };
				strcpy_s(str, machineName);
				int k=0,j=0,num;
				int a[50];
				while (str[k] != '\0') 
				{
					num = 0;
					while (str[k] >= '0' && str[k] <= '9') {				//判断当前字符是否为数字字符
						if (str[k + 1] >= '0' && str[k + 1] <= '9') {		//如果下一个字符仍是数字字符，就把这个数字字符取出并使它变高一位
							num += str[k] - '0';
							num *= 10;
							k++;
						}
						else {									//如果下一个字符不是数字字符，就把这个字符累加到已经取出的数字上，并结束本次循环						
							num += str[k] - '0';
							a[j] = num;
							j++;
							break;
						}
					}
					k++;
				}
				int t = a[0];
				char tstr[10] = { 0 };
				_itoa_s(t,tstr,10);
				char newStr[100] = "";
				strcat_s(newStr, "NO");
				strcat_s(newStr, tstr);
				const char* machname;
				machname = newStr;
				MultiByteToWideChar(CP_ACP, 0, machineName, strlen(machineName), wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0]));
				xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(&env, wcs_deviceno, wcsnlen_s(wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0])));
				if (env.fault_occurred) {
					break;
				}
				//先查询当前线体的测试方案是否存在，如果不存在的话将线体条件删除
				char deviceno_str[200] = { 0 };
				memcpy_s(deviceno_str, 200, xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));
				xmlrpc_value* resultP = xmlrpc_client_call(&env, get_db_server_url(), "proc.tv_testplan_exist", "({s:s,s:s,s:s,s:s,s:s})", 
					"templatetype", templatetype, "modulebom", modulebom, "materialno", materialno,
					"areaname", areaName, "machinename", "");
				if (env.fault_occurred) {
					L_WARN("proc.tv_testplan_exist.{0}", env.fault_string);
					notify_result_tv(typeName, areaName, machineName, -1);
					continue;
				}
				int ec;
				xmlrpc_decompose_value(&env, resultP, "{s:i,*}", "ec", &ec);
				xmlrpc_DECREF(resultP);


				if (env.fault_occurred || ec < EC_SUCCESS) {
					L_WARN("fault_cde: {0}: size: {1}", env.fault_code, ec);
					notify_result_tv(typeName, areaName, machineName, -1);
					continue;
				}
				if (ec==0)
				{
					xmlrpc_value* resultP = xmlrpc_client_call(&env, get_db_server_url(), "proc.tv_testplan_exist", "({s:s,s:s,s:s,s:s,s:s})",
						"templatetype", templatetype, "modulebom", modulebom, "materialno", materialno,
						"areaname", "", "machinename", "");
					if (env.fault_occurred)
					{
						L_WARN("proc.tv_testplan_exist.{0}", env.fault_string);
						notify_result_tv(typeName, areaName, machineName, -1);
						continue;
					}
					xmlrpc_decompose_value(&env, resultP, "{s:i,*}", "ec", &ec);
					xmlrpc_DECREF(resultP);


					if (env.fault_occurred || ec < EC_SUCCESS) {
						L_WARN("fault_cde: {0}: size: {1}", env.fault_code, ec);
						notify_result_tv(typeName, areaName, machineName, -1);
						continue;
					}
				}
				//L_INFO("[{0}] = machineName: {1}, boardStyle: {2}, machineStyleName: {3}, halfMaterialNumber: {4}", i, machineName, boardStyle, machineStyleName, halfMaterialNumber);
				xmlrpc_value* d = xmlrpc_struct_new(&env);
				xmlrpc_struct_set_value(&env, d, "AreaName", xmlrpc_string_new(&env, areaName));
				//xmlrpc_struct_set_value(&env, d, "MachineName", xmlrpc_base64_new(&env, strlen(machineName), (unsigned char*)machineName));
				xmlrpc_struct_set_value(&env, d, "MachineName", xmlrpc_string_new(&env, machname));

				xmlrpc_struct_set_value(&env, d, "TemplateType", xmlrpc_string_new(&env, templatetype));
				xmlrpc_struct_set_value(&env, d, "ModuleBom", xmlrpc_string_new(&env, modulebom));
				xmlrpc_struct_set_value(&env, d, "MaterialNo", xmlrpc_string_new(&env, materialno));
				xmlrpc_struct_set_value(&env, d, "stat", xmlrpc_int_new(&env, 1));

				xmlrpc_array_append_item(&env, array, d);
			}

			if (env.fault_occurred) {
				break;
			}

			xmlrpc_value* resultP = xmlrpc_client_call(&env, get_db_server_url(), "proc.update_recipe_tv", "({s:s,s:i,s:A})", "testtype", typeName, "size", xmlrpc_array_size(&env, array), "data", array);
			xmlrpc_DECREF(array);
			if (env.fault_occurred) {
				continue;
			}

			xmlrpc_value* ecP;
			xmlrpc_struct_read_value(&env, resultP, "ec", &ecP);
			xmlrpc_DECREF(resultP);
			if (env.fault_occurred) {
				break;
			}
			int ec;
			xmlrpc_read_int(&env, ecP, &ec);
			if (env.fault_occurred || ec < 0) {
				break;
			}

			postfields.buf_ = buffer;
		} while (0);

		check_upgrade_postfields_tv_.push_front(postfields);
	}

	check_upgrade_stoped_tv_ = true;

	curl_easy_cleanup(curl);
}
void check_update_machine_tv(const char* type)
{
	std::lock_guard<std::mutex> lock1(check_upgrade_mutex_tv_);
	auto iter = std::find_if(check_upgrade_postfields_tv_.begin(), check_upgrade_postfields_tv_.end(),
		[type](const check_upgrade_postfields_tv& c)
	{
		return c.type_ == type;
	});

	if (iter != check_upgrade_postfields_tv_.end()) {
		return;
	}

	Json::Value root;
	root["reType"] = type;
	Json::Value requesetData;
	root["requesetData"].append(requesetData);
	Json::FastWriter writer;

	check_upgrade_postfields_tv check;
	check.type_ = type;
	check.fields_ = writer.write(root);
	check.buf_ = "";
	check.ts_ = 0;
	check_upgrade_postfields_tv_.push_back(check);
}
void notify_result_tv(const char* type, const char* line, const char* machine, int result)
{
	std::lock_guard<std::mutex> lock(mutex_tv_);
	upgrade_result_tv up_result = { 0 };
	const char* p = strrchr(type, '_');
	if (!p) { // 非法请求
		return;
	}

	strncpy_s(up_result.type_, type, (p + 1) - type);
	strcpy_s(up_result.line_, line);
	strcpy_s(up_result.machine_, machine);
	up_result.result_ = result;

	upgrade_result_tv_.push(up_result);
}

static void notify_upgrade_result_tv()
{
	CURL* curl = curl_easy_init();
	char buffer[4096] = { 0 };

	// 	curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.8.131");
	// 	curl_easy_setopt(curl, CURLOPT_PORT, 8080);
	curl_easy_setopt(curl, CURLOPT_URL, center_url_);
	curl_easy_setopt(curl, CURLOPT_PORT, center_port_);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, on_write_result_data);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	Json::Reader reader;
	for (; !check_upgrade_stoped_;) {
		std::this_thread::sleep_for(std::chrono::milliseconds(300));

		std::lock_guard<std::mutex> lock(mutex_tv_);
		if (upgrade_result_tv_.empty()) {
			continue;
		}

		upgrade_result_tv result = upgrade_result_tv_.front();
		upgrade_result_tv_.pop();

		char reType[200] = { 0 };
		Json::Value root;
		if (result.result_ != EC_SUCCESS) {
			sprintf_s(reType, "%s%s", result.type_, "NULL");
		}
		else {
			sprintf_s(reType, "%s%s", result.type_, "RESULT");
		}
		root["reType"] = reType;
		root["lineName"] = result.line_;
		root["machineName"] = result.machine_;
		// 		root["result"] = result.result_;

		Json::FastWriter writer;
		std::string postfields = writer.write(root);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postfields.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postfields.size());

		L_TRACE(postfields.c_str());
		memset(buffer, 0, sizeof(buffer));
		if (curl_easy_perform(curl) != CURLE_OK) {
			continue;
		}

		L_DEBUG(buffer);
		if (!reader.parse(buffer, root)) {
			upgrade_result_tv_.push(result);
			continue;
		}

		if (!root["code"].isString()) {
			upgrade_result_tv_.push(result);
			continue;
		}
	}

	curl_easy_cleanup(curl);
}