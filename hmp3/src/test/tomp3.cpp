/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: 2022-12-19, Maik Merten
 *   
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.  
 *       
 * The contents of this file, and the files included with this file, 
 * are subject to the current version of the RealNetworks Public 
 * Source License (the "RPSL") available at 
 * http://www.helixcommunity.org/content/rpsl unless you have licensed 
 * the file under the current version of the RealNetworks Community 
 * Source License (the "RCSL") available at 
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL 
 * will apply. You may also obtain the license terms directly from 
 * RealNetworks.  You may not use this file except in compliance with 
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable 
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for 
 * the rights, obligations and limitations governing use of the 
 * contents of the file. 
 *   
 * This file is part of the Helix DNA Technology. RealNetworks is the 
 * developer of the Original Code and owns the copyrights in the 
 * portions it created. 
 *   
 * This file, and the files included with this file, is distributed 
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS 
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET 
 * ENJOYMENT OR NON-INFRINGEMENT. 
 *  
 * Technology Compatibility Kit Test Suite(s) Location:  
 *    http://www.helixcommunity.org/content/tck  
 *  
 * Contributor(s):  
 *   
 * ***** END LICENSE BLOCK ***** */

char versionstring[24] = "5.2.1, 2022-12-19";

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <fcntl.h>      /* file open flags */
#include <sys/types.h>  /* someone wants for port */
#include <sys/stat.h>   /* use forward slash for port */
#include <time.h>
#ifdef _MSC_VER
#include <io.h> /* lseek MC C++ 4.1 */
#else
#include <unistd.h>
#endif
#include <string.h>

#include "port.h"
#include "xhead.h"      /* Xing header */
#include "pcmhpm.h"     // parse wave file header

#include "mp3enc.h"     //  Mp3Enc Class

#if defined(_WIN32) && defined(_M_IX86) && !defined(WINCE)
/* timing test Pentium only */
//#define TIME_TEST
//#define ETIME_TEST
#endif

/*---------------------------------------*/

// mpeg_select:   0 = track, 1 = mpeg1,  2 = mpeg2  x=output_freq
static int mpeg_select = 0;
static int mono_convert = 0;
static int XingHeadFlag = 0;
static int display_flag = 1;
static int ec_display_flag = 0;

static char default_outfile[] = "TEST.MP3";
static char default_file[] = "TEST.WAV";

/*---- timing test Pentium only ---*/
#ifdef TIME_TEST
static double tot_cycles;
static int tot_cycles_n;
static unsigned int global_cycles;

void set_clock()
{
	global_cycles = clock();
}

void get_clock()
{
	global_cycles = clock() - global_cycles;
}
#endif

#ifdef ETIME_TEST
double clock64bit()
{
	return clock() / (double) CLOCKS_PER_SEC;
}
static double start, finish, dtime;
#endif

/*********  bitstream buffer */
//#define BS_BUFBYTES  32000
#define BS_BUFBYTES  (128*1024)
static unsigned char *bs_buffer;
static unsigned int bs_bufbytes;
static unsigned int bs_trigger = ( BS_BUFBYTES - 2880 );
static FILE *handout = NULL;

/********************** */

/* for down sample may need 3 of audio frames in pcm buffer */

/* buffer defined in terms of short for pcm convert (short>16) */

/********  pcm buffer ********/
//#define PCM_BUFBYTES  (6*sizeof(short)*2304)  /* 6 stereo frames */
#define PCM_BUFBYTES  (128*sizeof(short)*2304)  /* 128 stereo frames */
static unsigned char *pcm_buffer;
static int pcm_bufbytes;
static int pcm_bufptr;
static FILE *handle = NULL;

/****************************/
int ff_encode ( char *filename, char *fileout, E_CONTROL * ec );

int pcmhead ( FILE *handle, F_INFO * f_info );

int get_mnr_adjust ( char *fname, int mnr[21] );

int out_useage ( void );
int print_ec ( E_CONTROL * ec );

