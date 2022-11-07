
#include "processor.h"

#include "spdlogwapper.hpp"
#include "../GlobDef.h"

#include "xmlrpc-c/base.h"
#include "xmlrpc-c/client.h"
#include "xmlrpc-c/client_global.h"
#include "xmlrpc-c/util.h"
#include "json.h"

#define RPCSERVER_NAME			"TVRPCSERVER"
#define RPCSERVER_VERSION		"1.0"

static char db_server_url[256] = { 0 };
static xmlrpc_env env;

bool startup_db_client(const char* host, unsigned short port)
{
	/* Initialize our error-handling environment. */
	xmlrpc_env_init(&env);
	if (env.fault_occurred) {
		L_ERROR("{0} @ {1}, fault_string : {2} fault_code {3})",
			__FUNCTION__, __LINE__, env.fault_string, env.fault_code);
		return false;
	}

	/* Create the global XML-RPC client object. */
	xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, RPCSERVER_NAME, RPCSERVER_VERSION, NULL, 0);
	if (env.fault_occurred) {
		L_ERROR("{0} @ {1}, fault_string : {2} fault_code {3})",
			__FUNCTION__, __LINE__, env.fault_string, env.fault_code);
		return false;
	}

	sprintf_s(db_server_url, sizeof(db_server_url) / sizeof(db_server_url[0]), "http://%s:%d/RPC2", host, port);

	L_DEBUG("[DB] addr: {0}", db_server_url);

	return true;
}

void clean_db_client()
{
	/* Clean up our error-handling environment. */
	xmlrpc_env_clean(&env);

	/* Shutdown our XML-RPC client library. */
	xmlrpc_client_cleanup();
}

const char* get_db_server_url()
{
	return db_server_url;
}

xmlrpc_value* 
proc_login(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* usr;
	const char* pass;
	xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &usr, &pass);
	CheckIfFaultOccurredServer();

	xmlrpc_value* resultP = xmlrpc_client_call_params(&env, db_server_url, "proc.login", paramArrayP);
	if (env.fault_occurred) {
		return build_ret_value(EC_DB_ERROR, dataP);
	}
	
	xmlrpc_DECREF(dataP);
	
	decompose_ret_value();

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_usrinfo(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.usrinfo", paramArrayP);
}

xmlrpc_value*
proc_usrauth(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.usrauth", paramArrayP);
}

xmlrpc_value*
proc_authorusr(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.authorusr", paramArrayP);
}

xmlrpc_value*
proc_testplan(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan", paramArrayP);
}

