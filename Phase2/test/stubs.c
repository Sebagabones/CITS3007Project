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
/* caused warnings so removed for testing */
/* static void panic(const char *msg) */
/* { */
/* 	fprintf(stderr, "PANIC: %s\n", msg); */
/* 	abort(); */
/* } */

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
		/* fprintf(stderr, "DEBUG: "); */
		break;

	case LOG_INFO:
		/* fprintf(stdout, "INFO: "); */
		break;

	case LOG_WARN:
		/* fprintf(stderr, "WARNING: "); */
		break;

	case LOG_ERROR:
		/* fprintf(stderr, "ERROR: "); */
		break;

	default:
		/* panic("Invalid log level"); */
		break;
	}
	/* vfprintf(stderr, fmt, args); */
	/* fprintf(stderr, "\n"); // newline, optional */
	va_end(args);

	pthread_mutex_unlock(&log_mutex);
}

// Mock implementation (replace stubs version for testing only)
bool account_lookup_by_userid(const char *userid, account_t *acc)
{
	// Literally hardcode test users
	if (strcmp(userid, "Ichigo") == 0)
	{
		strcpy(acc->userid, "Ichigo");
		/* strcpy(acc->password_hash, "Bankai2"); */
		acc->account_id		  = 1;
		acc->unban_time		  = 0;
		acc->expiration_time  = 0;
		acc->login_fail_count = 0;

		if (!account_update_password(acc, "Bankai22222$$$$$"))
		{
			return(false);
		}

		return(true);
	}
	else if (strcmp(userid, "Banned-Account") == 0)
	{
		strcpy(acc->userid, "Banned-Account");
		acc->unban_time		  = time(NULL) + 86400;
		acc->expiration_time  = 0;
		acc->login_fail_count = 0;

		return(true);
	}
	else if (strcmp(userid, "Expired-Account") == 0)
	{
		strcpy(acc->userid, "Expired-Account");
		acc->unban_time		  = 0;
		acc->expiration_time  = time(NULL) - 86400;
		acc->login_fail_count = 0;

		return(true);
	}
	else if (strcmp(userid, "Too-Many") == 0)
	{
		strcpy(acc->userid, "Too-Many");
		strcpy(acc->password_hash, "someBankai192####");
		acc->unban_time		  = 0;
		acc->expiration_time  = 0;
		acc->login_fail_count = 11;

		return(true);
	}

	return(false);
}
