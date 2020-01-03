/*
DLL说明：

此动态库是作为网络音频设备的接口动态库，DLL和设备间采用TCP长连接方式保持长连接。
当前动态库设计是基于局域网内，管理服务与数据服务在同一电脑的简单应用。
SR_Init后创建tcp 监听线程。设备采用客户端模式，主动连接动态库。
DLL的TCP服务采用重叠IO的完成例程模式，以支持大量和高效的客户端连接。
每个设备连接对应一个DEVICE_ITEM设备结构，所有结构用单向链表组织。（可以使用哈希表的方式，在大量设备连接时提供快速的查询）
JSON的序列化和反序列化采用开源的cJson库
版本：v0.1
*/

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
#include "TcpServer.h"
#include "../common/g722/g722.h"
//#include <WinSock2.h>

#define Version 0x00010002
typedef struct
{
	UINT Handle;
	UINT UserID;	//
	SOCKET udpsock;//通讯用的socket
	UINT PeerIp;
	UINT PeerPort;
	RTP rtp;
	g722_state Encoder;//数据解码用的解码器
	g722_state Decoder;//数据编码用的编码器
	char  RemainData[640];//未能处理的数据
	int   RemainDataSize;//未能处理的数据长度
}UDP_CONNECT_HANDLE, *PUDP_CONNECT_HANDLE;

extern INT TcpServerStart(UINT32 port);

DWORD gLastError = 0;
fExceptionCallBack gfExceptionCallBack = NULL;
CRITICAL_SECTION gCriticalSection = {NULL};//全局变量临界保护区变量

BOOL _stdcall SR_Init(UINT32 Mode, UINT32 ListenPort)
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
	DeviceRemoveAll();
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
登录方式1，调用者为每个设备调用一次SR_Login函数，pLoginInfo中的sDeviceAddress必填，系统只允许已经注册的地址登录。
返回值：-1：内存分配错误，-2：该ip地址已经存在
>0:注册成功，返回用户ID
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

UINT32 _stdcall SR_GetCapacity(UINT32 lUserID, CHAR* lpJsonBuffer, UINT32 nJsonBufferSize, UINT32 *nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)
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
		UINT32 jsonlength = (INT)strlen(jsonout);
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

//删除sd卡内的文件
UINT32 _stdcall SR_DeleteFile(UINT32 lUserID, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	return DeviceDeleteFile(d, (CONST CHAR*)sFileName);
}

//开始播放sd卡内文件
UINT32 _stdcall SR_PutFile(UINT32 lUserID, CONST CHAR* sFileName, UINT32 nVolume, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	if (nVolume > 100)nVolume = 100;
	result = DeviceSDCardPlayFileStart(d, (CHAR*)sFileName, nVolume);
	return result;
}

//获取播放sd卡文件的进度信息
UINT32 _stdcall SR_PutFileStatus(UINT32 lUserID, UINT32* nProcess, LPVOID lpInputParam, LPVOID lpOutputParam)
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

//终止sd文件播放
UINT32 _stdcall SR_PutFileClose(UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	result = DeviceSDCardPlayFileStop(d);
	return result;
}

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


