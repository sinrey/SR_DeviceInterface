#include <stdio.h>
#include <windows.h>
#include "device.h"
#include "list.h"
#include "cJSON.h"
#include "DeviceInterfaceDll.h"
#include "command.h"
#include "TcpServer.h"

static List gDeviceList = NULL;

extern CRITICAL_SECTION gCriticalSection;//全局变量临界保护区变量
//extern char gUserName[64];
//extern char gPassword[64];
extern fExceptionCallBack gfExceptionCallBack;
extern char* MDString(char *string);
//int TcpServerSend(int sock, char* buf, int len);

//创建一个device结构，并初始化，加入到链表

LPSR_DEVICE_ITEM DeviceAdd(LPSR_USER_LOGIN_INFO LogInfo, LPSR_DEVICEINFO DevInfo)
{
	LPSR_DEVICE_ITEM d;
	d = (LPSR_DEVICE_ITEM)malloc(sizeof(SR_DEVICE_ITEM));
	if (d == NULL)return NULL;
	memset(d, 0, sizeof(SR_DEVICE_ITEM));
	InitializeCriticalSection(&d->cs);
	memcpy(&d->LogInfo, LogInfo, sizeof(SR_USER_LOGIN_INFO));
	memcpy(&d->DevInfo, DevInfo, sizeof(LPSR_DEVICEINFO));

	char* p = malloc(8 * 1024);
	FifoInit(&d->Fifo, p, 8 * 1024);

	d->nCommandBufferSize = 0;
	d->hEvent = NULL;
	d->sCommand[0] = 0;
	d->psCommandBuffer = NULL;

	//v0.2.1
	d->uTimeOut = LogInfo->uTimeOut;
	if (d->uTimeOut == 0)d->uTimeOut = 60;
	if (d->uTimeOut < 10)d->uTimeOut = 10;
	if (d->uTimeOut > 3600)d->uTimeOut = 3600;
	d->uTickTimestamp = GetTickCount();

	d->nLoginStats = 0;
	d->Sock = INVALID_SOCKET;
	EnterCriticalSection(&gCriticalSection);
	if (gDeviceList != NULL)
	{
		LPSR_DEVICE_ITEM d1 = gDeviceList->data;
		d->nUserID = d1->nUserID + 1;
	}
	else
	{
		d->nUserID = 1;
	}
	gDeviceList = ListAdd(gDeviceList, d);
	LeaveCriticalSection(&gCriticalSection);
	return d;
}

void DeviceFree(LPSR_DEVICE_ITEM d)
{
	DeleteCriticalSection(&d->cs);
	free(d->Fifo.Buffer);
	free(d);
}
/*
LPSR_DEVICE_ITEM DeviceFindByAddr(char* addr)
{
	//LPSR_DEVICE_ITEM d;
	EnterCriticalSection(&gCriticalSection);
	List p = gDeviceList;
	while (p != NULL)
	{
		LPSR_DEVICE_ITEM d = p->data;
		if (strcmp(d->LogInfo.sDeviceAddress, addr) == 0)
		{
			LeaveCriticalSection(&gCriticalSection);
			return d;
		}
		p = p->next;
	}
	LeaveCriticalSection(&gCriticalSection);
	return NULL;
}
*/
//当找到设备d后，加锁
LPSR_DEVICE_ITEM DeviceGetByAddr(char* addr)
{
	//LPSR_DEVICE_ITEM d;
	EnterCriticalSection(&gCriticalSection);
	List p = gDeviceList;
	while (p != NULL)
	{
		LPSR_DEVICE_ITEM d = p->data;
		if (strcmp(d->LogInfo.sDeviceAddress, addr) == 0)
		{
			DeviceLock(d);//v0.2.1
			LeaveCriticalSection(&gCriticalSection);
			return d;
		}
		p = p->next;
	}
	LeaveCriticalSection(&gCriticalSection);
	return NULL;
}
/*
LPSR_DEVICE_ITEM DeviceFind(UINT32 lUserID)
{
	EnterCriticalSection(&gCriticalSection);
	//LeaveCriticalSection(&gCriticalSection);
	List p = gDeviceList;
	while (p != NULL)
	{
		LPSR_DEVICE_ITEM d = p->data;
		if (d->nUserID == lUserID)
		{
			LeaveCriticalSection(&gCriticalSection);
			return d;
		}
		p = p->next;
	}
	LeaveCriticalSection(&gCriticalSection);
	return NULL;
}
*/
//v0.2.1
//当找到设备d后，加锁
LPSR_DEVICE_ITEM DeviceGet(UINT32 lUserID)
{
	EnterCriticalSection(&gCriticalSection);
	//LeaveCriticalSection(&gCriticalSection);
	List p = gDeviceList;
	while (p != NULL)
	{
		LPSR_DEVICE_ITEM d = p->data;
		if (d->nUserID == lUserID)
		{
			DeviceLock(d);
			LeaveCriticalSection(&gCriticalSection);
			return d;
		}
		p = p->next;
	}
	LeaveCriticalSection(&gCriticalSection);
	return NULL;
}

