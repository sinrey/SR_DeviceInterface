#pragma once
#include "fifo.h"
#include "DeviceInterfaceDll.h"

#define RESULT_PROCESSING	100

#define RESULT_OK 200
#define RESULT_SOCKET_DISCONNECT  220	//���ӶϿ�
#define RESULT_SERVER_COMMAND 221	//����������µĶ���


#define RESULT_COMMAND_SIZE_TOO_LONG 400 //����̫������֧�֡�
#define RESULT_UNKNOWN_FORMAT 400	//����ʶ���﷨��ʽ
#define RESULT_REGISTER_FAULT 401
#define RESULT_BUSY	403				//�豸æ���޷�ִ�е�ǰ�������
#define RESULT_IDLE	403				//�豸���У��޷�ִ�е�ǰֹͣ���
#define RESULT_UNSUPPORT_COMMAND 404 //����ʶ������
#define RESULT_SERVER_UNREACHABLE 406 //���������ɴ�


#define RESULT_MODE_UNKNOWN 421		//��֧�ֵ�ģʽ
#define RESULT_UNSUPPORT_FORMAT 422 	//��֧����Ƶ��ʽ
#define RESULT_SAMPLERATE 423 		//��֧�ֵĲ�����
#define RESULT_MPEG_IN_DUPLEX 424	//��֧����˫��ģʽ��ʹ��mp3��Ƶ��ʽ
#define RESULT_UNKNOWN_OUTPUTPORT 425	//δ֪�������
#define RESULT_UNKNOWN_LEVEL 426		//δ֪�������ƽ
#define RESULT_UNKNOWN_PROTOCOL 427		//��֧�ֵ�Э��
#define RESULT_FILE_ERROR 428		//���ļ��쳣
#define RESULT_OPENFILE_ERROR 429		//���ļ��쳣
#define RESULT_FILETYPE_ERROR 430		//��֧�ֵ��ļ�����
#define RESULT_SESSION_ERROR  431		//ע�������Ҫ�ṩsession
#define RESULT_AUTH_FAULT  432			//��Ȩ��֤ʧ��

#define RESULT_UPDATE_FLASH	520		//����ʱ��flashд�����
#define RESULT_UPDATE_FILE	521		//����ʱ�������ļ������ϵ�ǰ�豸��
#define RESULT_UPDATE_CRC	522		//�����ļ�У���
#define RESULT_UPDATE_NO_FILE 523	//û������������

typedef struct DeviceItem
{
	CRITICAL_SECTION cs;//�ٽ����������̼߳�Խṹ�ڱ����ķ���ԭ�ӻ���
	UINT32 nUserID;//ϵͳΨһ���
	//ULONG uID;//�豸Ψһ���
	CHAR  uID[64];
	UINT32 nLoginStats;//��¼״̬ 0=δ��¼��1=���ӣ�2=��֤ͨ��
	SOCKET Sock;//���豸���ӵ�tcp socket
	UINT32 nPeerIp;//�豸ip��ַ
	UINT32 nPeerPort;//�豸�˿�
	UINT32 uTimeOut;//���߳�ʱʱ�䣬��λ�룬0��ʾ��ʹ�ó�ʱ�������ʱ���ͣ��ᴥ���ص�������//0.2.1
	UINT32 uTickTimestamp;//�ϴ��յ��豸���ݵ�ʱ�����//v0.2.1
	
	HANDLE hEvent;//�¼�	//Ӧ��ʹ��CreateEvent����һ���¼�����������룬���յ�Command�����Ӧʱ����Event����Ϊ��Ч��
	CHAR sCommand[128];//�����¼�
	CHAR* psCommandBuffer;//�¼�������ָ��,�����յ������ݻ��档
	UINT32 nCommandBufferSize;//�¼����ݻ���Ĵ�С
	UINT32 nRecvBytes;//���յ������ݳ���

	struct _Fifo Fifo;//�յ����ݵĻ���
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

