/*
 * =====================================================================================
 *
 *       Filename:  arc.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014년 12월 30일 13시 12분 59초
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

/* Get.. */
#include <memory.h>
#include <stddef.h>

/* Get... */
#define list_entry(ptr, type, field) \
  ((type*) (((char*)ptr) - offsetof(type,field)))

#define list_each(pos, head) \
  for (pos = (head)->next; pos != (head); pos = pos->next)


