/**
 * @file account_test.c
 * @brief Test cases for account management functions
 *
 * Tests for account creation, modification, validation, and status checking.
 * Follows the tst.h framework for consistent testing across the codebase.
 */
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "tst.h"
#include "account.h"
#include "logging.h"

bool hash_password_mock_success(const char *plaintext, char *out_hash, size_t hash_len)
{
	strlcpy(out_hash, "HASHED_", hash_len);
	strlcat(out_hash, plaintext, hash_len);

	return(true);
}

/* function to create a temporary file descriptor for  account_print_summary */
static int create_temp_fd()
{
	char temp_filename[] = "/tmp/account_test_XXXXXX";
	int	 fd = mkstemp(temp_filename);

	if (fd >= 0)
	{
		unlink(temp_filename); // Delete the file but  keep the fd open
	}

	return(fd);
}

tstsuite("Account Testing")
{
	tstcase("account_creation")
	{
		tstsection("Valid Account Creation")
		{
			account_t *acc = account_create("testuser", "password123", "test@example.com", "1990-01-01");

			tstcheck(acc != NULL)

			// Verify th fields were correctly set
			char userid_buf[USER_ID_LENGTH + 1] = { 0 };
			memcpy(userid_buf, acc->userid, USER_ID_LENGTH);
			tstcheck(strcmp(userid_buf, "testuser") == 0)

			char email_buf[EMAIL_LENGTH + 1] = { 0 };
			memcpy(email_buf, acc->email, EMAIL_LENGTH);
			tstcheck(strcmp(email_buf, "test@example.com") == 0)

			char birthdate_buf[BIRTHDATE_LENGTH + 1] = { 0 };
			memcpy(birthdate_buf, acc->birthdate, BIRTHDATE_LENGTH);
			tstcheck(strcmp(birthdate_buf, "1990-01-01") == 0)

			tstcheck(acc->account_id == 0)
			tstcheck(acc->unban_time == 0)
			tstcheck(acc->expiration_time == 0)
			tstcheck(acc->login_count == 0)
			tstcheck(acc->login_fail_count == 0)

			account_free(acc);
		}

		tstsection("Invalid UserID (Too Long)")
		{
			char long_userid[USER_ID_LENGTH + 10];

			memset(long_userid, 'a', sizeof(long_userid));
			long_userid[USER_ID_LENGTH + 9] = '\0';

			account_t *acc = account_create(long_userid, "password123", "test@example.com", "1990-01-01");
			tstcheck(acc == NULL)
		}

		tstsection("Invalid Email (Non-ASCII)")
		{
			account_t *acc = account_create("testuser", "password123", "test\x01@example.com", "1990-01-01");

			tstcheck(acc == NULL)
		}

		tstsection("Invalid Birthday Format")
		{
			account_t *acc = account_create("testuser", "password123", "test@example.com", "1990/01/01");

			tstcheck(acc != NULL)

			char birthdate_buf[BIRTHDATE_LENGTH + 1] = { 0 };
			memcpy(birthdate_buf, acc->birthdate, BIRTHDATE_LENGTH);
			tstcheck(strcmp(birthdate_buf, "0000-00-00") == 0)

			account_free(acc);
		}
	}

	tstcase("account_status_checking")
	{
		tstsection("Account Banned Status")
		{
			account_t acc = { 0 };

			tstcheck(!account_is_banned(&acc))

			// Set ban time to future
			time_t future_time = time(NULL) + 3600;
			account_set_unban_time(&acc, future_time);
			tstcheck(account_is_banned(&acc))

			// Set ban time to past
			time_t past_time = time(NULL) - 3600;
			account_set_unban_time(&acc, past_time);
			tstcheck(!account_is_banned(&acc))

			// Clear ban
			account_set_unban_time(&acc, 0);
			tstcheck(!account_is_banned(&acc))
		}

		tstsection("Account Expiration Status")
		{
			account_t acc = { 0 };

			tstcheck(!account_is_expired(&acc))

			// Set expiration time to past
			time_t past_time = time(NULL) - 3600;
			account_set_expiration_time(&acc, past_time);
			tstcheck(account_is_expired(&acc))

			// Set expiration time to future
			time_t future_time = time(NULL) + 3600;
			account_set_expiration_time(&acc, future_time);
			tstcheck(!account_is_expired(&acc))

			// Clear expiration
			account_set_expiration_time(&acc, 0);
			tstcheck(!account_is_expired(&acc))
		}
	}

	tstcase("account_login_tracking")
	{
		tstsection("Login Success Recording")
		{
			account_t  acc	   = { 0 };
			ip4_addr_t test_ip = 1; //im being lazy here and just using whatever lol

			account_record_login_success(&acc, test_ip);

			tstcheck(acc.login_count == 1)
			tstcheck(acc.login_fail_count == 0)
			tstcheck(acc.last_ip == test_ip)
			tstcheck(acc.last_login_time > 0)

			account_record_login_success(&acc, test_ip);
			tstcheck(acc.login_count == 2)
		}

		tstsection("Login Failure Recording")
		{
			account_t acc = { 0 };

			acc.login_count = 5;

			account_record_login_failure(&acc);

			tstcheck(acc.login_count == 0)
			tstcheck(acc.login_fail_count == 1)
			tstcheck(acc.last_login_time > 0)

			account_record_login_failure(&acc);
			tstcheck(acc.login_fail_count == 2)
		}

		tstsection("Login Tracking Reset")
		{
			account_t  acc	   = { 0 };
			ip4_addr_t test_ip = 2;

			account_record_login_failure(&acc);
			account_record_login_failure(&acc);
			tstcheck(acc.login_fail_count == 2)

			account_record_login_success(&acc, test_ip);
			tstcheck(acc.login_fail_count == 0)
			tstcheck(acc.login_count == 1)

			account_record_login_success(&acc, test_ip);
			tstcheck(acc.login_count == 2)

			account_record_login_failure(&acc);
			tstcheck(acc.login_count == 0)
			tstcheck(acc.login_fail_count == 1)
		}
	}

	tstcase("account_data_modification")
	{
		tstsection("Email Update")
		{
			account_t acc = { 0 };

			account_set_email(&acc, "initial@example.com");

			char email_buf[EMAIL_LENGTH + 1] = { 0 };
			memcpy(email_buf, acc.email, EMAIL_LENGTH);
			tstcheck(strcmp(email_buf, "initial@example.com") == 0)

			// Update email
			account_set_email(&acc, "updated@example.com");

			memset(email_buf, 0, sizeof(email_buf));
			memcpy(email_buf, acc.email, EMAIL_LENGTH);
			tstcheck(strcmp(email_buf, "updated@example.com") == 0)
		}

		tstsection("Invalid Email Update")
		{
			account_t acc = { 0 };

			// Set valid initial email
			account_set_email(&acc, "valid@example.com");

			// Try to update with invalid email
			account_set_email(&acc, "invalid\x01@example.com");

			// Email should remain unchanged
			char email_buf[EMAIL_LENGTH + 1] = { 0 };
			memcpy(email_buf, acc.email, EMAIL_LENGTH);
			tstcheck(strcmp(email_buf, "valid@example.com") == 0)
		}
	}

	tstcase("account_printing")
	{
		tstsection("Print Account Summary")
		{
			account_t acc = { 0 };

			strcpy(acc.userid, "testuser");
			strcpy(acc.email, "test@example.com");
			acc.login_count		 = 42;
			acc.login_fail_count = 5;
			acc.last_login_time	 = time(NULL);
			acc.last_ip			 = 3;

			// create temporary file descriptor
			int fd = create_temp_fd();
			tstcheck(fd >= 0)

			bool result = account_print_summary(&acc, fd);
			tstcheck(result)

			off_t file_size = lseek(fd, 0, SEEK_END);
			tstcheck(file_size > 0)

			// set file positon to begininging of fd
			lseek(fd, 0, SEEK_SET);

			// Read and check content of fd
			char	buffer[512] = { 0 };
			ssize_t bytes_read	= read(fd, buffer, sizeof(buffer) - 1);
			tstcheck(bytes_read > 0)

			// check for expected fields
			tstcheck(strstr(buffer, "UserID: testuser") != NULL)
			tstcheck(strstr(buffer, "Email: test@example.com") != NULL)
			tstcheck(strstr(buffer, "Login Count: 42") != NULL)
			tstcheck(strstr(buffer, "Login Failures: 5") != NULL)

			close(fd);
		}

		tstsection("Print Account Summary with Invalid File Descriptor")
		{
			account_t acc = { 0 };

			strcpy(acc.userid, "testuser");

			// use invalid file descriptor
			bool result = account_print_summary(&acc, -1);
			tstcheck(!result)
		}
	}
}
