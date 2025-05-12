#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "tst.h"
#include "account.h"
#include "login.h"
#include "logging.h"
#include "db.h"
#include "pwHandling.h"
// Mock implementation (replace stubs version for testing only)
bool account_lookup_by_userid(const char *userid, account_t *acc)
{
	// Literally hardcode test users
	if (strcmp(userid, "Ichigo") == 0)
	{
		strcpy(acc->userid, "Ichigo");
		strcpy(acc->password_hash, "Bankai2");
		acc->account_id		  = 1;
		acc->unban_time		  = 0;
		acc->expiration_time  = 0;
		acc->login_fail_count = 0;

		return(true);
	}
	else if (strcmp(userid, "Banned-Account") == 0)
	{
		strcpy(acc->userid, "Banned-Account");
		acc->unban_time		  = time(NULL) + 86400;
		acc->expiration_time  = 0;
		acc->login_fail_count = 0;

		return(true);
	}
	else if (strcmp(userid, "Expired-Account") == 0)
	{
		strcpy(acc->userid, "Expired-Account");
		acc->unban_time		  = 0;
		acc->expiration_time  = time(NULL) - 86400;
		acc->login_fail_count = 0;

		return(true);
	}
	else if (strcmp(userid, "Too-Many") == 0)
	{
		strcpy(acc->userid, "Too-Many");
		strcpy(acc->password_hash, "someBankai");
		acc->unban_time		 = 0;
		acc->expiration_time = 0;

		return(true);
	}

	return(false);
}

tstsuite("handle_login")
{
	ip4_addr_t			 testip		 = 0;
	time_t				 testtime	 = 100;
	login_session_data_t testsession = { 0 };

	tstcase("Username Empty Login")
	{
		tstcheck(handle_login("", "Bankai2", testip, testtime, -1, &testsession) == LOGIN_FAIL_USER_NOT_FOUND)
	}

	tstcase("Correct Username and Password")
	{
		tstcheck(handle_login("Ichigo", "Bankai2", testip, testtime, -1, &testsession) == LOGIN_SUCCESS)
	}

	tstcase("Username Not Found Login")
	{
		tstcheck(handle_login("Non-existent", "Bankai2", testip, testtime, -1, &testsession) == LOGIN_FAIL_USER_NOT_FOUND)
	}

	tstcase("Account Banned Login")
	{
		tstcheck(handle_login("Banned-Account", "Bankai3", testip, testtime, -1, &testsession) == LOGIN_FAIL_ACCOUNT_BANNED)
	}

	tstcase("Account Expired Login")
	{
		tstcheck(handle_login("Expired-Account", "Bankai2", testip, testtime, -1, &testsession) == LOGIN_FAIL_ACCOUNT_EXPIRED)
	}

	tstcase("Login Failed too many tries")
	{
		// first 10 times should be normal bad password
		for (int i = 0; i < 11; i++)
		{
			tstcheck(handle_login("Too-Many", "noBankai", testip, testtime, -1, &testsession) == LOGIN_FAIL_BAD_PASSWORD)
		}

		// 11th time onward is a different message, likely account banned if it gets implemented in login.c
		tstcheck(handle_login("Too-Many", "someBankai", testip, testtime, -1, &testsession) == LOGIN_FAIL_INTERNAL_ERROR)
	}

	tstcase("Password Wrong Login + Session Check")
	{
		tstcheck(handle_login("Ichigo", "Bankai3", testip, testtime, -1, &testsession) == LOGIN_FAIL_BAD_PASSWORD)
		tstcheck(testsession.account_id == 0)
		tstcheck(testsession.session_start == 0)
		tstcheck(testsession.expiration_time == 0)
	}

	tstcase("Correct Password Login + Session Check")
	{
		tstcheck(handle_login("Ichigo", "Bankai2", testip, testtime, -1, &testsession) == LOGIN_SUCCESS)
		tstcheck(testsession.account_id == 1)
		tstcheck(testsession.session_start == testtime)
		tstcheck(testsession.expiration_time == testtime + 86400)
	}

	tstcase("Session Expiry Overflow")
	{
		login_session_data_t specsession;
		time_t spectime = INT_MAX - 86200;

		tstcheck(handle_login("Ichigo", "Bankai2", testip, spectime, -1, &specsession) == LOGIN_SUCCESS)
		tstcheck(specsession.account_id == 1)
		tstcheck(specsession.session_start == spectime)
		tstcheck(specsession.expiration_time > specsession.session_start) // Check for overflow
	}
}
