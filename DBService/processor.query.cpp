
#include "processor.h"

#include "mysqlxwapper.h"
#include "spdlogwapper.hpp"
#include "../GlobDef.h"

#include <mysqlx/xdevapi.h>

#include <map>
#include <string>
#include "json.h"

class mysqlx_session* mysqlx_session_ = 0;

xmlrpc_value*
proc_login(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	char* usr = 0;
	char* pass = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "(ss)", &usr, &pass);

	CheckIfFaultOccurredServer();

	auto cb = [&envP, pass, dataP]
	(const std::string& uuid, const std::string& passwd, const std::string& nick, int stat, const std::string& auth_name, int auth_value)
	{
		xmlrpc_set_str_v(dataP, uuid);
		xmlrpc_set_b64_v(dataP, nick);
		xmlrpc_set_int_v(dataP, stat);

		xmlrpc_set_b64_v(dataP, auth_name);
		xmlrpc_set_int_v(dataP, auth_value);
	};
	int ec = mysqlx_session_->query_usrinfo(usr, cb);

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
	return 0;
}

xmlrpc_value*
proc_usrauth(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	return 0;
}

xmlrpc_value*
proc_authorusr(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	return 0;
}

#define testplan_array_item_init(v_) \
	xmlrpc_set_str_v(v_, uuid); \
	xmlrpc_set_str_v(v_, project); \
	xmlrpc_set_str_v(v_, board); \
	xmlrpc_set_str_v(v_, bom); \
	\
	xmlrpc_set_sec_v(v_, create_ts); \
	xmlrpc_set_sec_v(v_, edit_ts); \
	\
	xmlrpc_set_b64_v(v_, desc);

#define testplan_array_cb() \
	auto cb = [&envP, &valueP](const std::string& uuid, const std::string& project, const std::string& board, const std::string& bom, time_t create_ts, time_t edit_ts, const std::string& desc) \
	{ \
		xmlrpc_value* v = xmlrpc_struct_new(envP); \
		\
		testplan_array_item_init(v); \
		\
		xmlrpc_array_append_item(envP, valueP, v); \
	};

