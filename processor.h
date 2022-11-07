#pragma once

#include "xmlrpc-c/base.h"
#include "xmlrpc-c/server.h"
#include "xmlrpc-c/server_abyss.h"

#include "procact.h"

extern class mysqlx_session* mysqlx_session_;

#define PROC_ACT(act__) \
	xmlrpc_value* \
	proc_##act__(\
		xmlrpc_env* const envP, \
		xmlrpc_value* const paramArrayP, \
		void* const serverInfo, \
		void* const channelInfo);

PROC_ACTS()

bool check_auth(const char* uuid, const char* dst_auth);
bool check_auth2(const char* src_uuid, const char* dst_uuid);

bool startup_db_client(const char* host, unsigned short port);
void clean_db_client();

const char* get_db_server_url();

// 开始检查更新
void start_check_upgrade(const char* url, unsigned short port);
// 停止检查更新(停止服务的时候会被调用)
void stop_check_upgrade();
// 客户端检查更新会调用该函数，向检查更新线程的队列里添加一条数据(TCON_TEST->TCON_RESULT)
void check_update_machine(const char* type);
// 开始检查更新
void start_check_upgrade_tv(const char* url, unsigned short port);
// 停止检查更新(停止服务的时候会被调用)
void stop_check_upgrade_tv();
// 客户端检查更新会调用该函数，向检查更新线程的队列里添加一条数据(TCON_TEST->TCON_RESULT)
void check_update_machine_tv(const char* type);
// 通知中控更新结果
void notify_result(const char* type, const char* line, const char* machine, int result);
void notify_result_tv(const char* type, const char* line, const char* machine, int result);

#undef PROC_ACT
