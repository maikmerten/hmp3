/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: 2024-05-13, Maik Merten
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

const char versionstring[24] = "5.2.3, 2024-04-26";

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
#include <stdint.h>
#include <inttypes.h>
#include <signal.h>

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

/* Windows Unicode file name support + platform specific helpers (like pipes requiring binary mode on Windows) */
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define _FN_USE_WCHAR
#include <windows.h>

#define setbinarymode(handle)	_setmode(_fileno(handle), O_BINARY)

#define _FN(name)	L##name
#define fn_char		wchar_t
#define fn_off_t	__int64
#define fn_fopen	_wfopen
#define fn_fseek	_fseeki64
#define fn_ftell	_ftelli64
#define fn_strcmp	wcscmp

static HANDLE get_console_handle()
{
	HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
	if (!hOut || hOut == INVALID_HANDLE_VALUE || GetFileType(hOut) != FILE_TYPE_CHAR) hOut = NULL;
	return hOut;
}

int con_printf(FILE *stream, const char *format, ...)
{
	int ret;
	va_list argptr;
	static HANDLE hConsoleOut = get_console_handle();

	va_start(argptr, format);

	if (hConsoleOut) { /* output to console */
		char buf[1024];

		ret = _vsnprintf(buf, sizeof(buf), format, argptr);

		if (ret > 0) {
			buf[sizeof(buf) - 1] = '\0'; /* ensure string is null-terminated */
			DWORD out;
			size_t len = strlen(buf);
			if (len >= sizeof(buf)) len = sizeof(buf) - 1;
			if (WriteConsoleA(hConsoleOut, buf, (DWORD)len, &out, NULL) != 0) {
				ret = (int)out;
			} else {
				ret = -1;
			}
		}
	} else {
		ret = vfprintf(stream, format, argptr);
	}

	va_end(argptr);

	return ret;
}

static int wprint_console(FILE *stream, const wchar_t *text, size_t len)
{
	DWORD out;

	if (stream == stdout) {
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut && hOut != INVALID_HANDLE_VALUE && GetFileType(hOut) == FILE_TYPE_CHAR) {
			if (WriteConsoleW(hOut, text, len, &out, NULL) == 0)
				return -1;
			return (int)out;
		}
	} else if (stream == stderr) {
		HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
		if (hErr && hErr != INVALID_HANDLE_VALUE && GetFileType(hErr) == FILE_TYPE_CHAR) {
			if (WriteConsoleW(hErr, text, len, &out, NULL) == 0)
				return -1;
			return (int)out;
		}
	}

	if (fputws(text, stream) < 0) /* if there's an error or output is redirected to a file, revert to basic fputs() */
		return -1;

	return len;
}

int fn_fprintf(FILE *stream, const fn_char *format, ...)
{
	int ret;
	va_list argptr;
	HANDLE hStream = NULL;

	va_start(argptr, format);

	if (stream == stdout)
		hStream = GetStdHandle(STD_OUTPUT_HANDLE);
	else if (stream == stderr)
		hStream = GetStdHandle(STD_ERROR_HANDLE);

	if (hStream && hStream != INVALID_HANDLE_VALUE && GetFileType(hStream) == FILE_TYPE_CHAR) { /* output to console */
		size_t buflen = 32768 + 80;
		fn_char *buf = (fn_char *)malloc(buflen * sizeof(fn_char));
		if (!buf) { /* try to allocate buffer long enough to hold max NTFS name + space for a line of text on console screen */
			buflen = 1024; /* if that fails for some reason, revert to 1 KB buffer */
			buf = (fn_char *)malloc(buflen * sizeof(fn_char));
			if (!buf) return -1;
		}

		ret = _vsnwprintf(buf, buflen, format, argptr);

		if (ret > 0) {
			buf[buflen - 1] = _FN('\0'); /* ensure string is null-terminated */
			size_t len = wcslen(buf);
			if (len >= buflen) len = buflen - 1;

			ret = wprint_console(stream, buf, len);
		}

		free(buf);
	} else {
		ret = vfwprintf(stream, format, argptr);
	}

	va_end(argptr);

	return ret;
}

#else /* #ifdef _WIN32 - platform specific code + unicode handling */

#define setbinarymode(handle)	((void)0)

#define _FN(name)	name
#define fn_char		char
#define fn_off_t	off_t
#define fn_fopen	fopen
#define fn_fseek	fseeko
#define fn_ftell	ftello
#define fn_fprintf	fprintf
#define fn_strcmp	strcmp
#define con_printf	fprintf

#endif /* #ifdef _WIN32 - platform specific code + unicode handling */

