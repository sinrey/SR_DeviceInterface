/*�����������������̬�⣬ʹ�ûص���ʽ�����������ݣ���ǰ��֧��8K����16K��������������
�˶�̬��������ڿ�����SRϵ����Ƶ�豸�Խ���Ӧ�ó���
�ڲ��̶�ʹ��16K��������������������û���ʼ��Ϊ8K�����ڶ�д��������ʱ�������ݽ����ز�����
����ÿ������һ֡��WAVEIN_BUF_SIZE�ֽڣ��ᴥ��һ�λص����ص��������߳��ڵĵ��ã������ڻص������ڲ���UIԪ�ء����ڻص������ڷ�����Ϣ������ź������߳�ͬ��������
*/
#include <stdio.h>
//#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include "SoundCardDll.h"
#include "fifo.h"

#pragma comment(lib, "winmm.lib")

#define WAVEIN_HDR_NUM 2
#define WAVEIN_BUF_SIZE 640

#define WAVEOUT_HDR_NUM 4
#define WAVEOUT_BUF_SIZE 640

struct _WaveIn
{
	int Working;
	unsigned int ReadBytes;
};

struct _WaveOut
{
	int Working;
	unsigned int WriteBytes;
};

static HWAVEIN hWaveIn;
static HWAVEOUT hWaveOut;

static WAVEHDR WaveInHdr[WAVEIN_HDR_NUM];
static char WaveInBuffer[WAVEIN_HDR_NUM][WAVEIN_BUF_SIZE];

static WAVEHDR WaveOutHdr[WAVEOUT_HDR_NUM];
static char WaveOutBuffer[WAVEOUT_HDR_NUM][WAVEOUT_BUF_SIZE];

static unsigned int g_uSampleRate;	//�û���Ƶ���ݵĲ�����
SoundCardCallBack pSoundCardCallBack;	//�û��ص�����

static struct _Fifo g_WaveInFifo;	//�������뻺��
static struct _Fifo g_WaveOutFifo;	//�����������
static struct _WaveIn WaveIn;		//
static struct _WaveOut WaveOut;

static CRITICAL_SECTION g_CriticalSection;	//g_WaveInFifo��g_WaveOutFifo ���ٽ�������

static short* g_pTmpBuffer = NULL;			//��ʱ���壬�ڶ�д��������ʱ�����ز���
static unsigned int g_uTmpBufferSize;

void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	struct _WaveIn* pWaveIn;
	pWaveIn = (struct _WaveIn*)dwInstance;

	PWAVEHDR pWaveHdr;
	pWaveHdr = (PWAVEHDR)dwParam1;

	switch (uMsg)
	{
	case WIM_OPEN:
		//waveInStart(hwi);
		break;
	case WIM_CLOSE:
		for (int i = 0; i < WAVEIN_HDR_NUM; i++)
		{
			waveInUnprepareHeader(hwi, &WaveInHdr[i], sizeof(WAVEHDR));
		}
		break;
	case WIM_DATA:
		if (pWaveIn->Working == 0)break;

		if (pWaveHdr->dwBytesRecorded == WAVEIN_BUF_SIZE)
		{
			EnterCriticalSection(&g_CriticalSection);
			FifoPush(&g_WaveInFifo, pWaveHdr->lpData, WAVEIN_BUF_SIZE);
			LeaveCriticalSection(&g_CriticalSection);
		}
		if (pSoundCardCallBack != NULL)
		{
			pSoundCardCallBack(WIM_DATA, FifoGetDataLength(&g_WaveInFifo), (INT)dwParam2, NULL);
		}

		waveInUnprepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR));
		pWaveHdr->dwFlags = 0;
		waveInPrepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR));
		if (pWaveHdr->dwUser == 0)waveInAddBuffer(hwi, pWaveHdr, sizeof(WAVEHDR));
		break;
	default:
		break;
	}
}

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	struct _WaveOut* pWaveOut;
	pWaveOut = (struct _WaveOut*)dwInstance;

	PWAVEHDR pWaveHdr;
	pWaveHdr = (PWAVEHDR)dwParam1;
	switch (uMsg)
	{
	case WOM_OPEN:
		break;
	case WOM_CLOSE:
		for (int i = 0; i<WAVEOUT_HDR_NUM; i++)
		{
			waveOutUnprepareHeader(hWaveOut, &WaveOutHdr[i], sizeof(WAVEHDR));
		}
		break;
	case WOM_DONE:
		if (pWaveOut->Working == 0)break;

		EnterCriticalSection(&g_CriticalSection);
		if (FifoGetDataLength(&g_WaveOutFifo) >= WAVEOUT_BUF_SIZE)
		{
			FifoPop(&g_WaveOutFifo, pWaveHdr->lpData, WAVEOUT_BUF_SIZE);
		}
		else
		{
			memset(pWaveHdr->lpData, 0, WAVEOUT_BUF_SIZE);
		}
		LeaveCriticalSection(&g_CriticalSection);

		pWaveHdr->dwBufferLength = WAVEOUT_BUF_SIZE;
		waveOutWrite(hwo, pWaveHdr, sizeof(WAVEHDR));
		break;
	default:
		break;
	}
}

