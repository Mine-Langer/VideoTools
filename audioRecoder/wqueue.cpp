#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include "wqueue.h"

WQueue::WQueue()
{

}

WQueue::~WQueue()
{
	Clear();
}

int WQueue::SetMemoryMode(unsigned int fBuffered)
{
	Clear();
	
	c_fUnBuffered = 0;

	if (fBuffered == 0)
		c_fUnBuffered = 1;
	
	return 1;
}

unsigned int WQueue::PushFirst(void* pSource, unsigned int nSize)
{
	if (pSource == nullptr || nSize == 0)
		return 0;

	QUEUE_ELEMENT* elem = (QUEUE_ELEMENT*)malloc(sizeof(QUEUE_ELEMENT));
	if (elem == 0)
		return 0;

	if (c_fUnBuffered)
	{
		elem->alloc_buf = 0;
		elem->data = pSource;
	}
	else
	{
		void* data = malloc(nSize);
		if (data == nullptr)
		{
			free(elem);
			return 0;
		}
		memcpy(data, pSource, nSize);

		elem->alloc_buf = data;
		elem->data = data;
	}

	elem->data_id = pSource;
	elem->size = nSize;
	c_sum_size += nSize;
	c_num++;

	if (c_first == nullptr)
	{
		elem->prev = nullptr;
		elem->next = nullptr;
		c_first = elem;
		c_last = elem;
		return nSize;
	}

	elem->prev = nullptr;
	elem->next = c_first;
	c_first->prev = elem;
	c_first = elem;

	return nSize;
}

unsigned int WQueue::Clear()
{
	return 0;
}
