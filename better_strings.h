#ifndef BETTER_STRINGS_H_
#define BETTER_STRINGS_H_


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct bstr {
  char * h, * nt; //head and null terminator (not actually saving space for '\0', we know we are at the end if a pointer points to Bstr.nt.
} Bstr;

Bstr bstr_create(char * s);
char * bstr_unwrap(Bstr b);
void bstr_destroy(Bstr b);
void bstr_destroy_tokens(Bstr * tokens, int token_count);
Bstr bstr_substr(char * s, char * e);
void bstr_print(Bstr b);
void bstr_print_tokens(Bstr * tokens, int token_count);
int bstr_strlen(Bstr b);
//static Bstr bstr_resize(Bstr b, int new_len);
void bstr_append_str(Bstr * b, char * s);
void bstr_append(Bstr * b, Bstr s);
void bstr_print(Bstr b);
//static int bstr_count_tokens(Bstr b, char delim);
//static char * bstr_next_token(Bstr b, char * h, char delim);
Bstr * bstr_split(Bstr b, char delim, int * token_count, int keep_delim); //token_count is passed by ref, when keep_delim is 0 delim is automatically changed to newline
Bstr bstr_join(Bstr * b, int token_count);
void bstr_lclean(Bstr * b);
void bstr_lclean_tokens(Bstr ** tokens, int token_count);
void bstr_rclean(Bstr * b);
void bstr_rclean_tokens(Bstr ** tokens, int token_count);
void bstr_clean(Bstr * b);
void bstr_clean_tokens(Bstr ** tokens, int token_count);
void bstr_full_clean(Bstr * b);
int bstr_count_char(Bstr b, char c);
void bstr_reverse(Bstr * b);
void bstr_reverse_tokens(Bstr ** tokens, int token_count);
char * bstr_lmatch_str(Bstr b, Bstr str);


#endif //BETTER_STRINGS_H_

#ifdef  BETTER_STRINGS_IMPLEMENTATION_


static int str_strlen(char * s) {
  char * c = s;
  while (*c != '\0') {
    ++c;
  }
  return (c - s) + 1;
}



Bstr bstr_create(char * s) {
  Bstr b;
  int len = str_strlen(s)-1;
  b.h = malloc(sizeof(char)*len);
  b.nt = b.h + len;
  for (int i = 0; i < len; ++i) {
    b.h[i] = s[i];
  }
  return b;
}


//TODO: implement a way to declare in static space so we do not have to free
//Problem: doing char str[bstr_strlen(b)+1] creates str in the stack, which does not persist after the function is returned from, and so we to return a pointer to that stack spac which breaks.
//*** MUST FREE STRING PASSED BACK FROM FUNCTION
char * bstr_unwrap(Bstr b) {
  char * str = malloc(sizeof(char)*bstr_strlen(b) + 1);
  char * c = b.h;
  char * s = str;

  while (c <= b.nt) {
    *s = *c;
    ++s;
    ++c;
  }
  *s = '\0';
  bstr_destroy(b);
  return &str[0];
}


inline void bstr_destroy(Bstr b) {
  free(b.h);
}


inline void bstr_destroy_tokens(Bstr * tokens, int token_count) {
  for (int i = 0; i < token_count; ++i)
    bstr_destroy(tokens[i]);
}


Bstr bstr_substr(char * s, char * e) {
  assert(e >= s);
  Bstr b;
  b.h = malloc(sizeof(char)*(e-s)+1);
  b.nt = b.h;
  char * c = s;
  while (c < e) {
    *b.nt = *c;
    ++c;
    ++b.nt;
  }
  *b.nt = *c;
  return b;
}


void bstr_print(Bstr b) {
  char * c = b.h;
  while (c != b.nt + 1) {
    printf("%c", *c);
    ++c;
  }
  printf("\n");
}


void bstr_print_tokens(Bstr * tokens, int token_count) {
  for (int i = 0; i < token_count; i++) {
    bstr_print(tokens[i]);
  }
}
	     

//we do not keep an extra space for the null terminator
inline int bstr_strlen(Bstr b) {
  return (b.h == NULL) ? 0 : b.nt - b.h + 1;
}


static Bstr bstr_resize(Bstr b, int new_len) {
  if (b.h != NULL) {
    char * s = malloc(sizeof(char)*new_len);
    assert(s != NULL);
    char * c = s;
    int i = 0;
    while (i <= b.nt - b.h + 1) {
      *c = b.h[i];
      ++i;
      ++c;
    }
    free(b.h);
    b.h = s;
    b.nt = b.h + new_len-1;
  } else {
    b.h = malloc(sizeof(char)*new_len);
    assert(b.h != NULL);
  }
  return b;
}


