#ifndef _SR_FIFO_H_
#define _SR_FIFO_H_

struct _Fifo
{
	unsigned int   BufferSize;
	unsigned int   ReadBytes;
	unsigned int   WriteBytes;
	unsigned char* pWrite;
	unsigned char* pRead;
	unsigned char*  Buffer;
};

int FifoInit(struct _Fifo* pFifo, unsigned char* buffer, int bufsize);
int FifoGetDataLength(struct _Fifo* pFifo);
int FifoGetFreeSize(struct _Fifo* pFifo);
int FifoPeek(struct _Fifo* pFifo, unsigned char* Dat, int DatLength);
int FifoPush(struct _Fifo* pFifo, unsigned char* Dat, int DatLength);
int FifoPop(struct _Fifo* pFifo, unsigned char* Dat, int DatLength);
void FifoClear(struct _Fifo* pFifo);

#endif
