### Group Name:
TOCTOU *(/tɒk ˈtuːə/)*

#### Group Number:
23

#### Group Members:
| Student Number | Title | First Name | Last Name |
|----------------|-------|------------|-----------|
| 23832656       | Mr    | Peter      | Fang      |
| 23417131       | Mx    | Seb        | Gazey     |
| 23804015       | Mr    | Alec       | Hassell   |
| 23478063       | Mr    | Scotty     | Maw       |
| 23926055       | Lord  | Henry      | Yau       |


### Design Decisions
What design decisions did you have to make? How and why did you decide on the approach you chose?
1. Static helper functions for sanitising/neutralising email and birthday
    - Hidden from other files, prevents misuse
    - Reduces duplicate code/features

2. Used memcpy for majority of char* copying
    - Due to 'partial-pseudo-string' fields in account
    - Avoids Buffer Overflows by copying specified lengths
    - Doesn't scan for null terminator
    - Better control over partial strings being copied

3. memcpy was implemented in a larger `buffer_to_cstring` function for partial-string capability
    - Copies exact length of buffer
    - Null Terminator is appended manually
    - Protects from unterminated buffers
    - Prevents Undefined behaviour

4. `explicit_bzero` securely erases sensitive data in memory after it has been used
    - Prevent sensitive data from staying within memory by overwriting with zeroes
    - Forces compiler to wipe the memory

5. Account Field Validation was done by following the default specs from lecture
    - Instead of sanitisation we chose to neutralise any attempts with incorrect fields as we believe it's safer

6. Salting and Argon2
    - Salting with randomly generated salt for each password to prevent rainbow attacks
    - Argon2id has balanced protection against brute-force and side-channel attacks
    - plus more from Scotty if needed

7. Writing to file
    - Seb stuff here

8. `ip_to_cstring`
    - Older version inet_ntoa() is not thread safe, it uses shared static buffer across calls meaning race conditions are caused when multiple threads call it at the same time
    - We used inet_ntop because it is thread safe since it uses caller provided buffers, and it supports multiple protocols like IPv6

9. We used asprintf() because it's safe and convenient
    - Avoid Buffer Overflows by dynamically allocating the buffer size
    - Improve readability by saving space normally used for manual memory calculation
    - Improve safety by returning -1 upon failure

10. Session expiry time
    - We chose for the session expiry time to be the user account expiry time, or 24 hours in the cases where there is none
    - We decided on this after group discussion due to the complete lack of mention in the project spec

11. Session clearing
    - We decided to clear the session struct when there is a failed login

12. Casting user->account_id to int
    - Due to a mysterious case of inconsistent int types between the user struct and the session struct we had to cast user->account_id to int
    - If only there was a way to fix this slight inconsistency by removing 4 characters from account.h but alas we were not able to use this method


### 3.6 Coding Standards
1. We are using C11

### 3.7 Test Code
Threat Modelling:
1. Buffer Overflow: Fields entered into accounts are not validated correctly leading to overflow buffers
2. Memory Corruption: Fields entered into accounts are not validated correctly leading to memory corruption
3. Undefined Behaviour: Fields entered into accounts are not validated correctly leading to undefined errors
4. Injection Attacks: User information is not sanitized and therefore can be used to inject code
5. Making sure passwords are hashed correctly (TBH we dont need to worry cus we are using argon2 and unless anyone dedicates their life to crack that I doubt we need to care about this)
6. Can't use simple passwords to prevent attackers from scrolling through common passwords
7. DoS: Attacker inentionally locking users out by trying to login many times
8. Expired accounts are properly disabled so people can't continue using them
9. File descriptor is properly secured so people can't see unless they are authorised
10. Log files properly secured. Can contain passwords and other legititmate information
11. Make sure people can't inject stuff into log files and manipulate data <- very similar to 10
12. Properly restrict access to files to prevent unauthorised users from accessing high level accounts

Above is a list of threats that could pose an issue to our code. For testing we used this to generate possible input values that fit those criteria (e.g. Null/empty inputs causing buffer overflows). Below are examples of each.
1.  Input:
        Username: aaaaaaa(x100000)
        Password: bbbbb(x100000)
        Email: cccccc(x10000) @gmail.com
    Process:
        Check that running account_create() with any of these invalid inputs will return null instead of a pointer to a user (partially not implemented yet)
    Reason: Making sure overly large inputs can't cause Buffer Overflow
2.  Input:
        Username: AAA\x00\xFF\xAA
        Email: user\xFF@mail.com
    Process:
        Check that running account_create() with any of these invalid inputs will return null instead of a pointer to a user (not implemented yet)
    Reason: Checking that invalid strange inputs can't cause corruption by editing internal memory
3.  Input:
        Username:
        Password: LO L
        Email: email.com
    Process:
        Check that running account_create() with any of these invalid inputs will return null instead of a pointer to a user (not implemented yet)
    Reason: Making sure NULL values, whitespace and invalid inputs can't cause undefined behaviour
4.  Input:
        Username: admin'; DROP TABLE users--
        email: <script>alert('XSS');</script>
    Process:
        We do not have a db to test this on, probably just run account_create() and have it return null
    Reason: Ensuring SQLinjection and XSS are not possible