xmlrpc_value*
proc_testplan(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &uuid);

	CheckIfFaultOccurredServer();

	auto cb = [&envP, &dataP](const std::string& uuid, const std::string& project, const std::string& board, const std::string& bom, time_t create_ts, time_t edit_ts, const std::string& desc)
	{
		testplan_array_item_init(dataP);
	};
	int ec = mysqlx_session_->query_testplan(uuid, cb);
	
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan1(
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

	xmlrpc_value* valueP = xmlrpc_array_new(envP);

	testplan_array_cb();

	int ec = mysqlx_session_->query_testplan1(project, board, bom, cb);

	xmlrpc_struct_set_value(envP, dataP, "ctx", valueP);
	xmlrpc_DECREF(valueP);

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

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	
	const char* uuid = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &uuid);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, const std::string& board, const std::string& name, const std::string& type, int delay, int timeout, int order, int stat, const std::string& creator, const std::string& editor)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_str_v(v, board);
		xmlrpc_set_b64_v(v, name);
		xmlrpc_set_b64_v(v, type);
		xmlrpc_set_int_v(v, delay);
		xmlrpc_set_int_v(v, timeout);
		xmlrpc_set_int_v(v, order);
		xmlrpc_set_int_v(v, stat);

		xmlrpc_set_b64_v(v, creator);
		xmlrpc_set_b64_v(v, editor);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_cmd(uuid, cb);
	xmlrpc_struct_set_value(envP, dataP, "cmd", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_img(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	xmlrpc_read_string(envP, paramArrayP, &uuid);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, const std::string& board, const std::string& name, const std::string& pipe, int delay, int timeout, std::string img_md5, int order, int stat, const std::string& creator, const std::string& editor)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_str_v(v, board);
		xmlrpc_set_b64_v(v, name);
		xmlrpc_set_b64_v(v, pipe);
		xmlrpc_set_int_v(v, delay);
		xmlrpc_set_int_v(v, timeout);
		xmlrpc_set_str_v(v, img_md5);
		xmlrpc_set_int_v(v, order);
		xmlrpc_set_int_v(v, stat);

		xmlrpc_set_b64_v(v, creator);
		xmlrpc_set_b64_v(v, editor);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_img(uuid, cb);
	xmlrpc_struct_set_value(envP, dataP, "img", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_image(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* img_md5 = 0;
	xmlrpc_read_string(envP, paramArrayP, &img_md5);
	CheckIfFaultOccurredServer();

	auto cb = [&envP, &dataP](const std::string& img_md5, const std::string& content)
	{
		xmlrpc_set_str_v(dataP, img_md5);
		xmlrpc_set_str_v(dataP, content);
	};
	int ec = mysqlx_session_->query_testplan_image(img_md5, cb);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_main(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	xmlrpc_read_string(envP, paramArrayP, &uuid);

	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, int item, const std::string& name, int times, int order, int stat, const std::string& creator, const std::string& editor)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_int_v(v, item);
		xmlrpc_set_b64_v(v, name);
		xmlrpc_set_int_v(v, times);
		xmlrpc_set_int_v(v, order);
		xmlrpc_set_int_v(v, stat);

		xmlrpc_set_b64_v(v, creator);
		xmlrpc_set_b64_v(v, editor);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_main(uuid, cb);

	xmlrpc_struct_set_value(envP, dataP, "main", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_sound(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	xmlrpc_read_string(envP, paramArrayP, &uuid);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, const std::string& board, const std::string& name, int hzmax, int hzmin, int valuemax, int valuemin, int timeout, int order, int stat, const std::string& creator, const std::string& editor)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_str_v(v, board);
		xmlrpc_set_str_v(v, name);
		xmlrpc_set_int_v(v, hzmax);
		xmlrpc_set_int_v(v, hzmin);
		xmlrpc_set_int_v(v, valuemax);
		xmlrpc_set_int_v(v, valuemin);
		xmlrpc_set_int_v(v, timeout);
		xmlrpc_set_int_v(v, order);
		xmlrpc_set_int_v(v, stat);

		xmlrpc_set_b64_v(v, creator);
		xmlrpc_set_b64_v(v, editor);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_sound(uuid, cb);

	xmlrpc_struct_set_value(envP, dataP, "sound", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_status(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* editor = 0;
	xmlrpc_read_string(envP, paramArrayP, &editor);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, int yahe, int sn, int pcbon, int delay, int img, int comopen, int comclose, int mes, int pcboff, int order, int stat, const std::string& creator, const std::string& editor)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_int_v(v, yahe);
		xmlrpc_set_int_v(v, sn);
		xmlrpc_set_int_v(v, pcbon);
		xmlrpc_set_int_v(v, delay);
		xmlrpc_set_int_v(v, img);
		xmlrpc_set_int_v(v, comopen);
		xmlrpc_set_int_v(v, comclose);
		xmlrpc_set_int_v(v, mes);
		xmlrpc_set_int_v(v, pcboff);
		xmlrpc_set_int_v(v, order);
		xmlrpc_set_int_v(v, stat);

		xmlrpc_set_b64_v(v, creator);
		xmlrpc_set_b64_v(v, editor);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_status(editor, cb);

	xmlrpc_struct_set_value(envP, dataP, "status", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_strcmp(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	xmlrpc_read_string(envP, paramArrayP, &uuid);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, const std::string& board, const std::string& name, int valuemin, int valuemax, int timeout, int order, int stat, const std::string& creator, const std::string& editor)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_str_v(v, board);
		xmlrpc_set_b64_v(v, name);
		xmlrpc_set_int_v(v, valuemin);
		xmlrpc_set_int_v(v, valuemax);
		xmlrpc_set_int_v(v, timeout);
		xmlrpc_set_int_v(v, order);
		xmlrpc_set_int_v(v, stat);

		xmlrpc_set_b64_v(v, creator);
		xmlrpc_set_b64_v(v, editor);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_strcmp(uuid, cb);
	xmlrpc_struct_set_value(envP, dataP, "strcmp", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_testplan_voltage(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* uuid = 0;
	xmlrpc_read_string(envP, paramArrayP, &uuid);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, const std::string& board, const std::string& name, int valuemin, int valuemax, int timeout, int order, int stat, const std::string& creator, const std::string& editor)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_str_v(v, board);
		xmlrpc_set_b64_v(v, name);
		xmlrpc_set_int_v(v, valuemin);
		xmlrpc_set_int_v(v, valuemax);
		xmlrpc_set_int_v(v, timeout);
		xmlrpc_set_int_v(v, order);
		xmlrpc_set_int_v(v, stat);

		xmlrpc_set_b64_v(v, creator);
		xmlrpc_set_b64_v(v, editor);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_voltage(uuid, cb);

	xmlrpc_struct_set_value(envP, dataP, "voltage", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}

// 检查测试方案是否存在
xmlrpc_value*
proc_tcon_testplan_exist(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* boardStyle;
	const char* machineStyleName;
	const char* halfMaterialNumber;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "boardStyle", &boardStyle, "machineStyleName", &machineStyleName, "halfMaterialNumber", &halfMaterialNumber);
	CheckIfFaultOccurredServer();

	xmlrpc_int32 ec = mysqlx_session_->check_tcon_testplan_exist(boardStyle, machineStyleName, halfMaterialNumber);

	return build_ret_value(ec, dataP);
}

// 检查配置文件块是否存在
xmlrpc_value*
proc_check_recipe_trunk(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	xmlrpc_value* arrayP;
	xmlrpc_decompose_value(envP, paramArrayP, "(A)", &arrayP);
	CheckIfFaultOccurredServer();

	std::vector<std::string> md5;
	for (int i = 0; i < xmlrpc_array_size(envP, arrayP); i++) {
		xmlrpc_value* m = xmlrpc_array_get_item(envP, arrayP, i);
		const char* md5_str;
		xmlrpc_read_string(envP, m, &md5_str);
		md5.push_back(md5_str);
	}
	xmlrpc_DECREF(arrayP);

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& md5)
	{
		xmlrpc_array_append_item(envP, valueP, xmlrpc_string_new(envP, md5.c_str()));
	};
	xmlrpc_int32 ec = mysqlx_session_->check_tcon_hstvt_trunk(md5, cb);

	xmlrpc_struct_set_value(envP, dataP, "md5", valueP);
	xmlrpc_DECREF(valueP);

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

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	xmlrpc_value* arrayP;
	xmlrpc_decompose_value(envP, paramArrayP, "(A)", &arrayP);
	CheckIfFaultOccurredServer();

	std::vector<std::string> md5;
	for (int i = 0; i < xmlrpc_array_size(envP, arrayP); i++) {
		xmlrpc_value* m = xmlrpc_array_get_item(envP, arrayP, i);
		const char* md5_str;
		xmlrpc_read_string(envP, m, &md5_str);
		md5.push_back(md5_str);
	}
	xmlrpc_DECREF(arrayP);

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& md5)
	{
		xmlrpc_array_append_item(envP, valueP, xmlrpc_string_new(envP, md5.c_str()));
	};
	xmlrpc_int32 ec = mysqlx_session_->check_tv_hstvt_trunk(md5, cb);

	xmlrpc_struct_set_value(envP, dataP, "md5", valueP);
	xmlrpc_DECREF(valueP);

	return build_ret_value(ec, dataP);
}
// 查询配置文件块信息
xmlrpc_value*
proc_query_recipe(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	unsigned size = 0;
	auto cb = [&envP, &valueP, &size](const std::string& md5, unsigned offset, unsigned trunk_size, unsigned file_size)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, md5);
		xmlrpc_set_int_v(v, offset);
		xmlrpc_set_int_v(v, trunk_size);

		size = file_size;

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_tcon_testplan(name, cb);

	xmlrpc_struct_set_value(envP, dataP, "trunk", valueP);
	xmlrpc_DECREF(valueP);

	xmlrpc_struct_set_value(envP, dataP, "file_size", xmlrpc_int_new(envP, size));

	return build_ret_value(ec, dataP);
}

// 查询(下载)配置文件块
xmlrpc_value*
proc_query_recipe_trunk(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	CheckIfFaultOccurredServer();

	auto cb = [&envP, &dataP](const std::string& md5, const std::string content)
	{
		xmlrpc_set_str_v(dataP, md5);
		xmlrpc_set_str_v(dataP, content);
	};
	int ec = mysqlx_session_->query_tcon_hstvt(name, cb);

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

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* sn;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &sn);
	CheckIfFaultOccurredServer();

	auto cb = [&envP, &dataP](const std::string& uuid)
	{
		xmlrpc_struct_set_value(envP, dataP, "uuid", xmlrpc_string_new(envP, uuid.c_str()));
	};
	int ec = mysqlx_session_->query_databaseuuid(sn, cb);

	xmlrpc_struct_set_value(envP, dataP, "ec", xmlrpc_int_new(envP, ec));
	return dataP;
}

// 查询机台，线体更新信息
xmlrpc_value*
proc_check_recipe_update(
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

	// 防止数据库里不存在该机台，线体的数据，那么会导致无stat字段，导致客户端解析出错
	xmlrpc_struct_set_value(envP, dataP, "stat", xmlrpc_int_new(envP, 0));
	auto cb = [&envP, &dataP](const std::string& areaName, const std::string& machineName, const std::string& boardStyle, const std::string& machineStyleName, const std::string& halfMaterialNumber, int stat)
	{
		xmlrpc_set_int_v(dataP, stat);

		char name[200] = { 0 };
		if (stat) {
			if (boardStyle.size()) {
				sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s-", boardStyle.c_str());
			}
			if (machineStyleName.size()) {
				sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s-", machineStyleName.c_str());
			}
			if (halfMaterialNumber.size()) {
				sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s", halfMaterialNumber.c_str());
			}
		}

		xmlrpc_struct_set_value(envP, dataP, "name", xmlrpc_string_new(envP, name));
	};
	char machine_str[200] = { 0 };
	memcpy_s(machine_str, sizeof(machine_str) / sizeof(machine_str[0]) - 1, machine, machine_len);
	int ec = mysqlx_session_->query_tcon_upgrade(type, area, machine_str, cb);

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_query_databaselist(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_array_new(envP);

	const char*	qstarttime = 0; const char* qendtime = 0; const char* qsn = 0; const char* qtestplan = 0;
	const char* qfailprjname = 0; const char* qfailsubprjname = 0; const char* qtestip = 0; const char* qpcname = 0; const char* qtestresult = 0;
	const char* qareaname = 0; const char* qdeviceno = 0; const char* qtoolno = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "(ssssssssssss)", &qstarttime, &qendtime,&qsn, &qtestplan, &qfailprjname, &qfailsubprjname, 
		&qtestip, &qpcname, &qtestresult, &qareaname, &qdeviceno, &qtoolno);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid, const std::string& sn, const std::string& mac, const std::string& softwareversion, const std::string& keyname,
		const std::string& testresult, const std::string& testplan, const std::string& modulebom, const std::string& areaname, const std::string& deviceno, const std::string& toolno,
		const std::string& failprjname, const std::string& failsubprjname, const std::string& testip, const std::string& pcname, const std::string& testtime, const time_t testdate,
		const std::string& testperson)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_str_v(v, sn);
		xmlrpc_set_str_v(v, mac);
		xmlrpc_set_str_v(v, softwareversion);
		xmlrpc_set_str_v(v, keyname);
		xmlrpc_set_str_v(v, testresult);
		xmlrpc_set_str_v(v, testplan);
		xmlrpc_set_str_v(v, modulebom);
		xmlrpc_set_str_v(v, areaname);
		xmlrpc_set_str_v(v, deviceno);
		xmlrpc_set_str_v(v, toolno);
		xmlrpc_set_str_v(v, failprjname);
		xmlrpc_set_str_v(v, failsubprjname);
		xmlrpc_set_str_v(v, testip);
		xmlrpc_set_str_v(v, pcname);
		xmlrpc_set_str_v(v, testtime);
		//xmlrpc_struct_set_value(envP, v, "testdate", xmlrpc_string_new(envP, testdate.c_str()));
		xmlrpc_set_str_v(v, testperson);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_databaselist(qstarttime, qendtime, qsn, qtestplan, qfailprjname, qfailsubprjname,
		qtestip, qpcname, qtestresult, qareaname, qdeviceno, qtoolno, cb);

	return xmlrpc_build_value(envP, "{s:i,s:A}", "ec", ec, "data", valueP);
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

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	auto cb = [&envP, &dataP](const std::string& uuid, const std::string& project, const std::string& board, const std::string& bom, time_t create_ts, time_t edit_ts, const std::string& desc)
	{
		testplan_array_item_init(dataP);
	};
	int ec = mysqlx_session_->query_testplan2(project, board, bom, cb);
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

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* type;
	const char* area;
	const char* machine;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:6,*})", "testtype", &type, "area", &area, "machine", &machine, &machine_len);
	CheckIfFaultOccurredServer();

	auto cb = [&envP, &dataP](const std::string& areaName, const std::string& machineName, const std::string& boardStyle, const std::string& machineStyleName, const std::string& halfMaterialNumber, int stat)
	{
		xmlrpc_set_int_v(dataP, stat);
		xmlrpc_set_str_v(dataP, boardStyle);
		xmlrpc_set_str_v(dataP, machineStyleName);
		xmlrpc_set_str_v(dataP, halfMaterialNumber);
	};
	char machine_str[200] = { 0 };
	memcpy_s(machine_str, sizeof(machine_str) / sizeof(machine_str[0]) - 1, machine, machine_len);
	int ec = mysqlx_session_->query_tcon_upgrade(type, area, machine_str, cb);

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

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* project;
	const char* board;
	const char* bom;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	unsigned size = 0;
	auto cb = [&envP, &valueP, &size](const std::string& md5, unsigned offset, unsigned trunk_size, unsigned file_size)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, md5);
		xmlrpc_set_int_v(v, offset);
		xmlrpc_set_int_v(v, trunk_size);

		size = file_size;

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_trunk(project, board, bom, cb);

	xmlrpc_struct_set_value(envP, dataP, "trunk", valueP);
	xmlrpc_DECREF(valueP);

	xmlrpc_struct_set_value(envP, dataP, "file_size", xmlrpc_int_new(envP, size));

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

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* project;
	const char* board;
	const char* bom;

	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,*})", "project", &project, "board", &board, "bom", &bom);
	CheckIfFaultOccurredServer();

	unsigned size = 0;
	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP, &size](const std::string& md5, unsigned offset, unsigned trunk_size, unsigned file_size)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, md5);
		xmlrpc_set_int_v(v, offset);
		xmlrpc_set_int_v(v, trunk_size);

		size = file_size;

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_testplan_trunk(project, board, bom, cb);

 	xmlrpc_struct_set_value(envP, dataP, "trunk", valueP);
 	xmlrpc_DECREF(valueP);

	xmlrpc_struct_set_value(envP, dataP, "file_size", xmlrpc_int_new(envP, size));

	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_query_test_config_data(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_test_config_data is called");
	const char* boardtype;
	const char* halfmaterialnum;
	bool displayhistoryeditflag;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:b,*})", 
		"boardtype", &boardtype,
		"halfmaterialnum", &halfmaterialnum,
		"displayhistoryeditflag",&displayhistoryeditflag);
	if (displayhistoryeditflag)
	{
		L_INFO("boardtype is: {0},halfmaterialnum is: {1}", boardtype, halfmaterialnum);
	}
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](int uuid, 
		const std::string& boardtype, 
		const std::string& halfmaterialnum, 
		const std::string& areaname,
		const std::string& machinename, 
		const std::string& usrname, 
		const std::string& edittime)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_int_v(v, uuid);
		xmlrpc_set_str_v(v, boardtype);
		xmlrpc_set_str_v(v, halfmaterialnum);
		xmlrpc_set_str_v(v, areaname);
		xmlrpc_set_str_v(v, machinename);
		xmlrpc_set_str_v(v, usrname);
		xmlrpc_set_str_v(v, edittime);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_test_config_data(boardtype, halfmaterialnum, displayhistoryeditflag,cb);
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
	//return xmlrpc_build_value(envP, "{s:V}", "data", valueP);
	return build_ret_value(ec, dataP);
}

