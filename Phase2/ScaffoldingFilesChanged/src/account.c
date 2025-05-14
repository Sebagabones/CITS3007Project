#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <argon2.h>
#include <sodium.h>
#include <pthread.h>

#include "account.h"
#include "logging.h"
#include "banned.h"

/**
 * @file account.c
 *
 * @brief Implementation of functions used for user account management/creation
 *
 */

// 	PASSWORD CONSTANTS
#define MIN_PASSWORD_LENGTH	   12
#define MAX_PASSWORD_LENGTH	   256

//	HASHING CONSTANTS
#define SALT_LENGTH			   16
#define HASH_RAW_LENGTH		   32
#define T_COST				   3     // Time cost parameter
#define M_COST				   65536 // Memory cost parameter (64 MB)
#define PARALLELISM			   4     // Parallelism parameter

// Global Mutex for printing the account summary atomically
static pthread_mutex_t acc_summary_mutex = PTHREAD_MUTEX_INITIALIZER;

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
static bool password_valid(const char *plaintext_password)
{
	size_t pass_len = strnlen(plaintext_password, MAX_PASSWORD_LENGTH + 1);

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

	pthread_mutex_lock(&acc_summary_mutex);

	size_t	len		= (size_t)bytes_written;
	ssize_t written = write(fd, buffer, (size_t)bytes_written);
	
	pthread_mutex_unlock(&acc_summary_mutex);	

	if (written < 0)
	{
		log_message(LOG_ERROR, "Failed to write to fd for user %s: %s", printableUID, strerror(errno));
		free(buffer);

		return(false);
	}
	else if ((size_t)written < len)
	{
		log_message(LOG_WARN, "Partial write to client_fd for user %s: wrote %zd of %zu bytes", printableUID, written, len);
		free(buffer);

		return(false);
	}
	else
	{
		log_message(LOG_DEBUG, "Sent message to client_fd for user");
	}

	free(buffer); //need to free it after allocating it with asprintf

	return(true);
}

//-------------------------------------- PASSWORD STUFF

/**
 * @file pwHandling.c
 * @brief Implementation of password hashing and validation functions
 *
 * This file implements the functions for password hashing and validation using the Argon2id
 * algorithm, with sodium for salt generation and constant-time comparison.
 */
// For safe string/memory operations
#define __STDC_WANT_LIB_EXT1__    1

// if we include banned here – shit explodes

//  DO NOT TOUCH HASH_LENGTH
//  char password_hash[HASH_LENGTH];

/**
 * @brief Generates cryptographically secure random bytes
 *
 * @param output Buffer to store the generated random bytes
 * @param length Number of random bytes to generate
 * @return true if random bytes were successfully generated, false otherwise
 */
static bool generate_random_bytes(unsigned char *output, size_t length)
{
	// Validate parameters
	if (output == NULL || length == 0)
	{
		return(false);
	}

	// Use libsodium for secure random number generation
	if (sodium_init() < 0)
	{
		return(false);
	}

	randombytes_buf(output, length);

	return(true);
}

/**
 * @brief Performs constant-time comparison of two strings using sodium's memcmp
 *
 * @param a First string to compare
 * @param b Second string to compare
 * @return true if strings are equal, false otherwise
 */
static bool constant_compare(const char *a, const char *b)
{
	// Validate parameters
	if (a == NULL || b == NULL)
	{
		return(false);
	}

	// Get lengths first to avoid multiple strlen calls
	size_t len_a = strlen(a);
	size_t len_b = strlen(b);

	// If lengths are different, return false without doing comparison
	if (len_a != len_b)
	{
		return(false);
	}

	// Use libsodium's constant-time comparison to prevent timing attacks
	return(sodium_memcmp(a, b, len_a) == 0);
}

/**
 * @brief Formats the Argon2 hash with parameters, salt, and hash
 *
 * @param output Buffer to store the formatted hash string
 * @param t_cost Time cost parameter
 * @param m_cost Memory cost parameter
 * @param parallelism Parallelism parameter
 * @param salt Salt used for hashing
 * @param raw_hash Raw hash output from Argon2
 */
