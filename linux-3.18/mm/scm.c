#include <linux/scm.h>
#include <linux/memblock.h>
#include <linux/slab.h>

static void* reserve_scm_ptable_memory(void)
{
	size_t size;
	phys_addr_t phys;
	phys_addr_t scm_start_phys;
	struct scm_head *scm_head;

	/* get the first 1024 scm pages */
	size = 1024 * PAGE_SIZE;
	scm_start_phys = PFN_PHYS(max_pfn_mapped)-(SCM_PFN_NUM<<PAGE_SHIFT);
	phys = memblock_find_in_range(scm_start_phys, scm_start_phys + size, size, PAGE_SIZE);
	if (!phys) {
		daisy_printk("Error when getting persist table memory\n");
		return -ENOMEM;
	}
	memblock_reserve(phys, size);
	scm_head = (struct scm_head*)__va(phys);

	daisy_printk("scm_start_phys, phys: %lu %lu\n", scm_start_phys, phys);
	daisy_printk("Get pfn: %lu， max_pfn: %lu\n", phys >> PAGE_SHIFT, max_pfn_mapped);
	/* record the size */
	scm_head->total_size = size;
	return scm_head;
}

static void scm_ptable_init(struct scm_head *scm_head)
{
	scm_head->magic = SCM_MAGIC;
	scm_head->ptable_ptr = NULL;
	scm_head->hptable_ptr = NULL;


}

/**
 * scm persist table init
 * reference to: numa_alloc_distance & numa_reset_distance
 */
int scm_ptable_boot(void)
{
	struct scm_head *scm_head;

	scm_head = reserve_scm_ptable_memory();
	/**
	 * unsigned long *hacklen = ((char *)scm_head+sizeof(unsigned long)+2*sizeof(void*));
	 * *hacklen = 735;
	 *
	 * to test: make fake data here
	 * */

	/**
	 * check magic number to decide how to init
	 * 1. clear scm
	 * 2. traverse tree
	 * */
	daisy_printk("scm_head: %lu %lu %lu %lu\n",
			scm_head->magic,
			scm_head->ptable_ptr,
			scm_head->hptable_ptr,
			scm_head->len);
	if (scm_head->magic != SCM_MAGIC) {
		/* this is a total clear SCM */

	} else {
		/* SCM with data! */
	}
	return 0;
}

void scm_test_code(void) {
	char *test;
	daisy_printk("%s %s\n", __FILE__, __func__);
	test = kmalloc(10, GFP_KERNEL);
}