xmlrpc_value*
proc_get_test_details(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_get_test_details is called");
	const char* uuid;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,*})",
		"uuid", &uuid);
	CheckIfFaultOccurredServer();
	//xmlrpc_struct_set_value(envP, dataP, "name", xmlrpc_string_new(envP, name));
	auto cb = [&envP, &dataP](const std::string& testdetails)
	{
		xmlrpc_set_str_v(dataP, testdetails);
	};
	int ec = mysqlx_session_->get_test_details(uuid,cb);
	/*xmlrpc_struct_set_value(envP, dataP, "TestDetails",dataP);
	xmlrpc_DECREF(dataP);*/
	return build_ret_value(ec, dataP);
}
xmlrpc_value*
proc_query_template(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_template is called");
	const char* project;
	int project_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,*})",
		"project", &project, &project_len);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](
		const std::string& templatetype)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_str_v(v, templatetype);
		xmlrpc_array_append_item(envP, valueP, v);
	};
	char project_str[200] = { 0 };
	memcpy_s(project_str, sizeof(project_str) / sizeof(project_str[0]) - 1, project, project_len);
	int ec = mysqlx_session_->query_template(project_str, cb);
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
	//return xmlrpc_build_value(envP, "{s:V}", "data", valueP);
	return build_ret_value(ec, dataP);

}

