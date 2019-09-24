
#pragma once

typedef int(__stdcall *SoundCardCallBack)(INT MsgType, INT WParam, INT LParam, VOID* pOutParam);

__declspec(dllexport) INT __stdcall SoundCardInit(UINT32 uSamplerate, SoundCardCallBack pCallBack, UINT32 uFlag);
__declspec(dllexport) INT __stdcall SoundCardClose();

/*
从声卡缓冲读取nOutBufferSize长度数据，保存到pOutBuffer中，如果声卡缓冲数据小于nOutBufferSize，不返回取任何数据。pnReadBytes为实际读取的数据长度。
*/
__declspec(dllexport) INT __stdcall SoundCardReadFrom(char* pOutBuffer, UINT32 uOutBufferSize, PUINT32 puReadBytes);
__declspec(dllexport) INT __stdcall SoundCardWriteTo(const char* pInBuffer, UINT32 uInBufferLength);

