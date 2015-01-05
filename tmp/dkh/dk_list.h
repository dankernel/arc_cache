/**
 * =====================================================================================
 *
 *         @file:  lru.h
 *        @brief:  LRU에 대한 라이브러리
 *  
 *      Version:  1.0
 *        @date:  2014년 11월 23일 19시 16분 39초
 *     Revision:  none
 *     Compiler:  gcc
 *  
 *      @author:  Park jun hyung (), google@dankook.ac.kr
 *     @COMPANY:  Dankopok univ.
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "errno.h"

#define dk_list

#define offsetof(type, member) ((size_t) &((type *)0)->member)

#define container_of(ptr, type, member) ({\
    const typeof( ((type *)0)->member ) *__mptr = (ptr);\
    (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_for_each(pos, head) \
  for (pos = (head)->next; prefetch(pos->next), pos != (head); \
      pos = pos->next)

struct list_head
{/*{{{*/
  struct list_head *next;
  struct list_head *prev;
};/*}}}*/

static inline void __list_del(struct list_head *prev, struct list_head *next)
{/*{{{*/
  next->prev = prev;
  prev->next = next;
}/*}}}*/

/**
 * list_del - deletes entry from list.
 * @param entry: the element to delete from the list.
 * @param Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void list_del(struct list_head *entry)
{/*{{{*/
  __list_del(entry->prev, entry->next);
  entry->next = NULL;
  entry->prev = NULL;
}/*}}}*/

static inline void init_list(struct list_head *head)
{
  head->next = head->prev = head;
}

/**
 * make new node
 * @return : new node
 */
struct list_head *init_lnode()
{/*{{{*/
  /* allocation new node */
  struct list_head *nn = NULL;
  nn = malloc(sizeof(struct list_head));
  if (!nn)
    return NULL;

  /* init */
  nn->next = nn;
  nn->prev = nn;

  return nn;
}/*}}}*/

/**
 * Insert a new entry between two known consecutive entries.
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *new,
    struct list_head *prev,
    struct list_head *next)
{/*{{{*/
  next->prev = new;
  new->next = next;
  new->prev = prev;
  prev->next = new;
}/*}}}*/

/**
 * list_replace - replace old entry by new one
 * If @old was empty, it will be overwritten.
 * @param old : the element to be replaced
 * @param new : the new element to insert
 */
static inline void list_replace(struct list_head *old, struct list_head *new)
{/*{{{*/
  new->next = old->next;
  new->next->prev = new;
  new->prev = old->prev;
  new->prev->next = new;
}/*}}}*/

/**
 * list_add - add a new entry
    Insert a new entry after the specified head.
    This is good for implementing stacks.
 * @param new: new entry to be added
 * @param head: list head to add it after
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{/*{{{*/
  __list_add(new, head, head->next);
}/*}}}*/

/** 
 * list_move - delete from one list and add as another's head
 * @param list: the entry to move
 * @param head: the head that will precede our entry
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{/*{{{*/
  __list_del(list->prev, list->next);
  list_add(list, head);
}/*}}}*/

static inline void list_splice(struct list_head *prev, struct list_head *next)
{
  next->prev = prev;
  prev->next = next;
}


static inline void list_remove(struct list_head *head)
{
  list_splice(head->prev, head->next);
  head->next = head->prev = NULL;
}

static inline void list_insert(struct list_head *list, struct list_head *prev, struct list_head *next)
{
  next->prev = list;
  list->next = next;
  list->prev = prev;
  prev->next = list;
}

static inline void list_prepend(struct list_head *head, struct list_head *list)
{
  list_insert(head, list, list->next);
}
