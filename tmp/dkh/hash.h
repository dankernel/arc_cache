/*
 * =====================================================================================
 *
 *       Filename:  hash.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014년 11월 08일 00시 54분 52초
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         author:   (), 
 *   organization:  
 *
 * =====================================================================================
 */

#ifdef dk_list
#else
#include "dk_list.h"
#endif

#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

static __always_inline unsigned int hash_64(unsigned int val, unsigned int bits)
{/*{{{*/
  unsigned int hash = val;

  /*   Sigh, gcc can't optimise this alone like it does for 32 bits. */
  unsigned int n = hash;
  n <<= 18;
  hash -= n;
  n <<= 31;
  hash -= n;
  n <<= 3;
  hash += n;
  n <<= 3;
  hash -= n;
  n <<= 4;
  hash += n;
  n <<= 2;
  hash += n;

  /*  High bits are more random, so use them. */
  return hash >> (64 - bits);
}/*}}}*/
 
struct hash_row
{
  void *p;
  struct list_head head;
};

struct hash_table
{
  int len;
  struct hash_row *row;
};

int add_hash_table(struct hash_table *ht, unsigned int val)
{
  struct hash_row *rr = NULL;

  rr = ht->row;

  hash_64(val, 32);

  return 0;
}

int lookup_hash(struct hash_table *ht)
{
  struct hash_row *rr = NULL;

  rr = ht->row;
  printf("%d\n", rr[0]);

  return 0;
}

struct hash_table *init_hash(int len)
{
  struct hash_table *ret = NULL;

  if (len < 0)
    return NULL;

  ret = malloc(sizeof(struct hash_table));
  ret->len = len;
  ret->row = malloc(sizeof(struct hash_row) * len);

  memset(ret->row, sizeof(struct hash_row) * len, '\0');
  return ret;


  return 0;
}
