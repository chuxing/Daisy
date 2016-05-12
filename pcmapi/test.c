#include "test.h"
#include "p_mmap.h"
#include "stdlib.h"

/**
* 这段程序的作用是在PCM中新建一个链表，绑定一个用户指定的ID，并支持在下一次启动时，根据ID将该链表重新构建出来。
* 示例:
* ./a.out c (reset the memory allocator)
* ./a.out w 10 (write the linked list)
* ./a.out r (read and check the linked list)
* ./a.out n (alloc and write a large chunk of memory on scm)
* ./a.out m (re-map and read a large chunk of memory on scm)
*/

typedef struct {
	int data;
	int next;
} LinkedNode;

int main(int argc, char **argv) {
    int iRet = 0;
    char *ptr = NULL;
    printf("HEAPO test\n");
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
    	p_bind(1234,head,sizeof(LinkedNode));
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
    }else if (argc == 2 && argv[1][0] == 'n') {
    	int j;
    	for(j=0;j<100;j++)
    	{
			int* buf=p_new(10000+j,4096);
			int i;
			for(i=0;i<4096/sizeof(int);i++)
			{
				buf[i]=i;
			}
    	}
    }else if (argc == 2 && argv[1][0] == 'm') {
    	int j;
    	for(j=0;j<100;j++)
    	{
			int* buf=p_get(10000+j,4096);
			int i;
			for(i=0;i<4096/sizeof(int);i++)
			{
				if(buf[i]!=i)
				{
					printf("Check Error! data=%d i=%d\n",buf[i],i);
					break;
				}
			}
    	}
    	printf("Check finish!\n");
    }
    else if (argc == 2 && argv[1][0] == 'd') {
    	int j;
    	for(j=0;j<100;j++)
    	{
        	printf("p_delete returns %d\n",p_delete(10000+j));
    	}
    }
    else {

    	printf("No op\n");
    }

    return 0;
}
