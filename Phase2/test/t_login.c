#include "tst.h"

tstsuite("handle_login")
{
	tstcase("Username Empty")
	{
		tstcheck(handle_login("", "Bankai2", 0, 3) == LOGIN_FAIL_INTERNAL_ERROR)
	}

	tstcase("Username Not Found")
	{
		tstcheck(handle_login("Non-existent", "Bankai2", 0, 3) == LOGIN_FAIL_USER_NOT_FOUND)
	}

	tstcase("Account Banned")
	{
		tstcheck(handle_login("Banned-Account", "Bankai3", 0, 3) == LOGIN_FAIL_ACCOUNT_BANNED)
	}

	tstcase("Account Expired")
	{
		tstcheck(handle_login("Expired-Account", "Bankai2", 0, 3) == LOGIN_FAIL_ACCOUNT_EXPIRED)
	}

	tstcase("Login Failed too many tries")
	{
		tstcheck(handle_login("Account", "Bankai2", 0, 3) == LOGIN_FAIL_INTERNAL_ERROR)
	}

	tstcase("Password Wrong")
	{
		tstcheck(handle_login("Account", "Bankai2", 0, 3) == LOGIN_FAIL_BAD_PASSWORD)
	}

	tstcase("Correct Password")
	{
		tstcheck(handle_login("Ichigo", "Bankai2", 0, 3) == LOGIN_SUCCESS)
	}
}
