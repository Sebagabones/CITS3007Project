#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>

#include "account.h"
#include "logging.h"
#include "banned.h"

//#define MAIN2
#ifdef MAIN2
int main(void)
{
	account_t *acc = account_create("user1", "P@ssword1234", "user@example.com", "2000-01-01");

	if (!acc)
	{
		log_message(LOG_ERROR, "Failed to create account.");

		return(1);
	}

	account_set_email(acc, "new_email@example.com");
	account_set_unban_time(acc, time(NULL) + 3600);
	account_set_expiration_time(acc, time(NULL) + 86400);

	// Password validation tests
	if (account_validate_password(acc, "wrongpassword"))
	{
		log_message(LOG_WARN, "Incorrect password incorrectly validated.");
	}
	else
	{
		log_message(LOG_INFO, "Incorrect password correctly rejected.");
	}

	if (account_validate_password(acc, "P@ssword1234"))
	{
		log_message(LOG_INFO, "Correct password validated successfully.");
	}
	else
	{
		log_message(LOG_ERROR, "Correct password failed validation.");
	}

	if (!account_update_password(acc, ""))
	{
		log_message(LOG_INFO, "Empty password update correctly failed.");
	}
	else
	{
		log_message(LOG_ERROR, "Empty password update incorrectly succeeded.");
	}

	if (account_update_password(acc, "Newpassw*rd456"))
	{
		log_message(LOG_INFO, "Password updated to 'Newpassw*rd456'.");
	}
	else
	{
		log_message(LOG_ERROR, "Password update to 'Newpassw*rd456' failed.");
	}

	if (account_validate_password(acc, "Newpassw*rd456"))
	{
		log_message(LOG_INFO, "New password validated successfully.");
	}
	else
	{
		log_message(LOG_ERROR, "New password validation failed.");
	}

	if (account_print_summary(acc, STDOUT_FILENO))
	{
		log_message(LOG_INFO, "Summary printed.");
	}

	account_free(acc);

	return(0);
}

#endif
