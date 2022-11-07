
#include "spdlogwapper.hpp"

#include "spdlog/spdlog.h"

#include <string>

std::shared_ptr<spdlog::logger> spd_log_;

namespace spdlog
{
void spdlog_init(const char* name, const char* file_name, int lvl, int h, int m, int l)
{
	auto console = std::make_shared<sinks::wincolor_stdout_sink_mt>();
	auto daily = std::make_shared<sinks::daily_file_sink_mt>(file_name, h, m);

	spd_log_.reset(new logger(name, { console, daily }));

	spd_log_->set_level((level::level_enum)lvl);
	spd_log_->flush_on((level::level_enum)l);

	set_pattern("[%H:%M:%S %z] [thread %t] %v");
}

void spdlog_hex(const unsigned char* buf, int len, level::level_enum lv)
{
	std::string hex;
	hex.append("\r\n");
	char tmp[16] = { 0 };
	for (int i = 0; i < len; i++) {
		sprintf_s(tmp, "0x%02x", buf[i]);
		hex.append(tmp);
		if (!((i + 1) % 16)) {
			hex.append("    ");
			for (int j = 0; j < 16; j++) {
				unsigned char c = buf[i - 15 + j];
				sprintf_s(tmp, "%c", c);

				hex.append(tmp);
			}
			hex.append("\r\n");
		}
		else {
			hex.append(" ");
		}
	}

	int remain = len % 16;
	if (remain) {
		for (int i = remain; i < 16; i++) {
			hex.append("    ").append(" ");
		}
		hex.append("   ");

		for (int i = 0; i < remain; i++) {
			unsigned char c = buf[len - remain + i];
			sprintf_s(tmp, "%c", c);

			hex.append(tmp);
		}
	}

	switch (lv)
	{
	case level::level_enum::trace:
		L_TRACE(hex.c_str());
		break;
	case level::level_enum::debug:
		L_DEBUG(hex.c_str());
		break;
	case level::level_enum::info:
		L_INFO(hex.c_str());
		break;
	case level::level_enum::warn:
		L_WARN(hex.c_str());
		break;
	case level::level_enum::err:
		L_ERROR(hex.c_str());
		break;
	case level::level_enum::critical:
		L_CRITICAL(hex.c_str());
		break;
	default:
		L_TRACE(hex.c_str());
		break;
	}
}
}

std::shared_ptr<spdlog::logger> spd_log_ptr()
{
	return spd_log_;
}