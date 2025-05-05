#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "account.h"
#include "banned.h"
#include "logging.h"                         // This seemed to be breaking things

#define PLAINTEXT_PASSWORD_MAX_LENGTH    100 // alter this for password hashing

// -----------------Sanitization Prototypes---------------
static bool only_ASCII_printable_chars(const char *s);
static bool birthday_valid(const char *s);

// -------------------------------------------------------

/**
 * @brief Create a new account with the specified parameters.
 *
 * Initializes a dynamically allocated account structure with the given user ID,
 * password, email, and birthdate. Performs validation and sets default metadata fields.
 *
 * @param userid Null-terminated string for the user ID.
 * @param plaintext_password Null-terminated string for the password (to be hashed).
 * @param email Null-terminated string for the email address.
 * @param birthdate Null-terminated string for the birthdate in YYYY-MM-DD format.
 * @return Pointer to a new account on success, or NULL on error.
 */
account_t *account_create(const char *userid, const char *plaintext_password,
                          const char *email, const char *birthdate)
{
// Precondition: all pointers are non-null and strings are null-terminated.
	if (strnlen(userid, USER_ID_LENGTH + 1) > USER_ID_LENGTH)
	{
		log_message(LOG_ERROR, "UserID too long.");

		return(NULL);
	}

	if (strnlen(plaintext_password, PLAINTEXT_PASSWORD_MAX_LENGTH + 1) > PLAINTEXT_PASSWORD_MAX_LENGTH)
	// Depends on how we do hashing !! (might only care about hashlength?)
	{
		log_message(LOG_ERROR, "Password too long.");

		return(NULL);
	}

	if (!(only_ASCII_printable_chars(email)))
	{
		log_message(LOG_ERROR, "Invalid email format.");

		return(NULL);
	}

	const char *final_birthdate;

	if (!birthday_valid(birthdate)) // Or log info?
	{
		final_birthdate = "0000-00-00";
		log_message(LOG_WARN, "Invalid birthdate format. Using default '0000-00-00'.");
	}
	else
	{
		final_birthdate = birthdate;
	}

	// Allocate memory
	account_t *actptr = malloc(sizeof(account_t));

	if (actptr == NULL)
	{
		log_message(LOG_ERROR, "Memory allocation failed.");

		return(NULL);
	}

	// Assign account ID (Not sure how it happens)
	actptr->account_id = generate_account_id();

	// Copy values into struct (make sure to handle null-termination)
	strncpy(actptr->userid, userid, USER_ID_LENGTH); //safe as userid is always valid & null-terminated according to precondtions, and actptr seems to be safe?
	actptr->userid[USER_ID_LENGTH - 1] = '\0';

	hash_password(plaintext_password, actptr->password_hash, HASH_LENGTH);
	// Assuming hash_password writes the result into the buffer and handles
	// null-termination

	strncpy(actptr->email, email, EMAIL_LENGTH); //safe as email is valid & null-terminated according to precondtions, and actptr seems to be safe?
	actptr->email[EMAIL_LENGTH - 1] = '\0';

	strncpy(actptr->birthdate, final_birthdate, BIRTHDATE_LENGTH); //safe as birthdate is valid & null-terminated according to preconditions, and actptr seems to be safe?
	actptr->birthdate[BIRTHDATE_LENGTH - 1] = '\0';

	// Set defaults
	actptr->unban_time		 = 0;
	actptr->expiration_time	 = 0;
	actptr->login_count		 = 0;
	actptr->login_fail_count = 0;
	actptr->last_login_time	 = 0;
	actptr->last_ip			 = 0;

	return(actptr);
}

/**
 * @brief Free memory and resources used by the account.
 *
 * Clears and releases any memory associated with an account, zeroing out
 * the contents to prevent later access.
 *
 * @param acc Pointer to an account structure previously returned by `account_create`,
 *            or NULL. If NULL, no action is taken.
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

/**
 * @brief Validate a plaintext password against the stored hash.
 *
 * Compares the given plaintext password with the stored password hash
 * to verify if they match.
 *
 * @param acc Pointer to the account structure.
 * @param plaintext_password Null-terminated string of the password to validate.
 * @return true if the password matches, false otherwise.
 */
bool account_validate_password(const account_t *acc, const char *plaintext_password)
{
	// remove the contents of this function and replace it with your own code.
	(void)acc;
	(void)plaintext_password;

	return(false);
}

/**
 * @brief Update the account's password.
 *
 * Hashes the new plaintext password and updates the stored password hash.
 *
 * @param acc Pointer to the account structure to update.
 * @param new_plaintext_password Null-terminated string of the new password.
 * @return true if the update was successful, false otherwise.
 */
bool account_update_password(account_t *acc, const char *new_plaintext_password)
{
	// remove the contents of this function and replace it with your own code.
	(void)acc;
	(void)new_plaintext_password;

	return(false);
}

