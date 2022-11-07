#pragma once

#include "mysqlx/xdevapi.h"
#include "xmlrpc-c/base.h"

#include <mutex>
#include <string>
#include <functional>
#include <future>
#include <functional>
#include <thread>
#include <chrono>
#include <stdexcept>

using usrinfo_cb_type = std::function<void(const std::string& uuid, const std::string& passwd, const std::string& nick, int status, const std::string& auth_name, int auth_value)>;
using usrauth_cb_type = std::function<void(const std::string& uuid, unsigned auth_value)>;
using auth_cb_type = usrauth_cb_type;
using testplan_cb_type = std::function<void(const std::string& uuid, const std::string& project, const std::string& board, const std::string& bom, time_t create_ts, time_t edit_ts, const std::string& d)>;
using testplan_main_cb_type = std::function<void(const std::string& uuid, int item, const std::string& name, int times, int order, int stat, const std::string& creator, const std::string& editor)>;
using testplan_cmd_cb_type = std::function<void(const std::string& uuid, const std::string& board, const std::string& name, const std::string& type, int delay, int timeout, int order, int stat, const std::string& creator, const std::string& editor)>;
using testplan_img_cb_type = std::function<void(const std::string& uuid, const std::string& board, const std::string& name, const std::string& pipe, int value, int timeout, const std::string& img_md5, int order, int stat, const std::string& creator, const std::string& editor)>;
using testplan_image_cb_type = std::function<void(const std::string& md5, const std::string& img)>;
using testplan_sound_cb_type = std::function<void(const std::string& uuid, const std::string& board, const std::string& name, int hzmax, int hzmin, int valuemax, int valuemin, int timeout, int order, int stat, const std::string& creator, const std::string& editor)>;
using testplan_status_cb_type = std::function<void(const std::string& uuid, int yahe, int sn, int pcbon, int delay, int img, int comopen, int comclose, int mes, int pcboff, int order, int stat, const std::string& creator, const std::string& editor)>;
using testplan_strcmp_cb_type = std::function<void(const std::string& uuid, const std::string& board, const std::string& name, int valuemin, int valuemax, int timeout, int order, int stat, const std::string& creator, const std::string& editor)>;
using testplan_voltage_cb_type = std::function<void(const std::string& uuid, const std::string& board, const std::string& name, int valuemin, int valuemax, int timeout, int order, int stat, const std::string& creator, const std::string& editor)>;
using tcon_testplan_cb_type = std::function<void(const std::string& md5, unsigned offset, unsigned trunk_size, unsigned file_size)>;
using tcon_testhstvt_cb_type = std::function<void(const std::string& md5, const std::string& content)>;
using testdatabaseuuid_cb_type = std::function<void(const std::string& uuid)>;
using tcon_query_upgrade_cb_type = std::function<void(const std::string& areaName, const std::string& machineName, const std::string& boardStyle, const std::string& machineStyleName, const std::string& halfMaterialNumber, int stat)>;
using tcon_check_hstvt_trunk_cb = std::function<void(const std::string& uuid)>;
using testdatabaselist_cb_type= std::function<void(const std::string& uuid,const std::string& sn, const std::string& mac, const std::string& softwareversion, const std::string& keyname,
	const std::string& testresult, const std::string& testplan, const std::string& modulebom, const std::string& areaname, const std::string& deviceno, const std::string& toolno,
	const std::string& failprjname, const std::string& failsubprjname, const std::string& testip, const std::string& pcname, const std::string& testtime, const time_t testdate,
	const std::string& testperson)>;
using testplan_trunk_cb_type = std::function<void(const std::string& md5, unsigned offset, unsigned trunk_size, unsigned file_size)>;
using test_config_data_cb_type = std::function<void(int uuid,std::string& boardtype,
	std::string& halfmaterialnum,std::string& areaname,
	std::string&machinename,std::string&usrname,std::string& edittime)>;
