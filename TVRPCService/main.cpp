
#include <winsock2.h>
#include <windows.h>

#include "xmlrpc-c/base.h"
#include "xmlrpc-c/server.h"
#include "xmlrpc-c/server_abyss.h"

#include "curl/curl.h"
#include <boost\program_options.hpp>
#include <boost\filesystem.hpp>

#include "spdlogwapper.hpp"

#include "processor.h"

#include <stdio.h>
#include <stdlib.h>
#include <direct.h>

#if _UNICODE
#define SERVICE_N L"TVRPCSERVICE"
#else
#define SERVICE_N "TVRPCSERVICE"
#endif // _UNICODE

static SERVICE_STATUS_HANDLE hServiceStatus;
static SERVICE_STATUS ServiceSt;

static xmlrpc_env env;
static xmlrpc_server_abyss_t* serverP = 0;

void WINAPI ServiceMain(DWORD dwNumServicesArgs, LPSTR* lpServiceArgVectors);
static void WINAPI ServiceCtrl(DWORD dwOpcode);
static void Run();
static void Stop();
static bool IsInstalled();
static bool Install();
static void UnInstall();
static void InitSpdLog(const char* logfile, int level);
static const char* GetExePath(char* szExePath);

int main(int argc, char* argv[])
{
	ServiceMain(argc, argv);
	return -1;
#ifdef IS_SERVICE_VER_
	SERVICE_TABLE_ENTRY te[] =
	{
		{ SERVICE_N, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	hServiceStatus = NULL;
	ServiceSt.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceSt.dwCurrentState = SERVICE_STOPPED;
	ServiceSt.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	ServiceSt.dwWin32ExitCode = 0;
	ServiceSt.dwServiceSpecificExitCode = 0;
	ServiceSt.dwCheckPoint = 0;
	ServiceSt.dwWaitHint = 0;

	Stop();

	if (!IsInstalled()) {
		Install();
	}

	if (!StartServiceCtrlDispatcher(te)) {
		return 0;
	}
#else
	ServiceMain(argc, argv);
#endif // IS_SERVICE_VER_
	return 0;
}

void WINAPI ServiceMain(DWORD dwNumServicesArgs, LPSTR* lpServiceArgVectors)
{
#ifdef IS_SERVICE_VER__
	ServiceSt.dwCurrentState = SERVICE_START_PENDING;

	hServiceStatus = RegisterServiceCtrlHandler(SERVICE_N, ServiceCtrl);
	if (!hServiceStatus) {
		return;
	}
	SetServiceStatus(hServiceStatus, &ServiceSt);
#endif // IS_SERVICE_VER_

	boost::program_options::options_description desc("options");
	desc.add_options()
		("port,p", boost::program_options::value<unsigned short>()->default_value(8880), "端口号")
		("logfile,f", boost::program_options::value<std::string>()->default_value("logs\\log.log"), "log文件")
		("loglevel,l", boost::program_options::value<int>()->default_value(2), "log等级")
		("dbaddr,H", boost::program_options::value<std::string>()->default_value("127.0.0.1"), "db服务器地址")
		("dbport,P", boost::program_options::value<unsigned short>()->default_value(8997), "db服务器端口号")
		("centerurl,u", boost::program_options::value<std::string>()->default_value("http://127.0.0.1"), "db服务器端口号")
		("centerport,c", boost::program_options::value<unsigned short>()->default_value(8080), "db服务器端口号")
		;

	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::command_line_parser(dwNumServicesArgs, lpServiceArgVectors).options(desc).run(), vm);

	InitSpdLog(vm["logfile"].as<std::string>().c_str(), vm["loglevel"].as<int>());

#ifdef IS_SERVICE_VER_
	ServiceSt.dwWin32ExitCode = S_OK;
	ServiceSt.dwCheckPoint = 0;
	ServiceSt.dwWaitHint = 0;
	ServiceSt.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hServiceStatus, &ServiceSt);
#endif // IS_SERVICE_VER_

	curl_global_init(CURL_GLOBAL_ALL);

	xmlrpc_server_abyss_parms parms = { 0 };
	xmlrpc_registry* registryP = 0;

	do {
		xmlrpc_env_init(&env);

		registryP = xmlrpc_registry_new(&env);
		if (env.fault_occurred) {
			L_ERROR("xmlrpc_registry_new() failed.  {0}", env.fault_string);
			break;
		}

#define PROC_ACT(act__) {"proc."###act__, &proc_##act__ },
		static xmlrpc_method_info3 methodInfo[] = {
			PROC_ACTS()
		};
#undef PROC_ACT

		for (int i = 0; i < sizeof(methodInfo) / sizeof(methodInfo[0]); i++) {
			xmlrpc_registry_add_method3(&env, registryP, &methodInfo[i]);
			if (env.fault_occurred) {
				L_WARN("xmlrpc_registry_add_method3() failed.  {0}", methodInfo[i].methodName);
				break;
			}
		}

		if (env.fault_occurred) {
			L_CRITICAL("xmlrpc_registry_add_method3() failed.  {0}", env.fault_string);
			break;
		}

		char szLogName[MAX_PATH] = { 0 };
		GetExePath(szLogName);
		strcat_s(szLogName, SERVICE_N);

		_mkdir(szLogName);
		strcat_s(szLogName, "\\xmlrpc_log.log");

		parms.config_file_name = NULL;   /* Select the modern normal API */
		parms.registryP = registryP;
		parms.port_number = vm["port"].as<unsigned short>();
		//parms.port_number = 8888;
		parms.log_file_name = szLogName;

		xmlrpc_server_abyss_global_init(&env);
		if (env.fault_occurred) {
			L_ERROR("xmlrpc_server_abyss_global_init() failed.  {0}", env.fault_string);
			break;
		}

		xmlrpc_server_abyss_create(&env, &parms, XMLRPC_APSIZE(log_file_name), &serverP);
		if (env.fault_occurred) {
			L_CRITICAL("xmlrpc_server_abyss_create() failed.  {0}", env.fault_string);
			break;
		}
		xmlrpc_server_abyss_sig* oldHandlersP;

		xmlrpc_server_abyss_setup_sig(&env, serverP, &oldHandlersP);

		if (!env.fault_occurred) {
			if (!startup_db_client(vm["dbaddr"].as<std::string>().c_str(), vm["dbport"].as<unsigned short>())) {
				break;
			}

			start_check_upgrade(vm["centerurl"].as<std::string>().c_str(), vm["centerport"].as<unsigned short>());
			
			start_check_upgrade_tv(vm["centerurl"].as<std::string>().c_str(), vm["centerport"].as<unsigned short>());


			L_INFO("Running server @ {0} ...", parms.port_number);
			xmlrpc_server_abyss_run_server(&env, serverP);

			L_INFO("Stopping server @ {0} ...", parms.port_number);
			xmlrpc_server_abyss_restore_sig(oldHandlersP);

			free(oldHandlersP);

			stop_check_upgrade();
			stop_check_upgrade_tv();
		}
		xmlrpc_server_abyss_destroy(serverP);

		if (env.fault_occurred) {
			L_CRITICAL("xmlrpc_server_abyss() failed.  {0}", env.fault_string);
			break;
		}
	} while (0);

	if (registryP) {
		xmlrpc_registry_free(registryP);
	}

	xmlrpc_server_abyss_global_term();

#ifdef IS_SERVICE_VER_
	ServiceSt.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(hServiceStatus, &ServiceSt);
#endif // IS_SERVICE_VER_

	L_ERROR("server @ {0} STOPED ...", parms.port_number);

	clean_db_client();

	curl_global_cleanup();
}