void bstr_append_str(Bstr * b, char * s) {
  int len = bstr_strlen(*b), new_len = str_strlen(s) + len-1;
  *b = bstr_resize(*b, new_len);
  char * c1 = (b->h + (len-1)), * c2 = s;
  while (c1 < b->nt) {
    *c1 = *c2;
    ++c1;
    ++c2;
  }
  b->nt = b->h + new_len-1;
}


void bstr_append(Bstr * b, Bstr s) {
  int len = bstr_strlen(*b), new_len = bstr_strlen(s) + len-1;
  *b = bstr_resize(*b, new_len);
  char * c1 = (b->h + (len-1)), * c2 = s.h;
  while (c1 < b->nt) {
    *c1 = *c2;
    ++c1;
    ++c2;
  }
  b->nt = b->h + new_len-2;
}


static int bstr_count_tokens(Bstr b, char delim) {
  char * c = b.h;
  int count = 0;
  if (*c == delim)
    ++count;
  ++c;
  while (c != b.nt+1) {
    if ((*c == delim || c == b.nt) && *(c-1) != delim)
      ++count;
    ++c;
  }
  return count;
}


static char * bstr_next_delim(Bstr b, char * h, char delim) {
  while (*h != delim && h != b.nt) {
    ++h;
  }
  return h;
}


static char * bstr_next_non_delim(Bstr b, char * h, char delim) {
  while (*h == delim && h != b.nt) {
    ++h;
  }
  return h;
}


Bstr * bstr_split(Bstr b, char delim, int * token_count, int keep_delim) {
  *token_count = bstr_count_tokens(b, delim);
  Bstr * tokens = malloc(sizeof(Bstr)**token_count);
  assert(tokens != NULL);
  char * c = b.h, * d;
  for (int i = 0; i < *token_count; ++i) {
    d = bstr_next_delim(b, c, delim);
    if (keep_delim == 1 && d != b.nt) {
      d = bstr_next_non_delim(b, d, delim);
      --d;
    }
    tokens[i] = bstr_substr(c, d - (1 - keep_delim));
    c = bstr_next_non_delim(b, d, delim);
  }
  return tokens;
}


Bstr bstr_join(Bstr * tokens, int token_count) {
  int joined_len = 0;
  for (int i = 0; i < token_count; ++i)
    joined_len += bstr_strlen(tokens[i]);
  Bstr b;
  b.h = malloc(sizeof(char)*joined_len);
  assert(b.h != NULL);
  char * h = b.h;
  for (int i = 0; i < token_count; ++i) {
    char * c = tokens[i].h;
    while (c != tokens[i].nt + 1) {
      *h = *c;
      ++c;
      ++h;
    }
  }
  b.nt = b.h + joined_len - 1;
  return b;
}


void bstr_lclean(Bstr * b) {
  char * c = b->h;
  while (*c == ' ') {
    ++c;
  }
  Bstr * temp = b;
  Bstr temp2 = bstr_substr(c, b->nt);
  b = &temp2;
  bstr_destroy(*b);
}


void bstr_lclean_tokens(Bstr ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bstr_lclean(&((*tokens)[i]));
  }
}


void bstr_rclean(Bstr * b) {
  char * c = b->nt;
  while (*c == ' ') {
    --c;
  }

  Bstr * temp = b;
  Bstr temp2 = bstr_substr(b->h, c);
  b = &temp2;
  bstr_destroy(*b);
}


void bstr_rclean_tokens(Bstr ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bstr_rclean(&((*tokens)[i]));
  }
}


void bstr_clean(Bstr * b) {
  bstr_lclean(b);
  bstr_rclean(b);
}


void bstr_clean_tokens(Bstr ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bstr_clean(&((*tokens)[i]));
  }
}


void bstr_full_clean(Bstr * b) {
  int token_count = 0;
  Bstr * tokens = bstr_split(*b,  ' ', &token_count, 0);
  bstr_clean_tokens(&tokens, token_count);
  *b = bstr_join(tokens, token_count);
  bstr_destroy_tokens(tokens, token_count);
}


int bstr_count_char(Bstr b, char c) {
  int count = 0;
  for (char * p = b.h; p <= b.nt; ++p) {
    if (*p == c)
      ++count;
  }
  return count;
}


void bstr_reverse(Bstr * b) {
  char * s = b->h, * e = b->nt;

  while (e > s) {
    char temp = *s;
    *s = *e;
    *e = temp;
    ++s;
    --e;
  }
}


void bstr_reverse_tokens(Bstr ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bstr_reverse(&((*tokens)[i]));
  }
}


//TODO: rmatch_str, edit distance
char * bstr_lmatch_str(Bstr b, Bstr str) {
  char * c = b.h, * s = str.h;

  while (c <= b.nt) {
    if (*s == *c) {
      if (s == str.nt)
	return c - (s-str.nt);
      ++s;
    }
    ++c;
  }
  return NULL;
}

#endif

