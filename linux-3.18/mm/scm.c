#include <linux/scm.h>
#include <linux/memblock.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/rbtree.h>

static struct scm_head *scm_head;
static struct table_freelist *table_freelist;

/* FOR DEBUG */
static u64 freecount = 0; // count freelist
static void scm_print_freelist(void)
{
	struct table_freelist *tmp;
	int i=0;
	daisy_printk("freelist %lu: ", freecount);
	list_for_each_entry(tmp, &table_freelist->list, list) {
		daisy_printk("%lu %lu\t", tmp->node_addr, ((unsigned long)tmp->node_addr-(unsigned long)&scm_head->data)/sizeof(struct ptable_node));
		++i;
		if (i>=10) break;
	}
	daisy_printk("\n");
}

static void scm_print_pnode(struct ptable_node *n)
{
	if (n) {
		daisy_printk("id: %lu, vaddr %lu\n", n->_id, n);
	} else {
		daisy_printk("NULL\n");
	}
}

static void scm_fake_initdata(void)
{
	struct ptable_node *pnode;
	struct hptable_node *hnode;

	if (!scm_head) return;

	pnode = (struct ptable_node *)((char *)&scm_head->data + 3*sizeof(struct ptable_node));
	scm_head->ptable_rb.rb_node = &pnode->ptable_rb;
	hnode = (struct hptable_node *)((char *)&scm_head->data + 5*sizeof(struct ptable_node));
	scm_head->hptable_rb.rb_node = &hnode->hptable_rb;
}

void scm_full_test(void)
{
	struct ptable_node *n;
	struct page *page;

	insert_big_region_node(345, 0, 0);
	scm_print_freelist();
	insert_small_region_node(344, 0, 0, 557);
	insert_heap_region_node(557, 0, 0);
	insert_big_region_node(342, 0, 0);
	scm_print_freelist();
	n = search_big_region_node(342);
	scm_print_pnode(n);
	n = search_small_region_node(344);
	scm_print_pnode(n);
	n = search_heap_region_node(557);
	scm_print_pnode(n);
	delete_big_region_node(342);
	delete_heap_region_node(557);
	delete_small_region_node(344);
	delete_big_region_node(345);
	scm_print_freelist();
	page = alloc_pages(GFP_KERNEL | GFP_SCM, 0);
	daisy_printk("alloc_pages: %s %lu %lu %lu\n", page_zone(page)->name, page_to_pfn(page), PFN_PHYS(page_to_pfn(page)), page_address(page));
	page = alloc_pages(GFP_KERNEL | GFP_DMA, 0);
	daisy_printk("alloc_pages: %s %lu %lu %lu\n", page_zone(page)->name, page_to_pfn(page), PFN_PHYS(page_to_pfn(page)), page_address(page));
	page = alloc_pages(GFP_KERNEL, 0);
	daisy_printk("alloc_pages: %s %lu %lu %lu\n", page_zone(page)->name, page_to_pfn(page), PFN_PHYS(page_to_pfn(page)), page_address(page));
}
/* end FOR DEBUG */

static void reserve_scm_ptable_memory(void)
{
	unsigned long size;
	phys_addr_t phys;

	/* get the first 1024 scm pages */
	size = SCM_PTABLE_PFN_NUM * PAGE_SIZE;
	/* pages in ZONE_SCM */
	phys = PFN_PHYS(max_pfn_mapped)-(SCM_PFN_NUM<<PAGE_SHIFT);
	memblock_reserve(phys, size);
	scm_head = (struct scm_head*)__va(phys);

	daisy_printk("scm_start_phys: %lu scm_head vaddr %lu\n", phys, scm_head);
	daisy_printk("Get start pfn: %lu， max_pfn: %lu\n", phys >> PAGE_SHIFT, max_pfn_mapped);
	/* record the size */
	/* TODO if scm has old data, total_size cannot change, do a realloc; now just check */
	if (scm_head->magic == SCM_MAGIC && scm_head->total_size !=size) {
		daisy_printk("TODO we need a warning or realloc here.\n");
	}
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

static void scm_reserve_used_memory(void) {
	struct rb_node *nd;
	/* ptable */
	if (!RB_EMPTY_ROOT(&scm_head->ptable_rb)) {
		for (nd = rb_first(&scm_head->ptable_rb); nd; nd = rb_next(nd)) {
			struct ptable_node *touch;
			touch = rb_entry(nd, struct ptable_node, ptable_rb);
			/* ignore small memory region */
			if (touch->flags == BIG_MEM_REGION) {
				memblock_reserve(touch->phys_addr, touch->size);
			}
		}
	}
	/* hptable */
	if (!RB_EMPTY_ROOT(&scm_head->hptable_rb)) {
		for (nd = rb_first(&scm_head->hptable_rb); nd; nd = rb_next(nd)) {
			struct hptable_node *touch;
			touch = rb_entry(nd, struct hptable_node, hptable_rb);
			memblock_reserve(touch->phys_addr, touch->size);
		}
	}
}

/**
 * scm persist table boot step
 * reference to: numa_alloc_distance & numa_reset_distance
 */
void scm_ptable_boot(void)
{
	reserve_scm_ptable_memory();

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
		//scm_fake_initdata();
	} else {
		/* SCM with data! */
		scm_reserve_used_memory();
	}
}