static const fn_char default_outfile[] = _FN("TEST.MP3");
static const fn_char default_file[] = _FN("TEST.WAV");
static const fn_char pipe_file[] = _FN("-");

// mpeg_select:   0 = track, 1 = mpeg1,  2 = mpeg2  x=output_freq
static int mpeg_select = 0;
static int mono_convert = 0;
static int XingHeadFlag = 0;
static int display_flag = 1;
static int ignore_length = 0;
static int ec_display_flag = 0;

static volatile int stop_encoder = 0;

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

/* buffer defined in terms of float for pcm convert (float>32) */

/********  pcm buffer ********/
//#define PCM_BUFBYTES  (6*sizeof(short)*2304)  /* 6 stereo frames */
#define PCM_BUFBYTES  (128*sizeof(float)*2304)  /* 128 stereo frames */
static unsigned char *pcm_buffer;
static int pcm_bufbytes;
static int pcm_bufptr;
static FILE *handle = NULL;

/****************************/
unsigned int ff_encode ( const fn_char *filename, const fn_char *fileout, E_CONTROL * ec );
int get_mnr_adjust ( const fn_char *fname, int mnr[21] );

int pcmhead ( FILE *handle, F_INFO * f_info, unsigned char *pcm_buffer, int *bytes_read, uint64_t *data_size );

int out_usage ( void );
int print_ec ( E_CONTROL * ec );

#ifdef _FN_USE_WCHAR
int get_wchar_commandline( wchar_t ***wargv )
{
	typedef int(__cdecl *wgetmainargs_t)(int *, wchar_t ***, wchar_t ***, int, int *);
	wgetmainargs_t wgetmainargs;
	HMODULE handle;
	int wargc, i;
	wchar_t **wenv;

	*wargv = NULL;
	if ((handle = LoadLibraryW(L"msvcrt.dll")) == NULL) return 0;
	if ((wgetmainargs = (wgetmainargs_t)GetProcAddress(handle, "__wgetmainargs")) == NULL) {
		FreeLibrary(handle);
		return 0;
	}
	i = 0;
	if (wgetmainargs(&wargc, wargv, &wenv, 0, &i) != 0) {
		FreeLibrary(handle);
		return 0;
	}
	return 1;
}
#endif

/*===============================================*/
// Handler for SIGINT (usually CTRL+c)
void handler_sigint(int sig)
{
	stop_encoder = 1;
}


/*===============================================*/
int
main ( int argc, char *argv_real[] )
{
	int i, k;
	E_CONTROL ec;
	unsigned int frames;
	const fn_char *filename;
	const fn_char *fileout;
	char **argv = argv_real;
#ifdef _FN_USE_WCHAR
	wchar_t **wargv = NULL;
	if ( !get_wchar_commandline( &wargv ) || !wargv ) {
		fprintf(stderr, "\n CANNOT RETRIEVE WCHAR COMMANDLINE");
		return 1;
	}
	fn_char **fn_argv = wargv;
#else
	fn_char **fn_argv = argv;
#endif

	fprintf (stderr, "\n hmp3 MPEG Layer III audio encoder %s"
		"\n Utilizing the Helix MP3 encoder, Copyright 1995-2005 RealNetworks, Inc."
		"\n\n With contributions from HydrogenAudio members."
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
		"\n           -D -IL -Qquick -Ffreq_limit -Ucpu_select -TXtest1"
		"\n           -SBTshort_block_threshold -EC"
		"\n           -h (detailed help)\n\n", versionstring );
	filename = default_file;
	fileout = default_outfile;

	signal(SIGINT, handler_sigint);

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
				filename = fn_argv[i];
			if (k == 1)
				fileout = fn_argv[i];
			k++;
			continue;
		}

		switch ( argv[i][1] )
		{
			case '\0':
				if ( k == 0 )
					filename = pipe_file;
				if ( k == 1 )
					fileout = pipe_file;
				k++;
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
					out_usage (  );        // help
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
				if ( ( argv[i][2] == 'l' ) || ( argv[i][2] == 'L' ) )
					ignore_length = 1;
				else
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
				get_mnr_adjust ( fn_argv[i] + 2, ec.mnr_adjust );
			break;

		}
	}

	if ( ec.mode == 3 )
		mono_convert = 1;

	if ( ec.bitrate < 0 )
		ec.vbr_flag = 1;
	else
		ec.vbr_flag = 0;

	fprintf (stderr, "\n  <press CTRL+c to stop encoder> \n" );

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

	if( frames == 0 )
		return 1; // return with error
	else
		return 0; // return normally
}

