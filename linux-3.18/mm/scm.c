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

static void scm_fake_root(void)
{
}

void scm_full_test(void)
{

}
/* end FOR DEBUG */

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
		/* TODO BUG here must abort */
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
	scm_head->ptable_rb = RB_ROOT;
	scm_head->hptable_rb = RB_ROOT;
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
			scm_head->ptable_rb,
			scm_head->hptable_rb,
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
void scm_freelist_boot(void)
{
	struct table_freelist *tmp;
	unsigned long i;

	/* this SCM is new */
	if (RB_EMPTY_ROOT(&scm_head->ptable_rb) && RB_EMPTY_ROOT(&scm_head->hptable_rb)) {
		for (i=0; i<scm_head->len; ++i) {
			tmp= (struct table_freelist *)kmalloc(sizeof(struct table_freelist), GFP_KERNEL);
			tmp->node_addr = (char *)&scm_head->data + i*sizeof(struct ptable_node);
			list_add_tail(&tmp->list, &table_freelist.list);
		}
	} else {
	/**
	 * SCM is not new
	 * Reference: find_vma browse_rb...
	 * */
		unsigned long index;
		char usage_map[scm_head->len];
		for (i=0; i<scm_head->len; ++i) {
			usage_map[i] = 0;
		}
		/* ptable */
		if (!RB_EMPTY_ROOT(&scm_head->ptable_rb)) {
			struct rb_node *nd;
			for (nd = rb_first(&scm_head->ptable_rb); nd; nd = rb_next(nd)) {
				struct ptable_node *touch;
				touch = rb_entry(nd, struct ptable_node, ptable_rb);
				/* ignore small memory region */
				if (touch->flags == 0) {
					index = ((unsigned long)touch-(unsigned long)&scm_head->data)/sizeof(struct ptable_node);
					usage_map[index] = 1;
				}
			}
		}
		/* hptable */
		if (!RB_EMPTY_ROOT(&scm_head->hptable_rb)) {
			struct rb_node *nd;
			for (nd = rb_first(&scm_head->hptable_rb); nd; nd = rb_next(nd)) {
				struct hptable_node *touch;
				touch = rb_entry(nd, struct hptable_node, hptable_rb);
				index = ((unsigned long)touch-(unsigned long)&scm_head->data)/sizeof(struct hptable_node);
				usage_map[index] = 1;
			}
		}
		/* freelist */
		for (i=0; i<scm_head->len; ++i) {
			if (usage_map[i] == 0) {
				tmp= (struct table_freelist *)kmalloc(sizeof(struct table_freelist), GFP_KERNEL);
				tmp->node_addr = (char *)&scm_head->data + i*sizeof(struct ptable_node);
				list_add_tail(&tmp->list, &table_freelist.list);
			}
		}
	}
	/* TODO test SCM is not new */
	scm_print_freelist();
}

/* pop an item from freelist, free the item, return the addr (NULL if no more) */
static void *get_freenode_addr(void)
{
	void *ret;
	struct table_freelist *entry;
	if (list_empty(&table_freelist.list)) {
		return NULL;
	}
	entry = list_first_entry(&table_freelist.list, struct table_freelist, list);
	ret = entry->node_addr;
	kfree(entry);
	return ret;
}

/* return -1 if error & 0 if success */
static int insert_ptable_node_rb(u64 _id, u64 phys_addr, u64 size, u64 hptable_id, unsigned long flags)
{
	struct rb_node **n = &scm_head->ptable_rb.rb_node;
	struct rb_node *parent = NULL;
	struct ptable_node *new, *touch;
	new = (struct ptable_node *)get_freenode_addr();
	if (!new) {
		return -1;
	}
	new->_id = _id;
	new->phys_addr = phys_addr;
	new->size = size;
	new->flags = flags;
	new->hptable_id = hptable_id;

	/* insert to rbtree */
	while (*n) {
		parent = *n;
		touch = rb_entry(parent, struct ptable_node, ptable_rb);
		if (_id < touch->_id) {
			n = &(*n)->rb_left;
		} else if (_id > touch->_id) {
			n = &(*n)->rb_right;
		} else {
			return -1;
		}
	}
	rb_link_node(&new->ptable_rb, parent, n);
	rb_insert_color(&new->ptable_rb, &scm_head->ptable_rb);
	return 0;
}

