
## Design Decisions



> What design decisions did you have to make? How and why did you decide on the approach you chose?

static helper functions for sanitising/neutralising email and birthday

- Used memcpy for majority of char* copying – due to 'partial-pseudo-string' fields in account & [list other benefits of memcpy]

- memcpy was implemented in a larger `buffer_to_cstring` function to partial-string above

- `explicit_bzero` & what + why we used it

- Setting account details: we used defaults set out in spec. We didn't attempt to 'sanitise' (find proper word from lecture) – we just entirely discarded any incorrect fields and declined creation

- password hashing – what we did and why: salting, argon2 etc etc

- writing to file: [todo: alec must think about mutex rll quick if possible, otehrwise just mention] [probs other todos here too, note in report for me and hopefully ill know what im talking about]

- `ip_to_cstring` we used inot_ntop because the older version is not thread safe etc. search this up to see if you can find what I mean – if not revisit me and ask

- maybe briefly explain why we used asprintf?

- how we chose what expiration time: (check my ting in there); note casting user->account_id to int (flame arran for this 🙏)

- @everyone do we want session struct to be cleared on failed login? (alec says -1)

<!-- - likewise - should a failed login return a `login_result_t`` struct of //  --> ignore


## FOR TESTS FOR THE LOVE OF GOD MENTION THAT WE ARE USING OUR OWN STUBS.C BECAUSE WE DEFINE OUR OWN `account_lookup_by_userid`
