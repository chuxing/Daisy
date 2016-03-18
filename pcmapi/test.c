#include "test.h"
#include "p_mmap.h"
#include "stdlib.h"

typedef struct
{
	int data;
	int next;
} LinkedNode;

void print_mem(void* ptr)
{
	int i = 0, j = 0;
	int *p = (int*)ptr;

	for(i=0; i<5; i++) {
		for(j=0; j<4; j++) {
			printf("%x ",*p);
			p++;
		}
		printf("\n");
	}
}

int main(int argc, char **argv) {
    int iRet = 0;
    char *ptr = NULL;

    if (argc < 2) {
        printf("times of pnew!\n");
        return 0;
    }

    int times = atoi(argv[1]);

    int i=0;
    for (; i<times; i++) {
        ptr = p_new(i, 4096);
        if (ptr == NULL) {
            printf("error p_new\n");
        }
    }
    
#if 0
    printf("ready to call p_init\n");
    iRet = p_init(4096 * 16);
    if (iRet < 0) {
        printf("error: p_init\n");
        return -1;
    }

    printf("%d %s\n", argc,argv[1]);
    if (argc == 2 && argv[1][0] == 'c') {
        printf("ready to clear\n");
        iRet = p_clear();
        if (iRet < 0) {
            printf("error: p_clear\n");
            return -1;
        }

    } else if (argc == 3 && argv[1][0] == 'w') { //write the linked list
    	int i;
    	int t;
    	char *base = (char*)p_get_base();
    	LinkedNode* nd,*last,*head;
    	t = atoi(argv[2]);
    	last = (LinkedNode*)p_malloc(sizeof(LinkedNode));
    	printf("first node=%llx base=%llx\n", last, base);
		last->data = -1;
		head = last;
		printf("bind %d\n",p_bind(1234, last, sizeof(LinkedNode)));
		printf("head->data %d\n", head->data);
    	printf("============================\nhead mem\n");
    	print_mem(head);
    	printf("----------------------------\n");

    	for(i=0; i<t; i++) {
        	nd = (LinkedNode*)p_malloc(sizeof(LinkedNode));
        	printf("new node=%llx i=%d\n", nd, i);
    		nd->data = i;
    		last->next = (char*)nd-base;
    		last = nd;
    	}
    	last->next = 0;
    	printf("============================\nbase mem\n");
    	print_mem(base);
    	printf("----------------------------\n");

    	printf("============================\nhead mem\n");
    	print_mem(head);
    	printf("----------------------------\n");

    } else if (argc == 2 && argv[1][0] == 'r') { //read and check the linked list
    	char *base=(char*)p_get_base();
    	int sz,i;
    	LinkedNode* nd = p_get_bind_node(1234,&sz);

    	printf("============================\nbase mem\n");
    	print_mem(base);
    	printf("----------------------------\n");

		printf("head->data %d\n",nd);
    	printf("============================\nhead mem\n");
    	print_mem(nd);
    	printf("----------------------------\n");

    	printf("First Node ptr=%llx sz=%d\n",nd,sz);
    	i = -1;
    	for(;;) {
        	if(nd->data != i) {
        		printf("Check Error! data=%d i=%d\n",nd->data,i);
        		//break;
        	}
        	i++;
    		if (nd->next)
    			nd=(LinkedNode*)(base+nd->next);
    		else
    			break;
    	}
    	printf("Check finish! i=%d\n",i);
    } else {
    	printf("No op\n");
    }

#endif
    
    return 0;
}
