#pragma once

int LogCreate();
int LogDestroy();
int LogErr(char* err);
int Log(char* info);
int LogErrWithCode(char* err, int errcode);
