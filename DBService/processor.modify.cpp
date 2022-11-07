
#include "processor.h"

#include "mysqlxwapper.h"
#include "spdlogwapper.hpp"
#include "../GlobDef.h"
#include "md5.h"

#include <mysqlx/xdevapi.h>

#include <string>
#include "json.h"

#define RESET_PASSWD "passwd"

extern class mysqlx_session* mysqlx_session_;

xmlrpc_value*
proc_addusr(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	char* creator = 0;
	char* usr = 0;
	char* pass = 0;
	char* auth = 0;
	char* nick = 0;
	int nick_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:6,s:s,*})", "usr", &usr, "pass", &pass, "auth", &auth, "nick", &nick, &nick_len, "creator", &creator);

	CheckIfFaultOccurredServer();

	if (!check_auth(creator, auth)) {
		return build_ret_value(EC_AUTH_FAILED, dataP)
	}

	char nick_str[200] = { 0 };
	memcpy_s(nick_str, sizeof(nick_str) / sizeof(nick_str[0]) - 1, nick, nick_len);
	int ec = mysqlx_session_->add_usr(usr, pass, auth, nick_str, creator);
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_resetpass(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	const char* session = 0;

	xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &uuid, &session);

	CheckIfFaultOccurredServer();

	if (!check_auth2(session, uuid)) {
		return build_ret_value(EC_AUTH_FAILED, dataP)
	}

	int ec = mysqlx_session_->modify_usr_pass(uuid, RESET_PASSWD, session);
	return build_ret_value(ec, dataP)
}

xmlrpc_value*
proc_blockusr(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	const char* session = 0;

	xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &uuid, &session);
	CheckIfFaultOccurredServer();

	if (!check_auth2(session, uuid)) {
		return build_ret_value(EC_AUTH_FAILED, dataP);
	}

	int ec = mysqlx_session_->modify_usr_stat(uuid, ST_BLOCK, session);
	return build_ret_value(ec, dataP);
}

// tv4th：新增配置文件
xmlrpc_value*
proc_add_testplan(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	int project;
	const char* board;
	const char* bom;
	int stat;
	const char* session;

	xmlrpc_decompose_value(envP, paramArrayP
		, "({s:i,s:s,s:s,s:i,s:s,*})"
		, "project", &project
		, "board", &board
		, "bom", &bom
		, "stat", &stat
		, "session", &session);
	CheckIfFaultOccurredServer();

// 	if (!check_auth2(session, session)) {
// 		return build_ret_value(EC_AUTH_FAILED, dataP)
// 	}

	int ec = mysqlx_session_->add_testplan(project, board, bom, stat, session);

	return build_ret_value(ec, dataP);
}

