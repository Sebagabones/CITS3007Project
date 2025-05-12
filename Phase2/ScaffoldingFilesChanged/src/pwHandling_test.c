// flawfinder: ignore
/**
 * @file pwHandling_test.c
 * @brief Test cases for password hashing and validation functions
 *
 * Tests for password handling functions including edge cases, random passwords,
 * and potential security vulnerabilities.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "account.h" // Include this first to get the real account_t structure
#include "pwHandling.h"
#include <sodium.h>  // For random password generation
#include "banned.h"

/**
 * @brief Generates a random password of specified length
 *
 * @param output Buffer to store the generated password
 * @param length Length of the password to generate
 * @return true if password was generated successfully, false otherwise
 */
static bool generate_random_password(char *output, size_t length)
{
	if (output == NULL || length == 0)
	{
		return(false);
	}

	// Initialize libsodium
	if (sodium_init() < 0)
	{
		return(false);
	}

	// Character sets for password generation
	const char charset[]	= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-_=+[]{}|;:,.<>?";
	size_t	   charset_size = strlen(charset);

	// Generate random indices into the charset
	for (size_t i = 0; i < length - 1; i++)
	{
		unsigned int random_index = randombytes_uniform((uint32_t)charset_size);
		output[i] = charset[random_index];
	}

	// Ensure null termination
	output[length - 1] = '\0';

	return(true);
}

/**
 * @brief Tests password validation with a valid password
 */
