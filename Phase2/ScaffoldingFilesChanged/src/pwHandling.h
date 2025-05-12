/**
 * @file pwHandling.h
 * @brief Secure password hashing and validation using Argon2id
 *
 * This file provides functions for hashing and validating user passwords
 * using the Argon2id algorithm, which provides strong protection against both
 * side-channel attacks and GPU-based attacks.
 */
#ifndef PWHANDLING_H
#define PWHANDLING_H

#include <stdbool.h>
#include "account.h"

// Define constants for password hashing
#define SALT_LENGTH		   16
#define HASH_RAW_LENGTH	   32
#define T_COST			   3     // Time cost parameter
#define M_COST			   65536 // Memory cost parameter (64 MB)
#define PARALLELISM		   4     // Parallelism parameter

/**
 * @brief Validates a plaintext password against the stored hash in an account.
 *
 * @param acc Pointer to the account structure containing the stored hash.
 * @param plaintext_password The plaintext password to validate.
 * @return true if the password matches, false otherwise.
 */
bool account_validate_password(const account_t *acc, const char *plaintext_password);

/**
 * @brief Updates an account's password with a new plaintext password.
 *
 * @param acc Pointer to the account structure to update.
 * @param new_plaintext_password The new plaintext password.
 * @return true if the password was updated successfully, false otherwise.
 */
bool account_update_password(account_t *acc, const char *new_plaintext_password);

#endif // PWHANDLING_H
