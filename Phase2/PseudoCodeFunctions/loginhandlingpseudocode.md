### Henry Log In 3.5 

login_t handle_login(const char *username, const char *password, ip_t ip, time_t login_time, int output, login_session_t *session)

### Precondition: Check That the password, username and session isn't NULL
if ((username == NULL) || (password == NULL) || session == NULL) {
    return LOGIN_INVALID;
}