/*ʹ��16λ��������ͨ�����̶��Ĳ����ʣ���������������*/
//������
int __stdcall SoundCardInit(UINT32 uSamplerate, SoundCardCallBack pCallBack, UINT32 uFlag)
{
	int ret;
	WAVEFORMATEX wfrm;
	WAVEFORMATEX woutfrm;

	if ((uSamplerate != 8000) && (uSamplerate != 16000))return -1;

	InitializeCriticalSection(&g_CriticalSection);

	g_uTmpBufferSize = sizeof(short)*WAVEIN_BUF_SIZE;
	g_pTmpBuffer = malloc(g_uTmpBufferSize);
	if (g_pTmpBuffer == NULL)return -2;

	g_uSampleRate = uSamplerate;
	pSoundCardCallBack = pCallBack;

	char* inbuf = malloc(64 * WAVEIN_BUF_SIZE);
	if (inbuf == NULL)return -3;
	FifoInit(&g_WaveInFifo, inbuf, 64 * WAVEIN_BUF_SIZE);

	char* outbuf = malloc(64 * WAVEOUT_BUF_SIZE);
	if (inbuf == NULL)return -4;

	FifoInit(&g_WaveOutFifo, outbuf, 64 * WAVEOUT_BUF_SIZE);

	WaveIn.Working = 1;
	WaveIn.ReadBytes = 0;

	wfrm.wFormatTag = WAVE_FORMAT_PCM;
	wfrm.nChannels = 1;
	wfrm.nSamplesPerSec = 16000;//�ڲ��̶�Ϊ16K������������벻��16K����Ҫ�����ز�����
	wfrm.wBitsPerSample = 16;
	wfrm.nBlockAlign = (1 * wfrm.wBitsPerSample) / 8;
	wfrm.nAvgBytesPerSec = wfrm.nSamplesPerSec * wfrm.nBlockAlign;
	wfrm.cbSize = 0;
	ret = waveInOpen(&hWaveIn, WAVE_MAPPER, &wfrm, (DWORD_PTR)waveInProc, (DWORD_PTR)&WaveIn, CALLBACK_FUNCTION);
	if (ret != MMSYSERR_NOERROR)return (1000+ret);

	for (int i = 0; i < WAVEIN_HDR_NUM; i++)
	{
		WaveInHdr[i].lpData = WaveInBuffer[i];
		WaveInHdr[i].dwBufferLength = WAVEIN_BUF_SIZE;
		WaveInHdr[i].dwBytesRecorded = 0;
		WaveInHdr[i].dwUser = 0;
		WaveInHdr[i].dwFlags = 0;
		WaveInHdr[i].dwLoops = 0;
		WaveInHdr[i].reserved = 0;
		WaveInHdr[i].lpNext = NULL;
		ret = waveInPrepareHeader(hWaveIn, &WaveInHdr[i], sizeof(WAVEHDR));
		if (ret != MMSYSERR_NOERROR)return (2000+ret);
		ret = waveInAddBuffer(hWaveIn, &WaveInHdr[i], sizeof(WAVEHDR));
		if (ret != MMSYSERR_NOERROR)return (3000+ret);
	}

	ret = waveInStart(hWaveIn);
	if (ret != MMSYSERR_NOERROR)return (4000+ret);

	WaveOut.Working = 1;
	WaveOut.WriteBytes = 0;

	woutfrm.wFormatTag = WAVE_FORMAT_PCM;
	woutfrm.nChannels = 1;
	woutfrm.nSamplesPerSec = 16000;
	woutfrm.wBitsPerSample = 16;
	woutfrm.nBlockAlign = (1 * woutfrm.wBitsPerSample) / 8;
	woutfrm.nAvgBytesPerSec = woutfrm.nSamplesPerSec * woutfrm.nBlockAlign;
	woutfrm.cbSize = 0;
	ret = waveOutOpen(&hWaveOut, WAVE_MAPPER, &woutfrm, (DWORD_PTR)waveOutProc, (DWORD_PTR)&WaveOut, CALLBACK_FUNCTION);
	if (ret != MMSYSERR_NOERROR)return (5000+ret);

	for (int i = 0; i<WAVEOUT_HDR_NUM; i++)
	{
		WaveOutHdr[i].lpData = WaveOutBuffer[i];
		WaveOutHdr[i].dwBufferLength = 0;
		WaveOutHdr[i].dwBytesRecorded = 0;
		WaveOutHdr[i].dwUser = 0;
		//WaveOutHdr[i].dwFlags = 0;

		if (i == 0)WaveOutHdr[i].dwFlags |= WHDR_BEGINLOOP;
		if (i == WAVEOUT_HDR_NUM - 1)WaveOutHdr[i].dwFlags |= WHDR_ENDLOOP;

		WaveOutHdr[i].dwLoops = 0;
		WaveOutHdr[i].lpNext = NULL;
		WaveOutHdr[i].reserved = 0;

		ret = waveOutPrepareHeader(hWaveOut, &WaveOutHdr[i], sizeof(WAVEHDR));
		if (ret != MMSYSERR_NOERROR)return (6000+ret);
	}

	for (int i = 0; i<WAVEOUT_HDR_NUM; i++)
	{
		memset(WaveOutHdr[i].lpData, 0, sizeof(WAVEOUT_BUF_SIZE));
		WaveOutHdr[i].dwBufferLength = WAVEOUT_BUF_SIZE;
		waveOutWrite(hWaveOut, &WaveOutHdr[i], sizeof(WAVEHDR));
	}
	return 0;
}

