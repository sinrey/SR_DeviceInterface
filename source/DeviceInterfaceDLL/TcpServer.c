/*
//以下代码主要参考自，稍作修改
https://www.winsocketdotnetworkprogramming.com/winsock2programming/winsock2advancediomethod5g.html
*/
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include "device.h"
#include "log.h"
#include "DeviceInterfaceDll.h"

#pragma comment(lib, "ws2_32.lib")

#define DATA_BUFSIZE 2048

typedef struct _SOCKET_INFORMATION {
	OVERLAPPED Overlapped;
	SOCKET Socket;
	CHAR Buffer[DATA_BUFSIZE];
	WSABUF DataBuf;
	LPVOID pDevice;
	//DWORD BytesSEND;
	//DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

typedef struct TcpServerThreadArg
{
	SOCKET ListenSocket;

}TCPSERVER_THREAD_ARG, *LPTCPSERVER_THREAD_ARG;

void CALLBACK WorkerRecvRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
DWORD WINAPI WorkerThread(LPVOID lpParameter);
DWORD WINAPI TcpServerThread(LPVOID lpParameter);
//SOCKET AcceptSocket;
//SOCKADDR_IN RemoteAddr;
extern DWORD gLastError;
HANDLE ThreadHandle;

extern fExceptionCallBack gfExceptionCallBack;

INT TcpServerStart(UINT32 port)
{
	SOCKET ListenSocket;
	SOCKADDR_IN InternetAddr;
//	INT Ret;
	
	DWORD ThreadId;
//	WSAEVENT AcceptEvent;
	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		gLastError = WSAGetLastError();
		LogErrWithCode("create listen socket is fault", gLastError);
		return 1;
	}
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(port);

	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		gLastError = WSAGetLastError();
		LogErrWithCode("bind socket is fault", gLastError);
		return 1;
	}

	if (listen(ListenSocket, 5))
	{
		gLastError = WSAGetLastError();
		LogErrWithCode("listen is fault", gLastError);
		return 1;
	}

	static TCPSERVER_THREAD_ARG arg;
	arg.ListenSocket = ListenSocket;

	ThreadHandle = (void*)_beginthreadex(NULL, 0, TcpServerThread, &arg, 0, &ThreadId);
	if (ThreadHandle != NULL)
	{
		return 0;
	}
	else
	{
		LogErr("can not create tcp server thread");
		return 2;
	}
}

void TcpServerStop()
{
	TerminateThread(ThreadHandle, 0);
}
/*
DWORD WINAPI TcpServerThread(LPVOID* lpParameter)
{
	SOCKET ListenSocket;
	WSAEVENT AcceptEvent;
	HANDLE ThreadHandle;
	DWORD ThreadId;

	LPTCPSERVER_THREAD_ARG parg = (LPTCPSERVER_THREAD_ARG)lpParameter;
	ListenSocket = parg->ListenSocket;

	if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		gLastError = WSAGetLastError();
		return FALSE;
	}


	if ((ThreadHandle = (void*)_beginthreadex(NULL, 0, WorkerThread, (LPVOID)AcceptEvent, 0, &ThreadId)) == NULL)
	{
		return FALSE;
	}

	while (TRUE)
	{
		int size = sizeof(RemoteAddr);
		AcceptSocket = accept(ListenSocket, (struct sockaddr*)&RemoteAddr, &size);
		if (WSASetEvent(AcceptEvent) == FALSE)
		{
			return FALSE;
		}
	}
}
*/