xmlrpc_value*
proc_query_test_config_data_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_test_config_data_tv is called");
	const char* project1;
	const char* templatetype1;
	const char* shortbom1;
	const char* materialno1;
	const char* areaname1;
	const char* machinename1;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:6,s:s,s:s,s:s,s:s,*})",
		"areaname", &areaname1,
		"machinename", &machinename1,&machine_len,
		"project", &project1,
		"templatetype", &templatetype1,
		"shortbom", &shortbom1,
		"materialno", &materialno1);

	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](const std::string& uuid,
		const std::string& recipename,
		const std::string& areaname,
		const std::string& machinename,
		const std::string& project,
		const std::string& templatetype,
		const std::string& shortbom,
		const std::string& modulebom,
		const std::string& materialno,
		const std::string& testdata,
		const std::string& usrname,
		const std::string& edittime)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_str_v(v, uuid);
		xmlrpc_set_str_v(v, recipename);
		xmlrpc_set_str_v(v, areaname);
		xmlrpc_set_str_v(v, machinename);
		xmlrpc_set_str_v(v, project);
		xmlrpc_set_str_v(v, templatetype);
		xmlrpc_set_str_v(v, shortbom);
		xmlrpc_set_str_v(v, modulebom);
		xmlrpc_set_str_v(v, materialno);
		xmlrpc_set_str_v(v, testdata);
		xmlrpc_set_str_v(v, usrname);
		xmlrpc_set_str_v(v, edittime);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = 0;

	ec = mysqlx_session_->query_test_config_data_tv(areaname1, machinename1, project1, templatetype1, shortbom1, materialno1, cb);

	if (ec==0)
	{
		ec = mysqlx_session_->query_test_config_data_tv("", "", project1, templatetype1, shortbom1, materialno1, cb);
		
	}
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
	return build_ret_value(ec, dataP);
}

