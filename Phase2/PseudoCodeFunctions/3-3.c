/**
 * Free memory and resources used by the account
 *
 * Releases any memory associated with an account.
 *
 * @param acc A pointer returned by `account_create`, or `NULL`.
 */
void account_free(account_t *acc)
{
// as we are not using pointers in account_t struct, we don't need to free each item in the struct (I think, feel free to fact check) - https://stackoverflow.com/a/13590879
// however, since this this will be freeing user accounts, we probably want to do more than just free the memory, instead zeroing it out so that it cannot be accessed later on - probaly explicit_bzero or similar (if we use C23 then we should use memset_explict)

	if (acc) //only run explicit_bzero when not null
	{
		explicit_bzero(acc, sizeof(*account_t));
	}

	free(acc);
}

/**
 * Set a ban and an expire time.
 *
 * Ban an account for a period of `t` time; The code in `account.h` details how this works.
 *
 * @param acc Must be non-`NULL`
 * @param t Ban the account up until this time (0 = no ban)
 */
void account_set_unban_time(account_t *acc, time_t t)
{
	acc->unban_time = t; //this seems like this should be harder, maybe im missing something??
}

//

/**
 * Set an account expiration time
 *
 * Sets the expiration time of an account until the account is no longer valid; More details in  `account.h`
 *
 * @param acc  Must be non-`NULL`
 * @param t Account is only valid until this time (0 = unlimited)
 */
void account_set_expiration_time(account_t *acc, time_t t)
{
	acc->expiration_time = t; //ditto
}

/**
 * Record a successful login.
 *
 * Update account metadata following a successful login.
 * Sets the `last_login_time` using the current system time, sets the accounts `last_ip` to the last IP address connected from, updates the accounts `login_count` and sets teh accounts `login_fail_count` to zero
 * Whenever a user logs in successfully, their `login_fail_count` is reset to 0; it’s thus a measure of the number of _consecutive_ login failures.
 *
 * @param acc Must be non-`NULL`.
 * @param ip Must be a valid IPv4 address.
 */
void account_record_login_success(account_t *acc, ip4_addr_t ip)
{
	//NOTE: We will need to add this to doxygen, but as this didnt specify what to do with errors, im going to do what account_create does -
	// On error, logs an appropriate error message using log_message - doesnt return anything as void - does not update the last login time, but does update other values.
	// but check on the help forum first

	time_t currentTime = time(NULL);

	if (currentTime == (time)(-1))
	{
		log_message("LOG_INFO", "time failed");
	}
	else
	{
		acc->last_login_time = currentTime;
	}

	acc->login_fail_count = 0;
	acc->login_count	  = acc.login_count + 1;
	acc->last_ip		  = ip;
}

/**
 * Record a failed login
 *
 * Update account metadata following a failed login.
 * Sets the `last_login_time` using the current system time, updates the accounts `login_fail_count` and sets the accounts `login_count` to zero
 * time, and the last IP address connected from be set correctly.
 * Whenever a user fails to log in successfully, their login_count is reset to 0.
 *
 * @param acc Must be non-`NULL`.
 */
void account_record_login_failure(account_t *acc)
{
	//NOTE: We will need to add this to doxygen, but as this didnt specify what to do with errors, im going to do what account_create does -
	// On error, logs an appropriate error message using log_message - doesnt return anything as void - does not update the last login time, but does update other values.
	// but check on the help forum first

	time_t currentTime = time(NULL);

	if (currentTime == (time)(-1))
	{
		log_message("time failed")
	}
	else
	{
		acc->last_login_time = currentTime;
	}

	acc->login_fail_count = login_fail_count + 1;;
	acc->login_count	  = 0;
}

/* **Questions I have:** */
/* - It doesn't say to recheck the qualities from `create_account` but I image we have to */

/**
 * Updates the account’s email address.
 *
 * @param acc Must be non-`NULL`.
 * @param new_email Must be a valid, null-terminated string & be non-`NULL`.
 */
void account_set_email(account_t *acc, const char *new_email)
{
	// Preconds: both args are non-null & new-email has '\0'
	if (!only_ASCII_printable_chars(new_email) || !no_spaces(new_email))
	{
		log_message(LOG_ERROR, "Invalid email format in account_set_email.");

		return; // Not sure how to handle here?
	}

	// Safe copy into the email field
	strncpy(acc->email, new_email, EMAIL_LENGTH); //this is safe as new_email must be null-terminated
	acc->email[EMAIL_LENGTH - 1] = '\0';          // Ensure null-termination
}

