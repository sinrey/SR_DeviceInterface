#include <windows.h>
#include "cJSON.h"
#include "command.h"

INT CommandSDCardGetInfo(PCHAR Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_GET_DISK_INFO);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize,jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardGetFirstFile(PCHAR Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_GET_FIRST_FILE);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardGetNextFile(PCHAR Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_GET_NEXT_FILE);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardUploadFile(PCHAR Out, INT OutSize, INT DatPort, PCHAR sFileName, BOOL bCover)
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
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
	//
}

INT CommandSDCardDeleteFile(PCHAR Out, INT OutSize, PCHAR sFileName)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_DELETE_FILE);
	cJSON_AddStringToObject(jsonroot, "param", sFileName);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandPlayFileStart(PCHAR Out, INT OutSize, INT DatPort, PCHAR sFileName, INT nVolume)
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
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandAudioStop(PCHAR Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_STOP);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandPlayFileStop(PCHAR Out, INT OutSize)
{
	return CommandAudioStop(Out, OutSize);
}

INT CommandPlayFileEmergencyStart(PCHAR Out, INT OutSize, PCHAR sTargetAddr, INT nTargetPort, PCHAR sStreamType, PCHAR sProtocol, INT nVolume, PCHAR sParam)
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
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandPlayFileEmergencyStop(PCHAR Out, INT OutSize)
{
	cJSON* jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_EMERGENCY_STOP);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardPlayFileStart(PCHAR Out, INT OutSize, PCHAR sFileName, INT nVolume)
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
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandSDCardPlayFileStop(PCHAR Out, INT OutSize)
{
	return CommandAudioStop(Out, OutSize);
}

INT CommandSDCardPlayFileGetStatus(PCHAR Out, INT OutSize)
{
	cJSON * jsonroot = NULL;
	char* jsonout;
	jsonroot = cJSON_CreateObject();
	cJSON_AddStringToObject(jsonroot, "command", CMD_SDCARD_PLAYFILE_STATUS);
	jsonout = cJSON_Print(jsonroot);
	cJSON_Delete(jsonroot);
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandIntercomStart(PCHAR Out, INT OutSize, PCHAR sTargetAddr, INT nTargetPort, PCHAR sStreamType, PCHAR sProtocol, INT nInputGain, PCHAR sInputSource, INT nVolume, PCHAR sAecMode, PCHAR sParam)
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
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}

INT CommandIntercomStop(PCHAR Out, INT OutSize)
{
	return CommandAudioStop(Out, OutSize); 
}

INT CommandRegisterAck(PCHAR Out, INT OutSize, PCHAR session, PCHAR auth, INT err)
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
	INT jsonlength = strlen(jsonout);
	strncpy_s(Out, OutSize, jsonout, jsonlength);
	free(jsonout);
	return jsonlength;
}