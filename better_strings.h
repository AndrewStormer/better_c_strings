#ifndef BETTER_STRINGS_H_
#define BETTER_STRINGS_H_

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


typedef struct bstr {
  char * h, * nt; //head and null terminator (not actually saving space for '\0', we know we are at the end if a pointer points to bstr.nt.
} bstr_t;

//Create/Destroy functions
bstr_t *bstr_wrap(char * s);
char *bstr_unwrap(bstr_t *b);
uint16_t bstr_destroy(bstr_t *b);
uint16_t bstr_destroy_tokens(bstr_t ** tokens, uint16_t token_count);

//Printing functions
void bstr_print(bstr_t *b);
void bstr_print_tokens(bstr_t **tokens, uint16_t token_count);

//String utility
size_t bstr_strlen(bstr_t *b);
uint16_t bstr_count_char(bstr_t *b, char c);

//String Algorithms
bstr_t *bstr_substr(bstr_t *b, uint32_t start, size_t len);
uint16_t bstr_append_str(bstr_t *b, char * s);
uint16_t bstr_append(bstr_t *b, bstr_t *s);
bstr_t **bstr_split(bstr_t *b, char delim, uint16_t *token_count, uint8_t keep_delim);
bstr_t *bstr_join(bstr_t **b, uint16_t token_count);
uint16_t bstr_reverse(bstr_t *b);
uint16_t bstr_reverse_tokens(bstr_t **tokens, uint16_t token_count);
uint16_t bstr_lmatch(bstr_t *b, bstr_t *str);
uint16_t bstr_rmatch(bstr_t *b, bstr_t *str);

//Cleaning Algorithms
uint16_t bstr_lclean(bstr_t **b);
uint16_t bstr_lclean_tokens(bstr_t **tokens, uint16_t token_count);
uint16_t bstr_rclean(bstr_t **b);
uint16_t bstr_rclean_tokens(bstr_t **tokens, uint16_t token_count);
uint16_t bstr_clean(bstr_t **b);
uint16_t bstr_clean_tokens(bstr_t **tokens, uint16_t token_count);
uint16_t bstr_full_clean(bstr_t *b);

//Serialize/Deserialize functions
uint16_t bstr_serialize(bstr_t *b, char *filename);
bstr_t *bstr_deserialize(char *filename);


#endif //BETTER_STRINGS_H_

#ifdef  BETTER_STRINGS_IMPLEMENTATION_

//NOTE: These are internal functions to be used by bstr interface functions

//Returns the length of a null terminated array of characters
static size_t str_strlen(char * s) {
  char * c = s;
  while (*c != '\0') {
    ++c;
  }
  return (c - s)+1;
}


