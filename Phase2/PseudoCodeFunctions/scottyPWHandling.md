## Pseudocode for password handling functions
Password hashing and validation functions for 3.4 by Scotty

### Important Notes
Essential helper functions have been left out to keep this mockup short. However most of the helpers can be modularized and reused between each implementation because of the similarities and overlaps. But for ease of reading the code, I have repeated the same functions regardless.

### Argon2
Probably the best heavyweight champion when it comes to password handling, but may be difficult or not possible at all to implement due to it being relatively new. Amazing customizable parameters such as memory cost, time cost and multithreading (parallelism).

#### Argon2 Prerequisites
```c
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <argon2.h>
```

#### Argon2 Data Structure
```c
// Assuming this is part of a larger account structure, I have also copied some of the fields from Seb's work.
typedef struct {
    int64_t      account_id;
    char         userid[USER_ID_LENGTH];
    char         password_hash[HASH_LENGTH];
    char         email[EMAIL_LENGTH];
    char         birthdate[BIRTHDATE_LENGTH];
    // Other account fields...
} account_t;

// Constants
#define SALT_LENGTH 16  // 16 bytes for salt
#define HASH_RAW_LENGTH 32  // 32 bytes for the raw hash
// Time, memory and parallelism parameters
#define T_COST 3       // Number of iterations
#define M_COST 65536   // Memory usage in KiB (64 MB)
#define PARALLELISM 4  // Number of threads
```

#### Argon2 Implementation
```c
bool account_validate_password(const account_t *acc, const char *plaintext_password) {
    // Validate preconditions as per requirement
    if (acc == NULL || plaintext_password == NULL) {
        return false;
    }
    
    // Parse the stored hash to extract salt and parameters
    char stored_salt[SALT_LENGTH];
    int t_cost, m_cost, parallelism;
    
    // Helper function to extract salt and parameters from stored format
    if (!extract_hash_components(acc->password_hash, stored_salt, &t_cost, &m_cost, &parallelism)) {
        return false;
    }
    
    // Calculate hash of provided plaintext password using extracted parameters
    char computed_hash[HASH_LENGTH];
    if (!generate_argon2_hash(plaintext_password, stored_salt, t_cost, m_cost, parallelism, computed_hash)) {
        return false;
    }
    
    // Compare the computed hash with the stored hash using constant-time comparison
    return secure_compare(computed_hash, acc->password_hash);
}

bool account_update_password(account_t *acc, const char *new_plaintext_password) {
    // Validate preconditions
    if (acc == NULL || new_plaintext_password == NULL) {
        return false;
    }
    
    // Generate random salt
    unsigned char salt[SALT_LENGTH];
    if (!generate_random_bytes(salt, SALT_LENGTH)) {
        return false;
    }
    
    // Hash the new password with Argon2id
    char new_hash[HASH_LENGTH];
    if (!generate_argon2_hash(new_plaintext_password, salt, T_COST, M_COST, PARALLELISM, new_hash)) {
        return false;
    }
    
    // Update the account's password hash
    strncpy(acc->password_hash, new_hash, HASH_LENGTH - 1);
    acc->password_hash[HASH_LENGTH - 1] = '\0'; // Ensure null termination
    
    return true;
}

// Helper function for Argon2 hash generation
bool generate_argon2_hash(const char *password, const unsigned char *salt, 
                         int t_cost, int m_cost, int parallelism, char *output) {
    // Create a buffer for the raw hash
    unsigned char raw_hash[HASH_RAW_LENGTH];
    
    // Use Argon2id variant (combines protection against side-channel and GPU attacks)
    int result = argon2id_hash_raw(
        t_cost,                 // Time cost
        m_cost,                 // Memory cost
        parallelism,            // Parallelism
        password,               // Password
        strlen(password),       // Password length
        salt,                   // Salt
        SALT_LENGTH,            // Salt length
        raw_hash,               // Output hash
        HASH_RAW_LENGTH         // Output hash length
    );
    
    if (result != ARGON2_OK) {
        return false;
    }
    
    // Format the final hash string with parameters and salt
    // Format: $argon2id$v=19$m=65536,t=3,p=4$[salt_base64]$[hash_base64]
    format_argon2_hash(output, t_cost, m_cost, parallelism, salt, raw_hash);
    
    return true;
}
```

### SCrypt
Password-based key derivation function with customizable parameters for memory and CPU costs. More likely to work than Argon2, however it is more memory hungry.

#### SCrypt Prerequisites
```c
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <crypto/scrypt.h> // libscrypt or equivalent library
```

#### SCrypt Data Structure
```c
// Assuming this is part of a larger account structure, I have also copied some of the fields from Seb's work.
typedef struct {
    int64_t      account_id;
    char         userid[USER_ID_LENGTH];
    char         password_hash[HASH_LENGTH];
    char         email[EMAIL_LENGTH];
    char         birthdate[BIRTHDATE_LENGTH];
    // Other account fields...
} account_t;

// Constants
#define SALT_LENGTH 16  // 16 bytes for salt
// SCrypt parameters
#define N_COST 16384    // CPU/memory cost parameter (2^14)
#define R_COST 8        // Block size parameter
#define P_COST 1        // Parallelization parameter
#define HASH_RAW_LENGTH 32  // 32 bytes for the raw hash
```

