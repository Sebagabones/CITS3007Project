### Henry Log In 3.5 

```c
login_t handle_login(const char *username, const char *password, ip_t ip, time_t login_time, int output, login_session_t *session) {
    '''Precondition: Check That the password, username and session isnt NULL'''
    if ((username == NULL) || (password == NULL) || session == NULL) {
        return NULL_INVALID;
    }


    '''Look up the user'''
    user_t *user = account_lookup_by_userid(username);
    if (user = NULL) {
        log_message("User ", username, "not found");
        client_msg(client_output_fd, "Invalid username");
        return USER_NOT_FOUND;
    }

    '''Banned or Expired Accounts'''
    time_t current_time = get_current_time();
    '''Check if account is banned'''
    if (user -> account_banned) {
        log_message(username, " is banned");
        client_msg(client_output_fd, "Your account is banned");
        return ACCOUNT_BANNED;
    }

    '''Check if account is expired'''
    if (user -> account_expired < current_time) {
        log_message(username, " account is expired");
        client_msg(client_output_fd, "Your account is expired");
        return ACCOUNT_EXPIRED;
    }

    '''Login Failure Count'''
    if (user -> login_failures >= 10) {
        log_message("Too many login attempts");
        client_msg(client_output_fd, "Too many login attempts");
        return TOO_MANY_ATTEMPTS;
    }

    '''Check Password'''
    '''Password Wrong'''
    if(!password_valid(user, password)) {
        login_fails(user); '''Increment number of failed logins'''
        log_message(username, " entered wrong password");
        client_msg(client_output_fd, "Wrong Password");
        return WRONG_PASSWORD;
    }

    reset_failed_logins(user); '''Reset number of tries'''
    log_message(username, " logged in successfully");
    client_msg(client_output_fd, "Wrong Password");

    return LOGIN_SUCCESS;
}
```

