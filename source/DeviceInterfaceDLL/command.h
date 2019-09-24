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

INT CommandSDCardGetInfo(PCHAR Out, INT OutSize);
INT CommandRegisterAck(PCHAR Out, INT OutSize, PCHAR session, PCHAR auth, INT err);
INT CommandSDCardGetFirstFile(PCHAR Out, INT OutSize);
INT CommandSDCardGetNextFile(PCHAR Out, INT OutSize);
INT CommandSDCardUploadFile(PCHAR Out, INT OutSize, INT DatPort, PCHAR sFileName, BOOL bCover);

INT CommandSDCardDeleteFile(PCHAR Out, INT OutSize, PCHAR sFileName);
INT CommandPlayFileStart(PCHAR Out, INT OutSize, INT DatPort, PCHAR sFileName, INT nVolume);
INT CommandPlayFileStop(PCHAR Out, INT OutSize);
INT CommandSDCardPlayFileStart(PCHAR Out, INT OutSize, PCHAR sFileName, INT nVolume);
INT CommandSDCardPlayFileStop(PCHAR Out, INT OutSize);
INT CommandSDCardPlayFileGetStatus(PCHAR Out, INT OutSize);
INT CommandIntercomStart(PCHAR Out, INT OutSize, PCHAR sTargetAddr, INT nTargetPort, PCHAR sStreamType, PCHAR sProtocol, INT nInputGain, PCHAR sInputSource, INT nVolume, PCHAR sAecMode, PCHAR sParam);

INT CommandPlayFileEmergencyStart(PCHAR Out, INT OutSize, PCHAR sTargetAddr, INT nTargetPort, PCHAR sStreamType, PCHAR sProtocol, INT nVolume, PCHAR sParam);
INT CommandPlayFileEmergencyStop(PCHAR Out, INT OutSize);