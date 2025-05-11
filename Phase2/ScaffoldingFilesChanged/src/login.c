#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>

#include "db.h"
#include "login.h"
#include "banned.h"
#include "logging.h"

/**
 * @file   login.c
 *
 * @brief Implementation of the functions that handle user login
 *
 */

static void send_client_and_log(int client_fd, log_level_t level, const char *username, const char *client_msg, const char *log_fmt)
{
	if (client_msg)
	{
		write(client_fd, client_msg, strlen(client_msg));
	}

	if (log_fmt)
	{
		log_message(level, log_fmt, username);
	}
}

/**
 * login_result_t  handles the login process
 *
 * @param username		The username of the account logging in
 * @param password		The password being entered
 * @param ip			The IP address of user logging in
 * @param login_time	The server time currently
 * @param output		The file descriptor for message sent to client/user
 * @param session		The Pointer to the session structure. Used to populate when login successfule
 *
 * @return login_result_t
 *		- LOGIN_SUCCESS: Login successful
 *		- LOGIN_FAIL_USER_NOT_FOUND: Account not found
 *		- LOGIN_FAIL_ACCOUNT_BANNED: Account is banned
 *		- LOGIN_FAIL_ACCOUNT_EXPIRED: Account is expired
 *		- LOGIN_FAIL_BAD_PASSWORD: Password is incorrect
 *		- LOGIN_FAIL_INTERNAL_ERROR: Too many login tries or Memory allocation failure
 */
login_result_t handle_login(const char *username, const char *password,
                            ip4_addr_t client_ip, time_t login_time,
                            int client_output_fd,
                            login_session_data_t *session)
{
	//Allocate memory from heap
	account_t *user = malloc(sizeof(account_t));

	if (user == NULL)
	{
		log_message(LOG_ERROR, "Memory allocation failed for user");

		return(LOGIN_FAIL_INTERNAL_ERROR);
	}

	//The account_lookup_by_userid() function returns a boolean true or false depending on if there was an account found matching the given username, user is provided so that if an account is found the data is copied to the heap
	if (!account_lookup_by_userid(username, user))
	{
		send_client_and_log(client_output_fd, LOG_INFO, username,
		                    "Login failed: User account doesn't exist\n",
		                    "User %s not found");
		free(user);

		return(LOGIN_FAIL_USER_NOT_FOUND);
	}

	//Banned or Expired Accounts

	//Check if current time is before unban time
	if (account_is_banned(user))
	{
		send_client_and_log(client_output_fd, LOG_INFO, username,
		                    "Login failed: User account is banned\n",
		                    "User %s is banned");
		free(user);

		return(LOGIN_FAIL_ACCOUNT_BANNED);
	}

	//Check if account is expired (current time is after expiry and account is not unlimited)
	if (account_is_expired(user))
	{
		send_client_and_log(client_output_fd, LOG_INFO, username,
		                    "Login failed: User account is expired\n",
		                    "User %s's account is expired");
		free(user);

		return(LOGIN_FAIL_ACCOUNT_EXPIRED);
	}

	//Login Failure Count
	if (user->login_fail_count > 10)
	{
		send_client_and_log(client_output_fd, LOG_WARN, username,
		                    "Login failed: too many failed password attempts\n",
		                    "Too many login attempts for user %s");
		free(user);

		return(LOGIN_FAIL_INTERNAL_ERROR);
	}

	//Check Password
	//If Password Wrong
	if (!account_validate_password(user, password))
	{
		account_record_login_failure(user); //record unsuccessful login
		send_client_and_log(client_output_fd, LOG_INFO, username,
		                    "Login failed: Wrong password\n",
		                    "User %s entered wrong password");
		free(user);

		return(LOGIN_FAIL_BAD_PASSWORD);
	}

	//If password correct
	account_record_login_success(user, client_ip); //record successful login with ip and reset unsuccessful logins (assuming both will be done in the function)
	send_client_and_log(client_output_fd, LOG_INFO, username,
	                    "Login Successful\n",
	                    "User %s logged in successfully");

	//populate session struct with username, login time and session end time
	session->account_id		 = user->account_id;
	session->session_start	 = login_time;
	session->expiration_time = login_time + 86400;  //Maximum session length 24 hours, usually session closes when closing game

	free(user);

	return(LOGIN_SUCCESS);
}
