#ifndef MATH_H
#define MATH_H

#include <stdbool.h>
#include <string.h>

/**
 * @brief Check if the given text is a valid Typst math expression. It may return false negatives but never
 * false positives.
 *
 * @param name the string to check
 * @return true if the string is a valid math expression
 */
bool is_math(const char *name);

#endif // MATH_H