static int insert_hptable_node_rb(u64 _id, u64 phys_addr, u64 size)
{
	struct rb_node **n = &scm_head->hptable_rb.rb_node;
	struct rb_node *parent = NULL;
	struct hptable_node *new, *touch;
	new = (struct hptable_node *)get_freenode_addr();
	if (!new) {
		return -1;
	}
	new->_id = _id;
	new->phys_addr = phys_addr;
	new->size = size;
	new->flags = HEAP_REGION;
	/* insert to rbtree */
	while (*n) {
		parent = *n;
		touch = rb_entry(parent, struct hptable_node, hptable_rb);
		if (_id < touch->_id) {
			n = &(*n)->rb_left;
		} else if (_id > touch->_id) {
			n = &(*n)->rb_right;
		} else {
			return -1;
		}
	}
	rb_link_node(&new->hptable_rb, parent, n);
	rb_insert_color(&new->hptable_rb, &scm_head->hptable_rb);
	return 0;
}

int insert_big_region_node(u64 _id, u64 phys_addr, u64 size)
{
	return insert_ptable_node_rb(_id, phys_addr, size, 0, BIG_MEM_REGION);
}

int insert_small_region_node(u64 _id, u64 phys_addr, u64 size, u64 hptable_id)
{
	return insert_ptable_node_rb(_id, phys_addr, size, hptable_id, SMALL_MEM_REGION);
}

int insert_heap_region_node(u64 _id, u64 phys_addr, u64 size)
{
	return insert_hptable_node_rb(_id, phys_addr, size);
}

/* return NULL if not found */
static struct ptable_node *search_ptable_node_rb(u64 _id, unsigned long flags)
{
	struct rb_node *n;
	struct ptable_node *touch;
	if (flags != 0 || flags != 1) {
		return NULL;
	}
	n = scm_head->ptable_rb.rb_node;
	while (n) {
		touch = rb_entry(n, struct ptable_node, ptable_rb);
		if (_id < touch->_id) {
			n = n->rb_left;
		} else if (_id > touch->_id) {
			n = n->rb_right;
		} else {
			/* do a simple check */
			if (touch->flags == flags) {
				return touch;
			} else {
				return NULL;
			}
		}
	}
	return NULL;
}

/* return NULL if not found */
static struct hptable_node *search_hptable_node_rb(u64 _id)
{
	struct rb_node *n;
	struct hptable_node *touch;
	n = scm_head->hptable_rb.rb_node;
	while (n) {
		touch = rb_entry(n, struct hptable_node, hptable_rb);
		if (_id < touch->_id) {
			n = n->rb_left;
		} else if (_id > touch->_id) {
			n = n->rb_right;
		} else {
			/* do a simple check */
			if (touch->flags == HEAP_REGION) {
				return touch;
			} else {
				return NULL;
			}
		}
	}
	return NULL;
}

struct ptable_node *search_big_region_node(u64 _id)
{
	return search_ptable_node_rb(_id, BIG_MEM_REGION);
}

struct ptable_node *search_small_region_node(u64 _id)
{
	return search_ptable_node_rb(_id, SMALL_MEM_REGION);
}

struct hptable_node *search_heap_region_node(u64 _id)
{
	return search_hptable_node_rb(_id);
}

static void add_freenode_addr(void *addr)
{
	struct table_freelist *tmp;
	tmp= (struct table_freelist *)kmalloc(sizeof(struct table_freelist), GFP_KERNEL);
	tmp->node_addr = addr;
	list_add_tail(&tmp->list, &table_freelist.list);
}

/* -1 error, 0 success */
static int delete_ptable_node_rb(u64 _id, unsigned long flags)
{
	struct ptable_node *n;
	n = search_ptable_node_rb(_id, flags);
	if (!n) {
		return -1;
	}
	rb_erase(&n->ptable_rb, &scm_head->ptable_rb);
	add_freenode_addr((void *)n);
	return 0;
}

static int delete_hptable_node_rb(u64 _id)
{
	struct hptable_node *n;
	n = search_hptable_node_rb(_id);
	if (!n) {
		return -1;
	}
	rb_erase(&n->hptable_rb, &scm_head->hptable_rb);
	add_freenode_addr((void *)n);
	return 0;
}


int delete_big_region_node(u64 _id)
{
	return delete_ptable_node_rb(_id, BIG_MEM_REGION);
}

int delete_small_region_node(u64 _id)
{
	return delete_ptable_node_rb(_id, SMALL_MEM_REGION);
}

int delete_heap_region_node(u64 _id)
{
	return delete_hptable_node_rb(_id);
}
