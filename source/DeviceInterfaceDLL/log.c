#include <stdio.h>
#include <windows.h>

static FILE* fpErrorLog;
static FILE* fpLog;
int LogCreate()
{
	char errlogfilename[512];
	char filename[512];

	GetModuleFileNameA(0, filename, sizeof(filename));
	memcpy(errlogfilename, filename, sizeof(errlogfilename));

	char* pstr = strrchr(filename, '\\');
	if (pstr != NULL)
	{
		filename[pstr - filename + 1] = '\0';
		strcat_s(filename,sizeof(filename), "log.txt");
	}
	else
	{
		strcpy_s(filename,sizeof(filename), "log.txt");
	}

	char* pstr2 = strrchr(errlogfilename, '\\');
	if (pstr2 != NULL)
	{
		errlogfilename[pstr2 - errlogfilename + 1] = '\0';
		strcat_s(errlogfilename, sizeof(errlogfilename), "errlog.txt");
	}
	else
	{
		strcpy_s(errlogfilename, sizeof(errlogfilename), "errlog.txt");
	}

	int ret;
	ret = fopen_s(&fpErrorLog, errlogfilename, "a");

	ret = fopen_s(&fpLog,filename, "w");

	return 0;
}

int LogDestroy()
{
	fclose(fpErrorLog);
	fclose(fpLog);
	return 0;
}

int LogErr(char* err)
{
	char buffer[1024];
	if (fpErrorLog != NULL)
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf_s(buffer, sizeof(buffer), "%02u%02u_%02u%02u%02u_%03u:%s\n",
			time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
			err);
		fwrite(buffer, 1, strlen(buffer), fpErrorLog);
		fflush(fpErrorLog);
	}
	return 0;
}

int LogErrWithCode(char* err, int errcode)
{
	char buffer[1024];
	if (fpErrorLog != NULL)
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf_s(buffer, sizeof(buffer), "%2u%2u_%2u%2u%2u_%3u:%s(errcode=%u)\n",
			time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
			err, errcode);
		fwrite(buffer, 1, strlen(buffer), fpErrorLog);
		fflush(fpErrorLog);
	}
	return 0;
}

int Log(char* info)
{
	char buffer[1024];
	if (fpLog != NULL)
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf_s(buffer, sizeof(buffer), "%02u%02u_%02u%02u%02u_%03u:%s\n",
			time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
			info);
		fwrite(buffer, 1, strlen(buffer), fpLog);
		fflush(fpLog);
	}
	return 0;
}

int LogWithText(char* info, char* text)
{
	char buffer[1024];
	if (fpLog != NULL)
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf_s(buffer, sizeof(buffer), "%2u%2u_%2u%2u%2u_%3u:%s(%s)\n",
			time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds,
			info, text);
		fwrite(buffer, 1, strlen(buffer), fpLog);
		fflush(fpLog);
	}
	return 0;
}