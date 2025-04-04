#include "math/math.h"

const char *greek_alphabet[] = {"alpha", "beta",  "gamma",  "delta",   "epsilon", "zeta", "eta",     "theta",
                                "iota",  "kappa", "lambda", "mu",      "nu",      "xi",   "omicron", "pi",
                                "rho",   "sigma", "tau",    "upsilon", "phi",     "chi",  "psi",     "omega"};

/**
 * @brief Check if a string is a greek letter with no case sensitivity
 *
 * @param str the string to check
 * @param len the length of the string
 * @return true if the string is a greek letter
 */
bool is_greek(const char *str, size_t len) {
    for (int i = 0; i < sizeof(greek_alphabet) / sizeof(greek_alphabet[0]); i++) {
        if (len < strlen(greek_alphabet[i])) {
            continue;
        }
        if (strncasecmp(str, greek_alphabet[i], len) == 0) {
            return true;
        }
    }
    return false;
}

bool is_identifier_char(char c) {
    return 'A' <= c && c <= 'Z' || 'a' <= c && c <= 'z';
}

bool is_number(char c) {
    return '0' <= c && c <= '9';
}

bool is_math(const char *name) {
    if (name[0] == ' ' && name[1] == '\0') {
        return false;
    }
    bool was_identifier_character = false;
    size_t identifier_length = 0;
    size_t len = strlen(name);
    if (len == 0) {
        return false;
    }
    for (int i = 0; i < len; i++) {
        if (name[i] < 0) {
            // Non-ASCII characters are too complicated to handle properly.
            return false;
        }
        if (was_identifier_character && is_identifier_char(name[i])) {
            identifier_length++;
        } else if (was_identifier_character && is_number(name[i])) {
            return false;
        } else if (is_identifier_char(name[i])) {
            was_identifier_character = true;
            identifier_length = 1;
        } else {
            if (was_identifier_character && identifier_length > 1 &&
                !is_greek(name + i - identifier_length, identifier_length)) {
                return false;
            }
            identifier_length = 0;
            was_identifier_character = false;
        }
    }
    if (was_identifier_character && identifier_length > 1) {
        return is_greek(name + len - identifier_length, identifier_length);
    }
    return true;
}
