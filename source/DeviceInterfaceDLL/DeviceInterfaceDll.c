/*
DLL˵����

�˶�̬������Ϊ������Ƶ�豸�Ľӿڶ�̬�⣬DLL���豸�����TCP�����ӷ�ʽ���ֳ����ӡ�
��ǰ��̬������ǻ��ھ������ڣ��������������ݷ�����ͬһ���Եļ�Ӧ�á�
SR_Init�󴴽�tcp �����̡߳��豸���ÿͻ���ģʽ���������Ӷ�̬�⡣
DLL��TCP��������ص�IO���������ģʽ����֧�ִ����͸�Ч�Ŀͻ������ӡ�
ÿ���豸���Ӷ�Ӧһ��DEVICE_ITEM�豸�ṹ�����нṹ�õ���������֯��������ʹ�ù�ϣ���ķ�ʽ���ڴ����豸����ʱ�ṩ���ٵĲ�ѯ��
JSON�����л��ͷ����л����ÿ�Դ��cJson��
��̬��ӿں������̰߳�ȫ���룬��Ҫ�ڶ���߳���ͬʱ���ýӿں�����
�汾��v1.0
*/
//#include <stdlib.h>
//#include <string.h>
//#include <stdbool.h>
#include <windows.h>
#include <stdio.h>
#include "device.h"
#include "DeviceInterfaceDll.h"
#include "command.h"
#include "cJSON.h"
#include "list.h"
#include "rtp.h"
#include "device.h"
#include "log.h"
#include "../common/g722/g722.h"

#define Version 0x01010001
typedef struct
{
	UINT Handle;
	UINT UserID;	//
	SOCKET udpsock;//ͨѶ�õ�socket
	UINT PeerIp;
	UINT PeerPort;
	RTP rtp;
	g722_state Encoder;//���ݽ����õĽ�����
	g722_state Decoder;//���ݱ����õı�����
	char  RemainData[640];//δ�ܴ���������
	int   RemainDataSize;//δ�ܴ��������ݳ���
}UDP_CONNECT_HANDLE, *PUDP_CONNECT_HANDLE;

extern INT TcpServerStart(UINT32 port);

DWORD gLastError = 0;
fExceptionCallBack gfExceptionCallBack = NULL;
CRITICAL_SECTION gCriticalSection = {NULL};//ȫ�ֱ����ٽ籣��������

BOOL _stdcall SR_Init(UINT32 ListenPort)
{
	UINT32 Ret;
	WSADATA wsaData;

	LogCreate();
	Log("startup");
	if ((Ret = WSAStartup((2, 2), &wsaData)) != 0)
	{
		WSACleanup();
		LogErrWithCode("WSAStartup false",Ret);
		return FALSE;
	}
	InitializeCriticalSection(&gCriticalSection);
	return (0 == TcpServerStart(ListenPort));
}

BOOL _stdcall SR_Cleanup()
{
	TcpServerStop();
	//EnterCriticalSection(&gCriticalSection);
	DeviceRemoveAll();
	//LeaveCriticalSection(&gCriticalSection);
	WSACleanup();
	Log("shutdown");
	LogDestroy();

	DeleteCriticalSection(&gCriticalSection);
	return TRUE;
}

UINT32 _stdcall SR_GetLastError()
{
	return gLastError;
}

UINT32 _stdcall SR_GetVersion()
{
	return Version;
}

/*
��¼��ʽ1��������Ϊÿ���豸����һ��SR_Login������pLoginInfo�е�sDeviceAddress���ϵͳֻ�����Ѿ�ע��ĵ�ַ��¼��
����ֵ��-1���ڴ�������-2����ip��ַ�Ѿ�����
>0:ע��ɹ��������û�ID
*/
UINT32 _stdcall SR_Login(LPSR_USER_LOGIN_INFO pLoginInfo, LPSR_DEVICEINFO lpDeviceInfo)
{
	LPSR_DEVICE_ITEM d = DeviceFindByAddr(pLoginInfo->sDeviceAddress);
	if (d == NULL)
	{
		LPSR_DEVICE_ITEM d = DeviceAdd(pLoginInfo, lpDeviceInfo);
		if (d != NULL)
			return d->nUserID;
		else
		{ 
			return -2;
		}
	}
	else
	{
		return -1;
	}
}

UINT32 _stdcall SR_Logout(UINT32 lUserID)
{
	int ret = DeviceRemove(lUserID);
	return ret;
}

UINT32 _stdcall SR_GetCapacity(UINT32 lUserID, PCHAR lpJsonBuffer, UINT32 nJsonBufferSize, UINT32 *nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int ret;
	UINT32 tol=0;
	UINT32 fre=0;
	cJSON* jsonroot = cJSON_CreateObject();
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	ret = DeviceGetDiskInfo(d, &tol, &fre);
	if (ret != 0)return ret;
	cJSON_AddNumberToObject(jsonroot, "totalCapacity", tol);
	cJSON_AddNumberToObject(jsonroot, "surplusCapacity", fre);
	
	//cJSON* files = cJSON_CreateArray();//audioFileList
	cJSON* audioFileList = cJSON_AddArrayToObject(jsonroot, "audioFileList");
	int count = 0;
	while (TRUE)
	{
		CHAR sFileName[256];
		UINT32 nFileSize;
		if (count == 0)
		{
			ret = DeviceGetFirstFile(d, sFileName, sizeof(sFileName), &nFileSize);
			count++;
		}
		else
		{
			ret = DeviceGetNextFile(d, sFileName, sizeof(sFileName), &nFileSize);
			count++;
		}
		if ((ret == RC_OK)&&(sFileName[0]))
		{
			cJSON* item = cJSON_CreateObject();
			cJSON_AddStringToObject(item, "fileName", sFileName);
			cJSON_AddNumberToObject(item, "fileSize", nFileSize);

			cJSON_AddItemToArray(audioFileList, item);
		}
		else
		{
			break;
		}
	}

	if (ret == RC_OK)
	{
		PCHAR jsonout = cJSON_Print(jsonroot);
		UINT32 jsonlength = strlen(jsonout);
		if(jsonlength < nJsonBufferSize)
		{
			*nRecvBytes = jsonlength;
			strncpy_s(lpJsonBuffer, nJsonBufferSize, jsonout, jsonlength);
		}
		else
		{
			*nRecvBytes = jsonlength;
			ret = RC_MEMERY_LACK;
		}
		free(jsonout);
	}
	cJSON_Delete(jsonroot);
	return ret;
}