#### SCrypt Implementation
```c
bool account_validate_password(const account_t *acc, const char *plaintext_password) {
    // Validate preconditions
    if (acc == NULL || plaintext_password == NULL) {
        return false;
    }
    
    // Parse the stored hash to extract salt and parameters
    unsigned char stored_salt[SALT_LENGTH];
    int n_cost, r_cost, p_cost;
    
    // Helper function to extract salt and parameters from stored format
    if (!extract_scrypt_components(acc->password_hash, stored_salt, &n_cost, &r_cost, &p_cost)) {
        return false;
    }
    
    // Calculate hash of provided plaintext password using extracted parameters
    char computed_hash[HASH_LENGTH];
    if (!generate_scrypt_hash(plaintext_password, stored_salt, n_cost, r_cost, p_cost, computed_hash)) {
        return false;
    }
    
    // Compare the computed hash with the stored hash using constant-time comparison
    return secure_compare(computed_hash, acc->password_hash);
}

bool account_update_password(account_t *acc, const char *new_plaintext_password) {
    // Validate preconditions
    if (acc == NULL || new_plaintext_password == NULL) {
        return false;
    }
    
    // Generate random salt
    unsigned char salt[SALT_LENGTH];
    if (!generate_random_bytes(salt, SALT_LENGTH)) {
        return false;
    }
    
    // Hash the new password with scrypt
    char new_hash[HASH_LENGTH];
    if (!generate_scrypt_hash(new_plaintext_password, salt, N_COST, R_COST, P_COST, new_hash)) {
        return false;
    }
    
    // Update the account's password hash
    strncpy(acc->password_hash, new_hash, HASH_LENGTH - 1);
    acc->password_hash[HASH_LENGTH - 1] = '\0'; // Ensure null termination
    
    return true;
}

// Helper function for scrypt hash generation
bool generate_scrypt_hash(const char *password, const unsigned char *salt,
                         int n, int r, int p, char *output) {
    // Create a buffer for the raw hash
    unsigned char raw_hash[HASH_RAW_LENGTH];
    
    // Use scrypt to derive the key
    int result = crypto_scrypt(
        (const uint8_t *)password,  // Password
        strlen(password),           // Password length
        salt,                       // Salt
        SALT_LENGTH,                // Salt length
        n,                          // N parameter (CPU/memory cost)
        r,                          // r parameter (block size)
        p,                          // p parameter (parallelization)
        raw_hash,                   // Output buffer
        HASH_RAW_LENGTH             // Output buffer length
    );
    
    if (result != 0) {
        return false;
    }
    
    // Format the final hash string with parameters and salt
    // Format: $scrypt$ln=14,r=8,p=1$[salt_base64]$[hash_base64]
    format_scrypt_hash(output, n, r, p, salt, raw_hash);
    
    return true;
}
```

### BCrypt
Hashing function based on the Blowfish cipher, with a salt designed against rainbow tables or similar attacks. It has built in salt generation and management with a simple API so its very simple to implement. Does not have customizable parameters like the other 2 but it does adjust its work factor. However major drawback is a limited password length of 72 bytes.

#### BCrypt Prerequisites
```c
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <crypt.h>      // For systems with glibc
// Alternative: #include <bcrypt.h> // For dedicated bcrypt libraries
```

#### BCrypt Data Structure
```c
// Assuming this is part of a larger account structure, I have also copied some of the fields from Seb's work.
typedef struct {
    int64_t      account_id;
    char         userid[USER_ID_LENGTH];
    char         password_hash[HASH_LENGTH];
    char         email[EMAIL_LENGTH];
    char         birthdate[BIRTHDATE_LENGTH];
    // Other account fields...
} account_t;

// Constants
#define BCRYPT_WORKFACTOR 12  // Cost factor (2^12 iterations)
```

#### BCrypt Implementation
```c
bool account_validate_password(const account_t *acc, const char *plaintext_password) {
    // Validate preconditions
    if (acc == NULL || plaintext_password == NULL) {
        return false;
    }
    
    // BCrypt hashes contain the salt and cost factor, so we can directly compare
    // by computing a new hash with the same parameters
    
    // Check if the password matches the stored hash
    // The stored hash contains the salt and parameters needed for validation
    return bcrypt_checkpw(plaintext_password, acc->password_hash) == 0;
}

bool account_update_password(account_t *acc, const char *new_plaintext_password) {
    // Validate preconditions
    if (acc == NULL || new_plaintext_password == NULL) {
        return false;
    }
    
    // Generate a bcrypt hash with a new random salt
    char new_hash[HASH_LENGTH];
    
    // Generate salt string (format: "$2b$12$...")
    char salt[32];
    if (!generate_bcrypt_salt(BCRYPT_WORKFACTOR, salt)) {
        return false;
    }
    
    // Generate the hash with the generated salt
    char *result = bcrypt(new_plaintext_password, salt);
    if (result == NULL) {
        return false;
    }
    
    // Copy the resulting hash to the account
    strncpy(acc->password_hash, result, HASH_LENGTH - 1);
    acc->password_hash[HASH_LENGTH - 1] = '\0'; // Ensure null termination
    
    return true;
}

// Helper function to generate bcrypt salt (pseudocode)
bool generate_bcrypt_salt(int work_factor, char *salt_output) {
    // Generate 16 random bytes for the salt
    unsigned char random_bytes[16];
    if (!generate_random_bytes(random_bytes, 16)) {
        return false;
    }
    
    // Format the salt string: $2b$[work_factor]$[22_chars_base64_salt]
    sprintf(salt_output, "$2b$%02d$", work_factor);
    
    // Encode the random bytes to base64 format used by bcrypt
    encode_base64_bcrypt(random_bytes, 16, salt_output + strlen(salt_output));
    
    return true;
}
```