/*===============================================*/
int
main ( int argc, char *argv[] )
{
	int i, k;
	char *filename;
	char *fileout;
	E_CONTROL ec;
	int frames;
	fprintf (stderr, "\n hmp3 MPEG Layer III audio encoder %s"
		"\n Utilizing the Helix MP3 encoder, Copyright 1995-2005 RealNetworks, Inc."
		"\n"
		"\n Usage:  hmp3 <input> <output> [options]"
		"\n          <input> and/or <output> can be \"-\", which means stdin/stdout."
		"\n"
		"\n Example:"
		"\n         hmp3 input.wav output.mp3"
		"\n"
		"\n Options:"
		"\n           -Nnsbstereo -Sfilter_select -Aalgor_select"
		"\n           -C -X -O"
		"\n           -D -Qquick -Ffreq_limit -Ucpu_select -TXtest1"
		"\n           -SBTshort_block_threshold -EC"
		"\n           -h (detailed help)\n\n", versionstring );
	filename = default_file;
	fileout = default_outfile;

	ec.mode = 1;
	ec.bitrate = -1;    /* let encoder choose */
	ec.samprate = 44100;
	ec.nsbstereo = -1;  /* let encoder choose */
	ec.filter_select = -1;      /* let encoder choose */
	ec.nsb_limit = -1;
	ec.freq_limit = 24000;
	//ec.npass         = -1;         /* let encoder choose */
	//ec.npred         = -1;         /* encoder chooses */
	ec.cr_bit = 1;      /* copyright bit */
	ec.original = 1;    /* original/copy bit */
	ec.layer = 3;
	ec.hf_flag = 0;
	ec.vbr_flag = 1;
	ec.vbr_mnr = 50;    // each unit = 0.1 db or 10 mb
	ec.vbr_br_limit = 160;      // bitrate limit
	ec.chan_add_f0 = 24000;
	ec.chan_add_f1 = 24000;
	ec.sparse_scale = -1;       // reserved, set to -1
	ec.vbr_delta_mnr = 0;       // set 0 default
	ec.cpu_select = 0;
	ec.quick = -1;      // -1 let encoder choose, 1 = fast acoustic model
	ec.test1 = -1;      // set -1 for default, test reserved 6 or 8 seems best, Neal says 6
	ec.test2 = 0;       // reserved, set to 0
	ec.test3 = 0;       // reserved, set to 0
	ec.short_block_threshold = 700;
	for ( i = 0; i < 21; i++ )
		ec.mnr_adjust[i] = 0;   /* special, set 0 default */

	mpeg_select = 0;
	XingHeadFlag = (3 | INFOTAG_FLAG);	// write Xing header and info tag by default

/****** process command line args */
	for ( k = 0, i = 1; i < argc; i++ )
	{
		if ( argv[i][0] != '-' )
		{
			if ( k == 0 )
				filename = argv[i];
			if ( k == 1 )
				fileout = argv[i];
			k++;
			continue;
		}

		switch ( argv[i][1] )
		{
			case '\0':
				if ( k == 0 )
					filename = (char*) "-";
				if ( k == 1 )
					fileout = (char*) "-";
				k++;
				display_flag = -1;
				break;

			case 'e':
			case 'E':
				if ( ( argv[i][2] == 'c' ) || ( argv[i][2] == 'C' ) )
					ec_display_flag = 1;
				break;

			case 'h':
			case 'H':
				if ( ( argv[i][2] == 'f' ) || ( argv[i][2] == 'F' ) )
				{
					//ec.hf_flag  = 1;   // ms Layer III high freq
					//ec.hf_flag  = 3;   // ms+stereo+mono Layer III high freq
					ec.hf_flag = 1 | atoi ( argv[i] + 3 );
				}
				else
				{
					out_useage (  );        // help
					return 0;
				}
			break;

			case 'q':
			case 'Q':
				ec.quick = atoi ( argv[i] + 2 );
			break;

			case 'd':
			case 'D':
				display_flag = -1;
			break;

			case 'u':
			case 'U':
				ec.cpu_select = atoi ( argv[i] + 2 );
			break;

			case 'x':
			case 'X':
				XingHeadFlag = atoi ( argv[i] + 2 );
 				// TOC also needs Xing header bit enabled
				if(XingHeadFlag == 2) XingHeadFlag = 3;
			break;

			case 'b':
			case 'B':
				ec.bitrate = atoi ( argv[i] + 2 );
			break;

			case 'c':
			case 'C':       // copyright bit setting
				ec.cr_bit = atoi ( argv[i] + 2 );
			break;

			case 'o':
			case 'O':       // original/copy bit setting, 1=original
				ec.original = atoi ( argv[i] + 2 );
			break;

			case 'p':
			case 'P':
			break;

			case 'm':
			case 'M':
				ec.mode = atoi ( argv[i] + 2 );
			break;

			case 'n':
			case 'N':
				ec.nsbstereo = atoi ( argv[i] + 2 );
			break;

			case 's':
			case 'S':
				if ( ( argv[i][2] == 'b' ) || ( argv[i][2] == 'B' ) )
				{
					if ( ( argv[i][3] == 't' ) || ( argv[i][3] == 'T' ) )
					{
						ec.short_block_threshold = atoi ( argv[i] + 4 );
					}
				}
				else
				{
					ec.filter_select = atoi ( argv[i] + 2 );
				}
			break;

			case 'f':
			case 'F':
				ec.freq_limit = atoi ( argv[i] + 2 );
			break;

			//case 'l': case 'L':
				//ec.layer = atoi(argv[i]+2);
			//break;

			case 'z':
			case 'Z':
				//ec.npred = atoi(argv[i]+2);
			break;

			case 't':
			case 'T':
				if ( ( argv[i][2] == 'x' ) || ( argv[i][2] == 'X' ) )
					ec.test1 = atoi ( argv[i] + 3 );
				else
					ec.vbr_delta_mnr = atoi ( argv[i] + 2 );
			break;

			case 'i':
			case 'I':
				ec.chan_add_f0 = atoi ( argv[i] + 2 );
			break;

			case 'j':
			case 'J':
				ec.chan_add_f1 = atoi ( argv[i] + 2 );
			break;

			case 'v':
			case 'V':
				ec.vbr_flag = 1;    // Layer III vbr
				ec.vbr_mnr = atoi ( argv[i] + 2 );
			break;

			case 'l':
			case 'L':
				ec.vbr_br_limit = atoi ( argv[i] + 2 );
			break;

			case 'a':
			case 'A':
				mpeg_select = atoi ( argv[i] + 2 );
				if ( mpeg_select < 0 )
					mpeg_select = 0;
			break;

			case 'w':
			case 'W':
				get_mnr_adjust ( argv[i] + 2, ec.mnr_adjust );
			break;

		}
	}

	if ( ec.mode == 3 )
		mono_convert = 1;

	if ( ec.bitrate < 0 )
		ec.vbr_flag = 1;
	else
		ec.vbr_flag = 0;

	fprintf (stderr, "\n\n  <press any key to stop encoder>" );

/******** encode *********/
#ifdef ETIME_TEST
	start = clock64bit (  );
#endif

	if((frames = ff_encode ( filename, fileout, &ec ))!=1)
	{

#ifdef ETIME_TEST
		finish = clock64bit (  );
		dtime = ( finish - start );
		fprintf (stderr, "\n elapsed time seconds = %8.3lf       ms per frame = %8.3lf",
			dtime, 1000.0 * dtime / frames );
#endif

/*************************/

#ifdef TIME_TEST
		if ( tot_cycles_n )
		{
			// ms per frame
			tot_cycles = tot_cycles / tot_cycles_n;
			fprintf (stderr, "\n ave frame time ms = %8.8lf", tot_cycles );
		}
#endif

		fprintf (stderr, "\n" );
	}

	if( frames <= 0 )
		return 1; // return with error
	else
		return 0; // return normally
}

