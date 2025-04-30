### Henry Login Handling 3.5 + Peter edits

```c
login_result_t handle_login(const char *username, const char *password, ip_t ip, time_t login_time, int output, login_session_t *session) {
    //Allocate memory from heap
    account_t *user = malloc(sizeof(account_t));
    if (user == NULL) {
        log_message(LOG_ERROR, "Memory allocation failed for user");
        return LOGIN_FAIL_INTERNAL_ERROR;
    }

    //The account_lookup_by_userid() function returns a boolean true or false depending on if there was an account found matching the given username, user is provided so that if an account is found the data is copied to the heap
    if (!account_lookup_by_userid(username, user)) {
        log_message(LOG_INFO, "User %s not found", username);
        client_msg(client_output_fd, "Invalid username");
        return LOGIN_FAIL_USER_NOT_FOUND;
    }

    //Banned or Expired Accounts

    //Check if current time is before unban time
    if (account_is_banned(user)) {
        log_message(LOG_INFO, "User %s is banned", username);
        client_msg(client_output_fd, "Your account is banned");
        return LOGIN_FAIL_ACCOUNT_BANNED;
    }

    //Check if account is expired (current time is after expiry and account is not unlimited)
    if (account_is_expired(user)) {
        log_message(LOG_INFO, "User %s's account is expired", username);
        client_msg(client_output_fd, "Your account is expired");
        return LOGIN_FAIL_ACCOUNT_EXPIRED;
    }

    //Login Failure Count
    if (user -> login_fail_count >= 10) {
        log_message(LOG_WARN, "Too many login attempts"); //This could indicate a brute force attack but as someone that has to try at least 4 passwords before remembering which one I used a warn instead of error seems reasonable
        client_msg(client_output_fd, "Too many login attempts");
        return LOGIN_FAIL_INTERNAL_ERROR;
    }

    //Check Password
    //If Password Wrong
    if(!account_validate_password(user, password)) {
        account_record_login_failure(user); //record unsuccessful login
        log_message(LOG_INFO, "User %s entered wrong password", username);
        client_msg(client_output_fd, "Wrong Password");
        return LOGIN_FAIL_BAD_PASSWORD;
    }
    //If password correct
    account_record_login_success(user, client_ip); //record successful login with ip and reset unsuccessful logins (assuming both will be done in the function)
    log_message(LOG_INFO, "User %s logged in successfully", username);
    client_msg(client_output_fd, "Login Successful");

    //populate session struct with username, login time and session end time
    session -> account_id = user -> account_id;
    session -> session_start = login_time;
    session -> expiration_time = login_time + 86400;//Maximum session length 24 hours, usually session closes when closing game

    return LOGIN_SUCCESS;
}
