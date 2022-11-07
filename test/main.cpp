
//#include <windows.h>

#include "xmlrpc-c/base.h"
#include "xmlrpc-c/client.h"
#include "xmlrpc-c/client_global.h"
#include "xmlrpc-c/util.h"
#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <string>
#include <thread>
#include <algorithm>

//#include "spdlogwapper.hpp"

#include "mysqlxwapper.h"

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <iostream>
#include "json.h"
#include <afx.h>
#include <winsock2.h>
#include "md5.h"
#include <future>
#include <functional>
#include <thread>
#include <chrono>

#define NAME "Xmlrpc-c Test Client"
#define VERSION "1.0"
#define SERVER_URL	"http://192.168.0.200:8880/RPC2"
static char s_rpc_server_url_[256] = { 0 };
static xmlrpc_env s_env_;
static char session_[100] = { 0 };

#define REPICE_TRUNK_SIZE		(64 * 1024)
#define env_clean() \
if (s_env_.fault_occurred) {\
	xmlrpc_env_clean(&s_env_); \
	memset(&s_env_, 0, sizeof(s_env_)); \
}
//static char s_rpc_server_url_[256] = { 0 };
//static xmlrpc_env s_env_;

struct hstvt_trunk {
	std::string md5_, content_;
	unsigned offset_, size_;
	int stat_;
};
using recipe_trunk_md5 = std::vector<hstvt_trunk>;

static void GetExePath(char* szExePath)
{
	GetModuleFileName(NULL, szExePath, MAX_PATH);
	char* p = strrchr(szExePath, '\\');
	if (!p) {
		p = strrchr(szExePath, '/');
	}

	if (p) {
		p[1] = '\0';
	}
}

static void
dieIfFaultOccurred(xmlrpc_env * const envP) {
	if (envP->fault_occurred) {
		//L_ERROR("Something failed. {0} (XML-RPC fault code {1})",
			//envP->fault_string, envP->fault_code);
		exit(1);
	}
}

static void test_login(xmlrpc_env* env, const char* usr, const char* pass);
static void test_addusr(xmlrpc_env* env, const char* usr, const char* pass, const char* auth, const char* nick);
static void test_block_usr(xmlrpc_env* env, const char* usr);
static void test_reset_pass(xmlrpc_env* envP, const char* usr);
static void test_query_testplan(xmlrpc_env* envP, const char* uuid); // by uuid
static void test_query_testplan_by_name(xmlrpc_env* envP, const char* name);
static void test_query_testplan_by_short_name(xmlrpc_env* envP, const char* short_name);
static void test_query_cmd(xmlrpc_env* envP, const char* uuid);
static void test_addtestdatabase(xmlrpc_env* env, const char* sn, const char* mac, const char* softwareversion, const char* key, const char* testplanname, const char* testresult,
	const char* modulebom, const char* areaname, const char* deviceno, const char* toolno, const char* failprojectname, const char* failsubprojectname,
	const char* ip, const char* pcname, const char* testtime, const char* testperson);
static void test_addtestdatabasedetail(xmlrpc_env* env, const char* uuid, const char* sn, const char* id, const char* testprojectname,
	const char* subtestprojectname, const char* testresult, const char* testval,
	const char* testlowlimit, const char* testhighlimit, const char* ismain,
	const char* testtime);
static void test_query_databaseuuid(xmlrpc_env* envP, const char* sn);
static void test_updatetestbase(xmlrpc_env* envP, const char* uuid, const char* failprojectname, const char* failsubprojectname);
static void import_from_csv(const char* csv_name);
static void test_query_databaselist(xmlrpc_env* envP, const char* starttime, const char* endtime, const char* sn, const char* testplan,
	const char* failprjname, const char* failsubprjname, const char* testip, const char* pcname, const char* testresult,
	const char* areaname, const char* deviceno, const char* toolno);
static void test_checkupdate(xmlrpc_env* envP, const char* areaname, const char* devicenop);
static void test_check_recipe_trunk(xmlrpc_env* envP);
static void test_updatestatus(xmlrpc_env* envP, const char* areaname, const char* devicenop);
int xmlrpc_test_checkupdatestatus(const char* areaname, const char* deviceno, const char* testtype, int &status, const char* &filename);
static int get_recipe_utf8_name(const wchar_t* file_name, char utf8_name[]);
static int query_hstvt_trunk(const char* utf8_name, xmlrpc_mem_block* b, xmlrpc_mem_block* buff, recipe_trunk_md5& trunk_md5);
static int upload_hstvt_trunk(const char* utf8_name, const recipe_trunk_md5& trunk_md5);
static int query_hstvt_trunk_tv(const char* utf8_name, xmlrpc_mem_block* b, xmlrpc_mem_block* buff, recipe_trunk_md5& trunk_md5);
static int upload_hstvt_trunk_tv(const char* utf8_name, const recipe_trunk_md5& trunk_md5);

#define RPCSERVER_DEFAULT_URL	"http://127.0.0.1:8880/RPC2"
#define RPCCLIENT_NAME			"TCON"
#define RPCCLIENT_VERSION		"1.0"
void xmlrpc_startup(const char* rpc_url)
{
	strcpy_s(s_rpc_server_url_, rpc_url && *rpc_url ? rpc_url : RPCSERVER_DEFAULT_URL);
	/* Initialize our error-handling environment. */
	xmlrpc_env_init(&s_env_);

	/* Create the global XML-RPC client object. */
	xmlrpc_client_init2(&s_env_, XMLRPC_CLIENT_NO_FLAGS, RPCCLIENT_NAME, RPCCLIENT_VERSION, NULL, 0);
}

