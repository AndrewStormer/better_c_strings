#ifndef BETTER_STRINGS_H_
#define BETTER_STRINGS_H_


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


typedef struct bcs {
  char * h, * nt; //head and null terminator (not actually saving space for '\0', we know we are at the end if a pointer points to Bcs.nt.
} Bcs;

Bcs bcs_wrap(char * s);
char * bcs_unwrap(Bcs b);
void bcs_destroy(Bcs b);
void bcs_destroy_tokens(Bcs * tokens, int token_count);
Bcs bcs_subcs(char * s, char * e);
void bcs_print(Bcs b);
void bcs_print_tokens(Bcs * tokens, int token_count);
int bcs_strlen(Bcs b);
//static Bcs bcs_resize(Bcs b, int new_len);
void bcs_append_str(Bcs * b, char * s);
void bcs_append(Bcs * b, Bcs s);
void bcs_print(Bcs b);
//static int bcs_count_tokens(Bcs b, char delim);
//static char * bcs_next_token(Bcs b, char * h, char delim
Bcs * bcs_split(Bcs b, char delim, int * token_count, int keep_delim);
Bcs bcs_join(Bcs * b, int token_count);
void bcs_lclean(Bcs * b);
void bcs_lclean_tokens(Bcs ** tokens, int token_count);
void bcs_rclean(Bcs * b);
void bcs_rclean_tokens(Bcs ** tokens, int token_count);
void bcs_clean(Bcs * b);
void bcs_clean_tokens(Bcs ** tokens, int token_count);
void bcs_full_clean(Bcs * b);
int bcs_count_char(Bcs b, char c);
void bcs_reverse(Bcs * b);
void bcs_reverse_tokens(Bcs ** tokens, int token_count);
int bcs_lmatch(Bcs b, Bcs str);


#endif //BETTER_STRINGS_H_

#ifdef  BETTER_STRINGS_IMPLEMENTATION_


static int str_strlen(char * s) {
  char * c = s;
  while (*c != '\0') {
    ++c;
  }
  return (c - s) + 1;
}


Bcs bcs_wrap(char * s) {
  Bcs b;
  int len = str_strlen(s)-1;
  assert(len != 0 && "Cant wrap empty string!");
  b.h = malloc(sizeof(char)*len);
  assert(b.h != NULL);
  b.nt = b.h + len-1;
  
  for (int i = 0; i < len; ++i)
    b.h[i] = s[i];
    
  return b;
}


//TODO: implement a way to declare in static space so we do not have to free
//Problem: doing char str[bcs_strlen(b)+1] creates str in the stack, which does not persist after the function is returned from, and so we try to return a pointer to that stack space which changes when stack is used next.
//*** MUST FREE STRING PASSED BACK FROM FUNCTION
char * bcs_unwrap(Bcs b) {
  char * str = malloc(sizeof(char)*bcs_strlen(b) + 1);
  char * c = b.h;
  char * s = str;

  while (c <= b.nt) {
    *s = *c;
    ++s;
    ++c;
  }
  *s = '\0';
  bcs_destroy(b);
  return &str[0];
}


inline void bcs_destroy(Bcs b) {
  free(b.h);
}


inline void bcs_destroy_tokens(Bcs * tokens, int token_count) {
  for (int i = 0; i < token_count; ++i)
    bcs_destroy(tokens[i]);
}


Bcs bcs_subcs(char * s, char * e) {
  assert(e >= s);
  Bcs b;
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


void bcs_print(Bcs b) {
  char * c = b.h;
  while (c != b.nt + 1) {
    printf("%c", *c);
    ++c;
  }
  printf("\n");
}


void bcs_print_tokens(Bcs * tokens, int token_count) {
  for (int i = 0; i < token_count; i++) {
    bcs_print(tokens[i]);
  }
}
	     

//we do not keep an extra space for the null terminator
inline int bcs_strlen(Bcs b) {
  return (b.h == NULL) ? 0 : b.nt - b.h + 1;
}


static Bcs bcs_resize(Bcs b, int new_len) {
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
    bcs_destroy(b);
    b.h = s;
    b.nt = b.h + new_len-1;
  } else {
    b.h = malloc(sizeof(char)*new_len);
    assert(b.h != NULL);
  }
  return b;
}


