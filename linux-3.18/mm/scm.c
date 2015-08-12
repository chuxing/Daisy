#include <linux/scm.h>
#include <linux/memblock.h>
#include <linux/slab.h>

static struct scm_head *scm_head;
static struct table_freelist *table_freelist;

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
static void scm_ptable_init()
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
	/* this SCM is new */
	if (scm_head->ptable_ptr == NULL && scm_head->hptable_ptr == NULL) {

		return;
	}
	/* ptable */
	if (scm_head->ptable_ptr) {
		struct ptable_node *root;

	}
	/* hptable */
}

void scm_test_code(void)
{
	char *test;
	daisy_printk("%s %s\n", __FILE__, __func__);
	test = kmalloc(10, GFP_KERNEL);
}
