#ifndef _SCM_H
#define _SCM_H

#include <linux/rbtree.h>

/* 128M pfns 128*1024/4*/
#define SCM_PFN_NUM 32768UL

/**
 * persist table
 * 0. big memory region
 * 1. small memory region
 * */
struct ptable_head_node {

};

struct ptable_node {
	u64 ptable_id;
	u64 phys_addr;
	union {
		u64 size;
		u64 offset;
	};
	struct rb_node	ptable_rb;
	/* 0 or 1 */
	unsigned long flags;
};

/* linux/mm/scm.c */
int scm_ptable_init(void);

void scm_test_code(void);

#endif /* _SCM_H */
