#ifndef BETTER_STRINGS_H_
#define BETTER_STRINGS_H_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>


typedef struct bcs bcs_t;

bcs_t *bcs_wrap(char * s);
char *bcs_unwrap(bcs_t *b);
void bcs_destroy(bcs_t *b);
void bcs_destroy_tokens(bcs_t ** tokens, uint32_t token_count);
bcs_t *bcs_substr(char * s, char * e);
void bcs_print(bcs_t *b);
void bcs_print_tokens(bcs_t **tokens, uint32_t token_count);
size_t bcs_strlen(bcs_t *b);
//static Bcs bcs_resize(Bcs b, uint32_t new_len);
void bcs_append_str(bcs_t *b, char * s);
void bcs_append(bcs_t *b, bcs_t *s);
//static uint32_t bcs_count_tokens(Bcs b, char delim);
//static char * bcs_next_token(Bcs b, char * h, char delim
bcs_t **bcs_split(bcs_t *b, char delim, uint32_t *token_count, uint8_t keep_delim);
bcs_t *bcs_join(bcs_t **b, uint32_t token_count);
void bcs_lclean(bcs_t *b);
void bcs_lclean_tokens(bcs_t **tokens, uint32_t token_count);
void bcs_rclean(bcs_t *b);
void bcs_rclean_tokens(bcs_t **tokens, uint32_t token_count);
void bcs_clean(bcs_t **b);
void bcs_clean_tokens(bcs_t ***tokens, uint32_t token_count);
void bcs_full_clean(bcs_t *b);
uint32_t bcs_count_char(bcs_t *b, char c);
void bcs_reverse(bcs_t *b);
void bcs_reverse_tokens(bcs_t **tokens, uint32_t token_count);
uint32_t bcs_lmatch(bcs_t *b, bcs_t *str);


#endif //BETTER_STRINGS_H_

#ifdef  BETTER_STRINGS_IMPLEMENTATION_

//Opaque data structure
struct bcs {
  char * h, * nt; //head and null terminator (not actually saving space for '\0', we know we are at the end if a pointer points to Bcs.nt.
};

//NOTE: These are internal functions to be reused by Bcs interface functions; Those should be used instead

static size_t str_strlen(char * s) {
  char * c = s;
  while (*c != '\0') {
    ++c;
  }
  return (c - s) + 1;
}

static bcs_t *bcs_create(size_t len) {
  bcs_t * b = malloc(sizeof(bcs_t));
  if (!b)
      return NULL;

  b->h = malloc(sizeof(char)*len);
  if (!b->h) {
    free(b);
    return NULL;
  }
  b->nt = b->h + len-1;

  return b;
}


static uint32_t bcs_count_tokens(bcs_t *b, char delim) {
  if (!b)
    return -1;

  char * c = b->h;
  uint32_t count = 0;
  if (*c == delim)
    ++count;
  ++c;
  while (c != b->nt+1) {
    if ((*c == delim || c == b->nt) && *(c-1) != delim)
      ++count;
    ++c;
  }
  return count;
}


static char * bcs_next_delim(bcs_t *b, char * h, char delim) {
  while (*h != delim && h != b->nt) {
    ++h;
  }
  return h;
}


static char * bcs_next_non_delim(bcs_t *b, char * h, char delim) {
  while (*h == delim && h != b->nt) {
    ++h;
  }
  return h;
}

static bcs_t *bcs_resize(bcs_t *b, size_t new_len) {
  if (!b || new_len == 0)
    return b;
  if (b->h) {
    char *s = malloc(sizeof(char)*new_len);
    if (!s)
      return NULL;

    char *c = s;
    size_t i = 0;
    while (i <= b->nt - b->h + 1) {
      *c = b->h[i];
      ++i;
      ++c;
    }
    free(b->h);

    b->h = s;
    b->nt = b->h + new_len-1;
  } else {
    b->h = malloc(sizeof(char)*new_len);
    if (!b->h)  {
      free(b);
      return NULL;
    }
  }
  return b;
}


//BCS interface function:

