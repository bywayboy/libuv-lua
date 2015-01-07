#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "minheap.h"

//参考:https://github.com/libevent/libevent/blob/master/minheap-internal.h

static int minheap_reserve_(minheap_t* s, unsigned n);
static void minheap_shift_up_(minheap_t* s, unsigned hole_index, minheapnode_t * e);
static void minheap_shift_down_(minheap_t* s, unsigned hole_index, minheapnode_t* e);
static void minheap_shift_up_unconditional_(minheap_t* s, unsigned hole_index, minheapnode_t * e);

#define minheap_elem_greater(a, b) \
	((a)->ev_timeout > (b)->ev_timeout)


void minheap_init(minheap_t* s)
{
	s->p = NULL; s->n = 0; s->a = 0;
}

void minheap_uninit(minheap_t* s)
{
	if (s->p)
		free(s->p);
	s->p = NULL; s->n = 0; s->a = 0;
}

int minheap_empty(minheap_t * s){
	return (0 == s->n);
}
unsigned int minheap_size(minheap_t * s)
{
	return s->n;
}
void minheap_elem_init(minheapnode_t * elm)
{
	elm->minheap_idx = -1;
}

/*节点压入*/
int minheap_push(minheap_t* s, minheapnode_t * e)
{
	if (minheap_reserve_(s, s->n + 1))
		return -1;
	minheap_shift_up_(s, s->n++, e);
	return 0;
}

/*节点弹出*/
minheapnode_t * minheap_pop(minheap_t* s)
{
	if (s->n)
	{
		minheapnode_t * e = *s->p;
		minheap_shift_down_(s, 0u, s->p[--s->n]);
		e->minheap_idx = -1;
		return e;
	}
	return 0;
}


int minheap_elm_is_top(const minheapnode_t * e)
{
	return e->minheap_idx == 0;
}
int minheap_elm_inheap(const minheapnode_t * e)
{
	return e->minheap_idx != -1;
}

/* 从最小堆中干掉一个节点 */
int minheap_erase(minheap_t* s, minheapnode_t* e)
{
	if (-1 != e->minheap_idx)
	{
		minheapnode_t *last = s->p[--s->n];
		unsigned parent = (e->minheap_idx - 1) / 2;
		/* we replace e with the last element in the heap.  We might need to
		   shift it upward if it is less than its parent, or downward if it is
		   greater than one or both its children. Since the children are known
		   to be less than the parent, it can't need to shift both up and
		   down. */
		if (e->minheap_idx > 0 && minheap_elem_greater(s->p[parent], last))
			minheap_shift_up_unconditional_(s, e->minheap_idx, last);
		else
			minheap_shift_down_(s, e->minheap_idx, last);
		e->minheap_idx = -1;
		return 0;
	}
	return -1;
}

/* 调整位置, 用于当超时值被改变后，挑中其在 minheap中的位置. */
int minheap_adjust(minheap_t *s, minheapnode_t *e)
{
	if (-1 == e->minheap_idx) {
		return minheap_push(s, e);
	} else {
		unsigned parent = (e->minheap_idx - 1) / 2;
		/* The position of e has changed; we shift it up or down
		 * as needed.  We can't need to do both. */
		if (e->minheap_idx > 0 && minheap_elem_greater(s->p[parent], e))
			minheap_shift_up_unconditional_(s, e->minheap_idx, e);
		else
			minheap_shift_down_(s, e->minheap_idx, e);
		return 0;
	}
}


/* 扩容*/
static int minheap_reserve_(minheap_t* s, unsigned n)
{
	if (s->a < n)
	{
		minheapnode_t ** p;
		unsigned a = s->a ? s->a * 2 : 8;
		if (a < n)
			a = n;
		if (!(p = (minheapnode_t **)realloc(s->p, a * sizeof *p)))
			return -1;
		s->p = p;
		s->a = a;
	}
	return 0;
}
static void minheap_shift_up_(minheap_t* s, unsigned hole_index, minheapnode_t * e)
{
	unsigned parent = (hole_index - 1) / 2;
	while (hole_index && minheap_elem_greater(s->p[parent], e))
	{
		(s->p[hole_index] = s->p[parent])->minheap_idx = hole_index;
		hole_index = parent;
		parent = (hole_index - 1) / 2;
	}
	(s->p[hole_index] = e)->minheap_idx = hole_index;
}

static void minheap_shift_down_(minheap_t* s, unsigned hole_index, minheapnode_t* e)
{
	unsigned min_child = 2 * (hole_index + 1);
	while (min_child <= s->n)
	{
		min_child -= min_child == s->n || minheap_elem_greater(s->p[min_child], s->p[min_child - 1]);
		if (!(minheap_elem_greater(e, s->p[min_child])))
			break;
		(s->p[hole_index] = s->p[min_child])->minheap_idx = hole_index;
		hole_index = min_child;
		min_child = 2 * (hole_index + 1);
	}
	(s->p[hole_index] = e)->minheap_idx = hole_index;
}

static void minheap_shift_up_unconditional_(minheap_t* s, unsigned hole_index, minheapnode_t * e)
{
	unsigned parent = (hole_index - 1) / 2;
	do
	{
		(s->p[hole_index] = s->p[parent])->minheap_idx = hole_index;
		hole_index = parent;
		parent = (hole_index - 1) / 2;
	} while (hole_index && minheap_elem_greater(s->p[parent], e));
	(s->p[hole_index] = e)->minheap_idx = hole_index;
}