//ɾ��sd���ڵ��ļ�
UINT32 _stdcall SR_DeleteFile(UINT32 lUserID, CONST PCHAR sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	return DeviceDeleteFile(d, sFileName);
}

//��ʼ����sd�����ļ�
UINT32 _stdcall SR_PutFile(UINT32 lUserID, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	result = DeviceSDCardPlayFileStart(d, (CHAR*)sFileName, 97);
	return result;
}

//��ȡ����sd���ļ��Ľ�����Ϣ
UINT32 _stdcall SR_PutFile_Status(UINT32 lUserID, UINT32* nProcess, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	int runtime, process;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	result = DeviceSDCardPlayFileGetStatus(d, &runtime, &process);
	if (result == 0)
	{
		*nProcess = process;
	}
	return result;
}

//��ֹsd�ļ�����
UINT32 _stdcall SR_PutFile_Stop(UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	result = DeviceSDCardPlayFileStop(d);
	return result;
}
/*
UINT32 TcpListen(SOCKET *sock, UINT32* port)
{
	SOCKET listen_sock;
	struct sockaddr_in local_addr;
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		gLastError = GetLastError();
		return RC_SOCKET_ERROR;
	}
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));
	if (bind(listen_sock, (struct sockaddr*) & local_addr, sizeof(struct sockaddr)) == -1)
	{
		closesocket(listen_sock);
		return RC_SOCKET_ERROR;
	}
	int tmpint = sizeof(local_addr);
	getsockname(listen_sock, (struct sockaddr*) & local_addr, (int*)& tmpint);
	if (listen(listen_sock, 1) == -1)
	{
		closesocket(listen_sock);
		return RC_SOCKET_ERROR;
	}
	*port = htons(local_addr.sin_port);
	*sock = listen_sock;
	return RC_OK;
}

UINT32 TcpAccept(UINT32 nPeerIp, SOCKET sock, SOCKET* accept_sock)
{
	int result = RC_UNKNOWN;
	fd_set fr;
	struct timeval tv;
	FD_ZERO(&fr);
	FD_SET(sock, &fr);
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	int ret2 = select(sock + 1, &fr, NULL, NULL, &tv);
	if (ret2 > 0)
	{
		struct sockaddr_in remote_addr;
		int tmp = sizeof(struct sockaddr_in);
		if (FD_ISSET(sock, &fr))
		{
			sock = accept(sock, (struct sockaddr*) & remote_addr, &tmp);
			if (remote_addr.sin_addr.S_un.S_addr == nPeerIp)
			{
				//UDP_CONNECT_HANDLE* p = malloc(sizeof(UDP_CONNECT_HANDLE));
				//*lUploadHandle = sock;
				*accept_sock = sock;
				result = RC_OK;
			}
		}
		else
		{
			result = RC_UNKNOWN;
		}
	}
	else
	{
		result = RC_TIMEOUT;
	}
	return result;
		
}
*/
//���ȴ���һ������socket��Ȼ����������豸��ʹ�豸���ӵ��˶˿ڣ��������ӵ�socket��Ϊhandle����Ӧ�ó��򡣼���socket�ں����˳�ǰ�رա�
//������ж����ӵ��˶˿ڵ��豸ip��ַ��Ԥ��һ��������Ϊ����Ч���ӡ�
//��ǰ�豸��֧��Ӣ��������ɵ��ļ�����֧�ֳ��ļ�����
//SR_UploadFile_V40,SR_Upload_Process,SR_UploadClose ��������ͬ����ļ��ϴ������ܡ�
/*
UINT32 _stdcall SR_UploadFile_V40(UINT32* lUploadHandle, UINT32 lUserID, CONST CHAR* sFileName, UINT32 dwUploadType, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	SOCKET listen_sock;
	SOCKET sock;
	UINT32 port = 0;
	//struct sockaddr_in local_addr;

	int ret = TcpListen(&listen_sock, &port);
	if (ret == RC_OK)
	{
		//�Ӿ���·����ȡ�ļ�����
		char fname[256];
		char* p = strrchr(sFileName, '\\');
		if (p != NULL)p = p + 1;
		else p = sFileName;
		strncpy_s(fname, sizeof(fname), p, strlen(p));

		int ret2;
		if (dwUploadType == UPLOAD_AUDIO_FILE_STORAGE)
			ret2 = DeviceUploadFileStart(d, port, fname, bCover);
		else if (dwUploadType == UPLOAD_AUDIO_FILE_RELEASE)
			ret2 = DevicePlayFileStart(d, port, fname, 97);
		else
			ret2 = -1;

		if (ret2 == RC_OK)
		{
			int ret3 = TcpAccept(d->nPeerIp, listen_sock, &sock);
			if (ret3 == RC_OK)
			{
				*lUploadHandle = sock;
			}
			result = ret3;
		}
		else
		{
			result = ret2;
		}
		closesocket(listen_sock);
	}
	else
	{
		result = ret;
	}
	return result;
}


*/
UINT32 _stdcall SR_UploadFile_V40(UINT32* lUploadHandle, UINT32 lUserID, CONST CHAR* sFileName, UINT32 dwUploadType, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	SOCKET listen_sock;
	SOCKET sock;
	struct sockaddr_in local_addr;
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		gLastError = GetLastError();
		return RC_SOCKET_ERROR;
	}
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));
	if (bind(listen_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
	{
		closesocket(listen_sock);
		return RC_SOCKET_ERROR;
	}
	int tmpint = sizeof(local_addr);
	getsockname(listen_sock, (struct sockaddr *)&local_addr, (int *)&tmpint);
	if (listen(listen_sock, 1) == -1)
	{
		closesocket(listen_sock);
		return RC_SOCKET_ERROR;
	}

	//�Ӿ���·����ȡ�ļ�����
	char fname[256];
	char* p = strrchr(sFileName, '\\');
	if (p != NULL)p = p + 1;
	else p = sFileName;
	strncpy_s(fname, sizeof(fname), p, strlen(p));

	int ret;
	if (dwUploadType == UPLOAD_AUDIO_FILE_STORAGE)
		ret = DeviceUploadFileStart(d, htons(local_addr.sin_port), fname, bCover);
	else if(dwUploadType == UPLOAD_AUDIO_FILE_RELEASE)
		ret = DevicePlayFileStart(d, htons(local_addr.sin_port), fname, 97);
	else
	{
		ret = -1;
	}

	if (ret == 0)
	{
		fd_set fr;
		struct timeval tv;
		FD_ZERO(&fr);
		FD_SET(listen_sock, &fr);
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		int ret2 = select(listen_sock + 1, &fr, NULL, NULL, &tv);
		if (ret2 > 0)
		{
			struct sockaddr_in remote_addr;
			int tmp = sizeof(struct sockaddr_in);
			if (FD_ISSET(listen_sock, &fr))
			{
				sock = accept(listen_sock, (struct sockaddr*)&remote_addr, &tmp);
				if (remote_addr.sin_addr.S_un.S_addr == d->nPeerIp)
				{
					//UDP_CONNECT_HANDLE* p = malloc(sizeof(UDP_CONNECT_HANDLE));
					
					*lUploadHandle = sock;
					result = RC_OK;
				}
			}
			else
			{
				result = RC_UNKNOWN;
			}
		}
		else
		{
			result = RC_NO_CLIENT_CONNECT;
		}
	}
	else
	{
		result = RC_TIMEOUT;
	}
		

	closesocket(listen_sock);
	return result;
}

UINT32 _stdcall SR_Upload_Process(UINT32 lUploadHandle, PCHAR lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	SOCKET sock = lUploadHandle;
	fd_set fw;
	FD_ZERO(&fw);
	FD_SET(sock, &fw);
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	int ret = select(sock, NULL, &fw, NULL, &tv);
	if (ret > 0)
	{
		if (FD_ISSET(sock, &fw))
		{
			send(sock, lpInBuffer, BufferSize, 0);
			return RC_OK;
		}
	}
	return RC_SOCKET_ERROR;
}

UINT32 _stdcall SR_UploadClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	SOCKET sock = lUploadHandle;
	closesocket(sock);
	return 0;
}

