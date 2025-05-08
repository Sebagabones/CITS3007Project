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
login_result_t handle_login(const char *username, const char *password, ip_t ip, time_t login_time, int output, login_session_t *session)
{
	//Allocate memory from the heap
	account_t *user = malloc(sizeof(account_t));

	if (user == NULL)
	{
		log_message(LOG_ERROR, "Memory allocation failed for user");

		return(LOGIN_FAIL_INTERNAL_ERROR);
	}

	//Retrieve account information for the username
	//Data is copied into 'user' when information is found
	if (!account_lookup_by_userid(username, user))
	{
		log_message(LOG_INFO, "User is not found");

		//Write message to file descriptor: User doesn't exist
		return(LOGIN_FAIL_USER_NOT_FOUND);
	}

	// === Banned or Expired Accounts ===

	
	//Check if the account for the user is banned
	if (account_is_banned(user))
	{
		log_message(LOG_INFO, "User is banned");

		//Write message to file descriptor: Account is Banned
		return(LOGIN_FAIL_ACCOUNT_BANNED);
	}

	//Check if the account for the user is expired
	if (account_is_expired(user))
	{
		log_message(LOG_INFO, "User account is expired");

		//Write message to file descriptor: Account is Expired
		return(LOGIN_FAIL_ACCOUNT_EXPIRED);
	}

	// === Too Many Attempts ===

	//Checks if the failed login attempts exceed 10 attempts
	if (user->login_fail_count >= 10)
	{
		log_message(LOG_WARN, "Too many login attempts"); //This could indicate a brute force attack but as someone that has to try at least 4 passwords before remembering which one I used a warn instead of error seems reasonable

		//Write message to file descriptor: Too Many login attempts
		return(LOGIN_FAIL_INTERNAL_ERROR);
	}

	// === Password Check ===

	
	//Checks if the password is correct
	//If password wrong increment count for attempts
	if (!account_validate_password(user, password))
	{
		account_record_login_failure(user); //record unsuccessful login
		log_message(LOG_INFO, "User entered wrong password");

		//Write message to file descriptor: Password wrong
		return(LOGIN_FAIL_BAD_PASSWORD);
	}

	// === Login Successful ===

	//Reset failure count when login successful
	account_record_login_success(user, client_ip); //record successful login with ip and reset unsuccessful logins (assuming both will be done in the function)
	//Write message to file descriptor: Log in Successful
	log_message(LOG_INFO, "User logged in successfully");

	// === Populate the Session Data ===
	session->account_id		 = user->account_id;
	session->session_start	 = login_time;
	session->expiration_time = login_time + 86400;  //Maximum session length 24 hours, usually session closes when closing game

	// === Login Successful Return ===
	return(LOGIN_SUCCESS);
}