xmlrpc_value*
proc_testplan1(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	call_db_server(testplan1);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_cmd(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_cmd", paramArrayP);
}

xmlrpc_value*
proc_testplan_img(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_img", paramArrayP);
}

xmlrpc_value*
proc_testplan_image(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_image", paramArrayP);
}

xmlrpc_value*
proc_testplan_main(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_main", paramArrayP);
}

xmlrpc_value*
proc_testplan_sound(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_sound", paramArrayP);
}

xmlrpc_value*
proc_testplan_status(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_status", paramArrayP);
}

xmlrpc_value*
proc_testplan_strcmp(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_strcmp", paramArrayP);
}

xmlrpc_value*
proc_testplan_voltage(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_client_call_params(&env, db_server_url, "proc.testplan_voltage", paramArrayP);
}

// 枚举配方信息
xmlrpc_value*
proc_query_recipe(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	L_TRACE("name = {0}", name);
	CheckIfFaultOccurredServer();

	call_db_server(query_recipe);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 枚举配方文件块
xmlrpc_value*
proc_query_recipe_trunk(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	CheckIfFaultOccurredServer();

	call_db_server(query_recipe_trunk);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_query_databaseuuid(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
xmlrpc_value*
proc_addusr(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return 0;
}

xmlrpc_value*
proc_resetpass(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return 0;
}

xmlrpc_value*
proc_blockusr(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return 0;
}

xmlrpc_value*
proc_add_testplan(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

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

	call_db_server(testplan);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 检查tcon测试方案是否存在
xmlrpc_value*
proc_tcon_testplan_exist(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* boardstyle;
	const char* machinestylename;
	const char* halfmaterialnum;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "boardStyle", &boardstyle, "machineStyleName", &machinestylename, "halfMaterialNumber", &halfmaterialnum);
	CheckIfFaultOccurredServer();

	call_db_server(tcon_testplan_exist);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 检查当前文件块是否存在
xmlrpc_value*
proc_check_recipe_trunk(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	xmlrpc_value* arrayP;
	xmlrpc_decompose_value(envP, paramArrayP, "(A)", &arrayP);
	CheckIfFaultOccurredServer();

	if (xmlrpc_value_type(arrayP) != XMLRPC_TYPE_ARRAY || !xmlrpc_array_size(envP, arrayP)) {
		return build_ret_value(EC_INVALID_ARGS, dataP);
	}
	call_db_server(check_recipe_trunk);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_check_recipe_trunk_TV(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	xmlrpc_value* arrayP;
	xmlrpc_decompose_value(envP, paramArrayP, "(A)", &arrayP);
	CheckIfFaultOccurredServer();

	if (xmlrpc_value_type(arrayP) != XMLRPC_TYPE_ARRAY || !xmlrpc_array_size(envP, arrayP)) {
		return build_ret_value(EC_INVALID_ARGS, dataP);
	}
	call_db_server(check_recipe_trunk_TV);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
// tcon：保存配置文件大小
xmlrpc_value*
proc_recipe_size(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	int size;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,*})", "name", &name, "size", &size);
	CheckIfFaultOccurredServer();

	call_db_server(recipe_size);
	decompose_ret_value();
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
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	const char* line;
	const char* deviceno;
	int size;
	int device_len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:s,s:6,*})", "name", &name, "size", &size, "line", &line, "deviceno", &deviceno,&device_len);
	CheckIfFaultOccurredServer();

	call_db_server(recipe_size_TV);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
// 保存配置文件块
xmlrpc_value*
proc_save_recipe(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* name;
	const char* md5_str;
	const char* ctx;
	int offset;
	int size;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:i,s:i,*})", "name", &name, "md5", &md5_str, "data", &ctx, "offset", &offset, "size", &size);
	CheckIfFaultOccurredServer();

	call_db_server(save_recipe);
	decompose_ret_value();
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
	CleanClientEnv();

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

	call_db_server(save_recipe_TV);
	decompose_ret_value();
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
	CleanClientEnv();
	return xmlrpc_build_value(envP, "i", 0);
}

xmlrpc_value*
proc_addtestdatabasedetail(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_build_value(envP, "i", 0);
}

xmlrpc_value*
proc_update_databasebyuuid(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	return xmlrpc_build_value(envP, "i", 0);
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

// tcon：检查机台，线体更新状态
xmlrpc_value*
proc_check_recipe_update(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	const char* area;
	const char* machine;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,*})", "testtype", &type, "area", &area, "machine", &machine, &machine_len);
	std::string tmpMA = UTF8ToGBK(machine);
	std::string tmpArea = area;
	L_INFO("check_recipe_update @ {0}:{1}", tmpMA,tmpArea);
	CheckIfFaultOccurredServer();

	check_update_machine(type);

	call_db_server(check_recipe_update);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// tcon：检查机台，线体更新状态
xmlrpc_value*
proc_update_recipe(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	int size = 0;
	xmlrpc_value* valueP;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:A,*})", "testtype", &type, "size", &size, "data", &valueP);
	CheckIfFaultOccurredServer();

	/*int c = xmlrpc_array_size(envP, valueP);
	for (int i = 0; i < c; i++) {
		xmlrpc_value* v;
		xmlrpc_array_read_item(envP, valueP, i, &v);

		xmlrpc_value* areaName;
		const char* areaName_str;
		xmlrpc_struct_read_value(envP, v, "areaName", &areaName);
		xmlrpc_read_string(envP, areaName, &areaName_str);
		L_WARN("manual single change production-areaName:{0}", areaName_str);

		xmlrpc_value* machineName;
		size_t machineName_len;
		const unsigned char* machineName_b64;
		xmlrpc_struct_read_value(envP, v, "machineName", &machineName);
		xmlrpc_read_base64(envP, machineName, &machineName_len, &machineName_b64);
		char machineName_str[200] = { 0 };
		memcpy(machineName_str, machineName_b64, machineName_len);
		L_WARN("manual single change production-machineName:{0}", UTF8ToGBK(machineName_str));

		xmlrpc_value* boardStyle;
		const char* boardStyle_str;
		xmlrpc_struct_read_value(envP, v, "boardStyle", &boardStyle);
		xmlrpc_read_string(envP, boardStyle, &boardStyle_str);
		L_WARN("manual single change production-boardStyle:{0}", boardStyle_str);

		xmlrpc_value* machineStyleName;
		const char* machineStyleName_str;
		xmlrpc_struct_read_value(envP, v, "machineStyleName", &machineStyleName);
		xmlrpc_read_string(envP, machineStyleName, &machineStyleName_str);

		xmlrpc_value* halfMaterialNumber;
		const char* halfMaterialNumber_str;
		xmlrpc_struct_read_value(envP, v, "halfMaterialNumber", &halfMaterialNumber);
		xmlrpc_read_string(envP, halfMaterialNumber, &halfMaterialNumber_str);
		L_WARN("manual single change production-halfMaterialNumber:{0}", halfMaterialNumber_str);
	}*/
	L_WARN("********CHANGE PRODUCTION!*******");
	call_db_server(update_recipe);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_test_json_insert(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	return nullptr;
}

xmlrpc_value*
proc_test_json(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	
	CheckIfFaultOccurredServer();

	call_db_server(test_json_insert);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

//查询配置文件参数
xmlrpc_value*
proc_query_test_config_data(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(query_test_config_data);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
//获取tcon配置文件详细参数
xmlrpc_value*
proc_get_test_details(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(get_test_details);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 更新配置文件
xmlrpc_value*
proc_update_recipe_machine(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	char* type;
	char* areaname;
	char* machine;
	int len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,*})", "testtype", &type, "area", &areaname, "machine", &machine, &len);
	CheckIfFaultOccurredServer();

	char machine_str[256] = { 0 };
	strncpy_s(machine_str, machine, len);

	call_db_server(update_recipe_machine);
	decompose_ret_value();

	if (ec >= EC_SUCCESS) {
		notify_result(type, areaname, machine_str, EC_SUCCESS);
		L_TRACE("线体: {0}, 机台: {1}, 类型{2} 换产【成功】", areaname, machine_str, type);
	}
	else {
		notify_result(type, areaname, machine_str, EC_DB_ERROR);
		L_TRACE("线体: {0}, 机台: {1}, 类型{2} 换产 #失败#", areaname, machine_str, type);
	}

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
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	call_db_server(new_testplan);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// tv4th：查询机台，线体更新信息
xmlrpc_value*
proc_check_tv_recipe_update(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	const char* area;
	const char* machine;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,*})", "testtype", &type, "area", &area, "machine", &machine, &machine_len);

	CheckIfFaultOccurredServer();

	check_update_machine(type);

	call_db_server(check_tv_recipe_update);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 查询TV测试方案信息
xmlrpc_value*
proc_query_testplan2(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	call_db_server(query_testplan2);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 查询(下载)配置文件块
xmlrpc_value*
proc_query_testplan_trunk(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	call_db_server(query_testplan_trunk);
	decompose_ret_value();
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
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	int size;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:i,*})", "project", &project, "board", &board, "bom", &bom, "size", &size);
	CheckIfFaultOccurredServer();

	call_db_server(update_testplan_size);
	decompose_ret_value();
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
	CleanClientEnv();

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

	call_db_server(update_testplan_trunk);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// tv4th：检查配置文件块是否存在
xmlrpc_value*
proc_query_tv_recipe(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;

	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	call_db_server(query_tv_recipe);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

//////////////////////////////////////////////////////////////////////////////////////
bool check_auth(const char* src_uuid, const char* dst_auth)
{
	return false;
}

bool check_auth2(const char* src_uuid, const char* dst_uuid)
{
	return false;
}
//新建板型
xmlrpc_value*
proc_add_template(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* project;
	int project_len = 0;
	const char* templatetype; 
	const char* creator;

	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,s:s,s:s,*})", "project", &project,&project_len, "templatetype", &templatetype, "creator", &creator);
	CheckIfFaultOccurredServer();

	call_db_server(add_template);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

//
xmlrpc_value*
proc_query_template(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(query_template);
	decompose_ret_value();
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
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(test_json_insert_tv);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
//查询配置文件参数
xmlrpc_value*
proc_query_test_config_data_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(query_test_config_data_tv);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 枚举配方信息
xmlrpc_value*
proc_query_recipe_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	L_TRACE("name = {0}", name);
	CheckIfFaultOccurredServer();

	call_db_server(query_recipe_tv);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
// 枚举配方文件块
xmlrpc_value*
proc_query_recipe_trunk_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	CheckIfFaultOccurredServer();

	call_db_server(query_recipe_trunk_tv);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 检查tcon测试方案是否存在
xmlrpc_value*
proc_tv_testplan_exist(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* templatetype;//板型
	const char* modulebom;//机型（BOM）
	const char* materialno;//物料号
	const char* areaname;//线体
	const char* machineno;//机台
	int machine_len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:s,s:s,*})", "templatetype", &templatetype, "modulebom", &modulebom, "materialno", &materialno, "areaname", &areaname, "machinename", &machineno);
	CheckIfFaultOccurredServer();

	call_db_server(tv_testplan_exist);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
// tv：检查机台，线体更新状态
xmlrpc_value*
proc_check_recipe_update_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	const char* area;
	const char* emachine;
	const char* machine;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:6,*})", "testtype", &type, "area", &area, "emachine", &emachine,"machine", &machine,&machine_len);
	std::string tmpMA = UTF8ToGBK(machine);
	std::string tmpArea = area;
	L_INFO("check_recipe_update_tv @ {0}:{1}", tmpMA, tmpArea);
	CheckIfFaultOccurredServer();

	check_update_machine_tv(type);

	call_db_server(check_recipe_update_tv);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_update_recipe_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* type;
	int size = 0;
	xmlrpc_value* valueP;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:i,s:A,*})", "testtype", &type, "size", &size, "data", &valueP);
	CheckIfFaultOccurredServer();

	L_WARN("********CHANGE PRODUCTION!*******");
	call_db_server(update_recipe_tv);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