typedef struct
{
	UINT Handle;
	//UINT UserID;	//
	SOCKET udpsock;//ͨѶ�õ�socket
	UINT LocalPort;
	UINT PeerIp;
	UINT PeerPort;
	RTP rtp;
	g722_state Encoder;//���ݽ����õĽ�����
	g722_state Decoder;//���ݱ����õı�����
	char  RemainData[640];//δ�ܴ���������
	int   RemainDataSize;//δ�ܴ��������ݳ���
}RTP_SESSION,*LPRTP_SESSION;

static List gRtpSessionList = NULL;

#define INTERCOM 1
#define EMERGENCY_PLAY 2

INT RtpSessionCreate(LPRTP_SESSION* rtpsession)
{
	LPRTP_SESSION session = NULL;
	SOCKET udpsock;
	struct sockaddr_in local_addr;
	if ((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		gLastError = GetLastError();
		return RC_SOCKET_ERROR;
	}
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));
	if (bind(udpsock, (struct sockaddr*) & local_addr, sizeof(struct sockaddr)) == -1)
	{
		closesocket(udpsock);
		return RC_SOCKET_ERROR;
	}

	int tmpint = sizeof(local_addr);
	getsockname(udpsock, (struct sockaddr*) & local_addr, (int*)& tmpint);

	session = (LPRTP_SESSION)malloc(sizeof(RTP_SESSION));
	if (session != NULL)
	{
		memset(session, 0, sizeof(RTP_SESSION));
		EnterCriticalSection(&gCriticalSection);
		//LeaveCriticalSection(&gCriticalSection);
		if (gRtpSessionList == NULL)
		{
			session->Handle = 1;
		}
		else
		{
			session->Handle = ((LPRTP_SESSION)gRtpSessionList->data)->Handle + 1;
		}
		LeaveCriticalSection(&gCriticalSection);

		session->LocalPort = htons(local_addr.sin_port);
		session->udpsock = udpsock;
		session->RemainDataSize = 0;

		rtp_init(&session->rtp, PAYLOAD_G722);
		g722_reset_decoder(&session->Decoder);
		g722_reset_encoder(&session->Encoder);
		*rtpsession = session;
		return 0;
	}
	return RC_ERROR;

}

