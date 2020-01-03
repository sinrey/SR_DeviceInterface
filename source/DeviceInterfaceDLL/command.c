#include <windows.h>
#include "cJSON.h"
#include "command.h"

INT CommandSDCardGetInfo(CHAR* Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_GET_DISK_INFO);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize,jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardGetFirstFile(CHAR* Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_GET_FIRST_FILE);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardGetNextFile(CHAR* Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_GET_NEXT_FILE);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardUploadFile(CHAR* Out, INT OutSize, INT DatPort, CHAR* sFileName, BOOL bCover)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_UPLOAD_FILE);
	cJSON_AddStringToObject(jsonroot, "dataserver", "0.0.0.0");
	cJSON_AddNumberToObject(jsonroot, "dataserverport", DatPort);
	if(bCover)cJSON_AddStringToObject(jsonroot, "cover", "true");
	cJSON_AddStringToObject(jsonroot, "param", sFileName);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
	//
}

INT CommandSDCardDeleteFile(CHAR* Out, INT OutSize, CHAR* sFileName)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_DELETE_FILE);
	cJSON_AddStringToObject(jsonroot, "param", sFileName);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandPlayFileStart(CHAR* Out, INT OutSize, INT DatPort, CHAR* sFileName, INT nVolume)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_START);
	cJSON_AddStringToObject(jsonroot, "mode", MODE_RECVONLY);
	cJSON_AddStringToObject(jsonroot, "dataserver", "0.0.0.0");
	cJSON_AddNumberToObject(jsonroot, "dataserverport", DatPort);
	cJSON_AddStringToObject(jsonroot, "streamtype", "mp3");
	cJSON_AddNumberToObject(jsonroot, "volume", nVolume);
	cJSON_AddStringToObject(jsonroot, "param", sFileName);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandAudioStop(CHAR* Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_STOP);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandPlayFileStop(CHAR* Out, INT OutSize)
{
	return CommandAudioStop(Out, OutSize);
}

INT CommandPlayFileEmergencyStart(CHAR* Out, INT OutSize, CHAR* sTargetAddr, INT nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nVolume, CHAR* sParam)
{
	cJSON* jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_EMERGENCY_START);
	cJSON_AddStringToObject(jsonroot, "mode", MODE_RECVONLY);
	cJSON_AddStringToObject(jsonroot, "dataserver", sTargetAddr);
	cJSON_AddNumberToObject(jsonroot, "dataserverport", nTargetPort);
	cJSON_AddStringToObject(jsonroot, "streamtype", STREAMTYPE_G722);
	cJSON_AddStringToObject(jsonroot, "protocol", PROTOCOL_RTP);
	cJSON_AddNumberToObject(jsonroot, "volume", nVolume);
	cJSON_AddStringToObject(jsonroot, "param", sParam);

	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandPlayFileEmergencyStop(CHAR* Out, INT OutSize)
{
	cJSON* jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_EMERGENCY_STOP);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardPlayFileStart(CHAR* Out, INT OutSize, CHAR* sFileName, INT nVolume)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_START);
	cJSON_AddStringToObject(jsonroot, "mode", MODE_FILE);
	cJSON_AddNumberToObject(jsonroot, "volume", nVolume);
	cJSON_AddStringToObject(jsonroot, "param", sFileName);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardPlayFileStop(CHAR* Out, INT OutSize)
{
	return CommandAudioStop(Out, OutSize);
}

INT CommandSDCardPlayFileGetStatus(CHAR* Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_SDCARD_PLAYFILE_STATUS);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandIntercomStart(CHAR* Out, INT OutSize, CHAR* sTargetAddr, INT nTargetPort, CHAR* sStreamType, CHAR* sProtocol, INT nInputGain, CHAR* sInputSource, INT nVolume, CHAR* sAecMode, CHAR* sParam)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_START);
	cJSON_AddStringToObject(jsonroot, "mode", MODE_RECVSEND);
	cJSON_AddStringToObject(jsonroot, "dataserver", sTargetAddr);
	cJSON_AddNumberToObject(jsonroot, "dataserverport", nTargetPort);
	cJSON_AddStringToObject(jsonroot, "streamtype", sStreamType);
	cJSON_AddNumberToObject(jsonroot, "inputgain", nInputGain);
	cJSON_AddStringToObject(jsonroot, "inputsource", sInputSource);
	cJSON_AddStringToObject(jsonroot, "protocol", sProtocol);
	cJSON_AddStringToObject(jsonroot, "aecmode", sAecMode);
	cJSON_AddNumberToObject(jsonroot, "volume", nVolume);
	cJSON_AddStringToObject(jsonroot, "param", sParam);
	
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandIntercomStop(CHAR* Out, INT OutSize)
{
	return CommandAudioStop(Out, OutSize); 
}

INT CommandSetVolume(CHAR* Out, INT OutSize, UINT32 nVolume)
{
	cJSON* jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_SET);
	cJSON_AddNumberToObject(jsonroot, "volume", nVolume);

	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandUpdate(CHAR* Out, INT OutSize, INT nTargetPort, INT nMode, CHAR* sFilename)
{
	cJSON* jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_UPDATE);
	if(nMode == 0)cJSON_AddStringToObject(jsonroot, "mode", "bin");
	else if(nMode == 1)cJSON_AddStringToObject(jsonroot, "mode", "data");
	cJSON_AddStringToObject(jsonroot, "dataserver", "0.0.0.0");
	cJSON_AddNumberToObject(jsonroot, "dataserverport", nTargetPort);
	cJSON_AddStringToObject(jsonroot, "param", sFilename);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandApply(CHAR* Out, INT OutSize)
{
	cJSON* jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_APPLY);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandRegisterAck(CHAR* Out, INT OutSize, CHAR* session, CHAR* auth, INT err)
//char* CommandRegisterAck(char* session, char* auth, int err)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_REGISTER);
	if (session != NULL)cJSON_AddStringToObject(jsonroot, "session", session);
	if (auth != NULL)cJSON_AddStringToObject(jsonroot, "authentication", auth);
	cJSON_AddNumberToObject(jsonroot, "result", err);

	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = (INT)strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}