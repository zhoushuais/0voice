


// 

// 每个节点对应一块连续的池内小内存区
typedef struct mp_node_s {
	unsigned char *last;	// 节点内当前可用内存起始地址（分配起点，分配后向后偏移）
	unsigned char *end; 	 // 节点内内存结束地址（内存边界，last<end时才有可用空间）
	struct mp_node_s *next; 	// 下一个小内存节点指针，构成小内存节点链表
} mp_node_t;

// 大内存管理节点
// 池内不存储实际大内存数据，仅通过该结构体维护大内存的malloc分配地址，通过next指针形成单向链表，实现大内存的统一管理和复用：
typedef struct mp_large_s {
	struct mp_large_s *next;	 // 下一个大内存管理节点指针，构成大内存管理链表
	void *alloc;				// 实际大内存的malloc分配地址（核心，释放时操作该地址）
} mp_large_t;
 
// 内存池管理器：mp_pool_t（总控入口）
typedef struct mp_pool_s {
	size_t max; 			// 小/大内存分配阈值（≤为小内存，>为大内存）
	struct mp_node_s *head;		// 小内存节点链表的头指针（指向第一个小内存节点）
	struct mp_large_s *large; 	// 大内存管理链表的头指针（指向第一个大内存管理节点）
} mp_pool_t;


int mp_create(mp_pool_t *pool, size_t size);
void mp_destory(mp_pool_t *pool);
void *mp_alloc(mp_pool_t *pool, size_t size);
void mp_free(mp_pool_t *pool, void *ptr);



// size : 4096
int mp_create(mp_pool_t *pool, size_t size) {

	if (!pool || size <= 0) return -1;

	void *mem = malloc(size);


	struct mp_node_s *node = (struct mp_node_s *)mem;
	node->last = (char *)mem + sizeof(struct mp_node_s);
	node->end = (char *)mem + size;
	node->next = NULL;

	pool->head = node;
	pool->max = size;
	pool->large = NULL;

	return 0;
}

// 彻底销毁内存池，回收所有资源（大内存、所有小内存节点），调用后内存池不可再使用；
void mp_destory(mp_pool_t *pool) {

	// 第一步：释放所有大内存（仅释放实际malloc的内存，管理节点随小内存节点释放）
	mp_large_t *l;

	for (l = pool->large;l;l = l->next) {
		if (l->alloc) {
			free(l->alloc);
		}
	}
	pool->large = NULL; // 清空大内存链表头，避免野指针

	mp_node_t *node = pool->head;

	// 第二步：释放所有小内存节点（包含大内存的管理节点，因为其从池内分配）
	while (!node) {
		mp_node_t *tmp = node->next;
		free(node);
		node = tmp;
	}
	
}


// 小内存节点扩展：mp_alloc_block
// 功能：当所有现有小内存节点均无可用空间时，新建一个小内存节点并加入链表尾部，为后续小内存分配提供空间；
static void *mp_alloc_block(mp_pool_t *pool, size_t size) {

	struct mp_node_s *node = (struct mp_node_s *)mem;

	// 初始化新节点的可用区间（与第一个节点逻辑一致）
	node->last = (char *)mem + sizeof(struct mp_node_s);
	node->end = (char *)mem + size;
	node->next = NULL;
	
	// 分配内存：从新节点的可用起始地址取空间，更新last指针
	void *ptr = node->last;
	node->last += size;

	// 尾插法：将新节点加入小内存链表尾部（原版为头插法，简化后性能降低）
	mp_node_t *iter = pool->head;
	while (iter->next != NULL) {
		iter = iter->next;
	}
	iter->next = node;

	return ptr;
}

// 大内存分配：mp_alloc_large（静态内部函数）
// 功能：处理大内存分配请求（size>pool->max），直接malloc实际大内存，池内仅维护管理节点，支持管理节点复用；
static void *mp_alloc_large(mp_pool_t *pool, size_t size) {
	if (!pool)

	// 大内存直接池外malloc，池内仅管理地址
	void *ptr = malloc(size);
	if (ptr == NULL) return NULL;

	// 第一步：复用空闲的大内存管理节点（alloc=NULL表示空闲），避免频繁创建/销毁
	mp_large_t *l;
	for (l = pool->large;l;l = l->next) {
		if (l->alloc == NULL) {
			l->alloc = ptr;
			return ptr;
		}
	}

	// 第二步：无空闲节点，从内存池小内存中分配一个新的管理节点
	l = mp_alloc(pool, sizeof(mp_large_t));
	if (l == NULL) {
		free(ptr);
		return NULL;
	}
	l->alloc = ptr;

	// 头插法：将新管理节点加入大内存链表头部（O(1)时间复杂度，高效）
	l->next = pool->large;
	pool->large = l;
	
	return ptr;
}

// 内存分配主接口：mp_alloc
// 内存池对外核心分配接口，自动判断小 / 大内存，调用对应内部函数完成分配，是用户分配内存的唯一入口；
void *mp_alloc(mp_pool_t *pool, size_t size) {

	if (size > pool->max) {
		// malloc_large;

		return mp_alloc_large(pool, size);
	} 

	// size <= pool->max
	void *ptr = NULL;
	
	mp_node_t *node = pool->head;

	do {

		if (node->end - node->last > size) {
			ptr = node->last;
			node->last += size;

			return ptr;
		} 
		node = node->next;
		
	} while (node);

	

	return mp_alloc_block(pool, size);
	

}

// 
void mp_free(mp_pool_t *pool, void *ptr) {

	mp_large_t *l;

	for (l = pool->large;l;l = l->next) {
		if (l->alloc == ptr) {
			free(l->alloc);
			l->alloc = NULL;
			return ;
		}
	}

	// 
	
}

int main() {

	

}

