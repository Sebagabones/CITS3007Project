#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "account.h"
#include "banned.h"
#include "logging.h"
//We may be meant to make our own .h file? idk
//chat are we meant to do this?
//also idk it wouldnt compile if i didnt add logging.h but i think im doing something wrong this so /shrug

/**
 * Create a new account with the specified parameters.
 *
 * This function initializes a new dynamically allocated account structure
 * with the given user ID, hash information derived from the specified plaintext password, email address,
 * and birthdate. Other fields are set to their default values.
 *
 * On success, returns a pointer to the newly created account structure.
 * On error, returns NULL and logs an error message.
 */
account_t *account_create(const char *userid, const char *plaintext_password,
                          const char *email, const char *birthdate
                          )
{
	// remove the contents of this function and replace it with your own code.
	(void)userid;
	(void)plaintext_password;
	(void)email;
	(void)birthdate;

	return(NULL);
}

/**
 * Free memory and resources used by the account
 *
 * Clears and releases any memory associated with an account.
 *
 * @param acc A pointer returned by `account_create`, or `NULL`.
 */
void account_free(account_t *acc)
{
// as we are not using pointers in account_t struct, we don't need to free each item in the struct (I think, feel free to fact check) - https://stackoverflow.com/a/13590879
// however, since this this will be freeing user accounts, we probably want to do more than just free the memory, instead zeroing it out so that it cannot be accessed later on - probaly explicit_bzero or similar (if we use C23 then we should use memset_explict)

	if (acc) //only run explicit_bzero when not null
	{
		explicit_bzero(acc, sizeof(*acc));
	}

	free(acc);
}

bool account_validate_password(const account_t *acc, const char *plaintext_password)
{
	// remove the contents of this function and replace it with your own code.
	(void)acc;
	(void)plaintext_password;

	return(false);
}

bool account_update_password(account_t *acc, const char *new_plaintext_password)
{
	// remove the contents of this function and replace it with your own code.
	(void)acc;
	(void)new_plaintext_password;

	return(false);
}

/**
 * Record a successful login.
 *
 * Update account metadata following a successful login.
 * Sets the `last_login_time` using the current system time, sets the accounts `last_ip` to the last IP address connected from, updates the accounts `login_count` and sets teh accounts `login_fail_count` to zero
 * Whenever a user logs in successfully, their `login_fail_count` is reset to 0; it’s thus a measure of the number of _consecutive_ login failures.
 *
 * @param acc Must be non-`NULL`.
 * @param ip Must be a valid IPv4 address.
 */
void account_record_login_success(account_t *acc, ip4_addr_t ip)
{
	//NOTE: We will need to add this to doxygen, but as this didnt specify what to do with errors, im going to do what account_create does -
	// On error, logs an appropriate error message using log_message - doesnt return anything as void - does not update the last login time, but does update other values.
	// but check on the help forum first

	time_t currentTime = time(NULL);

	acc->last_login_time = currentTime;

	acc->login_fail_count = 0;
	acc->login_count	  = acc->login_count + 1;
	acc->last_ip		  = ip;
}

/**
 * Record a failed login
 *
 * Update account metadata following a failed login.
 * Sets the `last_login_time` using the current system time, updates the accounts `login_fail_count` and sets the accounts `login_count` to zero
 * time, and the last IP address connected from be set correctly.
 * Whenever a user fails to log in successfully, their login_count is reset to 0.
 *
 * @param acc Must be non-`NULL`.
 */
void account_record_login_failure(account_t *acc)
{
	//NOTE: We will need to add this to doxygen, but as this didnt specify what to do with errors, im going to do what account_create does -
	// On error, logs an appropriate error message using log_message - doesnt return anything as void - does not update the last login time, but does update other values.
	// but check on the help forum first

	time_t currentTime = time(NULL);

	log_message(LOG_INFO, "time failed");

	acc->last_login_time = currentTime;

	acc->login_fail_count = acc->login_fail_count + 1;;
	acc->login_count	  = 0;
}

/**
 * Returns true if the account is currently banned
 *
 * Compares the current system time with the relevant field in the account structure.
 *
 * @param acc Must be non-`NULL`.
 *
 * @return Boolean representing if the account is banned or not
 */
bool account_is_banned(const account_t *acc)
{
	if (acc->unban_time == 0)
	{
		return(false); // no ban
	}

	time_t current_time = time(NULL);

	if (current_time == -1)
	{
		log_message(LOG_ERROR, "acc_banned: Failed to get the current time.");

		return(true); // False positive probably better than false negative here
	}

	return(acc->unban_time > current_time);
}

/**
 * Returns true if the account is currently expired
 *
 * Compares the current system time with the relevant field in the account structure.
 *
 * @param acc Must be non-`NULL`.
 *
 * @return Boolean representing if the account is expired or not
 */
bool account_is_expired(const account_t *acc)
{
	if (acc->expiration_time == 0)
	{
		return(false); // unlimited
	}

	time_t current_time;
	current_time = time(NULL);

	if (current_time == -1)
	{
		log_message(LOG_ERROR, "acc_expired: Failed to get the current time.");

		return(true); // False positive probably better than false negative here
	}

	return(acc->expiration_time < current_time);
}

/**
 * Set a ban time.
 *
 * Ban an account for a period of `t` time; The code in `account.h` details how this works.
 *
 * @param acc Must be non-`NULL`
 * @param t Ban the account up until this time (0 = no ban)
 */
void account_set_unban_time(account_t *acc, time_t t)
{
	acc->unban_time = t; //this seems like this should be harder, maybe im missing something??
}

/**
 * Set an account expiration time
 *
 * Sets the expiration time of an account until the account is no longer valid; More details in  `account.h`
 *
 * @param acc  Must be non-`NULL`
 * @param t Account is only valid until this time (0 = unlimited)
 */
void account_set_expiration_time(account_t *acc, time_t t)
{
	acc->expiration_time = t; //ditto
}

void account_set_email(account_t *acc, const char *new_email)
{
	// remove the contents of this function and replace it with your own code.
	(void)acc;
	(void)new_email;
}

bool account_print_summary(const account_t *acct, int fd)
{
	// remove the contents of this function and replace it with your own code.
	(void)acct;
	(void)fd;

	return(false);
}
