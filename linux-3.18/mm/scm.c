#include <linux/scm.h>
#include <linux/memblock.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/rbtree.h>

static struct scm_head *scm_head;
static struct table_freelist table_freelist = {
	.node_addr = NULL,
	.list = LIST_HEAD_INIT(table_freelist.list),
};

/* FOR DEBUG */
static void scm_print_freelist(void)
{
	struct table_freelist *tmp;
	int i=0;
	daisy_printk("freelist: ");
	list_for_each_entry(tmp, &table_freelist.list, list) {
		daisy_printk("%lu\t", tmp->node_addr);
		++i;
		if (i>=10) break;
	}
	daisy_printk("\n");
}

static void reserve_scm_ptable_memory(void)
{
	size_t size;
	phys_addr_t phys;
	phys_addr_t scm_start_phys;

	/* get the first 1024 scm pages */
	size = 1024 * PAGE_SIZE;
	/* pages in ZONE_SCM */
	scm_start_phys = PFN_PHYS(max_pfn_mapped)-(SCM_PFN_NUM<<PAGE_SHIFT);
	phys = memblock_find_in_range(scm_start_phys, scm_start_phys + size, size, PAGE_SIZE);
	if (!phys) {
		daisy_printk("Error when getting persist table memory!!!!\n");
		/* TODO BUG here */
		return;
	}
	memblock_reserve(phys, size);
	scm_head = (struct scm_head*)__va(phys);

	daisy_printk("scm_start_phys, phys: %lu %lu\n", scm_start_phys, phys);
	daisy_printk("Get pfn: %lu， max_pfn: %lu\n", phys >> PAGE_SHIFT, max_pfn_mapped);
	/* record the size */
	scm_head->total_size = size;
}

/* Init a total new SCM (no data) */
static void scm_ptable_init(void)
{
	scm_head->magic = SCM_MAGIC;
	scm_head->ptable_ptr = NULL;
	scm_head->hptable_ptr = NULL;
	/* calc scm_head->len */
	scm_head->len = (scm_head->total_size - sizeof(struct scm_head))/sizeof(struct ptable_node);
	/* do i need a whole memset (set to 0)? */
}

/**
 * scm persist table boot step
 * reference to: numa_alloc_distance & numa_reset_distance
 */
void scm_ptable_boot(void)
{
	reserve_scm_ptable_memory();
	/**
	 * unsigned long *hacklen = ((char *)scm_head+sizeof(unsigned long)+2*sizeof(void*));
	 * *hacklen = 735;
	 *
	 * to test: make fake data here
	 * */

	/**
	 * check magic number to decide how to init
	 * 1. clear scm, just set scm_head data
	 * 2. traverse tree to create freelist (do it later)
	 * */
	daisy_printk("scm_head: %lu %lu %lu %lu\n",
			scm_head->magic,
			scm_head->ptable_ptr,
			scm_head->hptable_ptr,
			scm_head->len);
	if (scm_head->magic != SCM_MAGIC) {
		/* this is a new SCM */
		scm_ptable_init();
	} else {
		/* SCM with data! */
		/* do_nothing this time */
	}
}

/* Just traverse the tree to init the freelist in DRAM */
void scm_freelist_init(void)
{
	struct table_freelist *tmp;
	unsigned long i;
	/* this SCM is new */
	if (scm_head->ptable_ptr == NULL && scm_head->hptable_ptr == NULL) {
		for (i=0; i<scm_head->len; ++i) {
			tmp= (struct table_freelist *)kmalloc(sizeof(struct table_freelist), GFP_KERNEL);
			tmp->node_addr = (char *)&scm_head->data + i*sizeof(struct ptable_node);
			list_add_tail(&tmp->list, &table_freelist.list);
		}
	} else {
	/* SCM is not new */
		/* ptable */
		if (scm_head->ptable_ptr) {
			struct ptable_node *root;

		}
		/* hptable */
	}
	scm_print_freelist();
}