5.  Input:
        Correct Password: P@ssw0rd123
        Incorrect Password: 1234
    Process:
        Check that after running account_update_password() on the correct password, running !account_validate_password() on the wrong password returns true
    Reason: Testing that argon2 password hashing is working
6.  Input:
        Password: 1234
        Password: password
        Password: qwerty
    Process:
        Check that after running account_update_password() on the correct password, running !account_validate_password() on the wrong password returns true (not implemented)
    Reason: Weak passwords should still be accepted as they will be hashed <- might be better as a design choice than a threat
7.  Input:
        login_fail_count: 11
        Username: Too-Many
        Password: someBankai
    Process:
        Check that after running handle_login() LOGIN_FAIL_INTERNAL_ERROR is returned
    Reason: Check the account gets banned or locked after more than 10 failed logins to prevent DoS
8.  Input:
        Username: Expired_User
    Process:
        Check that after running handle_login() LOGIN_FAIL_ACCOUNT_EXPIRED is returned
    Reason: Make sure expired user cannot login to check that accounts are disabled
9.  Input:
        Trying to access file descriptor
    Reason: Ensure users can't get into file descriptor
10. Input:
        Failed login attempts + Checking log files
    Reason: Check that users can't manipulate and look into the log files
11. Input:
        Username: admin'; DROP TABLE users--
    Reason: Make sure users can't inject into log files and manipulate stuff <- very similar to 10
12. Input:
        Try to access restricted files
    Reason: Make sure users don't have access to admin privileges

We used the tst testing framework to run unit tests to make sure that the output/error handling working properly based on the inputs given.

Tst takes simple inputs and allows us to define the correct output as well as adding our own output to show debug messages.

The reason we went with tst is because it is simple and clear.
You can split tst into testsuites which can contain many testcases each with their own testchecks:

tstsuite(){
    tstcase(1) {
        tstcheck(1);
        tstcheck(2);
        .
        .
        .
    }
    .
    .
}

You can split suite for each function that requires testing. Then have cases for all the different functions to be tested.

Additionally, we had our own stubs.c file that allows us to use mocks of functions that were not required to be written by us, including log_message() and account_lookup_by_userid(). We did not use the given stubs.c inside src/ because that one gets overwritten and we wanted different functionalities such as custom debug messages.

tstcheck also has simple functionality:

tstcase(function(values) == Correct_Output, Message);

This will give a message if the output is not correct based on boolean.

One struggle we had was figuring out how to simulate db querying for account_lookup_by_userid, in the end we decided to hardcode a few test users as shown in the stubs.c file:
Valid Account:
    User ID:    Ichigo
    Password:   Bankai2
    Account ID: 1
Banned Account:
    User ID:    Banned-Account
    unban_time: time() + 86400
    Note:       Password and ID not set as it should be impossible to get in
                regardless
Expired Account:
    User ID:    Expired-Account
    expiration_time: time() + 86400
    Note:       Password and ID not set as it should be impossible to get in
                regardless
Too Many Attempts Account:
    User ID:    Too-Many
    Password:   someBankai
    login_fail_count: 11

Here are some specific tests we made for each section of the code: <-- Kind of duplicate with previous section? May want to only keep one
handle_login():
    1. Case: Username Empty Login
        Input:
            Username:
            Password: Bankai2
        Expected Output: LOGIN_USER_NOT_FOUND
        Reason: Ensures that the code doesn't crash but also gracefully handle failed memory allocation
    2. Case: Username Not Found Login
        Input:
            Username: Non-existent
            Password: Bankai2
        Expected Output: LOGIN_FAIL_USER_NOT_FOUND
        Reason: Ensures system handles a non existent account gracefully without crashing
    3. Case: Account Banned Login
        Input:
            Username: Banned-Account
            Password: Bankai3
        Expected Output: LOGIN_FAIL_ACCOUNT_BANNED
        Reason: Ensures system handles banned accounts and prevents login
    4. Case: Account Expired Login
        Input:
            Username: Expired-Account
            Password: Bankai2
        Expected Output: LOGIN_FAIL_ACCOUNT_EXPIRED
        Reason: Prevents expired accounts from logging in gracefully with the error
    5. Case: Login Failed too many tries
        Input:
            Username: Too-Many
            Password: someBankai
        Expected Output: LOGIN_FAILED_INTERNAL_ERROR
        Reason: Ensure that account signed incorrectly can't get in after 10 failed attempts
    6. Case: Password Wrong Login + Session Check
        Check:
            Input:
                Username: Ichigo
                Password: Bankai3
            Expected Output: LOGIN_FAIL_BAD_PASSWORD
            Reason: Ensures wrong passwords are not given access to the account for security
        Check: Session ID == -1 means that session is reset for failed login
        Check: Session Start time == -1
        Check: Session Expiration Time == -1
    7. Case: Correct Password Login + Session Check
        Check:
            Input:
                Username: Ichigo
                Password: Bankai2
            Expected Output: LOGIN_SUCCESS
            Reason: Ensures user can log in correctly
        Check: Session ID == 1 means the user id 1 is logged in
        Check: Session Start time is correct
        Check: Session Expiration Time is correct
    8. Case: Session Expiry Overflow
        We set a default maximum time here
        Check:
            Input:
                Username: Ichigo
                Password: Bankai2
            Expected Output: LOGIN_SUCCESS
        Check: Account session is 1
        Check: Session start time is correct
        Check: Check for overflow in expiration time

