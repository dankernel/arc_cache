/**
 * =====================================================================================
 *
 *          @file:  lru.h
 *         @brief:  LRU에 대한 라이브러리
 *
 *        Version:  1.0
 *          @date:  2014년 11월 23일 19시 16분 39초
 *       Revision:  none
 *       Compiler:  gcc
 *
 *        @author:  Park jun hyung (), google@dankook.ac.kr
 *       @COMPANY:  Dankopok univ.
 * =====================================================================================
 */

#include <math.h>
#include <stdio.h>
#include "dk_list.h"
#include "errno.h"
#include "md5.h"
#include "md5.c"

/* Get.. */
#include <memory.h>
#include <stddef.h>

#define list_entry(ptr, type, field) \
  ((type*) (((char*)ptr) - offsetof(type,field)))

#define list_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)

#define MAX(a, b) ( (a > b) ? (a) : (b) )
#define MIN(a, b) ( (a < b) ? (a) : (b) )

/* READ WRITE FLAGS */
#define READ  1
#define WRITE 2

/* SIZE */
#define KB (1024)
#define MB (KB * KB)
#define GB (MB * KB)

/* CONFIG */
#define CACHE_BLOCK_SIZE (4 * KB)
#define CACHE_SIZE (128 * MB)
#define CACHE_LEN (CACHE_SIZE/ CACHE_BLOCK_SIZE)

#define DEBUG_OPTION 0

struct workload
{/*{{{*/
  char *time;
  char *host;
  int disk_num;
  int type;
  long long offset;
  double size;
  long respone; //respones time

};/*}}}*/

struct cache_line
{/*{{{*/
  long long line;
  struct list_head head;
  struct list_head hash;
  struct cache_state *state;
  unsigned char md5[33];
};/*}}}*/

struct cache_state
{/*{{{*/
  long size;
  struct list_head head;
};/*}}}*/

struct arc_hash
{/*{{{*/
  unsigned long size;
  struct list_head *bucket;
};/*}}}*/

struct cache_mem
{/*{{{*/
  /* unsigned long c, p; */
  long c, p;
  struct cache_state mrug, mru, mfu, mfug;

  long size;
  long max;

  long read;
  long write;
  long hit;

  struct arc_hash hash;
};/*}}}*/

unsigned long atonum(char *c)
{/*{{{*/
  unsigned long ret = 0;
  int i = 0, len = strlen(c);
  while (len--) {
    ret += c[i++];
    ret <<= 4;
  }
  return ret;
}/*}}}*/

int init_hash_list(struct cache_mem *cm, unsigned long s);
struct cache_mem *init_cache_mem(unsigned long c);
void report_cm(struct cache_mem *cm);
int print_cm(struct cache_mem *cm);
static int *get_hash(char *ret, double test);
struct cache_line *ARC_state_lru(struct cache_state *state);
int contain_list(struct cache_mem *cm, struct cache_line *l);
struct cache_line *ARC_move(struct cache_mem *cm, struct cache_line *l, struct cache_state *state);
static void ARC_balance(struct cache_mem *cm, unsigned long size);
static inline struct cache_line *ARC_print(struct list_head *start);

int init_hash_list(struct cache_mem *cm, unsigned long s)
{/*{{{*/
  int i = 0;

  cm->hash.size = s;
  cm->hash.bucket = malloc(s * sizeof(struct list_head));

  for (i = 0; i < s; i++) {
    init_list(&cm->hash.bucket[i]);
  }

  return 0;
}/*}}}*/

/**
 * Init cache memory.
 * @param c : cache size (max list size)
 * @return : cache_mem strcut pointer
 */
struct cache_mem *init_cache_mem(unsigned long c)
{/*{{{*/
  struct cache_mem *cm = NULL;

  if (!(cm = malloc(sizeof(struct cache_mem))))
    return NULL;

  init_list(&cm->mrug.head);
  init_list(&cm->mru.head);
  init_list(&cm->mfu.head);
  init_list(&cm->mfug.head);

