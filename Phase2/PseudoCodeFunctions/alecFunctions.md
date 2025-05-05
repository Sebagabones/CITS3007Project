3.3 Account management
```c
account_t *account_create(const char *userid, const char *plaintext_password, const char *email, const char *birthdate)
{
    // Precondition: all pointers are non-null and strings are null-terminated.

    if (len(userid) > USER_ID_LENGTH)
    {
        log_message(LOG_ERROR, "UserID too long.");
        return NULL;
    }
    if (len(plaintext_password) > PLAINTEXT_PASSWORD_MAX_LENGTH)
    { // Depends on how we do hashing !! (might only care about hashlength??
        log_message(LOG_ERROR, "Password too long.");
        return NULL;
    }
    if (! (only_ASCII_printable_chars(email) && no_spaces(email)))
    {	// Brief says only the above checks occur here
        log_message(LOG_ERROR, "Invalid email format.");
        return NULL;
    }

    bool birthdate_valid = date_correct_form(birthdate) && date_is_real(birthdate);
    const char *final_birthdate = birthdate_valid ? birthdate : "0000-00-00";
    if (!birthdate_valid) { // Or log info?
        log_message(LOG_WARN, "Invalid birthdate format. Using default '0000-00-00'.");
    }

    // Allocate memory
    account_t *actptr = malloc(sizeof(account_t));
    if (actptr == NULL) {
        log_message(LOG_ERROR, "Memory allocation failed.");
        return NULL;
    }

    // Assign account ID (Not sure how it happens)
    actptr->account_id = generate_account_id();

    // Copy values into struct (make sure to handle null-termination)
    strncpy(actptr->userid, userid, USER_ID_LENGTH);
    actptr->userid[USER_ID_LENGTH - 1] = '\0';

    hash_password(plaintext_password, actptr->password_hash, HASH_LENGTH); 
    // Assuming hash_password writes the result into the buffer and handles null-termination

    strncpy(actptr->email, email, EMAIL_LENGTH);
    actptr->email[EMAIL_LENGTH - 1] = '\0';

    strncpy(actptr->birthdate, final_birthdate, BIRTHDATE_LENGTH);
    actptr->birthdate[BIRTHDATE_LENGTH - 1] = '\0';

    // Set defaults
    actptr->unban_time = 0;
    actptr->expiration_time = 0;
    actptr->login_count = 0;
    actptr->login_fail_count = 0;
    actptr->last_login_time = 0;
    actptr->last_ip = 0;

    return actptr;
}
```
**Questions I have:**
- What password hashing function are we using. 
- How are account ID's generated? Do I need to be checking them here
- Whats the log format
- Can also check if email is being truncated (not currently doing that)

```c
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
	acc->email[EMAIL_LENGTH - 1] =  '\0'; // Ensure null-termination
}

```
**Questions I have:**
- It doesn't say to recheck the qualities from `create_account` but I image we have to

```c
bool account_is_banned(const account_t *acc)
{
	if (acc->unban_time == 0)
	{
		return false; // no ban
	}
	time_t current_time = time(NULL);
	if (current_time == -1)
	{  
		log_message(LOG_ERROR, "acc_banned: Failed to get the current time.");  
		return  true; // False positive probably better than false negative here  
	}
	return acc->unban_time > current_time;
	
	
}

bool  account_is_expired(const account_t *acc){
	if (acc->expiration_time == 0)
	{
		return false; // unlimited
	}
	time_t current_time;  
	current_time = time(NULL);
	if (current_time == -1)
	{  
		log_message(LOG_ERROR, "acc_expired: Failed to get the current time.");  
		return  true; // False positive probably better than false negative here  
	}
	return acc->expiration_time < current_time;
}
```

### Printing Account Summary
Complete freedom as to format, I've chosen to use `snprintf` to format the string and then write it to the file descriptor. 

```c
bool account_print_summary(const account_t *acct, int fd)
{	// Caller is required to make sure fd is valid + writeable

    char buffer[256]; // Buffer to hold the formatted string
    char *string = "Account ID: %d\nUserID: %s\nEmail: %s\nBirthdate: %s\n";

    int bytes_written = snprintf(buffer, sizeof(buffer), "%s", string);

    if (bytes_written < 0 || bytes_written >= sizeof(buffer))
    {
        log_message(LOG_ERROR, "account_print_summary: Failed to format account summary.");
        return false;
    }

    ssize_t result = write(fd, buffer, bytes_written);
    if (result == -1)
    {
        log_message(LOG_ERROR, "account_print_summary: Failed to write to file descriptor.");
        return false;
    }

    return true;
}

```
