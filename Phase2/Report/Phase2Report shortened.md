### Group Name:
TOCTOU *(/tɒk ˈtuːə/)*

#### Group Number:
23

#### Group Members:
| Student Number | Title |   First Name   | Last Name |
|----------------|-------|----------------|-----------|
| 23832656       | Mr    | Peter          | Fang      |
| 23417131       | Mx    | Seb            | Gazey     |
| 23804015       | Mr    | Alec           | Hassell   |
| 23478063       | Mr    | Minn Khant     | Maw       |
| 23926055       | Lord  | Henry          | Yau       |


### Design Decisions
What design decisions did you have to make? How and why did you decide on the approach you chose?
1. Static helper functions for sanitising/neutralising inputs, logging, as well as hashing
    - We made many static helper functions to make our code more readable by reducing the instances of duplicate code
    - Since they are static, they are hidden from other files preventing potential misuse

2. Used `memcpy` for majority of char* copying
    - We did this because it offers better control of the 'partial-pseudo-string' fields in `account.c` – which according to the spec are not *necessarily* null terminated
    - Also Avoids Buffer Overflows by copying specified lengths
    - Null Terminator is appened manually
    - Prevents Undefined behaviour

3. We implemented `explicit_bzero` to securely erases sensitive data in memory after it has been used
    - Prevent sensitive data from staying within memory by overwriting with zeroes
    - This is necessary because sometimes compilers gloss over freed memory and do not actually wipe it, while `explicit_bzero` forces the compiler to wipe this potentially sensitive data

4. For Account Field Validation we neutralise rather than sanitise
    - We decided it was more simple and sensible to reject invalid inputs rather than trying to fix them
    - To do this we included many validation functions including `password_valid`, `birthday_valid` and `only_ASCII_printable_chars`

5. Salting and Hashing using Sodium and Argon2
    - Salting using the Sodium library for each password to prevent rainbow table attacks
    - Argon2id has balanced protection against brute-force and side-channel attacks
    - Due to the possibility of brute forcing the hash as well, we have chosen to use Sodium's constant time comparison function to compare the computed hash and the hash stored.
    - In order to be more efficient with calculating the hashes, Argon2 provides us with the ability to use multi-threading to make use of multiple CPU cores (in our case 4 cores) to calculate or validate the hashes for very long passwords.
    - We employed the standards of Argon2's recommended parameters from their documentation, which are:
        - Memory cost: 65536 KiB
        - Time cost: 4 iterations
        - Parallelism: 4 threads
    - We enforced a minimum password length of 8 characters, and a maximum of 256 characters. The minimum length is to prevent brute force attacks – as short passwords are weaker even when salted and hashed. The maximum length serves to prevent the user from entering a password that is too long, which could cause issues with memory allocation and performance. We also enforce a minimum of 1 uppercase letter, 1 lowercase letter, 1 number, and 1 symbol to ensure that the password is strong enough to resist brute force attacks.
    - A main motivator of using Argon2 is that it is the winner of the Password Hashing Competition (PHC) and is widely considered to be the most secure password hashing algorithm available. It is very modern and industry standard. 

6. Writing to file
    - We used write to file instead of using `fprintf` because it is more efficient, and we are given a file descriptor
    - For printing the account summary *solely*, we employ a mutex lock to prevent a data race condition. We do this by locking the mutex before writing to the file and unlocking it after we are done writing. This ensures that only one thread can write to the file at a time, preventing any potential data corruption or inconsistencies
    - We did this because we believe the ACS would best be used in a multi-threaded environment, where it can handle multiple requests simultaneously. By using a mutex lock, we can ensure that the ACS can handle multiple requests to create accounts/log people in etc without issue – and printing the account summary won't block any of those more frequent requests. 
    - We did not use a mutex for the client fd printing, because we believe that the client wouldn't be sending multiple requests quickly enough to cause a race condition.

7. Using `inet_ntop` in `ip_to_cstring`
    - Previous version `inet_ntoa` is not thread safe, it uses shared static buffer across calls meaning race conditions are caused when multiple threads call it at the same time
    - We used `inet_ntop` because it is thread safe since it uses caller provided buffers, and it supports multiple protocols like IPv6

8. We used `asprintf` over alternatives like `sprintf`/`snprintf`
    - `asprintf` avoid Buffer Overflows by dynamically allocating the buffer size
    - Also improves readability by saving space normally used for manual memory calculation for alternative printf functions
    - It improves safety by returning -1 upon failure and the number of bytes written on success

9. Session expiry time
    - We chose for the session expiry time to be the user account expiry time, or 24 hours in the cases where there is none
    - The reason for this is there is nothing else related session expiry time so we simply assumed that the user account expiry and the session expiry are the same thing

