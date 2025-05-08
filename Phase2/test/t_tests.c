#include "tst.h"

tstsuite("handle_login") {
    tstcase("Username Empty") {
        tstcheck(handle_login("", "Bankai2", 0, 3) == LOGIN_FAIL_INTERNAL_ERROR)
    }

    tstcase("Username Empty") {
        tstcheck(handle_login("", "Bankai2", 0, 3) == LOGIN_FAIL_INTERNAL_ERROR)
    }

    tstcase("Username Empty") {
        tstcheck(handle_login("", "Bankai2", 0, 3) == LOGIN_FAIL_INTERNAL_ERROR)
    }

    tstcase("Username Empty") {
        tstcheck(handle_login("", "Bankai2", 0, 3) == LOGIN_FAIL_INTERNAL_ERROR)
    }

    tstcase("Username Empty") {
        tstcheck(handle_login("", "Bankai2", 0, 3) == LOGIN_FAIL_INTERNAL_ERROR)
    }
}