using tv_config_data_cb_type = std::function<void(std::string& uuid, std::string& recipename,
	std::string& areaname, std::string& machinename,
	std::string& project, std::string& templatetype,
	std::string&shortbom, std::string& modulebom, std::string& materialno,
	std::string&testdata, std::string&usrname, std::string&edittime)>;
using test_details_cb_type = std::function<void(std::string& testdetails)>;
using template_cb = std::function<void(const std::string& templatetype)>;
using tv_testplan_cb_type = std::function<void(const std::string& md5, unsigned offset, unsigned trunk_size, unsigned file_size)>;
using tv_testhstvt_cb_type = std::function<void(const std::string& md5, const std::string& content)>;
using tv_query_upgrade_cb_type = std::function<void(const std::string& areaName, const std::string& machineName, const std::string& TemplateType, const std::string& ModuleBom, const std::string& MaterialNo, int stat)>;
using filename_cb = std::function<void(const std::string& newfilename)>;
using cor_cb_type = std::function<void(std::string& programno, std::string& remarks,
	std::string& editor, std::string& edittime)>;
using cb_actype = std::function<void(std::string& acBoardType)>;
using cb_remark = std::function<void(std::string& Remark, std::string& uuid)>;
using cb_pcbtype = std::function<void(std::string& pcbtype)>;
using cb_moduleno = std::function<void(std::string& moduleno)>;

class mysqlx_session {
public:
	mysqlx_session(const char* host, unsigned short port, const char* usr, const char* pass);
	~mysqlx_session();

public:
	bool init_session();
	
	xmlrpc_int32 query_usrinfo(const char* usr, const usrinfo_cb_type& cb);
	xmlrpc_int32 query_usrauth(const char* uuid, const usrauth_cb_type& cb);
	xmlrpc_int32 query_usrauth2(const char* uuid, const char* uuid1, const usrauth_cb_type& cb);
	xmlrpc_int32 query_auth(const char* uuid, const auth_cb_type& cb);

	xmlrpc_int32 query_testplan(const char* uuid, const testplan_cb_type& cb);
	xmlrpc_int32 query_testplan1(const char* project, const char* board, const char* bom, const testplan_cb_type& cb);
	xmlrpc_int32 query_testplan2(const char* project, const char* board, const char* bom, const testplan_cb_type& cb);
	xmlrpc_int32 query_testplan_by_creator(const char* creator, const testplan_cb_type& cb);
	xmlrpc_int32 query_testplan_by_editor(const char* editor, const testplan_cb_type& cb);

	xmlrpc_int32 query_testplan_main(const char* uuid, const testplan_main_cb_type& cb);
	xmlrpc_int32 query_testplan_cmd(const char* uuid, const testplan_cmd_cb_type& cb);
	xmlrpc_int32 query_testplan_img(const char* uuid, const testplan_img_cb_type& cb);
	xmlrpc_int32 query_testplan_image(const char* uuid, const testplan_image_cb_type& cb);
	xmlrpc_int32 query_testplan_sound(const char* uuid, const testplan_sound_cb_type& cb);
	xmlrpc_int32 query_testplan_status(const char* uuid, const testplan_status_cb_type& cb);
	xmlrpc_int32 query_testplan_strcmp(const char* uuid, const testplan_strcmp_cb_type& cb);
	xmlrpc_int32 query_testplan_voltage(const char* uuid, const testplan_voltage_cb_type& cb);
	
	xmlrpc_int32 add_usr(const char* usr, const char* pass, const char* auth, const char* nick, const char* creator);
	xmlrpc_int32 delete_usr(const char* uuid, const char* editor);
	xmlrpc_int32 modify_usr(const char* uuid, const char* auth, const char* nick, const char* editor);
	xmlrpc_int32 modify_usr_stat(const char* uuid, xmlrpc_int32 stat, const char* editor);
	xmlrpc_int32 modify_usr_pass(const char* uuid, const char* pass, const char* editor);
	xmlrpc_int32 modify_usr_auth(const char* uuid, const char* auth, const char* editor);
	xmlrpc_int32 modify_usr_nick(const char* uuid, const char* nick, const char* editor);
	
