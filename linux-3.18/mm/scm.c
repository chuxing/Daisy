#include <linux/scm.h>
#include <linux/memblock.h>

static void* ptable;

void scm_print_test(void)
{
	daisy_printk("scm.c test\n");
}

/**
 * scm persist table init
 * get 1024 scm pages
 *
 * reference to: numa_alloc_distance & numa_reset_distance
 */
int scm_ptable_init(void)
{
	size_t size = 1024 * PAGE_SIZE;
	phys_addr_t phys;
	phys_addr_t start = PFN_PHYS(max_pfn_mapped)-(SCM_PFN_NUM<<PAGE_SHIFT);
	/* must from start to start+size */
	phys = memblock_find_in_range(start, start + size, size, PAGE_SIZE);
	if (!phys) {
		daisy_printk("memblock_find_in_range error\n");
		return -ENOMEM;
	}
	memblock_reserve(phys, size);
	ptable = __va(phys);
	daisy_printk("Get pt: %p\n", ptable);
	daisy_printk("Get pfn: %lu， max_pfn: %lu\n", (unsigned long)(phys >> PAGE_SHIFT), max_pfn_mapped);
	return 0;
}
