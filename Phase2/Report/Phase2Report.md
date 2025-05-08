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
1. We have used tst testing code from github
2. Simple and easy to use
3. Simply use the function and add the input + expected output