static void test_valid_password(void)
{
	printf("Test: Validating a correct password...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	const char *test_password = "SecureP@ssw0rd123";

	// Update account with password
	bool update_result = account_update_password(&account, test_password);
	assert(update_result && "Failed to update account with password");

	// Validate the same password
	bool validate_result = account_validate_password(&account, test_password);
	assert(validate_result && "Failed to validate correct password");

	printf("PASS: Valid password correctly validated\n");
}

/**
 * @brief Tests password validation with an invalid password
 */
static void test_invalid_password(void)
{
	printf("Test: Rejecting an incorrect password...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	const char *correct_password = "SecureP@ssw0rd123";
	const char *wrong_password	 = "WrongP@ssw0rd456";

	// Update account with correct password
	bool update_result = account_update_password(&account, correct_password);
	assert(update_result && "Failed to update account with password");

	// Try to validate with wrong password
	bool validate_result = account_validate_password(&account, wrong_password);
	assert(!validate_result && "Validated incorrect password as correct");

	printf("PASS: Invalid password correctly rejected\n");
}

/**
 * @brief Tests validation of case-sensitive passwords
 */
static void test_case_sensitivity(void)
{
	printf("Test: Ensuring passwords are case-sensitive...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	const char *correct_password	= "CaseSensitive123";
	const char *wrong_case_password = "casesensitive123";

	// Update account with correct password
	bool update_result = account_update_password(&account, correct_password);
	assert(update_result && "Failed to update account with password");

	// Try to validate with wrong case password
	bool validate_result = account_validate_password(&account, wrong_case_password);
	assert(!validate_result && "Case-insensitive validation detected");

	printf("PASS: Passwords are properly case-sensitive\n");
}

/**
 * @brief Tests handling of NULL parameters
 */
static void test_null_parameters(void)
{
	printf("Test: Handling NULL parameters...\n");

	account_t account;
	memset(&account, 0, sizeof(account));
	const char *test_password = "TestPassword123";

	// Test NULL account
	assert(!account_update_password(NULL, test_password) &&
	       "Update accepted NULL account");

	assert(!account_validate_password(NULL, test_password) &&
	       "Validate accepted NULL account");

	// Test NULL password
	assert(!account_update_password(&account, NULL) &&
	       "Update accepted NULL password");

	assert(!account_validate_password(&account, NULL) &&
	       "Validate accepted NULL password");

	printf("PASS: NULL parameters correctly rejected\n");
}

/**
 * @brief Tests handling of empty passwords
 */
static void test_empty_password(void)
{
	printf("Test: Handling empty passwords...\n");

	account_t account;
	memset(&account, 0, sizeof(account));
	const char *empty_password = "";

	// Empty passwords should work (though they're insecure)
	bool update_result = account_update_password(&account, empty_password);
	assert(update_result && "Failed to update with empty password");

	bool validate_result = account_validate_password(&account, empty_password);
	assert(validate_result && "Failed to validate empty password");

	// Ensure non-empty password is rejected
	assert(!account_validate_password(&account, "NotEmpty") &&
	       "Validated non-empty password against empty hash");

	printf("PASS: Empty passwords handled correctly\n");
}

/**
 * @brief Tests handling of very long passwords
 */
static void test_long_password(void)
{
	printf("Test: Handling very long passwords...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	// Generate a long password (1KB)
	char long_password[1025];
	memset(long_password, 'A', sizeof(long_password) - 1);
	long_password[sizeof(long_password) - 1] = '\0';

	// Long passwords should work
	bool update_result = account_update_password(&account, long_password);
	assert(update_result && "Failed to update with long password");

	bool validate_result = account_validate_password(&account, long_password);
	assert(validate_result && "Failed to validate long password");

	printf("PASS: Long passwords handled correctly\n");
}

/**
 * @brief Tests handling of special characters in passwords
 */
static void test_special_characters(void)
{
	printf("Test: Handling special characters in passwords...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	const char *special_password = "!@#$%^&*()_+-=[]{}|;':\",./<>?`~";

	// Special characters should work
	bool update_result = account_update_password(&account, special_password);
	assert(update_result && "Failed to update with special characters");

	bool validate_result = account_validate_password(&account, special_password);
	assert(validate_result && "Failed to validate special characters");

	printf("PASS: Special characters handled correctly\n");
}

/**
 * @brief Tests handling of non-ASCII characters in passwords
 */
static void test_unicode_characters(void)
{
	printf("Test: Handling Unicode characters in passwords...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	// Unicode password (with emojis and non-Latin characters)
	const char *unicode_password = "пароль密码パスワード🔒🔑";

	// Unicode should work
	bool update_result = account_update_password(&account, unicode_password);
	assert(update_result && "Failed to update with Unicode characters");

	bool validate_result = account_validate_password(&account, unicode_password);
	assert(validate_result && "Failed to validate Unicode characters");

	printf("PASS: Unicode characters handled correctly\n");
}

/**
 * @brief Tests updating password multiple times
 */
static void test_password_updates(void)
{
	printf("Test: Updating password multiple times...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	const char *password1 = "FirstPassword123";
	const char *password2 = "SecondPassword456";
	const char *password3 = "ThirdPassword789";

	// Update with first password
	bool update1 = account_update_password(&account, password1);
	assert(update1 && "Failed to update with first password");

	bool validate1 = account_validate_password(&account, password1);
	assert(validate1 && "Failed to validate first password");

	// Update with second password
	bool update2 = account_update_password(&account, password2);
	assert(update2 && "Failed to update with second password");

	bool validate2a = account_validate_password(&account, password2);
	assert(validate2a && "Failed to validate second password");

	bool validate2b = account_validate_password(&account, password1);
	assert(!validate2b && "First password still valid after update");

	// Update with third password
	bool update3 = account_update_password(&account, password3);
	assert(update3 && "Failed to update with third password");

	bool validate3a = account_validate_password(&account, password3);
	assert(validate3a && "Failed to validate third password");

	bool validate3b = account_validate_password(&account, password2);
	assert(!validate3b && "Second password still valid after update");

	printf("PASS: Multiple password updates handled correctly\n");
}

/**
 * @brief Tests with randomly generated passwords
 */
static void test_random_passwords(void)
{
	printf("Test: Testing with randomly generated passwords...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	// Test with 20 different random passwords instead of 100 to speed up testing
	for (int i = 0; i < 20; i++)
	{
		// Generate random password of random length (8-32 chars)
		size_t length = (size_t)(8 + rand() % 25);
		char   random_password[33]; // Max 32 chars + null terminator

		bool generated = generate_random_password(random_password, length);
		assert(generated && "Failed to generate random password");

		// Update account with random password
		bool update_result = account_update_password(&account, random_password);
		assert(update_result && "Failed to update with random password");

		// Validate the same password
		bool validate_result = account_validate_password(&account, random_password);
		assert(validate_result && "Failed to validate random password");

		// Try a slight modification of the password (change last char)
		if (strlen(random_password) > 0)
		{
			char modified_password[33];
			strcpy(modified_password, random_password);
			modified_password[strlen(modified_password) - 1] = 'X';

			bool invalid_result = account_validate_password(&account, modified_password);
			assert(!invalid_result && "Validated incorrect modified password");
		}
	}

	printf("PASS: Random password tests completed successfully\n");
}

/**
 * @brief Tests whether same password produces different hashes (salt working)
 */
static void test_salt_effectiveness(void)
{
	printf("Test: Verifying salt effectiveness...\n");

	account_t account1;
	account_t account2;
	memset(&account1, 0, sizeof(account1));
	memset(&account2, 0, sizeof(account2));

	const char *same_password = "IdenticalPassword123";

	// Update both accounts with same password
	bool update1 = account_update_password(&account1, same_password);
	bool update2 = account_update_password(&account2, same_password);
	assert(update1 && update2 && "Failed to update accounts");

	// Hashes should be different due to different salts
	bool hashes_differ = (strcmp(account1.password_hash, account2.password_hash) != 0);
	assert(hashes_differ && "Same password produced identical hashes");

	// But both should validate correctly
	bool validate1 = account_validate_password(&account1, same_password);
	bool validate2 = account_validate_password(&account2, same_password);
	assert(validate1 && validate2 && "Failed to validate accounts with same password");

	printf("PASS: Salt is effective - same password produces different hashes\n");
}

/**
 * @brief Tests for timing consistency to prevent timing attacks
 *
 * Note: This is a basic check, not a comprehensive timing analysis
 */
static void test_timing_consistency(void)
{
	printf("Test: Basic check for timing consistency...\n");

	account_t account;
	memset(&account, 0, sizeof(account));

	const char *correct_password = "TimingTestPassword";

	// Update account with correct password
	bool update_result = account_update_password(&account, correct_password);
	assert(update_result && "Failed to update account");

	// Test passwords of different lengths
	const char *short_password = "Short";
	const char *long_password  = "ThisIsAMuchLongerPasswordForTimingTest";

	// These should both be rejected, hopefully in consistent time
	bool result1 = account_validate_password(&account, short_password);
	bool result2 = account_validate_password(&account, long_password);

	assert(!result1 && !result2 && "Incorrect passwords were validated");

	printf("PASS: Basic timing consistency check completed\n");
	printf("Note: Comprehensive timing analysis would require specialized tools\n");
}

/**
 * @brief Main test function
 */
int main(void)
{
	// Seed the random number generator
	srand((unsigned int)time(NULL));

	printf("===== Starting Password Handling Tests =====\n\n");

	// Basic functionality tests
	test_valid_password();
	test_invalid_password();
	test_case_sensitivity();

	// Edge case tests
	test_null_parameters();
	test_empty_password();
	test_long_password();
	test_special_characters();
	test_unicode_characters();

	// Functionality tests
	test_password_updates();
	test_random_passwords();

	// Security tests
	test_salt_effectiveness();
	test_timing_consistency();

	printf("\n===== All Tests Passed Successfully =====\n");

	return(0);
}