UINT32 _stdcall SR_PlayFile(UINT32* lPlayHandle, UINT32 lUserID, CONST CHAR* sFileName, UINT32 nVolume, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = -1;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	SOCKET listen_sock;
	UINT32 port;
	UINT32 ret = TcpListen(&listen_sock, &port);
	if (ret != RC_OK)return ret;

	//从绝对路径获取文件名。
	char fname[256];
	char* p = strrchr(sFileName, '\\');
	if (p != NULL)p = p + 1;
	else p = sFileName;
	strncpy_s(fname, sizeof(fname), p, strlen(p));

	ret = DevicePlayFileStart(d, port, fname, nVolume);
	if (ret == RC_OK)
	{
		SOCKET accept_sock;
		ret = TcpAccept(d->nPeerIp, listen_sock, &accept_sock);
		if (ret == RC_OK)
		{
			*lPlayHandle = (UINT32)accept_sock;
			result = ret;
		}
	}

	closesocket(listen_sock);
	return ret;
}
UINT32 _stdcall SR_PlayFileData(UINT32 lPlayHandle, CHAR* lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	SOCKET sock = lPlayHandle;
	fd_set fw;
	FD_ZERO(&fw);
	FD_SET(sock, &fw);
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	int ret = select(sock+1, NULL, &fw, NULL, &tv);
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

UINT32 _stdcall SR_PlayFileClose(UINT32 lPlayHandle, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	SOCKET sock = lPlayHandle;
	closesocket(sock);
	return 0;
}

UINT32 _stdcall SR_UploadFile(UINT32* lUploadHandle, UINT32 lUserID, CONST CHAR* sFileName, BOOL bCover, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = RC_UNKNOWN;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	SOCKET listen_sock;
	UINT32 port;
	UINT32 ret = TcpListen(&listen_sock, &port);
	if (ret != RC_OK)return ret;

	//从绝对路径获取文件名。
	char fname[256];
	char* p = strrchr(sFileName, '\\');
	if (p != NULL)p = p + 1;
	else p = sFileName;
	strncpy_s(fname, sizeof(fname), p, strlen(p));

	ret = DeviceUploadFileStart(d, port, fname, bCover);
	if (ret == RC_OK)
	{
		SOCKET accept_sock;
		ret = TcpAccept(d->nPeerIp, listen_sock, &accept_sock);
		if (ret == RC_OK)
		{
			*lUploadHandle = (UINT32)accept_sock;
			result = ret;
		}
	}

	closesocket(listen_sock);
	return ret;
}

UINT32 _stdcall SR_UploadFileData(UINT32 lUploadHandle, CHAR* lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	SOCKET sock = lUploadHandle;
	fd_set fw;
	FD_ZERO(&fw);
	FD_SET(sock, &fw);
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	int ret = select(sock+1, NULL, &fw, NULL, &tv);
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

UINT32 _stdcall SR_UploadFileClose(UINT32 lUploadHandle, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	SOCKET sock = lUploadHandle;
	closesocket(sock);
	return 0;
}

UINT32 _stdcall SR_Update(UINT32* lUpdateHandle, UINT32 lUserID, INT nMode, CONST CHAR* sFileName, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	int result = RC_UNKNOWN;
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	SOCKET listen_sock;
	UINT32 port;
	UINT32 ret = TcpListen(&listen_sock, &port);
	if (ret != RC_OK)return ret;

	//从绝对路径获取文件名。
	char fname[256];
	char* p = strrchr(sFileName, '\\');
	if (p != NULL)p = p + 1;
	else p = sFileName;
	strncpy_s(fname, sizeof(fname), p, strlen(p));

	ret = DeviceUpdateStart(d, port, nMode, fname);
	if (ret == RC_OK)
	{
		SOCKET accept_sock;
		ret = TcpAccept(d->nPeerIp, listen_sock, &accept_sock);
		if (ret == RC_OK)
		{
			*lUpdateHandle = (UINT32)accept_sock;
			result = ret;
		}
	}

	closesocket(listen_sock);
	return ret;
}

UINT32 _stdcall SR_UpdateData(UINT32 lUpdateHandle, CHAR* lpInBuffer, UINT32 BufferSize, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	SOCKET sock = lUpdateHandle;
	fd_set fw;
	FD_ZERO(&fw);
	FD_SET(sock, &fw);
	struct timeval tv;
	tv.tv_sec = 3;
	tv.tv_usec = 0;
	int ret = select(sock + 1, NULL, &fw, NULL, &tv);
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

UINT32 _stdcall SR_UpdateClose(UINT32 lUpdateHandle, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	/*
	SOCKET sock = lUpdateHandle;
	closesocket(sock);
	//shutdown(sock, 2);
	return 0;
	*/
	char buf[2048];
	unsigned int timer = GetTickCount();
	SOCKET sock = lUpdateHandle;
	shutdown(sock, 1);
	fd_set fread;
	TIMEVAL tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	for (int i = 0; i < 10; i++)
	{
		FD_ZERO(&fread);
		FD_SET(sock, &fread);
		int ret = select(sock + 1, &fread, NULL, NULL, &tv);
		if (ret > 0)
		{
			if (FD_ISSET(sock, &fread))
			{
				int rlen = recv(sock, buf, sizeof(buf), 0);
				if (rlen <= 0)break;
			}
		}
	}

	closesocket(sock);
	timer = GetTickCount() - timer;
	if (timer > 0)
	{
		Sleep(1);
	}
	return 0;
}

typedef struct
{
	UINT Handle;
	//UINT UserID;	//
	SOCKET udpsock;//通讯用的socket
	UINT LocalPort;
	UINT PeerIp;
	UINT PeerPort;
	RTP rtp;
	g722_state Encoder;//数据解码用的解码器
	g722_state Decoder;//数据编码用的编码器
	char  RemainData[640];//未能处理的数据
	int   RemainDataSize;//未能处理的数据长度
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

	if (select(sock+1, NULL, &fread, NULL, &tv) > 0)
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

UINT32 _stdcall RtpSessionData(LPRTP_SESSION session, PCHAR pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes)
{
	if (session == NULL)return RC_INVALID_HANDLE;

	unsigned int readpos = 0;//输入数据的读索引
	while (readpos < nSendBufferSize)
	{
		if (session->RemainDataSize == 0)
		{
			if ((nSendBufferSize - readpos) >= 640)//读取数据打包成rtp帧
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

				if (1)//block mode 简单，不精确的延时代码，可优化。
				{
					UINT32 tick = GetTickCount() - session->rtp.rtp_starttime + 200;//+200ms是因为下面设备有200ms的输出缓存。这样可以减少rtp抖动带来的音频播放抖动。
					if (tick < (session->rtp.rtp_timestamp / 8))
					{
						UINT32 waittime = (session->rtp.rtp_timestamp / 8) - tick;
						Sleep(waittime);
					}
				}
				sendto(session->udpsock, sbuf, slen, 0, (struct sockaddr*) & remote_addr, sizeof(remote_addr));

				readpos += 640;
			}
			else//将数据拷贝到临时缓冲
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

	//这里使用0等待时间，检查udp socket是否有数据，有则解码后拷贝给应用程序。
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
					//数据解码
					char* payload;
					int payload_size;
					short pcmbuf[1024];
					//解码
					int payload_type = rtp_parse(&session->rtp, recvbuf, rlen, &payload, &payload_size);
					if (payload_type == session->rtp.payload)
					{
						if (payload_size < sizeof(pcmbuf) / 4)//确保有足够的缓冲保持解码后的数据。
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
		else break;//没有数据立即退出while循环

	}
	*nRecvBytes = writepos;

	return writepos;
}


//操作设备，使之与本电脑建立会话连接
//会话使设备进入recvsend模式，使用rtp传输协议，g722数据压缩
//SR_StartVoiceCom_V30创建一个udp监听，让后发送命令让设备连接到此socket上，完成后，创建一个VOICE_COM_HANDLE结构，并将此结构的ID返回给应用程序，
//应用程序后续使用此句柄，调用SR_VoiceCom_Data向设备读取和写入音频数据(16K,单声道，16bit，PCM)
//使用SR_StopVoiceCom结束会话
//SR_StartVoiceCom_V30，SR_VoiceCom_Data，SR_StopVoiceCom共同使用完成会话功能。
UINT32 _stdcall SR_VoiceCom(UINT32* iVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	LPRTP_SESSION session;
	int ret = RtpSessionCreate(&session);
	if (ret == 0)
	{
		ret = DeviceIntercomStart(d, NULL, session->LocalPort, STREAMTYPE_G722, PROTOCOL_RTP, 20, INPUTSOURCE_MIC, 97, AECMODE_DISABLE, d->uID);
		if (ret == RC_OK)
		{
			char buffer[1024];
			int  nRecvBytes;
			UINT32 RemoteIp;
			UINT32 RemotePort;
			int rlen = UdpRecv(session->udpsock, buffer, sizeof(buffer), &nRecvBytes, &RemoteIp, &RemotePort);
			if ((rlen == 0) && (nRecvBytes > 0) && (RemoteIp == d->nPeerIp))//设备连接
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

UINT32 _stdcall SR_VoiceComData(UINT32 iVoiceComHandle, CHAR* pSendDataBuffer, UINT32 nSendBufferSize, PCHAR pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)
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

UINT32 _stdcall SR_VoiceComClose(UINT32 iVoiceComHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	DeviceIntercomStop(d);

	LPRTP_SESSION session;
	int ret = RtpSessionFind(&session, iVoiceComHandle);
	if (session != NULL)RtpSessionDestroy(session);
	return RC_OK;
}

UINT32 _stdcall SR_Emergency(UINT32* iEmergencyHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	LPRTP_SESSION session;
	int ret = RtpSessionCreate(&session);
	if (ret == 0)
	{
		ret = DeviceEmergencyPlayFileStart(d, NULL, session->LocalPort, STREAMTYPE_G722, PROTOCOL_RTP, 97, d->uID);
		if (ret == RC_OK)
		{
			char buffer[1024];
			int  nRecvBytes;
			UINT32 RemoteIp;
			UINT32 RemotePort;
			int rlen = UdpRecv(session->udpsock, buffer, sizeof(buffer), &nRecvBytes, &RemoteIp, &RemotePort);
			if ((rlen == 0) && (nRecvBytes > 0) && (RemoteIp == d->nPeerIp))//设备连接
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

UINT32 _stdcall SR_EmergencyData(UINT32 iEmergencyHandle, CHAR* pSendDataBuffer, UINT32 nSendBufferSize, CHAR* pRecvDataBuffer, UINT32 nRecvBufferSize, UINT32* nRecvBytes, LPVOID lpInputParam, LPVOID lpOutputParam)
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

UINT32 _stdcall SR_EmergencyClose(UINT32 iEmergencyHandle, UINT32 lUserID, LPVOID lpInputParam, LPVOID lpOutputParam)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	DeviceEmergencyPlayFileStop(d);

	LPRTP_SESSION session;
	int ret = RtpSessionFind(&session, iEmergencyHandle);
	if(session != NULL)RtpSessionDestroy(session);
	return RC_OK;
}
UINT32 _stdcall SR_SetExceptionCallBack(fExceptionCallBack pCallBack)
{
	gfExceptionCallBack = pCallBack;
	return 0;
}

//设置实时播放音量//v0.1.2
UINT32 _stdcall SR_SetVolume(UINT32 lUserID, UINT32 nVolume)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	return DeviceSetVolume(d, nVolume);
}

UINT32 _stdcall SR_Apply(UINT32 lUserID)
{
	LPSR_DEVICE_ITEM d = DeviceFind(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;

	return DeviceApply(d);
}