// 查询配置文件块信息
xmlrpc_value*
proc_query_recipe_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	unsigned size = 0;
	auto cb = [&envP, &valueP, &size](const std::string& md5, unsigned offset, unsigned trunk_size, unsigned file_size)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);

		xmlrpc_set_str_v(v, md5);
		xmlrpc_set_int_v(v, offset);
		xmlrpc_set_int_v(v, trunk_size);

		size = file_size;

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_tv_testplan(name, cb);

	xmlrpc_struct_set_value(envP, dataP, "trunk", valueP);
	xmlrpc_DECREF(valueP);

	xmlrpc_struct_set_value(envP, dataP, "file_size", xmlrpc_int_new(envP, size));

	return build_ret_value(ec, dataP);
}

// 查询(下载)配置文件块
xmlrpc_value*
proc_query_recipe_trunk_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* name;
	xmlrpc_decompose_value(envP, paramArrayP, "(s)", &name);
	CheckIfFaultOccurredServer();

	auto cb = [&envP, &dataP](const std::string& md5, const std::string content)
	{
		xmlrpc_set_str_v(dataP, md5);
		xmlrpc_set_str_v(dataP, content);
	};
	int ec = mysqlx_session_->query_tv_hstvt(name, cb);

	return build_ret_value(ec, dataP);
}
// 检查测试方案是否存在
xmlrpc_value*
proc_tv_testplan_exist(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	const char* templatetype;//板型
	const char* modulebom;//机型（BOM）
	const char* materialno;//物料号
	const char* areaname;//线体
	const char* machineno;//机台
	int machine_len;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:s,s:s,*})", "templatetype", &templatetype, "modulebom", &modulebom, "materialno", &materialno,"areaname",&areaname,"machinename",&machineno);
	CheckIfFaultOccurredServer();

	xmlrpc_int32 ec = mysqlx_session_->check_tv_testplan_exist(templatetype,modulebom,materialno,areaname,machineno);

	return build_ret_value(ec, dataP);
}

