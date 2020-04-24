#pragma once
#include "fifo.h"
#include "DeviceInterfaceDll.h"

#define RESULT_PROCESSING	100

#define RESULT_OK 200
#define RESULT_SOCKET_DISCONNECT  220	//连接断开
#define RESULT_SERVER_COMMAND 221	//服务器命令导致的动作


#define RESULT_COMMAND_SIZE_TOO_LONG 400 //命令太长，不支持。
#define RESULT_UNKNOWN_FORMAT 400	//不认识的语法格式
#define RESULT_REGISTER_FAULT 401
#define RESULT_BUSY	403				//设备忙，无法执行当前启动命令。
#define RESULT_IDLE	403				//设备空闲，无法执行当前停止命令。
#define RESULT_UNSUPPORT_COMMAND 404 //不认识的命令
#define RESULT_SERVER_UNREACHABLE 406 //服务器不可达


#define RESULT_MODE_UNKNOWN 421		//不支持的模式
#define RESULT_UNSUPPORT_FORMAT 422 	//不支持音频格式
#define RESULT_SAMPLERATE 423 		//不支持的采样率
#define RESULT_MPEG_IN_DUPLEX 424	//不支持在双向模式下使用mp3音频格式
#define RESULT_UNKNOWN_OUTPUTPORT 425	//未知的输出口
#define RESULT_UNKNOWN_LEVEL 426		//未知的输出电平
#define RESULT_UNKNOWN_PROTOCOL 427		//不支持的协议
#define RESULT_FILE_ERROR 428		//打开文件异常
#define RESULT_OPENFILE_ERROR 429		//打开文件异常
#define RESULT_FILETYPE_ERROR 430		//不支持的文件类型
#define RESULT_SESSION_ERROR  431		//注册过程需要提供session
#define RESULT_AUTH_FAULT  432			//授权验证失败

#define RESULT_UPDATE_FLASH	520		//升级时，flash写入错误。
#define RESULT_UPDATE_FILE	521		//升级时，升级文件不符合当前设备。
#define RESULT_UPDATE_CRC	522		//升级文件校验错。
#define RESULT_UPDATE_NO_FILE 523	//没有下载升级包

typedef struct DeviceItem
{
	CRITICAL_SECTION cs;//临界区，用于线程间对结构内变量的访问原子化。
	UINT32 nUserID;//系统唯一序号
	//ULONG uID;//设备唯一序号
	CHAR  uID[64];
	UINT32 nLoginStats;//登录状态 0=未登录，1=连接，2=认证通过
	SOCKET Sock;//与设备连接的tcp socket
	UINT32 nPeerIp;//设备ip地址
	UINT32 nPeerPort;//设备端口
	UINT32 uTimeOut;//离线超时时间，单位秒，0表示不使用超时。如果超时发送，会触发回调函数。//0.2.1
	UINT32 uTickTimestamp;//上次收到设备数据的时间戳。//v0.2.1
	
	HANDLE hEvent;//事件	//应用使用CreateEvent创建一个事件句柄，并传入，当收到Command命令回应时，将Event设置为有效。
	CHAR sCommand[128];//监听事件
	CHAR* psCommandBuffer;//事件的数据指针,保存收到的数据缓存。
	UINT32 nCommandBufferSize;//事件数据缓存的大小
	UINT32 nRecvBytes;//接收到是数据长度

	struct _Fifo Fifo;//收到数据的缓存
	SR_USER_LOGIN_INFO LogInfo;
	SR_DEVICEINFO DevInfo;
}SR_DEVICE_ITEM, *LPSR_DEVICE_ITEM;

LPSR_DEVICE_ITEM DeviceAdd(LPSR_USER_LOGIN_INFO LogInfo, LPSR_DEVICEINFO DevInfo);
//LPSR_DEVICE_ITEM DeviceFindByAddr(char* addr);
//LPSR_DEVICE_ITEM DeviceFind(UINT32 lUserID);
VOID DeviceLock(LPSR_DEVICE_ITEM d);
VOID DeviceUnLock(LPSR_DEVICE_ITEM d);

LPSR_DEVICE_ITEM DeviceGetByAddr(char* addr);
LPSR_DEVICE_ITEM DeviceGet(UINT32 lUserID);
VOID DeviceRelease(LPSR_DEVICE_ITEM d);
VOID CommandProcess(LPSR_DEVICE_ITEM d);
VOID DeviceSetProbe(LPSR_DEVICE_ITEM d, HANDLE hEvent, CHAR* sCmd, CHAR* psBuffer, UINT32 nBufferSize);
VOID DeviceReleaseProbe(LPSR_DEVICE_ITEM d);

void DeviceRemoveAll();
INT DeviceRemove(UINT32 nUserID);

INT DeviceGetDiskInfo(UINT32 lUserID, UINT32* totalCapacity, UINT32* remainCapacity);
INT DeviceGetFirstFile(UINT32 lUserID, CHAR* sFileName, UINT32 nFileNameSize, UINT32* nFileSize);
INT DeviceGetNextFile(UINT32 lUserID, CHAR* sFileName, UINT32 nFileNameSize, UINT32* nFileSize);
INT DevicePlayFileStart(UINT32 lUserID, UINT32 nDataPort, CHAR* sFileName, INT nVolume);

INT DeviceDeleteFile(UINT32 lUserID, CHAR* sFileName);
INT DeviceSDCardPlayFileStart(UINT32 lUserID, CHAR* sFileName, INT nVolume);
INT DeviceSDCardPlayFileGetStatus(UINT32 lUserID, INT* runtime, INT* process);
INT DeviceSDCardPlayFileStop(UINT32 lUserID);

INT DeviceUploadFileStart(UINT32 lUserID, UINT32 nDataPort, CHAR* sFileName, BOOL bConver);

INT DeviceIntercomStart(UINT32 lUserID, CHAR* sTargetAddr, UINT32 nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nInputGain, CHAR* sInputSource, INT nVolume, CHAR* sAecMode, CHAR* sSession);
INT DeviceIntercomStop(UINT32 lUserID);

INT DeviceEmergencyPlayFileStart(UINT32 lUserID, CHAR* sTargetAddr, UINT32 nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nVolume, CHAR* sFileName);
INT DeviceEmergencyPlayFileStop(UINT32 lUserID);

INT DeviceUpdateStart(UINT32 lUserID, UINT32 nDataPort, INT nMode, CHAR* sFileName);

INT DeviceSetVolume(UINT32 lUserID, UINT32 nVolume);
INT DeviceApply(UINT32 lUserID);

