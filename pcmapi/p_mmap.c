#include "p_mmap.h"

static char *pAddr = NULL;
static const int iBitsCount = (SHM_SIZE) / 9 * 8;

static void* p_mmap(void* addr,unsigned long len,unsigned long prot,unsigned long id) {
	return (void*)syscall(__NR_p_mmap, addr, len, prot, id);
}

static int p_search_big_region_node(unsigned long id) {
	return (int)syscall(__NR_p_search_big_region_node, id);
}

static int p_alloc_and_insert(unsigned long id, int size) {
    return (int)syscall(__NR_p_alloc_and_insert, id, size);
}

int p_init() {
    key_t key;
    int shmid;
    int mode;
    int iRet = 0;

    /* make the key: */
    if ((key = ftok("/bin", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((shmid = shmget(key, SHM_SIZE, 0777 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    /* attach to the segment to get a pointer to it: */
    pAddr = shmat(shmid, (void *)0, 0);
    if (pAddr == (char *)(-1)) {
        perror("shmat");
        exit(1);
    }

    printf("get and attach succee! addr=%p\n", pAddr);

    return 0;
}

int p_clear() {
    if (pAddr == NULL) {
        printf("error: call p_init first\n");
        return -1;
    }

    memset(pAddr, 0, SHM_SIZE/9);

    return 0;
}

void* p_malloc(int size) {
    if (size < 0) {
        printf("error: p_malloc, size must be greater than 0");
        return NULL;
    }

    if (pAddr == NULL) {
        printf("error: call p_init first\n");
        return NULL;
    }

    char curChar;
    unsigned char mask;
    
    enum {
        STOP = 0,
        LOOKING
    } state;

    state = STOP;
    int iStartBit = 0;

    int n;
    for (n=0; n<iBitsCount; n++) {
        mask = 1 << (7 - n%8);
        if (!(pAddr[n/8] & mask)) {
            // nth bit is empty

            switch (state) {
                case STOP:
                    iStartBit = n;
                    state = LOOKING;
                case LOOKING:
                    if (n - iStartBit + 1>= size) {
                        // we find it 
                        printf("we find it, ready to set bit\n"); 
                        set_bit_to_one(iStartBit, n);
                        return pAddr + SHM_SIZE/9 + iStartBit; 
                    }

                    break;
                default:
                    break;
            }
        } else {
            // nth bit is not empty
            switch (state) {
                case LOOKING:
                    state = STOP;
                    break;
                default:
                    break;
            }
        }
    }

    return NULL;
}

void set_bit_to_one(int iStartBit, int iEnd) {
    unsigned char mask;
    //printf("in set_bit_to_one, %d, %d", iStartBit, iEnd);
    int n;
    for (n=iStartBit; n<=iEnd; n++) {
        mask = 1 << (7 - n%8);
        pAddr[n/8] |= mask;

        //printf("mask=%d,after set: %d\n", mask,pAddr[n/8]&mask);
    }
}

int p_free(void *addr, int size) {
    if (!addr || size <= 0) {
        printf("invalid arguments\n");
        return -1;
    }

    if (addr < pAddr + SHM_SIZE/9 || addr + size > pAddr + SHM_SIZE - 1) {
        printf("addr out of range\n"); 
        return -1;
    }
    
    int nth = (char*)addr - pAddr - SHM_SIZE/9;
    unsigned char mask;
    int n;
    for (n=nth ; n<nth+size; n++) {
        mask = 1 << (7 - n%8);
        pAddr[n/8] &= ~mask;
    }

    return 0;
}

void *p_new(int pId, int iSize) {
    /*
    if (iSize < 4096) {
        return NULL;
    }
    */

    int iRet = 0;

    iRet = p_search_big_region_node(pId);
    printf("return from p_search_big_region_node: %d\n", iRet);
    if (iRet) {
        printf("id %d already exist\n", pId);
        //return NULL;
    }

    iRet = p_alloc_and_insert(pId, iSize);
    printf("return from p_alloc_and_insert: %d\n", (int)iRet);
    if (iRet != 0) {
        printf("error: p_alloc_and_insert\n");
        //return NULL;
    }

    void *pAddr = p_mmap(NULL, iSize, PROT_READ | PROT_WRITE, pId);
    if (!pAddr) {
        printf("p_mmap return NULL\n");
    } else {
        int *pInt = (int *)pAddr;
        printf("return address from p_mmap = %p\n", pAddr);
        printf("value = %d, ready to write %d\n", *pInt, *pInt+1);
        *pInt += 1;
    }

    return pAddr;
}

int p_delete(int pId) {
    /*
    p_unmap(pId);
    p_tab_delete(pId);
    */
   
    return 0;
}

void *p_get(int pId, int *pSize) {
    /*
    void *pAddr = sys_p_find_addr(pId, pSize);
    return pAddr;
    */
}
