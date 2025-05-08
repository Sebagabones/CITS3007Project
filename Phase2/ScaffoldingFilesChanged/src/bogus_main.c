// DO NOT SUBMIT THIS FILE
//
// When submitting your project, this file will be overwritten
// by the automated build and test system.

#include "login.h"
#include "account.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "banned.h"
#include "logging.h"

// bogus implementation of main that links in all account and login
// functions. You can delete this file and replace it with your own main
// function(s).
int main(void)
{
	account_t *acc = account_create("user1", "password123", "user@example.com", "2000-01-01");

	if (!acc)
	{
		log_message(LOG_ERROR, "Failed to create account.");

		return(1);
	}

	account_set_email(acc, "new_email@example.com");
	account_set_unban_time(acc, time(NULL) + 3600);
	account_set_expiration_time(acc, time(NULL) + 86400);

	account_record_login_failure(acc);
	account_record_login_success(acc, 0x7f000001); // 127.0.0.1

	if (account_validate_password(acc, "password123"))
	{
		log_message(LOG_INFO, "Password validated successfully.");
	}
	else
	{
		log_message(LOG_WARN, "Password validation failed.");
	}

	if (!account_update_password(acc, "newpassword456"))
	{
		log_message(LOG_ERROR, "Password update failed.");
	}

	if (account_print_summary(acc, STDOUT_FILENO))
	{
		log_message(LOG_INFO, "Summary printed.");
	}

	account_free(acc);

	return(0);
}

/*
 *  (void)argc;
 *  (void)argv;
 *
 *  account_t *acc = account_create("", "", "", "");
 *  account_free(acc);
 *  bool res = account_validate_password(acc, "");
 *  res = account_update_password(acc, "");
 *  account_record_login_success(acc, 0);
 *  account_record_login_failure(acc);
 *  res = account_is_banned(acc);
 *  res = account_is_expired(acc);
 *  account_set_unban_time(acc, 0);
 *  account_set_expiration_time(acc, 0);
 *  account_set_email(acc, "");
 *  (void)account_print_summary(acc, STDOUT_FILENO);
 *  (void)res;
 *  handle_login("", "", 0, 0, STDOUT_FILENO, NULL);
 *
 *  return(0);
 */