//�ر�����
int __stdcall SoundCardClose()
{
	int ret;
	WaveIn.Working = 0;

	ret = waveInStop(hWaveIn);
	ret = waveInReset(hWaveIn);
	ret = waveInClose(hWaveIn);

	WaveOut.Working = 0;

	ret = waveOutReset(hWaveOut);
	ret = waveOutClose(hWaveOut);

	free(g_WaveInFifo.Buffer);
	free(g_WaveOutFifo.Buffer);

	DeleteCriticalSection(&g_CriticalSection);
	return ret;
}

//������������
//uOutBufferSize ������ȡ���ֽ���
//puReadBytes ʵ�ʶ�ȡ���ֽ���
//˵�����������û���㹻���ݣ��������κ����ݡ�
int __stdcall SoundCardReadFrom(char* pOutBuffer, UINT32 uOutBufferSize, PUINT32 puReadBytes)
{
	EnterCriticalSection(&g_CriticalSection);
	unsigned int len = FifoGetDataLength(&g_WaveInFifo);
	if (g_uSampleRate == 8000)
	{
		if (len >= (2 * uOutBufferSize))
		{
			if (g_uTmpBufferSize < (2 * uOutBufferSize))
			{
				free(g_pTmpBuffer);
				g_uTmpBufferSize = 2 * uOutBufferSize;
				g_pTmpBuffer = malloc(g_uTmpBufferSize);
			}
			FifoPop(&g_WaveInFifo, (char*)g_pTmpBuffer, 2*uOutBufferSize);

			short* p = (short*)pOutBuffer;
			for (unsigned int i = 0; i < uOutBufferSize/2; i++)
			{
				p[i] = ((int)g_pTmpBuffer[2 * i + 0] + (int)g_pTmpBuffer[2 * i + 1]) / 2;
			}
			*puReadBytes = uOutBufferSize;
		}
		else
		{
			*puReadBytes = 0;
		}
	}
	else
	{
		if (len >= uOutBufferSize)
		{
			FifoPop(&g_WaveInFifo, pOutBuffer, uOutBufferSize);
			*puReadBytes = uOutBufferSize;
		}
		else
		{
			*puReadBytes = 0;
		}	
	}

	LeaveCriticalSection(&g_CriticalSection);
	return 0;
}

//uInBufferLengthд�����Ƶ�����ֽ���
int __stdcall SoundCardWriteTo(const char* pInBuffer, UINT32 uInBufferLength)
{
	static short predata = 0;//�ϴεĲ���ֵ������16000->8000���ز�������
	short* p = (short*)pInBuffer;

	EnterCriticalSection(&g_CriticalSection);
	if (g_uSampleRate == 8000)
	{
		if (g_uTmpBufferSize < (2*uInBufferLength))
		{
			free(g_pTmpBuffer);
			g_uTmpBufferSize = 2*uInBufferLength;
			g_pTmpBuffer = malloc(g_uTmpBufferSize);
		}
		
		for (unsigned int i = 0; i < uInBufferLength/2; i++)
		{
			g_pTmpBuffer[2*i+0] = (predata + p[i])/2;
			g_pTmpBuffer[2*i+1] = p[i];
			predata = p[i];
		}

		FifoPush(&g_WaveOutFifo, (char*)g_pTmpBuffer, 2*uInBufferLength);
	}
	else
	{
		FifoPush(&g_WaveOutFifo, (char*)p, uInBufferLength);
	}
	LeaveCriticalSection(&g_CriticalSection);
	return 0;
}