  cm->c = c;
  cm->p = c >> 1;

  /* Init */
  cm->size = 0;
  cm->max = c;
  cm->read = 0;
  cm->write = 0;
  cm->hit = 0;

  init_hash_list(cm, c);

  return cm;
}/*}}}*/

/**
 * Report result. 
 * @param cm : cache memory struct
 */
void report_cm(struct cache_mem *cm)
{/*{{{*/
  printf("========== report ==========\n");
  printf("List (%10ld/%10ld)\n", cm->size, cm->max);
  printf("Read (%10ld/%10ld)\n", cm->hit, cm->read);
  printf("Write(%10ld/%10ld)\n", cm->write, cm->write);
  printf("========== report ==========\n");
}/*}}}*/

/**
 * Print cache memory.(cache list)
 * @param cm : cache memory struct
 * @return : error code
 */
int print_cm(struct cache_mem *cm)
{/*{{{*/
  int i = 0;

  struct list_head *tmp = NULL;
  struct cache_line *l = NULL;

  if (!cm)
    return -1;

  if (!DEBUG_OPTION)
    return -2;

  // tmp = cm->list;

  if (!tmp)
    return -3;

  // printf("root node : %p \n", tmp);
  // printf("root node : %p \n", tmp->next);

  /* Print list node */
  printf("==========\n");
  while (i < cm->size) {
    tmp = tmp->next;
    l = container_of(tmp, struct cache_line, head);

    printf("LIST : %10d %10ld \n", i, l->line);
    i++;
  }

  return 0;
}/*}}}*/

static int *get_hash(char *ret, double test)
{/*{{{*/
	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 1];
	int di;
  char str[33] = {'\0', };

  sprintf(str, "%f", test);

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)str, strlen(str));
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di)
		sprintf(hex_output + di * 2, "%02x", digest[di]);

  strcpy(ret, hex_output);
  return 0;
	// return hex_output;
}/*}}}*/

/*  Return the LRU element from the given state. */
struct cache_line *ARC_state_lru(struct cache_state *state)
{/*{{{*/
  struct list_head *head = state->head.prev;
  return list_entry(head, struct cache_line, head);
}/*}}}*/

int contain_list(struct cache_mem *cm, struct cache_line *l)
{/*{{{*/
  if (!cm || !l || !l->state)
    printf("new node.. \n");

  if (l->state == &cm->mrug) {
    printf("mrug %ld \n", l->state->size);
    return 1;
  } else if (l->state == &cm->mru) {
    printf("mru %ld \n", l->state->size);
    return 2;
  } else if (l->state == &cm->mfu) {
    printf("mfu %ld \n", l->state->size);
    return 3;
  } else if (l->state == &cm->mrug) {
    printf("mfug %ld \n", l->state->size);
    return 4;
  }
}/*}}}*/

struct cache_line *ARC_move(struct cache_mem *cm, struct cache_line *l, struct cache_state *state) 
{/*{{{*/

  printf("this %p %p %p %ld ", &l->head, &l->hash, l, l->line);
  contain_list(cm, l);

  //이미 있는거 제거..//
  if (l->state) {
    l->state->size -= 1;

    list_remove(&l->head);
  }

  // 그냥 제거..//
  if (state == NULL) {
    /* destroy */
    printf("del %ld ", l->line);
    contain_list(cm, l);

    l->line = 0;
    l->state = NULL;
    list_remove(&l->hash);

    free(l);
    
    return NULL;
  } else {

    if (state == &cm->mrug || state == &cm->mfug) {

    } else if (l->state != &cm->mru && l->state != &cm->mfu) {
      printf("bal %p %p %p %ld ", &l->head, &l->hash, l, l->line);
      contain_list(cm, l);
      ARC_balance(cm, 1);
    }

    list_prepend(&l->head, &state->head);
    l->state = state;
    l->state->size += 1;

    printf("move %p %p %p %ld ", &l->head, &l->hash, l, l->line);
    contain_list(cm, l);

    printf("move %p ok. head : %p / size : %lu / val : %ld \n", l, &state->head, l->state->size, l->line);

  }
  return l;
}/*}}}*/