static void test_login(xmlrpc_env* envP, const char* usr, const char* pass)
{
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.login",
		"(ss)", usr, pass);

	dieIfFaultOccurred(envP);
	int ec;
	const char* session;
	const char* nick;
	int nick_len;
	int stat;
	const char* auth_name;
	int auth_name_len;
	int auth_value;
	xmlrpc_decompose_value(envP, resultP,
		"{s:i,s:s,s:6,s:i,s:6,s:i,*}",
		"ec", &ec,
		"uuid", &session,
		"nick", &nick, &nick_len,
		"stat", &stat,
		"auth_name", &auth_name, &auth_name_len,
		"auth_value", &auth_value);
	/* Get our sum and print it out. */
	dieIfFaultOccurred(envP);

	strcpy_s(session_, sizeof(session_) / sizeof(session_[0]), session);
	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_addusr(xmlrpc_env* envP, const char* usr, const char* pass, const char* auth, const char* nick)
{
	wchar_t wcs_nick[256] = { 0 };

	MultiByteToWideChar(CP_ACP, 0, nick, strlen(nick), wcs_nick, sizeof(wcs_nick) / sizeof(wcs_nick[0]));

	xmlrpc_mem_block* b = xmlrpc_wcs_to_utf8(envP, wcs_nick, wcsnlen_s(wcs_nick, sizeof(wcs_nick) / sizeof(wcs_nick[0])));

	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.addusr",
		"({s:s,s:s,s:s,s:6,s:s})",
		"usr", usr,
		"pass", pass,
		"auth", auth,
		"nick", xmlrpc_mem_block_contents(b), xmlrpc_mem_block_size(b),
		"creator", session_);

	dieIfFaultOccurred(envP);

	xmlrpc_mem_block_free(b);

	int ec;
	xmlrpc_read_int(envP, resultP, &ec);
	dieIfFaultOccurred(envP);

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_block_usr(xmlrpc_env* envP, const char* usr)
{
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.blockusr", "(ss)", usr, session_);

	dieIfFaultOccurred(envP);
	int ec;
	xmlrpc_read_int(envP, resultP, &ec);
	dieIfFaultOccurred(envP);

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_reset_pass(xmlrpc_env* envP, const char* usr)
{
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.resetpass", "(ss)", usr, session_);

	dieIfFaultOccurred(envP);
	int ec;
	xmlrpc_read_int(envP, resultP, &ec);
	dieIfFaultOccurred(envP);

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_query_testplan(xmlrpc_env* envP, const char* uuid)
{
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.testplan", "(s)", uuid);
	dieIfFaultOccurred(envP);

	xmlrpc_value* valueP = resultP;

	xmlrpc_value* ec_value;
	int ec = 0;
	xmlrpc_struct_read_value(envP, valueP, "ec", &ec_value);
	xmlrpc_read_int(envP, ec_value, &ec);
	if (ec == 1) {
		xmlrpc_value* uuid_value;
		const char* uuid_str;
		xmlrpc_struct_read_value(envP, valueP, "uuid", &uuid_value);
		xmlrpc_read_string(envP, uuid_value, &uuid_str);

		xmlrpc_value* project;
		int project_value;
		xmlrpc_struct_read_value(envP, valueP, "project", &project);
		xmlrpc_read_int(envP, project, &project_value);

		xmlrpc_value* board;
		const char* board_str;
		xmlrpc_struct_read_value(envP, valueP, "board", &board);
		xmlrpc_read_string(envP, board, &board_str);

		xmlrpc_value* bom;
		const char* bom_str;
		xmlrpc_struct_read_value(envP, valueP, "bom", &bom);
		xmlrpc_read_string(envP, bom, &bom_str);

		xmlrpc_value* name;
		const unsigned char* name_b64;
		size_t b64_len;
		char name_str[256];
		xmlrpc_struct_read_value(envP, valueP, "name", &name);
		xmlrpc_read_base64(envP, name, &b64_len, &name_b64);
		memcpy_s(name_str, sizeof(name_str) / sizeof(name_str[0]), name_b64, b64_len);

		xmlrpc_value* creator;
		char creator_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, valueP, "creator", &creator);
		xmlrpc_read_base64(envP, name, &b64_len, &name_b64);
		memcpy_s(creator_str, sizeof(creator_str) / sizeof(creator_str[0]), name_b64, b64_len);

		xmlrpc_value* editor;
		char editor_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, valueP, "editor", &editor);
		xmlrpc_read_base64(envP, name, &b64_len, &name_b64);
		memcpy_s(editor_str, sizeof(editor_str) / sizeof(editor_str[0]), name_b64, b64_len);

		xmlrpc_value* create_ts;
		time_t create_timestmp;
		xmlrpc_struct_read_value(envP, valueP, "create_ts", &create_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &create_timestmp);

		xmlrpc_value* edit_ts;
		time_t edit_timestmp;
		xmlrpc_struct_read_value(envP, valueP, "edit_ts", &edit_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &edit_timestmp);
	}
	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_query_testplan_by_name(xmlrpc_env* envP, const char* name)
{
	wchar_t wcs_name[256] = { 0 };

	MultiByteToWideChar(CP_ACP, 0, name, strlen(name), wcs_name, sizeof(wcs_name) / sizeof(wcs_name[0]));

	xmlrpc_mem_block* b = xmlrpc_wcs_to_utf8(envP, wcs_name, wcsnlen_s(wcs_name, sizeof(wcs_name) / sizeof(wcs_name[0])));
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.testplan_by_name", "(6)", xmlrpc_mem_block_contents(b), xmlrpc_mem_block_size(b));
	dieIfFaultOccurred(envP);

	xmlrpc_mem_block_free(b);

	int ec = 0;
	xmlrpc_value* valueP = 0;
	xmlrpc_decompose_value(envP, resultP, "{s:i,s:A,*}", "ec", &ec, "data", &valueP);
	dieIfFaultOccurred(envP);

	int c = xmlrpc_array_size(envP, valueP);
	for (int i = 0; i < c; i++) {
		xmlrpc_value* v;
		xmlrpc_array_read_item(envP, valueP, i, &v);

		xmlrpc_value* uuid;
		const char* uuid_str;
		xmlrpc_struct_read_value(envP, v, "uuid", &uuid);
		xmlrpc_read_string(envP, uuid, &uuid_str);

		xmlrpc_value* project;
		int project_value;
		xmlrpc_struct_read_value(envP, v, "project", &project);
		xmlrpc_read_int(envP, project, &project_value);

		xmlrpc_value* board;
		const char* board_str;
		xmlrpc_struct_read_value(envP, v, "board", &board);
		xmlrpc_read_string(envP, board, &board_str);

		xmlrpc_value* bom;
		const char* bom_str;
		xmlrpc_struct_read_value(envP, v, "bom", &bom);
		xmlrpc_read_string(envP, bom, &bom_str);

		xmlrpc_value* name;
		const unsigned char* name_b64;
		size_t b64_len;
		char name_str[256];
		xmlrpc_struct_read_value(envP, v, "name", &name);
		xmlrpc_read_base64(envP, name, &b64_len, &name_b64);
		memcpy_s(name_str, sizeof(name_str) / sizeof(name_str[0]), name_b64, b64_len);

		xmlrpc_value* creator;
		char creator_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "creator", &creator);
		xmlrpc_read_base64(envP, creator, &b64_len, &name_b64);
		memcpy_s(creator_str, sizeof(creator_str) / sizeof(creator_str[0]), name_b64, b64_len);

		xmlrpc_value* editor;
		char editor_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "editor", &editor);
		xmlrpc_read_base64(envP, editor, &b64_len, &name_b64);
		memcpy_s(editor_str, sizeof(editor_str) / sizeof(editor_str[0]), name_b64, b64_len);

		xmlrpc_value* create_ts;
		time_t create_timestmp;
		xmlrpc_struct_read_value(envP, v, "create_ts", &create_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &create_timestmp);

		xmlrpc_value* edit_ts;
		time_t edit_timestmp;
		xmlrpc_struct_read_value(envP, v, "edit_ts", &edit_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &edit_timestmp);
	}
	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_query_testplan_by_short_name(xmlrpc_env* envP, const char* short_name)
{
	wchar_t wcs_short_name[256] = { 0 };

	MultiByteToWideChar(CP_ACP, 0, short_name, strlen(short_name), wcs_short_name, sizeof(wcs_short_name) / sizeof(wcs_short_name[0]));

	xmlrpc_mem_block* b = xmlrpc_wcs_to_utf8(envP, wcs_short_name, wcsnlen_s(wcs_short_name, sizeof(wcs_short_name) / sizeof(wcs_short_name[0])));
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.testplan_by_short_name", "(6)", xmlrpc_mem_block_contents(b), xmlrpc_mem_block_size(b));
	dieIfFaultOccurred(envP);

	xmlrpc_mem_block_free(b);

	int ec = 0;
	xmlrpc_value* valueP = 0;
	xmlrpc_decompose_value(envP, resultP, "{s:i,s:A,*}", "ec", &ec, "data", &valueP);
	dieIfFaultOccurred(envP);

	int c = xmlrpc_array_size(envP, valueP);
	for (int i = 0; i < c; i++) {
		xmlrpc_value* v;
		xmlrpc_array_read_item(envP, valueP, i, &v);

		xmlrpc_value* uuid;
		const char* uuid_str;
		xmlrpc_struct_read_value(envP, v, "uuid", &uuid);
		xmlrpc_read_string(envP, uuid, &uuid_str);

		xmlrpc_value* project;
		int project_value;
		xmlrpc_struct_read_value(envP, v, "project", &project);
		xmlrpc_read_int(envP, project, &project_value);

		xmlrpc_value* board;
		const char* board_str;
		xmlrpc_struct_read_value(envP, v, "board", &board);
		xmlrpc_read_string(envP, board, &board_str);

		xmlrpc_value* bom;
		const char* bom_str;
		xmlrpc_struct_read_value(envP, v, "bom", &bom);
		xmlrpc_read_string(envP, bom, &bom_str);

		xmlrpc_value* name;
		const unsigned char* name_b64;
		size_t b64_len;
		char name_str[256];
		xmlrpc_struct_read_value(envP, v, "name", &name);
		xmlrpc_read_base64(envP, name, &b64_len, &name_b64);
		memcpy_s(name_str, sizeof(name_str) / sizeof(name_str[0]), name_b64, b64_len);

		xmlrpc_value* creator;
		char creator_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "creator", &creator);
		xmlrpc_read_base64(envP, creator, &b64_len, &name_b64);
		memcpy_s(creator_str, sizeof(creator_str) / sizeof(creator_str[0]), name_b64, b64_len);

		xmlrpc_value* editor;
		char editor_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "editor", &editor);
		xmlrpc_read_base64(envP, editor, &b64_len, &name_b64);
		memcpy_s(editor_str, sizeof(editor_str) / sizeof(editor_str[0]), name_b64, b64_len);

		xmlrpc_value* create_ts;
		time_t create_timestmp;
		xmlrpc_struct_read_value(envP, v, "create_ts", &create_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &create_timestmp);

		xmlrpc_value* edit_ts;
		time_t edit_timestmp;
		xmlrpc_struct_read_value(envP, v, "edit_ts", &edit_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &edit_timestmp);
	}
	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_query_cmd(xmlrpc_env* envP, const char* uuid)
{
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.testplan_cmd",
		"(s)", uuid);

	dieIfFaultOccurred(envP);

	int ec = 0;
	xmlrpc_value* valueP = 0;
	xmlrpc_decompose_value(envP, resultP, "{s:i,s:A,*}", "ec", &ec, "data", &valueP);
	dieIfFaultOccurred(envP);

	/*
	xmlrpc_struct_set_value(envP, v, "uuid", xmlrpc_string_new(envP, uuid.c_str()));
	xmlrpc_struct_set_value(envP, v, "name", xmlrpc_base64_new(envP, name.length(), (unsigned char*)name.c_str()));
	xmlrpc_struct_set_value(envP, v, "type", xmlrpc_base64_new(envP, type.length(), (unsigned char*)type.c_str()));
	xmlrpc_struct_set_value(envP, v, "delay", xmlrpc_int_new(envP, delay));
	xmlrpc_struct_set_value(envP, v, "timeout", xmlrpc_int_new(envP, timeout));
	xmlrpc_struct_set_value(envP, v, "order", xmlrpc_int_new(envP, order));
	xmlrpc_struct_set_value(envP, v, "stat", xmlrpc_int_new(envP, stat));

	xmlrpc_struct_set_value(envP, v, "creator", xmlrpc_string_new(envP, creator.c_str()));
	xmlrpc_struct_set_value(envP, v, "editor", xmlrpc_string_new(envP, editor.c_str()));
	*/
	int c = xmlrpc_array_size(envP, valueP);
	for (int i = 0; i < c; i++) {
		xmlrpc_value* v;
		xmlrpc_array_read_item(envP, valueP, i, &v);

		xmlrpc_value* uuid;
		const char* uuid_str;
		xmlrpc_struct_read_value(envP, v, "uuid", &uuid);
		xmlrpc_read_string(envP, uuid, &uuid_str);

		xmlrpc_value* name;
		const unsigned char* b64_str;
		size_t b64_len;
		char name_str[256];
		xmlrpc_struct_read_value(envP, v, "name", &name);
		xmlrpc_read_base64(envP, name, &b64_len, &b64_str);
		memcpy_s(name_str, sizeof(name_str) / sizeof(name_str[0]), b64_str, b64_len);

		xmlrpc_value* type;
		char type_str[256];
		xmlrpc_struct_read_value(envP, v, "type", &type);
		xmlrpc_read_base64(envP, name, &b64_len, &b64_str);
		memcpy_s(type_str, sizeof(type_str) / sizeof(type_str[0]), b64_str, b64_len);

		xmlrpc_value* creator;
		char creator_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "creator", &creator);
		xmlrpc_read_base64(envP, creator, &b64_len, &b64_str);
		memcpy_s(creator_str, sizeof(creator_str) / sizeof(creator_str[0]), b64_str, b64_len);

		xmlrpc_value* editor;
		char editor_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "editor", &editor);
		xmlrpc_read_base64(envP, editor, &b64_len, &b64_str);
		memcpy_s(editor_str, sizeof(editor_str) / sizeof(editor_str[0]), b64_str, b64_len);

		xmlrpc_value* create_ts;
		time_t create_timestmp;
		xmlrpc_struct_read_value(envP, v, "create_ts", &create_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &create_timestmp);

		xmlrpc_value* edit_ts;
		time_t edit_timestmp;
		xmlrpc_struct_read_value(envP, v, "edit_ts", &edit_ts);
		xmlrpc_read_datetime_sec(envP, create_ts, &edit_timestmp);
	}

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void import_from_csv(const char* csv_name)
{
}

