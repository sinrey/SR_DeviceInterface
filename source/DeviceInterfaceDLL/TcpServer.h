#pragma once
#include <windows.h>

int TcpServerStart(UINT32 port);
void TcpServerStop();
int TcpServerSend(SOCKET sock, char* buf, int len);
int TcpServerSend_Block(SOCKET sock, char* buf, int len);
