# better_c_strings
Lightweight STB-styled header only file that implements strings as a struct Bstr that holds a pointer to the head and a pointer to the null terminator, so no space is needed for the null terminating character (getting the strlen is more efficient this way).
Strings can easily be wrapped in a Bstr by using the bstr_create(string) function and manipulated on, and are easily unwrapped.

# To use:
#define BETTER_STRINGS_IMPLEMENTATION_

#include "better_strings.h"