static void format_argon2_hash(char *output, int t_cost, int m_cost, int parallelism,
                               const unsigned char *salt,
                               const unsigned char *raw_hash)
{
	// Validate parameters
	if (output == NULL || salt == NULL || raw_hash == NULL)
	{
		if (output != NULL)
		{
			output[0] = '\0'; // Set to empty string on error
		}

		return;
	}

	// Use smaller buffers to minimize the chance of truncation
	// Base64 encoding of binary data increases size by ~4/3
	// For SALT_LENGTH and HASH_RAW_LENGTH, calculate safe sizes
	char salt_b64[((SALT_LENGTH + 2) / 3) * 4 + 4]; // +4 for padding and null terminator
	char hash_b64[((HASH_RAW_LENGTH + 2) / 3) * 4 + 4];

	// Initialize buffers to prevent uninitialized data (using secure zeroing)
	sodium_memzero(salt_b64, sizeof(salt_b64));
	sodium_memzero(hash_b64, sizeof(hash_b64));

	// Encode salt and hash to base64
	sodium_bin2base64(salt_b64, sizeof(salt_b64), salt, SALT_LENGTH, sodium_base64_VARIANT_ORIGINAL);
	sodium_bin2base64(hash_b64, sizeof(hash_b64), raw_hash, HASH_RAW_LENGTH, sodium_base64_VARIANT_ORIGINAL);

	// Use a temp buffer for safe formatting to avoid potential buffer overflow
	char temp_buf[HASH_LENGTH * 2]; // Double size to ensure space
	sodium_memzero(temp_buf, sizeof(temp_buf));

	// Format the hash with all parameters
	int result = snprintf(temp_buf, sizeof(temp_buf), "$argon2id$v=19$m=%d,t=%d,p=%d$%s$%s",
	                      m_cost, t_cost, parallelism, salt_b64, hash_b64);

	// Check for formatting success and length constraints
	if (result < 0 || result >= HASH_LENGTH)
	{
		// Error or too long - set to empty string
		output[0] = '\0';

		return;
	}

	size_t copy_len = (result < HASH_LENGTH - 1) ? (size_t)result : (size_t)(HASH_LENGTH - 1);
	memcpy(output, temp_buf, copy_len);

	output[copy_len] = '\0'; // Ensure null termination
}

/**
 * @brief Extracts hash components from the stored hash string
 *
 * @param hash_str Full hash string in Argon2 format
 * @param salt_output Buffer to store the extracted salt
 * @param t_cost_output Pointer to store the time cost parameter
 * @param m_cost_output Pointer to store the memory cost parameter
 * @param parallelism_output Pointer to store the parallelism parameter
 * @return true if extraction was successful, false otherwise
 */
static bool extract_hash_components(const char *hash_str, unsigned char *salt_output,
                                    int *t_cost_output, int *m_cost_output, int *parallelism_output)
{
	// Validate parameters
	if (hash_str == NULL || salt_output == NULL ||
	    t_cost_output == NULL || m_cost_output == NULL || parallelism_output == NULL)
	{
		return(false);
	}

	// Expected format: $argon2id$v=19$m=65536,t=3,p=4$[salt_base64]$[hash_base64]

	// Check if the hash starts with the expected prefix
	if (strncmp(hash_str, "$argon2id$v=19$m=", 17) != 0)
	{
		return(false);
	}

	// Parse parameters
	char *params_start = (char *)hash_str + 17;
	char *params_end   = strchr(params_start, '$');

	if (params_end == NULL)
	{
		return(false);
	}

	// Range check
	char *	endptr		 = NULL;
	int32_t m_cost_value = strtol(params_start, &endptr, 10);

	if (m_cost_value < INT32_MIN || m_cost_value > INT32_MAX)
	{
		return(false);
	}

	*m_cost_output = (int32_t)m_cost_value;

	if (endptr == params_start || *endptr != ',' ||
	    m_cost_value <= 0 || m_cost_value > INT_MAX)
	{
		return(false);
	}

	*m_cost_output = (int)m_cost_value;

	// Extract time cost with validation
	const char *t_cost_start = strstr(params_start, ",t=");

	if (t_cost_start == NULL)
	{
		return(false);
	}

	endptr = NULL;
	int32_t t_cost_value = strtol(t_cost_start + 3, &endptr, 10);

	if (endptr == (t_cost_start + 3) || (*endptr != ',' && *endptr != '$') ||
	    t_cost_value <= 0 || t_cost_value > INT_MAX)
	{
		return(false);  // Invalid value
	}

	*t_cost_output = (int)t_cost_value;

	// Extract parallelism with validation
	const char *p_start = strstr(params_start, ",p=");

	if (p_start == NULL)
	{
		return(false);
	}

	endptr = NULL;
	int32_t p_value = strtol(p_start + 3, &endptr, 10);

	if (endptr == (p_start + 3) || (*endptr != ',' && *endptr != '$') ||
	    p_value <= 0 || p_value > INT_MAX)
	{
		return(false);  // Invalid value
	}

	*parallelism_output = (int)p_value;

	// Extract salt
	const char *salt_start = params_end + 1;
	const char *salt_end   = strchr(salt_start, '$');

	if (salt_end == NULL)
	{
		return(false);
	}

	if (salt_end <= salt_start)
	{
		return(false);
	}

	size_t salt_b64_len = (size_t)(salt_end - salt_start);

	// Validate salt_b64_len is reasonable
	if (salt_b64_len >= 64)
	{
		return(false);
	}

	// Temporary buffer for salt base64 string
	char salt_b64[64];

	// Clear the buffer using secure zeroing
	sodium_memzero(salt_b64, sizeof(salt_b64));

	if (salt_b64_len < sizeof(salt_b64))
	{
		// Safely copy salt string with proper bounds checking
		if (salt_b64_len > 0)
		{
			strncpy(salt_b64, salt_start, salt_b64_len);
		}

		salt_b64[salt_b64_len] = '\0';
	}
	else
	{
		return(false);
	}

	// Decode salt from base64
	size_t salt_bin_len;

	if (sodium_base642bin(salt_output, SALT_LENGTH, salt_b64,
	                      salt_b64_len, NULL, &salt_bin_len, NULL,
	                      sodium_base64_VARIANT_ORIGINAL) != 0 || salt_bin_len != SALT_LENGTH)
	{
		return(false);
	}

	return(true);
}