// 查询机台，线体更新信息
xmlrpc_value*
proc_check_recipe_update_tv(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{
	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* type;
	const char* area;
	const char* emachine;
	const char* machine;
	int machine_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,s:s,s:s,s:6,*})", "testtype", &type, "area", &area, "emachine",&emachine,"machine", &machine,&machine_len);
	CheckIfFaultOccurredServer();

	// 防止数据库里不存在该机台，线体的数据，那么会导致无stat字段，导致客户端解析出错
	xmlrpc_struct_set_value(envP, dataP, "stat", xmlrpc_int_new(envP, 0));
	auto cb = [&envP, &dataP](const std::string& areaName, const std::string& machineName, const std::string& TemplateType, const std::string& ModuleBom, const std::string& MaterialNo, int stat)
	{
		xmlrpc_set_int_v(dataP, stat);

		char name[200] = { 0 };
		if (stat) {
			if (TemplateType.size()) {
				sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s-", TemplateType.c_str());
			}
			if (ModuleBom.size()) {
				sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s-", ModuleBom.c_str());
			}
			if (MaterialNo.size()) {
				sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s-", MaterialNo.c_str());
			}
			if (areaName.size()) {
				sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s", areaName.c_str());
			}
			if (machineName.size()) {
				//sprintf_s(name + strlen(name), sizeof(name) / sizeof(name[0]) - strlen(name), "%s", machineName.c_str());
			}
		}

		xmlrpc_struct_set_value(envP, dataP, "name", xmlrpc_string_new(envP, name));
	};
	char machine_str[200] = { 0 };
	memcpy_s(machine_str, sizeof(machine_str) / sizeof(machine_str[0]) - 1, machine, machine_len);
	int ec = mysqlx_session_->query_tv_upgrade(type, area, emachine, cb);

	return build_ret_value(ec, dataP);
}
//查询测试方案名称
xmlrpc_value*
proc_check_filename(
	xmlrpc_env* const envP,
	xmlrpc_value* const paramArrayP,
	void* const serverInfo,
	void* const channelInfo)
{

	CleanServerEnv();

	xmlrpc_value* dataP = xmlrpc_struct_new(envP);

	const char* filename;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:s,*})", "filename", &filename);
	CheckIfFaultOccurredServer();
	auto cb = [&envP, &dataP](const std::string& newfilename)
	{
		xmlrpc_struct_set_value(envP, dataP, "newfilename", xmlrpc_string_new(envP, newfilename.c_str()));

	};
	xmlrpc_int32 ec = mysqlx_session_->check_filename(filename,cb);

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
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_cordata is called");
	const char* boardtype;
	int boardtype_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,*})",
		"boardtype", &boardtype, &boardtype_len);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](
		const std::string& programno,
		const std::string& remarks,
		const std::string& editor,
		const std::string& edittime)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_str_v(v, programno);
		xmlrpc_set_str_v(v, remarks);
		xmlrpc_set_str_v(v, editor);
		xmlrpc_set_str_v(v, edittime);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	char project_str[200] = { 0 };
	memcpy_s(project_str, sizeof(project_str) / sizeof(project_str[0]) - 1, boardtype, boardtype_len);
	int ec = mysqlx_session_->query_cordata(project_str, cb);
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
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
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_distinctacType is called");
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](
		const std::string& acBoardType)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_str_v(v, acBoardType);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_distinctacType( cb);
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
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
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_Remark is called");
	const char* programno;
	const char* boardtype;
	int boardtype_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,s:s,*})",
		"boardtype", &boardtype, &boardtype_len,
		"programno",&programno);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](
		const std::string& remarks,
		const std::string& uuidstr)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_str_v(v, remarks);
		xmlrpc_set_str_v(v, uuidstr);


		xmlrpc_array_append_item(envP, valueP, v);
	};
	char project_str[200] = { 0 };
	memcpy_s(project_str, sizeof(project_str) / sizeof(project_str[0]) - 1, boardtype, boardtype_len);
	int ec = mysqlx_session_->query_Remark(project_str, programno, cb);
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
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
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_distinctpcbtype is called");
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](
		const std::string& BoardType)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_str_v(v, BoardType);

		xmlrpc_array_append_item(envP, valueP, v);
	};
	int ec = mysqlx_session_->query_distinctpcbtype(cb);
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
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
	xmlrpc_value* dataP = xmlrpc_struct_new(envP);
	L_INFO("proc_query_moduleNo is called");
	const char* boardtype;
	int boardtype_len = 0;
	xmlrpc_decompose_value(envP, paramArrayP, "({s:6,*})",
		"boardtype", &boardtype, &boardtype_len);
	CheckIfFaultOccurredServer();

	xmlrpc_value* valueP = xmlrpc_array_new(envP);
	auto cb = [&envP, &valueP](
		const std::string& moduleNo)
	{
		xmlrpc_value* v = xmlrpc_struct_new(envP);
		xmlrpc_set_str_v(v, moduleNo);


		xmlrpc_array_append_item(envP, valueP, v);
	};
	char project_str[200] = { 0 };
	memcpy_s(project_str, sizeof(project_str) / sizeof(project_str[0]) - 1, boardtype, boardtype_len);
	int ec = mysqlx_session_->query_moduleNo(project_str,  cb);
	xmlrpc_struct_set_value(envP, dataP, "TestData", valueP);
	xmlrpc_DECREF(valueP);
	return build_ret_value(ec, dataP);
}