10. Not using `LOGIN_FAIL_IP_BANNED`
    - We decided not to use IP bans because of several reasons
    - For one thing everyone uses dynamic IP addresses nowadays so just by simply reconnecting they will bypass the ban
    - Even if one does not use dynamic IPs, VPNs exist to do the same thing
    - Since the given IP is IPv4, due to the scarcity ISPs use CGNAT meaning banning one ip can affect thousands of people which is exactly what you don't want when your product is kept alive by said people

11. Passing 10 failed logins will soft ban the account
    - We implemented an enforced login fail when there's more than 10 consecutive failed logins in a row, meaning the account will be impossible to access
    - We intend there to be an email sent out to the account owner prompting them to change their userid/password in order to unlock their account, however this is impossible to implement in our given scope
    - The alternative is to ban the account for a short period of time but we decided against that because of two reasons
    - One there is only one ban status, and it is for real bans where the only method of appealing is through presumably some customer service portal which is obviously too severe and inconvenient for just 10 consecutive failed logins
    - Two if we only ban the account for a short period of time there is nothing preventing the harasser to do it again after the ban expires
    - As a side note all of these problems would not even be problems if logins used emails (which are private) rather than userid (which is public) in the first place. This is because an attacker wouldn't be able to target an account user by username and DOS his account, violating it's accessibility.

12. Validation for password
    - We decided to add a few enforced rules to passwords being created, namely requiring the password to have at least a number, a lower case letter, an upper case letter and a symbol. 
    - This was done to prevent users from choosing passwords that may be vulnerable to some form of targetted password guessing attack.
    - After all, no matter how strong our hashing is it won't matter if someone socially engineers the password out of the user.

13. Accepting risk of certin overflows
    - We decided to accept the risk of certain overflows in our code, such as the `account_id` and `session_id` fields in the `account` and `session` structs respectively.
    - This is because we are not expecting to have more than 2^32 users or sessions at any given time, and we should have a long time before we reach that limit.
    - Note we had to cast the user's account_id int64_t to an int to prevent the implicit conversion warning – but this seems to be a problem with the spec, as there seems to be no reason for it to be half the size of the account_id

14. Type conversions from long int to int32_t in `extract_hash_components`
    - In this function there were multiple instances of the compiler warning us of the possibility of truncations during the conversions.
    - We chose to accept this risk because these conversions were being done on values that would dictate the time and memory usage of the hashing algorithm, which in our project and in the real world would never reach the LONG_MAX limit of 9223372036854775807 (in a modern 64bit machine); and because Argon2 takes those values in KiB, it is simply not possible to have a memory allocation of 8388608 Pebibytes.
    - And to make us feel safe, even though it is not necessary, we implemented a range check where these type conversion warnings came up to ensure an abundance of safety.

### Testing decisions

We used the tst testing framework to run unit tests to make sure that the output/error handling working properly based on the inputs given.

Tst takes simple inputs and allows us to define the correct output as well as adding our own output to show debug messages.

The reason we went with tst is because it is very simple to implement while retaining most of the features of more complicated test frameworks.
Unlike other bulky test frameworks with complex build dependencies, all we need to do for tst is to include `tst.h` and we're good to go.

Tst can split into testsuites which can contain many testcases each with their own testchecks, for example:

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

This means we can split a suite for each functionality that requires testing, then have cases for all the different scenarios to be tested.

Inside each tstcheck we have:

tstcase(function(values) == Correct_Output, Message);

This will give a message if the output is not correct based on boolean.

Additionally, we made our own stubs.c file that allowed us to use mocks of functions that were not required to be written by us, including `log_message` and `account_lookup_by_userid`. We did not use the given stubs.c inside src/ because that one gets overwritten and we wanted different functionalities such as custom debug messages.

There is also a header file in the test directory as our stubs code needs it to work.

Here is a brief summary of our testing:
To run the test, make the tests with `make`, but make sure to run `make clean` first.
Then, you can either run `make runtest` or `./tstrun --color` to run all of the tests, or run indivudal tests with `./t_<testname> --color`
-> insert some testing results or something here, might be too long to include all like 200 tests

### Project difficulties and how we addressed them

One of the major difficulties we had was in regards to the vagueness of the project spec. Due to the limited (and sometimes lack of) descriptions, we were often stuck deciding between different ways to implement things such what to do with the `account_lookup_by_userid` in stubs, how to use `account_id` and who will assign it and the purpose of `LOGIN_FAIL_IP_BANNED` and so on. We overcame these challenges through discussions among group members followed by a collective agreement on a path to take. This was a test of our teamwork and communication skills but fortunately we were able to decide quickly by hosting meetings both face to face and online.

As mentioned above, a struggle we had was figuring out how to simulate db querying for `account_lookup_by_userid` for testing, in the end we decided to hardcode a few test users as shown in the stubs.c file, including a normal user with userid of 'Ichigo', a banned user, an expired user and a user with too many consecutive failed logins.

Another small issue that was quickly evident after we started development was that there were some bugs with the pre commit hooks we set up in phase 1, which caused some small hiccups where some of us could not commit the changes made. The solution to this was communication with the group and prompt bug fixing by Seb.
