#include <stdio.h>

#define BETTER_STRINGS_IMPLEMENTATION_
#include "better_strings.h"


int main(void) {
    bcs_t *b = bcs_wrap("Hello world ");
    bcs_append_str(b, "my name is Andrew Stormer.");
    bcs_print(b);

    bcs_append_str(b, " I   am going to split this paragraph by spaces. This code should work for any delim and any Bcs input into it.");
    bcs_print(b);
    bcs_t *b2 = bcs_wrap(" This is going to be appended too.");
    bcs_append(b, b2);
    bcs_print(b);
    printf("%zu\n", bcs_strlen(b));

    //int count = bcs_count_char(b, 'l');
    //printf("%d\n", count);

    
    uint32_t token_count;
    bcs_t **tokens = bcs_split(b,  ' ', &token_count, 1);

    bcs_print_tokens(tokens, token_count);
    
    //bcs_lclean_tokens(tokens, token_count);
    bcs_print_tokens(tokens, token_count);

    printf("akdsljf;ldskjf\n");
    bcs_reverse_tokens(tokens, token_count);
    bcs_print_tokens(tokens, token_count);

    //bcs_print_tokens(tokens, token_count);
    bcs_reverse_tokens(tokens, token_count);
    
    bcs_t *joined_tokens = bcs_join(tokens, token_count);
    bcs_print(joined_tokens);
    

    return 0; 
}