DWORD WINAPI TcpServerThread(LPVOID* lpParameter)
{
	SOCKET ListenSocket;
	WSAEVENT AcceptEvent;
	//HANDLE ThreadHandle;
	DWORD ThreadId;
	SOCKET AcceptSocket;
	SOCKADDR_IN RemoteAddr;

	LPTCPSERVER_THREAD_ARG parg = (LPTCPSERVER_THREAD_ARG)lpParameter;
	ListenSocket = parg->ListenSocket;

	//if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	//{
	//	gLastError = WSAGetLastError();
	//	return FALSE;
	//}

	/*
	if ((ThreadHandle = (void*)_beginthreadex(NULL, 0, WorkerThread, (LPVOID)AcceptEvent, 0, &ThreadId)) == NULL)
	{
		return FALSE;
	}
	*/
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD RecvBytes;
	while (TRUE)
	{
		int size = sizeof(RemoteAddr);
		AcceptSocket = accept(ListenSocket, (struct sockaddr*) & RemoteAddr, &size);
		//if (WSASetEvent(AcceptEvent) == FALSE)
		//{
		//	return FALSE;
		//}
		SOCKADDR_IN Addr;
		memcpy(&Addr, &RemoteAddr, sizeof(Addr));

		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
		{
			//gLastError = GetLastError();
			LogErr("can not GlobalAlloc socketinfo, memory lack");
			return FALSE;
		}
		char tmpbuf[128];
		char* ipstr = inet_ntop(AF_INET, (const void*)& Addr.sin_addr, tmpbuf, sizeof(tmpbuf));
		LPSR_DEVICE_ITEM d = DeviceFindByAddr(ipstr);
		if (d != NULL)
		{
			SocketInfo->pDevice = d;
			if (d->Sock != INVALID_SOCKET)
				closesocket(d->Sock);
			DeviceLock(d);
			d->Sock = AcceptSocket;
			d->nPeerIp = Addr.sin_addr.S_un.S_addr;
			d->nPeerPort = Addr.sin_port;
			d->nLoginStats = 0;
			DeviceUnLock(d);
			SocketInfo->Socket = AcceptSocket;
			ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
			SocketInfo->DataBuf.len = DATA_BUFSIZE;
			SocketInfo->DataBuf.buf = SocketInfo->Buffer;

			if (gfExceptionCallBack != NULL)gfExceptionCallBack(MSGTYPE_CONNECTED, d->nUserID, 0, &d);

			Flags = 0;
			fd_set fread;
			FD_ZERO(&fread);
			FD_SET(d->Sock, &fread);
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			int ret = select(d->Sock + 1, &fread, NULL, NULL, &tv);
			if (ret > 0)
			{
				char recvbuf[1440];
				int rlen = recv(d->Sock, recvbuf, sizeof(recvbuf), 0);
				if (rlen > 0)
				{
					DeviceLock(d);
					FifoPush(&d->Fifo, recvbuf, rlen);
					CommandProcess(d);
					DeviceUnLock(d);
				}
			}

			if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
				&(SocketInfo->Overlapped), WorkerRecvRoutine) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					gLastError = WSAGetLastError();
					LogErrWithCode("pending socket is fault", gLastError);
					//return FALSE;
				}
			}
		}
		else
		{
			//没有注册，关闭连接。
			Log("unregistered");
			closesocket(AcceptSocket);
		}

	}

	_endthreadex(0);
}
/*
DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	WSAEVENT EventArray[1];
	DWORD Index;
	DWORD RecvBytes;

	EventArray[0] = (WSAEVENT)lpParameter;

	while (TRUE)
	{
		while (TRUE)
		{
			Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);
			if (Index == WSA_WAIT_FAILED)
			{
				gLastError = WSAGetLastError();
				return FALSE;
			}

			if (Index != WAIT_IO_COMPLETION)
			{
				break;
			}
		}
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		SOCKADDR_IN Addr;
		memcpy(&Addr, &RemoteAddr, sizeof(Addr));

		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
		{
			gLastError = GetLastError();
			return FALSE;
		}
		char tmpbuf[128];
		char* ipstr = inet_ntop(AF_INET,(const void*)&Addr.sin_addr,tmpbuf,sizeof(tmpbuf));
		LPSR_DEVICE_ITEM d = DeviceFindByAddr(ipstr);
		if (d != NULL)
		{
			SocketInfo->pDevice = d;
			if (d->Sock != INVALID_SOCKET)closesocket(d->Sock);
			DeviceLock(d);
			d->Sock = AcceptSocket;
			d->nPeerIp = Addr.sin_addr.S_un.S_addr;
			d->nPeerPort = Addr.sin_port;
			DeviceUnLock(d);
			SocketInfo->Socket = AcceptSocket;
			ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
			SocketInfo->DataBuf.len = DATA_BUFSIZE;
			SocketInfo->DataBuf.buf = SocketInfo->Buffer;

			if (gfExceptionCallBack != NULL)gfExceptionCallBack(MSGTYPE_CONNECTED, d->nUserID, 0, &d);

			Flags = 0;
			if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
				&(SocketInfo->Overlapped), WorkerRecvRoutine) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != WSA_IO_PENDING)
				{
					gLastError = WSAGetLastError();
					return FALSE;
				}
			}
		}
		else
		{
			//没有注册，关闭连接。
			closesocket(AcceptSocket);
		}
	}
}
*/
void CALLBACK WorkerRecvRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD Flags;
	DWORD RecvBytes;
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

	if (Error != 0 || BytesTransferred == 0)
	{
		SOCKADDR_IN addr;
		int size = sizeof(addr);
		getpeername(SI->Socket, (struct sockaddr*)&addr, &size);
		if (gfExceptionCallBack != NULL)
		{
			LPSR_DEVICE_ITEM d = SI->pDevice;
			gfExceptionCallBack(MSGTYPE_DISCONNECTED, d->nUserID, 0, &d);
			DeviceLock(d);
			if (SI->Socket == d->Sock)
			{
				d->Sock = INVALID_SOCKET;
			}
			DeviceUnLock(d);

		}
		closesocket(SI->Socket);
		GlobalFree(SI);
		return;
	}

	LPSR_DEVICE_ITEM d = SI->pDevice;
	DeviceLock(d);
	FifoPush(&d->Fifo, SI->DataBuf.buf, BytesTransferred);
	CommandProcess(d);
	DeviceUnLock(d);

	Flags = 0;
	ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
	SI->DataBuf.len = DATA_BUFSIZE;
	SI->DataBuf.buf = SI->Buffer;

	if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags, &(SI->Overlapped), WorkerRecvRoutine) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			gLastError = WSAGetLastError();
			return;
		}
	}
}

void CALLBACK WorkerSendRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;
	GlobalFree(SI);
}

int TcpServerSend(SOCKET sock, char* buf, int len)
{
	DWORD SendBytes;
	LPSOCKET_INFORMATION SocketInfo;
	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
	{
		gLastError = GetLastError();
		return FALSE;
	}
	SocketInfo->Socket = sock;
	ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
	memcpy(SocketInfo->Buffer, buf, len);

	SocketInfo->DataBuf.len = len;
	SocketInfo->DataBuf.buf = SocketInfo->Buffer;

	return WSASend(sock, &(SocketInfo->DataBuf), 1, &SendBytes, 0, &(SocketInfo->Overlapped), WorkerSendRoutine);
}

int TcpServerSend_Block(SOCKET sock, char* buf, int len)
{
	DWORD SendBytes;
	WSABUF DataBuf;
	DataBuf.len = len;
	DataBuf.buf = buf;
	return WSASend(sock, &DataBuf, 1, &SendBytes, 0, NULL, NULL);
}

