/*                                                          v2.0 - 01/Jul/1995
  ============================================================================

  ENCG722.C 
  ~~~~~~~~~

  Description: 
  ~~~~~~~~~~~~
  
  Demonstration program for UGST/ITU-T G.722 module with the G.722 
  encoding function.

  Input data is supposed to be aligned at word boundaries, i.e.,
  organized in 16-bit words, following the operating system normal
  organization (low-byte first for VMS and DOS; high byte first for most
  Unix systems). Input linear samples are supposed to be 16-bit right-adjusted
  and the 7kHz ADPCM bitstream is left-adjusted, i.e., the codewords are 
  located in the lower 8-bits of the encoded bitstream file. The MSB is
  always 0 for the bitstream file.
  
  Usage:
  ~~~~~~
  $ ENCG726 [-options] InpFile OutFile 
             [FrameSize [1stBlock [NoOfBlocks [Reset]]]]
  where:
  InpFile     is the name of the file to be processed;
  OutFile     is the name with the processed data;
  FrameSize   is the frame size, in number of samples; the bitrate 
              will only change in the boundaries of a frame 
              [default: 16 samples]
  1stBlock    is the number of the first block of the input file
              to be processed [default: 1st block]
  NoOfBlocks  is the number of blocks to be processed, starting on
    	      block "1stBlock" [default: all blocks]

  Options:
  -frame #    Number of samples per frame for switching bit rates.
              Default is 16 samples (or 2ms) 
  -noreset    don't apply reset to the encoder/decoder
  -?/-help    print help message

  Original author:
  ~~~~~~~~~~~~~~~~
  J-P PETIT 
  CNET - Centre Lannion A
  LAA-TSS                         Tel: +33-96-05-39-41
  Route de Tregastel - BP 40      Fax: +33-96-05-13-16
  F-22301 Lannion CEDEX           Email: petitjp@lannion.cnet.fr
  FRANCE
    
  History:
  ~~~~~~~~
  14.Mar.95    v1.0    Released for use ITU-T UGST software package Tool
                       based on the CNET's 07/01/90 version 2.00
  01.Jul.95    v2.0    Changed function declarations to work with 
                       many compilers; reformated <simao@ctd.comsat.com>

  ============================================================================
*/


/* Standard prototypes */
#include <stdio.h>
#include <stdlib.h>

#ifdef VMS
#include <stat.h>
#else
#include <sys/stat.h>
#endif

/* G.722- and UGST-specific prototypes */
#include "g722.h"
#include "ugstdemo.h"

void display_usage ARGS((void));

/*
 -------------------------------------------------------------------------
 void display_usage(void);
 ~~~~~~~~~~~~~~~~~~
 Display proper usage for the demo program. Generated automatically from
 program documentation.

 History:
 ~~~~~~~~
 01.Jul.95 v1.0 Created <simao>.
 -------------------------------------------------------------------------
*/
#define P(x) printf x
void display_usage()
{
  /* Print Message */
  printf ("\n\n");
  printf ("\n***************************************************************");
  printf ("\n* PROCESS OF THE ENCODER OF ITU-T G.722 WIDEBAND SPECH CODER  *");
  printf ("\n* COPYRIGHT CNET LANNION A TSS/CMC. Date: 24/Aug/90           *");
  printf ("\n***************************************************************");

  /* Quit program */
  P(("USAGE: encg722 file.inp file.adp (all binary files).\n"));
  exit(-128);
}
#undef P
/* .................... End of display_usage() ........................... */


/*
   **************************************************************************
   ***                                                                    ***
   ***        Demo-Program for testing the correct implementation         ***
   ***               and to show how to use the programs                  ***
   ***                                                                    ***
   **************************************************************************
*/
int             main (argc, argv)
int argc; 
char *argv[];
{
  /* Local Declarations */
  Word16          reset = 1;
  g722_state      encoder;

  /* Encode and decode operation specification */
  char encode=1, decode=0;

  /* Sample buffers */
  short   code[8192];
  short   incode[16384];

  /* File variables */
  char            FileIn[80], FileOut[80];
  FILE           *xmt, *cod;
  long            iter=0;
  long            N=16384, N1=1, N2=0;
  int             read1;
  long            start_byte;
#ifdef VMS
  char            mrs[15];
#endif

  /* Progress flag indicator */
  static char     quiet=0, funny[9] = "|/-\\|/-\\";

 
  /* *** ......... PARAMETERS FOR PROCESSING ......... *** */

  /* GETTING OPTIONS */

  if (argc < 2)
    display_usage();
  else
  {
    while (argc > 1 && argv[1][0] == '-')
      if (strcmp(argv[1], "-noreset") == 0)
      {
	/* No reset */
	reset = 0;

	/* Update argc/argv to next valid option/argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-enc") == 0)
      {
	/* Encoder-only operation */
	encode = 1;
	decode = 0;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-dec") == 0)
      {
	/*Decoder-only operation */
	encode = 0;
	decode = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-frame") == 0)
      {
	/* Define Frame size for rate change during operation */
        N = atoi(argv[2]);

	/* Move argv over the option to the next argument */
	argv+=2;
	argc-=2;
      }
      else if (strcmp(argv[1], "-q") == 0)
      {
	/* Don't print progress indicator */
	quiet = 1;

	/* Move argv over the option to the next argument */
	argv++;
	argc--;
      }
      else if (strcmp(argv[1], "-?") == 0 || strcmp(argv[1], "-help") == 0)
      {
	/* Print help */
	display_usage();
      }
      else
      {
	fprintf(stderr, "ERROR! Invalid option \"%s\" in command line\n\n",
		argv[1]);
	display_usage();
      }
  }

  /* Now get regular parameters */
  GET_PAR_S(1, "_Input File: .................. ", FileIn);
  GET_PAR_S(2, "_Output File: ................. ", FileOut);
  FIND_PAR_L(3, "_Block Size: .................. ", N, N);
  FIND_PAR_L(4, "_Starting Block: .............. ", N1, N1);
  FIND_PAR_L(5, "_No. of Blocks: ............... ", N2, N2);

  /* Find starting byte in file */
  start_byte = sizeof(short) * (long) (--N1) * (long) N;

  /* Check if is to process the whole file */
  if (N2 == 0)
  {
    struct stat     st;

    /* ... find the input file size ... */
    stat(FileIn, &st);
    N2 = (st.st_size - start_byte) / (N * sizeof(short));
  }

  /* Open input file */
  if ((xmt = fopen (FileIn, RB)) == NULL)
    KILL(FileIn, -2);

  /* Open output file */
  if ((cod = fopen (FileOut, WB)) == NULL)
    KILL(FileOut, -2);

  /* Reset lower and upper band encoders */
  g722_reset_encoder(&encoder);

  /* Read samples from input file and process */
  while ((read1 = fread ((char *) incode, sizeof (short), 16384, xmt)) != 0)
  {
    /* print progress flag */
    if (!quiet)
      fprintf(stderr, "%c\r", funny[(iter/read1) % 8]);

    /* Encode */
    iter += g722_encode(incode, code, read1, &encoder);

    /* Save encoded samples */
    if (fwrite ((char *) code, sizeof (short), read1/2, cod) != read1/2)
      KILL(FileOut,-4);
  }

  /* Close input and output files */
  fclose(xmt);
  fclose(cod);

  /* Exit with success for non-vms systems */
#ifndef VMS
  return (0);
#endif
}
/* ............................. end of main() ............................. */

