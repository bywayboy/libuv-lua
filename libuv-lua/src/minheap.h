#ifndef _WIFISERVER_MINHEAP_H
#define _WIFISERVER_MINHEAP_H

#include <stdint.h>


/*
	minheap ��С���㷨. �������� libevent.
	������ʱ����.
*/

typedef struct min_heap			minheap_t;
typedef struct minheapnode		minheapnode_t;

struct minheapnode
{
	int32_t minheap_idx;	// �� min heap. �е�����.
	time_t ev_timeout;		// ��ʱ���ݣ�
};

struct min_heap
{
	minheapnode_t ** p;
	unsigned int n, a;
};
/*
	��ʼ��һ����С��
*/
void minheap_init(minheap_t* s);

/*
	�ͷ�һ����С��
*/
void minheap_uninit(minheap_t* s);

/*
	�ж�һ����С���Ƿ�Ϊ��
*/
int minheap_empty(minheap_t * s);
/*
	������С���Ƿ�Ϊ��.
*/
unsigned int minheap_size(minheap_t * s);
/*
	��һ���ڵ�ѹ�뵽��С����.
*/
int minheap_push(minheap_t* s, minheapnode_t * e);
/*
	����С���е���һ���ڵ�. ֻ�������˵�.
*/
minheapnode_t* minheap_pop(minheap_t* s);
/*
	����С����ɾ��һ���ڵ�.
*/
int minheap_erase(minheap_t* s, minheapnode_t* e);
/*
	�����ڵ�����С���е�λ��. �� e�� ev_timeout��Աֵ�����ı����Ҫִ�иú���.
*/
int minheap_adjust(minheap_t *s, minheapnode_t *e);


/*
	��ʼ��һ����С��Ԫ�ص�������ע�⣺���Ԫ�ش�������С�ѣ��򲻿�ִ�иú�����
*/
void minheap_elem_init(minheapnode_t * elm);

/*
	�жϵ�ǰԪ���Ƿ��������С�ѵĶ���.
*/
int minheap_elm_is_top(const minheapnode_t * e);
/*
	�ж�һ��Ԫ���Ƿ������һ�� ��С����.
*/
int minheap_elm_inheap(const minheapnode_t * e);
#endif