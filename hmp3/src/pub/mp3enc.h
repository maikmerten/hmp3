/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: mp3enc.h,v 1.2 2005/08/09 20:43:45 karll Exp $ 
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

#ifndef _MP3ENC_H_
#define _MP3ENC_H_

// see encapp.h for encoder control structure

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "enc.h"
#include "l3e.h"
#include "amod.h"

#include "encapp.h"     // E_CONTROL structure, MPEG_HEAD structure

#include "hxtypes.h"

// BitAllo class
#include "bitallo1.h"
#include "bitallo3.h"

#include "filter2.h"

#include "srcc.h"       // sample rate converstion

typedef struct
{
    int a;
    int b;
}
INT_PAIR;

//===============================================================
class CMp3Enc
{

  public:
    CMp3Enc (  );

    //~CMp3Enc() {}
    ~CMp3Enc (  );

    //============================
    // basic encode, 16 bit input, no rate conversion
    //
    // initialize encoder, return is zero for failure 
    // see encapp.h for E_CONTROL structure
    int L3_audio_encode_init ( E_CONTROL * ec_arg );

    // Encode one MPEG Layer III frame.
    // Input (pcm[]) is 1152 pcm samples.  Oldest sample at 
    // lowest memory address.  Two channel modes are interleaved 
    // left/right for total of 2*1152 values.
    // pcm = short (not necessarly 16 bit).   
    // Output (bs_out) is encoded frame.  Returns 
    // bytes removed from pcm, number of bytes encoded.
    // Output bytes returned may be zero, may be more than one frame.
    IN_OUT L3_audio_encode ( short *pcm, unsigned char *bs_out );

    // encode reformatted frame (self contained frame) 
    // for streaming network packets
    // if bs_out != NULL bs_out returns standard bitstream
    // number of bytes given by IN_OUT.out_bytes.
    // if packet != NULL packet returns reformatted frame(s)
    // mpeg-1 returns one frame per call, mpeg-2 returns two frames
    // per call.  Number of bytes in each frame is given by
    // nbytes_out[0] and nbytes_out[1].  Second frame (for mpeg2) begins
    // at packet+nbytes_out[0].  Total bytes written to packet is 
    // nbytes_out[0]+nbytes_out[1].
    IN_OUT L3_audio_encode_Packet ( short *pcm,
                                    unsigned char *bs_out,
                                    unsigned char *packet,
                                    int nbytes_out[2] );

    //============================
    // encode with sample rate type conversion
    // 16 bit/ 8 bit input
    int MP3_audio_encode_init ( E_CONTROL * ec_arg, int input_type,     // 0=16 bit linear,1=8 bitlinear
                                int mpeg_select, int mono_convert );

    // standard encode
    IN_OUT MP3_audio_encode ( unsigned char *pcm, unsigned char *bs_out );

    // optional reformatted packet for streaming
    IN_OUT MP3_audio_encode_Packet ( unsigned char *pcm,
                                     unsigned char *bs_out,
                                     unsigned char *packet,
                                     int nbytes_out[2] );

    //=== public aux routines
    int L3_audio_encode_get_bitrate (  );
    float L3_audio_encode_get_bitrate_float (  );
    float L3_audio_encode_get_bitrate2_float (  );
    int L3_audio_encode_get_frames (  );
    void L3_audio_encode_info_ec ( E_CONTROL * ec );    /* info only */
    void L3_audio_encode_info_head ( MPEG_HEAD * head );        /* info only */
    void L3_audio_encode_info_string ( char *s );       /* info only */
    INT_PAIR L3_audio_encode_get_frames_bytes (  );

    //=== test routine
    void out_stats (  );

//-data------------------------------------------------
  private:

      MPEG_HEAD h;
    E_CONTROL ec_global;

//int iframe;

/*---- setup data -----*/

    int tot_frames_out;
    int tot_bytes_out;
    int ave_tot_bytes_out;      // running average

//int channel_adder_n;
//int channel_adder_nstart;

