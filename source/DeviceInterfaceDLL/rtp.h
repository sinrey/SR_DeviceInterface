#pragma once

#define PAYLOAD_G711U 0
#define PAYLOAD_G711A 8
#define PAYLOAD_G722 9

#define RTP_FIXED_HEADER_SIZE 12
typedef struct rtp_header
{
	unsigned short cc : 4;
	unsigned short extbit : 1;
	unsigned short padbit : 1;
	unsigned short version : 2;
	unsigned short paytype : 7;
	unsigned short markbit : 1;
	unsigned short seq_number;
	unsigned int timestamp;
	unsigned int ssrc;
	unsigned int csrc[16];
} rtp_header_t;

typedef struct
{
	unsigned char payload;//rtp 的负载类型
	unsigned int rtp_ssrc;//rtp id
	unsigned short rtp_snd_seq;//rtp发送序号
	unsigned int rtp_timestamp;//rtp时间戳
	unsigned int rtp_starttime;//rtp开始时间
}RTP, * PRTP;

void rtp_init(PRTP r, unsigned char payload);
int rtp_parse(PRTP r, char* buf, int len, char** dat, int* datlen);
int rtp_send_buf(PRTP r, char* sbuf, char* pdu, int len);
