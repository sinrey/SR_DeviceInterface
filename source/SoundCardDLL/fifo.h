#ifndef _SR_FIFO_H_
#define _SR_FIFO_H_
#include <windows.h>

struct _Fifo
{
	INT   BufferSize;
	INT   ReadBytes;
	INT   WriteBytes;
	char* pWrite;
	char* pRead;
	char*  Buffer;
};

int FifoInit(struct _Fifo* pFifo, char* buffer, INT bufsize);
int FifoGetDataLength(struct _Fifo* pFifo);
int FifoGetFreeSize(struct _Fifo* pFifo);
int FifoPeek(struct _Fifo* pFifo, char* Dat, INT DatLength);
int FifoPush(struct _Fifo* pFifo, char* Dat, INT DatLength);
int FifoPop(struct _Fifo* pFifo, char* Dat, INT DatLength);
void FifoClear(struct _Fifo* pFifo);

#endif
