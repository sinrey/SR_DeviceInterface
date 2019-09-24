#include <stdlib.h>
#include <string.h>
#include "fifo.h"

int FifoInit(struct _Fifo* pFifo, unsigned char* buffer, int bufsize)
{
	memset(pFifo, 0, sizeof(struct _Fifo));
	pFifo->Buffer = buffer;
	pFifo->BufferSize = bufsize;
	pFifo->pRead = pFifo->pWrite = pFifo->Buffer;
	pFifo->ReadBytes = 0;
	pFifo->WriteBytes = 0;
	return 0;
}
int FifoGetDataLength(struct _Fifo* pFifo)
{
	if (pFifo->pWrite >= pFifo->pRead)
	{
		return pFifo->pWrite - pFifo->pRead;
	}
	else
	{
		return pFifo->pWrite + pFifo->BufferSize - pFifo->pRead;
	}
}

int FifoGetFreeSize(struct _Fifo* pFifo)
{
	return (pFifo->BufferSize - FifoGetDataLength(pFifo));
}

int FifoPeek(struct _Fifo* pFifo, unsigned char* Dat, int DatLength)
{
	int i;
	int count = 0;
	unsigned char* pRead;
	unsigned char* pWrite;
	pRead = pFifo->pRead;
	pWrite = pFifo->pWrite;
	for (i = 0; i<DatLength; i++)
	{
		if (pRead == pWrite)break;
		*Dat = *pRead;
		pRead++; Dat++;
		count++;
		if (pRead >= (pFifo->Buffer + pFifo->BufferSize))pRead = pFifo->Buffer;
	}
	return count;

}

int FifoPush(struct _Fifo* pFifo, unsigned char* Dat, int DatLength)
{
	int i;
	pFifo->WriteBytes += DatLength;
	for (i = 0; i<DatLength; i++)
	{
		*pFifo->pWrite = *Dat;
		pFifo->pWrite++; Dat++;
		if (pFifo->pWrite >= (pFifo->Buffer + pFifo->BufferSize))pFifo->pWrite = pFifo->Buffer;
	}
	return 0;
}


int FifoPop(struct _Fifo* pFifo, unsigned char* Dat, int DatLength)
{
	int i;
	int count = 0;
	for (i = 0; i<DatLength; i++)
	{
		if (pFifo->pRead == pFifo->pWrite)break;
		*Dat = *pFifo->pRead;
		pFifo->pRead++; Dat++;
		count++;
		if (pFifo->pRead >= (pFifo->Buffer + pFifo->BufferSize))pFifo->pRead = pFifo->Buffer;
	}
	pFifo->ReadBytes += count;
	return count;
}

void FifoClear(struct _Fifo* pFifo)
{
	pFifo->pRead = pFifo->pWrite = pFifo->Buffer;
}
