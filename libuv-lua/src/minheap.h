#ifndef _WIFISERVER_MINHEAP_H
#define _WIFISERVER_MINHEAP_H

#include <stdint.h>


/*
	minheap 最小堆算法. 代码来自 libevent.
	用作超时计算.
*/

typedef struct min_heap			minheap_t;
typedef struct minheapnode		minheapnode_t;

struct minheapnode
{
	int32_t minheap_idx;	// 在 min heap. 中的索引.
	time_t ev_timeout;		// 超时依据：
};

struct min_heap
{
	minheapnode_t ** p;
	unsigned int n, a;
};
/*
	初始化一个最小堆
*/
void minheap_init(minheap_t* s);

/*
	释放一个最小堆
*/
void minheap_uninit(minheap_t* s);

/*
	判断一个最小堆是否为空
*/
int minheap_empty(minheap_t * s);
/*
	返回最小堆是否为空.
*/
unsigned int minheap_size(minheap_t * s);
/*
	将一个节点压入到最小堆中.
*/
int minheap_push(minheap_t* s, minheapnode_t * e);
/*
	从最小堆中弹出一个节点. 只弹出顶端的.
*/
minheapnode_t* minheap_pop(minheap_t* s);
/*
	从最小堆中删除一个节点.
*/
int minheap_erase(minheap_t* s, minheapnode_t* e);
/*
	调整节点在最小堆中的位置. 当 e的 ev_timeout成员值发生改变后需要执行该函数.
*/
int minheap_adjust(minheap_t *s, minheapnode_t *e);


/*
	初始化一个最小堆元素的索引。注意：如果元素存在于最小堆，则不可执行该函数！
*/
void minheap_elem_init(minheapnode_t * elm);

/*
	判断当前元素是否存在于最小堆的顶端.
*/
int minheap_elm_is_top(const minheapnode_t * e);
/*
	判断一个元素是否存在于一个 最小堆中.
*/
int minheap_elm_inheap(const minheapnode_t * e);
#endif