// 更新配置文件
xmlrpc_value*
proc_update_recipe_machine_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	char* type;
	char* areaname;
	char* machine;
	int len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,*})", "testtype", &type, "area", &areaname, "machine", &machine,&len);
	CheckIfFaultOccurredServer();

	char machine_str[256] = { 0 };
	strncpy_s(machine_str, machine, len);

	call_db_server(update_recipe_machine_tv);
	decompose_ret_value();

	if (ec >= EC_SUCCESS) {
		notify_result_tv(type, areaname, machine, EC_SUCCESS);
		L_TRACE("线体: {0}, 机台: {1}, 类型{2} 换产【成功】", areaname, machine, type);
	}
	else {
		notify_result_tv(type, areaname, machine, EC_DB_ERROR);
		L_TRACE("线体: {0}, 机台: {1}, 类型{2} 换产 #失败#", areaname, machine, type);
	}

	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_check_filename(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* filename;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,*})", "filename", &filename);

	CheckIfFaultOccurredServer();


	call_db_server(check_filename);
	decompose_ret_value();
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
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(add_cor);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_query_cordata(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(query_cordata);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_query_distinctacType(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(query_distinctacType);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_query_Remark(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(query_Remark);
	decompose_ret_value();
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
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(add_platetype);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_query_distinctpcbtype(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	CheckIfFaultOccurredServer();

	call_db_server(query_distinctpcbtype);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_query_moduleNo(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	CleanClientEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	CheckIfFaultOccurredServer();
	call_db_server(query_moduleNo);
	decompose_ret_value();
	return build_ret_value(ec, dataP);
}