//v0.2.1
VOID DeviceRelease(LPSR_DEVICE_ITEM d)
{
	DeviceUnLock(d);
}

//移除成功返回0，目标不存在返回1
INT DeviceRemove(UINT32 nUserID)
{
	int result = 1;
	if (gDeviceList == NULL)return RC_INVALID_USER_HANDLE;
	EnterCriticalSection(&gCriticalSection);
	//LeaveCriticalSection(&gCriticalSection);
	List pb = gDeviceList;
	List pf = NULL;
	LPSR_DEVICE_ITEM d_pb;
	d_pb = pb->data;
	while (pb != NULL)
	{
		d_pb = pb->data;
		if (d_pb->nUserID == nUserID)break;
		else
		{
			pf = pb;
			pb = pb->next;
		}
	}

	if (d_pb->nUserID == nUserID)
	{
		DeviceLock(d_pb);//v0.2.1 设备加锁，确保没有其他代码使用设备d，否则释放d将导致其他代码异常。
		if (pb == gDeviceList)//被删除设备在链表第一位置
		{
			gDeviceList = pb->next;
		}
		else
		{ 
			pf->next = pb->next;
		}
		if (d_pb->Sock != INVALID_SOCKET)shutdown(d_pb->Sock,2);

		DeviceFree(d_pb);//DeviceFree(pb->data);//0.1.4
		free(pb);
		result = 0;
	}
	LeaveCriticalSection(&gCriticalSection);
	return result;
}

void DeviceRemoveAll()
{
	EnterCriticalSection(&gCriticalSection);
	//LeaveCriticalSection(&gCriticalSection);
	List p = gDeviceList;
	LPSR_DEVICE_ITEM d_pb;
	while (p != NULL)
	{
		List p1 = p->next;
		d_pb = p->data;
		DeviceFree(d_pb);
		free(p);
		p = p1;
	}
	gDeviceList = NULL;
	LeaveCriticalSection(&gCriticalSection);
}


VOID DeviceSetProbe(LPSR_DEVICE_ITEM d, HANDLE hEvent, CHAR* sCmd, CHAR* psBuffer, UINT32 nBufferSize)
{
	if (d->hEvent != NULL)return;
	//DeviceLock(d);//v0.2.1 设备d加锁是调用者的责任
	d->hEvent = hEvent;
	d->psCommandBuffer = psBuffer;
	d->nCommandBufferSize = nBufferSize;
	strncpy_s(d->sCommand, sizeof(d->sCommand), sCmd, strlen(sCmd));
	//DeviceUnLock(d);//0.2.1
}

VOID DeviceReleaseProbe(LPSR_DEVICE_ITEM d)
{
	//DeviceLock(d);//v0.2.1 设备d加锁是调用者的责任
	d->hEvent = NULL;
	d->psCommandBuffer = NULL;
	d->nCommandBufferSize = 0;
	memset(d->sCommand, 0, sizeof(d->sCommand));
	//DeviceUnLock(d);
}

void DeviceLock(LPSR_DEVICE_ITEM d)
{
	EnterCriticalSection(&d->cs);
}

void DeviceUnLock(LPSR_DEVICE_ITEM d)
{
	LeaveCriticalSection(&d->cs);
}