bcs_t *bcs_wrap(char * s) {
  if (!s) 
    return NULL;

  size_t len = str_strlen(s)-1;
  if (len == 0) {
    printf("ERROR: Cannot wrap empty string.\n");
    return NULL;
  }
  bcs_t *b = bcs_create(len);
  if (!b)
    return NULL;

  for (size_t i = 0; i < len; ++i)
    b->h[i] = s[i];
    
  return b;
}


//TODO: implement a way to declare in static space so we do not have to free
//Problem: doing char str[bcs_strlen(b)+1] creates str in the stack, which does not persist after the function is returned from, and so we try to return a pointer to that stack space which changes when stack is used next.
//*** MUST FREE STRING PASSED BACK FROM FUNCTION
char *bcs_unwrap(bcs_t *b) {
  if (!b)
    return NULL;

  char *str = malloc(sizeof(char)*(bcs_strlen(b)+1));
  if (!str)
    return NULL;

  char *c = b->h;
  char *s = str;
  while (c <= b->nt) {
    *s = *c;
    ++s;
    ++c;
  }
  *s = '\0';
  bcs_destroy(b);
  return &str[0];
}


void bcs_destroy(bcs_t *b) {
  if (b) {
    if (b->h)
      free(b->h);
    free(b);
  }
}


inline void bcs_destroy_tokens(bcs_t **tokens, uint32_t token_count) {
  for (size_t i = 0; i < token_count; ++i)
    bcs_destroy(tokens[i]);
}


bcs_t *bcs_substr(char *s, char *e) {
  if (!s || !e)
    return NULL;
  bcs_t *b = malloc(sizeof(bcs_t));
  if (!b)
    return NULL;

  b->h = malloc(sizeof(char)*(e-s)+1);
  if (!b->h) {
    free(b);
    return NULL;
  }

  b->nt = b->h;
  char *c = s;
  while (c <= e) {
    *b->nt = *c;
    ++c;
    ++b->nt;
  }
  return b;
}


void bcs_print(bcs_t *b) {
  char * c = b->h;
  while (c != b->nt + 1) {
    printf("%c", *c);
    ++c;
  }
  printf("\n");
}


void bcs_print_tokens(bcs_t **tokens, uint32_t token_count) {
  for (size_t i = 0; i < token_count; i++) {
    bcs_print(tokens[i]);
  }
}
	     

//we do not keep an extra space for the null terminator
inline size_t bcs_strlen(bcs_t *b) {
  return (b->h == NULL) ? 0 : b->nt - b->h + 1;
}


void bcs_append_str(bcs_t *b, char *s) {
  if (b && s) {
    size_t len = bcs_strlen(b);
    size_t new_len = str_strlen(s)-1 + len;
    b = bcs_resize(b, new_len);
    char * c1 = ((b)->h + len);
    char * c2 = s;
    while (c1 <= (b)->nt) {
      *c1 = *c2;
      ++c1;
      ++c2;
    }
    (b)->nt = (b)->h + (new_len-1);
  }
}


void bcs_append(bcs_t *b, bcs_t *s) {
  size_t len = bcs_strlen(b);
  size_t new_len = bcs_strlen(s) + len;
  b = bcs_resize(b, new_len);
  char * c1 = (b->h + (len)), * c2 = s->h;
  while (c1 <= b->nt) {
    *c1 = *c2;
    ++c1;
    ++c2;
  }
  b->nt = b->h + new_len-1;
}




//token_count is passed by reference, when keep_delim is 1 we keep the last found delim (if multiple delims, it keeps all of them)
bcs_t **bcs_split(bcs_t *b, char delim, uint32_t *token_count, uint8_t keep_delim) {
  if (!b)
    return NULL;
  
  *token_count = bcs_count_tokens(b, delim);
  if (*token_count <= 1)
    return NULL;
  bcs_t **tokens = malloc(sizeof(bcs_t *)*(*token_count));
  if (!tokens)
      return NULL;

  char * c = b->h, * d;
  for (size_t i = 0; i < *token_count; ++i) {
    d = bcs_next_delim(b, c, delim);
    if (keep_delim == 1 && d != b->nt) {
      d = bcs_next_non_delim(b, d, delim);
      --d;
    }
    tokens[i] = bcs_substr(c, d - (1 - keep_delim));
    c = bcs_next_non_delim(b, d, delim);
  }
  return tokens;
}


