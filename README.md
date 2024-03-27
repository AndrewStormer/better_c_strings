# better_c_strings
## Getting started:
```
#define BETTER_STRINGS_IMPLEMENTATION_
#include "better_string.h"

bstr_t *bstr = bstr_wrap("This can be any string, as well as any char *");
//Do something cool with your new bstr! e.g) bstr_append, bstr_split, bstr_substr, bstr_join, etc.
char *str = bstr_unwrap(bstr); /NOTE: bstr object is destroyed.
```
