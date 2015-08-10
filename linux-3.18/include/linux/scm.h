#ifndef _SCM_H
#define _SCM_H

#include <linux/rbtree.h>

/* 128M pfns */
#define SCM_PFN_NUM (unsigned long)(128*1024/4)

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
void scm_print_test(void);
int scm_ptable_init(void);

#endif /* _SCM_H */