static void
print_progress(CMp3Enc *Encode, uint64_t in_bytes, unsigned int out_bytes, int progress)
{
	/*
		* progress indicator
		*     Frames  |  Bytes In  /  Bytes Out | Progress | Current/Average Bitrate
		*/
	char progress_str[12] = " N/A";
	char bytesin_str[16];

	if ( in_bytes < 10000000000000ULL ) { /* we can print 13 digits max */
		sprintf(bytesin_str, "%13" PRIu64 "", in_bytes);
	} else {
		double terabytes_in = (double)in_bytes / 1000000000000.0;
		double petabytes_in = terabytes_in / 1000.0;
		double exabytes_in = petabytes_in / 1000.0;
		if ( exabytes_in >= 1.0 ) {
			sprintf(bytesin_str, "%10.6f EB", exabytes_in);
		} else if ( petabytes_in >= 1.0 ) {
			sprintf(bytesin_str, "%10.6f PB", petabytes_in);
		} else {
			sprintf(bytesin_str, "%10.6f TB", terabytes_in);
		}
	}

	if ( progress >= 0 ) sprintf ( progress_str, "%3d%%", progress );
	con_printf (stderr, "\r  %10u  | %s / %10u |   %s   | %6.2f / %6.2f Kbps",
		Encode->L3_audio_encode_get_frames() + 1, bytesin_str, out_bytes, progress_str,
		Encode->L3_audio_encode_get_bitrate2_float (  ),
		Encode->L3_audio_encode_get_bitrate_float (  ) );
	fflush(stderr) ;
}

