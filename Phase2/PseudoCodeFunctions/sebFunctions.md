## Pseudo code functions
Pseudo code functions for 3.3, Seb's

def of `account_t`
```c
typedef struct
{
    int64_t      account_id;
    char         userid[USER_ID_LENGTH];      // User ID of up to USER_ID_LENGTH chars; null-terminated if less than that
    char         password_hash[HASH_LENGTH];  // Encodes details of a hashed password (e.g. hash algorithm, salt, actual hash,
                                              // etc.).
                                              // Is always null terminated and is of length < HASH_LENGTH.
    char         email[EMAIL_LENGTH];         // An email address of up to EMAIL_LENGTH chars; null-terminated if less than that
    time_t       unban_time;                  // Ban the account up until this time (0 = no ban)
    time_t       expiration_time;             // Account is only valid until this time (0 = unlimited)
    unsigned int login_count;                 // Number of successful auth attempts, default = 0
    unsigned int login_fail_count;            // Number of unsuccessful auth attempts, default = 0
    time_t       last_login_time;             // Time of last successful login, default = time 0.
    ip4_addr_t   last_ip;                     // Last IP connected from, default = 0
    char         birthdate[BIRTHDATE_LENGTH]; // Birth date (format: YYYY-MM-DD, default: 0000-00-00)
} account_t;
```

### Account memory free
```c
// free memory and resources used by the account
void account_free(account_t *acc)
{
// as we are not using pointers in account_t struct, we don't need to free each item in the struct (I think, feel free to fact check) - https://stackoverflow.com/a/13590879
// however, since this this will be freeing user accounts, we probably want to do more than just free the memory, instead zeroing it out so that it cannot be accessed later on - probaly explicit_bzero or similar (if we use C23 then we should use memset_explict)

    if(acc) //only run explicit_bzero when not null
    {
        explicit_bzero(acc, sizeof(*account_t));
    }
    free(acc);
}
```

#### Preconditions:
- `acc` must be a pointer returned by `account_create`, or `NULL`.

Releases any memory associated with an account.


### Metadata updates
```c
// set a ban and an expire time.
void account_set_unban_time(account_t *acc, time_t t)
{
    acc->unban_time = t; //this seems like this should be harder, maybe im missing something??
}
```

```c
// set an account expiration time
void account_set_expiration_time(account_t *acc, time_t t)
{
    acc->expiration_time = t; //ditto
}
```

#### Preconditions:
-  `acc` must be non-`NULL`

Set the unban or expiration time of an account; look at the code in `account.h` for details of how these work.



### Login tracking
```c
// record a successful login. ip must be a valid IPv4 address.
//NOTE: We will need to add this to doxygen, but as this didnt specify what to do with errors, im going to do what account_create does - On error, logs an appropriate error message using log_message - doesnt return anything as void - does not update the last login time, but does update other values.

void account_record_login_success(account_t *acc, ip4_addr_t ip)
{
    time_t currentTime = time(NULL);
    if(currentTime == (time)(-1))
    {
        log_message("time failed")
    }
    else
    {
        acc->last_login_time = currentTime;
    }
    acc->login_fail_count = 0;
    acc->login_count = acc.login_count + 1;
    acc->last_ip = ip;
}
```


```c
// record a failed login
//NOTE: We will need to add this to doxygen, but as this didnt specify what to do with errors, im going to do what account_create does - On error, logs an appropriate error message using log_message - doesnt return anything as void - does not update the last login time, but does update other values.


void account_record_login_failure(account_t *acc)
{
    time_t currentTime = time(NULL);
    if(currentTime == (time)(-1))
    {
        log_message("time failed")
    }
    else
    {
        acc->last_login_time = currentTime;
    }
    acc->login_fail_count = login_fail_count + 1;;
    acc->login_count = 0;
    acc->last_ip = ip;
}
```
#### Preconditions:
- `acc` must be non-`NULL`.

Update account metadata following either a successful or a failed login.
This requires that any failure or logon counts be set correctly, that the last login time be set using the current system time, and the last IP address connected from be set correctly.
Whenever a user logs in successfully, their `login_fail_count` is reset to 0; it’s thus a measure of the number of _consecutive_ login failures.
Likewise, whenever a user fails to log in successfully, their `login_count` is reset to 0.