//从接收缓冲提取命令并进行处理,输出到out缓冲，长度为int
int GetCommand(LPSR_DEVICE_ITEM d, CHAR* out, UINT32 outsize)
{
	BOOL header = FALSE;
	BOOL end = FALSE;
	int level = 0;
	UINT32 count = 0;
	struct _Fifo* pFifo = &d->Fifo;
	while (FifoGetDataLength(pFifo) > 0)
	{
		char c;
		FifoPop(pFifo, &c, 1);
		

		if (count >= outsize)return -1;
		
		if (c == '{')
		{
			header = TRUE;
			level++;
		}
		else if (c == '}')
		{
			level--;
			if ((level == 0) && (header))
			{
				end = TRUE;
			}
		}

		if (header)out[count++] = c;

		if (end)break;

	}
	
	if (end)
	{
		return count;
	}
	else
	{
		//找不到完整是json语句,将刚才读取的字符串再次填入fifo
		FifoPush(pFifo, out, count);
		return 0;
	}
}


void CommandProcess(LPSR_DEVICE_ITEM d)
{
	char out[4096];
	unsigned int len;
	do
	{
		len = GetCommand(d, out, 4096);
		if (len > 0)
		{
			cJSON* json = NULL;
			json = cJSON_Parse(out);
			if (json != NULL)
			{
				cJSON* command = cJSON_GetObjectItem(json, "command");
				if (command != NULL)
				{
					if (d->sCommand[0])//有监听事件的请求，当收到指定的消息后，将对应的事件同步量设置为有效。
					{
						if (strcmp(command->valuestring, d->sCommand) == 0)
						{
							if (d->nCommandBufferSize > len)
							{
								memcpy(d->psCommandBuffer, out, len);
								d->psCommandBuffer[len] = 0;
								d->nRecvBytes = len;
								if (d->hEvent != NULL)
								{
									d->sCommand[0] = 0;//收到预期命令后，就清空命令，取消回应探测。
									SetEvent(d->hEvent);
								}
							}
						}
					}
					else if (strcmp(command->valuestring, "register") == 0)
					{
						UINT32 systick = (d->uTimeOut * 2) / 3;
						systick = min(systick, 3000);
						systick = max(systick, 5);

						if (d->nLoginStats == 0)//刚连接，这里应该是设备的第一次注册请求
						{
							cJSON* authentication = cJSON_GetObjectItem(json, "authentication");
							cJSON* session = cJSON_GetObjectItem(json, "session");
							cJSON* auth_dir = cJSON_GetObjectItem(json, "auth_dir");
							cJSON* id = cJSON_GetObjectItem(json, "id");

							int len = 0;
							if (session != NULL)
							{
								//if (id != NULL)d->uID = id->valueint;
								if (id != NULL)strcpy_s(d->uID, sizeof(d->uID), id->valuestring);

								//char* jsontext;
								
								if ((session != NULL) && (authentication != NULL))
								{
									char tmpbuf[256];
									sprintf_s(tmpbuf, sizeof(tmpbuf), "%s@%s@%s", session->valuestring, d->LogInfo.sUserName, d->LogInfo.sPassword);
									char* s = MDString(tmpbuf);


									if (strcmp(s, authentication->valuestring) == 0)
									{
										if ((auth_dir != NULL) && (strcmp(auth_dir->valuestring, "bi-direction") == 0))
										{
											UINT64 tick = GetTickCount64();
											//char tmpbuf[256];
											char session[64];
											sprintf_s(tmpbuf, sizeof(tmpbuf), "%8x%8x", (UINT32)(tick >> 32), (UINT32)tick);
											char* s = MDString(tmpbuf);
											strncpy_s(session, sizeof(session), s, strlen(s));
											sprintf_s(tmpbuf, sizeof(tmpbuf), "%s@%s@%s", session, d->LogInfo.sUserName, d->LogInfo.sPassword);
											char* auth = MDString(tmpbuf);

											//d->nLoginStats = 1;
											
											//jsontext = CommandRegisterAck(session, auth, RESULT_OK);
											len = CommandRegisterAck(out, sizeof(out), session, auth, systick, RESULT_OK);
										}
										else//仅服务器鉴权
										{
											len = CommandRegisterAck(out, sizeof(out), NULL, NULL, systick, RESULT_OK);
										}
										d->nLoginStats = 1;
									}
									else//鉴权失败，非法设备登录
									{
										len = CommandRegisterAck(out, sizeof(out), NULL, NULL, 0, RESULT_REGISTER_FAULT);
									}
								}
								else//设备登录数据不包含必须字段。
								{
									len = CommandRegisterAck(out, sizeof(out), NULL, NULL, 0, RESULT_REGISTER_FAULT);
								}
							}
							else////v0.1.3
							{
								if(id != NULL)sprintf_s(d->uID, sizeof(d->uID), "%u", id->valueint);

								char tmpbuf[256];
								sprintf_s(tmpbuf, sizeof(tmpbuf), "%s@%s", d->LogInfo.sPassword, d->LogInfo.sUserName);
								char* s = MDString(tmpbuf);

								if ((authentication != NULL)&&(strcmp(s, authentication->valuestring) == 0))
								{
									if ((auth_dir != NULL) && (strcmp(auth_dir->valuestring, "bi-direction") == 0))
									{
										char* auth = MDString(d->LogInfo.sPassword);
										len = CommandRegisterAck(out, sizeof(out), NULL, auth, systick, RESULT_OK);
									}
									else//仅服务器鉴权
									{
										len = CommandRegisterAck(out, sizeof(out), NULL, NULL, systick, RESULT_OK);
									}
									d->nLoginStats = 1;
								}
								else//鉴权失败，非法设备登录
								{
									len = CommandRegisterAck(out, sizeof(out), NULL, NULL, 0, RESULT_REGISTER_FAULT);
								}
							}
							TcpServerSend(d->Sock, out, len);
						}
						else if (d->nLoginStats == 1)//服务器已经通过设备登录鉴权，等到设备回应200
						{
							cJSON* result = cJSON_GetObjectItem(json, "result");
							if (result->valueint == RESULT_OK)
							{
								d->nLoginStats = 2;
								d->uTickTimestamp = GetTickCount();//v0.2.1 登录后复位心跳时间戳，用于超时计时。
								if (gfExceptionCallBack != NULL)
								{
									gfExceptionCallBack(MSGTYPE_DEVICE_LOGIN, d->nUserID, 0, &d);
								}
							}
						}
					}
					else if (strcmp(command->valuestring, "status") == 0)
					{
					}
					else if(strcmp(command->valuestring, "message") == 0)
					{ }
					else if (strcmp(command->valuestring, "close") == 0)
					{

					}
					else
					{
					}

					d->uTickTimestamp = GetTickCount();//v0.2.1 每次收到合法的通讯帧，都这个时间戳进行复位。
					if (d->nLoginStats == 3)//如果之前发生了timeout事件，这里进行回复。
					{
						if (gfExceptionCallBack != NULL)
						{
							gfExceptionCallBack(MSGTYPE_RECONNECT, d->nUserID, 0, &d);
						}
						d->nLoginStats = 2;
					}
				}
				cJSON_Delete(json);
			}
		}
	} while (len > 0);
}

