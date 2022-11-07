#include "mysqlxwapper.h"
#include "spdlogwapper.hpp"
#include "../GlobDef.h"

#include <boost\lexical_cast.hpp>
#include <boost\uuid\uuid.hpp>
#include <boost\uuid\uuid_io.hpp>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>

#include "threadpool.h"
std::threadpool tpool(5000);

#define DB_TV4TH				"tv4th"
#define TBL_USRINFO				DB_TV4TH##".usrinfo"
#define TBL_AUTH				DB_TV4TH##".auth"
#define TBL_TESTPLAN			DB_TV4TH##".testplan"
#define TBL_CMD					DB_TV4TH##".cmd"
#define TBL_IMG					DB_TV4TH##".img"
#define TBL_IMAGE				DB_TV4TH##".image"
#define TBL_MAIN				DB_TV4TH##".main"
#define TBL_SOUND				DB_TV4TH##".sound"
#define TBL_STATUS				DB_TV4TH##".status"
#define TBL_STRCMP				DB_TV4TH##".strcmp"
#define TBL_VOLTAGE				DB_TV4TH##".voltage"
#define TBL_TCON_TESTPLAN		DB_TV4TH##".tcon_testplan"
#define TBL_TESTDATABASE		DB_TV4TH##".testdatabase"
#define TBL_TESTDATADETAIL		DB_TV4TH##".testdatabasedetail"
#define TBL_TCON_HSTVT			DB_TV4TH##".tcon_hstvt"
#define TBL_TCON_RECIPE_SIZE	DB_TV4TH##".tcon_recipe_size"
#define TBL_TV_RECIPE_SIZE	    DB_TV4TH##".tv_recipe_size"
#define TBL_TCON_UPGRADE		DB_TV4TH##".tcon_upgrade"
#define TBL_TCON_UPGRADE_STAT	DB_TV4TH##".tcon_upgrade_stat"
#define TBL_TESTPLAN_SIZE		DB_TV4TH##".testplan_size"
#define TBL_TESTPLAN_LN			DB_TV4TH##".testplan_ln"
#define TBL_TEST_JSON			DB_TV4TH##".tcon_recipe_edit_record"
#define TBL_TEMPLATE 	        DB_TV4TH##".template"
#define TBL_TV_HSTVT			DB_TV4TH##".tv_hstvt"
#define TBL_TV_TESTPLAN		    DB_TV4TH##".tv_testplan"
#define TBL_TEST_EDITRECORD		DB_TV4TH##".tv_recipe_edit_record"
#define TBL_TEST_EDITRECORDHIS	DB_TV4TH##".tv_recipe_edit_record_his"
#define TBL_TV_UPGRADE		    DB_TV4TH##".tv_upgrade"
#define TBL_TV_COR_Table		DB_TV4TH##".tv_COR_Table"
#define TBL_TV_COR_Table_His	DB_TV4TH##".tv_COR_Table_his"
#define TBL_TV_Plate_Table		DB_TV4TH##".tv_plate_table"
#define TBL_TV_Plate_Table_His	DB_TV4TH##".tv_plate_table_his"

#define DEFAULT_AUTH_UUID	"97E293A7-27F9-11EC-AE37-E86A64970C4F"

#define exec_query_sql_begin() \
	try { \
		mysqlx::SqlResult result = tv4th_session_->sql(sql).execute(); \
		L_TRACE(sql.c_str()); \
		xmlrpc_int32 count = result.count();

#define exec_query_sql_end() \
		return count; \
	} \
	catch (...) { \
	} \
	\
	return EC_DB_ERROR;

#define exec_update_sql_begin exec_query_sql_begin
#define exec_update_sql_end exec_query_sql_end

#define exec_update_sql() \
	try { \
		tv4th_session_->sql(sql).execute(); \
		return EC_SUCCESS; \
	} catch (...) { \
	} \
	\
	return EC_DB_ERROR;

mysqlx_session::mysqlx_session(const char* host, unsigned short port, const char* usr, const char* pass)
	: tv4th_session_(0)
	, host_(host && host[0] ? host : "127.0.0.1")
	, port_(port)
	, usr_(usr)
	, pass_(pass)
{
}

mysqlx_session::~mysqlx_session()
{
	if (tv4th_session_) {
		tv4th_session_->close();
	}
}

bool mysqlx_session::init_session()
{
	try {
		tv4th_session_ = new mysqlx::Session(mysqlx::SessionSettings(host_, port_, usr_, pass_));
	}
	catch (...) {}

	return tv4th_session_ != 0;
}

