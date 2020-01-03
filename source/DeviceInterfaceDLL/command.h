#pragma once

#define CMD_REGISTER      "register"
#define CMD_GET_DISK_INFO "get_disk_info"
#define CMD_GET_FIRST_FILE "get_first_file"
#define CMD_GET_NEXT_FILE "get_next_file"
#define CMD_UPLOAD_FILE "write_file"
#define CMD_DELETE_FILE "delete_file"
#define CMD_START "start"
#define CMD_STOP "stop"
#define CMD_EMERGENCY_START "emergency_start"
#define CMD_EMERGENCY_STOP "emergency_stop"
#define CMD_SET "set"
#define CMD_UPDATE "update"
#define CMD_APPLY "apply"

#define CMD_SDCARD_PLAYFILE_STATUS "status_playfile"

#define MODE_RECVONLY "recvonly"
#define MODE_RECVSEND "recvsend"
#define MODE_FILE "file"

#define AECMODE_DISABLE "disable"
#define AECMODE_NORMAL "normal"
#define AECMODE_NOISY "noisy"

#define PROTOCOL_RTP "rtp"
#define PROTOCOL_TCP "tcp"

#define STREAMTYPE_G711U  "g711-u"
#define STREAMTYPE_G711A  "g711-a"
#define STREAMTYPE_G722 "g722"

#define INPUTSOURCE_MIC "mic"
#define INPUTSOURCE_LINEIN "linein"


#define INPUTSOURCE_MIC "mic"

INT CommandSDCardGetInfo(CHAR* Out, INT OutSize);
INT CommandRegisterAck(CHAR* Out, INT OutSize, CHAR* session, CHAR* auth, INT err);
INT CommandSDCardGetFirstFile(CHAR* Out, INT OutSize);
INT CommandSDCardGetNextFile(CHAR* Out, INT OutSize);
INT CommandSDCardUploadFile(CHAR* Out, INT OutSize, INT DatPort, CHAR* sFileName, BOOL bCover);

INT CommandSDCardDeleteFile(CHAR* Out, INT OutSize, CHAR* sFileName);
INT CommandPlayFileStart(CHAR* Out, INT OutSize, INT DatPort, CHAR* sFileName, INT nVolume);
INT CommandPlayFileStop(CHAR* Out, INT OutSize);
INT CommandSDCardPlayFileStart(CHAR* Out, INT OutSize, CHAR* sFileName, INT nVolume);
INT CommandSDCardPlayFileStop(CHAR* Out, INT OutSize);
INT CommandSDCardPlayFileGetStatus(CHAR* Out, INT OutSize);
INT CommandIntercomStart(CHAR* Out, INT OutSize, CHAR* sTargetAddr, INT nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nInputGain, CHAR* sInputSource, INT nVolume, CHAR* sAecMode, CHAR* sParam);

INT CommandPlayFileEmergencyStart(CHAR* Out, INT OutSize, CHAR* sTargetAddr, INT nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nVolume, CHAR* sParam);
INT CommandPlayFileEmergencyStop(CHAR* Out, INT OutSize);
INT CommandSetVolume(CHAR* Out, INT OutSize, UINT32 nVolume);
INT CommandUpdate(CHAR* Out, INT OutSize, INT nTargetPort, INT nMode, CHAR* sFilename);
INT CommandApply(CHAR* Out, INT OutSize);