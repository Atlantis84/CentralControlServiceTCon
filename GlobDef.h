
#define lock_it() std::lock_guard<std::mutex> l__(mutex_);

#define xmlrpc_set_str_v(dataP__, v__) xmlrpc_struct_set_value(envP, dataP__, #v__, xmlrpc_string_new(envP, v__.c_str()));
#define xmlrpc_set_b64_v(dataP__, v__) xmlrpc_struct_set_value(envP, dataP__, #v__, xmlrpc_base64_new(envP, v__.size(), (unsigned char*)v__.c_str()));
#define xmlrpc_set_int_v(dataP__, v__) xmlrpc_struct_set_value(envP, dataP__, #v__, xmlrpc_int_new(envP, v__));
#define xmlrpc_set_sec_v(dataP__, v__) xmlrpc_struct_set_value(envP, dataP__, #v__, xmlrpc_datetime_new_sec(envP, v__));

#define decompose_ret_value() \
	xmlrpc_int ec; \
	xmlrpc_DECREF(dataP); \
	xmlrpc_decompose_value(envP, resultP, "{s:i,s:S,*}", "ec", &ec, "data", &dataP); \
	xmlrpc_DECREF(resultP); \
	\
	if ((envP)->fault_occurred) { \
		L_ERROR("{0} @ {1}, fault_string : {2} fault_code {3})", \
			__FUNCTION__, __LINE__, (envP)->fault_string, (envP)->fault_code); \
		return build_ret_value(EC_INVALID_ARGS, dataP); \
	}

#define build_ret_value(ec__, dataP__) \
	xmlrpc_build_value(envP, "{s:i,s:S}", "ec", (ec__), "data", (dataP__));

#define call_db_server(fun_name__) \
	xmlrpc_value* resultP = xmlrpc_client_call_params(&env, db_server_url, "proc."###fun_name__, paramArrayP); \
	if (env.fault_occurred) { \
		L_ERROR("{0} @ {1}, fault_string : {2} fault_code {3})", \
			__FUNCTION__, __LINE__, env.fault_string, env.fault_code); \
		return build_ret_value(EC_DB_ERROR, dataP); \
	} \

#define CheckIfFaultOccurred(envP__) \
	if ((envP__)->fault_occurred) { \
		L_ERROR("{0} @ {1}, fault_string : {2} fault_code {3})", \
			__FUNCTION__, __LINE__, (envP__)->fault_string, (envP__)->fault_code); \
		return build_ret_value(-1, dataP); \
	}

#define CheckIfFaultOccurredServer() CheckIfFaultOccurred(envP);
#define CheckIfFaultOccurredClient() CheckIfFaultOccurred(&env);

#define Env_Clean(envP__) \
	if ((envP__)->fault_occurred) { \
		xmlrpc_env_clean((envP__)); \
		memset((envP__), 0, sizeof(*(envP__))); \
	}

#define CleanServerEnv() Env_Clean(envP);
#define CleanClientEnv() Env_Clean(&env);

enum LST {
	LST_SUCCESS			= 0,
	LST_INVALID_USR		= 1,
	LST_INVALID_PASS	= 2,
	LST_UNKNOWN			= 3,
};

enum RST {
	ST_NOPE		= 0x00,
	ST_NORMAL	= 0x01,
	ST_DISABLED = 0x02,
	ST_DELETE	= 0x04,
	ST_BLOCK	= 0x08,		// for user
};

enum EC {
	EC_SUCCESS		= 0,
	EC_INVALID_ARGS = -600,
	EC_DB_ERROR		= -601,
	EC_INVALID_USR	= -602,
	EC_INVALID_PASS	= -603,
	EC_BLOCK_USR	= -604,
	EC_AUTH_FAILED	= -605,
};