// tcon：保存配置文件大小(下载下来创建文件需要用的)
xmlrpc_value*
proc_recipe_size(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	int size;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,*})", "name", &name, "size", &size);

	CheckIfFaultOccurredServer();

	int32_t ec = mysqlx_session_->add_tcon_recipe_size(name, size);

	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_recipe_size_TV(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	int size;
	const char* line;
	const char* deviceno;
	int device_len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:s,s:6,*})", "name", &name, "size", &size, "line", &line, "deviceno", &deviceno,&device_len);

	CheckIfFaultOccurredServer();

	int32_t ec = mysqlx_session_->add_tv_recipe_size(name, size,line,deviceno);

	return build_ret_value(ec, dataP);
}
// 保存配置文件块，文件分块是客户端完成的，服务端并不关心，只需要保存即可
xmlrpc_value*
proc_save_recipe(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* name;
	const char* md5_str;
	const char* ctx;
	int offset;
	int size;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:i,s:i,*})", "name", &name, "md5", &md5_str, "data", &ctx, "offset", &offset, "size", &size);

	CheckIfFaultOccurredServer();

	int ec = mysqlx_session_->add_tcon_testplan(name, md5_str, ctx, offset, size, "");

	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_save_recipe_TV(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* name;
	const char* line;
	const char* deviceno;
	const char* md5_str;
	const char* ctx;
	int offset;
	int size;
	int device_len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,s:s,s:s,s:i,s:i,*})", "name", &name, "line", &line, "deviceno", &deviceno,&device_len, "md5", &md5_str, "data", &ctx, "offset", &offset, "size", &size);

	CheckIfFaultOccurredServer();

	int ec = mysqlx_session_->add_tv_testplan(name,md5_str, ctx, offset, size, "");

	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_addtestdatabase(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_int_new(envP, 0);

	char* sn;
	char* mac;
	char* softwareversion;
	char* key;
	char* testresult;
	char* testplanname;
	int testplan_len = 0;
	char* modulebom;
	char* areaname;
	char* deviceno;
	char* toolno;
	int toolno_len = 0;
	char* failprojectname;
	int failprj_len = 0;
	char* failsubprojectname;
	int failsubprj_len = 0;
	char* testip;
	char* pcname;
	char* testtime;
	char* testperson;
	xmlrpc_decompose_value(envP, paramArrayP,
		"({s:s,s:s,s:s,s:s,s:s,s:6,s:s,s:s,s:s,s:6,s:6,s:6,s:s,s:s,s:s,s:s,*})",
		"sn", &sn, "mac", &mac, "softwareversion", &softwareversion, "key", &key, "testresult", &testresult,
		"testplanname", &testplanname, &testplan_len, "modulebom", &modulebom, "areaname", &areaname, "deviceno", &deviceno,
		"toolno", &toolno, &toolno_len, "failprojectname", &failprojectname, &failprj_len, "failsubprojectname", &failsubprojectname, &failsubprj_len,
		"testip", &testip, "pcname", &pcname, "testtime", &testtime, "testperson", &testperson);

	CheckIfFaultOccurredServer();

	xmlrpc_DECREF(dataP);

	char testplan_str[200] = { 0 };
	memcpy_s(testplan_str, sizeof(testplan_str) / sizeof(testplan_str[0]) - 1, testplanname, testplan_len);
	char toolno_str[200] = { 0 };
	memcpy_s(toolno_str, sizeof(toolno_str) / sizeof(toolno_str[0]) - 1, toolno, toolno_len);
	char failprj_str[200] = { 0 };
	memcpy_s(failprj_str, sizeof(failprj_str) / sizeof(failprj_str[0]) - 1, failprojectname, failprj_len);
	char failsubprj_str[200] = { 0 };
	memcpy_s(failsubprj_str, sizeof(failsubprj_str) / sizeof(failsubprj_str[0]) - 1, failsubprojectname, failsubprj_len);

	int ec = mysqlx_session_->add_testdatabase(sn, mac, softwareversion, key, testresult,
		testplan_str, modulebom, areaname, deviceno, toolno_str, failprj_str,
		failsubprj_str, testip, pcname, testtime, testperson);
	return xmlrpc_build_value(envP, "i", ec);
}

xmlrpc_value*
proc_addtestdatabasedetail(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_int_new(envP, 0);

	char* uuid;
	char* sn;
	char* id;
	char* testprojectname;
	int testprj_len = 0;
	char* subtestprojectname;
	int subtestprj_len = 0;
	char* testresult;
	char* testval;
	char* testlowlimit;
	char* testhightlimit;
	char* ismain;
	char* testtime;
	xmlrpc_decompose_value(envP, paramArrayP,
		"({s:s,s:s,s:s,s:6,s:6,s:s,s:s,s:s,s:s,s:s,s:s,*})",
		"uuid", &uuid, "sn", &sn, "id", &id, "testprojectname", &testprojectname, &testprj_len, "subtestprojectname", &subtestprojectname, &subtestprj_len,
		"testresult", &testresult, "testval", &testval, "testlowlimit", &testlowlimit, "testhightlimit", &testhightlimit,
		"ismain", &ismain, "testtime", &testtime);

	CheckIfFaultOccurredServer();

	xmlrpc_DECREF(dataP);

	char testprj_str[200] = { 0 };
	memcpy_s(testprj_str, sizeof(testprj_str) / sizeof(testprj_str[0]) - 1, testprojectname, testprj_len);
	char subtestprj_str[200] = { 0 };
	memcpy_s(subtestprj_str, sizeof(subtestprj_str) / sizeof(subtestprj_str[0]) - 1, subtestprojectname, subtestprj_len);

	int ec = mysqlx_session_->add_testdatabasedetail(uuid, sn, id, testprj_str, subtestprj_str, testresult, testval, testlowlimit, testhightlimit, ismain, testtime);
	return xmlrpc_build_value(envP, "i", ec);
}