    int ivbr_min;       // min and max vbr bitrate_index
    int ivbr_max;
    int vbr_main_framebytes[16];
    int vbr_framebytes[16];

    int monodual;
    int nchan;
    int h_id;
    int is_flag;
    int ms_flag;
    int sr_index;
    int nband;
    int band_limit;
    int band_limit_stereo;
    int nsb;
    int stereo_flag;
    int nsb_limit;
    int nsb_limitMS[2];
    int nsbstereo;
    int nsbstereo_limit;
    int nsf_stereo;
    int AveTargetBits;
    int sf_bit_max;
    unsigned char head[4];
    int framebytes;
    int pad;
    int main_framebytes;
    int side_bytes;
    int padcount;
    int divisor;
    int remainder;
    int padbytes;
    int bytesout;
    int bytes_in;

/*--------------------------*/
    int totbitrate;
    int samprate;

/*--------------------------*/

/* acoustic model */
//int npolar;
//int npred;
//int npred_long;
//int npred_short;
//int npred_short0;

//float xfftbuf_short[2*3*256+4]; 
//typedef float FFTBUF_SHORT[3][256];
//FFTBUF_SHORT *fftbuf_short;
//float xpmeas[512+4];
//float *pmeas;

    float eng[3][64];   // sized [3] for use by short, [2] required for long
    int nsum[68];       // epart table
    SPD_CNTL spd_cntl[65];      // spd, spread fnc
    float w_spd[2200];  // spd, probably bigger that necessary
    float ecsave[2][2][64];     // snr  
//SNR_TABLE2 tv[64];          // snr.c
//int snr_npart;              // snr
//MAP_TABLE map[21];          // smr

    int nsumShort[68];  // epart table
    SPD_CNTL spd_cntlShort[65]; // spd, spread fnc
    float w_spdShort[1000];     // spd, probably bigger that necessary

//float zgeobuf[2][2][2*256];    // for long geo pred, size fixed in pred code
//float xzgeobuf[2*2*2*256+4];    // for long geo pred, size fixed in pred code
//typedef float ZGEOBUF[2][2*256];
//ZGEOBUF *zgeobuf;

//float rrx[512];
//float xfftbuf[1024+4];
//float *fftbuf;

// center points for FFT
    float *amod_audio_buf[2][2];

//---------- audio buffers
// buf increased one granule 3/14/00, to two granule 3/29/00
    float buf1[2192 + 1152];    // changed 8/13/98
    float buf2[2192 + 1152];
    float sample[2][4][576];    // [ch][igrx]  extended igrx for lookahead
// xr entire frame required for ms decision
// otherwise only channels  required
    float xr[2][2][576];        /* [gr][ch] */
    int ix[2][576];     /* [ch] quantized samples */
    unsigned char signx[2][576];        /* [ch] */
// transform pointers
    float *audio_buf[2][2];     /* [ch][gr] */

    int xr_clear_flag[2][2];    // indicates xr need to be cleared
//--------------------------
// sig_mask sized for use by short also, short needs
// sig_mask[2][3][12], 2 chan, 3 window, 12 sfbs
// long needs sig_mask[2][21]
// so sized sig_mask[2][36]
    SIG_MASK sig_mask[2][36];   // mask + sig by channel

    int byte_pool, byte_min, byte_max;
    int vbr_pool_target;        // 4/27/99
    int igr, igr_prev;

    int igrx;   // extended igr for sample transform 0/1/2/3

// attack detector
    int attack_buf[2][32];      // energy history two channels

//--------------------------
    SCALEFACT sf[2][2]; /* [gr][ch] */
    SIDE_INFO side_info;
//------- bistream stuff
    unsigned char mode_ext_buf[32];
    unsigned char br_index_buf[32];     // for vbr
    struct
    {
        unsigned int main_pos;
        signed int mf_bytes;    /* byte of main data in frame, includes pad */
    }
    frame_info[32];
    unsigned char side_buf[32][32];     /* 32 frames max 32 char each */
#define MB_TRIGGER (16*1024)
    unsigned char main_buf[MB_TRIGGER + 512 + 1440 + 256];
    unsigned int side_p0, side_p1;
    int main_p0, main_p1;

/* byte counters (unsigned, may wrap) */
    unsigned int main_tot;      /* total main data bytes encoded */
    unsigned int main_sent;     /* total main data bytes output */
    unsigned int mf_tot;        /* total main data frame bytes */
    signed int main_bytes;      /* current bytes in main data buf */

// encode selector
    int iL3_audio_encode_function;
// encode selector
    int iencode_function;
// filter 
    FILTER2_CONTROL fc2;

// cpu selection
    int cpu_select;

