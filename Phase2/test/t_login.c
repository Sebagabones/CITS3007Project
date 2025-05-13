#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#include "tst.h"
#include "account.h"
#include "login.h"
#include "logging.h"
#include "db.h"
#include "pwHandling.h"

tstsuite("handle_login")
{
	ip4_addr_t			 testip		 = 0;
	time_t				 testtime	 = 100;
	login_session_data_t testsession = { 0 };

	tstcase("Username Empty Login")
	{
		tstcheck(handle_login("", "Bankai2", testip, testtime, -1, &testsession) == LOGIN_FAIL_USER_NOT_FOUND)
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
		// check if trying to log in after more than 10 failed attempts will fail with the correct code
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
	testtime = time(NULL); //this needs to be updated because successful login
	tstcase("Correct Username and Password")
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
