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
>>> What design decisions did you have to make? How and why did you decide on the approach you chose?

#### Guiding Philosophy
- Boundary of Trust: Our system defines its trust boundary at the point of user input. Once fields such as `userid` and `email` have been validated and sanitised/neutralised, we treat them as clean.

- Null-Termination Assumptions: For fields within the `account_t` struct, we assume that `char*` fields are not null-terminated unless explicitly specified. We have been careful to never print our account fields using `%s`, as they are treated as fixed-size buffers (`char[10]`) per the specification and supporting forum clarifications. They are essentially up to their `#define`'d length, or their `\0`, whichever comes first, and we have a function that treats them as such, and avoid str-related functions on them specifically.

- No Redundant Checks: We deliberately avoid re-validating preconditions (e.g. checking for null-termination or NULL pointers) within internal logic. This is both for performance reasons and due to the inherent limitations of C, where one cannot reliably determine the length or termination of an arbitrary char*.

- Spec-defined minimalism: we tried to abide by the specification as closely as possible, without under-providing or overreaching. "Economy of design" meant that whenever possible, if we could safely leave assumptions to other modules, we did. For example our email validation solely checks for spaces + non-printable, not a truly valid email address.


1. Static helper functions for sanitising/neutralising inputs, logging, as well as hashing
    - We utilised multiple static helper functions to make our code more readable by reducing the amount of duplicated code.
    - Making these helper functions static reduced the possibility of misuse, as they are given internal linkage, preventing them from being referenced by other `.c` files.

2. Used `memcpy` for majority of `char*` copying.
    - We did this because it offers better control of the 'partial-pseudo-string' fields in `account.c` – which according to the spec are not *necessarily* null terminated.
    - It also avoids buffer overflows by copying specified lengths, when used carefully (eg by ensuring a null terminator is appended manually - code reviews and testing were both crucial for this to be successful).
    - Overall this helped to prevent undefined behaviour.

