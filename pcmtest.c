/*
 ============================================================================
 Name        : pcmtest.c
 Author      : Menooker
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#define __NR_p_mmap 322
#define __NR_p_alloc_and_insert 324
/*
 SYSCALL_DEFINE4(p_mmap, unsigned long, addr, unsigned long, len,
		unsigned long, prot,	unsigned long, id)
 */

unsigned long p_alloc_and_insert(unsigned long id,int size)
{
	return (unsigned long)syscall(__NR_p_alloc_and_insert, id,size);
}

void* p_mmap(void* addr,unsigned long len,unsigned long prot,unsigned long id)
{
	return (void*)syscall(__NR_p_mmap, addr, len,prot,id);
}

int main(void) {
	printf("alloc : ret= %lx\n",p_alloc_and_insert(2,4096));
	long * addr=(long*)p_mmap(NULL,4096,PROT_READ | PROT_WRITE, 2);
	printf("mmap : %lx %lx\n",addr,*addr);
	*addr=0xcafebabe;
	return EXIT_SUCCESS;
}
