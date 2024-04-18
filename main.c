#include <stdio.h>

#define BETTER_STRINGS_IMPLEMENTATION_
#include "better_strings.h"


int main(void) {
    bstr_t *b = bstr_wrap("Hello world ");
    bstr_append_str(b, "my name is Andrew Stormer.");
    bstr_print(b);

    //bstr_append_str(b, " I   am going to split this paragraph by spaces. This code should work for any delim and any bstr input into it.");
    //bstr_print(b);
    bstr_t *b2 = bstr_wrap(" This is going to be appended too.");
    bstr_append(b, b2);
    bstr_print(b);
    printf("%zu\n", bstr_strlen(b));



    //int count = bstr_count_char(b, 'l');
    //printf("%d\n", count);

    
    uint32_t token_count;
    bstr_t **tokens = bstr_split(b,  ' ', &token_count, 1);

    bstr_print_tokens(tokens, token_count);
    
    //bstr_rclean_tokens(tokens, token_count);
    bstr_print_tokens(tokens, token_count);

    bstr_reverse_tokens(tokens, token_count);
    bstr_print_tokens(tokens, token_count);

    bstr_print_tokens(tokens, token_count);
    bstr_reverse_tokens(tokens, token_count);
    
    bstr_t *joined_tokens = bstr_join(tokens, token_count);
    bstr_print(joined_tokens);
    

    return 0; 
}

