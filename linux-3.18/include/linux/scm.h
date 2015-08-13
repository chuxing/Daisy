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
 * FLAG:
 * 0. big memory region
 * 1. small memory region
 * 2. heap region
 *
 * Requirement:
 * sizeof(hptable_node) === sizeof(ptable_node)
 * */
struct ptable_node {
	u64 _id;
	union {
		u64 phys_addr;
		u64 offset;
	};
	u64 size;
	unsigned long flags; /* 0 or 1 */
	u64 hptable_id;
	struct rb_node	ptable_rb;
};

struct hptable_node {
	u64 _id;
	u64 phys_addr;
	u64 size;
	unsigned long flags; /* 2 */
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

#endif /* _SCM_H */
