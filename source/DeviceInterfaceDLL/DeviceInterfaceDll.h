#pragma once
#include <windows.h>

typedef struct
{
	char 	sDeviceAddress[129];	// IPV4 & IPV6 
	BYTE 	byRes;
	WORD 	wPort; 	// 端口
	char 	sUserName[64];
	char 	sPassword[64];
	BYTE 	byRes2[128];
}SR_USER_LOGIN_INFO, *LPSR_USER_LOGIN_INFO;

typedef struct
{
	BYTE  sSerialNumber[48];  		// 序列号
	BYTE  bySingleStartDTalkChan; 	// 独立音轨接入的设备，起始接入通道号，从1开始；
	BYTE  bySingleDTalkChanNums;  	// 独立音轨接入的设备的通道总数，0 - 表示不支持；
	BYTE  byRes[128];
}SR_DEVICEINFO, *LPSR_DEVICEINFO;

enum MSG_TYPE
{
	MSGTYPE_NONE = 0,
	MSGTYPE_CONNECTED,
	MSGTYPE_DISCONNECTED,
	MSGTYPE_DEVICE_LOGIN,
	MSGTYPE_DEVICE_LOGOUT,
};

enum RESULT_CODE
{
	RC_UNKNOWN = -1,
	RC_OK = 0,
	RC_TIMEOUT,
	RC_INVALID_USER_HANDLE,
	RC_MEMERY_LACK,
	RC_SOCKET_ERROR,
	RC_NO_CLIENT_CONNECT,
	RC_ERROR,
	RC_INVALID_HANDLE,
};

enum UPLOAD_TYPE
{
	UPLOAD_AUDIO_FILE_STORAGE = 56,
	UPLOAD_AUDIO_FILE_RELEASE = 57,
	UPLOAD_UPDATA_FILE = 58,
};

typedef INT32(CALLBACK *fExceptionCallBack)(UINT32 dwType, UINT32 lUserID, UINT32 lHandle, LPVOID pUser);

__declspec(dllexport) BOOL _stdcall SR_Init(UINT32 Mode, UINT32 ListenPort);
__declspec(dllexport) BOOL _stdcall SR_Cleanup();
__declspec(dllexport) UINT32 _stdcall SR_GetLastError();
__declspec(dllexport) UINT32 _stdcall SR_GetVersion();

__declspec(dllexport) UINT32 _stdcall SR_Login(LPSR_USER_LOGIN_INFO pLoginInfo, LPSR_DEVICEINFO lpDeviceInfo);
__declspec(dllexport) UINT32 _stdcall SR_Logout(UINT32 lUserID);

__declspec(dllexport) UINT32 _stdcall SR_GetCapacity(UINT32 lUserID, CHAR* lpJsonBuffer, UINT32 nJsonBufferSize, UINT32 *nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_DeleteFile(UINT32 lUserID, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam);

__declspec(dllexport) UINT32 _stdcall SR_PutFile(UINT32 lUserID, CONST CHAR* sFileName, UINT32 nVolume, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_PutFileStatus(UINT32 lUserID, UINT32* Process, LPVOID lpInputParam, LPVOID lpOutputParam);;
__declspec(dllexport) UINT32 _stdcall SR_PutFileClose(UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);

__declspec(dllexport) UINT32 _stdcall SR_UploadFile(UINT32* lUploadHandle, UINT32 lUserIDconst, CONST CHAR* sFileName, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_UploadFileData(UINT32 lUploadHandle, CHAR* lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_UploadFileClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam);

__declspec(dllexport) UINT32 _stdcall SR_PlayFile(UINT32* lUploadHandle, UINT32 lUserIDconst, CONST CHAR* sFileName, UINT32 nVolume, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_PlayFileData(UINT32 lUploadHandle, CHAR* lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_PlayFileClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam);

__declspec(dllexport) UINT32 _stdcall SR_VoiceCom(UINT32* IVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_VoiceComData(UINT32 IVoiceComHandle, CHAR* pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_VoiceComClose(UINT32 iVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);

__declspec(dllexport) UINT32 _stdcall SR_Emergency(UINT32* iEmergencyHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_EmergencyData(UINT32 iEmergencyHandle, CHAR* pSendDataBuffer, UINT32 nSendBufferSize, CHAR* pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_EmergencyClose(UINT32 iEmergencyHandle, UINT32 lUserID,LPVOID lpInputParam, LPVOID lpOutputParam);

__declspec(dllexport) UINT32 _stdcall SR_SetExceptionCallBack(fExceptionCallBack pCallBack);

__declspec(dllexport) UINT32 _stdcall SR_SetVolume(UINT32 lUserID, UINT32 nVolume);

__declspec(dllexport) UINT32 _stdcall SR_Update(UINT32* lUpdateHandle, UINT32 lUserID, INT nMode, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_UpdateData(UINT32 lUpdateHandle, CHAR* lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_UpdateClose(UINT32 lUpdateHandle, LPVOID lpInputParam, LPVOID lpOutputParam);

__declspec(dllexport) UINT32 _stdcall SR_Apply(UINT32 lUserID);