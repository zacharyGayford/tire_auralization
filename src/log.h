#ifndef LOG_H
#define LOG_H

#include "defines.h"

#define log_debug(fmt, ...) log::_message_log(log::LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) log::_message_log(log::LEVEL_INFO, fmt, ##__VA_ARGS__)
#define log_ambiguous(fmt, ...) log::_message_log(log::LEVEL_AMBIGUOUS, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) log::_message_log(log::LEVEL_WARN, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log::_message_log_with_location(log::LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...) log::_message_log_with_location(log::LEVEL_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

namespace log {
	typedef enum Level : u32 {
		LEVEL_DEBUG,
		LEVEL_INFO,
		LEVEL_AMBIGUOUS,
		LEVEL_WARN,
		LEVEL_ERROR,
		LEVEL_FATAL,
		LEVEL_COUNT
	} Level;

	void init(Level initialLevel, const char* logFile);
	void deinit();

	void level_set(Level level);

	// NOTE(zgayford): for internal use only, use "log_*" macros instead
	void _message_log(Level level, const char* fmt, ...);
	void _message_log_with_location(Level level, const char* file, u32 line, const char* fmt, ...);
	void _assert_failure_log_with_location(const char* file, u32 line, const char* condition, const char* fmt, ...);
}

#endif // LOG_H
