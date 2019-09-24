#pragma once
#include <windows.h>

typedef struct
{
	char 	sDeviceAddress[129];	// IPV4 & IPV6 
	BYTE 	byRes;
	WORD 	wPort; 	// �˿�
	char 	sUserName[64];
	char 	sPassword[64];
	BYTE 	byRes2[128];
}SR_USER_LOGIN_INFO, *LPSR_USER_LOGIN_INFO;

typedef struct
{
	BYTE  sSerialNumber[48];  		// ���к�
	BYTE  bySingleStartDTalkChan; 	// �������������豸����ʼ����ͨ���ţ���1��ʼ��
	BYTE  bySingleDTalkChanNums;  	// �������������豸��ͨ��������0 - ��ʾ��֧�֣�
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

__declspec(dllexport) BOOL _stdcall SR_Init(UINT32 ListenPort);
__declspec(dllexport) BOOL _stdcall SR_Cleanup();
__declspec(dllexport) UINT32 _stdcall SR_GetLastError();
__declspec(dllexport) UINT32 _stdcall SR_GetVersion();
__declspec(dllexport) UINT32 _stdcall SR_Login(LPSR_USER_LOGIN_INFO pLoginInfo, LPSR_DEVICEINFO lpDeviceInfo);
__declspec(dllexport) UINT32 _stdcall SR_Logout(UINT32 lUserID);
__declspec(dllexport) UINT32 _stdcall SR_GetCapacity(UINT32 lUserID, PCHAR lpJsonBuffer, UINT32 nJsonBufferSize, UINT32 *nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_DeleteFile(UINT32 lUserID, CONST PCHAR sFileName, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_PutFile(UINT32 lUserID, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_PutFile_Status(UINT32 lUserID, UINT32* Process, LPVOID lpInputParam, LPVOID lpOutputParam);;
__declspec(dllexport) UINT32 _stdcall SR_PutFile_Stop(UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_UploadFile_V40(UINT32* lUploadHandle, UINT32 lUserIDconst, CONST CHAR* sFileName, UINT32 dwUploadType, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_Upload_Process(UINT32 lUploadHandle, PCHAR lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_UploadClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_StartVoiceCom_V30(UINT32* IVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_VoiceCom_Data(UINT32 IVoiceComHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_StopVoiceCom(UINT32 iVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_SetExceptionCallBack_V30(fExceptionCallBack pCallBack);

__declspec(dllexport) UINT32 _stdcall SR_StartEmergency(UINT32* iEmergencyHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_Emergency_Data(UINT32 iEmergencyHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam);
__declspec(dllexport) UINT32 _stdcall SR_StopEmergency(UINT32 iEmergencyHandle, UINT32 lUserID,LPVOID lpInputParam, LPVOID lpOutputParam);