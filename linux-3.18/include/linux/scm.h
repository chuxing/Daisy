#ifndef _SCM_H
#define _SCM_H

/* 128M pfns 128*1024/4*/
#define SCM_PFN_NUM 32768UL

/* linux/mm/scm.c */
int scm_ptable_init(void);

void scm_test_code(void);

#endif /* _SCM_H */
