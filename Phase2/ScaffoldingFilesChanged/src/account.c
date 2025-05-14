#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <errno.h>

#include "account.h"
#include "logging.h"
#include "banned.h"
#include "pwHandling.h"

/**
 * @file account.c
 *
 * @brief Implementation of functions used for user account management/creation
 *
 */

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
			log_message(LOG_DEBUG, "only_ASCII_printable_chars: Non-printable ASCII character found: %d", c);

			return(false);
		}

		s++;
	}

	log_message(LOG_DEBUG, "only_ASCII_printable_chars: All characters are printable ASCII.");

	return(true);
}

/**
 * @brief birthday input much match expected format YYYY-MM-DD
 */
static bool birthday_valid(const char *s)
{
	if (strlen(s) != 10)
	{
		log_message(LOG_DEBUG, "birthday_valid: Incorrect length (%zu).", strlen(s));

		return(false);
	}

	if (s[4] != '-' || s[7] != '-')
	{
		log_message(LOG_DEBUG, "birthday_valid: Dashes not in correct positions.");

		return(false);
	}

	for (int i = 0; i < 10; i++)
	{
		if (i == 4 || i == 7)
		{
			continue;
		}

		if (!isdigit(s[i]))
		{
			log_message(LOG_DEBUG, "birthday_valid: Non-digit character at position %d.", i);

			return(false);
		}
	}

	log_message(LOG_DEBUG, "birthday_valid: Format is valid.");

	return(true);
}

/**
 * @brief Checks whether a plaintext password meets required length and complexity rules.
 *
 * A valid password must be between MIN_PASSWORD_LENGTH and MAX_PASSWORD_LENGTH characters,
 * and contain at least one lowercase letter, one uppercase letter, one digit, and one symbol.
 *
 * @param plaintext_password Null-terminated password string to validate.
 * @return true if the password meets all criteria, false otherwise.
 */
bool password_valid(const char *plaintext_password)
{
	int pass_len = strnlen(plaintext_password, MAX_PASSWORD_LENGTH + 1);

	if (pass_len > MAX_PASSWORD_LENGTH || pass_len < MIN_PASSWORD_LENGTH)
	{
		log_message(LOG_ERROR, "Password wrong length. Must be between %d-%d characters.", MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH);

		return(false);
	}

	bool hasSymbol = false;
	bool hasNumber = false;
	bool hasLower  = false;
	bool hasUpper  = false;

	for (const char *p = plaintext_password; *p; p++)
	{
		if (isdigit(*p))
		{
			hasNumber = true;
		}
		else if (islower(*p))
		{
			hasLower = true;
		}
		else if (isupper(*p))
		{
			hasUpper = true;
		}
		else if (ispunct(*p))
		{
			hasSymbol = true;
		}
	}

	if (!hasNumber || !hasLower || !hasUpper || !hasSymbol)
	{
		log_message(LOG_ERROR, "Password must contain at least one lowercase, one uppercase, one digit, and one symbol.");

		return(false);
	}

	return(true);
}

/**
 * @brief Copies full input up to FIELD_SIZE. Adds '\0' only if input is shorter
 *
 * @param field where in account the char pointer will be stored
 * @param input null-terminated input string to be stored
 * @param FIELD_SIZE maximum size of field, given by constants in account.h
 */
static bool pseudo_string_copy(char *field, const char *input, size_t FIELD_SIZE)
{
	size_t len = strlen(input);

	if (len > FIELD_SIZE)
	{
		return(false);
	}

	log_message(LOG_DEBUG, "pseudo_string_copy: Copying string of length %zu into field of size %zu.", len, FIELD_SIZE);
	memcpy(field, input, len);

	if (len < FIELD_SIZE)
	{
		field[len] = '\0';
	}

	return(true);
}

/**
 * @brief Changes the 'partial pseudo-strings' in acc struct to valid null-terminated char*
 *
 * @param field
 * @param field_len
 * @param out_buf
 * @param out_buf_size
 */
static void buffer_to_cstring(const char *field, size_t field_len, char *out_buf, size_t out_buf_size)
{
	size_t copy_len = 0;

	// Scan for null or reach end of field
	while (copy_len < field_len && field[copy_len] != '\0')
	{
		copy_len++;
	}

	// Limit copy to fit in out_buf (leave space for null terminator)
	if (copy_len >= out_buf_size)
	{
		copy_len = out_buf_size - 1;
	}

	memcpy(out_buf, field, copy_len);
	out_buf[copy_len] = '\0';
}

