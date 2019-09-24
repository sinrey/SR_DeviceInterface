/*                                                   Version: 2.00 - 01.Jul.95
  ============================================================================

                          U    U   GGG    SSSS  TTTTT
                          U    U  G       S       T
                          U    U  G  GG   SSSS    T
                          U    U  G   G       S   T
                           UUU     GG     SSS     T

                   ========================================
                    ITU-T - USER'S GROUP ON SOFTWARE TOOLS
                   ========================================


       =============================================================
       COPYRIGHT NOTE: This source code, and all of its derivations,
       is subject to the "ITU-T General Public License". Please have
       it  read  in    the  distribution  disk,   or  in  the  ITU-T
       Recommendation G.191 on "SOFTWARE TOOLS FOR SPEECH AND  AUDIO
       CODING STANDARDS". 
       ** This code has  (C) Copyright by CNET Lannion A TSS/CMC **
       =============================================================


MODULE:         USER-LEVEL FUNCTIONS FOR THE UGST G.722 MODULE

ORIGINAL BY:
   Simao Ferraz de Campos Neto
   COMSAT Laboratories                    Tel:    +1-301-428-4516
   22300 Comsat Drive                     Fax:    +1-301-428-9287
   Clarksburg MD 20871 - USA              E-mail: simao.campos@labs.comsat.com
    
   History:
   14.Mar.95    v1.0    Released for use ITU-T UGST software package Tool
                        based on the CNET's 07/01/90 version 2.00
   01.Jul.95    v2.0    Changed function declarations to work with 
                        many compilers; reformated <simao@ctd.comsat.com>
  ============================================================================
*/
#include "g722.h"

void g722_reset_encoder(encoder)
g722_state *encoder;
{

  Word16          xl, reset, il;
  Word16          xh, ih;

  reset=1;
  encoder->init_qmf_tx = 0;
  xl = xh = 0;
  il = lsbcod (xl, reset, encoder);
  ih = hsbcod (xh, reset, encoder);
}
/* .................... end of g722_reset_encoder() ....................... */


long g722_encode(incode,code,read1,encoder)
  short *incode;
  char *code;
  long read1;
  g722_state     *encoder;
{
  /* Encoder variables */
  Word16          xl, il;
  Word16          xh, ih;
  Word16          xin0, xin1;

  /* Auxiliary variables */
  long            iter;
  int             i, j;
  Word16          reset;

  /* Divide sample counter by 2 to account for QMF operation */
  read1 >>= 1;

  /* Main loop - never reset */
  for (reset=0, iter=0, i = 0, j = 0; i < read1; i++)
  {
    xin1 = incode[j++];
    xin0 = incode[j++];

    /* Calculation of the synthesis QMF samples */
    qmf_tx (xin0, xin1, &xl, &xh, encoder);

    /* Call the upper and lower band ADPCM encoders */
    il = lsbcod (xl, reset, encoder);
    ih = hsbcod (xh, reset, encoder);

    /* Mount the output G722 codeword: bits 0 to 5 are the lower-band
     * portion of the encoding, and bits 6 and 7 are the upper-band
     * portion of the encoding */
    code[i] = ((ih << 6) + il) & 0x00FF;

    /* Increase sample counter */
    iter++;
  }

  /* Return number of samples read */
  return(iter);
}
/* .................... end of g722_encode() .......................... */


void g722_reset_decoder(decoder)
g722_state *decoder;
{
  Word16          il, ih;
  Word16          rl, rh;
  Word16          reset;

  reset = 1;
  decoder->init_qmf_rx=0;
  il = ih = 0;
  rl = lsbdec (il, (Word16)0, reset, decoder);
  rh = hsbdec (ih, reset, decoder);
}
/* .................... end of g722_reset_decoder() ....................... */


long g722_decode(code,outcode,mode,read1,decoder)
  char *code;
  short *outcode, mode;
  long read1;
  g722_state     *decoder;
{
  /* Decoder variables */
  Word16          il, ih;
  Word16          rl, rh;
  Word16          reset;
  Word16          xout1, xout2;

  /* Auxiliary variables */
  long            iter;
  int             i, j;


    /* Decode - reset is never applied here */
    for (reset=0, iter=0, i = 0, j = 0; i < read1; i++)
    {
    /* Separate the input G722 codeword: bits 0 to 5 are the lower-band
     * portion of the encoding, and bits 6 and 7 are the upper-band
     * portion of the encoding */
      il = ((Word16) code[i]) & 0x003F;	/* 6 bits of low SB */
      ih = (((Word16) code[i]) >> 6) & 0x0003;/* 2 bits of high SB */

     /* Call the upper and lower band ADPCM decoders */
      rl = lsbdec (il, mode, reset, decoder);
      rh = hsbdec (ih, reset, decoder);

      /* Calculation of output samples from QMF filter */
      qmf_rx (rl, rh, &xout1, &xout2, decoder);

      /* Save samples in output vector */
      outcode[j++] = xout1;
      outcode[j++] = xout2;

      /* Increment sample counter */
      iter++;
    }

  /* Return number of samples read */
  return(j);
}
/* .................... end of g722_decode() .......................... */

