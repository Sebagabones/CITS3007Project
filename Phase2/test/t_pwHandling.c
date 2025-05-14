/**
 * @file pwHandling_test.c
 * @brief Test cases for password hashing and validation functions
 *
 * Tests for password handling functions including edge cases, random passwords,
 * and potential security vulnerabilities.
 */
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sodium.h>

#include "tst.h"
#include "account.h"
#include "login.h"
#include "logging.h"
#include "db.h"

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
	for (size_t i = 0; i < length - 5; i++)
	{
		unsigned int random_index = randombytes_uniform((uint32_t)charset_size);
		output[i] = charset[random_index];
	}

	//this is jank but meh, it doesnt have to be great, just make sure each password is valid
	output[length - 5] = '#';
	output[length - 4] = 'f';
	output[length - 3] = 'G';
	output[length - 2] = '3';

	// Ensure null termination
	output[length - 1] = '\0';

	return(true);
}

tstsuite("account_password_validation")
{
	tstcase("Valid Password")
	{
		account_t	account		  = { 0 };
		const char *test_password = "SecureP@ssw0rd123";

		tstcheck(account_update_password(&account, test_password))
		tstcheck(account_validate_password(&account, test_password))
	}

	tstcase("Invalid Password No Nums")
	{
		account_t	account		  = { 0 };
		const char *test_password = "Nonums!";

		tstcheck(account_update_password(&account, test_password) == false)
	}

	tstcase("Invalid Password No Caps")
	{
		account_t	account		  = { 0 };
		const char *test_password = "nocaps1!";

		tstcheck(account_update_password(&account, test_password) == false)
	}

	tstcase("Invalid Password No Symbol")
	{
		account_t	account		  = { 0 };
		const char *test_password = "Nosymbol1";

		tstcheck(account_update_password(&account, test_password) == false)
	}

	tstcase("Invalid Password No Anything")
	{
		account_t	account		  = { 0 };
		const char *test_password = "literallynothing";

		tstcheck(account_update_password(&account, test_password) == false)
	}

	tstcase("Incorrect Password")
	{
		account_t	account			 = { 0 };
		const char *correct_password = "SecureP@ssw0rd123";
		const char *wrong_password	 = "WrongP@ssw0rd456";

		tstcheck(account_update_password(&account, correct_password))
		tstcheck(!account_validate_password(&account, wrong_password))
	}

	tstcase("Case Sensitivity")
	{
		account_t	account				= { 0 };
		const char *correct_password	= "#CaseSensitive123";
		const char *wrong_case_password = "#casesensitive123";

		tstcheck(account_update_password(&account, correct_password))
		tstcheck(!account_validate_password(&account, wrong_case_password))
	}

	tstcase("Empty Password")  ////gotta make sure empty passwords get rejected

	{
		account_t	account		   = { 0 };
		const char *empty_password = "";

		tstcheck(!account_update_password(&account, empty_password))
		tstcheck(!account_validate_password(&account, empty_password))

		tstcheck(!account_validate_password(&account, "NotEmpty"))
	}

	tstcase("Long Password")
	{
		account_t account = { 0 };

		// create a massive password - should fail as longer than 256
		char long_password[1025];

		memset(long_password, 'A', sizeof(long_password) - 1);
		long_password[sizeof(long_password) - 1] = '\0';

		tstcheck(!account_update_password(&account, long_password))
		tstcheck(!account_validate_password(&account, long_password))
	}

	tstcase("Special Characters")
	{
		account_t	account			 = { 0 };
		const char *special_password = "abC123!@#$%^&*()_+-=[]{}|;':\",./<>?`~";

		tstcheck(account_update_password(&account, special_password))
		tstcheck(account_validate_password(&account, special_password))
	}

	tstcase("Unicode Characters")
	{
		account_t	account			 = { 0 };
		const char *unicode_password = "aC1*пароль密码パスワード🔒🔑"; //lmao hope your editor supports this (tbf nano does so yall should be chill)

		tstcheck(account_update_password(&account, unicode_password))
		tstcheck(account_validate_password(&account, unicode_password))
	}

	tstcase("Password Updates")
	{
		account_t	account	  = { 0 };
		const char *password1 = "FirstPassword123@";
		const char *password2 = "SecondPassword45#6";
		const char *password3 = "ThirdPassword78$9";

		tstcheck(account_update_password(&account, password1))
		tstcheck(account_validate_password(&account, password1))

		tstcheck(account_update_password(&account, password2))
		tstcheck(account_validate_password(&account, password2))
		tstcheck(!account_validate_password(&account, password1))

		tstcheck(account_update_password(&account, password3))
		tstcheck(account_validate_password(&account, password3))
		tstcheck(!account_validate_password(&account, password2))
	}

	tstcase("Random Passwords")
	{
		account_t account = { 0 };

		for (int i = 0; i < 15; i++)
		{
			size_t length = (size_t)(15 + rand() % 18);
			char   random_password[33]; // Max 32 chars + null terminator

			tstcheck(generate_random_password(random_password, length))

			tstcheck(account_update_password(&account, random_password))

			tstcheck(account_validate_password(&account, random_password))

			// now try modifing the password and see if it passes
			if (strlen(random_password) > 0)
			{
				char modified_password[33];
				strcpy(modified_password, random_password);

				if (modified_password[strlen(modified_password) - 1] == 'X')
				{
					modified_password[strlen(modified_password) - 1] = 'Y';
				}
				else
				{
					modified_password[strlen(modified_password) - 1] = 'X';
				}

				tstcheck(!account_validate_password(&account, modified_password), "initial password was: %s, changed password was: %s", random_password, modified_password)
			}
		}
	}

	tstcase("Salt Effectiveness")
	{
		account_t	account1	  = { 0 };
		account_t	account2	  = { 0 };
		const char *same_password = "IdenticalPassword123#";

		// set two accounts with the same password
		tstcheck(account_update_password(&account1, same_password))
		tstcheck(account_update_password(&account2, same_password))

		// make sure that the hashes aren't the same
		tstcheck(strcmp(account1.password_hash, account2.password_hash) != 0)

		// make sure they work
		tstcheck(account_validate_password(&account1, same_password))
		tstcheck(account_validate_password(&account2, same_password))
	}
}