3. We implemented `explicit_bzero` to securely erases sensitive data in memory after it has been used
    - Prevent sensitive data from remaining within memory by overwriting it with zeroes.
    - Compilers can skip clearing memory before freeing it if the compiler knows that the memory is not used between being cleared and being freed. This optimisation can cause security concerns if sensitive data is not securely erased before releasing memory. `explicit_bzero` tells the compiler to ensure that this memory is erased (by writing 0's over the data) before freeing it.

4. For account field validation we neutralise rather than sanitise.
    - The group decided that instead of trying to sanitise invalid input, we would instead just check for and then refuse any invalid inputs.
      - This was chosen as it is simpler, reducing the possibility of missing a potential vulnerability.
      - Some inputs also make little sense to sanitise (e.g. if the email with invalid ASCII characters, it is unlikely that it is someone's real email address).
    - To do this we wrote many validation functions including `password_valid`, `birthday_valid` and `only_ASCII_printable_chars`.

5. Salting and Hashing using Sodium and Argon2
    - Salting using the Sodium library for each password was done to prevent rainbow table attacks.
    - Argon2id has balanced protection against brute-force and side-channel attacks.
    - Due to the possibility of brute forcing the hash as well, we have chosen to use Sodium's constant time comparison function to compare the computed hash and the hash stored.
    - In order to be more efficient with calculating the hashes, Argon2 provides us with the ability to use multi-threading to make use of multiple CPU cores (in our case 4 cores) to calculate or validate the hashes for very long passwords.
    - We employed the standards of Argon2's recommended parameters from their documentation, which are:
        - Memory cost: 65536 KiB
        - Time cost: 4 iterations
        - Parallelism: 4 threads
    - We enforced a minimum password length of 8 characters, and a maximum of 256 characters. The minimum length is to prevent brute force attacks – as short passwords are weaker even when salted and hashed. The maximum length serves to prevent the user from entering a password that is too long, which could cause issues with memory allocation and performance. We also enforce a minimum of 1 uppercase letter, 1 lowercase letter, 1 number, and 1 symbol to ensure that the password is strong enough to resist brute force attacks.
    - A main motivator of using Argon2 is that it is the winner of the Password Hashing Competition (PHC) and is widely considered to be the most secure password hashing algorithm available. It is very modern and industry standard.

6. Writing to file descriptors
    - We used `write` to file instead of using `fprintf` because it is more efficient, as we were writing to a file descriptor.
    - For printing the account summary *solely*, we employ a mutex lock to prevent a data race condition. We do this by locking the mutex before writing to the file and unlocking it after we are done writing. This ensures that only one thread can write to the file at a time, preventing any potential data corruption or inconsistencies
    - We did this because we believe the ACS would best be used in a multi-threaded environment, where it can handle multiple requests simultaneously. By using a mutex lock, we can ensure that the ACS can handle multiple requests to create accounts/log people in etc without issue – and printing the account summary won't block any of those more frequent requests.
    - We did not use a mutex for the client file descriptor printing, because we believe that the client wouldn't be sending multiple requests quickly enough to cause a race condition.

7. Using `inet_ntop` in `ip_to_cstring`
    - Previous version `inet_ntoa` is not thread safe, it uses shared static buffer across calls meaning race conditions are caused when multiple threads call it at the same time.
    - We used `inet_ntop` because it is thread safe since it uses caller provided buffers, and it supports multiple protocols like IPv6.

8. We used `asprintf` over alternatives like `sprintf`/`snprintf`
    - `asprintf` helps to avoid buffer overflows as it automatically allocates a buffer large enough to hold the formatted string.
    - It improves safety by returning -1 upon failure and the number of bytes written on success, meaning checking for successful writes is trivially easy.

9. Session expiry time
   - For our implementation we chose for the session expiry time to be equal to the user account expiry time, or 24 hours in the cases where there was none.
   - The reason for this is there is nothing else related to the session expiry time so it was assumed that the user account expiry and the session expiry were intended to be related.

10. Not using `LOGIN_FAIL_IP_BANNED`
    - We decided not to use IP banning because of several reasons:
      - For one thing everyone uses dynamic IP addresses nowadays so just by simply rebooting their router they could bypass the ban (or worse, possibly passing the ban along to another innocent player).
      - Even if one does not use dynamic IPs, VPNs exist to do the same thing.
      - Since the given IP is IPv4, due to the scarcity of IPv4 addresses, a lot of Internet Service Providers use CGNAT, meaning banning one IP could inadvertently impact thousands of players, which is not ideal when your product's success is dependant among the player-base being able to, well, play the game.
    - While there are of course solutions to these problems, our group felt like it was out of scope of the provided project brief.

11. Passing 10 failed logins will soft ban the account
    - We implemented an enforced login fail when there was more than 10 consecutive failed logins in a row, meaning the account will be impossible to access.
    - We intend there to be an email sent out to the account owner prompting them to change their userid/password in order to unlock their account, however this would be implemented outside of the ACS, and so was out of scope for this project.
    - The alternative is to ban the account for a short period of time but we decided against that because of two reasons:
      - There is only one ban status, and it is for real bans where the only method of appealing is presumably through a customer service portal which is obviously too severe and inconvenient for just 10 consecutive failed logins (and could be used to grief players by malicous entities)
      - If we only ban the account for a short period of time there is nothing preventing the harasser to do it again after the ban expires.
    - This should be noted that our group feel the project spec of banning after ten logins is still able to be misused by players to ban each other quite easily, and that this would be a more useful use-case of IP banning (ie, preventing that IP from accessing this account for 24 hours).

12. Validation for password
    - We decided to add a few enforced rules to passwords being created, namely requiring the password to have at least a number, a lower case letter, an upper case letter and a symbol.
      - This was done to prevent users from choosing passwords that may be vulnerable to some form of targeted password guessing attack.
    - After all, no matter how strong our hashing is it won't matter if someone can just crack the password by guessing it/using a bot to try common passwords.

13. Accepting risk of certain integer overflows
    - We decided to accept the risk of certain overflows in our code, such as the `account_id` and `session_id` fields in the `account` and `session` structs respectively.
    - This is because we are not expecting to have more than 2^32 users or sessions at any given time, and we should have a long time before we reach that limit.
    - Note we had to cast the user's `account_id` from a `int64_t` to an `int` to prevent the implicit conversion warning when filling the `session` struct – but this seems to be a problem with the provided spec, as there seems to be no reason for it to be a different size of to `account_id`.

14. Type conversions from `long int` to `int32_t` in `extract_hash_components`
    - In this function there were multiple instances of the compiler warning us of the possibility of truncation during the conversions.
    - We chose to accept this risk because these conversions were being done on values that would dictate the time and memory usage of the hashing algorithm, which in our project and in the real world would never reach the LONG_MAX limit of `9223372036854775807` (in a modern 64bit machine); and because Argon2 takes those values in KiB, it is simply not possible to have a memory allocation of `8388608` Pebibytes.
    - The type conversion warning for `p_value` will never become an issue for the same reason; it's the number of threads for Argon2 to use when it calculates the hashes. It would be absolutely impractical for someone to use that many threads.
    - To make us feel safe, even though it is not necessary, we implemented a range check where these type conversion warnings came up to ensure an abundance of safety.
    - It is also important to mention, these values are set in the code (and are hardcoded in), so we are confident of the safety here.

### Testing decisions

We used the `tst` testing framework (available [here](https://github.com/rdentato/tst/tree/main)) to run unit tests to make sure that the output/error handling working properly based on the inputs given.

`tst` takes simple inputs and allows us to define the correct output as well as adding our own output to show debug messages.

The reason we went with `tst` is because it was very simple to implement while retaining most of the features of more complicated test frameworks (such as `check`).
This helped to increase the speed at which we developed tests, allowing us to test a larger surface area of our code.

Due to the ease of setup, (unlike some other test frameworks with complex build dependencies/setups), all that `tst` required was to include `tst.h`.

`tst` can be split into a test-suites per file, which can contain multiple test-cases each with their own test-checks, for example:

```
tstsuite(){
    tstcase(1) {
        tstcheck(1)
        tstcheck(2)
        .
        .
        .
    }
    .
    .
}
```
This means we can split a suite for each functionality that requires testing, then have cases for all the different scenarios to be tested.

We also were able to use `tstsection`'s to increase the separation of different tests, purely for increasing the clarity of the results.

Inside each `tstcheck` we have:

`tstcase(function(values) == Correct_Output, Message)`

If a `tstcase` fails (eg the expression is not true), then `Message` will be printed out to assist with debugging.


Additionally, we made our own version of the `stubs.c` file for testing, which allowed us to use functions that were not in the project brief to help with creating more end to end testing, without increasing the code-base for the non-testing sections of our code. Examples of our custom functions are modifications to `log_message` and `account_lookup_by_userid`. This effectively meant we were able to write better tests, using different functionalities from the `stubs.c`, such as custom debug messages (which was carefully done to ensure the code did not deviate from the requirements).

The `Makefile` in our code uses `stubs.c` and the custom header file in the testing directory to overwrite `stubs.c` functions when compiling the tests.

### How to run tests.
First go to the testing directory (named `tests`).
To run the test, make the tests with `make`, making sure to run `make clean` first.
Then, either run `make runtest` or `./tstrun --color` to run all of the tests, or run individual tests with `./t_<testname> --color`.


## Dynamic Analysis
We implemented both Valgrind and Google sanitisers in our testing to help with both debugging memory issues, but to also help use check for memory issues/vulnerabilities. This proved to be invaluable, as there were buffer overflows that would not have been caught without our testing system.

