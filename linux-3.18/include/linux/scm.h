#ifndef _SCM_H
#define _SCM_H

/* 128M pfns */
#define SCM_PFN_NUM (unsigned long)(128*1024/4)

/* linux/mm/scm.c */
void scm_print_test(void);
int scm_ptable_init(void);

#endif /* _SCM_H */