/*-------------------------------------------------------------*/
unsigned int
ff_encode ( const fn_char *filename, const fn_char *fileout, E_CONTROL *ec0 )
{
	int u;
	int nread, readbytes;
	int pcm_size_factor;
	unsigned int nwrite;
	uint64_t in_bytes;
	unsigned int out_bytes;
	F_INFO fi;
	int file_type, unsupported;
	E_CONTROL ec;
	MPEG_HEAD head;
	IN_OUT x;
	int bytes_in_init;
	char info_string[80];
	uint64_t audio_bytes = 0;
	int head_bytes = 0;
	int head_flags;
    unsigned short head_musiccrc = 0x0000;
	unsigned int frames;
	unsigned int frames_expected = 0;
	INT_PAIR fb;
	int toc_counter;
	int vbr_scale;
	CMp3Enc Encode;
	int nbytes_out[2];  // test reformatted frames, num bytes in each frame
	//unsigned char packet_buf[2000];     // reformatted frame buffer, test
	uint64_t indatasize = 0;
	bool ispad=false;

	nbytes_out[0] = nbytes_out[1] = 0;
	//memset ( packet_buf, 0, sizeof ( packet_buf ) );

/*------- copy control, may be changed ----*/
	ec = *ec0;

/*-----------------------*/
	fn_fprintf (stderr, _FN("\n  PCM input file: %s"), filename );
	fn_fprintf (stderr, _FN("\nMPEG output file: %s"), fileout );

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

	if ( fn_strcmp ( filename, pipe_file ) == 0 )
	{
		handle=stdin;
		setbinarymode(handle);
		ignore_length=1; /* automatically ignore source length info when encoding from pipe */
	} else {
		handle = fn_fopen ( filename, _FN("rb") );
		if ( handle == NULL )
		{
			fprintf (stderr, "\n CANNOT_OPEN_INPUT_FILE" );
			goto abort;
		}
	}

	/*
	* get the input wave file format
	*/

	file_type = pcmhead ( handle, &fi, pcm_buffer, &pcm_bufbytes, &indatasize );
	if ( file_type == 0 )
	{
		fprintf (stderr, "\n UNRECOGNIZED PCM FILE TYPE" );
		goto abort;
	}
	if ( ignore_length ) indatasize = UINT64_MAX;
	if ( indatasize == 0 )
	{
		fprintf ( stderr, "\n INPUT FILE CONTAINS NO AUDIO" );
		goto abort;
	}
	if ( indatasize == 0xFFFFFFFF && fn_strcmp ( filename, pipe_file ) != 0 )
	{
		fn_off_t size = -1;
		fn_off_t pos = fn_ftell ( handle );
		if ( pos != -1 ) {
			if ( fn_fseek ( handle, 0, SEEK_END ) == 0 ) {
				size = fn_ftell ( handle );
				if ( fn_fseek ( handle, pos, SEEK_SET ) != 0 )
					size = -1;
				if ( size != -1 ) size -= (pos - pcm_bufbytes); /* exclude header size */
			}
		}
		if ( size < 0 || size > indatasize ) {
			fprintf ( stderr, "\n THE INPUT FILE INDICATES MAX DATA SIZE BUT IS LARGER. THE WAV FILE IS INVALID." );
			fprintf ( stderr, "\n USE OPTION '-IL' TO ENCODE ANYWAY." );
			goto abort;
		}
	}
	if ( (uint64_t)pcm_bufbytes > indatasize ) pcm_bufbytes = (int)indatasize; /* make sure input doesn't read beyond data section */

	fprintf (stderr, "\n pcm file:  channels = %d  bits = %d,  rate = %d  type = %d",
		fi.channels, fi.bits, fi.rate, fi.type );

	/*
	* check if we can encode this file
	*/

	unsupported = 0 ;
	if (fi.channels < 1 || fi.channels > 2 ||
		fi.rate < 8000  || fi.rate > 48000 || (fi.type != 1 && fi.type != 3))
	{
		unsupported = 1;
	}

	if ( fi.type == 3 && fi.bits != 32 ) /* with floating point source only 32-bit input is supported */
		unsupported = 1;
	if ( fi.bits != 8 && fi.bits != 16 && fi.bits != 24 && fi.bits != 32 )
		unsupported = 1;

	if ( unsupported )
	{
		fprintf (stderr, "\n UNSUPPORTED PCM FILE TYPE\n" );
		if ( fi.channels < 1 || fi.channels > 2 )
			fprintf(stderr, " Only mono and stereo inputs supported.");
		if ( fi.rate < 8000 || fi.rate > 48000 )
			fprintf(stderr, " Supported sampling rates are 8000 Hz - 48000 Hz.");
		if ( !(((fi.type == 1) && (fi.bits == 8 || fi.bits == 16 || fi.bits == 24 || fi.bits == 32)) || ((fi.type == 3) && (fi.bits == 32))) )
			fprintf(stderr, " Only 8, 16, 24 and 32 bit linear PCM or 32-bit floating point supported.");
		goto abort;
	}

	pcm_size_factor = cvt_to_pcm_init ( fi.channels, fi.bits, fi.bigendian );


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
		Encode.MP3_audio_encode_init ( &ec, fi.bits, fi.type == 3, mpeg_select,
			mono_convert );

	if ( !bytes_in_init )
	{
		fprintf (stderr, "\n ENCODER INIT FAIL" );
		goto abort;
	}

	/*
	* create the MPEG output file
	*/

	if ( fn_strcmp ( fileout, pipe_file ) == 0 )
	{
		handout = stdout;
		setbinarymode(handout);
	} else
		handout = fn_fopen(fileout, _FN("w+b"));

	if ( handout == NULL )
	{
		fprintf(stderr, "\n CANNOT CREATE OUTPUT FILE");
		goto abort;
	}

	if ( pcm_bufbytes < bytes_in_init ) {
		readbytes = ((PCM_BUFBYTES - (PCM_BUFBYTES % pcm_size_factor)) - pcm_bufbytes); /* pcm_bufbytes needs to be divisible by frame size */
		if ( readbytes + pcm_bufbytes > indatasize ) readbytes = indatasize - pcm_bufbytes; /* make sure input doesn't read beyond data section */
		nread = fread ( pcm_buffer + pcm_bufbytes, 1, readbytes, handle );
		if (nread < 0) {
			fprintf(stderr, "\n FILE READ ERROR");
			goto abort;
		}
		pcm_bufbytes += nread;
	}

	pcm_bufbytes = cvt_to_pcm ( pcm_buffer, pcm_bufbytes );
	audio_bytes = pcm_bufbytes;

	/*
	* get and display info string
	*/

	Encode.L3_audio_encode_info_string ( info_string );
	fprintf (stderr, "\n %s", info_string );

	Encode.L3_audio_encode_info_ec ( &ec );    /* get actual encode settings */

	if ( ec_display_flag )
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
			0,0, ec.bitrate * fi.channels /* XingHeaderBitrateIndex( head.mode, ec.bitrate * fi.channels ) */ );

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
	fprintf (stderr, "\n      Frames  |     Bytes In  /  Bytes Out | Progress | Current/Average Bitrate");
	fprintf (stderr, "\n       None   |       None    /    None    |   None   | None");

	for ( u = 1;; u++ )
	{
		if ( pcm_bufbytes < bytes_in_init )
		{
			memmove ( pcm_buffer, pcm_buffer + pcm_bufptr, pcm_bufbytes );
			readbytes = ( ( PCM_BUFBYTES - (PCM_BUFBYTES % pcm_size_factor) ) - pcm_bufbytes ); /* pcm_bufbytes needs to be divisible by frame size */
			if ( readbytes + audio_bytes > indatasize ) readbytes = indatasize - audio_bytes; /* make sure input doesn't read beyond data section */
			nread = fread ( pcm_buffer + pcm_bufbytes, 1, readbytes, handle );

			if(nread > 0)
				audio_bytes += nread;

			if ( nread < 0 )
				break;  // read error

			if((feof(handle) || audio_bytes == indatasize)&&!ispad)
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
		if (in_bytes > indatasize) /* finalizing the encoder takes extra calls where no output is produced. And the encoder always handles full 1152 frames. This counter can go beyond the actual input data size */
			in_bytes = indatasize;

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
		 * bail out if SIGINT-handler signaled stop
		 */
		if ( stop_encoder )
			break;

		/*
		 * progress indicator
		 */
		if ( ( u & 511 ) == display_flag )
		{
			print_progress ( &Encode, in_bytes, out_bytes, (!ignore_length ? (int)(in_bytes * 100. / indatasize) : -1) );
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

	/* fprintf (stderr, "\r  %10u  | %10d / %10d |   %3d%%   | %6.2f / %6.2f  Kbps", */
	print_progress ( &Encode, in_bytes, out_bytes, ( !stop_encoder ? 100 : (int)(in_bytes*100. / indatasize) ) );

	fprintf (stderr, "\n-------------------------------------------------------------------------------");
	/* fprintf (stderr, "\n Compress Ratio %3.6f%%", out_bytes*100./indatasize ); */
	fprintf (stderr, "\n Compress Ratio ");
	if ( !ignore_length )
		fprintf (stderr, "%3.6f%%", (double)(out_bytes*100./indatasize) );
	else
		fprintf (stderr, "N/A");
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
			uint64_t samples_audio = audio_bytes / (fi.channels * (fi.bits / 8));
			frames = Encode.L3_audio_encode_get_frames (  );
			if ( stop_encoder ) samples_audio = 0;
			XingHeaderUpdateInfo ( frames, out_bytes, vbr_scale, NULL, bs_buffer, 0, 0, samples_audio, out_bytes, ec.freq_limit, fi.rate, ec.samprate, head_musiccrc );
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

	return Encode.L3_audio_encode_get_frames() + 1;
}

/*-------------------------------------------------------------*/
/* parse wave file header using pcmhead_file parser */
/*-------------------------------------------------------------*/

int pcmhead ( FILE *handle, F_INFO * f_info, unsigned char *pcm_buffer, int *bytes_read, uint64_t *data_size )
{
	int nread;
	
	unsigned char buf[2048];
	assert ( sizeof ( buf ) < PCM_BUFBYTES );

	nread = fread ( buf, 1, sizeof ( buf ), handle );
	
	if ( nread <= 0 )
		return 0;
	
	nread = pcmhead_file( handle, buf, nread, f_info, data_size );
	if ( !nread )
		return 0;

	*bytes_read = 0;
	if ( nread > 0 && nread < sizeof ( buf ) ) {
		memcpy ( pcm_buffer, buf + nread, sizeof ( buf ) - nread );
		*bytes_read = sizeof ( buf ) - nread;
	}
	return 1;
}

/*-------------------------------------------------------------*/
/* help text                                                   */
/*-------------------------------------------------------------*/
int
out_usage (  )
{
	fprintf (stderr,
	"\nB[bitrate]Per channel bitrate in kbits per second."
	"\n          Encoder will choose if -1. (default)"
	"\nM[mode]   Select encoding mode: mode-0 stereo=0 mode-1 stereo=1 dual=2 mono=3."
	"\nV[vbr_scale]"
	"\n          Selects vbr encoding and vbr scale.  Valid values are 0-150."
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
	"\nIL        Ignore source file length."
	"\nF         Limits encoded subbands to specified frequency, f24000"
	"\nHF        high frequency encoding. Allows coding above 16000Hz."
	"\n          hf1=(mode-1 granules), hf2=(all granules), -B96 or -V80 needed"
	"\nT         Bias for VBR quality scale, default 0. Valid values are -40-50."
	"\n          Can be used to reach higher or lower VBR bitrates than with the"
	"\n          base VBR scale -V"
	"\nTX        Band noise target adjustments (startup_adjustNT1B), default 6"
	"\n          6 or 8 presumed best"
	"\nSBT[short_block_threshold]"
	"\n          short_block_threshold, default = 700"
	"\n          Lower values mean increased sensitivity to transients."
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
get_mnr_adjust ( const fn_char *fname, int mnr[21] )
{
	FILE *stream;
	int m, n, i;

	stream = fn_fopen ( fname, _FN("rt") );
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