/*-------------------------------------------------------------*/
int
ff_encode ( char *filename, char *fileout, E_CONTROL * ec0 )
{
	int u;
	int nread, readbytes;
	int pcm_size_factor;
	unsigned int nwrite;
	int in_bytes, out_bytes;
	F_INFO fi;
	int file_type, unsupported;
	E_CONTROL ec;
	MPEG_HEAD head;
	int input_type;
	IN_OUT x;
	int bytes_in_init;
	char info_string[80];
	long audio_bytes = 0;
	int head_bytes;
	int head_flags;
    unsigned short head_musiccrc = 0x0000;
	int frames;
	int frames_expected;
	INT_PAIR fb;
	int toc_counter;
	int vbr_scale;
	CMp3Enc Encode;
	int nbytes_out[2];  // test reformatted frames, num bytes in each frame
	unsigned char packet_buf[2000];     // reformatted frame buffer, test
	int infilesize;
	bool ispad=false;

	nbytes_out[0] = nbytes_out[1] = 0;
	memset ( packet_buf, 0, sizeof ( packet_buf ) );

/*------- copy control, may be changed ----*/
	ec = *ec0;

/*-----------------------*/
	fprintf (stderr, "\n PCM input file: %s", filename );
	fprintf (stderr, "\nMPEG ouput file: %s", fileout );

/*-----------------------*/
	u = 0;
	vbr_scale = -1;
	toc_counter = 0;
	head_flags = 0;
	if ( XingHeadFlag )
		head_flags = FRAMES_FLAG | BYTES_FLAG | VBR_SCALE_FLAG;
	if ( XingHeadFlag & 2 )
		head_flags |= TOC_FLAG | INFOTAG_FLAG; // indicate toc and info tag
	in_bytes = out_bytes = 0;
	bs_buffer = NULL;
	bs_bufbytes = 0;
	pcm_buffer = NULL;
	pcm_bufbytes = 0;
	pcm_bufptr = 0;
	handle = NULL;
	handout = NULL;

	/*
	* allocate buffers
	*/

	bs_buffer = new unsigned char[BS_BUFBYTES];

	if ( bs_buffer == NULL )
	{
		fprintf (stderr, "\n CANNOT ALLOCATE BUFFER" );
		goto abort;
	}
	pcm_buffer = new unsigned char[PCM_BUFBYTES];

	if ( pcm_buffer == NULL )
	{
		fprintf (stderr, "\n CANNOT ALLOCATE BUFFER" );
		goto abort;
	}

	/*
	* open the input wave file
	*/

	if(strcmp(filename,"-")==0)
	{
		handle=stdin;
#ifdef _WIN32
		setmode( fileno( handle ), O_BINARY );
#endif
	}
	else
		handle = fopen ( filename, "rb" );
	if ( handle == NULL )
	{
		fprintf (stderr, "\n CANNOT_OPEN_INPUT_FILE" );
		goto abort;
	}

	infilesize=lseek(fileno(handle),0,SEEK_END);
	fseek(handle,0,SEEK_SET);

	/*
	* get the input wave file format
	*/

	file_type = pcmhead ( handle, &fi );
	if ( file_type == 0 )
	{
		fprintf (stderr, "\n UNRECOGNIZED PCM FILE TYPE" );
		goto abort;
	}

	fprintf (stderr, "\n pcm file:  channels = %d  bits = %d,  rate = %d  type = %d",
		fi.channels, fi.bits, fi.rate, fi.type );

	/*
	* check if we can encode this file
	*/

	unsupported = 0 ;
	if (fi.channels < 1 || fi.channels > 2 ||
		fi.rate < 8000  || fi.rate > 480000 || fi.type != 0)
	{
		unsupported = 1;
	}

	switch (fi.bits)
	{
		case 8:  input_type = 1 ; break ;
		case 16: input_type = 0 ; break ;
		default: unsupported = 1 ; break ;
	}

	if ( unsupported )
	{
		fprintf (stderr, "\n UNSUPPORTED PCM FILE TYPE" );
		goto abort;
	}

	pcm_size_factor = cvt_to_pcm_init ( fi.bits );

	/*
	* create the MPEG output file
	*/

	if(strcmp(fileout,"-")==0)
	{
		handout=stdout;
#ifdef _WIN32
		setmode( fileno( handout ), O_BINARY );
#endif
	}
	else
		handout = fopen ( fileout, "w+b" );

	if ( handout == NULL )
	{
		fprintf (stderr, "\n CANNOT CREATE OUTPUT FILE" );
		goto abort;
	}

	/*
	* initialize the encoder
	*/

	if ( ec.mode < 0 )
		ec.mode = 0; // default
	if ( fi.channels == 1 )
		ec.mode = 3;
	if ( fi.channels == 2 )
		if ( ec.mode == 3 )
			ec.mode = 1;
	ec.samprate = fi.rate;

	bytes_in_init =
		Encode.MP3_audio_encode_init ( &ec, input_type, mpeg_select,
			mono_convert );

	if ( !bytes_in_init )
	{
		fprintf (stderr, "\n ENCODER INIT FAIL" );
		goto abort;
	}

	/*
	* get and display info string
	*/

	Encode.L3_audio_encode_info_string ( info_string );
	fprintf (stderr, "\n %s", info_string );

	Encode.L3_audio_encode_info_ec ( &ec );    /* get actual encode settings */

	if(ec_display_flag)
		print_ec( &ec);  

	if ( XingHeadFlag )
	{
		/* get actual encode settings */
		Encode.L3_audio_encode_info_head ( &head );
		
		if ( ec.vbr_flag )
			vbr_scale = ec.vbr_mnr;
		
		head_bytes = XingHeader ( ec.samprate, head.mode,
			ec.cr_bit, ec.original, head_flags, 0, 0,
			vbr_scale, NULL, bs_buffer + bs_bufbytes,
			0,0, XingHeaderBitrateIndex( head.mode, ec.bitrate * fi.channels ) );

		bs_bufbytes += head_bytes;
		out_bytes += head_bytes;

        // flush buffer with Xing header now, so it won't interfere with
        // calculating MusicCRC later on.
        nwrite = fwrite ( bs_buffer, 1, bs_bufbytes, handout );
        if ( nwrite != bs_bufbytes )
		{
            fprintf (stderr, "\n FILE WRITE ERROR" );
            goto abort;
		}
        bs_bufbytes = 0;
	}

	fprintf (stderr, "\n" );

	/*
	* the encoding loop
	*/

	fprintf (stderr, "\n-------------------------------------------------------------------------------");
	fprintf (stderr, "\n  Frames  |  Bytes In  /  Bytes Out | Progress | Current/Average Bitrate");
	fprintf (stderr, "\n   None   |    None    /    None    |   None   | None");

	for ( u = 1;; u++ )
	{
		if ( pcm_bufbytes < bytes_in_init )
		{
			memmove ( pcm_buffer, pcm_buffer + pcm_bufptr, pcm_bufbytes );
			readbytes =
				( ( ( PCM_BUFBYTES -
				pcm_bufbytes ) << 1 ) / pcm_size_factor ) & ( ~1 );
			nread = fread ( pcm_buffer + pcm_bufbytes,1, readbytes ,handle);

			if(nread > 0)
				audio_bytes += nread;

			if ( nread < 0 )
				break;  // read error

			if(feof(handle)&&!ispad)
			{
				if( (((PCM_BUFBYTES << 1 ) / pcm_size_factor ) & ( ~1 )) > (nread+bytes_in_init*4))
				{
					memset(pcm_buffer+pcm_bufbytes+nread,0,bytes_in_init*4);
					nread+=bytes_in_init*4;
					ispad=true;
				}
			}
			nread = cvt_to_pcm ( pcm_buffer + pcm_bufbytes, nread );
			pcm_bufbytes += nread;
			pcm_bufptr = 0;

			if ( pcm_bufbytes < bytes_in_init )
				break;  /* eof */
		}
#ifdef TIME_TEST
		set_clock (  );
#endif
		x = Encode.MP3_audio_encode ( pcm_buffer + pcm_bufptr,
						bs_buffer + bs_bufbytes );
#ifdef TIME_TEST
		get_clock (  );
		tot_cycles += global_cycles;
		tot_cycles_n++;
#endif
		frames_expected++;
		pcm_bufbytes -= x.in_bytes;
		pcm_bufptr   += x.in_bytes;
	        bs_bufbytes  += x.out_bytes;

		if ( bs_bufbytes > bs_trigger )
		{
			nwrite = fwrite ( bs_buffer, 1, bs_bufbytes, handout );
			if ( nwrite != bs_bufbytes )
			{
				fprintf (stderr, "\n FILE WRITE ERROR" );
				break;
			}
            head_musiccrc = XingHeaderUpdateCRC(head_musiccrc, bs_buffer, bs_bufbytes);

			bs_bufbytes = 0;
		}

		in_bytes  += x.in_bytes;
		out_bytes += x.out_bytes;

		/*
		 * make entries into the TOC in the Xing Header
		 */
	
		if ( head_flags & TOC_FLAG )
		{
			toc_counter--;
			if ( toc_counter <= 0 )
			{
				fb = Encode.L3_audio_encode_get_frames_bytes (  );
				toc_counter = XingHeaderTOC ( fb.a + 1, fb.b + head_bytes );
			}
		}

		/*
		 * bail out if user hits key
		 */

		if ( kbhit (  ) )
			break;

		/*
		 * progress indicator
		 */
		if ( ( u & 127 ) == display_flag )
		{
			fprintf (stderr, "\r  %6d  | %10d / %10d |   %3d%%   | %6.2f / %6.2f Kbps",
				Encode.L3_audio_encode_get_frames() + 1, in_bytes, out_bytes, (int)(in_bytes*100./infilesize),
				Encode.L3_audio_encode_get_bitrate2_float (  ),
				Encode.L3_audio_encode_get_bitrate_float (  ) );
			fflush(stderr) ;
		}
	}

	/*
	* flush the bitstream buffer
	*/
	if ( bs_bufbytes > 0 )
	{
		nwrite = fwrite ( bs_buffer, 1, bs_bufbytes, handout );
		if ( nwrite != bs_bufbytes )
		{
			fprintf (stderr, "\n FILE WRITE ERROR" );
			goto abort;
		}
        head_musiccrc = XingHeaderUpdateCRC(head_musiccrc, bs_buffer, bs_bufbytes);
	}

	// Due to internal housekeeping, the encoder may not actually emit
	// a frame every call. To flush out all frames, encode additional
	// silent frames until we get get the expected number of frames.

	// write zero samples into the pcm buffer
	memset(pcm_buffer,0,bytes_in_init*4);

	if(fi.rate < 32000) // adjust for MPEG2
		frames_expected *= 2;

	while(Encode.L3_audio_encode_get_frames() < frames_expected) {
		x = Encode.MP3_audio_encode (pcm_buffer, bs_buffer);
		bs_bufbytes  = x.out_bytes;
		out_bytes += x.out_bytes;
		nwrite = fwrite ( bs_buffer, 1, bs_bufbytes, handout );
		if ( nwrite != bs_bufbytes )
		{
			fprintf (stderr, "\n FILE WRITE ERROR" );
			goto abort;
		}
        head_musiccrc = XingHeaderUpdateCRC(head_musiccrc, bs_buffer, bs_bufbytes);
	}

	fprintf (stderr, "\r  %6d  | %10d / %10d |   %3d%%   | %6.2f / %6.2f  Kbps",
		Encode.L3_audio_encode_get_frames() + 1, in_bytes, out_bytes, (int)(in_bytes*100./infilesize),
		Encode.L3_audio_encode_get_bitrate2_float (  ),
		Encode.L3_audio_encode_get_bitrate_float (  ) );
	fprintf (stderr, "\n-------------------------------------------------------------------------------");
	fprintf (stderr, "\n Compress Ratio %3.6f%%", out_bytes*100./infilesize );
	//printf(" %d", Encode.L3_audio_encode_get_frames() ); // actual frames in output


	/*
	* if we were writing a Xing header, go back and update that header now
	*/

	if ( XingHeadFlag )
	{
		fseek ( handout, 0, SEEK_SET );
		size_t read_res = fread ( bs_buffer, 1, head_bytes ,handout);
		if(read_res == (size_t) head_bytes)
		{
			unsigned long samples_audio = audio_bytes / (fi.channels * (fi.bits / 8));
			frames = Encode.L3_audio_encode_get_frames (  );
			XingHeaderUpdateInfo ( frames, out_bytes, vbr_scale, NULL, bs_buffer, 0, 0, samples_audio, out_bytes, ec.freq_limit, fi.rate, head_musiccrc );
			fseek ( handout, 0, SEEK_SET );
			fwrite ( bs_buffer, 1, head_bytes, handout );
		}
		else
		{
			fprintf (stderr, "\n READ UNEXPECTED NUMBER OF BYTES WRITING XING HEADER");
		}
	}

	/*------- optional info display ----*/
	//audio_encode_info_display();
	//audio_encode_info_display2();
	
	//Encode.out_stats();  // test test test
	
	abort:
	delete[] bs_buffer;
	delete[] pcm_buffer;
	if(handle != NULL)
		fclose ( handle );
	if(handout != NULL)
		fclose ( handout );
	while ( kbhit (  ) )
		getch (  );     /* purge key board buffer */
	return Encode.L3_audio_encode_get_frames() + 1;
}

