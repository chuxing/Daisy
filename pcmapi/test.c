#include "test.h"
#include "p_mmap.h"

int main(int argc, char **argv) {
    printf("in the main\n");
    int iRet = 0;
    char *ptr = NULL;

    /*
    iRet = p_init();
    if (iRet < 0) {
        printf("error: p_init\n");
        return -1;
    }

    if (argc > 1) {
        iRet = p_clear();
        if (iRet < 0) {
            printf("error: p_clear\n");
            return -1;
        }
    }
    
    ptr = (char *)p_malloc(1);
    printf("return from p_malloc 1, addr=%p\n", ptr);
    */
    
    /*
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
    */
    
    ptr = p_new(1,1);
    printf("return from p_new, addr=%p\n", ptr);
    return 0;
}
