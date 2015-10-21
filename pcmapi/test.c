#include "test.h"
#include "p_mmap.h"

int main(int argc, char **argv) {
    
    int iRet = 0;
    char *ptr = NULL;

    printf("ready to call p_init\n");
    iRet = p_init();
    if (iRet < 0) {
        printf("error: p_init\n");
        return -1;
    }

    if (argc == 2 && argv[1][0] == 'c') {
        printf("ready to clear\n");
        iRet = p_clear();
        if (iRet < 0) {
            printf("error: p_clear\n");
            return -1;
        }

        return 0;
    }
    
    ptr = (char *)p_malloc(1);
    printf("return from p_malloc 1, addr=%p\n", ptr);
    
    p_free(ptr, 1);
    printf("after free %p\n", ptr);

    ptr = (char *)p_malloc(1);
    printf("return from p_malloc 1, addr=%p\n", ptr);

    ptr = (char *)p_malloc(1);
    printf("return from p_malloc 1, addr=%p\n", ptr);

    ptr = (char *)p_malloc(10);
    printf("return from p_malloc 10, addr=%p\n", ptr);

    ptr = (char *)p_malloc(100);
    printf("return from p_malloc 100, addr=%p\n", ptr);

    p_free(ptr, 100);
    printf("after free %p\n", ptr);

    ptr = (char *)p_malloc(100);
    printf("return from p_malloc 100, addr=%p\n", ptr);
    
    /*
    printf("ready to call p_get\n");
    ptr = p_get(23, 4096);
    if (!ptr) {
        printf("ready to call p_new\n");
        ptr = p_new(23, 4096);
        if (!ptr) {
            printf("p_new failed\n");
            return -1;
        }
    }

    
    int *tmp = (int *)ptr;
    printf("=== ptr = %p, value = %d, ready to add 1\n", ptr,*tmp);
    *tmp += 1;
    */
    return 0;
}