static void test_addtestdatabase(xmlrpc_env* envP, const char* sn, const char* mac, const char* softwareversion, const char* key,
	const char* testresult, const char* testplanname, const char* modulebom, const char* areaname, const char* deviceno, const char* toolno,
	const char* failprojectname, const char* failsubprojectname, const char* ip,
	const char* pcname, const char* testtime, const char* testperson)
{
	wchar_t wcs_testplanname[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, testplanname, strlen(testplanname), wcs_testplanname, sizeof(wcs_testplanname) / sizeof(wcs_testplanname[0]));
	xmlrpc_mem_block* ptestplanname = xmlrpc_wcs_to_utf8(envP, wcs_testplanname, wcsnlen_s(wcs_testplanname, sizeof(wcs_testplanname) / sizeof(wcs_testplanname[0])));

	wchar_t wcs_toolno[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, toolno, strlen(toolno), wcs_toolno, sizeof(wcs_toolno) / sizeof(wcs_toolno[0]));
	xmlrpc_mem_block* ptoolno = xmlrpc_wcs_to_utf8(envP, wcs_toolno, wcsnlen_s(wcs_toolno, sizeof(wcs_toolno) / sizeof(wcs_toolno[0])));

	wchar_t wcs_failprojectname[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, failprojectname, strlen(failprojectname), wcs_failprojectname, sizeof(wcs_failprojectname) / sizeof(wcs_failprojectname[0]));
	xmlrpc_mem_block* pfailprojectname = xmlrpc_wcs_to_utf8(envP, wcs_failprojectname, wcsnlen_s(wcs_failprojectname, sizeof(wcs_failprojectname) / sizeof(wcs_failprojectname[0])));

	wchar_t wcs_failsubprojectname[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, failsubprojectname, strlen(failsubprojectname), wcs_failsubprojectname, sizeof(wcs_failsubprojectname) / sizeof(wcs_failsubprojectname[0]));
	xmlrpc_mem_block* pfailsubprojectname = xmlrpc_wcs_to_utf8(envP, wcs_failsubprojectname, wcsnlen_s(wcs_failsubprojectname, sizeof(wcs_failsubprojectname) / sizeof(wcs_failsubprojectname[0])));


	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.addtestdatabase",
		"({s:s,s:s,s:s,s:s,s:s,s:6,s:s,s:s,s:s,s:6,s:6,s:6,s:s,s:s,s:s,s:s})",
		"sn", sn,
		"mac", mac,
		"softwareversion", softwareversion,
		"key", key,
		"testresult", testresult,
		"testplanname", xmlrpc_mem_block_contents(ptestplanname), xmlrpc_mem_block_size(ptestplanname),
		"modulebom", modulebom,
		"areaname", areaname,
		"deviceno", deviceno,
		"toolno", xmlrpc_mem_block_contents(ptoolno), xmlrpc_mem_block_size(ptoolno),
		"failprojectname", xmlrpc_mem_block_contents(pfailprojectname), xmlrpc_mem_block_size(pfailprojectname),
		"failsubprojectname", xmlrpc_mem_block_contents(pfailsubprojectname), xmlrpc_mem_block_size(pfailsubprojectname),
		"testip", ip,
		"pcname", pcname,
		"testtime", testtime,
		"testperson", testperson);

	dieIfFaultOccurred(envP);

	xmlrpc_mem_block_free(ptestplanname);
	xmlrpc_mem_block_free(ptoolno);
	xmlrpc_mem_block_free(pfailprojectname);
	xmlrpc_mem_block_free(pfailsubprojectname);
	int ec;
	xmlrpc_read_int(envP, resultP, &ec);
	dieIfFaultOccurred(envP);

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_addtestdatabasedetail(xmlrpc_env* envP, const char* uuid, const char* sn, const char* id, const char* testprojectname,
	const char* subtestprojectname, const char* testresult, const char* testval,
	const char* testlowlimit, const char* testhighlimit, const char* ismain,
	const char* testtime)
{

	wchar_t wcs_testprojectname[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, testprojectname, strlen(testprojectname), wcs_testprojectname, sizeof(wcs_testprojectname) / sizeof(wcs_testprojectname[0]));
	xmlrpc_mem_block* ptestprojectname = xmlrpc_wcs_to_utf8(envP, wcs_testprojectname, wcsnlen_s(wcs_testprojectname, sizeof(wcs_testprojectname) / sizeof(wcs_testprojectname[0])));

	wchar_t wcs_subtestprojectname[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, subtestprojectname, strlen(subtestprojectname), wcs_subtestprojectname, sizeof(wcs_subtestprojectname) / sizeof(wcs_subtestprojectname[0]));
	xmlrpc_mem_block* psubtestprojectname = xmlrpc_wcs_to_utf8(envP, wcs_subtestprojectname, wcsnlen_s(wcs_subtestprojectname, sizeof(wcs_subtestprojectname) / sizeof(wcs_subtestprojectname[0])));


	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.addtestdatabasedetail",
		"({s:s,s:s,s:s,s:6,s:6,s:s,s:s,s:s,s:s,s:s,s:s})",
		"uuid", uuid,
		"sn", sn,
		"id", id,
		"testprojectname", xmlrpc_mem_block_contents(ptestprojectname), xmlrpc_mem_block_size(ptestprojectname),
		"subtestprojectname", xmlrpc_mem_block_contents(psubtestprojectname), xmlrpc_mem_block_size(psubtestprojectname),
		"testresult", testresult,
		"testval", testval,
		"testlowlimit", testlowlimit,
		"testhightlimit", testhighlimit,
		"ismain", ismain,
		"testtime", testtime);

	dieIfFaultOccurred(envP);

	xmlrpc_mem_block_free(ptestprojectname);
	xmlrpc_mem_block_free(psubtestprojectname);

	int ec;
	xmlrpc_read_int(envP, resultP, &ec);
	dieIfFaultOccurred(envP);

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);

}

