
#pragma once

typedef int(__stdcall *SoundCardCallBack)(INT MsgType, INT WParam, INT LParam, VOID* pOutParam);

__declspec(dllexport) INT __stdcall SoundCardInit(UINT32 uSamplerate, SoundCardCallBack pCallBack, UINT32 uFlag);
__declspec(dllexport) INT __stdcall SoundCardClose();

/*
�����������ȡnOutBufferSize�������ݣ����浽pOutBuffer�У����������������С��nOutBufferSize��������ȡ�κ����ݡ�pnReadBytesΪʵ�ʶ�ȡ�����ݳ��ȡ�
*/
__declspec(dllexport) INT __stdcall SoundCardReadFrom(char* pOutBuffer, UINT32 uOutBufferSize, PUINT32 puReadBytes);
__declspec(dllexport) INT __stdcall SoundCardWriteTo(const char* pInBuffer, UINT32 uInBufferLength);