/*-------------------------------------------------------------*/
/* parse wave file header using pcmhead_mem parser */
/*-------------------------------------------------------------*/

int pcmhead ( FILE *handle, F_INFO * f_info )
{
	int nread;
	
	unsigned char buf[256]; /* usually big enough */
	
	nread = fread ( buf,1, sizeof ( buf ) ,handle);
	
	if ( nread <= 0 )
		return 0;
	
	nread = pcmhead_mem ( buf, nread, f_info );
	if ( !nread )
		return 0;
	
	fseek ( handle, nread, 0 ); /* position at beginning of data */
	return 1;
}

/*-------------------------------------------------------------*/
/* help text                                                   */
/*-------------------------------------------------------------*/
int
out_useage (  )
{
	fprintf (stderr,
	//"\nlayer"
	//"\n        layer = 1 for Layer I, = 2 for Layer II, default = Layer III"
	"\nB[bitrate]Per channel bitrate in kbits per second."
	"\n          Encoder will choose if -1. (default)"
	"\nM[mode]   Select encoding mode: mode-0 stereo=0 mode-1 stereo=1 dual=2 mono=3."
	"\nV[vbr_scale]"
	"\n          Selects vbr encoding and vbr scale.  Valid values are 0-150."
	//"\nhf        Enables high frequency encoding (mode-1 stereo only)"
	//"\nhf2       Enables high frequency encoding (all modes)"
	"\nN[nsbstereo]"
	"\n          Applies to mode-1 stereo mode only.  Number of subbands to"
	"\n          encode in independent stereo.  Valid values are 4, 8, 12, and 16."
	"\n          The encoder limits choices to valid values.  The encoder"
	"\n          will make a default selection if nsbstereo = -1."
	"\n          Valid values for Layer III are 3-32."
	"\nS[filter_select]"
	"\n          Selects input filtering:  no filter = 0,  DC blocking filter = 1"
	"\n          if filter = -1 the encoder will choose (default)"
	"\nA[algor_select]  0 = track input, 1=MPEG-1, 2=MPEG-2, xxxxx=sample_rate"
	"\nC         c0 clear copyright bit, c1 set copyright bit"
	"\nO         o0=copy, o1=original"
	"\nX         -X1 MPEG compatible Xing header, -X2 with TOC (default), -X0 disable"
	"\nU         u0=generic, u2=Pentium III(SSE)"
	"\nQ         disable_taper, q0 = base, q1 = fast, q-1 = encoder chooses"
	"\nD         Don't display progress"
	"\nF         Limits encoded subbands to specified frequency, f24000"
	"\nHF        high frequency encoding. Allows coding above 16000Hz."
	"\n          hf1=(mode-1 granules), hf2=(all granules), -B96 or -V80 needed"
	"\nTX        tx6, test reserved 6 or 8 seems best (startup_adjustNT1B)"
	"\n            ** v5.0  TEST 1  as of 8/15/00"
	"\n            ** v5.0  TEST 2  8/18/00"
	"\n            ** v5.0  TEST 3  default tx6 (prev = tx8)"
	"\n            ** v5.0  TEST 4  mods to short fnc_sf, ms corr. hf enable > 80"
	"\n            ** v5.0  TEST 5  fix odd npart, ix clear"
	"\n            ** v5.0  TEST 6  add reformatted frames"
	"\n            ** v5.0  TEST 7  drop V4 amod"
	"\n            ** v5.1  2005.08.09 (see CVS log for details)"
	"\nSBT[short_block_threshold]"
	"\n          short_block_threshold default = 700"
	"\nEC        Display Encoder Setting\n" );

	return 0;
}