#define COMMAND_BUFFER_SIZE 1024
/*
INT SendCommand(LPSR_DEVICE_ITEM d, CHAR* cmd, CHAR* sendbuf, UINT32 slen, CHAR* recvbuf, UINT32 recvbufsize, UINT32* nrecvbyte)
{
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL)return -1;
	LPSR_DEVICE_ITEM d1 = DeviceGet(d->nUserID);
	if (d1 == NULL)return RC_INVALID_USER_HANDLE;
	DeviceSetProbe(d, hEvent, cmd, recvbuf, recvbufsize);
	TcpServerSend_Block(d->Sock, sendbuf, slen);
	int ret = WaitForSingleObject(hEvent, 3000);
	DeviceReleaseProbe(d);
	CloseHandle(hEvent);

	if (ret == WAIT_OBJECT_0)
	{
		*nrecvbyte = d->nRecvBytes;
		return 0;
	}
	else
	{
		*nrecvbyte = 0;
		return -1;
	}
}
*/
INT SendCommand(UINT32 lUserID, CHAR* cmd, CHAR* sendbuf, UINT32 slen, CHAR* recvbuf, UINT32 recvbufsize, UINT32* nrecvbyte)
{
	LPSR_DEVICE_ITEM d;
	UINT32 nRecvBytes = 0;
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL)return -1;
	d = DeviceGet(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	DeviceSetProbe(d, hEvent, cmd, recvbuf, recvbufsize);
	TcpServerSend_Block(d->Sock, sendbuf, slen);
	DeviceRelease(d);
	int ret = WaitForSingleObject(hEvent, 3000);
	
	d = DeviceGet(lUserID);
	if (d != NULL)
	{
		nRecvBytes = d->nRecvBytes;
		DeviceReleaseProbe(d);
		DeviceRelease(d);
	}
	CloseHandle(hEvent);

	if (ret == WAIT_OBJECT_0)
	{
		*nrecvbyte = nRecvBytes;
		return 0;
	}
	else
	{
		*nrecvbyte = 0;
		return -1;
	}
}

