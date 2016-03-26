#include "test.h"
#include "p_mmap.h"
#include "stdlib.h"

/**
* 这段程序的作用是在PCM中新建一个链表，绑定一个用户指定的ID，并支持在下一次启动时，根据ID将该链表重新构建出来。
* 示例:
* ./a.out c (reset the memory allocator)
* ./a.out w 10 (write the linked list)
* ./a.out r (read and check the linked list)
*/

typedef struct {
	int data;
	int next;
} LinkedNode;

int main(int argc, char **argv) {
    int iRet = 0;
    char *ptr = NULL;

    iRet = p_init(4096 * 16);
    if (iRet < 0) {
        printf("error: p_init\n");
        return -1;
    }

    if (argc == 2 && argv[1][0] == 'c') { /* reset the memory allocator */
        iRet = p_clear();
        if (iRet < 0) {
            printf("error: p_clear\n");
            return -1;
        }
    } else if (argc == 3 && argv[1][0] == 'w') { /* write the linked list */
    	int i;
    	int t;
    	char *base = (char*)p_get_base();
    	LinkedNode *nd, *last, *head;

    	t = atoi(argv[2]);
    	last = (LinkedNode*)p_malloc(sizeof(LinkedNode));
		last->data = -1;
		head = last;

    	for(i=0; i<t; i++) {
        	nd = (LinkedNode*)p_malloc(sizeof(LinkedNode));
    		nd->data = i;
    		last->next = (char*)nd-base;
    		last = nd;
    	}

    	last->next = 0;
    } else if (argc == 2 && argv[1][0] == 'r') { /* read and check the linked list */
    	char *base = (char*)p_get_base();
    	int sz,i;
    	LinkedNode* nd = p_get_bind_node(1234, &sz);

    	i = -1;
    	while (1) {
            /* check if the value is consistent */
        	if(nd->data != i) {
        		printf("Check Error! data=%d i=%d\n",nd->data,i);
        	}
        	i++;
    		if (nd->next)
    			nd = (LinkedNode*)(base + nd->next);
    		else
    			break;
    	}

    	printf("Check finish! i=%d\n",i);
    } else {
    	printf("No op\n");
    }

    return 0;
}