    int int_dummy;

// short_block_threshold 
    int short_block_threshold;

//-functions------------------------------------------------
  private:

      CBitAllo * BitAllo;

    void pack_head_local ( MPEG_HEAD * h, unsigned char *outbuf );
    int calc_freq_limit_L3 ( int samprate, int totbitrate, int mode );

    void gen_vbr_table ( int h_mode, int samplerate, int max_tot_bitrate );

/*-------------- encode function ----------------*/
    IN_OUT L3_audio_encode_MPEG1 ( short *pcm, unsigned char *bs_out );
    IN_OUT L3_audio_encode_vbr_MPEG1 ( short *pcm, unsigned char *bs_out );
    IN_OUT L3_audio_encode_MPEG2 ( short *pcm, unsigned char *bs_out );
    IN_OUT L3_audio_encode_vbr_MPEG2 ( short *pcm, unsigned char *bs_out );

    IN_OUT L3_audio_encode_MPEG1Packet ( short *pcm,
                                         unsigned char *bs_out,
                                         unsigned char *packet,
                                         int nbytes_out[2] );
    IN_OUT L3_audio_encode_vbr_MPEG1Packet ( short *pcm,
                                             unsigned char *bs_out,
                                             unsigned char *packet,
                                             int nbytes_out[2] );
    IN_OUT L3_audio_encode_MPEG2Packet ( short *pcm,
                                         unsigned char *bs_out,
                                         unsigned char *packet,
                                         int nbytes_out[2] );
    IN_OUT L3_audio_encode_vbr_MPEG2Packet ( short *pcm,
                                             unsigned char *bs_out,
                                             unsigned char *packet,
                                             int nbytes_out[2] );

/*-------------- base encode functions ----------------*/
    int encode_function (  );   // master
    int encode_jointA (  );
    int encode_jointB (  );
    int encode_singleA (  );
    int encode_singleB (  );    // cannot handle dual 10/14/98
    int encode_jointA_MPEG2 (  );
    int encode_jointB_MPEG2 (  );
    int encode_singleA_MPEG2 (  );
    int encode_singleB_MPEG2 (  );

// block type selection
    void blocktype_selectB_igr_dual ( int igr );        // uses sbt data
    void blocktype_selectB_igr_dual_MPEG2 ( int igr );  // uses sbt data
    void blocktype_selectB_igr_single ( int igr );      // uses sbt data
    void blocktype_selectB_igr_single_MPEG2 ( int igr );        // uses sbt data

    void acoustic_model ( int igr, int block_type = 0, int block_type_prev =
                          0 );
//void L3acoustic(float *pcm, int ch, int igr, 
//                int block_type, int block_type_prev);
//void xL3acoustic(float *pcm, int ch, int igr, 
//                 int block_type, int block_type_prev);   // Pentium III simd
    void L3acousticQuick ( int ch, int igr,
                           int block_type, int block_type_prev );

//--------------------
    void transform_igr ( int igr );
//void transform();
//void transform_MPEG2();

    int L3_pack_head ( unsigned char *bs_out, int pad_flag, int mode_ext );
    int L3_pack_head_vbr ( unsigned char *bs_out, int pad_flag,
                           int mode_ext );

//------------------- MP3_audio_encode (includes rate conversion)
    int src_encode;
    Csrc *src;
    short *src_pcmbuf;

};

#endif //  _MP3ENC_H_