INT SendCommand_WithTimeOut(UINT32 lUserID, CHAR* cmd, CHAR* sendbuf, UINT32 slen, CHAR* recvbuf, UINT32 recvbufsize, UINT32* nrecvbyte, UINT32 uTimeOut)
{
	LPSR_DEVICE_ITEM d;
	UINT32 nRecvBytes = 0;
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL)return -1;
	d = DeviceGet(lUserID);
	if (d == NULL)return RC_INVALID_USER_HANDLE;
	DeviceSetProbe(d, hEvent, cmd, recvbuf, recvbufsize);
	TcpServerSend_Block(d->Sock, sendbuf, slen);
	DeviceRelease(d);
	int ret = WaitForSingleObject(hEvent, uTimeOut);

	d = DeviceGet(lUserID);
	if (d != NULL)
	{
		nRecvBytes = d->nRecvBytes;
		DeviceReleaseProbe(d);
		DeviceRelease(d);
	}
	CloseHandle(hEvent);

	if (ret == WAIT_OBJECT_0)
	{
		*nrecvbyte = nRecvBytes;
		return 0;
	}
	else
	{
		*nrecvbyte = 0;
		return -1;
	}
}

//返回sd卡的总容量和剩余容量
int DeviceGetDiskInfo(UINT32 lUserID, UINT32* totalCapacity, UINT32* remainCapacity)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];

	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSDCardGetInfo(sendbuf, COMMAND_BUFFER_SIZE);
	if (len == 0)return RC_UNKNOWN;

	//int ret = SendCommand(lUserID, CMD_GET_DISK_INFO, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);
	int ret = SendCommand_WithTimeOut(lUserID, CMD_GET_DISK_INFO, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes, 8000);
	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_GET_DISK_INFO) == 0)
			{
				CHAR tmpbuf[128];
				PCHAR pEnd;

				cJSON* tol = cJSON_GetObjectItem(json, "disk_size");
				cJSON* fre = cJSON_GetObjectItem(json, "free_size");
				*totalCapacity = 0;
				*remainCapacity = 0;
				if ((tol != NULL) && (tol->valuestring != NULL))
				{
					strncpy_s(tmpbuf, 128, tol->valuestring, strlen(tol->valuestring));
					*totalCapacity = strtol(tmpbuf, &pEnd, 10);
				}
				if ((fre != NULL) && (fre->valuestring != NULL))
				{
					strncpy_s(tmpbuf, 128, fre->valuestring, strlen(fre->valuestring));
					*remainCapacity = strtol(tmpbuf, &pEnd, 10);
				}
				cJSON_Delete(json);
				return RC_OK;
			}
			return RC_UNKNOWN;
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}

INT DeviceGetFirstFile(UINT32 lUserID, CHAR* sFileName, UINT32 nFileNameSize, UINT32* nFileSize)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSDCardGetFirstFile(sendbuf, COMMAND_BUFFER_SIZE);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_GET_FIRST_FILE, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE,&nRecvBytes);
	
	if(ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_GET_FIRST_FILE) == 0)
			{
				cJSON* filename = cJSON_GetObjectItem(json, "filename");
				cJSON* filesize = cJSON_GetObjectItem(json, "filesize");
				if ((filename != NULL) && (filename->valuestring != NULL))
				{
					strncpy_s(sFileName, nFileNameSize, filename->valuestring, strlen(filename->valuestring));
				}
				if (filesize != NULL)
				{
					*nFileSize = filesize->valueint;
				}
				cJSON_Delete(json);
				return 0;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}

