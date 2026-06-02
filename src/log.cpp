#include "log.h"

#include <cstdio>
#include <sec_api/stdio_s.h>
#include <stdio.h>
#include <stdarg.h>
#include <memory.h>

#include "defines.h"
#include "thread.h"

#define TERM_COLOR_RESET "\033[39m\033[49m"
#define TERM_COLOR_FG(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
#define TERM_COLOR_BG(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m"

namespace log {
	internal const char* LEVEL_COLORS[LEVEL_COUNT] = {
		TERM_COLOR_FG(175, 175, 175),
		TERM_COLOR_FG(255, 255, 255),
		TERM_COLOR_FG(255, 132, 0),
		TERM_COLOR_FG(255, 0,   255),
		TERM_COLOR_FG(255, 0,   0),
		TERM_COLOR_BG(255, 0,   0)
	};

	internal const char* LEVEL_STRINGS[LEVEL_COUNT] = {
		"debug",
		"info",
		"warn",
		"ambiguous",
		"error",
		"fatal"
	};

	typedef struct Message {
		Level level;
		va_list args;
		const char* fmt;
		const char* termCodes;
		const char* name;
		const char* file;
		u32 line;
	} Message;

	// NOTE(zgayford): this is technically not thread safe
	// but its probably fine
	internal Level currentLevel = LEVEL_DEBUG;
	internal FILE* logFile = NULL;
	internal bool initialized = false;

	void init(Level initialLevel, const char* logFilepath) {
		if (initialized) return;
		initialized = true;
		currentLevel = initialLevel;
		if (logFilepath != NULL)
			logFile = fopen(logFilepath, "w");
	}

	void deinit() {
		if (!initialized) return;
		initialized = false;
		fclose(logFile);
	}

	void level_set(Level level) {
		if (!initialized) return;
		currentLevel = level;
	}

	// NOTE(zgayford): there is probably a more intelligent way 
	// of doing this, but this is fine.
	internal void message_print(FILE* stream, Message* message) {
		#define THREAD_NAME_FMT "[%-20.20s]"
		#define MESSAGE_BUFFER_MAX_LENGTH 1024
		// NOTE(zgayford): +1 so there is always null term
		char messageBuffer[MESSAGE_BUFFER_MAX_LENGTH + 1];
		memset(messageBuffer, 0, MESSAGE_BUFFER_MAX_LENGTH + 1);
		vsnprintf(messageBuffer, MESSAGE_BUFFER_MAX_LENGTH, message->fmt, message->args);

		u32 messageConfiguration = 0;
		messageConfiguration |= (message->termCodes != NULL) << 2;
		messageConfiguration |= (message->name      != NULL) << 1;
		messageConfiguration |= (message->file      != NULL) << 0;
		switch (messageConfiguration) {
			case 0b000: // no term codes, no name & no file info
			case 0b001: // no term codes, no name & file info
				fprintf(stream, THREAD_NAME_FMT " %s\n", 
						thread::name_get(), messageBuffer);
				break;
			case 0b010: // no term codes, name & no file info
				fprintf(stream, THREAD_NAME_FMT " %s: %s\n", 
						thread::name_get(), message->name, messageBuffer);
				break;
			case 0b011: // no term codes, name & file info
				fprintf(stream, THREAD_NAME_FMT " %s(%s: %d): %s\n", 
						thread::name_get(), message->name, message->file, message->line, messageBuffer);
				break;
			case 0b100: // term codes, no name & no file info
			case 0b101: // term codes, no name & file info
				fprintf(stream, "%s" THREAD_NAME_FMT " %s" TERM_COLOR_RESET "\n",
						message->termCodes, thread::name_get(), messageBuffer);
				break;
			case 0b110: // term codes, name & no file info
				fprintf(stream, "%s" THREAD_NAME_FMT " %s: %s" TERM_COLOR_RESET "\n",
						message->termCodes, thread::name_get(), message->name, messageBuffer);
				break;
			case 0b111: // term codes, name & file info
				fprintf(stream, "%s" THREAD_NAME_FMT " %s(%s: %d): %s" TERM_COLOR_RESET "\n",
						message->termCodes, thread::name_get(), message->name, message->file, message->line, messageBuffer);
				break;
		}
		#undef THREAD_NAME_FMT
		#undef MESSAGE_BUFFER_MAX_LENGTH
	}

	void _message_log(Level level, const char* fmt, ...) {
		if (!initialized) return;
		__builtin_va_list args;
		va_start(args, fmt);

		if (level < currentLevel) {
			va_end(args);
			return;
		}
		
		FILE* stream = stdout;
		Message message = {
			.level = level,
			.args = args,
			.fmt = fmt,
			.termCodes = LEVEL_COLORS[level],
			.name = NULL,
			.file = NULL,
			.line = 0
		};

		if (level >= LEVEL_ERROR) {
			message.name = LEVEL_STRINGS[level];
			stream = stderr;
		}

		message_print(stream, &message);
		if (logFile != NULL) {
			message.termCodes = NULL;
			message_print(logFile, &message);
		}

		va_end(args);
	}

	void _message_log_with_location(Level level, const char* file, const u32 line, const char* fmt, ...) {
		if (!initialized) return;
		__builtin_va_list args;
		va_start(args, fmt);

		if (level < currentLevel) {
			va_end(args);
			return;
		}

		FILE* stream = stdout;
		Message message = {
			.level = level,
			.args = args,
			.fmt = fmt,
			.termCodes = LEVEL_COLORS[level],
			.name = LEVEL_STRINGS[level],
			.file = file,
			.line = line 
		};

		if (level >= LEVEL_ERROR)
			stream = stderr;

		message_print(stream, &message);
		if (logFile != NULL) {
			message.termCodes = NULL;
			message_print(logFile, &message);
		}

		va_end(args);
	}

	void _assert_failure_log_with_location(const char* file, u32 line, const char* condition, const char* fmt, ...) {
		if (!initialized) return;
		__builtin_va_list args;
		va_start(args, fmt);

		Message message = {
			.level = LEVEL_FATAL,
			.args = args,
			.fmt = fmt,
			.termCodes = LEVEL_COLORS[LEVEL_FATAL],
			.name = "assert",
			.file = file,
			.line = line 
		};

		message_print(stderr, &message);
		if (logFile != NULL) {
			message.termCodes = NULL;
			message_print(logFile, &message);
		}

		va_end(args);
	}
}

#undef TERM_COLOR_RESET
#undef TERM_COLOR_FG
#undef TERM_COLOR_BG