xmlrpc_value*
proc_update_databasebyuuid(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_int_new(envP, 0);

	char* uuid = 0;
	char* failprjname = 0;
	int prj_len = 0;
	char* failsubprjname = 0;
	int subprj_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:6,s:6,*})", "uuid", &uuid, "failprjname", &failprjname, &prj_len,
		"failsubprjname", &failsubprjname, &subprj_len);

	CheckIfFaultOccurredServer();

	xmlrpc_DECREF(dataP);

	char failprjname_str[200] = { 0 };
	memcpy_s(failprjname_str, sizeof(failprjname_str) / sizeof(failprjname_str[0]) - 1, failprjname, prj_len);
	char failsubprjname_str[200] = { 0 };
	memcpy_s(failsubprjname_str, sizeof(failsubprjname_str) / sizeof(failsubprjname_str[0]) - 1, failsubprjname, subprj_len);
	int ec = mysqlx_session_->update_testdatabase(uuid, failprjname_str, failsubprjname_str);
	return xmlrpc_build_value(envP, "i", ec);
}

std::string UTF8ToGBK(const char* strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	return strTemp;
}

// 更新配置文件
xmlrpc_value*
proc_update_recipe(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	int size = 0;
	xmlrpc_value* valueP;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:A,*})", "testtype", &type, "size", &size, "data", &valueP);
	CheckIfFaultOccurredServer();

	int ec;
	int c = xmlrpc_array_size(envP, valueP);
	for (int i = 0; i < c; i++) {
		xmlrpc_value* v;
		xmlrpc_array_read_item(envP, valueP, i, &v);

		xmlrpc_value* areaName;
		const char* areaName_str;
		xmlrpc_struct_read_value(envP, v, "areaName", &areaName);
		xmlrpc_read_string(envP, areaName, &areaName_str);

		xmlrpc_value* machineName;
		size_t machineName_len;
		const unsigned char* machineName_b64;
		xmlrpc_struct_read_value(envP, v, "machineName", &machineName);
		xmlrpc_read_base64(envP, machineName, &machineName_len, &machineName_b64);
		char machineName_str[200] = { 0 };
		memcpy(machineName_str, machineName_b64, machineName_len);

		xmlrpc_value* boardStyle;
		const char* boardStyle_str;
		xmlrpc_struct_read_value(envP, v, "boardStyle", &boardStyle);
		xmlrpc_read_string(envP, boardStyle, &boardStyle_str);

		xmlrpc_value* machineStyleName;
		const char* machineStyleName_str;
		xmlrpc_struct_read_value(envP, v, "machineStyleName", &machineStyleName);
		xmlrpc_read_string(envP, machineStyleName, &machineStyleName_str);

		xmlrpc_value* halfMaterialNumber;
		const char* halfMaterialNumber_str;
		xmlrpc_struct_read_value(envP, v, "halfMaterialNumber", &halfMaterialNumber);
		xmlrpc_read_string(envP, halfMaterialNumber, &halfMaterialNumber_str);

		xmlrpc_value* stat;
		int stat_val;
		xmlrpc_struct_read_value(envP, v, "stat", &stat);
		xmlrpc_read_int(envP, stat, &stat_val);

		ec = mysqlx_session_->updata_tcon_upgrade(type, areaName_str, machineName_str, boardStyle_str, machineStyleName_str, halfMaterialNumber_str, stat_val);
		if (ec < 0) {
			break;
		}
	}

	return build_ret_value(ec, dataP);
}

