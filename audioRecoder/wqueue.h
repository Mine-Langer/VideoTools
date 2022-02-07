#pragma once

typedef struct
{
	unsigned int nStart;
	unsigned int nEnd;
	unsigned int nSamplesNum;
	unsigned int nSeekPosition;
	unsigned int fReverse;
	void* pSamples;
	unsigned int nSize;
}QUEUE_SAMPLES_BUFFER;

typedef struct w_queue_elem
{
	void* alloc_buf;
	void* data;
	void* data_id;
	unsigned int size;
	struct w_queue_elem* prev;
	struct w_queue_elem* next;
}QUEUE_ELEMENT;

class WQueue
{
public:
	WQueue();
	~WQueue();

	int SetMemoryMode(unsigned int fBuffered);

	unsigned int PushFirst(void* pSource, unsigned int nSize);

	unsigned int PushLast(void* pSource, unsigned int nSize);

	unsigned int PullFirst(void* pDest, unsigned int nSize);

	unsigned int PullLast(void* pDest, unsigned int nSize);

	unsigned int QueryFirst(void* pDest, unsigned int nSize);

	unsigned int QueryLast(void* pDest, unsigned int nSize);

	int FindFromFirst(void* pSource);

	int FindFromLast(void* pSource);

	unsigned int Clear();

	unsigned int GetCount();

	unsigned int GetSizeSum();

	unsigned int PullDataFifo(void* pOutputBuffer, unsigned int nBytesToRead, int* pnBufferIsOut);

	unsigned int QueryFirstPointer(void** ppDest, unsigned int* pnSize);

	unsigned int QueryLastPointer(void** ppDest, unsigned int* pnSize);

	int QueryData(unsigned int nOffset, void* pOutputBuffer, unsigned int nBytesToRead, unsigned int* nBytesRead);

	int CutDataFifo(unsigned int nBytesToCut);

private:
	QUEUE_ELEMENT* c_first = nullptr;
	QUEUE_ELEMENT* c_last = nullptr;
	unsigned int c_num = 0;
	unsigned int c_sum_size = 0;
	unsigned int c_fUnBuffered = 0;
};

class DelayLine
{
public:
	DelayLine();
	~DelayLine();
	int Allocate(unsigned int nMaxDelaySize, unsigned int nChunkSize);
	int ReAllocate(unsigned int nMaxDelaySize, unsigned int nChunkSize);
	int Free();
	int GetLoad() { return c_nLoad; }
	int AddData(char* pchData, unsigned int nDataSize);
	char* GetData(unsigned int nDelay);
	void Clear();

private:
	char* c_pchMainBuffer = nullptr;
	unsigned int c_nMainBufferSize = 0;
	char* c_pchWindowStart = nullptr;
	char* c_pchWindowEnd = nullptr;
	char* c_pchNewData = nullptr;
	unsigned int c_nWindowSize = 0;
	unsigned int c_nChunkSize = 0;
	unsigned int c_nLoad = 0;
	unsigned int c_nShift = 0;
};