/**
 * @brief Record a successful login for the account.
 *
 * Updates the account's metadata following a successful login:
 * sets the last login time to the current system time, updates the last IP address,
 * increments the login count, and resets the login failure count to zero.
 *
 * @param acc Pointer to the account structure (must be non-NULL).
 * @param ip The IPv4 address from which the login occurred.
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
 * @brief Record a failed login attempt for the account.
 *
 * Updates the account's metadata following a failed login:
 * sets the last login time to the current system time,
 * increments the login failure count, and resets the login count to zero.
 *
 * @param acc Pointer to the account structure (must be non-NULL).
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
 * @brief Check if the account is currently banned.
 *
 * Compares the current system time with the account's unban time to determine
 * if the account is still banned.
 *
 * @param acc Pointer to the account structure (must be non-NULL).
 * @return true if the account is banned, false otherwise.
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
 * @brief Check if the account is currently expired.
 *
 * Compares the current system time with the account's expiration time to determine
 * if the account has expired.
 *
 * @param acc Pointer to the account structure (must be non-NULL).
 * @return true if the account is expired, false otherwise.
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
 * @brief Set the unban time for the account.
 *
 * Specifies the time until which the account is banned.
 *
 * @param acc Pointer to the account structure (must be non-NULL).
 * @param t Time until which the account is banned (0 means no ban).
 */
void account_set_unban_time(account_t *acc, time_t t)
{
	acc->unban_time = t; //this seems like this should be harder, maybe im missing something??
}

/**
 * @brief Set the expiration time for the account.
 *
 * Specifies the time until which the account remains valid.
 *
 * @param acc Pointer to the account structure (must be non-NULL).
 * @param t Time until which the account is valid (0 means unlimited).
 */
void account_set_expiration_time(account_t *acc, time_t t)
{
	acc->expiration_time = t; //ditto
}

/**
 * @brief Safely update the account's email address.
 *
 * Validates the new email address for printable ASCII characters and absence of spaces,
 * then updates the account's email field.
 *
 * @param acc Pointer to the account structure (must be non-NULL).
 * @param new_email Null-terminated string containing the new email address.
 */
void account_set_email(account_t *acc, const char *new_email)
{
	// Preconds: both args are non-null & new-email has '\0'
	if (!(only_ASCII_printable_chars(new_email)))
	{
		log_message(LOG_ERROR, "Invalid email format.");

		return; // not sure how to handle
	}

	// Safe copy into the email field
	strncpy(acc->email, new_email, EMAIL_LENGTH);
	acc->email[EMAIL_LENGTH - 1] = '\0';  // Ensure null-termination
}

/**
 * @brief Print a summary of the account information to a file descriptor.
 *
 * Formats the account ID, user ID, email, and login-related statistics
 * into a string and writes it to the given file descriptor.
 *
 * @param acct Pointer to the account structure (must be non-NULL).
 * @param fd File descriptor to write the summary to.
 * @return true if the summary was successfully written, false otherwise.
 */
static bool account_print_summary(const account_t *acct, int fd)
{
	// Caller is required to make sure fd is valid + writeable
	char buffer[516];       // Buffer to hold the formatted string
	char timebuf[64];

	struct tm		 tm_result;
	const struct tm *tm_info = localtime_r(&(acct->last_login_time), &tm_result);

	if (tm_info == NULL || strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info) == 0)
	{
		strncpy(timebuf, "Invalid time", sizeof(timebuf));
		timebuf[sizeof(timebuf) - 1] = '\0';
	}

	int bytes_written = snprintf(buffer, sizeof(buffer),
	                             "UserID: %.31s\n"
	                             "Email: %.63s\n"
	                             "Last Login Time: %s\n"
	                             "Login Count: %u\n"
	                             "Login Failures: %u\n"
	                             "Last IP: %u\n", // potentially format ip address...
	                             acct->userid,
	                             acct->email,
	                             timebuf,
	                             acct->login_count,
	                             acct->login_fail_count,
	                             acct->last_ip);

	if (bytes_written < 0 || bytes_written >= sizeof(buffer))
	{
		log_message(LOG_ERROR, "account_print_summary: Failed to format summary.");

		return(false);
	}

	ssize_t result = write(fd, buffer, bytes_written);

	if (result == -1)
	{
		log_message(LOG_ERROR, "account_print_summary: Failed to write to file descriptor.");

		return(false);
	}

	return(true);
}

/**
 * @brief Function to neutralise email inputs
 *
 * c Printable ASCII = characters with decimal values from 32 (space) to 126 (~)
 * Excludes 32
 *
 * @param s char pointer to be checked
 * @return true if the string is only ASCII printable characters, false otherwise.
 */
static bool only_ASCII_printable_chars(const char *s)
{
	while (*s)
	{
		unsigned char c = (unsigned char)*s;

		if (c < 33 || c > 126)
		{
			return(false);
		}

		s++;
	}

	return(true);
}

/**
 * @brief needs to check is_real and is_valid_format
 */
static bool birthday_valid(const char *s)
{
	return(false);
}
