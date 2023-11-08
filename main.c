#include <stdio.h>

#define BETTER_STRINGS_IMPLEMENTATION_
#include "better_strings.h"


int main(void) {
    Bstr b = bstr_create("");
    bstr_print(b);
    //bstr_append_str(&b, "my name is Andrew Stormer.");
    bstr_print(b);
    //bstr_append_str(&b, " I   am going to split this paragraph by sentences. This code should work for any delim and any Bstr input into it.");
    Bstr b2 = bstr_create(" This is going to be appended too. ");
    bstr_append_bstr(&b, b2);
    printf("%d\n", bstr_strlen(b));

    //Bstr b2 = bstr_substr(b.h, b.h + 5);
    bstr_print(b);
    printf("%c\n", *b.nt);
    // bstr_print(b2);
    int count = bstr_count_char(b, 'l');
    printf("%d\n", count);


    int token_count;
    Bstr * tokens = bstr_split(b,  ' ', &token_count, 1);

    bstr_print_tokens(tokens, token_count);    
    //bstr_clean_tokens(&tokens, token_count);
    bstr_print_tokens(tokens, token_count);
    bstr_reverse_tokens(&tokens, token_count);
    bstr_print_tokens(tokens, token_count);
    bstr_reverse_tokens(&tokens, token_count);
    
    Bstr joined_tokens = bstr_join(tokens, token_count);
    bstr_print(joined_tokens);

    return 0;
    
}