INT DeviceGetNextFile(UINT32 lUserID, CHAR* sFileName, UINT32 nFileNameSize, UINT32* nFileSize)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSDCardGetNextFile(sendbuf, COMMAND_BUFFER_SIZE);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_GET_NEXT_FILE, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_GET_NEXT_FILE) == 0)
			{
				cJSON* filename = cJSON_GetObjectItem(json, "filename");
				cJSON* filesize = cJSON_GetObjectItem(json, "filesize");
				if ((filename != NULL) && (filename->valuestring != NULL))
				{
					strncpy_s(sFileName, nFileNameSize, filename->valuestring, strlen(filename->valuestring));
				}
				if (filesize != NULL)
				{
					*nFileSize = filesize->valueint;
				}
				cJSON_Delete(json);
				return 0;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}

//CommandSDCardDeleteFile
INT DeviceDeleteFile(UINT32 lUserID, CHAR* sFileName)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSDCardDeleteFile(sendbuf, COMMAND_BUFFER_SIZE, sFileName);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_DELETE_FILE, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_DELETE_FILE) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if (result->valueint == 200)
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}

INT DeviceUploadFileStart(UINT32 lUserID, UINT32 nDataPort, CHAR* sFileName, BOOL bConver)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSDCardUploadFile(sendbuf, COMMAND_BUFFER_SIZE, nDataPort, sFileName, bConver);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_UPLOAD_FILE, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_UPLOAD_FILE) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");
				
				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}

INT DeviceUpdateStart(UINT32 lUserID, UINT32 nDataPort, INT nMode, CHAR* sFileName)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandUpdate(sendbuf, COMMAND_BUFFER_SIZE, nDataPort, nMode, sFileName);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_UPDATE, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_UPDATE) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}

INT DeviceApply(UINT32 lUserID)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandApply(sendbuf, COMMAND_BUFFER_SIZE);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID , CMD_APPLY, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_APPLY) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
	//
//INT CommandApply(CHAR* Out, INT OutSize)
}

//当前函数仅支持mp3文件的播放
//sFileName仅作为播放信息使用，函数并不操作该文件。
//INT DevicePlayFileStart(LPSR_DEVICE_ITEM d, UINT32 nDataPort, CHAR* sFileName, INT nVolume)
INT DevicePlayFileStart(UINT32 lUserID, UINT32 nDataPort, CHAR* sFileName, INT nVolume)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandPlayFileStart(sendbuf, COMMAND_BUFFER_SIZE, nDataPort, sFileName, nVolume);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_START, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_START) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
	//
}

INT DevicePlayFileStop(UINT32 lUserID)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandPlayFileStop(sendbuf, COMMAND_BUFFER_SIZE);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_STOP, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_STOP) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					//if ((result->valueint == 100) || (result->valueint == 200))
					if(result->valueint == 221)
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
	//
}

INT DeviceEmergencyPlayFileStart(UINT32 lUserID, CHAR* sTargetAddr, UINT32 nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nVolume, CHAR* sFileName)
{
	PCHAR sIp = NULL;
	if (sTargetAddr == NULL)sIp = "0.0.0.0";
	else sIp = sTargetAddr;

	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandPlayFileEmergencyStart(sendbuf, COMMAND_BUFFER_SIZE, sIp, nTargetPort, sStreamType, sProtocol, nVolume, sFileName);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_EMERGENCY_START, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_EMERGENCY_START) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}
INT DeviceEmergencyPlayFileStop(UINT32 lUserID)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandPlayFileEmergencyStop(sendbuf, COMMAND_BUFFER_SIZE);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID , CMD_EMERGENCY_STOP, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_EMERGENCY_STOP) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
	//
}

INT DeviceSDCardPlayFileStart(UINT32 lUserID, CHAR* sFileName, INT nVolume)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSDCardPlayFileStart(sendbuf, COMMAND_BUFFER_SIZE, sFileName, nVolume);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_START, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_START) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
	//
}