//creates an empty bstr_t. returns NULL for any errors.
static bstr_t *bstr_create(size_t len) {
  bstr_t * b = malloc(sizeof(bstr_t));
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


//counts the number of tokens given a delimitter. returns -1 for any error
static uint16_t bstr_count_tokens(bstr_t *b, char delim) {
  if (!b)
    return -1;

  char * c = b->h;
  uint16_t count = 0;
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


//Finds the next delimitter in b where h is a character pointer into the b->h
//Returns the pointer to the position of the next delimitter (or the nt), or NULL if any errors occur
static char * bstr_next_delim(bstr_t *b, char * h, char delim) {
  if (!b || !h)
    return NULL;

  while (*h != delim && h != b->nt) {
    ++h;
  }
  return h;
}


//Finds the next non delimitter where h is a character pointer into b->h
//Returns the pointer to the position of the next non delimitter character (or the nt),  or NULL if any errors occur
static char * bstr_next_non_delim(bstr_t *b, char * h, char delim) {
  if (!b || !h)
    return NULL;

  while (*h == delim && h != b->nt) {
    ++h;
  }
  return h;
}

//Resizes the bstr, filling the content that was there originally and leaving the new content initialized to 0
//Returns resized bstr, or NULL on error
static bstr_t *bstr_resize(bstr_t *b, size_t new_len) {
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
    b->h = calloc(new_len, sizeof(char));
    if (!b->h)  {
      free(b);
      return NULL;
    }
  }
  return b;
}

//DANGEROUS: IF s AND e ARE NOT POINTERS TO THE SAME bstr_t, LONG LOOP
//Creates a new bstr_t substring that runs from s to e
//Returns the substr, or NULL on error
static bstr_t *bstr_substr_s(char *s, char *e) {
  if (!s || !e || (e-s) < 0)
    return NULL;
  
  size_t len = (e-s)+1;
  bstr_t *b = bstr_create(len);

  char * ptr = b->h;
  for (char * i = s; i <= e; ++i) {
    *ptr = *i;
    ++ptr;
  }
  return b;
}


//bstr_t interface functions:

//*** USER ALWAYS RESPONSIBLE FOR FREEING BSTR MEMORY USING GIVEN DESTROY FUNCTIONS IF NOT UNWRAPPED
//*** THIS MEANS SERIALIZE, JOIN, ETC. DO NOT DESTROY THE GIVEN BSTR
//Wraps a null-terminated string s into a new bstr_t struct pointer
//Returns the new bstr_t pointer, or NULL on error
bstr_t *bstr_wrap(char * s) {
  if (!s) 
    return NULL;

  size_t len = str_strlen(s)-1;
  if (len == 0)
    return NULL;

  bstr_t *b = bstr_create(len);
  if (!b)
    return NULL;

  for (size_t i = 0; i < len; ++i)
    b->h[i] = s[i];
    
  return b;
}


//TODO: implement a way to declare in static space so we do not have to free
//Problem: doing char str[bstr_strlen(b)+1] creates str in the stack, which does not persist after the function is returned from, and so we try to return a pointer to that stack space which changes when stack is used next.
//*** MUST FREE STRING PASSED BACK FROM FUNCTION
//Unwraps a bstr_t struct pointer into a null-terminated dynamic string
//Returns the unwrapped string, or NULL on error
char *bstr_unwrap(bstr_t *b) {
  if (!b)
    return NULL;

  char *str = malloc(sizeof(char)*(bstr_strlen(b)+1));
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
  bstr_destroy(b);
  return &str[0];
}


//Destroys a bstr_t struct pointer
//Returns 0 on success, -1 for any failure
uint16_t bstr_destroy(bstr_t *b) {
  if (!b)
    return -1;
  if (!b->h) {
    free(b);
    return -1;
  }
  free(b->h);
  free(b);
  return 0;
}


//Destroys the array of bstr_t struct pointers
//Returns 0 on success, -1 for any failure
inline uint16_t bstr_destroy_tokens(bstr_t **tokens, uint16_t token_count) {
  for (size_t i = 0; i < token_count; ++i) {
    if (bstr_destroy(tokens[i]) < 0)
      return -1;
  }
  return 0;
}


//Prints a bstr_r pointer
void bstr_print(bstr_t *b) {
  if (b) {
    char * c = b->h;
    while (c != b->nt + 1) {
      printf("%c", *c);
      ++c;
    }
    printf("\n");
  }
}


//Prints an array of bstr_t pointers
void bstr_print_tokens(bstr_t **tokens, uint16_t token_count) {
  if (tokens && token_count > 0) {
    for (size_t i = 0; i < token_count; i++) {
      bstr_print(tokens[i]);
    }
  }
}

	     
//we do not keep an extra space for the null terminator
//Returns the length of a bstr_t, or 0 for invalid bstr_t
inline size_t bstr_strlen(bstr_t *b) {
  return (!b || !b->h) ? 0 : b->nt - b->h + 1;
}


//Counts the instances of a specific character in the string
//Returns the count on success, -1 for any failure
uint16_t bstr_count_char(bstr_t *b, char c) {
  if (!b)
    return -1;
  
  uint16_t count = 0;
  for (char * p = b->h; p <= b->nt; ++p) {
    if (*p == c)
      ++count;
  }
  return count;
}


//Creates a new bstr_t substring that runs from s to e
//Returns the substr, or NULL on error
bstr_t *bstr_substr(bstr_t *b, uint32_t start, size_t len) {
  if (!b || start < 0 || start > bstr_strlen(b) || len == 0)
    return NULL;

  bstr_t *s = bstr_create(len);
  if (!s)
    return NULL;
  
  char *h = s->h;
  char* c = b->h + start;
  for (; c < b->h + (start + len) && c < b->nt; ++c) {
    *h = *c;
    ++h;
  }
  return s;
}


//Appends a null terminated string onto the end of b
//Returns 0 on success, -1 for any failure
uint16_t bstr_append_str(bstr_t *b, char *s) {
  if (!b || !s)
    return -1;

  size_t len = bstr_strlen(b);
  size_t new_len = str_strlen(s) - 1 + len;
  if (len < 0 || new_len < 0) //should never happen
    return -1;
  
  b->nt = b->h + new_len-1;

  b = bstr_resize(b, new_len);
  if (!b)
    return -1;
  char *c1 = ((b)->h + len);
  while (c1 <= (b)->nt) {
    *c1 = *s;
    ++c1;
    ++s;
  }
  return 0;
}


//*** DOES NOT DESTROY s
//Appends a bstr_t s to the end of another bstr_t b
//Returns 0 on success, -1 for any failure
uint16_t bstr_append(bstr_t *b, bstr_t *s) {
  if (!b || !s)
    return -1;
  size_t len = bstr_strlen(b);
  size_t new_len = bstr_strlen(s) + len;
  if (len < 0 || new_len < 0) //should never happen
    return -1;


  b = bstr_resize(b, new_len);
  if (!b)
    return -1;

  b->nt = b->h + new_len-1;

  char * c1 = (b->h + (len)), * c2 = s->h;
  while (c1 <= b->nt) {
    *c1 = *c2;
    ++c1;
    ++c2;
  }
  return 0;
}


//*** DOES NOT DESTROY b
//Splits the bstr_t into tokens given the delim
//parameter token_count passed by reference; to be changed by the function
//parameter keep_delim is 0 if tokens should not contain the delimitter provided, 1 if they should
//Returns token_count amount of tokens, split from the bstr_t *b given by delim
bstr_t **bstr_split(bstr_t *b, char delim, uint16_t *token_count, uint8_t keep_delim) {
  if (!b || !token_count)
    return NULL;
  
  uint8_t kd = 0;
  if (keep_delim != 0)
    kd = 1; //To accept user input of keep_delim other than 0 or 1 (if not 0, assume 1)
  
  *token_count = bstr_count_tokens(b, delim);
  if (*token_count < 1)
    return NULL;
  bstr_t **tokens = malloc(sizeof(bstr_t *)*(*token_count));
  if (!tokens)
      return NULL;

  char * c = b->h;
  char * d;
  for (size_t i = 0; i < *token_count; ++i) {
    d = bstr_next_delim(b, c, delim);
    if (kd == 1 && d != b->nt) {
      d = bstr_next_non_delim(b, d, delim);
      --d;
    }
    tokens[i] = bstr_substr_s(c, d - (1 - kd));
    c = bstr_next_non_delim(b, d, delim);
  }
  return tokens;
}


//*** DOES NOT DESTROY tokens
//Joins token_count tokens into 1 long bstr_t
//returns the joined bstr_t, or NULL on faliure
bstr_t *bstr_join(bstr_t **tokens, uint16_t token_count) {
  if (!tokens || token_count == 0)
    return NULL;
  uint16_t joined_len = 0;
  for (size_t i = 0; i < token_count; ++i)
    joined_len += bstr_strlen(tokens[i]);
  
  bstr_t *b = bstr_create(joined_len);
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


//Reverses the given bstr_t in place
//Returns 0 on success, -1 for any failure
uint16_t bstr_reverse(bstr_t *b) {
  if (!b) 
    return -1;
  char * s = b->h, * e = b->nt;

  while (e > s) {
    char temp = *s;
    *s = *e;
    *e = temp;
    ++s;
    --e;
  }
  return 0;
}


//Reverses token_count bstr_t's
//Returns 0 on success, -1 for any failure
uint16_t bstr_reverse_tokens(bstr_t ** tokens, uint16_t token_count) {
  if (!tokens || token_count < 1)
    return -1;
  for (size_t i = 0; i < token_count; ++i) {
    if (bstr_reverse(tokens[i]) < 0)
      return -1;
  }
  return 0;
}


//Returns the index of the first letter of the first occurence of str on success
//Or -2 for invalid parameters, or -1 for str not in b
uint16_t bstr_lmatch(bstr_t *b, bstr_t *str) {
  if (!b || !b->h || !str || !str->nt)
    return -2;

  char * c = b->h;
  char * s = str->h;

  while (c <= b->nt) {
    if (*s == *c) {
      if (s == str->nt)
	return (c - b->h) - bstr_strlen(str) + 1;
      ++s;
    } else
      s = str->h;
    ++c;
  }
  return -1;
}


//Returns the index of the first letter of the last occurence of str on success
//Returns -2 for invalid parameters, or -1 for str not in b
uint16_t bstr_rmatch(bstr_t *b, bstr_t *str) {
  if (!b || !b->h || !str || !str->nt)
    return -2;

  char * c = b->nt;
  char * s = str->nt;

  while (c <= b->nt) {
    if (*s == *c) {
      if (s == str->nt)
	return (c - b->h) - bstr_strlen(str) + 1;
      --s;
    } else
      s = str->nt;
    --c;
  }
  return -1;
}


//Deletes all the whitespace on the left of the string
//Returns 0 on success, -1 for any failure
uint16_t bstr_lclean(bstr_t **b) {
  if (!*b || !(*b)->h)
    return -1;

  if ((int)*(*b)->h != ' ')
    return 0;
  char *c = (*b)->h;
  while ((int)*c == ' ')
    ++c;

  bstr_t *nb = bstr_substr_s(c, (*b)->nt);
  if (nb && b != &nb) {
    bstr_destroy(*b);
    *b = nb;
    (*b)->h = nb->h;
    (*b)->nt = nb->nt;
  } else 
    return -1;
  return 0;
}


//Deletes all the whitespace on the left of token_count tokens
//Returns 0 on success, -1 for any failure
uint16_t bstr_lclean_tokens(bstr_t **tokens, uint16_t token_count) {
  if (!tokens || token_count <= 0) 
    return -1;
  for (size_t i = 0; i < token_count; ++i) {
   if (bstr_lclean(&tokens[i]) < 0)
    return -1;
  }

  return 0;
}


//Deletes all the whitespace on the right of the string
//Returns 0 on success, -1 for any failure
uint16_t bstr_rclean(bstr_t **b) {
  if (!*b || !(*b)->h)
    return -1;

  if ((int)*(*b)->nt != ' ')
    return 0;
  char *c = (*b)->nt;
  while ((int)*c == ' ')
    --c;

  bstr_t *nb = bstr_substr_s((*b)->h, c);
  if (nb && b != &nb) {
    bstr_destroy(*b);
    *b = nb;
    (*b)->h = nb->h;
    (*b)->nt = nb->nt;
  } else 
    return -1;
  return 0;
}


//Deletes all the whitespace on the right of token_count tokens
//Returns 0 on success, -1 for any failure
uint16_t bstr_rclean_tokens(bstr_t **tokens, uint16_t token_count) {
  if (!tokens || token_count <= 0) 
    return -1;
  for (size_t i = 0; i < token_count; ++i) {
   if (bstr_rclean(&tokens[i]) < 0)
    return -1;
  }

  return 0;
}


//Deletes all the whitespace on both sides of the string
//Returns 0 on success, -1 for any failure
uint16_t bstr_clean(bstr_t **b) {
  if (!b && !*b)
    return -1;

  if ((int)*(*b)->h != ' ' && (int)*(*b)->nt != ' ')
    return 0;

  char *c1 = (*b)->h;
  while ((int)*c1 == ' ')
    ++c1;

  char *c2 = (*b)->nt;
  while ((int)*c2 == ' ')
    --c2;

  bstr_t *nb = bstr_substr_s(c1, c2);
  if (nb && &nb != b) {
    bstr_destroy(*b);
    *b = nb;
    (*b)->h = nb->h;
    (*b)->nt = nb->nt;
  }
  return 0;
}


//Deletes all the whitespace on both sides of token_count tokens
//Returns 0 on success, -1 for any failure
uint16_t bstr_clean_tokens(bstr_t **tokens, uint16_t token_count) {
  if (!tokens || token_count < 1)
    return -1;

  for (size_t i = 0; i < token_count; ++i) {
      if (bstr_clean(&tokens[i]) < 0)
        return -1;
  }

  return 0;
}


//Deletes all of the whitespace in the string
//Returns 0 on success, -1 for any failure
uint16_t bstr_full_clean(bstr_t * b) {
  if (!b)
    return -1;

  uint16_t token_count = 0;
  bstr_t **tokens = bstr_split(b,  ' ', &token_count, 0);
  bstr_destroy(b);
  bstr_clean_tokens(tokens, token_count);
  b = bstr_join(tokens, token_count);
  bstr_destroy_tokens(tokens, token_count);

  return 0;
}


//*** DOES NOT DESTROY b
//serializes the data in b->h into a new file with the filename provided
//returns 0 for success, -1 for failure 
uint16_t bstr_serialize(bstr_t *b, char *filename) {
  if (!b || !b->h || !filename)
    return -1;

  int fd = open(filename, O_CREAT|O_TRUNC|O_WRONLY, 0660);
  if (fd < 0)
    return -1;

  size_t len = bstr_strlen(b);
  size_t bytes_written = (size_t)write(fd, &len, sizeof(size_t));
  if (bytes_written < sizeof(size_t)) {
    close(fd);
    return -1;
  }

  bytes_written = (size_t)write(fd, b->h, len);
  close(fd);
  if (bytes_written < len)
    return -1;
  return 0;
}


//deserializes the data in file with filename provided into the b->h of a new bstr_t *b
//returns b for success, NULL for failure 
bstr_t *bstr_deserialize(char *filename) {
  if (!filename)
    return NULL;

  int fd = open(filename, O_RDONLY);
  if (fd < 0)
    return NULL;

  size_t len = 0;
  size_t bytes_read = (size_t)read(fd, &len, sizeof(size_t));
  if (bytes_read < sizeof(size_t)) {
    close(fd);
    return NULL;
  }

  bstr_t *b = bstr_create(len);
  if (!b) {
    close(fd);
    return NULL;
  }
  bytes_read = read(fd, b->h, len);
  close(fd);
  if (bytes_read < len) {
    bstr_destroy(b);
    return NULL;
  }  
  return b;
}

#endif
