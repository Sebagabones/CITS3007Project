/**
 * @file pwHandling.c
 * @brief Implementation of password hashing and validation functions
 *
 * This file implements the functions for password hashing and validation using the Argon2id
 * algorithm, with sodium for salt generation and constant-time comparison.
 */

// if we include banned here – shit explodes
#include "pwHandling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <argon2.h>
#include <sodium.h>
#include "banned.h"

// For safe string/memory operations
#define __STDC_WANT_LIB_EXT1__    1

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

	// Copy to output buffer with safe length handling
	size_t copy_len = (result < HASH_LENGTH - 1) ? result : HASH_LENGTH - 1;
	strncpy(output, temp_buf, copy_len);
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

	// Extract memory cost with validation
	char *	endptr		 = NULL;
	int32_t m_cost_value = strtol(params_start, &endptr, 10);

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
	char *salt_start = params_end + 1;
	char *salt_end	 = strchr(salt_start, '$');

	if (salt_end == NULL)
	{
		return(false);
	}

	size_t salt_b64_len = salt_end - salt_start;

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
	int result = argon2id_hash_raw(t_cost,           // Time cost
	                               m_cost,           // Memory cost
	                               parallelism,      // Parallelism
	                               password,         // Password
	                               password_len,     // Password length
	                               salt,             // Salt
	                               SALT_LENGTH,      // Salt length
	                               raw_hash,         // Output hash
	                               HASH_RAW_LENGTH); // Output hash length

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
	if (acc == NULL || plaintext_password == NULL)
	{
		return(false);
	}

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
{
	// Validate preconditions
	if (acc == NULL || new_plaintext_password == NULL)
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