// 更新机台，线体更新配置文件完成
xmlrpc_value*
proc_update_recipe_machine(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	const char* area;
	const char* machine;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,*})", "testtype", &type, "area", &area, "machine", &machine, &machine_len);
	CheckIfFaultOccurredServer();

	char machine_str[200] = { 0 };
	memcpy_s(machine_str, sizeof(machine_str) / sizeof(machine_str[0]) - 1, machine, machine_len);
	int ec = mysqlx_session_->updata_tcon_upgrade_machine(type, area, machine_str, 0);
	return build_ret_value(ec, dataP);
}

// tv4th：新建测试方案
xmlrpc_value*
proc_new_testplan(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	int ec = mysqlx_session_->new_testplan(project, board, bom);

	return build_ret_value(ec, dataP);
}

// tv4th：测试方案大小，客户端创建文件用
xmlrpc_value*
proc_update_testplan_size(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	int size;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:i,*})", "project", &project, "board", &board, "bom", &bom, "size", &size);
	CheckIfFaultOccurredServer();

	int ec = mysqlx_session_->update_testplan_size(project, board, bom, size);
	return build_ret_value(ec, dataP);
}

// tv4th：更新配置文件
xmlrpc_value*
proc_update_testplan_trunk(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	const char* md5_str;
	const char* ctx;
	int offset;
	int size;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:s,s:s,s:i,s:i,*})", "project", &project, "board", &board, "bom", &bom, "md5", &md5_str, "data", &ctx, "offset", &offset, "size", &size);
	CheckIfFaultOccurredServer();

	int ec = mysqlx_session_->update_testplan_trunk(project, board, bom, md5_str, ctx, offset, size, "");
	return build_ret_value(ec, dataP);
}

//////////////////////////////////////////////////////////////////////////////////////
bool check_auth(const char* src_uuid, const char* dst_auth)
{
	if (!src_uuid || !*src_uuid) {
		return false;
	}

	unsigned src_value = 0, dst_value;
	auto cb = [&](const std::string& uuid, unsigned value)
	{
		if (src_uuid == uuid) {
			src_value = value;
		}
	};
	if (mysqlx_session_->query_usrauth(src_uuid, cb) != 1) { // 查询出错
		return false;
	}
	auto cb_auth = [&](const std::string& uuid, unsigned value)
	{
		if (dst_auth == uuid) {
			dst_value = value;
		}
	};
	if (mysqlx_session_->query_auth(dst_auth, cb_auth) != 1) { // 查询出错
		return false;
	}

	return src_value > dst_value;
}

#pragma warning(disable: 4996)
bool check_auth2(const char* src_uuid, const char* dst_uuid)
{
	if (!src_uuid || !*src_uuid) {
		return false;
	}
	if (!dst_uuid || !*dst_uuid) {
		return false;
	}

	if (!strcmpi(src_uuid, dst_uuid)) { // 自己可以操作自己
		return true;
	}

	unsigned src_auth, dst_auth;
	src_auth = dst_auth = 0;
	auto cb = [&](const std::string& uuid, unsigned value)
	{
		if (uuid == src_uuid) {
			src_auth = value;
		}
		else {
			dst_auth = value;
		}
	};
	if (mysqlx_session_->query_usrauth2(src_uuid, dst_uuid, cb) != 1) { // 查询出错
		return false;
	}

	return src_auth > dst_auth;
}

void notify_result(const char* line, const char* machine, int result)
{
}
void notify_result_tv(const char* line, const char* machine, int result)
{
}
xmlrpc_value*
proc_test_json(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	return nullptr;
}

