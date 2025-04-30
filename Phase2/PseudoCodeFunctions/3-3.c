/* typedef struct { */
/*   int64_t account_id; */

/*   char userid[USER_ID_LENGTH];     // User ID of up to USER_ID_LENGTH chars;
 */
/*                                    // null-terminated if less than that */

/*   char password_hash[HASH_LENGTH]; // Encodes details of a hashed password
 * (e.g. */
/*                                    // hash algorithm, salt, actual hash,
 * etc.). */
/*                                    // Is always null terminated and is of
 * length */
/*                                    // < HASH_LENGTH. */

/*   char email[EMAIL_LENGTH]; // An email address of up to EMAIL_LENGTH chars;
 */
/*                             // null-terminated if less than that */

/*   time_t unban_time;        // Ban the account up until this time (0 = no
 * ban) */
/*   time_t */

/*       expiration_time; // Account is only valid until this time (0 =
 * unlimited) */
/*   unsigned int login_count; // Number of successful auth attempts, default =
 * 0 */
/*   unsigned int */

/*       login_fail_count;   // Number of unsuccessful auth attempts, default =
 * 0 */
/*   time_t last_login_time; // Time of last successful login, default = time 0.
 */
/*   ip4_addr_t last_ip;     // Last IP connected from, default = 0 */

/*   char birthdate[BIRTHDATE_LENGTH]; // Birth date (format: YYYY-MM-DD,
 * default: */
/*                                     // 0000-00-00) */
/* } account_t; */
void account_set_email(account_t *acc, const char *new_email)
{
	// Preconds: both args are non-null & new-email has '\0'
	if (!only_ASCII_printable_chars(new_email) || !no_spaces(new_email))
	{
		log_message(LOG_ERROR, "Invalid email format in account_set_email.");

		return; // Not sure how to handle here?
	}

	// Safe copy into the email field
	strncpy(acc->email, new_email, EMAIL_LENGTH);
	acc->email[EMAIL_LENGTH - 1] = '\0'; // Ensure null-termination
}

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
		            "Invalid birthdate format. Using default '0000-00-00'.");
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

// free memory and resources used by the account
void account_free(account_t *acc)
{
	// as we are not using pointers in account_t struct, we don't need to free
	// each item in the struct (I think, feel free to fact check) -
	// https://stackoverflow.com/a/13590879 however, since this this will be
	// freeing user accounts, we probably want to do more than just free the
	// memory, instead zeroing it out so that it cannot be accessed later on -
	// probaly explicit_bzero or similar (if we use C23 then we should use
	// memset_explict)

	if (acc) // only run explicit_bzero when not null
	{
		explicit_bzero(acc, sizeof(*account_t));
	}

	free(acc);
}

// set a ban and an expire time.
void account_set_unban_time(account_t *acc, time_t t)
{
	acc->unban_time =
		t; // this seems like this should be harder, maybe im missing something??
}

// set an account expiration time
void account_set_expiration_time(account_t *acc, time_t t)
{
	acc->expiration_time = t; // ditto
}

// record a successful login. ip must be a valid IPv4 address.
// NOTE: We will need to add this to doxygen, but as this didnt specify what to
// do with errors, im going to do what account_create does - On error, logs an
// appropriate error message using log_message - doesnt return anything as void
// - does not update the last login time, but does update other values.

void account_record_login_success(account_t *acc, ip4_addr_t ip)
{
	time_t currentTime = time(NULL);

	if (currentTime == (time)(-1))
	{
		log_message("time failed")
	}
	else
	{
		acc->last_login_time = currentTime;
	}

	acc->login_fail_count = 0;
	acc->login_count	  = acc.login_count + 1;
	acc->last_ip		  = ip;
}

// record a failed login
// NOTE: We will need to add this to doxygen, but as this didnt specify what to
// do with errors, im going to do what account_create does - On error, logs an
// appropriate error message using log_message - doesnt return anything as void
// - does not update the last login time, but does update other values.

void account_record_login_failure(account_t *acc)
{
	time_t currentTime = time(NULL);

	if (currentTime == (time)(-1))
	{
		log_message("time failed")
	}
	else
	{
		acc->last_login_time = currentTime;
	}

	acc->login_fail_count = login_fail_count + 1;
	;
	acc->login_count = 0;
	acc->last_ip	 = ip;
}