/*  Balance the lists so that we can fit an object with the given size into
 * the cache. */
static void ARC_balance(struct cache_mem *cm, unsigned long size)
{/*{{{*/

  struct cache_line *l = NULL;

  /*  First move objects from MRU/MFU to their respective ghost lists. */
  while (cm->mru.size + cm->mfu.size + size > cm->c) {
    /* printf("bal : goto G . mur.size = %ld, p = %ld \n", cm->mru.size, cm->p); */
    if (cm->mru.size > cm->p) {
      l = ARC_state_lru(&cm->mru);
      ARC_move(cm, l, &cm->mrug);
    } else if (cm->mfu.size > 0) {
      l = ARC_state_lru(&cm->mfu);
      ARC_move(cm, l, &cm->mfug);
    } else {
      break;
    }
  }

  /*  Then start removing objects from the ghost lists. */
  while (cm->mrug.size + cm->mfug.size> cm->c) {

    /* printf("bal : goto NULL \n"); */
    if (cm->mfug.size > cm->p) {
      l = ARC_state_lru(&cm->mfug);
      ARC_move(cm, l, NULL);
    } else if (cm->mrug.size > 0) {
      l = ARC_state_lru(&cm->mrug);
      ARC_move(cm, l, NULL);
    } else {
      break;
    }
  }

  /* printf("bal : end \n"); */
}/*}}}*/

static inline struct cache_line *ARC_print(struct list_head *start)
{/*{{{*/
  struct list_head *tmp = NULL;
  struct cache_line *l = NULL;
  int i = 0;

  if (!start)
    return NULL;

  // Get hash.. //
  list_each(tmp, start) {
    l = container_of(tmp, struct cache_line, head);
    //print index, &list_head, &list, line..//
    printf("%10d %p %p %ld\n", i++, tmp, l, l->line);
  }

  return NULL;
}/*}}}*/

/**
 * Del cache memory.(cache list)
 * @param cm : cache memory struct
 * @return : error code
 */
int del_cm(struct cache_mem *cm)
{/*{{{*/
  int i = 0;

  struct list_head *tmp = NULL;
  struct cache_line *l = NULL;

  if (!cm)
    return -1;

  // tmp = cm->list;
  if (!tmp)
    return -3;

  /* Del list node */
  while (i < cm->size) {
    tmp = tmp->next;
    l = container_of(tmp, struct cache_line, head);

    // del
    list_del(tmp->prev);
    free(l);
    i++;
  }

  free(cm);
  return 0;
}/*}}}*/

static inline struct cache_line *ARC_lookup(struct cache_mem *cm, long line)
{/*{{{*/
  struct list_head *tmp = NULL;
  unsigned long hash_num = NULL;
  char hash_str[33] = {'\0', };

  if (!cm || line < 0)
    return NULL;

  get_hash(hash_str, line);

  // Get hash.. //
  hash_num = atonum(hash_str) % (cm->hash.size);
  list_each(tmp, &cm->hash.bucket[hash_num]) {
    struct cache_line *l = list_entry(tmp, struct cache_line, hash);
    /* if (strncmp(l->md5, hash_str, 33) == 0) { */
    if (line == l->line) {
      return l;
    } 
  }

  return NULL;
}/*}}}*/

static struct cache_line *create_line(double line)
{/*{{{*/
  struct cache_line *l = malloc(sizeof(struct cache_line));

  if (!l)
    return NULL;

  l->line = line;

  // Init hash..//
  get_hash(l->md5, line);
  /* printf("create : %s\n", l->md5); */

  // Init list..//
  init_list(&l->head);
  init_list(&l->hash);

  return l;
}/*}}}*/

void hash_insert(struct cache_mem *cm, struct cache_line *l)
{/*{{{*/
  unsigned long hash = atonum(l->md5) % (cm->c);
  list_prepend(&l->hash, &cm->hash.bucket[hash]);
}/*}}}*/