static void test_query_databaseuuid(xmlrpc_env* envP, const char* sn)
{
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.proc_query_databaseuuid",
		"(s)", sn);

	dieIfFaultOccurred(envP);

	int ec = 0;
	const char* uuid;
	xmlrpc_decompose_value(envP, resultP, "{s:i,s:s,*}", "ec", &ec, "uuid", &uuid);
	dieIfFaultOccurred(envP);


	//int c = xmlrpc_array_size(envP, valueP);
	//for (int i = 0; i < c; i++) {
	//	xmlrpc_value* v;
	//	xmlrpc_array_read_item(envP, valueP, i, &v);

	//	xmlrpc_value* uuid;
	//	const char* uuid_str;
	//	xmlrpc_struct_read_value(envP, v, "uuid", &uuid);
	//	xmlrpc_read_string(envP, uuid, &uuid_str);


	//}

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_updatetestbase(xmlrpc_env* envP, const char* uuid, const char* failprojectname, const char* failsubprojectname)
{
	wchar_t wcs_failprojectname[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, failprojectname, strlen(failprojectname), wcs_failprojectname, sizeof(wcs_failprojectname) / sizeof(wcs_failprojectname[0]));
	xmlrpc_mem_block* pfailprojectname = xmlrpc_wcs_to_utf8(envP, wcs_failprojectname, wcsnlen_s(wcs_failprojectname, sizeof(wcs_failprojectname) / sizeof(wcs_failprojectname[0])));
	char failprojectname_str[200] = { 0 };
	memcpy_s(failprojectname_str, 200, xmlrpc_mem_block_contents(pfailprojectname), xmlrpc_mem_block_size(pfailprojectname));

	wchar_t wcs_failsubprojectname[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, failsubprojectname, strlen(failsubprojectname), wcs_failsubprojectname, sizeof(wcs_failsubprojectname) / sizeof(wcs_failsubprojectname[0]));
	xmlrpc_mem_block* pfailsubprojectname = xmlrpc_wcs_to_utf8(envP, wcs_failsubprojectname, wcsnlen_s(wcs_failsubprojectname, sizeof(wcs_failsubprojectname) / sizeof(wcs_failsubprojectname[0])));
	char failsubprojectname_str[200] = { 0 };
	memcpy_s(failsubprojectname_str, 200, xmlrpc_mem_block_contents(pfailsubprojectname), xmlrpc_mem_block_size(pfailsubprojectname));

	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.update_databasebyuuid",
		"({s:s,s:6,s:6})",
		"uuid", uuid,
		"failprjname", xmlrpc_mem_block_contents(pfailprojectname), xmlrpc_mem_block_size(pfailprojectname),
		"failsubprjname", xmlrpc_mem_block_contents(pfailsubprojectname), xmlrpc_mem_block_size(pfailsubprojectname));

	dieIfFaultOccurred(envP);


	xmlrpc_mem_block_free(pfailprojectname);
	xmlrpc_mem_block_free(pfailsubprojectname);
	int ec;
	xmlrpc_read_int(envP, resultP, &ec);
	dieIfFaultOccurred(envP);

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_query_databaselist(xmlrpc_env* envP, const char* starttime, const char* endtime, const char* sn, const char* testplan,
	const char* failprjname, const char* failsubprjname, const char* testip, const char* pcname, const char* testresult,
	const char* areaname, const char* deviceno, const char* toolno)
{
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.query_databaselist",
		"(ssssssssssss)", starttime, endtime, sn, testplan, failprjname, failsubprjname,
		testip, pcname, testresult, areaname, deviceno, toolno);

	dieIfFaultOccurred(envP);

	int ec = 0;
	xmlrpc_value* valueP = 0;
	xmlrpc_decompose_value(envP, resultP, "{s:i,s:A,*}", "ec", &ec, "data", &valueP);
	dieIfFaultOccurred(envP);
	int c = xmlrpc_array_size(envP, valueP);
	for (int i = 0; i < c; i++)
	{
		xmlrpc_value* v;
		xmlrpc_array_read_item(envP, valueP, i, &v);

		xmlrpc_value* uuid;
		const char* uuid_str;
		xmlrpc_struct_read_value(envP, v, "uuid", &uuid);
		xmlrpc_read_string(envP, uuid, &uuid_str);

		xmlrpc_value* sn;
		const char* sn_str;
		xmlrpc_struct_read_value(envP, v, "sn", &sn);
		xmlrpc_read_string(envP, sn, &sn_str);

		xmlrpc_value* mac;
		const char* mac_str;
		xmlrpc_struct_read_value(envP, v, "mac", &mac);
		xmlrpc_read_string(envP, mac, &mac_str);

		xmlrpc_value* softwareversion;
		const char* softwareversion_str;
		xmlrpc_struct_read_value(envP, v, "softwareversion", &softwareversion);
		xmlrpc_read_string(envP, softwareversion, &softwareversion_str);

		xmlrpc_value* keyname;
		const char* keyname_str;
		xmlrpc_struct_read_value(envP, v, "keyname", &keyname);
		xmlrpc_read_string(envP, keyname, &keyname_str);

		xmlrpc_value* testresult;
		const char* testresult_str;
		xmlrpc_struct_read_value(envP, v, "testresult", &testresult);
		xmlrpc_read_string(envP, testresult, &testresult_str);

		xmlrpc_value* testplan;
		const unsigned char* b64_str;
		size_t b64_len;
		char testplan_str[256];
		xmlrpc_struct_read_value(envP, v, "testplan", &testplan);
		xmlrpc_read_base64(envP, testplan, &b64_len, &b64_str);
		memcpy_s(testplan_str, sizeof(testplan_str) / sizeof(testplan_str[0]), b64_str, b64_len);

		xmlrpc_value* modulebom;
		const char* modulebom_str;
		xmlrpc_struct_read_value(envP, v, "modulebom", &modulebom);
		xmlrpc_read_string(envP, modulebom, &modulebom_str);

		xmlrpc_value* areaname;
		const char* areaname_str;
		xmlrpc_struct_read_value(envP, v, "areaname", &areaname);
		xmlrpc_read_string(envP, areaname, &areaname_str);

		xmlrpc_value* deviceno;
		const char* deviceno_str;
		xmlrpc_struct_read_value(envP, v, "deviceno", &deviceno);
		xmlrpc_read_string(envP, deviceno, &deviceno_str);

		xmlrpc_value* toolno;
		char toolno_str[256];
		xmlrpc_struct_read_value(envP, v, "toolno", &toolno);
		xmlrpc_read_base64(envP, toolno, &b64_len, &b64_str);
		memcpy_s(toolno_str, sizeof(toolno_str) / sizeof(toolno_str[0]), b64_str, b64_len);

		xmlrpc_value* failprjname;
		char failprjname_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "failprjname", &failprjname);
		xmlrpc_read_base64(envP, failprjname, &b64_len, &b64_str);
		memcpy_s(failprjname_str, sizeof(failprjname_str) / sizeof(failprjname_str[0]), b64_str, b64_len);

		xmlrpc_value* failsubprjname;
		char failsubprjname_str[256] = { 0 };
		xmlrpc_struct_read_value(envP, v, "failsubprjname", &failsubprjname);
		xmlrpc_read_base64(envP, failsubprjname, &b64_len, &b64_str);
		memcpy_s(failsubprjname_str, sizeof(failsubprjname_str) / sizeof(failsubprjname_str[0]), b64_str, b64_len);

		xmlrpc_value* testip;
		const char* testip_str;
		xmlrpc_struct_read_value(envP, v, "testip", &testip);
		xmlrpc_read_string(envP, testip, &testip_str);

		xmlrpc_value* pcname;
		const char* pcname_str;
		xmlrpc_struct_read_value(envP, v, "pcname", &pcname);
		xmlrpc_read_string(envP, pcname, &pcname_str);

		xmlrpc_value* testtime;
		const char* testtime_str;
		xmlrpc_struct_read_value(envP, v, "testtime", &testtime);
		xmlrpc_read_string(envP, testtime, &testtime_str);

		xmlrpc_value* testdate;
		time_t testdate_timestmp;
		xmlrpc_struct_read_value(envP, v, "testdate", &testdate);
		xmlrpc_read_datetime_sec(envP, testdate, &testdate_timestmp);
	}

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}
static void test_checkupdate(xmlrpc_env* envP, const char* areaname, const char* devicenop)
{
	wchar_t wcs_deviceno[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, devicenop, strlen(devicenop), wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0]));
	xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(envP, wcs_deviceno, wcsnlen_s(wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0])));
	char deviceno_str[200] = { 0 };
	memcpy_s(deviceno_str, 200, xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));

	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.check_recipe_update",
		"({s:s,s:s,s:6})", "testtype", "TCON_TEST", "area", areaname,
		"machine", xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));

	dieIfFaultOccurred(envP);

	printf("SUCCESS");
}
static void test_updatestatus(xmlrpc_env* envP, const char* areaname, const char* devicenop)
{
	wchar_t wcs_deviceno[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, devicenop, strlen(devicenop), wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0]));
	xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(envP, wcs_deviceno, wcsnlen_s(wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0])));
	char deviceno_str[200] = { 0 };
	memcpy_s(deviceno_str, 200, xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.update_recipe_machine",
		"({s:s,s:s,s:6})",
		"testtype", "TCON_TEST",
		"area", areaname,
		"machine", xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));

	dieIfFaultOccurred(envP);

	int ec;
	xmlrpc_read_int(envP, resultP, &ec);
	dieIfFaultOccurred(envP);

	/* Dispose of our result value. */
	xmlrpc_DECREF(resultP);
}