static void WINAPI ServiceCtrl(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		L_DEBUG("Stopping ...");
		ServiceSt.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(hServiceStatus, &ServiceSt);
		xmlrpc_server_abyss_terminate(&env, serverP);
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		break;
	default:
		break;
	}
}

static void Run()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (!hSCM) {
		return;
	}

	SC_HANDLE hService = OpenService(hSCM, SERVICE_N, SERVICE_START);
	if (!hService) {
		CloseServiceHandle(hSCM);
		return;
	}

	StartService(hService, 0, 0);
	CloseServiceHandle(hService);
}

static void Stop()
{
	SERVICE_STATUS St;

	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) {
		return;
	}

	SC_HANDLE hService = OpenService(hSCM, SERVICE_N, SERVICE_STOP | SERVICE_QUERY_STATUS);
	if (!hService) {
		CloseServiceHandle(hSCM);
		return;
	}
	
	QueryServiceStatus(hService, &St);
	ControlService(hService, SERVICE_CONTROL_STOP, &St);
	CloseServiceHandle(hService);
}

static bool IsInstalled()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) {
		return false;
	}

	SC_HANDLE hService = OpenService(hSCM, SERVICE_N, SERVICE_QUERY_CONFIG);
	if (!hService) {
		CloseServiceHandle(hSCM);
		return false;
	}

	CloseServiceHandle(hService);

	return true;
}

static bool Install()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM) {
		return false;
	}

	char szExe[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szExe, MAX_PATH);

	SC_HANDLE hService = CreateService(
		hSCM,
		SERVICE_N,
		SERVICE_N,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_IGNORE,
		szExe,
		NULL,
		NULL,
		"",
		NULL,
		NULL);

	if (!hService) {
		CloseServiceHandle(hSCM);
		return false;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCM);

	return true;
}

static void UnInstall()
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (!hSCM) {
		return;
	}

	SC_HANDLE hService = OpenService(hSCM, SERVICE_N, SERVICE_STOP | DELETE);
	if (!hService) {
		CloseServiceHandle(hSCM);
		return;
	}
	
	DeleteService(hService);
	CloseServiceHandle(hService);
}

static void InitSpdLog(const char* logfile, int level)
{
	const char* p = strrchr(logfile, '\\');
	const char* p1 = strrchr(logfile, '/');
	p = p1 && p > p1 ? p1 : p;

	if (p) {
		char szpath[MAX_PATH] = { 0 };
		strncpy_s(szpath, logfile, p - logfile);
		boost::filesystem::path path(szpath);

		boost::system::error_code ec;
		boost::filesystem::create_directories(path, ec);
	}

	spdlog::spdlog_init(SERVICE_N, logfile, level);
}

static const char* GetExePath(char* szExePath)
{
	GetModuleFileName(NULL, szExePath, MAX_PATH);
	char* p = strrchr(szExePath, '\\');
	if (!p) {
		p = strrchr(szExePath, '/');
	}

	if (!p) {
		return 0;
	}
	p[1] = '\0';

	return szExePath;
}
