bool account_validate_password(const account_t *acc,
                               const char *plaintext_password)
{
	// Validate preconditions as per requirement
	if (acc == NULL || plaintext_password == NULL)
	{
		return(false);
	}

	// Parse the stored hash to extract salt and parameters
	char stored_salt[SALT_LENGTH];
	int	 t_cost, m_cost, parallelism;

	// Helper function to extract salt and parameters from stored format
	if (!extract_hash_components(acc->password_hash, stored_salt, &t_cost,
	                             &m_cost, &parallelism))
	{
		return(false);
	}

	// Calculate hash of provided plaintext password using extracted parameters
	char computed_hash[HASH_LENGTH];

	if (!generate_argon2_hash(plaintext_password, stored_salt, t_cost, m_cost,
	                          parallelism, computed_hash))
	{
		return(false);
	}

	// Compare the computed hash with the stored hash using constant-time
	// comparison
	return(secure_compare(computed_hash, acc->password_hash));
}

bool account_update_password(account_t *acc,
                             const char *new_plaintext_password)
{
	// Validate preconditions
	if (acc == NULL || new_plaintext_password == NULL)
	{
		return(false);
	}

	// Generate random salt
	unsigned char salt[SALT_LENGTH];

	if (!generate_random_bytes(salt, SALT_LENGTH))
	{
		return(false);
	}

	// Hash the new password with Argon2id
	char new_hash[HASH_LENGTH];

	if (!generate_argon2_hash(new_plaintext_password, salt, T_COST, M_COST,
	                          PARALLELISM, new_hash))
	{
		return(false);
	}

	// Update the account's password hash
	strncpy(acc->password_hash, new_hash, HASH_LENGTH - 1);
	acc->password_hash[HASH_LENGTH - 1] = '\0'; // Ensure null termination

	return(true);
}

// Helper function for Argon2 hash generation
bool generate_argon2_hash(const char *password, const unsigned char *salt,
                          int t_cost, int m_cost, int parallelism,
                          char *output)
{
	// Create a buffer for the raw hash
	unsigned char raw_hash[HASH_RAW_LENGTH];

	// Use Argon2id variant (combines protection against side-channel and GPU
	// attacks)
	int result = argon2id_hash_raw(t_cost,           // Time cost
	                               m_cost,           // Memory cost
	                               parallelism,      // Parallelism
	                               password,         // Password
	                               strlen(password), // Password length
	                               salt,             // Salt
	                               SALT_LENGTH,      // Salt length
	                               raw_hash,         // Output hash
	                               HASH_RAW_LENGTH   // Output hash length
	                               );

	if (result != ARGON2_OK)
	{
		return(false);
	}

	// Format the final hash string with parameters and salt
	// Format: $argon2id$v=19$m=65536,t=3,p=4$[salt_base64]$[hash_base64]
	format_argon2_hash(output, t_cost, m_cost, parallelism, salt, raw_hash);

	return(true);
}