static void test_check_recipe_trunk(xmlrpc_env* envP)
{
	xmlrpc_value* arrayP = xmlrpc_array_new(envP);
	static char* trunk_md5[] = {
		"44193484CA5260C21E33E591019EEC4F",
		"61AD6673E5EE9534D0FF0A5445726D42",
		"71AD272200CD10C02E8E65414A970EF7",
		"71AD272200CD10C02E8E65414A970E7",
	};
	for (int i = 0; i < sizeof(trunk_md5) / sizeof(trunk_md5[0]); i++) {
		xmlrpc_array_append_item(envP, arrayP, xmlrpc_string_new(envP, trunk_md5[i]));
	}
	xmlrpc_value* resultP = xmlrpc_client_call(envP, SERVER_URL, "proc.check_recipe_trunk",
		"(A)",
		arrayP);

	xmlrpc_int ec;
	xmlrpc_value* dataP;
	xmlrpc_decompose_value(envP, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
	xmlrpc_DECREF(resultP);

	xmlrpc_struct_read_value(envP, dataP, "md5", &arrayP);
	xmlrpc_DECREF(dataP);

	for (int i = 0; i < xmlrpc_array_size(envP, arrayP); i++) {
		xmlrpc_value* v = xmlrpc_array_get_item(envP, arrayP, i);
		const char* str;
		xmlrpc_read_string(envP, v, &str);
		printf("%s\n", str);
	}
}

//lilei add test

static int get_recipe_utf8_name(const wchar_t* file_name, char utf8_name[])
{
	const wchar_t* p = wcsrchr(file_name, L'\\') + 1;
	p = p ? p : wcsrchr(file_name, L'/') + 1;
	p = p ? p : file_name;
	wchar_t n[100] = { 0 };
	wcscpy_s(n, p);
	p = wcsrchr(n, L'.');
	if (p) *(wchar_t*)p = L'\0';

	xmlrpc_mem_block* utf8_block = xmlrpc_wcs_to_utf8(&s_env_, n, wcslen(n));
	strncpy_s(utf8_name, MAX_PATH, XMLRPC_MEMBLOCK_CONTENTS(char, utf8_block), XMLRPC_MEMBLOCK_SIZE(char, utf8_block));
	xmlrpc_mem_block_free(utf8_block);

	return strlen(utf8_name);
}

static int query_hstvt_trunk(const char* utf8_name, xmlrpc_mem_block* b, xmlrpc_mem_block* buff, recipe_trunk_md5& trunk_md5)
{
	env_clean();

	int b64_size = XMLRPC_MEMBLOCK_SIZE(char, b);
	int remain = b64_size;
	int offset = 0;
	int ec;
	hstvt_trunk trunk;

	for (;;)
	{
		if (!remain) {
			break;
		}
		int cur_size = remain;
		if (cur_size > REPICE_TRUNK_SIZE) cur_size = REPICE_TRUNK_SIZE;

		char* str = XMLRPC_MEMBLOCK_CONTENTS(char, buff);
		memset(str, 0, XMLRPC_MEMBLOCK_SIZE(char, buff));
		memcpy_s(str, cur_size, XMLRPC_MEMBLOCK_CONTENTS(char, b) + offset, cur_size);
		str[cur_size + 1] = '\0';

		MD5_CTX md5;
		unsigned char md5_ctx[16] = { 0 };
		MD5Init(&md5);
		MD5Update(&md5, (unsigned char*)str, cur_size);
		MD5Final(&md5, md5_ctx);
		char md5_str[100] = { 0 };
		for (int i = 0; i < 16; i++) {
			sprintf_s(md5_str + strlen(md5_str), sizeof(md5_str) - strlen(md5_str), "%02X", md5_ctx[i]);
		}

		trunk.md5_ = md5_str;
		trunk.content_ = str;
		trunk.offset_ = offset;
		trunk.size_ = cur_size;
		trunk.stat_ = 1;

		trunk_md5.push_back(trunk);

		offset += cur_size;
		remain -= cur_size;
	}

	xmlrpc_value* arrayP = xmlrpc_array_new(&s_env_);
	for (const auto& trunk : trunk_md5) {
		xmlrpc_value* v = xmlrpc_string_new(&s_env_, trunk.md5_.c_str());
		xmlrpc_array_append_item(&s_env_, arrayP, v);

		xmlrpc_DECREF(v);
	}

	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
		"proc.check_recipe_trunk",
		"(A)",
		arrayP);

	xmlrpc_DECREF(arrayP);
	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	xmlrpc_value* dataP;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	xmlrpc_value* valueP;
	xmlrpc_struct_read_value(&s_env_, dataP, "md5", &valueP);
	xmlrpc_DECREF(dataP);
	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	for (int i = 0; i < xmlrpc_array_size(&s_env_, valueP); i++) {
		xmlrpc_value* v = xmlrpc_array_get_item(&s_env_, valueP, i);
		const char* md5_str;
		xmlrpc_read_string(&s_env_, v, &md5_str);
		if (s_env_.fault_occurred) {
			break;
		}

		auto it = std::find_if(trunk_md5.begin(), trunk_md5.end(), [md5_str](const hstvt_trunk& trunk)
		{
			return trunk.md5_ == md5_str;
		});

		if (it != trunk_md5.end()) {
			it->stat_ = 0;
		}
	}

	return s_env_.fault_code;
}

static int query_hstvt_trunk_tv(const char* utf8_name, xmlrpc_mem_block* b, xmlrpc_mem_block* buff, recipe_trunk_md5& trunk_md5)
{
	env_clean();

	int b64_size = XMLRPC_MEMBLOCK_SIZE(char, b);
	int remain = b64_size;
	int offset = 0;
	int ec;
	hstvt_trunk trunk;

	for (;;)
	{
		if (!remain) {
			break;
		}
		int cur_size = remain;
		if (cur_size > REPICE_TRUNK_SIZE) cur_size = REPICE_TRUNK_SIZE;

		char* str = XMLRPC_MEMBLOCK_CONTENTS(char, buff);
		memset(str, 0, XMLRPC_MEMBLOCK_SIZE(char, buff));
		memcpy_s(str, cur_size, XMLRPC_MEMBLOCK_CONTENTS(char, b) + offset, cur_size);
		str[cur_size + 1] = '\0';

		MD5_CTX md5;
		unsigned char md5_ctx[16] = { 0 };
		MD5Init(&md5);
		MD5Update(&md5, (unsigned char*)str, cur_size);
		MD5Final(&md5, md5_ctx);
		char md5_str[100] = { 0 };
		for (int i = 0; i < 16; i++) {
			sprintf_s(md5_str + strlen(md5_str), sizeof(md5_str) - strlen(md5_str), "%02X", md5_ctx[i]);
		}

		trunk.md5_ = md5_str;
		trunk.content_ = str;
		trunk.offset_ = offset;
		trunk.size_ = cur_size;
		trunk.stat_ = 1;

		trunk_md5.push_back(trunk);

		offset += cur_size;
		remain -= cur_size;
	}

	xmlrpc_value* arrayP = xmlrpc_array_new(&s_env_);
	for (const auto& trunk : trunk_md5) {
		xmlrpc_value* v = xmlrpc_string_new(&s_env_, trunk.md5_.c_str());
		xmlrpc_array_append_item(&s_env_, arrayP, v);

		xmlrpc_DECREF(v);
	}

	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
		"proc.check_recipe_trunk_TV",
		"(A)",
		arrayP);

	xmlrpc_DECREF(arrayP);
	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	xmlrpc_value* dataP;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	xmlrpc_value* valueP;
	xmlrpc_struct_read_value(&s_env_, dataP, "md5", &valueP);
	xmlrpc_DECREF(dataP);
	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	for (int i = 0; i < xmlrpc_array_size(&s_env_, valueP); i++) {
		xmlrpc_value* v = xmlrpc_array_get_item(&s_env_, valueP, i);
		const char* md5_str;
		xmlrpc_read_string(&s_env_, v, &md5_str);
		if (s_env_.fault_occurred) {
			break;
		}

		auto it = std::find_if(trunk_md5.begin(), trunk_md5.end(), [md5_str](const hstvt_trunk& trunk)
		{
			return trunk.md5_ == md5_str;
		});

		if (it != trunk_md5.end()) {
			it->stat_ = 0;
		}
	}

	return s_env_.fault_code;
}