/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
int
print_ec ( E_CONTROL * ec )
{
	fprintf (stderr, "\n-------------------------------------------------------------------------------");
	fprintf (stderr, "\nec->layer =         %d", ec->layer );
	fprintf (stderr, "\t\t\tec->mode =          %d", ec->mode );
	fprintf (stderr, "\nec->bitrate =       %d", ec->bitrate );
	fprintf (stderr, "\t\t\tec->samprate =      %d", ec->samprate );
	fprintf (stderr, "\nec->nsbstereo =     %d", ec->nsbstereo );
	fprintf (stderr, "\t\t\tec->freq_limit =    %d", ec->freq_limit );
	fprintf (stderr, "\nec->filter_select = %d", ec->filter_select );
	fprintf (stderr, "\t\t\tec->nsb_limit =     %d", ec->nsb_limit );
	fprintf (stderr, "\nec->cr_bit =        %d", ec->cr_bit );
	fprintf (stderr, "\t\t\tec->original =      %d", ec->original );
	fprintf (stderr, "\nec->hf_flag =       %d", ec->hf_flag );
	fprintf (stderr, "\t\t\tec->vbr_flag =      %d", ec->vbr_flag );
	fprintf (stderr, "\nec->vbr_mnr =       %d", ec->vbr_mnr );
	fprintf (stderr, "\t\t\tec->vbr_br_limit =  %d", ec->vbr_br_limit );
	fprintf (stderr, "\nec->sparse_scale =  %d", ec->sparse_scale );
	fprintf (stderr, "\t\t\tec->chan_add_f0 =   %d", ec->chan_add_f0 );
	fprintf (stderr, "\nec->vbr_delta_mnr = %d", ec->vbr_delta_mnr );
	fprintf (stderr, "\t\t\tec->chan_add_f1 =   %d", ec->chan_add_f1 );
	fprintf (stderr, "\nec->quick =         %d", ec->quick );
	fprintf (stderr, "\t\t\tec->cpu_select =    %d", ec->cpu_select );
	fprintf (stderr, "\nec->test1 =         %d", ec->test1 );
	fprintf (stderr, "\t\t\tec->short_block_threshold = %d", ec->short_block_threshold );
	fprintf (stderr, "\n-------------------------------------------------------------------------------");

	return 0;
}

/*-------------------------------------------------------------*/
/* test function used for tuning                               */
/*-------------------------------------------------------------*/
int
get_mnr_adjust ( char *fname, int mnr[21] )
{
	FILE *stream;
	int m, n, i;

	stream = fopen ( fname, "rt" );
	if ( stream == NULL )
		return 1;

	for ( i = 0; i < 21; i++ )
		mnr[i] = 0;
	for ( i = 0; i < 21; i++ )
	{
		n = fscanf ( stream, "%d", &m );
		if ( n != 1 )
			break;
		mnr[i] = m;
	}
	for ( i = 0; i < 21; i++ )
	{
		mnr[i] = min ( mnr[i], 200 );
		mnr[i] = max ( mnr[i], -200 );
	}

	fprintf (stderr, "\nMNR adjust " );
	for ( i = 0; i < 21; i++ )
		fprintf (stderr, " %d", mnr[i] );

	fclose ( stream );
	return 0;
}