/**
 * Just traverse the tree to init the freelist in DRAM
 * memblock reserve at the same time
 */
void scm_freelist_boot(void)
{
	struct table_freelist *tmp;
	unsigned long index;
	struct rb_node *nd;
	char usage_map[scm_head->len];

	table_freelist = (struct table_freelist *) kmalloc(sizeof(struct table_freelist), GFP_KERNEL);
	INIT_LIST_HEAD(&table_freelist->list);

	for (index = 0; index < scm_head->len; ++index) {
		usage_map[index] = 0;
	}
	/* ptable */
	if (!RB_EMPTY_ROOT(&scm_head->ptable_rb)) {
		for (nd = rb_first(&scm_head->ptable_rb); nd; nd = rb_next(nd)) {
			struct ptable_node *touch;
			touch = rb_entry(nd, struct ptable_node, ptable_rb);
			/* ignore small memory region */
			if (touch->flags == BIG_MEM_REGION) {
				index = ((unsigned long) touch - (unsigned long) &scm_head->data) / sizeof(struct ptable_node);
				usage_map[index] = 1;
			}
		}
	}
	/* hptable */
	if (!RB_EMPTY_ROOT(&scm_head->hptable_rb)) {
		for (nd = rb_first(&scm_head->hptable_rb); nd; nd = rb_next(nd)) {
			struct hptable_node *touch;
			touch = rb_entry(nd, struct hptable_node, hptable_rb);
			index = ((unsigned long) touch - (unsigned long) &scm_head->data) / sizeof(struct hptable_node);
			usage_map[index] = 1;
		}
	}
	/* freelist */
	for (index = 0; index < scm_head->len; ++index) {
		if (usage_map[index] == 0) {
			tmp = (struct table_freelist *) kmalloc(sizeof(struct table_freelist), GFP_KERNEL);
			tmp->node_addr = (char *) &scm_head->data + index * sizeof(struct ptable_node);
			list_add_tail(&tmp->list, &table_freelist->list);
			freecount++;
		}
	}
	/* TODO test SCM is not new */
	scm_print_freelist();
	scm_full_test();
}

/* pop an item from freelist, free the item, return the addr (NULL if no more) */
static void *get_freenode_addr(void)
{
	void *ret;
	struct table_freelist *entry;
	if (list_empty(&table_freelist->list)) {
		return NULL;
	}
	entry = list_first_entry(&table_freelist->list, struct table_freelist, list);
	ret = entry->node_addr;
	list_del(&entry->list);
	freecount--;
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

int insert_small_region_node(u64 _id, u64 offset, u64 size, u64 hptable_id)
{
	return insert_ptable_node_rb(_id, offset, size, hptable_id, SMALL_MEM_REGION);
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
	if (flags != BIG_MEM_REGION && flags != SMALL_MEM_REGION) {
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
	list_add(&tmp->list, &table_freelist->list);
	freecount++;
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

SYSCALL_DEFINE1(p_search_big_region_node, unsigned long, id) {
	return 123;
}

SYSCALL_DEFINE2(p_alloc_and_insert, unsigned long, id, int, size) {
	return 234;
}