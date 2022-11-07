#pragma once

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT

#include <memory>

#include "spdlog/spdlog.h"

namespace spdlog
{
void spdlog_init(const char* name, const char* file_name, int lvl, int h = 23, int m = 59, int l = level::level_enum::err);
void spdlog_hex(const unsigned char* buf, int len, level::level_enum lv);
}

std::shared_ptr<spdlog::logger> spd_log_ptr();

#define L_TRACE(fmt__, ...)		spd_log_ptr()->trace(fmt__, __VA_ARGS__);
#define L_DEBUG(fmt__, ...)		spd_log_ptr()->debug(fmt__, __VA_ARGS__);
#define L_INFO(fmt__, ...)		spd_log_ptr()->info(fmt__, __VA_ARGS__);
#define L_WARN(fmt__, ...)		spd_log_ptr()->warn(fmt__, __VA_ARGS__);
#define L_ERROR(fmt__, ...)		spd_log_ptr()->error(fmt__, __VA_ARGS__);
#define L_CRITICAL(fmt__, ...)	spd_log_ptr()->critical(fmt__, __VA_ARGS__);

#define L_HEX(buf__, len__, lev__) spdlog::spdlog_hex((const unsigned char*)(buf__), (int)(len__), (spdlog::level::level_enum)lev__);
#define L_HEX_DEBUG(buf__, len__) spdlog::spdlog_hex((const unsigned char*)(buf__), (int)(len__), spdlog::level::level_enum::debug);

#define L_FUNCTION() L_DEBUG("{0} @ {1}", __FUNCTION__, __LINE__);