bcs_t *bcs_join(bcs_t **tokens, uint32_t token_count) {
  if (!tokens || token_count == 0)
    return NULL;
  uint32_t joined_len = 0;
  for (size_t i = 0; i < token_count; ++i)
    joined_len += bcs_strlen(tokens[i]);
  
  bcs_t *b = bcs_create(joined_len);
  if (!b)
    return NULL;

  char * h = b->h;
  for (size_t i = 0; i < token_count; ++i) {
    char * c = tokens[i]->h;
    while (c != tokens[i]->nt + 1) {
      *h = *c;
      ++c;
      ++h;
    }
  }
  b->nt = b->h + joined_len - 1;
  
  return b;
}


void bcs_lclean(bcs_t *b) {
  if (b) {
    char * c = b->h;
    while ((int)*c == ' ') {
      ++c;
    }

    if (c != b->h) {
      bcs_t *nb = bcs_substr(c, b->nt);
      if (nb && b != nb) {
        bcs_destroy(b);
        b = nb;
      }
    }
  }
}


void bcs_lclean_tokens(bcs_t **tokens, uint32_t token_count) {
  if (tokens && token_count > 0) {
    for (size_t i = 0; i < token_count; ++i) {
      bcs_lclean(tokens[i]);
    }
  }
}


void bcs_rclean(bcs_t *b) {
  if (!b) {
    char * c = b->nt;
    while ((int)*c == ' ') {
      --c;
    }

    if (c != b->nt) {
      bcs_t *nb = bcs_substr(b->h, c);
      if (nb && b != nb) {
        bcs_destroy(b);
        b = nb;
      }
    }
  }
}


void bcs_rclean_tokens(bcs_t **tokens, uint32_t token_count) {
  if (tokens && token_count > 0) {
    for (size_t i = 0; i < token_count; ++i) {
      bcs_rclean(tokens[i]);
    }
  }
}


void bcs_clean(bcs_t **b) {
  if (b && *b) {
    char * c1 = (*b)->h;
    while ((int)*c1 == ' ')
      ++c1;
    
    char * c2 = (*b)->nt;
    while ((int)*c2 == ' ')
      --c2;
    
    if (c1 != (*b)->h && c2 != (*b)->nt) {
      bcs_t *nb = bcs_substr(c1, c2);
      if (nb && &nb != b) {
        bcs_destroy(*b);
        b = &nb;
      }
    }
  }
}


void bcs_clean_tokens(bcs_t ***tokens, uint32_t token_count) {
  if (tokens && token_count > 0) {
    for (size_t i = 0; i < token_count; ++i) {
      bcs_clean(&(*tokens)[i]);
    }
  }
}


void bcs_full_clean(bcs_t * b) {
  if (b) {
    uint32_t token_count = 0;
    bcs_t **tokens = bcs_split(b,  ' ', &token_count, 0);
    bcs_clean_tokens(&tokens, token_count);
    b = bcs_join(tokens, token_count);
    bcs_destroy_tokens(tokens, token_count);
  }
}


uint32_t bcs_count_char(bcs_t *b, char c) {
  uint32_t count = 0;
  for (char * p = b->h; p <= b->nt; ++p) {
    if (*p == c)
      ++count;
  }
  return count;
}


void bcs_reverse(bcs_t *b) {
  char * s = b->h, * e = b->nt;

  while (e > s) {
    char temp = *s;
    *s = *e;
    *e = temp;
    ++s;
    --e;
  }
}


void bcs_reverse_tokens(bcs_t ** tokens, uint32_t token_count) {
  for (size_t i = 0; i < token_count; ++i) {
    bcs_reverse(tokens[i]);
  }
}


//TODO: rmatch_str, edit distance
uint32_t bcs_lmatch(bcs_t *b, bcs_t *str) {
  char * c = b->h, * s = str->h;

  while (c <= b->nt) {
    if (*s == *c) {
      if (s == str->nt)
	return (c - b->h) - bcs_strlen(str) + 1;
      ++s;
    } else
      s = str->h;
    ++c;
  }
  return -1;
}

#endif