	xmlrpc_int32 add_testplan(int project, const char* board, const char* bom, int stat, const char* creator);
	xmlrpc_int32 delete_testplan(const char* uuid);
	xmlrpc_int32 modify_testplan(const char* uuid, int stat, const char* editor);

	xmlrpc_int32 add_testplan_cmd(const char* uuid, const char* name, const char* type, int delay, int timeout, int order, const char* creator);
	xmlrpc_int32 delete_testplan_cmd(const char* uuid, const char* editor);
	xmlrpc_int32 modify_testplan_cmd(const char* uuid, const char* name, const char* type, int delay, int timeout, int order, const char* editor);
	xmlrpc_int32 add_tcon_recipe_size(const char* name, int size);
	xmlrpc_int32 add_tv_recipe_size(const char* name, int size,const char* line,const char* deviceno);
	xmlrpc_int32 add_testplan_img(const char* uuid, const char* name, const char* pipe, int value, int timeout, int order, const char* creator);
	xmlrpc_int32 delete_testplan_img(const char* uuid, const char* editor);
	xmlrpc_int32 modify_testplan_img(const char* uuid, const char* name, const char* type, int delay, int timeout, int order, const char* editor);

	xmlrpc_int32 add_testdatabase(const char* sn,const char* mac,const char* softwareversion,const char* key,const char* testresult,const char* TestPlanName,const char* modulebom,
								  const char* areaname,const char* deviceno,const char* toolno, const char* failprojectname, const char* failsubprojectname,
							      const char* testid,const char* pcname,const char* testtime,const char* testperson);
	xmlrpc_int32 add_testdatabasedetail(const char* uuid, const char* sn, const char* id, const char* testprojectname, const char* subtestprojectname,
		const char* testresult, const char* testval,const char* testlowlimit, const char* testhightlimit, const char* ismain, const char* testtime);
	xmlrpc_int32 add_tcon_testplan(const char* name, const char* md5, const char* ctx, int offset, int size, const char* author);
	xmlrpc_int32 add_tv_testplan(const char* name,const char* md5, const char* ctx, int offset, int size, const char* author);

	xmlrpc_int32 query_tcon_testplan(const char* name, const tcon_testplan_cb_type& cb);
	xmlrpc_int32 query_tcon_hstvt(const char* md5, const tcon_testhstvt_cb_type& cb);
	xmlrpc_int32 query_databaseuuid(const char* sn, const testdatabaseuuid_cb_type&cb );
	xmlrpc_int32 update_testdatabase(const char* uuid,const char* failprjname,const char* failsubprjname);

	xmlrpc_int32 query_tcon_upgrade(const char* type, const char* areaname, const char* machine_name, const tcon_query_upgrade_cb_type& cb);
	xmlrpc_int32 updata_tcon_upgrade(const char* type, const char* areaname, const char* machine_name, const char* board_style, const char* machine_style_name, const char* half_material_no, int stat);
	xmlrpc_int32 updata_tcon_upgrade_machine(const char* type, const char* areaname, const char* machine_name, int stat);
	xmlrpc_int32 check_tcon_hstvt_trunk(const std::vector<std::string>& md5, const tcon_check_hstvt_trunk_cb& cb);
	xmlrpc_int32 check_tv_hstvt_trunk(const std::vector<std::string>& md5, const tcon_check_hstvt_trunk_cb& cb);

	xmlrpc_int32 check_tcon_testplan_exist(const char* type, const char* areaname, const char* machine_name);
	xmlrpc_int32 query_databaselist(const char* starttime, const char* endtime, const char* sn, const char* testplan,const char* failprjname,const char* failsubprjname,
									const char* testip,const char* pcname,const char* testresult,const char* areaname,const char* deviceno,const char* toolno, 
		const testdatabaselist_cb_type& cb);