xmlrpc_value*
proc_test_json_insert(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_test_json_insert is called");
	const char* area;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,*})", "testdata", &area, &machine_len);
	CheckIfFaultOccurredServer();

	std::string strtocut = area;
	strtocut = strtocut.substr(strtocut.find("TestData")-1,strtocut.find_last_of("]")-strtocut.find("TestData")+2);
	//L_INFO("the str is: {0}", strtocut);
	Json::Value v;
	Json::Reader reader;
	if (reader.parse(area, v))
	{
		L_INFO("proc_test_json @ {0}", v["MachineNo"].asString());
	}
	else {
		L_ERROR("proc_test_json parse json ERROR!");
		return build_ret_value(EC_DB_ERROR, dataP);
	}
	
	std::string cBoardType = v["PlateType"].asString();
	std::string cHalfMaterialNum = v["SemiProductNo"].asString();
	std::string cAreaName = v["AreaName"].asString();
	std::string cMachineNo = v["MachineNo"].asString();
	std::string cUserName = v["UserName"].asString();
	std::string cDataArray = strtocut.c_str();
	int ec = mysqlx_session_->test_json_insert(cBoardType.c_str(),cHalfMaterialNum.c_str(),cAreaName.c_str(),cMachineNo.c_str(),cUserName.c_str(),strtocut.c_str());
	return build_ret_value(ec, dataP);
}

// tv4th：新增板型
xmlrpc_value*
proc_add_template(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* project;
	const char* templatetype;
	int project_len = 0;
	const char* creator;

	xmlrpc_decompose_value(envP, paramArrayP
		, "({s:6,s:s,s:s,*})"
		, "project", &project,&project_len
		, "templatetype", &templatetype
		, "creator", &creator);
	CheckIfFaultOccurredServer();

	char project_str[200] = { 0 };
	memcpy_s(project_str, sizeof(project_str) / sizeof(project_str[0]) - 1, project, project_len);
	int ec = mysqlx_session_->add_template(project_str,templatetype,creator);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_test_json_insert_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_test_json_insert_tv is called");
	const char* area;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,*})", "testdata", &area, &machine_len);
	CheckIfFaultOccurredServer();

	std::string strtocut = area;
	strtocut = strtocut.substr(strtocut.find("TestData") - 1, strtocut.find_last_of("]") - strtocut.find("TestData") + 2);
	//L_INFO("the str is: {0}", strtocut);
	Json::Value v;
	Json::Reader reader;
	if (reader.parse(area, v))
	{
		L_INFO("proc_test_json_tv @ {0}", v["MachineNo"].asString());
	}
	else {
		L_ERROR("proc_test_json_tv parse json ERROR!");
		return build_ret_value(EC_DB_ERROR, dataP);
	}

	std::string cTemplateType = v["TemplateType"].asString();//板型
	std::string cMaterialNo = v["MaterialNo"].asString();//物料号
	std::string cAreaName = v["AreaName"].asString();//线体
	std::string cMachineNo = v["MachineNo"].asString();//机台
	std::string cUserName = v["UserName"].asString();//用户名
	std::string cDataArray = strtocut.c_str();
	std::string cProject = v["Project"].asString();//方案
	std::string cShortBom = v["ShortBom"].asString();//机型短码
	std::string cModuleBom = v["ModuleBom"].asString();//机型BOM
	std::string cRecipeName = v["RecipeName"].asString();//文件名称
	int ec = mysqlx_session_->test_json_insert_tv(cRecipeName.c_str(), cAreaName.c_str(), cMachineNo.c_str(), cProject.c_str(), cTemplateType.c_str(),cShortBom.c_str(),cModuleBom.c_str(),cMaterialNo.c_str(),cUserName.c_str(), strtocut.c_str());
	return build_ret_value(ec, dataP);
}

