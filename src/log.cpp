#include "log.h"

#include <stdio.h>
#include <stdarg.h>

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

	void _message_log(Level level, const char* fmt, ...) {
		if (!initialized) return;
		__builtin_va_list args;
		va_start(args, fmt);

		if (level < currentLevel) {
			va_end(args);
			return;
		}

		switch (level) {
			case LEVEL_FATAL:
			case LEVEL_ERROR:
				fprintf(stderr, "%s[%-10.10s] %s:", LEVEL_COLORS[level], thread::name_get(), LEVEL_STRINGS[level]);
				vfprintf(stderr, fmt, args);
				fprintf(stderr, TERM_COLOR_RESET "\n");

				if (logFile != NULL) {
					fprintf(logFile, "[%-10.10s] %s:", thread::name_get(), LEVEL_STRINGS[level]);
					vfprintf(logFile, fmt, args);
					fprintf(logFile, "\n");
					fflush(logFile);
				}
				break;
			case LEVEL_WARN:
			case LEVEL_AMBIGUOUS:
			case LEVEL_INFO:
			case LEVEL_DEBUG:
			default:
				fprintf(stdout, "%s[%-10.10s]", LEVEL_COLORS[level], thread::name_get());
				vfprintf(stdout, fmt, args);
				fprintf(stdout, TERM_COLOR_RESET "\n");

				if (logFile != NULL) {
					fprintf(logFile, "[%-10.10s]", thread::name_get());
					vfprintf(logFile, fmt, args);
					fprintf(logFile, "\n");
					fflush(logFile);
				}
				break;
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

		fprintf(stderr, "%s[%-10.10s] %s(%s: %d):", LEVEL_COLORS[level], thread::name_get(), LEVEL_STRINGS[level], file, line);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, TERM_COLOR_RESET "\n");

		if (logFile != NULL) {
			fprintf(logFile, "[%-10.10s] %s(%s: %d):", thread::name_get(), LEVEL_STRINGS[level], file, line);
			vfprintf(logFile, fmt, args);
			fprintf(logFile, "\n");
			fflush(logFile);
		}

		va_end(args);
	}

	void _assert_failure_log_with_location(const char* file, u32 line, const char* condition, const char* fmt, ...) {
		if (!initialized) return;
		__builtin_va_list args;
		va_start(args, fmt);

		fprintf(stderr, "%s[%-10.10s] assert(%s: %d):", LEVEL_COLORS[LEVEL_FATAL], thread::name_get(), file, line);
		vfprintf(stderr, fmt, args);
		fprintf(stderr, TERM_COLOR_RESET "\n");

		if (logFile != NULL) {
			fprintf(logFile, "[%-10.10s] assert(%s: %d): %s\n\t", thread::name_get(), file, line, condition);
			vfprintf(logFile, fmt, args);
			fprintf(logFile, "\n");
			fflush(logFile);
		}

		va_end(args);
	}
}

#undef TERM_COLOR_RESET
#undef TERM_COLOR_FG
#undef TERM_COLOR_BG