INT RtpSessionFind(LPRTP_SESSION* session, UINT32 Handle)
{
	List p = NULL;
	EnterCriticalSection(&gCriticalSection);
	//LeaveCriticalSection(&gCriticalSection);
	List ls = gRtpSessionList;
	while (ls != NULL)
	{
		LPRTP_SESSION ps = ls->data;
		if (ps->Handle == Handle)
		{
			p = ls;
			*session = p->data;
			LeaveCriticalSection(&gCriticalSection);
			return RC_OK;
		}
		else
		{
			ls = ls->next;
		}
	}
	LeaveCriticalSection(&gCriticalSection);
	*session = NULL;
	return RC_ERROR;
}
INT RtpSessionDestroy(LPRTP_SESSION session)
{
	closesocket(session->udpsock);
	List p = NULL;
	EnterCriticalSection(&gCriticalSection);
	//LeaveCriticalSection(&gCriticalSection);
	List ls = gRtpSessionList;
	while (ls != NULL)
	{
		if (ls->data == session)
		{
			p = ls;
			break;
		}
	}
	if (p != NULL)
	{
		gRtpSessionList = ListRemove(gRtpSessionList, p);
	}

	free(session);
	LeaveCriticalSection(&gCriticalSection);
	return RC_OK;
}

INT UdpRecv(SOCKET sock, char* pBuffer, int nBufferSize, int* nRecvBytes, UINT32* pRemoteIp, UINT32* pRemotePort)
{
	//char rbuf[1024];
	fd_set fread;
	struct timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	FD_ZERO(&fread);
	FD_SET(sock, &fread);

	if (select(sock, NULL, &fread, NULL, &tv) > 0)
	{
		if (FD_ISSET(sock, &fread))
		{
			struct sockaddr_in remote_addr;
			int tmp = sizeof(struct sockaddr_in);
			int rlen = recvfrom(sock, pBuffer, nBufferSize, 0, (struct sockaddr*) & remote_addr, &tmp);
			if (rlen > 0)
			{
				*pRemoteIp = remote_addr.sin_addr.S_un.S_addr;
				*pRemotePort = htons(remote_addr.sin_port);
				*nRecvBytes = rlen;
				return 0;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}
/*
//����һ��rtp�����񣬲���g722����
UINT32 _stdcall RtpSessionStart(UINT32* Handle, LPSR_DEVICE_ITEM d, UINT32 Command, LPVOID pInputParam)
{
	int result = -1;
	SOCKET udpsock;
	struct sockaddr_in local_addr;
	if ((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		gLastError = GetLastError();
		return RC_SOCKET_ERROR;
	}
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));
	if (bind(udpsock, (struct sockaddr*) & local_addr, sizeof(struct sockaddr)) == -1)
	{
		closesocket(udpsock);
		return RC_SOCKET_ERROR;
	}

	int tmpint = sizeof(local_addr);
	getsockname(udpsock, (struct sockaddr*) & local_addr, (int*)& tmpint);
	//INT DeviceIntercomStart(LPSR_DEVICE_ITEM d, PCHAR sTargetAddr,UINT32 nTargetPort, PCHAR sStreamType, PCHAR sProtocol, INT nInputGain, PCHAR sInputSource, INT nVolume, PCHAR sAecMode)
	char session[32];
	sprintf_s(session, sizeof(session), "%u", d->uID);
	int ret = -1;
	if (Command == INTERCOM)
	{
		ret = DeviceIntercomStart(d, NULL, htons(local_addr.sin_port), STREAMTYPE_G722, PROTOCOL_RTP, 20, INPUTSOURCE_MIC, 97, AECMODE_DISABLE, session);
	}
	else if (Command == EMERGENCY_PLAY)
	{
		ret = DeviceEmergencyPlayFileStart(d, NULL, htons(local_addr.sin_port), STREAMTYPE_G722, PROTOCOL_RTP, 97, session);
	}
	else
	{
		ret = -1;
	}
	if (ret == 0)
	{
		char rbuf[1024];
		fd_set fread;
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		FD_ZERO(&fread);
		FD_SET(udpsock, &fread);

		if (select(udpsock, NULL, &fread, NULL, &tv) > 0)
		{
			if (FD_ISSET(udpsock, &fread))
			{
				struct sockaddr_in remote_addr;
				int tmp = sizeof(struct sockaddr_in);
				int rlen = recvfrom(udpsock, rbuf, sizeof(rbuf), 0, (struct sockaddr*) & remote_addr, &tmp);
				//if((rlen > 8)&&(strstr(session,rbuf) != NULL))
				if ((rlen > 0) && (remote_addr.sin_addr.s_addr == d->nPeerIp))
				{
					PUDP_CONNECT_HANDLE pv = (PUDP_CONNECT_HANDLE)malloc(sizeof(UDP_CONNECT_HANDLE));
					if (pv != NULL)
					{
						memset(pv, 0, sizeof(UDP_CONNECT_HANDLE));
						pv->PeerIp = remote_addr.sin_addr.S_un.S_addr;
						pv->PeerPort = htons(remote_addr.sin_port);
						if (gRtpHandleList == NULL)
						{
							pv->Handle = 1;
						}
						else
						{
							pv->Handle = ((PUDP_CONNECT_HANDLE)gRtpHandleList->data)->Handle + 1;
						}
						rtp_init(&pv->rtp, PAYLOAD_G722);

						pv->UserID = d->nUserID;
						pv->udpsock = udpsock;
						pv->RemainDataSize = 0;
						g722_reset_decoder(&pv->Decoder);
						g722_reset_encoder(&pv->Encoder);
						*Handle = pv->Handle;
						gRtpHandleList = ListAdd(gRtpHandleList, pv);
						result = RC_OK;
					}
				}
				else result = RC_UNKNOWN;
			}
		}
	}
	else result = RC_TIMEOUT;

	if (result != RC_OK)closesocket(udpsock);

	return result;
}
*/
UINT32 _stdcall RtpSessionData(LPRTP_SESSION session, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes)
{
	if (session == NULL)return RC_INVALID_HANDLE;

	unsigned int readpos = 0;//�������ݵĶ�����
	while (readpos < nSendBufferSize)
	{
		if (session->RemainDataSize == 0)
		{
			if ((nSendBufferSize - readpos) >= 640)//��ȡ���ݴ����rtp֡
			{
				struct sockaddr_in remote_addr;
				char sbuf[1024];
				char outcode[1024];
				int bytes = g722_encode((short*)& pSendDataBuffer[readpos], outcode, 320, (g722_state*)& session->Encoder);

				if (session->rtp.rtp_snd_seq == 0)
				{
					session->rtp.rtp_starttime = GetTickCount();
				}
				int slen = rtp_send_buf(&session->rtp, sbuf, outcode, bytes);

				remote_addr.sin_family = AF_INET;
				remote_addr.sin_addr.S_un.S_addr = session->PeerIp;
				remote_addr.sin_port = htons(session->PeerPort);

				if (1)//block mode �򵥣�����ȷ����ʱ���룬���Ż���
				{
					UINT32 tick = GetTickCount() - session->rtp.rtp_starttime + 200;//+200ms����Ϊ�����豸��200ms��������档�������Լ���rtp������������Ƶ���Ŷ�����
					if (tick < (session->rtp.rtp_timestamp / 8))
					{
						UINT32 waittime = (session->rtp.rtp_timestamp / 8) - tick;
						Sleep(waittime);
					}
				}
				sendto(session->udpsock, sbuf, slen, 0, (struct sockaddr*) & remote_addr, sizeof(remote_addr));

				readpos += 640;
			}
			else//�����ݿ�������ʱ����
			{
				int size = nSendBufferSize - readpos;
				memcpy(session->RemainData, &pSendDataBuffer[readpos], size);
				session->RemainDataSize = size;
				readpos = nSendBufferSize;
			}
		}
		else
		{
			unsigned int size = 640 - session->RemainDataSize;
			if ((nSendBufferSize - readpos) < size)size = nSendBufferSize - readpos;

			memcpy(&session->RemainData[session->RemainDataSize], &pSendDataBuffer[readpos], size);
			readpos += size;
			session->RemainDataSize += size;

			if (session->RemainDataSize >= 640)
			{
				struct sockaddr_in remote_addr;
				char sbuf[1024];
				char outcode[1024];
				int nSamples = g722_encode((short*)session->RemainData, outcode, 320, (g722_state*)& session->Encoder);
				int slen = rtp_send_buf(&session->rtp, sbuf, outcode, nSamples);

				remote_addr.sin_family = AF_INET;
				remote_addr.sin_addr.S_un.S_addr = session->PeerIp;
				remote_addr.sin_port = htons(session->PeerPort);

				sendto(session->udpsock, sbuf, slen, 0, (struct sockaddr*) & remote_addr, sizeof(remote_addr));

				session->RemainDataSize = 0;
			}
		}
	}

	//����ʹ��0�ȴ�ʱ�䣬���udp socket�Ƿ������ݣ��������󿽱���Ӧ�ó���
	unsigned int writepos = 0;
	char recvbuf[2048];
	//struct sockaddr_in remote_addr;
	int tmp = sizeof(struct sockaddr_in);

	while (writepos < nRecvBufferSize)
	{
		fd_set fread;
		struct timeval tv;
		struct sockaddr_in remote_addr;
		int tmp = sizeof(struct sockaddr_in);
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		FD_ZERO(&fread);
		FD_SET(session->udpsock, &fread);
		int ret = select(session->udpsock + 1, &fread, NULL, NULL, &tv);
		if (ret > 0)
		{
			if (FD_ISSET(session->udpsock, &fread))
			{
				int rlen = recvfrom(session->udpsock, recvbuf, 2048, 0, (struct sockaddr*)&remote_addr, &tmp);
				if ((rlen > 0) && (session->PeerIp == remote_addr.sin_addr.S_un.S_addr))
				{
					//���ݽ���
					char* payload;
					int payload_size;
					short pcmbuf[1024];
					//����
					int payload_type = rtp_parse(&session->rtp, recvbuf, rlen, &payload, &payload_size);
					if (payload_type == session->rtp.payload)
					{
						if (payload_size < sizeof(pcmbuf) / 4)//ȷ�����㹻�Ļ��屣�ֽ��������ݡ�
						{
							int nSamples = g722_decode((char*)payload, pcmbuf, 1, payload_size, (g722_state*)& session->Decoder);
							int size = nRecvBufferSize - writepos;
							if (size > (2 * nSamples))size = (2 * nSamples);
							memcpy(&pRecvDataBuffer[writepos], pcmbuf, size);
							writepos += size;
						}
					}
				}
			}
			//if (writepos >= nRecvBufferSize)break;
		}
		else break;//û�����������˳�whileѭ��

	}
	*nRecvBytes = writepos;

	return writepos;
}
/*
UINT32 _stdcall RtpSessionStop(UINT32 Handle, UINT32 Command)
{
	List p = NULL;
	List ls = gRtpHandleList;
	while (ls != NULL)
	{
		PUDP_CONNECT_HANDLE pv = ls->data;
		if (pv->Handle == Handle)
		{
			p = ls;
			break;
		}
		else
		{
			ls = ls->next;
		}
	}

	if (p != NULL)
	{
		PUDP_CONNECT_HANDLE pv = p->data;
		LPSR_DEVICE_ITEM d = DeviceFind(pv->UserID);
		if (d != NULL)
		{
			if (Command == INTERCOM)
			{
				DeviceIntercomStop(d);
			}
			else if (Command == EMERGENCY_PLAY)
			{
				DeviceEmergencyPlayFileStop(d);
			}
			
		}
		closesocket(pv->udpsock);
		gRtpHandleList = ListRemove(gRtpHandleList, p);
	}
	return 0;
}*/

//�����豸��ʹ֮�뱾���Խ����Ự����
//�Ựʹ�豸����recvsendģʽ��ʹ��rtp����Э�飬g722����ѹ��
//SR_StartVoiceCom_V30����һ��udp�������ú����������豸���ӵ���socket�ϣ���ɺ󣬴���һ��VOICE_COM_HANDLE�ṹ�������˽ṹ��ID���ظ�Ӧ�ó���
//Ӧ�ó������ʹ�ô˾��������SR_VoiceCom_Data���豸��ȡ��д����Ƶ����(16K,��������16bit��PCM)
//ʹ��SR_StopVoiceCom�����Ự
//SR_StartVoiceCom_V30��SR_VoiceCom_Data��SR_StopVoiceCom��ͬʹ����ɻỰ���ܡ�
UINT32 _stdcall SR_StartVoiceCom_V30(UINT32* iVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	LPRTP_SESSION session;
	int ret = RtpSessionCreate(&session);
	if (ret == 0)
	{
		//session->UserID = d->nUserID;
		char sessionid[32];
		sprintf_s(sessionid, sizeof(sessionid), "%u", d->uID);
		ret = DeviceIntercomStart(d, NULL, session->LocalPort, STREAMTYPE_G722, PROTOCOL_RTP, 20, INPUTSOURCE_MIC, 97, AECMODE_DISABLE, sessionid);
		if (ret == RC_OK)
		{
			char buffer[1024];
			int  nRecvBytes;
			UINT32 RemoteIp;
			UINT32 RemotePort;
			int rlen = UdpRecv(session->udpsock, buffer, sizeof(buffer), &nRecvBytes, &RemoteIp, &RemotePort);
			if ((rlen == 0) && (nRecvBytes > 0) && (RemoteIp == d->nPeerIp))//�豸����
			{
				session->PeerIp = RemoteIp;
				session->PeerPort = RemotePort;
				EnterCriticalSection(&gCriticalSection);
				gRtpSessionList = ListAdd(gRtpSessionList, session);
				LeaveCriticalSection(&gCriticalSection);
				*iVoiceComHandle = session->Handle;
				return RC_OK;
			}
		}

		RtpSessionDestroy(session);
	}
	return RC_ERROR;
}

UINT32 _stdcall SR_VoiceCom_Data(UINT32 iVoiceComHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPRTP_SESSION session;
	int ret = RtpSessionFind(&session, iVoiceComHandle);
	if ((ret != RC_OK) || (session == NULL))return RC_INVALID_HANDLE;

	int result = -1;
	UINT32 recvbytes = 0;
	result = RtpSessionData(session, pSendDataBuffer, nSendBufferSize, pRecvDataBuffer, nRecvBufferSize, &recvbytes);
	*nRecvBytes = recvbytes;
	return result;
}

UINT32 _stdcall SR_StopVoiceCom(UINT32 iVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	DeviceIntercomStop(d);

	LPRTP_SESSION session;
	int ret = RtpSessionFind(&session, iVoiceComHandle);
	if (session != NULL)RtpSessionDestroy(session);
}

UINT32 _stdcall SR_StartEmergency(UINT32* iEmergencyHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	LPRTP_SESSION session;
	int ret = RtpSessionCreate(&session);
	if (ret == 0)
	{
		//session->UserID = d->nUserID;
		char sessionid[32];
		sprintf_s(sessionid, sizeof(sessionid), "%u", d->uID);
		//ret = DeviceIntercomStart(d, NULL, session->LocalPort, STREAMTYPE_G722, PROTOCOL_RTP, 20, INPUTSOURCE_MIC, 97, AECMODE_DISABLE, sessionid);
		ret = DeviceEmergencyPlayFileStart(d, NULL, session->LocalPort, STREAMTYPE_G722, PROTOCOL_RTP, 97, sessionid);
		if (ret == RC_OK)
		{
			char buffer[1024];
			int  nRecvBytes;
			UINT32 RemoteIp;
			UINT32 RemotePort;
			int rlen = UdpRecv(session->udpsock, buffer, sizeof(buffer), &nRecvBytes, &RemoteIp, &RemotePort);
			if ((rlen == 0) && (nRecvBytes > 0) && (RemoteIp == d->nPeerIp))//�豸����
			{
				session->PeerIp = RemoteIp;
				session->PeerPort = RemotePort;
				EnterCriticalSection(&gCriticalSection);
				gRtpSessionList = ListAdd(gRtpSessionList, session);
				LeaveCriticalSection(&gCriticalSection);
				*iEmergencyHandle = session->Handle;
				return RC_OK;
			}
		}

		RtpSessionDestroy(session);
	}
	return RC_ERROR;
}

UINT32 _stdcall SR_Emergency_Data(UINT32 iEmergencyHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPRTP_SESSION session;
	int ret = RtpSessionFind(&session,iEmergencyHandle);
	if ((ret != RC_OK)||(session == NULL))return RC_INVALID_HANDLE;

	int result = -1;
	UINT32 recvbytes = 0;
	result = RtpSessionData(session, pSendDataBuffer, nSendBufferSize, NULL, 0, &recvbytes);
	*nRecvBytes = 0;
	return result;
	
}

UINT32 _stdcall SR_StopEmergency(UINT32 iEmergencyHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	DeviceEmergencyPlayFileStop(d);

	LPRTP_SESSION session;
	int ret = RtpSessionFind(&session, iEmergencyHandle);
	if(session != NULL)RtpSessionDestroy(session);
}
/*
UINT32 _stdcall SR_StartVoiceCom_V30(UINT32* iVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	SOCKET udpsock;
	struct sockaddr_in local_addr;
	if ((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		gLastError = GetLastError();
		return RC_SOCKET_ERROR;
	}
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = 0;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));
	if (bind(udpsock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
	{
		closesocket(udpsock);
		return RC_SOCKET_ERROR;
	}
	
	
	int tmpint = sizeof(local_addr);
	getsockname(udpsock, (struct sockaddr *)&local_addr, (int *)&tmpint);
	//INT DeviceIntercomStart(LPSR_DEVICE_ITEM d, PCHAR sTargetAddr,UINT32 nTargetPort, PCHAR sStreamType, PCHAR sProtocol, INT nInputGain, PCHAR sInputSource, INT nVolume, PCHAR sAecMode)
	char session[32];
	sprintf_s(session,sizeof(session),"%u",d->uID);
	int ret = DeviceIntercomStart(d,NULL,htons(local_addr.sin_port),STREAMTYPE_G722,PROTOCOL_RTP,20,INPUTSOURCE_MIC,97,AECMODE_DISABLE,session);
	if(ret == 0)
	{
		char rbuf[1024];
		fd_set fread;
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		FD_ZERO(&fread);
		FD_SET(udpsock, &fread);
		
		if(select(udpsock, NULL, &fread, NULL, &tv) > 0)
		{
			if (FD_ISSET(udpsock, &fread))
			{
				struct sockaddr_in remote_addr;
				int tmp = sizeof(struct sockaddr_in);
				int rlen = recvfrom(udpsock,rbuf,sizeof(rbuf),0,(struct sockaddr*)&remote_addr,&tmp);
				//if((rlen > 8)&&(strstr(session,rbuf) != NULL))
				if((rlen > 0)&&(remote_addr.sin_addr.s_addr == d->nPeerIp))
				{
					PUDP_CONNECT_HANDLE pv = (PUDP_CONNECT_HANDLE)malloc(sizeof(UDP_CONNECT_HANDLE));
					if (pv != NULL)
					{
						memset(pv, 0, sizeof(UDP_CONNECT_HANDLE));
						pv->PeerIp = remote_addr.sin_addr.S_un.S_addr;
						pv->PeerPort = htons(remote_addr.sin_port);
						if (gVoiceHandleList == NULL)
						{
							pv->Handle = 1;
						}
						else
						{
							pv->Handle = ((PUDP_CONNECT_HANDLE)gVoiceHandleList->data)->Handle + 1;
						}
						rtp_init(&pv->rtp, PAYLOAD_G722);

						//��socket���óɷǶ���ģʽ
						//unsigned long blocked = 1;
						//if (ioctlsocket(udpsock, FIONBIO, &blocked) < 0)
						//{
							//closesocket(udpsock);
							//return RC_SOCKET_ERROR;
						//}
						pv->UserID = lUserID;
						pv->udpsock = udpsock;
						pv->RemainDataSize = 0;
						g722_reset_decoder(&pv->Decoder);
						g722_reset_encoder(&pv->Encoder);
						*iVoiceComHandle = pv->Handle;
						gVoiceHandleList = ListAdd(gVoiceHandleList, pv);
						result = RC_OK;
					}
					
				}
				else result = RC_UNKNOWN;
			}
		}
	}
	else result = RC_TIMEOUT;
	
	if(result != RC_OK)closesocket(udpsock);
	
	return result;
}

//���ͺͶ�ȡ�Ự����
//Ĭ��ʹ��g722���б����
//ÿ��g722��rtp֡����320��pcm��������Ϊ640���ֽڣ�����ύ�����������640�ֽڵ�������������ߺ���ִ��Ч�ʡ�
UINT32 _stdcall SR_VoiceCom_Data(UINT32 iVoiceComHandle, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	PUDP_CONNECT_HANDLE pVoice = NULL;
	List ls = gVoiceHandleList;
	while (ls != NULL)
	{
		PUDP_CONNECT_HANDLE pv = ls->data;
		if (pv->Handle == iVoiceComHandle)
		{
			pVoice = pv;
			break;
		}
		else
		{
			ls = ls->next;
		}
	}
	if (pVoice == NULL)return RC_INVALID_HANDLE;

	unsigned int readpos = 0;//�������ݵĶ�����
	while (readpos < nSendBufferSize)
	{
		if (pVoice->RemainDataSize == 0)
		{
			if ((nSendBufferSize - readpos) >= 640)//��ȡ���ݴ����rtp֡
			{
				struct sockaddr_in remote_addr;
				char sbuf[1024];
				char outcode[1024];
				int nSamples = g722_encode((short*)&pSendDataBuffer[readpos], outcode, 320, (g722_state*)&pVoice->Encoder);
				int slen = rtp_send_buf(&pVoice->rtp, sbuf, outcode, nSamples);

				remote_addr.sin_family = AF_INET;
				remote_addr.sin_addr.S_un.S_addr = pVoice->PeerIp;
				remote_addr.sin_port = htons(pVoice->PeerPort);

				sendto(pVoice->udpsock, sbuf, slen, 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));

				readpos += 640;
			}
			else//�����ݿ�������ʱ����
			{
				int size = nSendBufferSize - readpos;
				memcpy(pVoice->RemainData, &pSendDataBuffer[readpos], size);
				pVoice->RemainDataSize = size;
				readpos = nSendBufferSize;
			}
		}
		else
		{
			unsigned int size = 640 - pVoice->RemainDataSize;
			if ((nSendBufferSize - readpos) < size)size = nSendBufferSize - readpos;

			memcpy(&pVoice->RemainData[pVoice->RemainDataSize], &pSendDataBuffer[readpos], size);
			readpos += size;
			pVoice->RemainDataSize += size;

			if (pVoice->RemainDataSize >= 640)
			{
				struct sockaddr_in remote_addr;
				char sbuf[1024];
				char outcode[1024];
				int nSamples = g722_encode((short*)pVoice->RemainData, outcode, 320, (g722_state*)& pVoice->Encoder);
				int slen = rtp_send_buf(&pVoice->rtp, sbuf, outcode, nSamples);

				remote_addr.sin_family = AF_INET;
				remote_addr.sin_addr.S_un.S_addr = pVoice->PeerIp;
				remote_addr.sin_port = htons(pVoice->PeerPort);

				sendto(pVoice->udpsock, sbuf, slen, 0, (struct sockaddr*) & remote_addr, sizeof(remote_addr));

				pVoice->RemainDataSize = 0;
			}
		}
	}

	//����ʹ��0�ȴ�ʱ�䣬���udp socket�Ƿ������ݣ��������󿽱���Ӧ�ó���
	unsigned int writepos = 0;
	char recvbuf[2048];
	struct sockaddr_in remote_addr;
	int tmp = sizeof(struct sockaddr_in);

	
	//int rlen = recvfrom(pVoice->udpsock, recvbuf, 2048, 0, (struct sockaddr*)&remote_addr, &tmp);
	//while (rlen > 0)
	//{
		//���ݽ���
	//	char* payload;
	//	int payload_size;
	//	short pcmbuf[1024];
		//����
	//	int payload_type = rtp_parse(&pVoice->rtp, recvbuf, rlen, &payload, &payload_size);
	//	if (payload_type == pVoice->rtp.payload)
	//	{
	//		if (payload_size < sizeof(pcmbuf) / 4)//ȷ�����㹻�Ļ��屣�ֽ��������ݡ�
	//		{
	//			int nSamples = g722_decode((char*)payload, pcmbuf, 1, payload_size, (g722_state*)& pVoice->Decoder);
	//			int size = nRecvBufferSize - writepos;
	//			if (size > (2 * nSamples))size = (2 * nSamples);
	//			memcpy(&pRecvDataBuffer[writepos], pcmbuf, size);
	//			writepos += size;
	//		}
	//	}
	//	if (writepos >= nRecvBufferSize)break;

		//�ٴμ��socket�Ƿ������ݡ�
	//	rlen = recvfrom(pVoice->udpsock, recvbuf, 2048, 0, (struct sockaddr*)&remote_addr, &tmp);
	//}

	while (TRUE)
	{
		fd_set fread;
		struct timeval tv;
		struct sockaddr_in remote_addr;
		int tmp = sizeof(struct sockaddr_in);
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		FD_ZERO(&fread);
		FD_SET(pVoice->udpsock, &fread);
		int ret = select(pVoice->udpsock + 1, &fread, NULL, NULL, &tv);
		if (ret > 0)
		{
			if (FD_ISSET(pVoice->udpsock, &fread))
			{
				int rlen = recvfrom(pVoice->udpsock, recvbuf, 2048, 0, &remote_addr, &tmp);
				if ((rlen > 0) && (pVoice->PeerIp == remote_addr.sin_addr.S_un.S_addr))
				{
					//���ݽ���
					char* payload;
					int* payload_size;
					short pcmbuf[1024];
					//����
					int payload_type = rtp_parse(&pVoice->rtp, recvbuf, rlen, &payload, &payload_size);
					if (payload_type == pVoice->rtp.payload)
					{
						if (payload_size < sizeof(pcmbuf) / 4)//ȷ�����㹻�Ļ��屣�ֽ��������ݡ�
						{
							int nSamples = g722_decode((char*)payload, pcmbuf, 1, payload_size, (g722_state*)& pVoice->Decoder);
							int size = nRecvBufferSize - writepos;
							if (size > (2 * nSamples))size = (2 * nSamples);
							memcpy(&pRecvDataBuffer[writepos], pcmbuf, size);
							writepos += size;
						}
					}
				}
			}
			if (writepos >= nRecvBufferSize)break;
		}
		else break;//û�����������˳�whileѭ��
		
	}
	*nRecvBytes = writepos;

	return writepos;
}

//��ֹ�Ự
UINT32 _stdcall SR_StopVoiceCom(UINT32 iVoiceComHandle, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	List p = NULL;
	List ls = gVoiceHandleList;
	while (ls != NULL)
	{
		PUDP_CONNECT_HANDLE pv = ls->data;
		if (pv->Handle == iVoiceComHandle)
		{
			p = ls;
			break;
		}
		else
		{
			ls = ls->next;
		}
	}

	if (p != NULL)
	{
		PUDP_CONNECT_HANDLE pv = p->data;
		LPSR_DEVICE_ITEM d = DeviceFind(pv->UserID);
		if (d != NULL)
		{
			DeviceIntercomStop(d);
		}
		closesocket(pv->udpsock);
		ListRemove(gVoiceHandleList, p);
	}
	return 0;
}
*/
UINT32 _stdcall SR_SetExceptionCallBack_V30(fExceptionCallBack pCallBack)
{
	gfExceptionCallBack = pCallBack;
	return 0;
}