xmlrpc_int32 mysqlx_session::query_usrinfo(const char* usr, const usrinfo_cb_type& cb)
{
	lock_it();
	std::string sql;
	
	sql.append("SELECT ")
		.append("u.uuid,")
		.append("u.pass,")
		.append("u.nick,")
		.append("u.stat,")
		.append("a.name,")
		.append("a.value")
		.append(" FROM ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" INNER JOIN ")
		.append(TBL_AUTH).append(" AS a")
		.append(" ON ")
		.append("UPPER(u.auth) = UPPER(a.uuid)")
		.append(" OR ")
		.append("(")
		.append("u.auth = \'\' AND ")
		.append("a.uuid = ").append("\'").append(DEFAULT_AUTH_UUID).append("\'")
		.append(")")
		.append(" WHERE ")
		.append("u.usr = ").append("UPPER(\'").append(usr).append("\')")
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<int>(), row[4].get<std::string>(), row[5].get<int>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_usrauth(const char* uuid, const usrauth_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("u.uuid,")
		.append("a.value")
		.append(" FROM ")
		.append(TBL_AUTH).append(" AS a")
		.append(" INNER JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("UPPER(a.uuid) = UPPER(u.auth)")
		.append(" WHERE ")
		.append("u.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<unsigned>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_usrauth2(const char* uuid, const char* uuid1, const usrauth_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("u.uuid,")
		.append("a.value")
		.append(" FROM ")
		.append(TBL_AUTH).append(" AS a")
		.append(" INNER JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("UPPER(a.uuid) = UPPER(u.auth)")
		.append(" WHERE ")
		.append("u.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(" OR ")
		.append("u.uuid = ").append("UPPER(\'").append(uuid1).append("\')")
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<unsigned>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_auth(const char* uuid, const auth_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("uuid,")
		.append("value")
		.append(" FROM ")
		.append(TBL_AUTH)
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<unsigned>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan(const char* uuid, const testplan_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("p.uuid,")
		.append("p.project, ")
		.append("p.board, ")
		.append("p.bom, ")
		.append("unix_timestamp(p.create_ts),")
		.append("unix_timestamp(p.edit_ts),")
		.append("p.desc")
		.append(" FROM ")
		.append(TBL_TESTPLAN).append(" AS p")
		.append(" WHERE ")
		.append("p.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(" AND ")
		.append("p.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<time_t>(), row[5].get<time_t>(), row[6].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan1(const char* project, const char* board, const char* bom, const testplan_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("p.uuid,")
		.append("p.project, ")
		.append("p.board, ")
		.append("p.bom, ")
		.append("unix_timestamp(p.create_ts),")
		.append("unix_timestamp(p.edit_ts),")
		.append("p.desc")
		.append(" FROM ")
		.append(TBL_TESTPLAN).append(" AS p")
		.append(" WHERE ")
		.append("p.project = \'").append(project).append("\'");
		
		if (board && *board) {
			sql.append(" AND ")
				.append("p.board LIKE ").append("\'%").append(board).append("%\'");
		}
		if (bom && *bom) {
			sql.append(" AND ")
				.append("p.bom LIKE ").append("\'%").append(bom).append("%\'");
		}
		
		sql.append(" AND ")
			.append("p.stat = ").append(std::to_string(ST_NORMAL))
			.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<time_t>(), row[5].get<time_t>(), row[6].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan2(const char* project, const char* board, const char* bom, const testplan_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("p.uuid,")
		.append("p.project, ")
		.append("p.board, ")
		.append("p.bom, ")
		.append("unix_timestamp(p.create_ts),")
		.append("unix_timestamp(p.edit_ts),")
		.append("p.desc")
		.append(" FROM ")
		.append(TBL_TESTPLAN).append(" AS p")
		.append(" WHERE ")
		.append("p.project=\'").append(project).append("\'")
		.append(" AND ")
		.append("p.board=").append("\'").append(board).append("\'")
		.append(" AND ")
		.append("p.bom=").append("\'").append(bom).append("\'")
		.append(" AND ")
		.append("p.stat=").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<time_t>(), row[5].get<time_t>(), row[6].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_by_creator(const char* creator, const testplan_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("p.uuid,")
		.append("p.project, ")
		.append("p.board, ")
		.append("p.bom, ")
		.append("unix_timestamp(p.create_ts),")
		.append("unix_timestamp(p.edit_ts),")
		.append("p.desc")
		.append(" FROM ")
		.append(TBL_TESTPLAN).append(" AS p")
		.append(" INNER JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("p.creator = u.uuid").append(" OR ").append("p.editor = u.uuid")
		.append(" WHERE ")
		.append("p.creator = ").append("UPPER(\'").append(creator).append("\')")
		.append(" AND ")
		.append("p.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<time_t>(), row[5].get<time_t>(), row[6].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_by_editor(const char* editor, const testplan_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("p.uuid,")
		.append("p.project, ")
		.append("p.board, ")
		.append("p.bom, ")
		.append("unix_timestamp(p.create_ts),")
		.append("unix_timestamp(p.edit_ts),")
		.append("p.desc")
		.append(" FROM ")
		.append(TBL_TESTPLAN).append(" AS p")
		.append(" INNER JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("p.creator = u.uuid").append(" OR ").append("p.editor = u.uuid")
		.append(" WHERE ")
		.append("p.editor = ").append("UPPER(\'").append(editor).append("\')")
		.append(" AND ")
		.append("p.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<time_t>(), row[5].get<time_t>(), row[6].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_main(const char* uuid, const testplan_main_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("m.uuid,")
		.append("m.item,")
		.append("m.name,")
		.append("m.times,")
		.append("m.order,")
		.append("m.stat,")
		.append("u.nick,")
		.append("u.nick,")
		.append("unix_timestamp(m.create_ts),")
		.append("unix_timestamp(m.edit_ts)")
		.append(" FROM ")
		.append(TBL_MAIN).append(" AS m")
		.append(" INNER JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("m.creator = u.uuid").append(" OR ").append("m.editor = u.uuid")
		.append(" WHERE ")
		.append("m.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<int>(), row[2].get<std::string>(), row[3].get<int>(), row[4].get<int>(), row[5].get<int>(), row[6].get<std::string>(), row[7].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_cmd(const char* uuid, const testplan_cmd_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("c.uuid,")
		.append("c.idx,")
		.append("c.name,")
		.append("c.type,")
		.append("c.delay,")
		.append("c.timeout,")
		.append("c.order,")
		.append("c.stat,")
		.append("u.nick,")
		.append("u.nick,")
		.append("unix_timestamp(c.create_ts),")
		.append("unix_timestamp(c.edit_ts)")
		.append(" FROM ")
		.append(TBL_CMD).append(" AS c")
		.append(" LEFT JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("c.creator = u.uuid").append(" OR ").append("c.editor = u.uuid")
		.append(" WHERE ")
		.append("c.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(" AND ")
		.append("c.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	xmlrpc_int32 count1 = result.count();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<int>(), row[5].get<int>(), row[6].get<int>(), row[7].get<int>(), row[8].isNull() ? "" : row[8].get<std::string>(), row[9].isNull() ? "" : row[9].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_img(const char* uuid, const testplan_img_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("i.uuid,")
		.append("i.idx,")
		.append("i.name,")
		.append("i.pipe,")
		.append("i.value,")
		.append("i.timeout,")
		.append("i.img,")
		.append("i.order,")
		.append("i.stat,")
		.append("i.creator,")
		.append("i.editor,")
		.append("unix_timestamp(i.create_ts),")
		.append("unix_timestamp(i.edit_ts)")
		.append(" FROM ")
		.append(TBL_IMG).append(" AS i")
		.append(" LEFT JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("i.creator = u.uuid").append(" OR ").append("i.editor = u.uuid")
		.append(" WHERE ")
		.append("i.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(" AND ")
		.append("i.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<int>(), row[5].get<int>(), row[6].get<std::string>(), row[7].get<int>(), row[8].get<int>(), row[9].get<std::string>(), row[10].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_image(const char* md5, const testplan_image_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("md5,")
		.append("content")
		.append(" FROM ")
		.append(TBL_IMAGE)
		.append(" WHERE ")
		.append("md5 = ").append("UPPER(\'").append(md5).append("\')")
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		auto bytes = row[1].getRawBytes();
		std::string content;
		std::copy(bytes.begin(), bytes.end(), std::back_inserter(content));
		cb(row[0].get<std::string>(), content);
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_sound(const char* uuid, const testplan_sound_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("s.uuid,")
		.append("s.idx,")
		.append("s.name,")
		.append("s.hzmax,")
		.append("s.hzmin,")
		.append("s.valuemax,")
		.append("s.valuemin,")
		.append("s.timeout,")
		.append("s.order,")
		.append("s.stat,")
		.append("u.nick,")
		.append("u.nick,")
		.append("unix_timestamp(s.create_ts),")
		.append("unix_timestamp(s.edit_ts)")
		.append(" FROM ")
		.append(TBL_SOUND).append(" AS s")
		.append(" LEFT JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("s.creator = u.uuid").append(" OR ").append("s.editor = u.uuid")
		.append(" WHERE ")
		.append("s.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(" AND ")
		.append("s.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<int>(), row[4].get<int>(), row[5].get<int>(), row[6].get<int>(), row[7].get<int>(), row[8].get<int>(), row[9].get<int>(), row[10].get<std::string>(), row[11].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_status(const char* uuid, const testplan_status_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("uuid,")
		.append("yahe,")
		.append("sn,")
		.append("pcbon,")
		.append("delay,")
		.append("img,")
		.append("comopen,")
		.append("comclose,")
		.append("mes,")
		.append("pcboff,")
		.append("order,")
		.append("stat,")
		.append("creator,")
		.append("editor,")
		.append("unix_timestamp(create_ts),")
		.append("unix_timestamp(edit_ts)")
		.append(" FROM ")
		.append(TBL_STATUS)
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>()								  // uuid 
			, row[1].get<int>(), row[2].get<int>(), row[3].get<int>() // yahe, sn, pcbon
			, row[4].get<int>(), row[5].get<int>(), row[6].get<int>() // delay, img, comopen
			, row[7].get<int>(), row[8].get<int>(), row[9].get<int>() // comclose, mes, pcboff
			, row[10].get<int>(), row[11].get<int>()			      // order, stat
			, row[12].get<std::string>(), row[13].get<std::string>()); // creator, editor
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_strcmp(const char* uuid, const testplan_strcmp_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("s.uuid,")
		.append("s.idx,")
		.append("s.name,")
		.append("s.valuemin,")
		.append("s.valuemax,")
		.append("s.timeout,")
		.append("s.order,")
		.append("s.stat,")
		.append("u.nick,")
		.append("u.nick,")
		.append("unix_timestamp(s.create_ts),")
		.append("unix_timestamp(s.edit_ts)")
		.append(" FROM ")
		.append(TBL_STRCMP).append(" AS s")
		.append(" LEFT JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("s.creator = u.uuid").append(" OR ").append("s.editor = u.uuid")
		.append(" WHERE ")
		.append("s.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(" AND ")
		.append("s.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<int>(), row[4].get<int>(), row[5].get<int>(), row[6].get<int>(), row[7].get<int>(), row[8].get<std::string>(), row[9].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_testplan_voltage(const char* uuid, const testplan_voltage_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("v.uuid,")
		.append("v.idx,")
		.append("v.name,")
		.append("v.valuemin,")
		.append("v.valuemax,")
		.append("v.timeout,")
		.append("v.order,")
		.append("v.stat,")
		.append("u.nick,")
		.append("u.nick,")
		.append("unix_timestamp(v.create_ts),")
		.append("unix_timestamp(v.edit_ts)")
		.append(" FROM ")
		.append(TBL_VOLTAGE).append(" AS v")
		.append(" LEFT JOIN ")
		.append(TBL_USRINFO).append(" AS u")
		.append(" ON ")
		.append("v.creator = u.uuid").append(" OR ").append("v.editor = u.uuid")
		.append(" WHERE ")
		.append("v.uuid = ").append("UPPER(\'").append(uuid).append("\')")
		.append(" AND ")
		.append("v.stat = ").append(std::to_string(ST_NORMAL))
		.append(";");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<int>(), row[4].get<int>(), row[5].get<int>(), row[6].get<int>(), row[7].get<int>(), row[8].get<std::string>(), row[9].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::add_usr(const char* usr, const char* pass, const char* auth, const char* nick, const char* creator)
{
	lock_it();
	std::string sql;

	sql.append("INSERT INTO ")
		.append(TBL_USRINFO)
		.append("(")
		.append("uuid,")
		.append("usr,")
		.append("pass,");
	if (auth && *auth) {
		sql.append("auth,");
	}
	if (nick && *nick) {
		sql.append("nick,");
	}
	if (creator && *creator) {
		sql.append("creator,");
	}

	sql.pop_back();
	sql.append(")");
	sql.append(" VALUES(")
		.append("UPPER(UUID()),")
		.append("UPPER(\'").append(usr).append("\'),")
		.append("UPPER(MD5(").append("\'").append(pass).append("\')),");
	if (auth && *auth) {
		sql.append("UPPER(\'").append(auth).append("\'),");
	}
	if (nick && *nick) {
		sql.append("\'").append(nick).append("\',");
	}
	if (creator && *creator) {
		sql.append("UPPER(\'").append(creator).append("\'),");
	}
	sql.pop_back();
	sql.append(");");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::delete_usr(const char* uuid, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_USRINFO)
		.append(" SET ")
		.append("stat = ").append(std::to_string(ST_DELETE)).append(", ")
		.append("editor = ").append("\'").append(editor).append("\'")
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::modify_usr(const char* uuid, const char* auth, const char* nick, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_USRINFO)
		.append(" SET ")
		.append("auth = ").append("UPPER(\'").append(auth).append("\'), ")
		.append("nick = ").append("\'").append(nick).append("\', ")
		.append("editor = ").append("UPPER(\'").append(editor).append("\')")
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::modify_usr_stat(const char* uuid, xmlrpc_int32 stat, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_USRINFO)
		.append(" SET ")
		.append("stat = ").append(std::to_string(stat))
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::modify_usr_pass(const char* uuid, const char* pass, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_USRINFO)
		.append(" SET ")
		.append("pass = ").append("UPPER(MD5(\'").append(pass).append("\')), ")
		.append("editor = ").append("UPPER(\'").append(editor).append("\')")
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::modify_usr_auth(const char* uuid, const char* auth, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_USRINFO)
		.append(" SET ")
		.append("auth = ").append("UPPER(\'").append(auth).append("\'), ")
		.append("editor = ").append("UPPER(\'").append(editor).append("\')")
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::modify_usr_nick(const char* uuid, const char* nick, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_USRINFO)
		.append(" SET ")
		.append("nick = ").append("\'").append(nick).append("\', ")
		.append("editor = ").append("UPPER(\'").append(editor).append("\')")
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::add_testplan(int project, const char* board, const char* bom, int stat, const char* creator)
{
	lock_it();
	std::string sql;
	sql.append("CALL tv4th.update_testplan(")
		.append("\'").append(std::to_string((char)project)).append("\',")
		.append("\'").append(board).append("\',")
		.append("\'").append(bom).append("\',")
		.append(std::to_string(stat)).append(",")
		.append("\'").append(creator).append("\');");

	exec_update_sql_begin();
	exec_update_sql_end();

	return 0;
}

xmlrpc_int32 mysqlx_session::delete_testplan(const char* uuid)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_TESTPLAN)
		.append(" SET ")
		.append("stat = ").append(std::to_string(ST_DELETE))
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::modify_testplan(const char* uuid, int stat, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_TESTPLAN)
		.append(" SET ");
	if (stat) {
		sql.append("stat = ").append(std::to_string(stat)).append(",");
	}
	if (editor && *editor) {
		sql.append("editor = \'").append("UPPER(\'").append(editor).append("\'),");
	}
	sql.pop_back();

	sql.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::add_testplan_cmd(const char* uuid, const char* name, const char* type, int delay, int timeout, int order, const char* creator)
{
	lock_it();
	std::string sql;
	sql.append("INSERT INTO ")
		.append(TBL_CMD)
		.append("(")
		.append("uuid,")
		.append("name,")
		.append("type,")
		.append("delay,")
		.append("timeout,")
		.append("order,")
		.append("creator")
		.append(")")
		.append(" VALUES(")
		.append("UPPER(\'").append(uuid).append("\'), ")
		.append("\'").append(name).append("\', ")
		.append("\'").append(type).append("\', ")
		.append(std::to_string(delay)).append(", ")
		.append(std::to_string(timeout)).append(", ")
		.append(std::to_string(order)).append(", ")
		.append("UPPER(\'").append(creator).append("\')")
		.append(");");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::delete_testplan_cmd(const char* uuid, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_CMD)
		.append(" SET ")
		.append("stat = ").append(std::to_string(ST_DELETE)).append(", ")
		.append("editor = \'").append("UPPER(\'").append(editor).append("\')")
		.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::modify_testplan_cmd(const char* uuid, const char* name, const char* type, int delay, int timeout, int order, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_CMD)
		.append(" SET");
	if (name && *name) {
		sql.append(" ").append("name = \'").append(name).append("\',");
	}
	if (type && *type) {
		sql.append(" ").append("type = \'").append(type).append("\',");
	}
	if (delay >= 0) {
		sql.append(" ").append("delay = ").append(std::to_string(delay)).append(",");
	}
	if (timeout >= 0) {
		sql.append(" ").append("timeout = ").append(std::to_string(timeout)).append(",");
	}
	if (order >= 0) {
		sql.append(" ").append("order = ").append(std::to_string(order)).append(",");
	}
	
	sql.pop_back();

	sql.append(" WHERE ")
		.append("uuid = ").append("UPPER(\'").append(uuid).append("\');");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::add_testplan_img(const char* uuid, const char* name, const char* type, int delay, int timeout, int order, const char* creator)
{
	lock_it();
	std::string sql;
	sql.append("INSERT INTO ")
		.append(TBL_IMG)
		.append("(")
		.append("uuid,")
		.append("name,")
		.append("type,")
		.append("delay,")
		.append("timeout,")
		.append("order,")
		.append("creator")
		.append(")")
		.append(" VALUES(")
		.append("UPPER(\'").append(uuid).append("\'), ")
		.append("\'").append(name).append("\', ")
		.append("\'").append(type).append("\', ")
		.append(std::to_string(delay)).append(", ")
		.append(std::to_string(timeout)).append(", ")
		.append(std::to_string(order)).append(", ")
		.append("UPPER(\'").append(creator).append("\')")
		.append(");");

	exec_update_sql();
}
xmlrpc_int32 mysqlx_session::add_tcon_recipe_size(const char* name, int size)
{
	lock_it();
	std::string sql;

	sql.append("DELETE FROM ")
		.append(TBL_TCON_TESTPLAN)
		.append(" WHERE ")
		.append("name = ").append("\'").append(name).append("\';");
	try {
		tv4th_session_->sql(sql).execute();
		L_TRACE(sql.c_str());
	}
	catch (...) {
	}

	sql.clear();
	sql.append("REPLACE INTO ")
		.append(TBL_TCON_RECIPE_SIZE)
		.append("(")
		.append("name,")
		.append("size")
		.append(")")
		.append(" VALUES")
		.append("(")
		.append("\'").append(name).append("\',")
		.append(std::to_string(size))
		.append(");");

	exec_update_sql();
}
xmlrpc_int32 mysqlx_session::add_tv_recipe_size(const char* name, int size, const char* line, const char* deviceno)
{
	lock_it();
	std::string sql;

	sql.append("DELETE FROM ")
		.append(TBL_TV_TESTPLAN)
		.append(" WHERE ")
		.append("name = ").append("\'").append(name).append("\'");
	try {
		tv4th_session_->sql(sql).execute();
		L_TRACE(sql.c_str());
	}
	catch (...) {
	}

	sql.clear();
	sql.append("REPLACE INTO ")
		.append(TBL_TV_RECIPE_SIZE)
		.append("(")
		.append("name,")
		.append("size")
		.append(")")
		.append(" VALUES")
		.append("(")
		.append("\'").append(name).append("\',")
		.append(std::to_string(size)).append("")
		.append(");");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::add_tcon_testplan(const char* name, const char* md5, const char* ctx, int offset, int size, const char* author)
{
	lock_it();
	std::string sql;

	sql.append("CALL tv4th.tcon_update(")
		.append("\'").append(name).append("\',")
		.append("\'").append(md5).append("\',")
		.append("\'").append(ctx).append("\',")
		.append(std::to_string(offset)).append(",")
		.append(std::to_string(size)).append(",")
		.append("\'").append(author).append("\');");
	L_INFO("CALL tv4th.tcon_update");
	exec_update_sql();
}

int thread_func(std::string sql)
{
	try
	{
		mysqlx::Session mySession("127.0.0.1", 33060, "root", "asdfasdf1");
		mySession.sql(sql).execute();
		L_INFO("CALL tv4th.tv_update SUCCESS");
		return 0;
	}
	catch (std::exception& e)
	{
		std::cout << "some unhappy happened...  " << std::this_thread::get_id() << e.what() << std::endl;
		L_ERROR("CALL tv4th.tv_update ERROR");
		return -1;
	}
}

xmlrpc_int32 mysqlx_session::add_tv_testplan(const char* name, const char* md5, const char* ctx, int offset, int size, const char* author)
{
	lock_it();
	std::string sql;
	sql.append("CALL tv4th.tv_update(")
		.append("\'").append(name).append("\',")
		.append("\'").append(md5).append("\',")
		.append("\'").append(ctx).append("\',")
		.append(std::to_string(offset)).append(",")
		.append(std::to_string(size)).append(",")
		.append("\'").append(author).append("\');");
	tpool.commit(thread_func, sql);
	return 0;
}

xmlrpc_int32 mysqlx_session::add_testdatabase(const char* sn, const char* mac, const char* softwareversion, const char* key, const char* testresult, 
	const char* testplanname, const char* modulebom,const char* areaname, const char* deviceno, const char* toolno, const char* failprojectname, 
	const char* failsubprojectname,const char* testip, const char* pcname, const char* testtime, const char* testperson)
{

	lock_it();
	std::string sql;
	sql.clear();
	sql.append("insert into ")
		.append(TBL_TESTDATABASE)
		.append("(")
		.append("uuid,")
		.append("sn");
	if (mac && *mac) {
		sql.append(", mac");
	}
	if (softwareversion && *softwareversion) {
		sql.append(", softwareversion");
	}
	if (key && *key) {
		sql.append(", keyname");
	}
	if (testresult && *testresult) {
		sql.append(", testresult");
	}
	if (testplanname && *testplanname) {
		sql.append(", testplanname");
	}
	if (modulebom && *modulebom) {
		sql.append(", modulebom");
	}
	if (areaname && *areaname) {
		sql.append(", areaname");
	}
	if (deviceno && *deviceno) {
		sql.append(", deviceno");
	}
	if (toolno && *toolno) {
		sql.append(", toolno");
	}
	if (failprojectname && *failprojectname) {
		sql.append(", FailProject");
	}
	if (failsubprojectname && *failsubprojectname) {
		sql.append(", FailSubProject");
	}
	if (testip && *testip) {
		sql.append(", testip");
	}
	if (pcname && *pcname) {
		sql.append(", pcname");
	}
	if (testperson && *testperson) {
		sql.append(", testperson");
	}
	if (testtime && *testtime) {
		sql.append(", testtime");
	}
	sql.append(" )VALUES(")
		.append("UPPER(UUID()),")
		.append("UPPER(\'").append(sn).append("\')");
	if (mac && *mac) {
		sql.append(",").append("\'").append(mac).append("\'");
	}
	if (softwareversion && *softwareversion) {
		sql.append(",").append("\'").append(softwareversion).append("\'");
	}
	if (key && *key) {
		sql.append(",").append("\'").append(key).append("\'");
	}
	if (testresult && *testresult) {
		sql.append(",").append("\'").append(testresult).append("\'");
	}
	if (testplanname && *testplanname) {
		sql.append(",").append("\'").append(testplanname).append("\'");
	}
	if (modulebom && *modulebom) {
		sql.append(",").append("\'").append(modulebom).append("\'");
	}
	if (areaname && *areaname) {
		sql.append(",").append("\'").append(areaname).append("\'");
	}
	if (deviceno && *deviceno) {
		sql.append(",").append("\'").append(deviceno).append("\'");
	}
	if (toolno && *toolno) {
		sql.append(",").append("\'").append(toolno).append("\'");
	}
	if (failprojectname && *failprojectname) {
		sql.append(",").append("\'").append(failprojectname).append("\'");
	}
	if (failsubprojectname && *failsubprojectname) {
		sql.append(",").append("\'").append(failsubprojectname).append("\'");
	}
	if (testip && *testip) {
		sql.append(",").append("\'").append(testip).append("\'");
	}
	if (pcname && *pcname) {
		sql.append(",").append("\'").append(pcname).append("\'");
	}
	if (testtime && *testtime) {
		sql.append(",").append("\'").append(testtime).append("\'");
	}
	if (testperson && *testperson) {
		sql.append(",").append("\'").append(testperson).append("\'");
	}

	sql.append(");");

	exec_update_sql();

}

xmlrpc_int32 mysqlx_session::add_testdatabasedetail(const char* uuid, const char* sn, const char* id, const char* testprojectname, const char* subtestprojectname,
	const char* testresult, const char* testval, const char* testlowlimit, const char* testhightlimit, const char* ismain, const char* testtime)
{

	lock_it();
	std::string sql;

	sql.clear();
	sql.append("insert into ")
		.append(TBL_TESTDATADETAIL)
		.append("(")
		.append("uuid,")
		.append("sn");
	if (id && *id) {
		sql.append(", id");
	}
	if (testprojectname&& *testprojectname) {
		sql.append(", testprojectname");
	}
	if (subtestprojectname && *subtestprojectname) {
		sql.append(", subtestprojectname");
	}
	if (testresult && *testresult) {
		sql.append(", testresult");
	}
	if (testval && *testval) {
		sql.append(", testval");
	}
	if (testlowlimit && *testlowlimit) {
		sql.append(", testlowlimit");
	}
	if (testhightlimit && *testhightlimit) {
		sql.append(", testhightlimit");
	}
	if (ismain && *ismain) {
		sql.append(", ismain");
	}
	if (testtime && *testtime) {
		sql.append(", testtime");
	}

	sql.append(" )VALUES(");
	sql.append("\'").append(uuid).append("\'");
	sql.append(",").append("\'").append(sn).append("\'");
	if (id && *id) {
		sql.append(",").append("\'").append(id).append("\'");
	}
	if (testprojectname && *testprojectname) {
		sql.append(",").append("\'").append(testprojectname).append("\'");
	}
	if (subtestprojectname && *subtestprojectname) {
		sql.append(",").append("\'").append(subtestprojectname).append("\'");
	}
	if (testresult && *testresult) {
		sql.append(",").append("\'").append(testresult).append("\'");
	}
	if (testval && *testval) {
		sql.append(",").append("\'").append(testval).append("\'");
	}
	if (testlowlimit && *testlowlimit) {
		sql.append(",").append("\'").append(testlowlimit).append("\'");
	}
	if (testhightlimit && *testhightlimit) {
		sql.append(",").append("\'").append(testhightlimit).append("\'");
	}
	if (ismain && *ismain) {
		sql.append(",").append("\'").append(ismain).append("\'");
	}
	if (testtime && *testtime) {
		sql.append(",").append("\'").append(testtime).append("\'");
	}

	sql.append(");");

	exec_update_sql();

}
xmlrpc_int32 mysqlx_session::query_tcon_testplan(const char* name, const tcon_testplan_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("l.md5,")
		.append("l.offset,")
		.append("l.size as trunk_size,")
		.append("r.size ")
		.append(" FROM ")
		.append(TBL_TCON_TESTPLAN).append(" AS l ")
		.append("INNER JOIN ").append(TBL_TCON_RECIPE_SIZE).append(" AS r")
		.append(" ON l.name = r.name")
		.append(" WHERE ")
		.append("l.name = ")
		.append("\'").append(name).append("\';");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<unsigned>(), row[2].get<unsigned>(), row[3].get<unsigned>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_tcon_hstvt(const char* md5, const tcon_testhstvt_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT")
		.append(" md5,")
		.append("content ")
		.append(" FROM ")
		.append(TBL_TCON_HSTVT)
		.append(" WHERE ")
		.append("md5 = ")
		.append("UPPER(\'").append(md5).append("\');");;

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_databaseuuid(const char* sn, const testdatabaseuuid_cb_type&cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT")
		.append(" uuid")
		.append(" FROM ")
		.append(TBL_TESTDATABASE)
		.append(" WHERE ")
		.append("sn = ")
		.append("\'").append(sn).append("\' and  TestDate in ")
		.append("(select max(testdate) from ")
		.append(TBL_TESTDATABASE)
		.append(" where sn= ")
		.append("\'").append(sn).append("\') ; ");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::update_testdatabase(const char* uuid, const char* failprjname, const char* failsubprjname)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_TESTDATABASE)
		.append(" SET ")
		.append(" failproject = \'").append(failprjname).append("\', ")
		.append(" failsubproject = \'").append(failsubprjname).append("\'")
		.append(" WHERE ")
		.append(" uuid = ").append("UPPER(\'").append(uuid).append("\');");
	L_ERROR("{0}", sql.c_str());

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::query_tcon_upgrade(const char* type, const char* areaname, const char* machine_name, const tcon_query_upgrade_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("areaName,")
		.append("machineName,")
		.append("boardStyle,")
		.append("machineStyleName,")
		.append("halfMaterialNumber,")
		.append("stat")
		.append(" FROM ")
		.append(TBL_TCON_UPGRADE)
		.append(" WHERE ")
		.append("typeName=").append("\'").append(type).append("\'")
		.append(" AND ")
		.append("areaName=").append("\'").append(areaname).append("\'")
		.append(" AND ")
		.append("machineName=").append("\'").append(machine_name).append("\';");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<std::string>(), row[5].get<int>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::updata_tcon_upgrade(const char* type, const char* areaname, const char* machine_name, const char* board_style, const char* machine_style_name, const char* half_material_no, int stat)
{
	lock_it();
	std::string sql;

	sql.append("CALL tv4th.tcon_check_upgrade(")
		.append("\'").append(type).append("\',")
		.append("\'").append(areaname).append("\',")
		.append("\'").append(machine_name).append("\',")
		.append("\'").append(board_style).append("\',")
		.append("\'").append(machine_style_name).append("\',")
		.append("\'").append(half_material_no).append("\',")
		.append(std::to_string(stat))
		.append(");");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::updata_tcon_upgrade_machine(const char* type, const char* areaname, const char* machine_name, int stat)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_TCON_UPGRADE)
		.append(" SET ")
		.append("stat = ")
		.append(std::to_string(stat))
		.append(" WHERE ")
		.append("typeName=").append("\'").append(type).append("\'")
		.append(" AND ")
		.append("areaName=").append("\'").append(areaname).append("\'")
		.append(" AND ")
		.append("machineName=").append("\'").append(machine_name).append("\';");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::check_tcon_hstvt_trunk(const std::vector<std::string>& md5, const tcon_check_hstvt_trunk_cb& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("md5")
		.append(" FROM ")
		.append(TBL_TCON_HSTVT)
		.append(" WHERE ")
		.append("md5")
		.append(" IN ");
	sql.append("(");
	for (const auto& m : md5) {
		sql.append("\'")
			.append(m)
			.append("\'")
			.append(",");
	}
	sql.pop_back();
	sql.append(");");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>());
	}
	exec_query_sql_end();
}
xmlrpc_int32 mysqlx_session::check_tv_hstvt_trunk(const std::vector<std::string>& md5, const tcon_check_hstvt_trunk_cb& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("md5")
		.append(" FROM ")
		.append(TBL_TV_HSTVT)
		.append(" WHERE ")
		.append("md5")
		.append(" IN ");
	sql.append("(");
	for (const auto& m : md5) {
		sql.append("\'")
			.append(m)
			.append("\'")
			.append(",");
	}
	sql.pop_back();
	sql.append(");");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::check_tcon_testplan_exist(const char* boardStyle, const char* machineStyleName, const char* halfMaterialNumber)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("size")
		.append(" FROM ")
		.append(TBL_TCON_RECIPE_SIZE)
		.append(" WHERE ")
		.append("name = ").append("\'");
	if (boardStyle && *boardStyle) {
		sql.append(boardStyle).append("-");
	}
	if (machineStyleName && *machineStyleName) {
		sql.append(machineStyleName).append("-");
	}
	if (halfMaterialNumber && *halfMaterialNumber) {
		sql.append(halfMaterialNumber).append("-");
	}
	sql.pop_back();
	sql.append("\'")
		.append(";");

	try {
		L_INFO("{0}", sql);
		mysqlx::SqlResult result = tv4th_session_->sql(sql).execute();
		xmlrpc_int32 count = result.count();
		if (count != 1) {
			return 0;
		}

		auto row = result.fetchOne();
		int size = row[0].get<int>();
		return size;
	}
	catch (...) {
		return EC_DB_ERROR;
	}
	return EC_DB_ERROR;
}

xmlrpc_int32 mysqlx_session::query_databaselist(const char* starttime, const char* endtime, const char* sn, const char* testplan, 
	const char* failprjname, const char* failsubprjname,const char* testip, const char* pcname, const char* testresult, 
	const char* areaname, const char* deviceno, const char* toolno,
	const testdatabaselist_cb_type& cb)
{
	lock_it();
	std::string sql;

	sql.append("SELECT ")
		.append("c.uuid,")
		.append("c.sn,")
		.append("c.mac,")
		.append("c.softwareversion,")
		.append("c.keyname,")
		.append("c.testresult,")
		.append("c.testplanname,")
		.append("c.modulebom,")
		.append("c.areaname,")
		.append("c.deviceno,")
		.append("c.toolno,")
		.append("c.failproject,")
		.append("c.failsubproject,")
		.append("c.testip,")
		.append("c.pcname,")
		.append("c.testtime,")
		.append("unix_timestamp(c.testdate),")
		.append("c.testperson")
		.append(" FROM ")
		.append(TBL_TESTDATABASE).append(" AS c")
		.append(" WHERE 1=1  ");
	if (starttime&& *starttime) 
	 {
		sql.append("and testdate>=").append("\'").append(starttime).append("\'  ");
	 }
	if (endtime&& *endtime)
	{
		sql.append("and testdate<=").append("\'").append(endtime).append("\'  ");
	}
	if (sn&& *sn)
	{
		sql.append("and sn like").append("\'%%").append(sn).append("%%\'  ");
	}
	if (testplan&& *testplan)
	{
		sql.append("and testplan like").append("\'%%").append(testplan).append("%%\'  ");
	}
	if (failprjname&& *failprjname)
	{
		sql.append("and failprjname like").append("\'%%").append(failprjname).append("%%\'  ");
	}
	if (failsubprjname&& *failsubprjname)
	{
		sql.append("and failsubprjname like").append("\'%%").append(failsubprjname).append("%%\'  ");
	}
	if (testip&& *testip)
	{
		sql.append("and testip like").append("\'%%").append(testip).append("%%\'  ");
	}
	if (pcname&& *pcname)
	{
		sql.append("and pcname =").append("\'").append(pcname).append("\'  ");
	}
	if (testresult&& *testresult)
	{
		sql.append("and testresult =").append("\'").append(testresult).append("\'  ");
	}
	if (areaname&& *areaname)
	{
		sql.append("and areaname =").append("\'").append(areaname).append("\'  ");
	}
	if (deviceno&& *deviceno)
	{
		sql.append("and deviceno =").append("\'").append(deviceno).append("\'  ");
	}
	if (toolno&& *toolno)
	{
		sql.append("and toolno =").append("\'").append(toolno).append("\'  ");
	}
	exec_query_sql_begin();
	xmlrpc_int32 count1 = result.count();
	for (const auto& row : result) {
		int size = (int)row.colCount();
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(),
			row[4].get<std::string>(), row[5].get<std::string>(), row[6].get<std::string>(), row[7].get<std::string>(), 
			row[8].get<std::string>(), row[9].get<std::string>(), row[10].get<std::string>(), row[11].get<std::string>(), 
			row[12].get<std::string>(), row[13].get<std::string>(), row[14].get<std::string>(), row[15].get<std::string>(), 
			row[16].get<time_t>(), row[17].isNull() ? "" : row[17].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::new_testplan(const char* project, const char* board, const char* bom)
{
	lock_it();
	std::string sql;
	sql.append("REPLACE INTO ")
		.append(TBL_TESTPLAN)
		.append("(")
		.append("uuid,")
		.append("project,")
		.append("board,")
		.append("bom")
		.append(") VALUES(")
		.append("UPPER(").append("UUID()").append("),")
		.append("\'").append(project).append("\',")
		.append("\'").append(board).append("\',")
		.append("\'").append(bom).append("\'")
		.append(");");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::update_testplan_size(const char* project, const char* board, const char* bom, int size)
{
	lock_it();
	std::string uuid;
	int ec;
	ec = testplan_uuid(project, board, bom, uuid);
	if (ec < 0 || uuid.empty()) {
		return ec;
	}

	// 把老的都清掉，更新起来麻烦
	ec = testplan_clean(uuid);
	if (ec < 0) {
		return ec;
	}

	std::string sql;
	sql.append("REPLACE INTO ")
		.append(TBL_TESTPLAN_SIZE)
		.append("(")
		.append("uuid,")
		.append("size")
		.append(") VALUES(")
		.append("\'").append(uuid).append("\',")
		.append(std::to_string(size))
		.append(");");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::update_testplan_trunk(const char* project, const char* board, const char* bom, const char* md5, const char* ctx, int offset, int size, const char* author)
{
	lock_it();
	std::string uuid;

	int ec;

	ec = testplan_uuid(project, board, bom, uuid);
	if (ec < 0 || uuid.empty()) {
		return ec;
	}
	
	ec = testplan_ln_insert(uuid, md5, offset, size, author);
	if (ec < 0) {
		return ec;
	}
	return testplan_trunk_insert(md5, ctx);
}

xmlrpc_int32 mysqlx_session::query_testplan_trunk(const char* project, const char* board, const char* bom, const testplan_trunk_cb_type& cb)
{
	lock_it();
	std::string uuid;

	int ec;

	ec = testplan_uuid(project, board, bom, uuid);
	if (ec < 0 || uuid.empty()) {
		return ec;
	}

	std::string sql;
	sql.append("SELECT ")
		.append("l.md5,")
		.append("l.offset,")
		.append("l.size as trunk_size,")
		.append("r.size as size")
		.append(" FROM ")
		.append(TBL_TESTPLAN_LN).append(" AS l ")
		.append("INNER JOIN ")
		.append(TBL_TESTPLAN_SIZE).append(" AS r")
		.append(" ON l.uuid = r.uuid")
		.append(" WHERE ")
		.append("l.uuid = ")
		.append("\'").append(uuid).append("\';");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<unsigned>(), row[2].get<unsigned>(), row[3].get<unsigned>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::test_json_insert(const char* boardtype, const char* halfmaterialnum, const char* areaname,const char* machinename,const char* username,const char* testdata)
{
	lock_it();
	std::string sql;
	sql.append("SELECT COUNT(*) FROM ")
		.append(TBL_TEST_JSON);
	int uuid;
	L_INFO("test_json_insert is called!");
	exec_query_sql_begin();
	if (count == 0)
	{
		uuid = 1;
		time_t now_time = time(NULL);
		tm* t_tm = localtime(&now_time);
		sql = "";
		sql.append("INSERT INTO ")
			.append(TBL_TEST_JSON)
			.append(" (")
			.append("uuid,")
			.append("board_type,")
			.append("halfmaterial_num,")
			.append("area_name,")
			.append("machine_name,")
			.append("user_name,")
			.append("test_data")
			.append(") ")
			.append("VALUES(")
			.append(std::to_string(uuid))
			.append(",")
			.append("'")
			.append(boardtype)
			.append("'")
			.append(",")
			.append("'")
			.append(halfmaterialnum)
			.append("'")
			.append(",")
			.append("'")
			.append(areaname)
			.append("'")
			.append(",")
			.append("'")
			.append(machinename)
			.append("'")
			.append(",")
			.append("'")
			.append(username)
			.append("'")
			.append(",")
			.append("'")
			.append(testdata)
			.append("'")
			.append(")");
		exec_query_sql_begin();
		exec_query_sql_end();
	}
	else
	{
		sql = "";
		sql.append("SELECT MAX(uuid) FROM ")
			.append(TBL_TEST_JSON);
		exec_query_sql_begin();
		for (const auto& row : result) {
			uuid = row[0].get<int>();
		}
		L_INFO("the uuid value is: {0}", uuid);
		uuid += 1;
		time_t now_time = time(NULL);
		tm* t_tm = localtime(&now_time);
		sql = "";
		sql.append("INSERT INTO ")
			.append(TBL_TEST_JSON)
			.append(" (")
			.append("uuid,")
			.append("board_type,")
			.append("halfmaterial_num,")
			.append("area_name,")
			.append("machine_name,")
			.append("user_name,")
			.append("test_data")
			.append(") ")
			.append("VALUES(")
			.append(std::to_string(uuid))
			.append(",")
			.append("'")
			.append(boardtype)
			.append("'")
			.append(",")
			.append("'")
			.append(halfmaterialnum)
			.append("'")
			.append(",")
			.append("'")
			.append(areaname)
			.append("'")
			.append(",")
			.append("'")
			.append(machinename)
			.append("'")
			.append(",")
			.append("'")
			.append(username)
			.append("'")
			.append(",")
			.append("'")
			.append(testdata)
			.append("'")
			.append(")");
		//L_INFO("{0}",sql);
		exec_query_sql_begin();
		exec_query_sql_end();
		exec_query_sql_end();
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_test_config_data(const char * boardtype, 
	const char * halfmaterialnum, 
	bool displayhistoryeditflag,const test_config_data_cb_type& cb)
{
	lock_it();
	std::string sql;
	if (displayhistoryeditflag)
	{ 
		if ((boardtype && *boardtype != '\0') && (halfmaterialnum && *halfmaterialnum != '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON)
				.append(" WHERE board_type LIKE '%")
				.append(boardtype)
				.append("%'")
				.append("AND halfmaterial_num LIKE '%")
				.append(halfmaterialnum)
				.append("%'");
		}

		if ((boardtype && *boardtype != '\0') && (halfmaterialnum && *halfmaterialnum == '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON)
				.append(" WHERE board_type LIKE '%")
				.append(boardtype)
				.append("%'");
		}

		if ((boardtype && *boardtype == '\0') && (halfmaterialnum && *halfmaterialnum != '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON)
				.append(" WHERE halfmaterial_num LIKE '%")
				.append(halfmaterialnum)
				.append("%'");
		}

		if ((boardtype && *boardtype == '\0') && (halfmaterialnum && *halfmaterialnum == '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON);
		}

		L_INFO("{0}",sql);
		exec_query_sql_begin();
		for (const auto& row : result) {
			int uuid = row[0].get<int>();
			time_t tst = row[4].get<time_t>();
			std::stringstream ss;
			ss << std::put_time(std::localtime(&tst),"%F %X");
			cb(row[0].get<int>(),
				row[5].get<std::string>(), 
				row[6].get<std::string>(),
				row[1].get<std::string>(), 
				row[2].get<std::string>(), 
				row[3].get<std::string>(),
				ss.str());
		}
		exec_query_sql_end();
	}
	else
	{
		if ((boardtype && *boardtype != '\0') && (halfmaterialnum && *halfmaterialnum != '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS aa WHERE edit_time=(SELECT MAX(bb.edit_time) FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS bb WHERE aa.board_type = bb.board_type AND aa.halfmaterial_num = bb.halfmaterial_num)")
				.append(" AND board_type LIKE '%")
				.append(boardtype)
				.append("%'")
				.append("AND halfmaterial_num LIKE '%")
				.append(halfmaterialnum)
				.append("%'");
		}

		if ((boardtype && *boardtype != '\0') && (halfmaterialnum && *halfmaterialnum == '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS aa WHERE edit_time=(SELECT MAX(bb.edit_time) FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS bb WHERE aa.board_type = bb.board_type)")
				.append(" AND board_type LIKE '%")
				.append(boardtype)
				.append("%'");
		}

		if ((boardtype && *boardtype == '\0') && (halfmaterialnum && *halfmaterialnum != '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS aa WHERE edit_time=(SELECT MAX(bb.edit_time) FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS bb WHERE aa.halfmaterial_num = bb.halfmaterial_num)")
				.append(" AND halfmaterial_num LIKE '%")
				.append(halfmaterialnum)
				.append("%'");
		}

		if ((boardtype && *boardtype == '\0') && (halfmaterialnum && *halfmaterialnum == '\0'))
		{
			sql.append("SELECT uuid,area_name,machine_name,user_name,unix_timestamp(edit_time),board_type,halfmaterial_num FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS aa WHERE edit_time=(SELECT MAX(bb.edit_time) FROM ")
				.append(TBL_TEST_JSON)
				.append(" AS bb WHERE aa.board_type = bb.board_type AND aa.halfmaterial_num = bb.halfmaterial_num)")
				.append(" AND board_type LIKE '%%'")
				.append(" AND halfmaterial_num LIKE '%%'");
		}
		L_INFO("{0}", sql);
		exec_query_sql_begin();
		for (const auto& row : result) {
			int uuid = row[0].get<int>();
			time_t tst = row[4].get<time_t>();
			std::stringstream ss;
			ss << std::put_time(std::localtime(&tst), "%F %X");
			cb(row[0].get<int>(), 
				row[5].get<std::string>(),
				row[6].get<std::string>(),
				row[1].get<std::string>(),
				row[2].get<std::string>(),
				row[3].get<std::string>(),
				ss.str());
		}
		exec_query_sql_end();
	}
}

xmlrpc_int32 mysqlx_session::get_test_details(const char * uuid, const test_details_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT test_data FROM ")
		.append(TBL_TEST_JSON)
		.append(" WHERE uuid = ")
		.append(uuid);
	L_INFO("{0}", sql);
	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::testplan_uuid(const char* project, const char* board, const char* bom, std::string& uuid)
{
	std::string sql;
	sql.append("SELECT ")
		.append("uuid ")
		.append("FROM ")
		.append(TBL_TESTPLAN)
		.append(" WHERE ")
		.append("project = \'").append(project).append("\'")
		.append(" AND ")
		.append("board = \'").append(board).append("\'")
		.append(" AND ")
		.append("bom = \'").append(bom).append("\'")
		.append(";");

	exec_query_sql_begin();
	if (result.count() != 1) {
		return -1;
	}
	const auto r = result.fetchOne();
	uuid = r[0].get<std::string>();
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::testplan_clean(const std::string& uuid)
{
	std::string sql;
	sql.append("DELETE FROM ")
		.append(TBL_TESTPLAN_LN)
		.append(" WHERE ")
		.append("uuid = ").append("\'").append(uuid).append("\';");
	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::testplan_trunk_insert(const char* md5, const char* content)
{
	std::string sql;
	sql.append("INSERT INTO ")
		.append(TBL_TCON_HSTVT)
		.append("(")
		.append("md5,")
		.append("content")
		.append(") ")
		.append("VALUES(")
		.append("\'").append(md5).append("\'")
		.append(",")
		.append("\'").append(content).append("\'")
		.append(")")
		.append(";");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::testplan_ln_insert(const std::string& uuid, const char* md5, int offset, int size, const char* author)
{
	std::string sql;
	sql.append("INSERT INTO ")
		.append(TBL_TESTPLAN_LN)
		.append("(")
		.append("uuid,")
		.append("md5,")
		.append("offset,")
		.append("size,")
		.append("creator")
		.append(") ")
		.append("VALUES(")
		.append("\'").append(uuid).append("\'")
		.append(",")
		.append("\'").append(md5).append("\'")
		.append(",")
		.append(std::to_string(offset))
		.append(",")
		.append(std::to_string(size))
		.append(",")
		.append("\'").append(author).append("\'")
		.append(")")
		.append(";");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::add_template(const char* project, const char* templatetype, const char* creator)
{
	lock_it();
	std::string sql;
	sql.append("REPLACE INTO ")
		.append(TBL_TEMPLATE)
		.append("(")
		.append("uuid,")
		.append("project,")
		.append("templatetype,")
		.append("creator")
		.append(") VALUES(")
		.append("UPPER(").append("UUID()").append("),")
		.append("\'").append(project).append("\',")
		.append("\'").append(templatetype).append("\',")
		.append("\'").append(creator).append("\'")
		.append(");");

	exec_update_sql();
}

xmlrpc_int32 mysqlx_session::query_template(const char* project, const template_cb& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT templatetype FROM ")
				.append(TBL_TEMPLATE)
				.append(" WHERE project = '")
				.append(project)
				.append("'");
		exec_query_sql_begin();
		for (const auto& row : result) {
			cb(row[0].get<std::string>());
		}
		exec_query_sql_end();

}
xmlrpc_int32 mysqlx_session::test_json_insert_tv(const char* recipe_name, const char* areaname, const char* machinename, const char* project, const char* templatetype, const char* shortbom, const char* modulebom, const char* materialno, const char* username, const char* testdata)

{
	lock_it();
	std::string sql;
	L_INFO("test_json_insert_tv is called!");
	sql.append("SELECT recipe_name FROM ")
		.append(TBL_TEST_EDITRECORD)
	    .append(" WHERE recipe_name=\'")
		.append(recipe_name)
		.append("\' ");
	exec_query_sql_begin();
	if (count == 1)//将这条数据复制到历史表，并将数据从该表中删除
	{
		sql = "";
		sql.append("INSERT INTO ")
		   .append(TBL_TEST_EDITRECORDHIS)
		   .append(" (SELECT * FROM ")
		   .append(TBL_TEST_EDITRECORD)
		   .append(" WHERE recipe_name=\'")
			.append(recipe_name)
			.append("\') ");
		try {
			tv4th_session_->sql(sql).execute();
			L_TRACE(sql.c_str());
		}
		catch (...) {
		}
		sql = "";
		sql.append("DELETE FROM ")
			.append(TBL_TEST_EDITRECORD)
			.append(" WHERE recipe_name=\'")
			.append(recipe_name)
			.append("\' ");
		try {
			tv4th_session_->sql(sql).execute();
			L_TRACE(sql.c_str());
		}
		catch (...) {
		}
	}
	sql = "";
	sql.append("INSERT INTO ")
		.append(TBL_TEST_EDITRECORD)
		.append(" (")
		.append("uuid,")
		.append("recipe_name,")
		.append("area_name,")
		.append("machine_name,")
		.append("project,")
		.append("templatetype,")
		.append("shortbom,")
		.append("modulebom,")
		.append("materialno,")
		.append("user_name,")
		.append("test_data")
		.append(") ")
		.append("VALUES(")
		.append("UPPER(").append("UUID()").append("),")
		.append("\'").append(recipe_name).append("\',")
		.append("\'").append(areaname).append("\',")
		.append("\'").append(machinename).append("\',")
		.append("\'").append(project).append("\',")
		.append("\'").append(templatetype).append("\',")
		.append("\'").append(shortbom).append("\',")
		.append("\'").append(modulebom).append("\',")
		.append("\'").append(materialno).append("\',")
		.append("\'").append(username).append("\',")
		.append("\'").append(testdata).append("\'")
		.append(");");
	exec_update_sql();
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::query_test_config_data_tv(const char* areaname, const char* machinename, const char* project, const char* templatetype,
	const char* shortbom, const char* materialno,const tv_config_data_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT uuid,recipe_name,area_name,machine_name,project,templatetype,shortbom,modulebom,materialno,test_data,user_name,unix_timestamp(edit_time) FROM ")
		.append(TBL_TEST_EDITRECORD)
		.append(" where 1=1 ");
	if (areaname && *areaname != '\0')//线体
	{
		sql.append(" and area_name='")
			.append(areaname)
			.append("'");
	}
	if (machinename && *machinename != '\0')//机台
	{
		sql.append(" and machine_name='")
			.append(machinename)
			.append("'");
	}
	if (project && *project != '\0')//方案
	{
		sql.append(" and project='")
			.append(project)
			.append("'");
	}
	if (templatetype&& *templatetype != '\0')//板型
	{
		sql.append(" and templatetype like'%")
			.append(templatetype)
			.append("%'");
	}
	if (shortbom&& *shortbom != '\0')//机型短码
	{
		sql.append(" and shortbom like'%")
			.append(shortbom)
			.append("%'");
	}
	if (materialno&& *materialno != '\0')//机型短码
	{
		sql.append(" and materialno like'%")
			.append(materialno)
			.append("%'");
	}	
	//sql.append(" ORDER BY edit_time DESC LIMIT 1 ");
	L_INFO("{0}", sql);
    exec_query_sql_begin();
	for (const auto& row : result) {
		time_t tst = row[11].get<time_t>();
		std::stringstream ss;
		ss << std::put_time(std::localtime(&tst), "%F %X");
		cb(row[0].get<std::string>(),
			row[1].get<std::string>(),
			row[2].get<std::string>(),
			row[3].get<std::string>(),
			row[4].get<std::string>(),
			row[5].get<std::string>(),
			row[6].get<std::string>(),
			row[7].get<std::string>(),
			row[8].get<std::string>(),
			row[9].get<std::string>(),
			row[10].get<std::string>(),
			ss.str());
	}
	exec_query_sql_end();
	
}
xmlrpc_int32 mysqlx_session::query_tv_testplan(const char* name, const tv_testplan_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("l.md5,")
		.append("l.offset,")
		.append("l.size as trunk_size,")
		.append("r.size ")
		.append(" FROM ")
		.append(TBL_TV_TESTPLAN).append(" AS l ")
		.append("INNER JOIN ").append(TBL_TV_RECIPE_SIZE).append(" AS r")
		.append(" ON l.name = r.name")
		.append(" WHERE ")
		.append("l.name = ")
		.append("\'").append(name).append("\';");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<unsigned>(), row[2].get<unsigned>(), row[3].get<unsigned>());
	}
	exec_query_sql_end();
}
xmlrpc_int32 mysqlx_session::query_tv_hstvt(const char* md5, const tv_testhstvt_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT")
		.append(" md5,")
		.append("content ")
		.append(" FROM ")
		.append(TBL_TV_HSTVT)
		.append(" WHERE ")
		.append("md5 = ")
		.append("UPPER(\'").append(md5).append("\');");;

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>());
	}
	exec_query_sql_end();
}
xmlrpc_int32 mysqlx_session::check_tv_testplan_exist(const char* templatetype, const char* modulebom, const char* materialno, const char* areaname, const char* machine_name)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("size")
		.append(" FROM ")
		.append(TBL_TV_RECIPE_SIZE)
		.append(" WHERE ");
	const char* filename;

	//if (areaname && *areaname&&machine_name && *machine_name)
	if (areaname && *areaname)
	{

		char str1[20] = { 0 };
		char str2[20] = { 0 };
		char str3[20] = { 0 };
		char str4[20] = { 0 };
		char str5[20] = {0} ;
		strcpy(str1, templatetype);
		strcpy(str2, modulebom);
		strcpy(str3, materialno);
		strcpy(str4, areaname);
		strcpy(str5, machine_name);
		char newStr[100] = "";
		strcat(newStr, str1);
		strcat(newStr, "-"); 
		strcat(newStr, str2);    
		strcat(newStr, "-"); 
		strcat(newStr, str3); 
		strcat(newStr, "-"); 
		strcat(newStr, str4);
		/*strcat(newStr, "-");
		strcat(newStr, str5);   */
		filename = newStr;
		sql.append("name = ").append("\'")
			.append(filename)
			/*.append("-")
            .append(machine_name)*/
			.append("\'");

	}
	else
	{
		char str1[20] = {0} ;
		char str2[20] = {0} ;
		char str3[20] = {0} ;
		strcpy(str1, templatetype);
		strcpy(str2, modulebom);
		strcpy(str3, materialno);
		char newStr[100] = "";
		strcat(newStr, str1);
		strcat(newStr, "-");
		strcat(newStr, str2);
		strcat(newStr, "-");
		strcat(newStr, str3);	
		filename = newStr;
		
		sql.append("name like ").append("\'%")
			.append(filename)
			.append("%\'");
	}
	sql.pop_back();
	sql.append("\'")
		.append(";");

	try {
		L_INFO("{0}", sql);
		mysqlx::SqlResult result = tv4th_session_->sql(sql).execute();
		xmlrpc_int32 count = result.count();
		if (count != 1) {
			return 0;
		}

		auto row = result.fetchOne();
		int size = row[0].get<int>();
		return size;
	}
	catch (...) {
		return EC_DB_ERROR;
	}
	return EC_DB_ERROR;
}
xmlrpc_int32 mysqlx_session::updata_tv_upgrade(const char* type, const char* areaname, const char* machine_name, const char* templatetype, const char* modulebom_sytle, const char* materialNo, int stat)
{
	lock_it();
	std::string sql;

	sql.append("CALL tv4th.tv_check_upgrade(")
		.append("\'").append(type).append("\',")
		.append("\'").append(areaname).append("\',")
		.append("\'").append(machine_name).append("\',")
		.append("\'").append(templatetype).append("\',")
		.append("\'").append(modulebom_sytle).append("\',")
		.append("\'").append(materialNo).append("\',")
		.append(std::to_string(stat))
		.append(");");

	exec_update_sql();
}
xmlrpc_int32 mysqlx_session::query_tv_upgrade(const char* type, const char* areaname, const char* machine_name, const tv_query_upgrade_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("areaName,")
		.append("machineName,")
		.append("templateType,")
		.append("moduleBom,")
		.append("materialNo,")
		.append("stat")
		.append(" FROM ")
		.append(TBL_TV_UPGRADE)
		.append(" WHERE ")
		.append("typeName=").append("\'").append(type).append("\'")
		.append(" AND ")
		.append("areaName=").append("\'").append(areaname).append("\'")
		.append(" AND ")
		.append("machineName=").append("\'").append(machine_name).append("\';");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>(), row[1].get<std::string>(), row[2].get<std::string>(), row[3].get<std::string>(), row[4].get<std::string>(), row[5].get<int>());
	}
	exec_query_sql_end();
}
xmlrpc_int32 mysqlx_session::update_tv_upgrade_machine(const char* type, const char* areaname, const char* machine_name, int stat)
{
	lock_it();
	std::string sql;
	sql.append("UPDATE ")
		.append(TBL_TV_UPGRADE)
		.append(" SET ")
		.append("stat = ")
		.append(std::to_string(stat))
		.append(" WHERE ")
		.append("typeName=").append("\'").append(type).append("\'")
		.append(" AND ")
		.append("areaName=").append("\'").append(areaname).append("\'")
		.append(" AND ")
		.append("machineName=").append("\'").append(machine_name).append("\';");

	exec_update_sql();


}
xmlrpc_int32 mysqlx_session::check_filename(const char* filename, const filename_cb& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ")
		.append("name ")
		.append(" FROM ")
		.append(TBL_TV_TESTPLAN)
		.append(" WHERE ")
		.append("name like ")
		.append("\'%").append(filename).append("\%' LIMIT 0,1;");

	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(row[0].get<std::string>());
	}
	exec_query_sql_end();
}

xmlrpc_int32 mysqlx_session::add_cor(const char* acBordType, const char* programno, const char* remarks, const char* editor)
{
	lock_it();
	std::string sql;
	sql.append("SELECT COUNT(*) FROM ")
		.append(TBL_TV_COR_Table)
		.append(" WHERE AcquisitionBoardType='")
		.append(acBordType)
		.append("' AND ProgramNo='")
		.append(programno)
		.append("'");
	L_INFO("add_cor is called!");
	exec_query_sql_begin();
	int count1;
	const auto r = result.fetchOne();
	count1 = r[0].get<int>();
	if (count1 >0)
	{
		sql.clear();
		sql.append("INSERT INTO ")
			.append(TBL_TV_COR_Table_His)
			.append(" SELECT * FROM ")
			.append(TBL_TV_COR_Table)
			.append(" WHERE AcquisitionBoardType='")
			.append(acBordType)
			.append("' AND ProgramNo='")
			.append(programno)
			.append("'")
			.append(";");
		try {
			tv4th_session_->sql(sql).execute();
			L_TRACE(sql.c_str());
		}
		catch (...) {
		}

		sql = "";
		sql.append("DELETE FROM ")
			.append(TBL_TV_COR_Table)
			.append(" WHERE AcquisitionBoardType='")
			.append(acBordType)
			.append("' AND ProgramNo='")
			.append(programno)
			.append("'")
			.append(";");
		try {
			tv4th_session_->sql(sql).execute();
			L_TRACE(sql.c_str());
		}
		catch (...) {
		}
	}

	sql.clear();
	sql.append("INSERT INTO ")
		.append(TBL_TV_COR_Table)
		.append("( uuid,");
	if (acBordType && *acBordType) {
		sql.append("AcquisitionBoardType,");
	}
	if (programno && *programno) {
		sql.append("ProgramNo,");
	}
	if (remarks && *remarks) {
		sql.append("Remarks,");
	}
	if (editor && *editor) {
		sql.append("Editor");
	}
	sql.append(")");
	sql.append(" VALUES(")
		.append("UPPER(UUID()),");
	if (acBordType && *acBordType) {
		sql.append("\'").append(acBordType).append("\',");
	}
	if (programno && *programno) {
		sql.append("\'").append(programno).append("\',");
	}
	if (remarks && *remarks) {
		sql.append("\'").append(remarks).append("\',");
	}
	if (editor && *editor) {
		sql.append("\'").append(editor).append("\'");
	}
	sql.append(");");

	exec_update_sql();

	exec_query_sql_end();



	
}
xmlrpc_int32 mysqlx_session::query_cordata(const char* boardtype, const cor_cb_type& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT ProgramNo,Remarks,Editor,unix_timestamp(Edit_time) FROM ")
		.append(TBL_TV_COR_Table)
		.append(" WHERE AcquisitionBoardType = '")
		.append(boardtype)
		.append("'");

		L_INFO("{0}", sql);
		exec_query_sql_begin();
		for (const auto& row : result) {
			time_t tst = row[3].get<time_t>();
			std::stringstream ss;
			ss << std::put_time(std::localtime(&tst), "%F %X");
			cb(
				row[0].get<std::string>(),
				row[1].get<std::string>(),
				row[2].get<std::string>(),
				ss.str());
		}
		exec_query_sql_end();

}
xmlrpc_int32 mysqlx_session::query_distinctacType(const cb_actype& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT DISTINCT AcquisitionBoardType FROM ")
		.append(TBL_TV_COR_Table);

	L_INFO("{0}", sql);
	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(
			row[0].get<std::string>());
	}
	exec_query_sql_end();

}
xmlrpc_int32 mysqlx_session::query_Remark(const char* acboardType, const char* programno, const cb_remark& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT Remarks,uuid FROM ")
		.append(TBL_TV_COR_Table)
		.append(" WHERE AcquisitionBoardType = '")
		.append(acboardType)
		.append("'  and ProgramNo='")
		.append(programno)
		.append("'");

	L_INFO("{0}", sql);
	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(
			row[0].get<std::string>(),
			row[1].get<std::string>());
	}
	exec_query_sql_end();
}
xmlrpc_int32 mysqlx_session::add_platetype(const char* BoardType/**主板型号*/, const char* moduleNo/**组件号*/, const char* uuidstr/**uuid*/, const char* editor/**操作员*/)
{
	lock_it();
	std::string sql;
	sql.append("SELECT COUNT(*) FROM ")
		.append(TBL_TV_Plate_Table)
		.append(" WHERE BoardType='")
		.append(BoardType)
		.append("' AND ModuleNo='")
		.append(moduleNo)
		.append("'");
	L_INFO("add_platetype is called!");
	exec_query_sql_begin();
	int count1;
	const auto r = result.fetchOne();
	count1 = r[0].get<int>();
	if (count1 > 0)
	{
		sql.clear();
		sql.append("INSERT INTO ")
			.append(TBL_TV_Plate_Table_His)
			.append(" SELECT * FROM ")
			.append(TBL_TV_Plate_Table)
			.append(" WHERE BoardType='")
			.append(BoardType)
			.append("' AND ModuleNo='")
			.append(moduleNo)
			.append("'")
			.append(";");
		try {
			tv4th_session_->sql(sql).execute();
			L_TRACE(sql.c_str());
		}
		catch (...) {
		}

		sql = "";
		sql.append("DELETE FROM ")
			.append(TBL_TV_Plate_Table)
			.append(" WHERE BoardType='")
			.append(BoardType)
			.append("' AND ModuleNo='")
			.append(moduleNo)
			.append("'")
			.append(";");
		try {
			tv4th_session_->sql(sql).execute();
			L_TRACE(sql.c_str());
		}
		catch (...) {
		}
	}

	sql.clear();
	sql.append("INSERT INTO ")
		.append(TBL_TV_Plate_Table)
		.append("(");
	if (BoardType && *BoardType) {
		sql.append(" BoardType,");
	}
	if (moduleNo && *moduleNo) {
		sql.append("moduleNo,");
	}
	if (uuidstr && *uuidstr) {
		sql.append("uuid,");
	}
	if (editor && *editor) {
		sql.append("Editor");
	}
	sql.append(")");
	sql.append(" VALUES(");
	if (BoardType && *BoardType) {
		sql.append("\'").append(BoardType).append("\',");
	}
	if (moduleNo && *moduleNo) {
		sql.append("\'").append(moduleNo).append("\',");
	}
	if (uuidstr && *uuidstr) {
		sql.append("\'").append(uuidstr).append("\',");
	}
	if (editor && *editor) {
		sql.append("\'").append(editor).append("\'");
	}
	sql.append(");");

	exec_update_sql();

	exec_query_sql_end();




}
xmlrpc_int32 mysqlx_session::query_distinctpcbtype(const cb_pcbtype& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT DISTINCT BoardType FROM ")
		.append(TBL_TV_Plate_Table);

	L_INFO("{0}", sql);
	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(
			row[0].get<std::string>());
	}
	exec_query_sql_end();
}
xmlrpc_int32 mysqlx_session::query_moduleNo(const char* baordtype, const cb_moduleno& cb)
{
	lock_it();
	std::string sql;
	sql.append("SELECT moduleno FROM ")
		.append(TBL_TV_Plate_Table)
		.append(" WHERE BOARDTYPE = '")
		.append(baordtype)
		.append("'");

	L_INFO("{0}", sql);
	exec_query_sql_begin();
	for (const auto& row : result) {
		cb(
			row[0].get<std::string>());
	}
	exec_query_sql_end();
}
