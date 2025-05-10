Threat Modelling

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