static int upload_hstvt_trunk(const char* utf8_name, const recipe_trunk_md5& trunk_md5)
{
	env_clean();

	for (const auto& trunk : trunk_md5) {
		xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
			"proc.save_recipe",
			"({s:s,s:s,s:s,s:i,s:i})",
			"name", utf8_name,
			"md5", trunk.md5_.c_str(),
			"data", trunk.stat_ == 1 ? trunk.content_.c_str() : "",
			"offset", trunk.offset_,
			"size", trunk.size_);

		if (s_env_.fault_occurred) {
			break;
		}

		xmlrpc_value* ecP;
		xmlrpc_struct_read_value(&s_env_, resultP, "ec", &ecP);
		xmlrpc_DECREF(resultP);
		if (s_env_.fault_occurred) {
			break;
		}

		int ec;
		xmlrpc_read_int(&s_env_, ecP, &ec);
		xmlrpc_DECREF(ecP);
		if (s_env_.fault_occurred) {
			break;
		}

		if (ec < 0) {
			return ec;
		}
	}

	return s_env_.fault_code;
}

static int upload_hstvt_trunk_tv(const char* utf8_name, const recipe_trunk_md5& trunk_md5)
{
	env_clean();
	const char* line = "T201";
	const char* deviceno = "Station1";

	wchar_t wcs_deviceno[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, deviceno, strlen(deviceno), wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0]));
	xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(&s_env_, wcs_deviceno, wcsnlen_s(wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0])));
	if (s_env_.fault_occurred) {
		return -1;
	}

	int count = 0;
	for (const auto& trunk : trunk_md5) {
		xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
			"proc.save_recipe_TV",
			//"({s:s,s:s,s:s,s:i,s:i})",
			"({s:s,s:s,s:6,s:s,s:s,s:i,s:i})",
			"name", utf8_name,
			"line",line,
			"deviceno",xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno),
			"md5", trunk.md5_.c_str(),
			"data", trunk.stat_ == 1 ? trunk.content_.c_str() : "",
			"offset", trunk.offset_,
			"size", trunk.size_);

		if (s_env_.fault_occurred) {
			break;
		}

		xmlrpc_value* ecP;
		xmlrpc_struct_read_value(&s_env_, resultP, "ec", &ecP);
		xmlrpc_DECREF(resultP);
		if (s_env_.fault_occurred) {
			break;
		}

		int ec;
		xmlrpc_read_int(&s_env_, ecP, &ec);
		xmlrpc_DECREF(ecP);
		if (s_env_.fault_occurred) {
			break;
		}

		if (ec < 0) {
			return ec;
		}
		count++;
	}
	printf("the count value is:%d", count);
	return s_env_.fault_code;
}

int xmlrpc_test_checkupdatestatus(const char* areaname, const char* deviceno, const char* testtype, int &status, const char* &filename)
{
	wchar_t wcs_deviceno[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, deviceno, strlen(deviceno), wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0]));
	xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(&s_env_, wcs_deviceno, wcsnlen_s(wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0])));
	if (s_env_.fault_occurred) {
		return -1;
	}

	char deviceno_str[200] = { 0 };
	memcpy_s(deviceno_str, 200, xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));

	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_, "proc.check_recipe_update",
		"({s:s,s:6,s:s})", "area", areaname,
		"machine", xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno),
		"testtype", testtype);
	xmlrpc_mem_block_free(pdeviceno);

	if (s_env_.fault_occurred) {
		return -1;
	}

	int ec;
	xmlrpc_value* dataP;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		return -1;
	}

	if (ec >= 0) {
		xmlrpc_value* statP;
		xmlrpc_struct_read_value(&s_env_, dataP, "stat", &statP);
		if (!s_env_.fault_occurred) {
			xmlrpc_read_int(&s_env_, statP, &status);
			xmlrpc_DECREF(statP);
			if (status)
			{
				xmlrpc_decompose_value(&s_env_, dataP, "{s:s,*}", "name", &filename);
			}
		}
	}

	xmlrpc_DECREF(dataP);

	return ec;

}

static int upload_hstvt_size(const char* utf8_name, int size)
{
	env_clean();

	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
		"proc.recipe_size",
		"({s:s,s:i})",
		"name", utf8_name,
		"size", size);

	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	int ec;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,*}", "ec", &ec);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	return ec;
}

static int upload_hstvt_size_tv(const char* utf8_name, int size)
{
	env_clean();

	const char* line = "T201";
	const char* deviceno = "Station1";

	wchar_t wcs_deviceno[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, deviceno, strlen(deviceno), wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0]));
	xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(&s_env_, wcs_deviceno, wcsnlen_s(wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0])));
	if (s_env_.fault_occurred) {
		return -1;
	}

	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
		"proc.recipe_size_TV",
		"({s:s,s:i,s:s,s:6})",
		"name", utf8_name,
		"size", size,
		"line", line,
		"deviceno", xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));

	if (s_env_.fault_occurred) {
		printf("%d", s_env_.fault_code);
		return s_env_.fault_code;
	}

	int ec;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,*}", "ec", &ec);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		printf("%d",s_env_.fault_code);
		return s_env_.fault_code;
	}

	return ec;
}

int xmlrpc_save_recipe(const wchar_t* file_name)
{
	HANDLE hFile = CreateFileW(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return -1;
	}

	int size = (int)GetFileSize(hFile, 0);
	HANDLE hFileMapping = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, size, 0);
	if (!hFileMapping) {
		CloseHandle(hFile);
		return -2;
	}
	env_clean();
	unsigned char* data = (unsigned char*)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, size);
	xmlrpc_mem_block* buff = xmlrpc_mem_block_new(&s_env_, size);
	unsigned long buff_size = XMLRPC_MEMBLOCK_SIZE(unsigned char, buff);
	if (compress2(XMLRPC_MEMBLOCK_CONTENTS(unsigned char, buff), &buff_size, data, size, 6) != Z_OK) {
		UnmapViewOfFile(data);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return -1;
	}

	xmlrpc_mem_block* b = xmlrpc_base64_encode_without_newlines(&s_env_, XMLRPC_MEMBLOCK_CONTENTS(unsigned char, buff), buff_size);

	char utf8_name[MAX_PATH] = { 0 };
	get_recipe_utf8_name(file_name, utf8_name);

	recipe_trunk_md5 trunk_md5;
	query_hstvt_trunk(utf8_name, b, buff, trunk_md5);
	upload_hstvt_size(utf8_name, size);
	if (trunk_md5.size()) {
		upload_hstvt_trunk(utf8_name, trunk_md5);
	}

	xmlrpc_mem_block_free(b);
	xmlrpc_mem_block_free(buff);

	UnmapViewOfFile(data);

	CloseHandle(hFileMapping);
	CloseHandle(hFile);

	return s_env_.fault_code;
}

int xmlrpc_save_recipe_tv(const wchar_t* file_name)
{
	HANDLE hFile = CreateFileW(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return -1;
	}

	int size = (int)GetFileSize(hFile, 0);
	HANDLE hFileMapping = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, size, 0);
	if (!hFileMapping) {
		CloseHandle(hFile);
		return -2;
	}
	env_clean();
	unsigned char* data = (unsigned char*)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, size);
	xmlrpc_mem_block* buff = xmlrpc_mem_block_new(&s_env_, size);
	unsigned long buff_size = XMLRPC_MEMBLOCK_SIZE(unsigned char, buff);
	if (compress2(XMLRPC_MEMBLOCK_CONTENTS(unsigned char, buff), &buff_size, data, size, 6) != Z_OK) {
		UnmapViewOfFile(data);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return -1;
	}

	xmlrpc_mem_block* b = xmlrpc_base64_encode_without_newlines(&s_env_, XMLRPC_MEMBLOCK_CONTENTS(unsigned char, buff), buff_size);

	char utf8_name[MAX_PATH] = { 0 };
	get_recipe_utf8_name(file_name, utf8_name);

	recipe_trunk_md5 trunk_md5;
	query_hstvt_trunk_tv(utf8_name, b, buff, trunk_md5);
	upload_hstvt_size_tv(utf8_name, size);
	if (trunk_md5.size()) {
		upload_hstvt_trunk_tv(utf8_name, trunk_md5);
	}

	xmlrpc_mem_block_free(b);
	xmlrpc_mem_block_free(buff);

	UnmapViewOfFile(data);

	CloseHandle(hFileMapping);
	CloseHandle(hFile);

	return s_env_.fault_code;
}