/**
 * Returns true if the account is currently banned
 *
 * Compares the current system time with the relevant field in the account structure.
 *
 * @param acc Must be non-`NULL`.
 *
 * @return Boolean representing if the account is banned or not
 */
bool account_is_banned(const account_t *acc)
{
	if (acc->unban_time == 0)
	{
		return(false); // no ban
	}

	time_t current_time = time(NULL);

	if (current_time == -1)
	{
		log_message(LOG_ERROR, "acc_banned: Failed to get the current time.");

		return(true); // False positive probably better than false negative here
	}

	return(acc->unban_time > current_time);
}

/**
 * Returns true if the account is currently expired
 *
 * Compares the current system time with the relevant field in the account structure.
 *
 * @param acc Must be non-`NULL`.
 *
 * @return Boolean representing if the account is expired or not
 */
bool account_is_expired(const account_t *acc)
{
	if (acc->expiration_time == 0)
	{
		return(false); // unlimited
	}

	time_t current_time;
	current_time = time(NULL);

	if (current_time == -1)
	{
		log_message(LOG_ERROR, "acc_expired: Failed to get the current time.");

		return(true); // False positive probably better than false negative here
	}

	return(acc->expiration_time < current_time);
}

/* **Questions I have:** */
/* -What password hashing function are we using. */
/* - How are account ID 's generated? Do I need to be checking them here */
/* - Whats the log format */
/* - Can also check if email is being truncated (not currently doing that) */

/**
 * Creates a new user account with the given parameters.
 *
 * Creates a new user account with the given parameters (and defaults for any other fields).
 * On error, returns NULL and logs an appropriate error message using log_message.
 * The caller is responsible for freeing any allocated memory with `account_free`.
 *
 * @param userid Must be valid, null-terminated strings, pointer may not be `NULL`.
 * @param plaintext_password Must be valid, null-terminated strings, pointer may not be `NULL`.
 * @param email  Must be valid, null-terminated strings, pointer may not be `NULL`.
 * @param birthdate Must be valid, null-terminated strings, pointer may not be `NULL`.
 *
 * @return A pointer to an account_t value, that may be `NULL`
 */
account_t *account_create(const char *userid, const char *plaintext_password,
                          const char *email, const char *birthdate)
{
	// Precondition: all pointers are non-null and strings are null-terminated.

	if (len(userid) > USER_ID_LENGTH)
	{
		log_message(LOG_ERROR, "UserID too long.");

		return(NULL);
	}

	if (len(plaintext_password) >
	    PLAINTEXT_PASSWORD_MAX_LENGTH) // Depends on how we do hashing !! (might
	                                   // only care about hashlength??
	{
		log_message(LOG_ERROR, "Password too long.");

		return(NULL);
	}

	if (!(only_ASCII_printable_chars(email) &&
	      no_spaces(email))) // Brief says only the above checks occur here
	{
		log_message(LOG_ERROR, "Invalid email format.");

		return(NULL);
	}

	bool birthdate_valid =
		date_correct_form(birthdate) && date_is_real(birthdate);
	const char *final_birthdate = birthdate_valid ? birthdate : "0000-00-00";

	if (!birthdate_valid) // Or log info?
	{
		log_message(LOG_WARN,
		            "Invalid birthdate format. Using default ' 0000 - 00 - 00'. ");
	}

	// Allocate memory
	account_t *actptr = malloc(sizeof(account_t));

	if (actptr == NULL)
	{
		log_message(LOG_ERROR, "Memory allocation failed.");

		return(NULL);
	}

	// Assign account ID (Not sure how it happens)
	actptr->account_id = generate_account_id();

	// Copy values into struct (make sure to handle null-termination)
	strncpy(actptr->userid, userid, USER_ID_LENGTH);
	actptr->userid[USER_ID_LENGTH - 1] = '\0';

	hash_password(plaintext_password, actptr->password_hash, HASH_LENGTH);
	// Assuming hash_password writes the result into the buffer and handles
	// null-termination

	strncpy(actptr->email, email, EMAIL_LENGTH);
	actptr->email[EMAIL_LENGTH - 1] = '\0';

	strncpy(actptr->birthdate, final_birthdate, BIRTHDATE_LENGTH);
	actptr->birthdate[BIRTHDATE_LENGTH - 1] = '\0';

	// Set defaults
	actptr->unban_time		 = 0;
	actptr->expiration_time	 = 0;
	actptr->login_count		 = 0;
	actptr->login_fail_count = 0;
	actptr->last_login_time	 = 0;
	actptr->last_ip			 = 0;

	return(actptr);
}
