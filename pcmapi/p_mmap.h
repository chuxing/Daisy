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

#define BITMAPGRAN 8    /* n means 1 bit to n byte */

/*
* initialize the SCM memory system
* @param size the size of small region
* @return value 0 if succeed, otherwise -1
*/ 
int p_init(int size);

/*
* clear the metadata of small memory region
* @return value 0 if succeed, otherwise -1
*/
int p_clear();

/*
* allocate a small chunk of memory based on size
* @param size the size of memory region
* @return value valid address if succeed, otherwise NULL
*/
void *p_malloc(int size);

/*
* deallocate the memory to allocator
* @param addr the address of memory to be freed
* @return value 0 if succeed, otherwise -1
*/
int p_free(void *addr);

/*
* allocate a large chunk of memory based on size
* @param pId the id of memory region to be allocated
* @param size the size of memory region
* @return value valid address if succeed, otherwise NULL
*/
void *p_new(int pId, int size);

/*
* deallocate the memory based on pId to allocator
* @param pId the id of memory region to be delete
* @return value 0 if succeed, otherwise -1
*/
int p_delete(int pId);

/*
* get the memory address base on pId and size
* @param pId the id of memory region to be retrieved
* @param size the size of memory
* @return value valid address if succeed, otherwise NULL
*/
void *p_get(int pId, int size);

/*
* bind address to id
* @param id the id to bind
* @param ptr the address to be bind
* @param size the size of memory region
* @return value 0 if succeed, otherwise -1
*/
int p_bind(int id, void *ptr, int size);

/*
* get the bind node based on pId, 
* @param pId the id of memory region to be retrieved
* @param psize push size into *psize
* @return value valid address if succeed, otherwise NULL
*/
void *p_get_bind_node(int pId, int *psize);

/*
* get the memory base address
* @return value valid address if succeed, otherwise NULL
*/
void *p_get_base();

/*
* helper functions 
*/
static void set_bit_to_one(int iStartBit, int iEnd);

#endif

