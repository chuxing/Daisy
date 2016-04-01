#ifndef _P_MMAP_H
#define _P_MMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <linux/unistd.h>

#define __NR_p_mmap 322
#define __NR_p_search_big_region_node 323
#define __NR_p_alloc_and_insert 324
#define __NR_p_get_small_region 325
#define __NR_p_bind             326
#define __NR_p_search_small_region_node 327

#define PCM_MAGIC 0x5A5AA5A5

typedef struct tagBuddy Buddy;

/*
* initialize the SCM memory system
* return 0 if succeed, otherwise -1
*/ 
int p_init(int size);

/*
* clear the metadata of small memory region
* return 0 if succeed, otherwise -1
*/
int p_clear();

/*
* allocate a small chunk of memory based on size
* return valid address if succeed, otherwise NULL
*/
void *p_malloc(int size);

/*
* deallocate the memory to allocator
* return 0 if succeed, otherwise -1
*/
int p_free(void *addr);

/*
* allocate a large chunk of memory based on size
* return valid address if succeed, otherwise NULL
*/
void *p_new(int pId, int size);

/*
* deallocate the memory based on pId to allocator
* return 0 if succeed, otherwise -1
*/
int p_delete(int pId);

/*
* get the memory address base on pId and size
* return valid address if succeed, otherwise NULL
*/
void *p_get(int pId, int size);

/*
* bind address ptr to id
* return 0 if succeed, otherwise -1
*/
int p_bind(int id, void *ptr, int size);

/*
* get the bind node based on pId, push size into *psize
* return valid address if succeed, otherwise NULL
*/
void *p_get_bind_node(int pId, int *psize);

/*
* get the memory base address
* return valid address if succeed, otherwise NULL
*/
void *p_get_base();

/*** helper functions ***/

void set_bit_to_one(int iStartBit, int iEnd);

#endif

