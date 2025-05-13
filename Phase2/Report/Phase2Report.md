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

### 3.3 Account Management
1. Fixed Length Fields + Null-Termination
    - This is used to prevent buffer overflow
    - It was done by using strncpy()
    - What tests?
    - How tests were made?
2. Neutralisation over Sanitisation
    - Instead of fixing inputs by deleting weird characters we just didn't allow the program to make the input at all. This prevents invalid data whilst also making sure users don't have an issue with accounts having the wrong credentials
    - Email, userid and birthday are all validated
    - What tests?
    - How tests were made?
3. Safe Memory Allocation
    - This stops credentials from leaking
    - We used explicit_bzero when freeing account struct
    - What tests?
    - How tests were made?
4. Invalid Birthdate Handling
    - Prevents fraudulent accounts (May not necessarily be a security issue in terms of the system)
    - Defaults date to 0000-00-00
    - What tests?
    - How tests were made?

### 3.4 Password Handling
1. Argon2id Hashing Algorithm
    - Used to hash paswords so that they are safe from exposure. A modern hashing algorithm that's much harder to crack than things such as bcrypt, MD5 or SHA. Combines Argon2d + Argon2i.
    - 64MB Memory, 2 Iterations
    - No tests were done for this
    - It would take far too long to try and crack as password
2. Salting Hashes
    - Salting a hash allows for ease of verification in the future
    - t_cost, m_cost, parallelism is used to salt the hash
    - What tests?
    - How tests were made?
3. Secure Comparisom
    - Prevents attackers from using timing attacks
    - secure_compare was used
    - What tests?
    - How tests were made?
4. Returning False
    - Prevents insecure states
    - Returns false when there is a fail case is met
    - What tests?
    - How tests were made?

### 3.5 Login Handling
1. If statement use
    - Separate every check so that attackers can't use a input that tricks a singular If statement that tries to cover all issues
    - Separate If statement for each precondition
    - What tests?
    - How tests were made?
2. Use of 3.4
    - Constant security feature
    - If statements just check imports from 3.4 therfore should have all the previously said features
    - What tests?
    - How tests were made?
3. File Descriptor

### 3.6 Coding Standards
1. We are using C11

### 3.7 Test Code
Threat Modelling:
    1. Buffer Overflow: Fields entered into accounts are not validated correctly leading to overflow buffers
    2. Memory Corruption: Fields entered into accounts are not validated correctly leading to memory corruption
    3. Undefined Behaviour: Fields entered into accounts are not validated correctly leading to undefined errors
    4. Injection Attacks: User information is not sanitized and therefore can be used to inject code
    5. Making sure passwords are hashed correctly (TBH we dont need to wrroy cus we are using argon2 and unless anyone wants to crack that I doubt we need to care about this)
    6. Can't use simple passwords to prevent attackers from scrolling through common passwords
    7. DoS: Attacker inentionally locking users out by trying to login many times
    8. Expired accounts are properly disabled so people can't continue using them
    9. File descriptor is properly secured so people can't see unless they are authorised
    10. Log files properly secured. Can contain passwords and other legititmate information
    11. Make sure people can't inject stuff into log files and manipulate data
    12. Properly restrict access to files to prevent unauthorised users from accessing high level accounts

Above is a list of threats that could pose an issue to our code. For testing we used this to generate possible input values that fit those criteria (e.g. Null/empty inputs causing buffer overflows). Below are examples of each.
    1.  Input:
            Username: aaaaaaa(x100000)
            Password: bbbbb(x100000)
            Email: cccccc(x10000) @gmail.com
        Reason: Making sure overly large inputs can't cause Buffer Overflow
    2.  Input:
            Username: AAA\x00\xFF\xAA
            Email: user\xFF@mail.com
        Reason: Checking that invalid strange inputs can't cause corruption by editing internal memory
    3.  Input:
            Username:
            Password: LO L
            Email: email.com
        Reason: Making sure NULL values, whitespace and invalid inputs can't cause undefined behaviour
    4.  Input:
            Username: admin'; DROP TABLE users--
            email: <script>alert('XSS');</script>
        Reason: Ensuring SQLinjection and XSS are not possible
    5.  Input:
            Password: P@ssw0rd123
            Password: 1234
        Reason: Testing that passwords come out as hashed argon2
    6.  Input:
            Password: 1234
            Password: password
            Password: qwerty
        Reason: Weak passwords should still be accepted as they will be hashed
    7.  Input:
            Many failed login
        Reason: Check the account gets banned or locked after 10 tries to prevent DoS
    8.  Input:
            Username: Expired_User
        Reason: Make sure expired user cannot login to check that accounts are disabled
    9.  Input:
            Trying to access file descriptor
        Reason: Ensure users can't get into file descriptor
    10. Input:
            Failed login attempts + Checking lofg files
        Reason: Check that users can't manipulate and look into the log files
    11. Input:
            Username: admin'; DROP TABLE users--
        Reason: Make sure users can't inject into log files and manipulate stuff
    12. Input:
            Try to access restricted files
        Reason: Make sure users don't have access to admin privileges

Then using the tst testing code we had it run unit tests to make sure that the output/error handling working properly based on the inputs given.

tst takes simple inputs and allows us to define the correct output as well as adding our own output to show messages that describe what went wrong with our code.

The reason we went with tst is because it is simple and clear.
You can split tst into tstsuite which can contain many tstcases and then a tstcheck:

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

You can split suite for each function that requires testing. Then have cases for all the different errors to be tested. Within you can have many checks for multiple values that check the case.
tstcheck also has simple functionality:

tstcase(function(values) == Correct_Output, Message);

This will give a message if the output is not correct based on boolean.

One struggle we had was what test values were needed, so as you will see at the beginning of our tst file is a hardcoded user example:
Valid Account:
    User ID:    Ichigo
    Password:   Bankai2
    Account ID: 1
Banned Account:
    User ID:    Banned-Account
    Note:       Password and ID not set as it should be impossible to get in
                regardless
Expired Account:
    User ID:    Expired-Account
    Note:       Password and ID not set as it should be impossible to get in
                regardless
Too Many Attempts Account:
    User ID:    Too-Many
    Password:   someBankai

Here are some specific tests we made for each section of the code:
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
        Check x 10:
            Input:
                Username: Too-Many
                Password: noBankai
                Input 11x
            Expected Output: LOGIN_BAD_PASSWORD
            Reason: Ensure accounts can't get in with wrong password for security of accounts
        Check 11:
            Input:
                Username: Too-Many
                Password: someBankai
                Input 11x
            Expected Output: LOGIN_FAILED_INTERNAL_ERROR
            Reason: Ensure that account signed incorrectly can't get in after 10 failed attempts
    6. Case: Password Wrong Login + Session Check
        Check:
            Input:
                Username: Ichigo
                Password: Bankai3
            Expected Output: LOGIN_FAIL_BAD_PASSWORD
            Reason: Ensures wrong passwords are not given access to the account for security
        Check: Session ID == 0 means that even the code itself is not logged in for the session
        Check: Session Start time is correct
        Check: Session Expiration Time is correct
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