	xmlrpc_int32 new_testplan(const char* project, const char* board, const char* bom);
	xmlrpc_int32 update_testplan_size(const char* project, const char* board, const char* bom, int size);
	xmlrpc_int32 update_testplan_trunk(const char* project, const char* board, const char* bom, const char* md5, const char* ctx, int offset, int size, const char* author);

	xmlrpc_int32 query_testplan_trunk(const char* project, const char* board, const char* bom, const testplan_trunk_cb_type& cb);
	xmlrpc_int32 test_json_insert(const char* boardtype, const char* halfmaterialnum, const char* areaname, const char* machinename, const char* username, const char* testdata);
	xmlrpc_int32 test_json_insert_tv(const char* recipe_name,const char* areaname, const char* machinename, const char* project,const char* templatetype, const char* shortbom,const char* modulebom,const char* materialno, const char* username, const char* testdata);

	xmlrpc_int32 query_test_config_data(const char* boardtype, const char* halfmaterialnum, bool displayhistoryeditflag, const test_config_data_cb_type& cb);
	xmlrpc_int32 get_test_details(const char* uuid, const test_details_cb_type& cb);
	xmlrpc_int32 add_template(const char* project, const char* templatetype,const char* creator);
	xmlrpc_int32 query_template(const char* project, const template_cb& cb);
	xmlrpc_int32 query_test_config_data_tv(const char* areaname,const char* machinename,const char* project, 
		const char* templatetype, const char* shortbom,const char* materialno,
		const tv_config_data_cb_type& cb);
	xmlrpc_int32 query_tv_hstvt(const char* md5, const tv_testhstvt_cb_type& cb);
	xmlrpc_int32 query_tv_testplan(const char* name, const tv_testplan_cb_type& cb);
	xmlrpc_int32 check_tv_testplan_exist(const char* templatetype, const char* modulebom, const char* materialno,const char* areaname, const char* machine_name);
	xmlrpc_int32 updata_tv_upgrade(const char* type, const char* areaname, const char* machine_name, const char* templatetype, const char* modulebom_sytle, const char* materialNo, int stat);
	xmlrpc_int32 query_tv_upgrade(const char* type, const char* areaname, const char* machine_name, const tv_query_upgrade_cb_type& cb);
	xmlrpc_int32 update_tv_upgrade_machine(const char* type, const char* areaname, const char* machine_name, int stat);
	xmlrpc_int32 check_filename(const char* filename, const filename_cb& cb);
	xmlrpc_int32 add_cor(const char* acBordType, const char* programno, const char* remarks, const char* editor);
	xmlrpc_int32 query_cordata(const char* boardtype, const cor_cb_type& cb);
	xmlrpc_int32 query_distinctacType(const cb_actype& cb);
	xmlrpc_int32 query_Remark(const char* acboardType,const char* programno,const cb_remark& cb);
	xmlrpc_int32 add_platetype(const char* BoardType/**主板型号*/, const char* moduleNo/**组件号*/, const char* uuid/**uuid*/, const char* editor/**操作员*/);
	xmlrpc_int32 query_distinctpcbtype(const cb_pcbtype& cb);
	xmlrpc_int32 query_moduleNo(const char* baordtype, const cb_moduleno& cb);

private:
	xmlrpc_int32 testplan_uuid(const char* project, const char* board, const char* bom, std::string& uuid);
	xmlrpc_int32 testplan_clean(const std::string& uuid);
	xmlrpc_int32 testplan_trunk_insert(const char* md5, const char* content);
	xmlrpc_int32 testplan_ln_insert(const std::string& uuid, const char* md5, int offset, int size, const char* author);

private:
	std::string host_;
	unsigned short port_;
	std::string usr_, pass_;
	std::string db_;

	std::mutex mutex_;
	mysqlx::Session* tv4th_session_;
};
