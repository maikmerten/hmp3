/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: encapp.h,v 1.2 2005/08/09 20:43:45 karll Exp $ 
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

#ifndef _ENCAPP_H_
#define _ENCAPP_H_

/*-------------- setup & control structure ---------------------*/
typedef struct
{
    int mode;   /* mode-0 stereo=0 mode-1 stereo=1 dual=2 mono=3 */
    int bitrate;        /* CBR PER CHANNEL bit rate, 1000's, default = -1 */
    int samprate;       /* samp rate e.g 44100 */
    int nsbstereo;      /* mode-1 only, stereo bands, 3-32 , for default set =-1 */
    int filter_select;  /* input filter selection - set to -1 for default */
    int freq_limit;     /* special purpose, set to 24000 */
    int nsb_limit;      /* special purpose, set to -1 */
//int npass;         /* not used for Layer III - set to -1 */
//int npred;         /* set to -1 */ 
    int layer;  /* 3=Layer III. Set to 3 */
    int cr_bit; /* header copyright bit setting */
    int original;       /* header original/copy bit setting, original=1 */
    int hf_flag;        /* MPEG1 high frequency */
    int vbr_flag;       /* 1=vbr, 0=cbr  */
    int vbr_mnr;        /* 0-150, suggested setting is 50 */
    int vbr_br_limit;   /* reserved for per chan vbr bitrate limit set to 160 */
    int vbr_delta_mnr;  /* special, set 0 default */
    int chan_add_f0;    /* channel adder start freq - set 24000 default */
    int chan_add_f1;    /* channel adder final freq - set 24000 default */
    int sparse_scale;   /* reserved set, to -1 (encoder chooses) */
    int mnr_adjust[21]; /* special, set 0 default */
    int cpu_select;     /* 0=generic, 1=3DNow reserved, 2=Pentium III */
    int quick;  /* 0 = base, 1 = fast, -1 = encoder chooses */
    int test1;  /* special test, set to -1 */
    int test2;  /* special test, set to 0 */
    int test3;  /* special test, set to 0 */
    int short_block_threshold;  /* short_block_threshold, default 700 */
}
E_CONTROL;

/*---------  E_CONTROL encoder init ----------------------
mode
    Select encoding mode:mode-0 stereo=0 mode-1 stereo=1 dual=2 mono=3.
    Two channel modes require two channel input.

bitrate
    Per channel bitrate in 1000's bits per second.
    Encoder will make a default selection if -1.
  
samprate
    Sample rate in samples per second (e.g. 44100).  Valid
    MPEG rates are 16000, 22050, 24000, 32000, 44100, and 48000.
    The encoder will encode at the nearest valid rate.

nsbstereo
    Applies to mode-1 stereo mode only. Controls the use of
    intensity stereo coding.  Number of subbands to
    encode in non-intensity stereo.  Valid values are 3-32.
    No intensity stereo will be coded if >= 30.
    The encoder limits choices to valid values.  The encoder
    will make a default selection if nsbstereo = -1.
    For MPEG2, should not be set lower that 8 except at
    very low bitrates.

filter_select
    Selects input filtering:  No filter = 0,  
    DC blocking filter = 1.  In general, the suggested setting 
    is 0 for professionally produced material.  The suggested
    setting is 1 for pcm captured by consumer quality sound
    cards.  If set to -1 encoder will select a default value.

npass
    Does not apply to Layer III.

npred
    Special purpose use only.  Sets number of frequency
    bins used in the acoustic model predictability measure
    computation. Set to -1.

freq_limit
    Special purpose use only.  Limits encoded subbands
    to specified frequency. Set to 24000. Same function as nsb_limit.

nsb_limit
    Limits number of subbands encoded.  For special purpose
    use only.  Set to -1.

hf_flag
    MPEG1 high frequency encoding.  Allows coding above 16000Hz 
    hf_flag=1 (mode-1 granules), hf=3 (all granules).

vbr_flag
    Select variable bitrate operation.  VBR=1, VBR=0 for CBR operation.

vbr_mnr;
    Valid values are 0-150.  Higher settings give higher SNR's
    and higher bitrates.  Suggested starting setting is 50

vbr_br_limit
    Per chan vbr bitrate limit (limitation is on br_index). 
    Special purpose. Set to 160.

----------------------------------------------------*/

/*-------------------------------------------------------------------*/

/* mpeg audio header   */
typedef struct
{
    int sync;
    int id;
    int option;
    int prot;
    int br_index;
    int sr_index;
    int pad;
    int private_bit;
    int mode;
    int mode_ext;
    int cr;
    int original;
    int emphasis;
}
MPEG_HEAD;

/*-------------------------------------------------------------------*/
typedef struct
{
    int in_bytes;
    int out_bytes;
}
IN_OUT;

#ifdef __cplusplus
extern "C"
{
#endif

/*xxxxxxxxxxxxx

// MP3_audio_encode adds sample rate/type conversion layer
int MP3_audio_encode_init( E_CONTROL *ec_arg, 
              int input_type, int mpeg_select, int mono_convert );
IN_OUT MP3_audio_encode(void *pcm, unsigned char *bs_out);

int L3_audio_encode_init( E_CONTROL *ec_arg);
IN_OUT L3_audio_encode(void *pcm, unsigned char *bs_out);

// aux routines
void L3_audio_encode_info_string(char *s);       // info only 
void L3_audio_encode_info_ec(E_CONTROL *ec);     // info only 
void L3_audio_encode_info_head(MPEG_HEAD *head); // info only 
int L3_audio_encode_get_bitrate();               // info only 
float L3_audio_encode_get_bitrate_float();       // info only 
float L3_audio_encode_get_bitrate2_float();      // info only 
int L3_audio_encode_get_frames();                // info only 

typedef struct {
    int a;
    int b;
} INT_PAIR;
INT_PAIR L3_audio_encode_get_frames_bytes();     //info only
// encoded trame count in x.a, encoded byte count in x.b

xxxxxxxxxx*/

#ifdef __cplusplus
}
#endif

#endif   //_ENCAPP_H_
