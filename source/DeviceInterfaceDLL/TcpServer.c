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
	CHAR Address[128];//v0.2.1
	//LPVOID pDevice;//v0.2.1
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

//v0.2.1
SOCKET _Accept(SOCKET ListenSocket, struct sockaddr* RemoteAddr, int timeout)
{
	SOCKET sock;
	fd_set fread;
	struct timeval tv = {timeout/1000,1000*(timeout % 1000)};
	FD_ZERO(&fread);
	FD_SET(ListenSocket, &fread);
	int ret = select(ListenSocket + 1, &fread, NULL, NULL, &tv);
	if (ret > 0)
	{
		if (FD_ISSET(ListenSocket, &fread))
		{
			int size = sizeof(struct sockaddr);
			sock = accept(ListenSocket, RemoteAddr, &size);
			return sock;
		}
	}
	return INVALID_SOCKET;

}

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
		//AcceptSocket = accept(ListenSocket, (struct sockaddr*) & RemoteAddr, &size);//V0.2.1
		AcceptSocket = _Accept(ListenSocket, &RemoteAddr, 1000);
		//if (WSASetEvent(AcceptEvent) == FALSE)
		//{
		//	return FALSE;
		//}
		DeviceTick();//v0.2.1 心跳，检查设备连接超时事件

		if (AcceptSocket == INVALID_SOCKET)continue;
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
		//LPSR_DEVICE_ITEM d = DeviceFindByAddr(ipstr);//DeviceGetByAddr
		LPSR_DEVICE_ITEM d = DeviceGetByAddr(ipstr);//v0.2.1
		if (d != NULL)
		{
			//SocketInfo->pDevice = d;//v0.2.1
			if (d->Sock != INVALID_SOCKET)
				closesocket(d->Sock);
			//DeviceLock(d);//v0.2.1
			d->Sock = AcceptSocket;
			d->nPeerIp = Addr.sin_addr.S_un.S_addr;
			d->nPeerPort = Addr.sin_port;
			d->nLoginStats = 0;
			//DeviceUnLock(d);//v0.2.1
			//if (gfExceptionCallBack != NULL)gfExceptionCallBack(MSGTYPE_CONNECTED, d->nUserID, 0, &d);
			//访问d是不安全的，这里修改为访问一个d的拷贝，只读。
			if (gfExceptionCallBack != NULL)
			{
				gfExceptionCallBack(MSGTYPE_CONNECTED, d->nUserID, 0, &d);
			}

			//SocketInfo->lUserID = d->nUserID;
			DeviceRelease(d);//v0.2.1

			strncpy_s(SocketInfo->Address, sizeof(SocketInfo->Address), ipstr, sizeof(tmpbuf));//v0.2.1
			SocketInfo->Socket = AcceptSocket;
			ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
			SocketInfo->DataBuf.len = DATA_BUFSIZE;
			SocketInfo->DataBuf.buf = SocketInfo->Buffer;

			//检查刚连接上了的设备是否发送了数据。
			Flags = 0;
			fd_set fread;
			FD_ZERO(&fread);
			FD_SET(AcceptSocket, &fread); //v0.2.1
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			int ret = select(AcceptSocket + 1, &fread, NULL, NULL, &tv);//v0.2.1
			if (ret > 0)
			{
				char recvbuf[1440];
				int rlen = recv(AcceptSocket, recvbuf, sizeof(recvbuf), 0);
				if (rlen > 0)
				{
					LPSR_DEVICE_ITEM d = DeviceGetByAddr(ipstr);//v0.2.1//DeviceLock(d);
					if (d != NULL)
					{
						FifoPush(&d->Fifo, recvbuf, rlen);
						CommandProcess(d);
						DeviceRelease(d);
					}
					//FifoPush(&d->Fifo, recvbuf, rlen);
					//CommandProcess(d);
					//DeviceUnLock(d);
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

	//SOCKADDR_IN addr;
	//int size = sizeof(addr);
	//int ret = getpeername(SI->Socket, (struct sockaddr*) & addr, &size);
	//char tmpbuf[128];//v0.2.1
	//char* ipstr = inet_ntop(AF_INET, (const void*)&addr.sin_addr, tmpbuf, sizeof(tmpbuf));//v0.2.1
	char* ipstr = SI->Address;

	if (Error != 0 || BytesTransferred == 0)
	{
		if (gfExceptionCallBack != NULL)
		{
			LPSR_DEVICE_ITEM d = DeviceGetByAddr(ipstr);// SI->pDevice;
			if (d != NULL)
			{
				//DeviceLock(d);
				if (SI->Socket == d->Sock)
				{
					d->Sock = INVALID_SOCKET;
					gfExceptionCallBack(MSGTYPE_DISCONNECTED, d->nUserID, 0, &d);//回调被放入if语句中，只有在判式为真才触发回调。(如果设备复位，会首先accept，然后才会触发回调，执行本段程序）)
				}
				//gfExceptionCallBack(MSGTYPE_DISCONNECTED, d->nUserID, 0, &d);
				DeviceRelease(d);//DeviceUnLock(d);//v0.2.1
			}
		}
		closesocket(SI->Socket);
		GlobalFree(SI);
		return;
	}

	//v0.2.1 替换下面被屏蔽的源码，原来的源码有可能在操作时，d已经被释放。
	LPSR_DEVICE_ITEM d = DeviceGetByAddr(ipstr);
	if (d != NULL)
	{
		FifoPush(&d->Fifo, SI->DataBuf.buf, BytesTransferred);
		CommandProcess(d);
		DeviceRelease(d);
	}
	//LPSR_DEVICE_ITEM d = SI->pDevice;
	//DeviceLock(d);
	//FifoPush(&d->Fifo, SI->DataBuf.buf, BytesTransferred);
	//CommandProcess(d);
	//DeviceUnLock(d);

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

