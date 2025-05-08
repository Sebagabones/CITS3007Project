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