// 更新配置文件
xmlrpc_value*
proc_update_recipe_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	int size = 0;
	xmlrpc_value* valueP;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:A,*})", "testtype", &type, "size", &size, "data", &valueP);
	CheckIfFaultOccurredServer();

	int ec;
	int c = xmlrpc_array_size(envP, valueP);
	for (int i = 0; i < c; i++) {
		xmlrpc_value* v;
		xmlrpc_array_read_item(envP, valueP, i, &v);

		xmlrpc_value* areaName;
		const char* areaName_str;
		xmlrpc_struct_read_value(envP, v, "AreaName", &areaName);
		xmlrpc_read_string(envP, areaName, &areaName_str);


		xmlrpc_value* machineName;
		const char* machineName_str;
		xmlrpc_struct_read_value(envP, v, "MachineName", &machineName);
		xmlrpc_read_string(envP, machineName, &machineName_str);

		/*xmlrpc_value* machineName;
		size_t machineName_len;
		const unsigned char* machineName_b64;
		xmlrpc_struct_read_value(envP, v, "MachineName", &machineName);
		xmlrpc_read_base64(envP, machineName, &machineName_len, &machineName_b64);
		char machineName_str[200] = { 0 };
		memcpy(machineName_str, machineName_b64, machineName_len);*/

		xmlrpc_value* boardStyle;
		const char* boardStyle_str;
		xmlrpc_struct_read_value(envP, v, "TemplateType", &boardStyle);
		xmlrpc_read_string(envP, boardStyle, &boardStyle_str);

		xmlrpc_value* machineStyleName;
		const char* machineStyleName_str;
		xmlrpc_struct_read_value(envP, v, "ModuleBom", &machineStyleName);
		xmlrpc_read_string(envP, machineStyleName, &machineStyleName_str);

		xmlrpc_value* halfMaterialNumber;
		const char* halfMaterialNumber_str;
		xmlrpc_struct_read_value(envP, v, "MaterialNo", &halfMaterialNumber);
		xmlrpc_read_string(envP, halfMaterialNumber, &halfMaterialNumber_str);

		xmlrpc_value* stat;
		int stat_val;
		xmlrpc_struct_read_value(envP, v, "stat", &stat);
		xmlrpc_read_int(envP, stat, &stat_val);

		ec = mysqlx_session_->updata_tv_upgrade(type, areaName_str, machineName_str, boardStyle_str, machineStyleName_str, halfMaterialNumber_str, stat_val);
		if (ec < 0) {
			break;
		}
	}

	return build_ret_value(ec, dataP);
}

// 更新机台，线体更新配置文件完成
xmlrpc_value*
proc_update_recipe_machine_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	const char* area;
	const char* machine;
	int len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,*})", "testtype", &type, "area", &area, "machine", &machine,&len);
	CheckIfFaultOccurredServer();

	char str[20] = { 0 };
	strcpy_s(str, machine);
	int k = 0, j = 0, num;
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
	_itoa_s(t, tstr, 10);
	char newStr[100] = "";
	strcat_s(newStr, "NO");
	strcat_s(newStr, tstr);
	const char* machname;
	machname = newStr;




	int ec = mysqlx_session_->update_tv_upgrade_machine(type, area, machname, 0);
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_add_cor(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{

	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_add_cor is called");
	const char* area;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,*})", "testdata", &area, &machine_len);
	CheckIfFaultOccurredServer();
	Json::Value v;
	Json::Reader reader;
	if (reader.parse(area, v))
	{
		L_INFO("proc_add_cor @ {0}", v["AcBoarType"].asString());
	}
	else {
		L_ERROR("proc_add_corn parse json ERROR!");
		return build_ret_value(EC_DB_ERROR, dataP);
	}

	std::string cBoardType = v["AcBoarType"].asString();
	std::string cProgramNo = v["ProgramNo"].asString();
	std::string cRemarks = v["Remarks"].asString();
	std::string cEditor = v["Editor"].asString();

	int ec = mysqlx_session_->add_cor(cBoardType.c_str(), cProgramNo.c_str(), cRemarks.c_str(), cEditor.c_str());
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_add_platetype(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{

	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_add_platetype is called");
	const char* area;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,*})", "testdata", &area, &machine_len);
	CheckIfFaultOccurredServer();
	Json::Value v;
	Json::Reader reader;
	if (reader.parse(area, v))
	{
		L_INFO("proc_add_platetype @ {0}", v["BoarType"].asString());
	}
	else {
		L_ERROR("proc_add_platetype parse json ERROR!");
		return build_ret_value(EC_DB_ERROR, dataP);
	}

	std::string cBoardType = v["BoarType"].asString();
	std::string cProgramNo = v["moduleNo"].asString();
	std::string cRemarks = v["uuidstr"].asString();
	std::string cEditor = v["Editor"].asString();

	int ec = mysqlx_session_->add_platetype(cBoardType.c_str(), cProgramNo.c_str(), cRemarks.c_str(), cEditor.c_str());
	return build_ret_value(ec, dataP);
}