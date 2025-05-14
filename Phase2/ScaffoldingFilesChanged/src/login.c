#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <ctype.h>

#include "db.h"
#include "login.h"
#include "logging.h"
#include "banned.h"

#define DEFAULT_SESSION_LENGTH    86400

/**
 * @file   login.c
 *
 * @brief Implementation of the functions that handle user login
 *
 */
#include <errno.h>  // for errno
#include <string.h> // for strerror()

static void send_client_and_log(int client_fd, log_level_t level, const char *msg)
{
	if (msg)
	{
		size_t	len		= strlen(msg);
		ssize_t written = write(client_fd, msg, len);

		if (written < 0)
		{
			log_message(LOG_ERROR, "Failed to write to client_fd: %s", strerror(errno));
		}
		else if ((size_t)written < len)
		{
			log_message(LOG_WARN, "Partial write to client_fd: wrote %zd of %zu bytes", written, len);
		}
		else
		{
			log_message(LOG_DEBUG, "Sent message to client_fd: %s", msg);
		}

		log_message(level, msg);
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
		log_message(LOG_DEBUG, "handle_login: malloc failed for user struct");
		log_message(LOG_ERROR, "Memory allocation failed for user");

		return(LOGIN_FAIL_INTERNAL_ERROR);
	}

	//The account_lookup_by_userid() function returns a boolean true or false depending on if there was an account found matching the given username, user is provided so that if an account is found the data is copied to the heap
	if (!account_lookup_by_userid(username, user))
	{
		send_client_and_log(client_output_fd, LOG_INFO, "Login failed: User account doesn't exist");
		account_free(user);

		return(LOGIN_FAIL_USER_NOT_FOUND);
	}

	//Banned or Expired Accounts

	//Check if current time is before unban time
	if (account_is_banned(user))
	{
		send_client_and_log(client_output_fd, LOG_INFO, "Login failed: User account is banned");
		account_free(user);

		return(LOGIN_FAIL_ACCOUNT_BANNED);
	}

	//Check if account is expired (current time is after expiry and account is not unlimited)
	if (account_is_expired(user))
	{
		send_client_and_log(client_output_fd, LOG_INFO, "Login failed: User account is expired");
		account_free(user);

		return(LOGIN_FAIL_ACCOUNT_EXPIRED);
	}

	//Login Failure Count
	if (user->login_fail_count > 10)
	{
		send_client_and_log(client_output_fd, LOG_WARN, "Login failed: too many failed password attempts");
		account_free(user);

		return(LOGIN_FAIL_INTERNAL_ERROR);
	}

	//If Password Wrong
	if (!account_validate_password(user, password))
	{
		account_record_login_failure(user); //record unsuccessful login
		log_message(LOG_DEBUG, "handle_login: wrong password");
		send_client_and_log(client_output_fd, LOG_INFO, "Login failed: Wrong password");
		account_free(user);

		return(LOGIN_FAIL_BAD_PASSWORD);
	}

	//If password correct
	account_record_login_success(user, client_ip); //record successful login with ip and reset unsuccessful logins (assuming both will be done in the function)
	send_client_and_log(client_output_fd, LOG_INFO, "Login Successful");

	//populate session struct with username, login time and session end time
	session->account_id	   = (int)user->account_id;
	session->session_start = login_time;

	if (user->expiration_time == 0)
	{   // If no expiration time, set to 24hrs
		session->expiration_time = login_time + DEFAULT_SESSION_LENGTH;
	}
	else
	{
		session->expiration_time = user->expiration_time;
	}

	log_message(LOG_DEBUG, "handle_login: login process completed successfully for user");
	account_free(user);

	return(LOGIN_SUCCESS);
}
