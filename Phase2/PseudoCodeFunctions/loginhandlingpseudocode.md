### Henry Login Handling 3.5 + Peter edits

```c
login_t handle_login(const char *username, const char *password, ip_t ip, time_t login_time, int output, login_session_t *session) {
    //Precondition: Check That the password, username and session isnt NULL
    if ((username == NULL) || (password == NULL) || session == NULL) {
        return NULL_INVALID;
    }


    //Allocate memory from heap
    account_t *user = malloc(sizeof(account_t));
    if (user == NULL) {
        log_message("Memory allocation failed for user");
        return MEMORY_ALLOCATION_FAILED;
    }

    //The account_lookup_by_userid() function returns a boolean true or false depending on if there was an account found
    //matching the given username, user is provided so that if an account is found the data is copied to the heap
    if (!account_lookup_by_userid(username, user)) {
        log_message("User ", username, "not found");
        client_msg(client_output_fd, "Invalid username");
        return USER_NOT_FOUND;
    }

    //Banned or Expired Accounts
    time_t current_time = get_current_time();
    //Check if current time is before unban time
    if (current_time < user -> unban_time) {
        log_message(username, " is banned");
        client_msg(client_output_fd, "Your account is banned");
        return ACCOUNT_BANNED;
    }

    //Check if account is expired (current time is after expiry and account is not unlimited)
    if (user -> expiration_time < current_time && user -> expiration_time != 0) {
        log_message(username, " account is expired");
        client_msg(client_output_fd, "Your account is expired");
        return ACCOUNT_EXPIRED;
    }

    //Login Failure Count
    if (user -> login_fail_count >= 10) {
        log_message("Too many login attempts");
        client_msg(client_output_fd, "Too many login attempts");
        return TOO_MANY_ATTEMPTS;
    }

    //Check Password
    //Password Wrong
    if(!password_valid(user, password)) {
        login_fails(user); //Increment number of failed logins
        log_message(username, " entered wrong password");
        client_msg(client_output_fd, "Wrong Password");
        return WRONG_PASSWORD;
    }

    reset_failed_logins(user); //Reset number of tries
    log_message(username, " logged in successfully");
    client_msg(client_output_fd, "Login Successful");

    start_session(user, session)//populate session struct with username, current time and session end time

    return LOGIN_SUCCESS;
}