static bool ip_to_cstring(const ip4_addr_t ip, char *out_buf, size_t out_buf_size)
{
	struct in_addr ip_addr;

	ip_addr.s_addr = ip;

	if (inet_ntop(AF_INET, &ip_addr, out_buf, (unsigned int)out_buf_size) == NULL)
	{
		log_message(LOG_ERROR, "ip_to_cstring: Failed to convert IP to string.");

		return(false);
	}

	return(true);
}

// ----------------------------------------------------------------------------------

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

	// Use shared password_valid logic for password requirements
	if (!password_valid(plaintext_password))
	{
		return(NULL);
	}

	if (strnlen(email, EMAIL_LENGTH + 1) > EMAIL_LENGTH)
	{
		log_message(LOG_ERROR, "Email too long.");

		return(NULL);
	}

	if (!(only_ASCII_printable_chars(email)))
	{
		log_message(LOG_ERROR, "Invalid email format.");

		return(NULL);
	}

	const char *final_birthdate;

	if (!birthday_valid(birthdate))
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

	// Copy values into struct (make sure to handle null-termination)
	if (!pseudo_string_copy(actptr->userid, userid, USER_ID_LENGTH))
	{
		log_message(LOG_ERROR, "pseudo_string_copy failed, tried to create a *char larger than USER_ID_LENGTH.");
		account_free(actptr); //prevent memory leak

		return(NULL);
	}

	log_message(LOG_DEBUG, "account_create: UserID copy succeeded.");

	if (!account_update_password(actptr, plaintext_password))
	{
		log_message(LOG_ERROR, "account_update_password failed.");
		account_free(actptr); //prevent memory leak

		return(NULL);
	}

	log_message(LOG_DEBUG, "account_create: Password hash stored.");

	if (!pseudo_string_copy(actptr->email, email, EMAIL_LENGTH))
	{
		log_message(LOG_ERROR, "pseudo_string_copy failed, tried to create a *char larger than EMAIL_LENGTH.");
		account_free(actptr); //prevent memory leak

		return(NULL);
	}

	log_message(LOG_DEBUG, "account_create: Email copy succeeded.");

	if (!pseudo_string_copy(actptr->birthdate, final_birthdate, BIRTHDATE_LENGTH))
	{
		log_message(LOG_ERROR, "pseudo_string_copy failed, tried to create a *char larger than BIRTHDATE_LENGTH.");
		account_free(actptr); //prevent memory leak

		return(NULL);
	}

	log_message(LOG_DEBUG, "account_create: Birthdate copy succeeded.");

	// Set defaults
	actptr->account_id		 = 0;
	actptr->unban_time		 = 0;
	actptr->expiration_time	 = 0;
	actptr->login_count		 = 0;
	actptr->login_fail_count = 0;
	actptr->last_login_time	 = 0;
	actptr->last_ip			 = 0;
	log_message(LOG_DEBUG, "account_create: Default fields initialized.");

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
void account_free(account_t *acc) //cppcheck-suppress staticFunction
{
	if (acc)
	{
		explicit_bzero(acc, sizeof(*acc)); //ensures the compiler does not optimise clearing the account memory before freeing it
		free(acc);
	}
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
//------------------------------------------------------------------------------------

/**
 * @brief Update the account's password.
 *
 * Hashes the new plaintext password and updates the stored password hash.
 *
 * @param acc Pointer to the account structure to update.
 * @param new_plaintext_password Null-terminated string of the new password.
 * @return true if the update was successful, false otherwise.
 */
//------------------------------------------------------------------------------------

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
	log_message(LOG_DEBUG, "account_record_login_success: Recording successful login. IP: %u", ip);
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
	log_message(LOG_DEBUG, "account_record_login_failure: Recording failed login.");
	time_t currentTime = time(NULL);

	acc->last_login_time = currentTime;

	acc->login_fail_count = acc->login_fail_count + 1;
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
		log_message(LOG_DEBUG, "account_is_banned: Current time: %ld, Unban time: %ld", (size_t)time(NULL), (size_t)acc->unban_time);

		return(false); // no ban
	}

	time_t current_time = time(NULL);

	if (current_time == -1)
	{
		log_message(LOG_ERROR, "acc_banned: Failed to get the current time.");

		return(true); // False positive probably better than false negative here
	}

	log_message(LOG_DEBUG, "account_is_banned: Current time: %ld, Unban time: %ld", (size_t)current_time, (size_t)acc->unban_time);

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
		log_message(LOG_DEBUG, "account_is_expired: Current time: %ld, Expiration time: %ld", (size_t)time(NULL), (size_t)acc->expiration_time);

		return(false); // unlimited
	}

	time_t current_time;
	current_time = time(NULL);

	if (current_time == -1)
	{
		log_message(LOG_ERROR, "acc_expired: Failed to get the current time.");

		return(true); // False positive probably better than false negative here
	}

	log_message(LOG_DEBUG, "account_is_expired: Current time: %ld, Expiration time: %ld", (size_t)current_time, (size_t)acc->expiration_time);

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
	acc->unban_time = t;
	log_message(LOG_DEBUG, "account_set_unban_time: Set to %ld", (size_t)t);
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
	log_message(LOG_DEBUG, "account_set_expiration_time: Set to %ld", (size_t)t);
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

		return;
	}

	if (!pseudo_string_copy(acc->email, new_email, EMAIL_LENGTH))
	{
		log_message(LOG_ERROR, "account_set_email: pseudo_string_copy failed — email too long.");

		return;
	}

	log_message(LOG_DEBUG, "account_set_email: Email updated to %s", new_email);
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
bool account_print_summary(const account_t *acct, int fd)
{
	// Caller is required to make sure fd is valid + writeable
	// char buffer[516];       // Buffer to hold the formatted string //  switching to asprintf which will allocate the buffer for us
	char *buffer;
	char  timebuf[64];

	struct tm		 tm_result;
	const struct tm *tm_info = localtime_r(&(acct->last_login_time), &tm_result);

	if (tm_info == NULL || strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info) == 0)
	{
		if (strlcpy(timebuf, "Invalid time", sizeof(timebuf)) >= sizeof(timebuf))
		{
			log_message(LOG_ERROR, "account_print_summary: strlcpy tried to create a string larger than sizeof(timebuf).");

			return(false);
		}

		/* strncpy(timebuf, "Invalid time", sizeof(timebuf)); */
		timebuf[sizeof(timebuf) - 1] = '\0';
	}

	char printableUID[USER_ID_LENGTH + 1];
	char printableEmail[EMAIL_LENGTH + 1];

	buffer_to_cstring(acct->userid, USER_ID_LENGTH, printableUID, sizeof(printableUID));
	buffer_to_cstring(acct->email, EMAIL_LENGTH, printableEmail, sizeof(printableEmail));

	char printableIP[INET_ADDRSTRLEN];  // typically 16 bytes

	if (!ip_to_cstring(acct->last_ip, printableIP, sizeof(printableIP)))
	{
		log_message(LOG_ERROR, "account_print_summary: Failed to convert IP.");

		return(false);
	}

	int bytes_written = asprintf(&buffer,
	                             "====SUMMARY=====\nUserID:\t\t%.101s\n"
	                             "Email:\t\t%.101s\n"
	                             "Last Login:\t%s\n"
	                             "Login Count:\t%u\n"
	                             "Login Failures:\t%u\n"
	                             "Last IP:\t%.16s\n================\n", // potentially format ip address...
	                             printableUID,
	                             printableEmail,
	                             timebuf,
	                             acct->login_count,
	                             acct->login_fail_count,
	                             printableIP); //switched from snprintf to asprintf https://stackoverflow.com/questions/12746885/why-use-asprintf-instead-of-sprintf

	if (buffer == NULL)
	{
		log_message(LOG_ERROR, "Memory allocation failed during asprintf");

		return(false);
	}

	if (bytes_written < 0)
	{
		log_message(LOG_ERROR, "account_print_summary: Failed to format summary.");
		free(buffer);

		return(false);
	}

	size_t	len		= (size_t)bytes_written;
	ssize_t written = write(fd, buffer, (size_t)bytes_written);

	if (written < 0)
	{
		log_message(LOG_ERROR, "Failed to write to fd for user %s: %s", printableUID, strerror(errno));

		return(false);
	}
	else if ((size_t)written < len)
	{
		log_message(LOG_WARN, "Partial write to client_fd for user %s: wrote %zd of %zu bytes", printableUID, written, len);

		return(false);
	}
	else
	{
		log_message(LOG_DEBUG, "Sent message to client_fd for user");
	}

	free(buffer); //need to free it after allocating it with asprintf

	return(true);
}
