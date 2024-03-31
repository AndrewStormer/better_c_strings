# better_c_strings
## Getting started:
```
#define BETTER_STRINGS_IMPLEMENTATION_
#include "better_string.h"

int main(void)
    bstr_t *bstr = bstr_wrap("This can be any string, as well as any char *");
    //Do something cool with your new bstr! e.g) bstr_append, bstr_split, bstr_substr, bstr_join, etc.
    char *str = bstr_unwrap(bstr); //NOTE: bstr object is destroyed.
    free(str) //NOTE: str must be free'd afterwords
```

## Use Cases:
My motivation behind this project was to make parsing of files easier - especially ones with clear delimitters.

```
file_parse(FILE *fp, ...) {
    char line[LINE_SIZE];

    while (fgets(line, sizeof(line), fp)) { //or some other way of reading from files
        bstr_t *b = bstr_wrap(line);
        uint16_t token_count;
        bstr_t **tokens = bstr_split(b, ' ', &token_count, 0); //We do not want to keep the delim, so 4th parameter 0.
        bstr_clean_tokens(tokens, token_count); //clean all whitespace from the tokens
        for (size_t i = 0; i < token_count; ++i) {
            //process data as you please
            //pack into parameters or return value
        }
        bstr_destroy_tokens(tokens); //unless you unwrap strings during the above for loop, replace with free(tokens)
    }
}
```

## TODO:
More comments!

-----Refector bstr_xclean algorithms.-----

Split function that make check several delimitters

String comparison algorithms.

Build Directory.

Potentially return string statically in bstr_unwrap, or take an already pre-allocated (statically or dynamically) char * as a second parameter to unwrap into, making unwrap void.

-----Make void functions return error codes?-----

Potentially make functions that must return accept a code as a pass-by-reference parameter.