void test_chinese(std::string name)
{
	/*int len = strlen(name.c_str()) + 1;
	char outch[MAX_PATH];
	WCHAR * wChar = new WCHAR[len];
	wChar[0] = 0;
	MultiByteToWideChar(CP_UTF8, 0, name.c_str(), len, wChar, len);
	WideCharToMultiByte(CP_ACP, 0, wChar, len, outch, len, 0, 0);
	delete[] wChar;
	char* pchar = (char*)outch;

	len = strlen(pchar) + 1;
	WCHAR outName[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, pchar, len, outName, len);*/


	int len = MultiByteToWideChar(CP_ACP, 0, name.c_str(), -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, name.c_str(), -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return;
}

std::string JsonToString(const Json::Value & root)
{
	static Json::Value def = []() {
		Json::Value def;
		Json::StreamWriterBuilder::setDefaults(&def);
		def["emitUTF8"] = true;
		return def;
	}();

	std::ostringstream stream;
	Json::StreamWriterBuilder stream_builder;
	stream_builder.settings_ = def;//Config emitUTF8
	std::unique_ptr<Json::StreamWriter> writer(stream_builder.newStreamWriter());
	writer->write(root, &stream);
	return stream.str();
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

static int xmlrpc_single_change_production(const char* boardtype,/*板型*/const char* halfmaterialnum,/*半品料号*/
	const char* areaname/*线体名称*/, const char* machinename/*机台名称*/)
{
	env_clean();
	const char* testtype = "TCON_TEST";
	xmlrpc_value* array = xmlrpc_array_new(&s_env_);

	for (int i = 0; i < 1; i++) {
		const char* areaName = areaname;
		wchar_t wcs_deviceno[256] = { 0 };
		MultiByteToWideChar(CP_ACP, 0, machinename, strlen(machinename), wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0]));
		xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(&s_env_, wcs_deviceno, wcsnlen_s(wcs_deviceno, sizeof(wcs_deviceno) / sizeof(wcs_deviceno[0])));
		if (s_env_.fault_occurred) {
			return -1;
		}

		char deviceno_str[200] = { 0 };
		memcpy_s(deviceno_str, 200, xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));

		const char* machineName = deviceno_str;
		const char* boardStyle = boardtype;
		const char* machineStyleName = "";
		const char* halfMaterialNumber = halfmaterialnum;

		xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_, "proc.tcon_testplan_exist", "({s:s,s:s,s:s})", "boardStyle", boardStyle, "machineStyleName", machineStyleName, "halfMaterialNumber", halfMaterialNumber);
		if (s_env_.fault_occurred) {
			return -1;
		}
		int ec;
		xmlrpc_decompose_value(&s_env_, resultP, "{s:i,*}", "ec", &ec);
		xmlrpc_DECREF(resultP);

		if (s_env_.fault_occurred || ec <= 0) {
			return -1;
		}

		xmlrpc_value* d = xmlrpc_struct_new(&s_env_);
		xmlrpc_struct_set_value(&s_env_, d, "areaName", xmlrpc_string_new(&s_env_, areaName));
		xmlrpc_struct_set_value(&s_env_, d, "machineName", xmlrpc_base64_new(&s_env_, strlen(machineName), (unsigned char*)machineName));
		xmlrpc_struct_set_value(&s_env_, d, "boardStyle", xmlrpc_string_new(&s_env_, boardStyle));
		xmlrpc_struct_set_value(&s_env_, d, "machineStyleName", xmlrpc_string_new(&s_env_, machineStyleName));
		xmlrpc_struct_set_value(&s_env_, d, "halfMaterialNumber", xmlrpc_string_new(&s_env_, halfMaterialNumber));
		xmlrpc_struct_set_value(&s_env_, d, "stat", xmlrpc_int_new(&s_env_, 1));

		xmlrpc_array_append_item(&s_env_, array, d);
	}

	if (s_env_.fault_occurred) {
		return -1;
	}

	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_, "proc.update_recipe", "({s:s,s:i,s:A})", "testtype", testtype, "size", xmlrpc_array_size(&s_env_, array), "data", array);
	xmlrpc_DECREF(array);
	if (s_env_.fault_occurred) {
		return -1;
	}
	return 1;
}

static int xmlrpc_testjson(const char* jsonobj)
{
	env_clean();
	size_t len = strlen(jsonobj) + 1;
	size_t converted = 0;
	wchar_t *wcs_deviceno;
	wcs_deviceno = (wchar_t*)malloc(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, jsonobj, strlen(jsonobj), wcs_deviceno, len * sizeof(wchar_t));
	xmlrpc_mem_block* pdeviceno = xmlrpc_wcs_to_utf8(&s_env_, wcs_deviceno, wcsnlen_s(wcs_deviceno, strlen(jsonobj)));
	if (s_env_.fault_occurred) {
		return -1;
	}
	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_, "proc.test_json",
		"({s:6})",
		"testdata", xmlrpc_mem_block_contents(pdeviceno), xmlrpc_mem_block_size(pdeviceno));
	if (s_env_.fault_occurred) {
		return -1;
	}
	return 1;
}

struct TestProjectList
{
	//板型
	CString TempleType;
	//半品料号
	CString SemiProductNo;
	//线体
	CString AreaName;
	//机台
	CString MachineNo;
	//上传者
	CString UserName;
	//上传时间
	CString UpdateTime;
	//uuid
	CString UUID;
};

static int query_test_config_data(const char* boardtype, const char* halfmaterialnum, bool displayhistoryeditflag, std::vector<TestProjectList*>& testprojectlst)
{
	env_clean();
	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_, "proc.query_test_config_data",
		"({s:s,s:s,s:b})",
		"boardtype", boardtype,
		"halfmaterialnum", halfmaterialnum,
		"displayhistoryeditflag", displayhistoryeditflag);
	if (s_env_.fault_occurred) {
		return -1;
	}

	int ec;
	int file_size;
	xmlrpc_value* dataP;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		return -1;
	}

	xmlrpc_value* valueP;
	xmlrpc_decompose_value(&s_env_, dataP, "{s:A,*}", "TestData", &valueP);
	xmlrpc_DECREF(dataP);

	if (s_env_.fault_occurred) {
		return -1;
	}

	int c = xmlrpc_array_size(&s_env_, valueP);
	for (int i = 0; i < c; i++) {
		TestProjectList *tmpS = new TestProjectList;
		xmlrpc_value* v;
		xmlrpc_array_read_item(&s_env_, valueP, i, &v);

		xmlrpc_value* uuid_value;
		int uuid;
		xmlrpc_struct_read_value(&s_env_, v, "uuid", &uuid_value);
		xmlrpc_read_int(&s_env_, uuid_value, &uuid);
		xmlrpc_DECREF(uuid_value);
		std::string struuid = std::to_string(uuid);
		tmpS->UUID = struuid.c_str();

		xmlrpc_value* boardtype_value;
		const char* boardtype;
		xmlrpc_struct_read_value(&s_env_, v, "boardtype", &boardtype_value);
		xmlrpc_read_string(&s_env_, boardtype_value, &boardtype);
		xmlrpc_DECREF(boardtype_value);
		tmpS->TempleType = boardtype;

		xmlrpc_value* halfmaterialnum_value;
		const char* halfmaterialnum;
		xmlrpc_struct_read_value(&s_env_, v, "halfmaterialnum", &halfmaterialnum_value);
		xmlrpc_read_string(&s_env_, halfmaterialnum_value, &halfmaterialnum);
		xmlrpc_DECREF(halfmaterialnum_value);
		tmpS->SemiProductNo = halfmaterialnum;

		xmlrpc_value* areaname_value;
		const char* areaname;
		xmlrpc_struct_read_value(&s_env_, v, "areaname", &areaname_value);
		xmlrpc_read_string(&s_env_, areaname_value, &areaname);
		xmlrpc_DECREF(areaname_value);
		tmpS->AreaName = areaname;

		xmlrpc_value* machinename_value;
		const char* machinename;
		xmlrpc_struct_read_value(&s_env_, v, "machinename", &machinename_value);
		xmlrpc_read_string(&s_env_, machinename_value, &machinename);
		xmlrpc_DECREF(machinename_value);
		tmpS->MachineNo = UTF8ToGBK(machinename).c_str();

		xmlrpc_value* usrname_value;
		const char* usrname;
		xmlrpc_struct_read_value(&s_env_, v, "usrname", &usrname_value);
		xmlrpc_read_string(&s_env_, usrname_value, &usrname);
		xmlrpc_DECREF(usrname_value);
		tmpS->UserName = UTF8ToGBK(usrname).c_str();

		xmlrpc_value* edittime_value;
		const char* edittime;
		xmlrpc_struct_read_value(&s_env_, v, "edittime", &edittime_value);
		xmlrpc_read_string(&s_env_, edittime_value, &edittime);
		xmlrpc_DECREF(edittime_value);
		tmpS->UpdateTime = edittime;
		testprojectlst.push_back(tmpS);
	}
	return 1;
}

void test_config_data(CString boardtype, CString halfmaterialnum, bool editflag, std::vector<TestProjectList*>& testprojectlst)
{
	query_test_config_data((char*)boardtype.GetBuffer(0), (char*)halfmaterialnum.GetBuffer(8), editflag, testprojectlst);
}

