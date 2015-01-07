#include <stdio.h>
#include <stdlib.h>


#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif
#include "automem.h"

void automem_init(automem_t* pmem, unsigned int initBufferSize)
{ 
	pmem->size = 0;
	pmem->buffersize = (initBufferSize == 0 ? 128 : initBufferSize);
	pmem->_malloc = malloc;
	pmem->_realloc = realloc;
	pmem->_free= free;
	pmem->pdata = (unsigned char*) pmem->_malloc(pmem->buffersize);

	
}

void automem_uninit(automem_t* pmem)
{
	if(NULL != pmem->pdata)
	{
		pmem->size = pmem->buffersize = 0;
		pmem->_free(pmem->pdata);
		pmem->pdata = NULL;
	}
}
/*清理并重新初始化*/
void automem_clean(automem_t* pmen, int newsize)
{
	automem_uninit(pmen);
	automem_init(pmen, newsize);
}

void automem_reset(automem_t* pmem)
{
	pmem->size = 0;
}

void automem_ensure_newspace(automem_t* pmem, unsigned int len)
{
	unsigned int newbuffersize = pmem->buffersize;

	if(newbuffersize < len)
		newbuffersize = len;
	while(newbuffersize - pmem->size < len)
	{
		newbuffersize *= 2;
	}

	if(newbuffersize > pmem->buffersize)
	{
		if(pmem->pdata)
			pmem->pdata = (unsigned char*) pmem->_realloc(pmem->pdata, newbuffersize);
		else
			pmem->pdata = (unsigned char*) pmem->_malloc(newbuffersize);
		pmem->buffersize = newbuffersize;
	}
}

void automem_init_by_ptr(automem_t* pmem, void* pdata, unsigned int len)
{
	pmem->_malloc = malloc;
	pmem->_realloc = realloc;
	pmem->_free= free;
	pmem->pdata = (unsigned char *)pdata;
	pmem->size = len;
}
/* 绑定到一块内存空间 */
void automem_attach(automem_t* pmem, void* pdata, unsigned int len)
{
	if(len > pmem->buffersize)
		automem_ensure_newspace(pmem, len - pmem->buffersize);
	pmem->size = len;
	memcpy(pmem->pdata, pdata, len);
}

//after automem_detach(), pmem is not available utill next automem_init()
//returned value must be pmem->_free(), if not NULL

/* 获取数据指针和长度 */

void* automem_detach(automem_t* pmem, unsigned int* plen)
{
	void* pdata = (pmem->size == 0 ? NULL : pmem->pdata);
	if(plen) *plen = pmem->size;
	return pdata;
}

void * automem_alloc(automem_t * pmem, int size){
	void *ret = NULL;
	if(size > 0){
		automem_ensure_newspace(pmem, size);
		ret = pmem->pdata + pmem->size;
		pmem->size += size;
	}
	return ret;
}
int automem_append_voidp(automem_t* pmem, const void* p, unsigned int len)
{
	automem_ensure_newspace(pmem, len);
	memcpy(pmem->pdata + pmem->size, p, len);
	pmem->size += len;
	return len;
}

int automem_erase(automem_t * pmem, unsigned int size)
{
	if(size < pmem->size)
	{
		memmove(pmem->pdata, pmem->pdata + size, pmem->size - size);
		pmem->size -= size;
	}else{
		pmem->size = 0;
	}
	return pmem->size;
}

int automem_erase_ex(automem_t* pmem, unsigned int size,unsigned int limit)
{

	if(size < pmem->size)
	{
		unsigned int newsize = pmem->size - size;

		if(pmem->buffersize > (newsize + limit))
		{
			char * buffer = (char*) pmem->_malloc(newsize + limit);
			memcpy(buffer, pmem->pdata + size, newsize);
			pmem->_free(pmem->pdata);
			pmem->pdata = (unsigned char*)buffer;
			pmem->size = newsize;
			pmem->buffersize = newsize + limit;
		}	
		else
		{
			memmove(pmem->pdata, pmem->pdata + size, newsize);
			pmem->size = newsize;
		}
	}
	else
	{
		pmem->size = 0;
		if(pmem->buffersize > limit)
		{
			automem_uninit(pmem);
			automem_init(pmem, limit);
		}
	}
	return pmem->size;
	
}

void automem_append_int(automem_t* pmem, int n)
{
	automem_append_voidp(pmem, &n, sizeof(int));
}
void automem_append_pchar(automem_t* pmem, char* n)
{
	automem_append_voidp(pmem, &n, sizeof(char*));
}

void automem_append_char(automem_t* pmem, char c)
{
	automem_append_voidp(pmem, &c, sizeof(char));
}
void automem_append_byte(automem_t* pmem, unsigned char c)
{
	automem_append_voidp(pmem, &c, sizeof(unsigned char));
}


