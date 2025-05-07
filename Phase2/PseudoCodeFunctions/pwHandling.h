/**
 * @file pwHandling.h
 * @brief Secure password hashing and validation using Argon2id
 *
 * This file provides functions for hashing and validating user passwords
 * using the goated Argon2id algorithm, which provides strong protection against both
 * side-channel attacks and GPU-based attacks. (textbook definition pulled from their website)
 *
 * V1.0.0 (Just to keep track of my work. Not the project's version number.)
 */

#ifndef PHASE2_PSEUDOCODEFUNCTIONS_PWHANDLING_H_
#define PHASE2_PSEUDOCODEFUNCTIONS_PWHANDLING_H_

#include <stdbool.h>

// Define constants for password hashing
#define SALT_LENGTH		   16
#define HASH_LENGTH		   128
#define HASH_RAW_LENGTH	   32
#define T_COST			   3     // Time cost parameter
#define M_COST			   65536 // Memory cost parameter (64 MB)
#define PARALLELISM		   4     // Parallelism parameter

// Forward declaration of account structure
typedef struct account account_t;

/**
 * @brief Validates a plaintext password against the stored hash
 *
 * This function compares a user-provided plaintext password with the
 * stored password hash in the account structure. It uses Argon2id
 * with the same parameters that were used to create the original hash.
 *
 * @param acc Pointer to the account structure containing the stored hash
 * @param plaintext_password The plaintext password to validate
 * @return true if the password matches, false otherwise
 */
bool account_validate_password(const account_t *acc,
                               const char *plaintext_password);

/**
 * @brief Updates an account's password with a new plaintext password
 *
 * This function generates a new salt, hashes the provided plaintext password
 * using Argon2id with predefined parameters, and updates the account structure
 * with the new password hash.
 *
 * @param acc Pointer to the account structure to update
 * @param new_plaintext_password The new plaintext password
 * @return true if the password was updated successfully, false otherwise
 */
bool account_update_password(account_t *acc,
                             const char *new_plaintext_password);

#endif  // PHASE2_PSEUDOCODEFUNCTIONS_PWHANDLING_H_
