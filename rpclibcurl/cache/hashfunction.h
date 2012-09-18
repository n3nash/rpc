
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>

#define cachesize 64

struct listofall{
char *url;
char *value;
int count;
int frequency;
struct listofall *nxt;
struct listofall *prev;
};

struct hashtab1
{
struct listofall *data;
};

typedef struct hashtab1 hashtab1;


unsigned int DEKHash(char* str, unsigned int len)
{
   unsigned int hash = len;
   unsigned int i    = 0;

   for(i = 0; i < len; str++, i++)
   {
      hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
   }
   return hash;
}

//typdef struct hashtab hashtab;
