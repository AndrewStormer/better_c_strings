#include <stdio.h>

#define BETTER_STRINGS_IMPLEMENTATION_
#include "better_strings.h"


int main(void) {
    Bcs b = bcs_wrap("Hello world ");
    bcs_print(b);
    bcs_append_str(&b, "my name is Andrew Stormer.");
    printf("%c\n", *b.nt);
    
    bcs_print(b);
    bcs_append_str(&b, " I   am going to split this paragraph by spaces. This code should work for any delim and any Bcs input into it.");
    Bcs b2 = bcs_wrap(" This is going to be appended too.");
    bcs_append(&b, b2);
    printf("%d\n", bcs_strlen(b));

    //Bcs b2 = bcs_subcs(b.h, b.h + 5);
    bcs_print(b);
    printf("%c\n", *b.nt);
    // bcs_print(b2);
    int count = bcs_count_char(b, 'l');
    printf("%d\n", count);


    int token_count;
    Bcs * tokens = bcs_split(b,  ' ', &token_count, 1);

    //bcs_print_tokens(tokens, token_count);
    printf("AAAA\n");
    bcs_clean_tokens(&tokens, token_count)
    //bcs_print_tokens(tokens, token_count);
    bcs_reverse_tokens(&tokens, token_count);
    //bcs_print_tokens(tokens, token_count);
    bcs_reverse_tokens(&tokens, token_count);
    
    Bcs joined_tokens = bcs_join(tokens, token_count);
    bcs_print(joined_tokens);
    // bcs_full_clean(&b);
    bcs_print(b);
    char * str = bcs_unwrap(joined_tokens);
    puts(str);
    Bcs name = bcs_wrap("name");
    printf("%c\n", *b.nt);
    
    int pos = bcs_lmatch(b, name);
    printf("%d\n", pos);

    return 0; 
}


