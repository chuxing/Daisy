#include <linux/scm.h>
#include <linux/memblock.h>
#include <linux/slab.h>

static void* ptable;

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
	phys_addr_t scm_start_phys = PFN_PHYS(max_pfn_mapped)-(SCM_PFN_NUM<<PAGE_SHIFT);
	phys = memblock_find_in_range(scm_start_phys, scm_start_phys + size, size, PAGE_SIZE);
	if (!phys) {
		daisy_printk("Error when getting persist table memory\n");
		return -ENOMEM;
	}
	memblock_reserve(phys, size);
	ptable = __va(phys);

	daisy_printk("scm_start_phys, phys: %lu %lu\n", scm_start_phys, phys);
	daisy_printk("Get pt: %p\n", ptable);
	daisy_printk("Get pfn: %lu， max_pfn: %lu\n", (unsigned long)(phys >> PAGE_SHIFT), max_pfn_mapped);

	return 0;
}

void scm_test_code(void) {
	char *test;
	daisy_printk("%s %s\n", __FILE__, __func__);
	test = kmalloc(10, GFP_KERNEL);
}