struct cache_line *ARC_cache(struct cache_mem *cm, long line)
{/*{{{*/
  /* TODO : .... */
  struct cache_line *lookup = NULL;
  struct cache_line *new = NULL;

  lookup = ARC_lookup(cm, line);

  if (lookup) {

    /* cm->hit++; */
    /* printf("hit %s\n", lookup->md5); */
    if (lookup->state == &cm->mru || lookup->state == &cm->mfu) {
      printf("== 01 %ld %ld %ld %ld\n", cm->mrug.size, cm->mru.size, cm->mfu.size, cm->mfug.size);
      ARC_move(cm, lookup, &cm->mfu);
      return lookup;
    } else if (lookup->state == &cm->mrug) {
      printf("== 02 %ld %ld %ld %ld\n", cm->mrug.size, cm->mru.size, cm->mfu.size, cm->mfug.size);
      contain_list(cm, lookup);

      printf("chp1 : %ld \n", cm->p);
      cm->p = MIN(cm->c, cm->p + MAX(cm->mfug.size / cm->mrug.size, 1));
      printf("chp2 : %ld \n", cm->p);
      ARC_move(cm, lookup, &cm->mfu);
      return lookup;
    } else if (lookup->state == &cm->mfug) {
      printf("== 03 %ld %ld %ld %ld\n", cm->mrug.size, cm->mru.size, cm->mfu.size, cm->mfug.size);

      printf("chp1 : %ld, test : %ld\n", cm->p, cm->p - MAX(cm->mrug.size / cm->mfug.size, 1));
      cm->p = MAX(0, cm->p - MAX(cm->mrug.size / cm->mfug.size, 1));
      printf("chp2 : %ld \n", cm->p);
      ARC_move(cm, lookup, &cm->mfu);
      return lookup;
    } else {
      /* ... */
      return NULL;
    }
  } else {

    /* Case4 : New line */
    printf("== 04 %ld %ld %ld %ld\n", cm->mrug.size, cm->mru.size, cm->mfu.size, cm->mfug.size);
    new = create_line(line);
    if (!new)
      return NULL;

    hash_insert(cm, new);
    ARC_move(cm, new, &cm->mru);
    return NULL;
  }

  return NULL;;
}/*}}}*/

/**
 * run cache.
 * @param cm : cache memory info strcut
 * @param wl : target workload struct
 * @return : error code
 */
int run_cache(struct cache_mem *cm, struct workload *wl)
{/*{{{*/
  int i = 0;
  struct cache_line *ret = NULL;
  long start = 0;
  long end = 0;

  /* NULL arg */
  if (!cm || !wl) {
    printf("[FAIL] arg NULL, %s \n", __func__);
    return -1;
  }

  /* start sector number and size(end) */
  // printf("1 s : %.2f , c : %.2f \n", wl->offset, wl->size);
  // printf("2 s : %.2f , c : %.2f \n", wl->offset/4096, wl->size/4096);
  start = (wl->offset / CACHE_BLOCK_SIZE);
  end = ((wl->offset + wl->size) / CACHE_BLOCK_SIZE);
  // printf("3 s : %14.2f, c : %14.2f \n", start, end);

  /* print */
  // printf("%d %20s %15ld %10.2f %10ld %5.2f\n", wl->type, wl->time, wl->offset, start, wl->size, end);

   do {
     // printf("%d >> %f\n", wl->type, start + i);

     /* ret is errno or hit */
     ret = ARC_cache(cm, start + i);

     /* printf("===== MRUG =====\n"); */
     /* ARC_print(&(cm->mrug.head)); */
     /* printf("===== MRU =====\n"); */
     /* ARC_print(&(cm->mru.head)); */
     /* printf("===== MFU =====\n"); */
     /* ARC_print(&(cm->mfu.head)); */
     /* printf("===== MFUG =====\n"); */
     /* ARC_print(&(cm->mfug.head)); */
     /* printf("===========\n"); */

     if (wl->type == READ) {
       cm->read++;
       if (ret) {
         cm->hit++;
         printf(">> hit\n");
       }
     } else if (wl->type == WRITE) {
       cm->write++;
     }

     i++;
   } while (start + i <= end);

   return 0;
}/*}}}*/