/**
 * @brief Generates an Argon2id hash of a password
 *
 * @param password The plaintext password to hash
 * @param salt The salt to use for hashing
 * @param t_cost Time cost parameter
 * @param m_cost Memory cost parameter
 * @param parallelism Parallelism parameter
 * @param output Buffer to store the formatted hash output
 * @return true if hashing was successful, false otherwise
 */
static bool generate_argon2_hash(const char *password,
                                 const unsigned char *salt,
                                 int t_cost, int m_cost, int parallelism,
                                 char *output)
{
	// Validate parameters
	if (password == NULL || salt == NULL || output == NULL)
	{
		return(false);
	}

	// Validate costs are positive
	if (t_cost <= 0 || m_cost <= 0 || parallelism <= 0)
	{
		return(false);
	}

	// Create a buffer for the raw hash
	unsigned char raw_hash[HASH_RAW_LENGTH];

	// Calculate password length safely
	size_t password_len = strlen(password);

	// Hash the password with Argon2id
	int result = argon2id_hash_raw((uint32_t)t_cost,           // Time cost
	                               (uint32_t)m_cost,           // Memory cost
	                               (uint32_t)parallelism,      // Parallelism
	                               password,                   // Password
	                               password_len,               // Password length
	                               salt,                       // Salt
	                               SALT_LENGTH,                // Salt length
	                               raw_hash,                   // Output hash
	                               HASH_RAW_LENGTH);           // Output hash length

	if (result != ARGON2_OK)
	{
		return(false);
	}

	// Format the final hash string with parameters and salt
	format_argon2_hash(output, t_cost, m_cost, parallelism, salt, raw_hash);

	// Check if formatting was successful (empty string on error)
	if (output[0] == '\0')
	{
		return(false);
	}

	// Securely wipe the raw hash from memory
	sodium_memzero(raw_hash, HASH_RAW_LENGTH);

	return(true);
}

/**
 * @brief Validates a plaintext password against the stored hash in an account
 *
 * @param acc Pointer to the account structure containing the stored hash
 * @param plaintext_password The plaintext password to validate
 * @return true if the password matches, false otherwise
 */
bool account_validate_password(const account_t *acc, const char *plaintext_password)
{
	// Validate preconditions
	// if (acc == NULL || plaintext_password == NULL)
	// {
	// 	return(false);
	// }

	// Parse the stored hash to extract salt and parameters
	unsigned char stored_salt[SALT_LENGTH];
	int			  t_cost, m_cost, parallelism;

	// Extract salt and parameters from stored format
	if (!extract_hash_components(acc->password_hash, stored_salt, &t_cost, &m_cost, &parallelism))
	{
		return(false);
	}

	// Calculate hash of provided plaintext password using extracted parameters
	char computed_hash[HASH_LENGTH];

	if (!generate_argon2_hash(plaintext_password, stored_salt, t_cost, m_cost,
	                          parallelism, computed_hash))
	{
		return(false);
	}

	// Compare the computed hash with the stored hash using constant-time comparison
	return(constant_compare(computed_hash, acc->password_hash));
}

/**
 * @brief Updates an account's password with a new plaintext password
 *
 * @param acc Pointer to the account structure to update
 * @param new_plaintext_password The new plaintext password
 * @return true if the password was updated successfully, false otherwise
 */
bool account_update_password(account_t *acc, const char *new_plaintext_password)
{   // Preconditions: acc is non-null and the char pointer is null-terminated
	if (!password_valid(new_plaintext_password))
	{
		return(false);
	}

	// Generate random salt
	unsigned char salt[SALT_LENGTH];
	sodium_memzero(salt, SALT_LENGTH); // Initialize to zeros using secure zeroing

	if (!generate_random_bytes(salt, SALT_LENGTH))
	{
		return(false);
	}

	// Hash the new password with Argon2id
	if (!generate_argon2_hash(new_plaintext_password, salt, T_COST, M_COST,
	                          PARALLELISM, acc->password_hash))
	{
		// Securely wipe the salt from memory before returning
		sodium_memzero(salt, SALT_LENGTH);

		return(false);
	}

	// Securely wipe the salt from memory
	sodium_memzero(salt, SALT_LENGTH);

	return(true);
}
