/* TODO mutex & lock */

#ifndef _SCM_H
#define _SCM_H

#include <linux/rbtree.h>
#include <linux/list.h>

/**
 * magic number
 * ptable addr
 * heaptable addr
 * data
*/

/* 128M pfns 128*1024/4*/
#define SCM_PFN_NUM 32768UL
#define SCM_MAGIC 0x01234567

/* flags in struct node */
/* TODO use bit such as 0001 0002 0004 0008 0010 */
#define BIG_MEM_REGION 0
#define SMALL_MEM_REGION 1
#define HEAP_REGION 2

struct ptable_node;
struct hptable_node;

struct scm_head {
	unsigned long magic;
	struct rb_root ptable_rb;
	struct rb_root hptable_rb;
	unsigned long total_size; /* the total memory length (bytes) */
	unsigned long len; /* item number */
	char data[0];
};

/**
 * SCM persist table node
 *
 * FLAGS:
 * 0. big memory region
 * 1. small memory region
 * 2. heap region
 *
 * Requirement:
 * sizeof(hptable_node) === sizeof(ptable_node)
 *
 * Note:
 * _id of 1 & 2 must unique!
 * */
struct ptable_node {
	u64 _id;
	union {
		u64 phys_addr;
		u64 offset;
	};
	u64 size;
	unsigned long flags; /* BIG_MEM_REGION or SMALL_MEM_REGION */
	u64 hptable_id; /* 0 or real _id */
	struct rb_node	ptable_rb;
};

struct hptable_node {
	u64 _id;
	u64 phys_addr;
	u64 size;
	unsigned long flags; /* HEAP_REGION */
	u64 dummy;
	struct rb_node	hptable_rb;
};

struct table_freelist {
	void *node_addr;
	struct list_head list;
};

/* linux/mm/scm.c */
void scm_ptable_boot(void);
void scm_freelist_boot(void);

struct ptable_node *search_big_region_node(u64 _id);
struct ptable_node *search_small_region_node(u64 _id);
struct hptable_node *search_heap_region_node(u64 _id);
int insert_big_region_node(u64 _id, u64 phys_addr, u64 size);
int insert_small_region_node(u64 _id, u64 phys_addr, u64 size, u64 hptable_id);
int insert_heap_region_node(u64 _id, u64 phys_addr, u64 size);
int delete_big_region_node(u64 _id);
int delete_small_region_node(u64 _id);
int delete_heap_region_node(u64 _id);

void scm_full_test(void);

#endif /* _SCM_H */