/**
 * Just open file and return.
 * @param file : target file path
 * @return : file pointer
 */
FILE *open_workload(char *file)
{/*{{{*/
  FILE *fp = NULL;

  /* NULL arg */
  if (!file) {
    printf("[FAIL] arg NULL, %s \n", __func__);
    return NULL;
  }

  /* open FILE */
  if ((fp = fopen(file, "r")) < 0)
    return NULL;

  return fp;
}/*}}}*/

/**
 * read colum. (=Inscribe workload struct)
 * @param wl : workload struct
 * @param buf : buffer string
 * @return : error code
 */
int read_column(struct workload *wl, char *buf)
{/*{{{*/
  int column = 1;
  char *tmp = NULL;

  /* NULL arg */
  if (!wl || !buf) {
    printf("[FAIL] arg NULL, %s \n", __func__);
    return -1;
  }
  
  /* read file and, save workload struct */
  tmp = strtok(buf, ",");
  while (tmp != NULL) {

    switch (column) {
      /* 
       * TODO : strdup is cause of memory leak
       * But, This case is needless..
       */

      // case 1 : wl->time = strdup(tmp); break;
      // case 2 : wl->host = strdup(tmp); break;
      // case 3 : wl->disk_num = atoi(tmp); break;
      case 4 : (wl->type) = (strcmp(tmp, "Read") == 0) ? READ : WRITE; break;
      case 5 : wl->offset = atoll(tmp); break;
      case 6 : wl->size = atol(tmp); break;
      // case 7 : wl->respone = atol(tmp); break;
    }

    /* Next */
    tmp = strtok(NULL, ",");
    column++;
  }

  free(tmp);
  return 0;
}/*}}}*/

/**
 * cache simulator main. read worklosd and analysis..
 * @param fp : file pointer
 * @return : error code
 */
int read_workload(FILE *fp, long cache_size)
{/*{{{*/
  int ret = 0;
  char buf[100];
  struct cache_mem *cm = NULL;
  struct workload *wl = NULL;

  long tmp = -2;
  printf("0, tmp MAX => %ld \n", MAX(0, tmp));

  /* NULL arg test */
  if (!fp)
    printf("arg is NULL\n");

  wl = malloc(sizeof(struct workload));
  cm = init_cache_mem(cache_size / CACHE_BLOCK_SIZE);

  printf("%lu\n", cm->c);
  printf("%lu\n", cm->p);

  if (!wl || !cm)
    goto end;

  /* read line by line */
  while (!feof(fp)){

    /* read line by line. saved buf */
    if (fscanf(fp, "%s", buf) < 0)
      goto end;

    /* read buf and Inscribe wl */
    if (read_column(wl, buf) < 0)
      goto end;

    /* run cache mem  */
    run_cache(cm, wl);
  }

end:
  /* reprot */
  report_cm(cm);

  printf("===== Info =====\n");
  printf("mrug %ld \n", cm->mrug.size);
  printf("mru %ld \n", cm->mru.size);
  printf("mfu %ld \n", cm->mfu.size);
  printf("mfug %ld \n", cm->mfug.size);

  printf("===== Info =====\n");

  printf("read : %ld\n", cm->read);
  printf("write : %ld\n", cm->write);
  printf("HIT : %ld\n", cm->hit);

  printf("===== MRU =====\n");
  /* ARC_print(&(cm->mru.head)); */
  printf("===== MFU =====\n");
  /* ARC_print(&(cm->mfu.head)); */
  printf("===========\n");

  /* DEBUG.. PRINT LIST */
  if (0) 
    ret = print_cm(cm);

  if (ret < 0)
    printf("Err\n");

  del_cm(cm);
  printf("END\n");

  return 0;
}/*}}}*/