static int get_test_details(const char* uuid, std::string& details)
{
	env_clean();
	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_, "proc.get_test_details",
		"({s:s})",
		"uuid", uuid);
	if (s_env_.fault_occurred) {
		return -1;
	}

	int ec;
	int file_size;
	xmlrpc_value* dataP;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		return -1;
	}

	//xmlrpc_value* valueP;
	const char* test_details;
	xmlrpc_decompose_value(&s_env_, dataP, "{s:s,*}", "testdetails", &test_details);
	xmlrpc_DECREF(dataP);

	if (s_env_.fault_occurred) {
		return -1;
	}
	details = UTF8ToGBK(test_details);
	return 1;
}

static int query_recipe(const char* utf8_name, recipe_trunk_md5& trunk_md5)
{
	env_clean();

	xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
		"proc.query_recipe",
		"(s)", utf8_name);
	if (s_env_.fault_occurred) {
		return -1;
	}

	int ec;
	int file_size;
	xmlrpc_value* dataP;
	xmlrpc_decompose_value(&s_env_, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
	xmlrpc_DECREF(resultP);
	if (s_env_.fault_occurred) {
		return -1;
	}

	xmlrpc_value* valueP;
	xmlrpc_decompose_value(&s_env_, dataP, "{s:i,s:A,*}", "file_size", &file_size, "trunk", &valueP);
	xmlrpc_DECREF(dataP);

	if (s_env_.fault_occurred) {
		return s_env_.fault_code;
	}

	int c = xmlrpc_array_size(&s_env_, valueP);
	for (int i = 0; i < c; i++) {
		xmlrpc_value* v;
		xmlrpc_array_read_item(&s_env_, valueP, i, &v);

		xmlrpc_value* md5_value;
		const char* md5_str;
		xmlrpc_struct_read_value(&s_env_, v, "md5", &md5_value);
		xmlrpc_read_string(&s_env_, md5_value, &md5_str);
		xmlrpc_DECREF(md5_value);

		xmlrpc_value* offset_value;
		int offset;
		xmlrpc_struct_read_value(&s_env_, v, "offset", &offset_value);
		xmlrpc_read_int(&s_env_, offset_value, &offset);
		xmlrpc_DECREF(offset_value);

		xmlrpc_value* trunk_size_value;
		int trunk_size;
		xmlrpc_struct_read_value(&s_env_, v, "trunk_size", &trunk_size_value);
		xmlrpc_read_int(&s_env_, trunk_size_value, &trunk_size);
		xmlrpc_DECREF(trunk_size_value);

		xmlrpc_DECREF(v);

		hstvt_trunk trunk;
		trunk.md5_ = md5_str;
		trunk.offset_ = offset;
		trunk.size_ = trunk_size;
		trunk_md5.push_back(trunk);
	}

	xmlrpc_DECREF(valueP);

	return file_size;
}

static int download_recipe(const wchar_t* file_name, int file_size, const recipe_trunk_md5& trunk_md5)
{
	env_clean();

	HANDLE hFile = CreateFileW(file_name, GENERIC_ALL, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		return -1;
	}

	unsigned long MAX_HSTVT_SIZE = file_size;
	for (const auto& trunk : trunk_md5) {
		MAX_HSTVT_SIZE += trunk.size_;
	}

	unsigned long offset = 0;
	unsigned char* data = new unsigned char[MAX_HSTVT_SIZE];
	unsigned char* zip_data = data + file_size;

	for (const auto& trunk : trunk_md5) {
		xmlrpc_value* resultP = xmlrpc_client_call(&s_env_, s_rpc_server_url_,
			"proc.query_recipe_trunk",
			"(s)", trunk.md5_.c_str());
		if (s_env_.fault_occurred) {
			CloseHandle(hFile);
			return 0;
		}

		int ec;
		xmlrpc_value* dataP;
		xmlrpc_decompose_value(&s_env_, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP);
		xmlrpc_DECREF(resultP);
		if (s_env_.fault_occurred) {
			CloseHandle(hFile);
			return 0;
		}
		if (ec < 0) {
			xmlrpc_DECREF(dataP);
			return ec;
		}

		const char* md5_str;
		const char* content_str;
		xmlrpc_decompose_value(&s_env_, dataP, "{s:s,s:s,*}", "md5", &md5_str, "content", &content_str);
		xmlrpc_DECREF(dataP);
		if (s_env_.fault_occurred) {
			return s_env_.fault_code;
		}

		if (content_str) {
			xmlrpc_mem_block* block = xmlrpc_base64_decode(&s_env_, content_str, strlen(content_str));

			memcpy_s(zip_data + offset, MAX_HSTVT_SIZE - file_size - offset, XMLRPC_MEMBLOCK_CONTENTS(unsigned char, block), XMLRPC_MEMBLOCK_SIZE(unsigned char, block));

			offset += XMLRPC_MEMBLOCK_SIZE(unsigned char, block);

			xmlrpc_mem_block_free(block);
		}
	}

	unsigned long f_size;
	if (uncompress2(data, &f_size, zip_data, &offset) != Z_OK) {
		delete[] data;
		CloseHandle(hFile);
		return -3;
	}

	SetFilePointer(hFile, file_size, 0, FILE_END);
	HANDLE hFileMapping = CreateFileMapping(hFile, 0, PAGE_READWRITE, 0, file_size, 0);
	if (!hFileMapping) {
		delete[] data;
		CloseHandle(hFile);
		return -2;
	}

	unsigned char* file_data = (unsigned char*)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, file_size);
	if (!file_data) {
		delete[] data;
		CloseHandle(hFileMapping);
		CloseHandle(hFile);

		return -4;
	}

	memcpy_s(file_data, file_size, data, file_size);
	delete[] data;

	FlushViewOfFile(file_data, file_size);
	UnmapViewOfFile(file_data);

	CloseHandle(hFileMapping);

	CloseHandle(hFile);
	return file_size;
}

int xmlrpc_query_recipe(const wchar_t* file_name)
{
	char utf8_name[MAX_PATH] = { 0 };
	get_recipe_utf8_name(file_name, utf8_name);

	recipe_trunk_md5 trunk;
	int file_size = query_recipe(utf8_name, trunk);
	if (file_size <= 0) {
		return file_size;
	}

	std::sort(trunk.begin(), trunk.end(),
		[](const hstvt_trunk& l, const hstvt_trunk& r)
	{
		return l.offset_ < r.offset_;
	});
	return download_recipe(file_name, file_size, trunk);
}

int main(int argc, char* argv[])
{
	char szLog[MAX_PATH] = { 0 };
	GetExePath(szLog);
	strcat_s(szLog, "logs");

	_mkdir(szLog);
	strcat_s(szLog, "\\logx.log");
	//spdlog::spdlog_init("CLIENT", szLog, 0);

	xmlrpc_startup("");

	dieIfFaultOccurred(&s_env_);
	//int tmpStatus;
	//const char* aa = "atlantis9";
	//const char* bb = "456";
	//xmlrpc_test_checkupdatestatus("T104", "机台9","1",tmpStatus,bb);//---测试线体机台是否需要换产

	//std::string details;
	//get_test_details("2", details);
	//printf(details.c_str());

	//std::vector<TestProjectList*> tmpS;
	//query_test_config_data("", "", false, tmpS);
	//for (int i = 0; i < tmpS.size(); i++)
	//{
	//	printf(tmpS[i]->TempleType);
	//	printf("-");
	//	printf(tmpS[i]->SemiProductNo);
	//	printf("\n");
	//	printf(tmpS[i]->UpdateTime);
	//	printf("\n");
	//}


	//lilei test json
	//Json::Value root,arrayObj,item;
	//root["AreaName"] = "T100M";//线体名称
	//root["MachineNo"] = "机台9";//机台名称
	//root["PlateType"] = "12505";//板型
	//root["SemiProductNo"] = "879156";//设备型号
	//root["UserName"] = "李四";
	//item["Avalue1"] = "发发发发发发发发发";
	//arrayObj.append(item);
	//root["TestData"] = arrayObj;
	//const char * testJ = root.toStyledString().c_str();
	//std::string strJ = root.toStyledString();
	//xmlrpc_testjson(strJ.data());

	//system("pause");
	//lilei test json
	const char* fileobj = "F:\\12121-75E3F(DS)(BOM9L)-331124.HStvt";
	size_t len = strlen(fileobj);
	size_t converted = 0;
	wchar_t wcs_deviceno[256] = { 0 };
	MultiByteToWideChar(CP_ACP, 0, fileobj, strlen(fileobj), wcs_deviceno, len * sizeof(wchar_t));
	xmlrpc_save_recipe_tv(wcs_deviceno);

	//xmlrpc_single_change_production("11113", "121112", "T104", "机台4");

	/* Clean up our error-handling environment. */
	xmlrpc_env_clean(&s_env_);

	/* Shutdown our XML-RPC client library. */
	xmlrpc_client_cleanup();

	return 0;
}