INT DeviceSDCardPlayFileStop(UINT32 lUserID)
{
	return DevicePlayFileStop(lUserID);
}

INT DeviceSDCardPlayFileGetStatus(UINT32 lUserID, INT* runtime, INT* process)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSDCardPlayFileGetStatus(sendbuf, COMMAND_BUFFER_SIZE);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_SDCARD_PLAYFILE_STATUS, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_SDCARD_PLAYFILE_STATUS) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						cJSON* filename = cJSON_GetObjectItem(json, "filename");
						cJSON* r = cJSON_GetObjectItem(json, "runtime");
						cJSON* p = cJSON_GetObjectItem(json, "process");
						if (r != NULL)*runtime = r->valueint;
						if (p != NULL)*process = p->valueint;
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
	//
}

INT DeviceIntercomStart(UINT32 lUserID, CHAR* sTargetAddr,UINT32 nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nInputGain, CHAR* sInputSource, INT nVolume, CHAR* sAecMode, CHAR* sSession)
{
	PCHAR sIp = NULL;
	if(sTargetAddr == NULL)sIp = "0.0.0.0";
	else sIp = sTargetAddr;
	
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandIntercomStart(sendbuf, COMMAND_BUFFER_SIZE, sIp, nTargetPort, sStreamType, sProtocol, nInputGain, sInputSource, nVolume, sAecMode, sSession);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_START, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_START) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if ((result->valueint == 100) || (result->valueint == 200))
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
	//INT CommandIntercomStart(PCHAR Out, INT OutSize, PCHAR sTargetAddr, INT nTargetPort, PCHAR sStreamType, PCHAR sProtocol, INT nInputGain, PCHAR sInputSource, INT nVolume, PCHAR sAecMode)
}

INT DeviceIntercomStop(UINT32 lUserID)
{
	return DevicePlayFileStop(lUserID);
}

INT DeviceSetVolume(UINT32 lUserID, UINT32 nVolume)
{
	INT nRecvBytes;
	CHAR sendbuf[COMMAND_BUFFER_SIZE];
	CHAR recvbuf[COMMAND_BUFFER_SIZE];
	//if (d == NULL)return RC_INVALID_USER_HANDLE;
	int len = CommandSetVolume(sendbuf, COMMAND_BUFFER_SIZE, nVolume);
	if (len == 0)return RC_UNKNOWN;

	int ret = SendCommand(lUserID, CMD_SET, sendbuf, len, recvbuf, COMMAND_BUFFER_SIZE, &nRecvBytes);

	if (ret == 0)
	{
		cJSON* json = cJSON_Parse(recvbuf);
		if (json != NULL)
		{
			cJSON* command = cJSON_GetObjectItem(json, "command");
			if (strcmp(command->valuestring, CMD_SET) == 0)
			{
				cJSON* result = cJSON_GetObjectItem(json, "result");

				if (result != NULL)
				{
					if (result->valueint == 200)
					{
						ret = RC_OK;
					}
					else
					{
						ret = RC_ERROR;
					}
				}
				else ret = RC_UNKNOWN;

				cJSON_Delete(json);
				return ret;
			}
		}
		return RC_UNKNOWN;
	}
	return RC_TIMEOUT;
}

//v0.2.1
//设备定时调用，主要用于发生连接超时事件时，调用回调
VOID DeviceTick(void)
{
	EnterCriticalSection(&gCriticalSection);//进入全局临界区
	List p = gDeviceList;
	LPSR_DEVICE_ITEM d;
	while (p != NULL)
	{
		//List p1 = p->next;
		d = p->data;
		if ((d->nLoginStats == 2)&&(d->uTimeOut > 0) && ((GetTickCount() - d->uTickTimestamp) > 1000 * d->uTimeOut))
		{
			d->nLoginStats = 3;
			if (gfExceptionCallBack != NULL)
			{
				gfExceptionCallBack(MSGTYPE_TIMEOUT, d->nUserID, 0, &d);
			}
		}
		p = p->next;
	}
	LeaveCriticalSection(&gCriticalSection);//离开临界区
}