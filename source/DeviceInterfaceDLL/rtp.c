#include <windows.h>
#include "rtp.h"

//static unsigned int hwrcv_since_last_SR;
//static bool ssrc_set;
//static unsigned int rcv_ssrc;
void rtp_init(PRTP r,unsigned char payload)
{
	memset(r, 0, sizeof(RTP));
	r->payload = payload;
	r->rtp_starttime = GetTickCount();
	srand(r->rtp_starttime);
	r->rtp_ssrc = 0xFFFFFFFF * rand();
	r->rtp_snd_seq = 0;
	r->rtp_timestamp = 0;
	return;
}

int rtp_parse(PRTP r, char* buf,int len, char** dat, int* datlen)
{
	//int tpy;
	rtp_header_t *rtp = (rtp_header_t*)buf;
	if(rtp->version != 2)
	{
		return -1;
	}

	rtp->seq_number=ntohs(rtp->seq_number);
	rtp->timestamp=ntohl(rtp->timestamp);
	rtp->ssrc=ntohl(rtp->ssrc);
	//hwrcv_since_last_SR++;
	if (rtp->cc*sizeof(unsigned int) > (unsigned int) (len-RTP_FIXED_HEADER_SIZE)){
		//_printf("Receiving too short rtp packet.");
		return -2;
	}

	for (int i=0;i<rtp->cc;i++)
		rtp->csrc[i]=ntohl(rtp->csrc[i]);

	int header_size=RTP_FIXED_HEADER_SIZE+ (4*rtp->cc);

	//if((rtp->paytype != 0)&&(rtp->paytype != 9))
	//{
	//	return -3;
	//}
	*dat = buf+header_size;
	*datlen = len-header_size;
	return rtp->paytype;
}
/*
static unsigned int rtp_ssrc;
static unsigned short rtp_snd_seq;
static unsigned short rtp_timestamp;

int rtp_send_init(PRTP r)
{
	srand(GetTickCount());
	r->rtp_ssrc = 0xFFFFFFFF*rand();
	r->rtp_snd_seq = 0;
	r->rtp_timestamp = 0;
	return 0;
}
*/

int rtp_send_buf(PRTP r, char* sbuf, char* pdu,int len)
{
	rtp_header_t* rtp = (rtp_header_t*)sbuf;
	rtp->version = 2;
	rtp->padbit = 0;
	rtp->extbit = 0;
	rtp->markbit= 0;
	if(r->rtp_snd_seq == 0)rtp->markbit = 1;//4.0.4
	rtp->cc = 0;
	rtp->paytype = r->payload;// datatype;//session->snd.pt;
	rtp->ssrc = r->rtp_ssrc;//session->snd.ssrc;
	rtp->timestamp = ntohl(r->rtp_timestamp);	/* set later, when packet is sended */
	/* set a seq number */
	rtp->seq_number= ntohs(r->rtp_snd_seq);//session->rtp.snd_seq;

	memcpy(&sbuf[RTP_FIXED_HEADER_SIZE],pdu,len);
	r->rtp_snd_seq++;
	r->rtp_timestamp+=len; 
	
	return RTP_FIXED_HEADER_SIZE + len;
}