void bcs_append_str(Bcs * b, char * s) {
  int len = bcs_strlen(*b), new_len = str_strlen(s)-1 + len;
  *b = bcs_resize(*b, new_len);
  char * c1 = (b->h + (len)), * c2 = s;
  while (c1 <= b->nt) {
    *c1 = *c2;
    ++c1;
    ++c2;
  }
  b->nt = b->h + new_len-1;
}


void bcs_append(Bcs * b, Bcs s) {
  int len = bcs_strlen(*b), new_len = bcs_strlen(s) + len;
  *b = bcs_resize(*b, new_len);
  char * c1 = (b->h + (len)), * c2 = s.h;
  while (c1 <= b->nt) {
    *c1 = *c2;
    ++c1;
    ++c2;
  }
  b->nt = b->h + new_len-1;
}


static int bcs_count_tokens(Bcs b, char delim) {
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


static char * bcs_next_delim(Bcs b, char * h, char delim) {
  while (*h != delim && h != b.nt) {
    ++h;
  }
  return h;
}


static char * bcs_next_non_delim(Bcs b, char * h, char delim) {
  while (*h == delim && h != b.nt) {
    ++h;
  }
  return h;
}


//token_count is passed by reference, when keep_delim is 1 we keep the last found delim (if multiple delims, it keeps all of them)
Bcs * bcs_split(Bcs b, char delim, int * token_count, int keep_delim) {
  *token_count = bcs_count_tokens(b, delim);
  Bcs * tokens = malloc(sizeof(Bcs)**token_count);
  assert(tokens != NULL);
  
  char * c = b.h, * d;
  for (int i = 0; i < *token_count; ++i) {
    d = bcs_next_delim(b, c, delim);
    if (keep_delim == 1 && d != b.nt) {
      d = bcs_next_non_delim(b, d, delim);
      --d;
    }
    tokens[i] = bcs_subcs(c, d - (1 - keep_delim));
    c = bcs_next_non_delim(b, d, delim);
  }
  return tokens;
}


Bcs bcs_join(Bcs * tokens, int token_count) {
  int joined_len = 0;
  for (int i = 0; i < token_count; ++i)
    joined_len += bcs_strlen(tokens[i]);
  
  Bcs b;
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


void bcs_lclean(Bcs * b) {
  char * c = b->h;
  while (*c == ' ') {
    ++c;
  }
  Bcs nb = bcs_subcs(c, b->nt);
  char * temp = b->h;
  b->h = nb.h;
  b->nt = nb.nt;
  free(temp);

}


void bcs_lclean_tokens(Bcs ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bcs_lclean(&((*tokens)[i]));
  }
}


void bcs_rclean(Bcs * b) {
  char * c = b->nt;
  while (*c == ' ') {
    --c;
  }

  Bcs nb = bcs_subcs(b->h, c);
  char * temp = b->h;
  b->h = nb.h;
  b->nt = nb.nt;
  free(temp);
}


void bcs_rclean_tokens(Bcs ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bcs_rclean(&((*tokens)[i]));
  }
}


void bcs_clean(Bcs * b) {
  bcs_lclean(b);
  bcs_rclean(b);
}


void bcs_clean_tokens(Bcs ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bcs_clean(&((*tokens)[i]));
  }
}


void bcs_full_clean(Bcs * b) {
  int token_count = 0;
  Bcs * tokens = bcs_split(*b,  ' ', &token_count, 0);
  bcs_clean_tokens(&tokens, token_count);
  *b = bcs_join(tokens, token_count);
  bcs_destroy_tokens(tokens, token_count);
}


int bcs_count_char(Bcs b, char c) {
  int count = 0;
  for (char * p = b.h; p <= b.nt; ++p) {
    if (*p == c)
      ++count;
  }
  return count;
}


void bcs_reverse(Bcs * b) {
  char * s = b->h, * e = b->nt;

  while (e > s) {
    char temp = *s;
    *s = *e;
    *e = temp;
    ++s;
    --e;
  }
}


void bcs_reverse_tokens(Bcs ** tokens, int token_count) {
  for (int i = 0; i < token_count; ++i) {
    bcs_reverse(&((*tokens)[i]));
  }
}


//TODO: rmatch_str, edit distance
int bcs_lmatch(Bcs b, Bcs str) {
  char * c = b.h, * s = str.h;

  while (c <= b.nt) {
    if (*s == *c) {
      if (s == str.nt)
	return (c  - b.h) - bcs_strlen(str) + 1ss;
      ++s;
    } else
      s = str.h;
    ++c;
  }
  return -1;
}

#endif
