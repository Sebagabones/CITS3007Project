// allow access to FILE-based IO (e.g. fprintf) in this translation unit
#define CITS3007_PERMISSIVE

#include "logging.h"
#include "db.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "banned.h"

/**
 * Abort immediately for unrecoverable errors /
 * invalid program state.
 *
 * Arguments:
 * - msg: message to log before aborting
 *
 * This function should not return.
 */
static void panic(const char *msg)
{
	fprintf(stderr, "PANIC: %s\n", msg);
	abort();
}

// Global mutex for logging
// This mutex is used to ensure that log messages are printed in a thread-safe manner.
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_message(log_level_t level, const char *fmt, ...)
{
	pthread_mutex_lock(&log_mutex);

	va_list args;
	va_start(args, fmt);
	switch (level)
	{
	case LOG_DEBUG:
		fprintf(stderr, "DEBUG: ");
		break;

	case LOG_INFO:
		fprintf(stdout, "INFO: ");
		break;

	case LOG_WARN:
		fprintf(stderr, "WARNING: ");
		break;

	case LOG_ERROR:
		fprintf(stderr, "ERROR: ");
		break;

	default:
		panic("Invalid log level");
		break;
	}
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n"); // newline, optional
	va_end(args);

	pthread_mutex_unlock(&log_mutex);
}
