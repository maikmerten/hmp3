/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: mp3low.h,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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

#ifndef _MP3LOW_H_
#define _MP3LOW_H_

#ifdef __cplusplus
extern "C"
{
#endif
//----------------------------------------------  

    void L3_outbits_init ( unsigned char *outbuf );
    int L3_outbits_flush (  );
    void L3_outbits ( unsigned int x, int nbits );

    int setup_nband_L3 ( MPEG_HEAD * h );
    void sbt_init ( void );
    void xsbt_init ( void );
    void L3table_init ( int sr_index, int h_id, int band_limit );
    int L3freq_nearest_sf_band ( int sr_index_arg, int h_id_arg, int freq );

    void b3FFT_init (  );
    void xFFT_init (  );        // Pentium III sse

    void amod_init ( int f_select_arg, int nsb_arg, int npred_arg, int npolar_arg, int h_id_arg, int cpu_select, int nsum[],    // epart 
                     SPD_CNTL spd_cntl[65], float w_spd[],      // spd
                     SNR_TABLE2 tv[64], int *snr_npart, //  snr
                     MAP_TABLE smr_map[21] );   // smr

    void amod_initLong ( int f_select_arg, int nsb_arg, int h_id_arg, int nsumShort[],  // epart 
                         SPD_CNTL spd_cntlShort[65], float w_spdShort[] );
    void amod_initShort ( int f_select_arg, int nsb_arg, int h_id_arg, int nsumShort[], // epart 
                          SPD_CNTL spd_cntlShort[65], float w_spdShort[] );

    void bFFT1024RealHann ( float *input, float *fftbuf );
    void bFFT1024RealHannApprox ( float *input, float *fftbuf );
    void bFFT256RealHann ( float *input, float *fftbuf );

// Pentium III
    void xFFT1024RealHannApprox ( float *input, float *fftbuf );
    void xFFT256RealHann ( float *input, float *fftbuf );

    void aL3pred_shortGeo ( float *fftbuf_short,
                            float *pmeas, int npred_short, int npred_short0 );
    void aL3pred_longGeo ( float zgeobuf[2][2 * 256], float *fftbuf,
                           float *pmeas, int npred_long );;
// Pentium III sse preds
    void xL3pred_shortGeo ( float *fftbuf_short,
                            float *pmeas, int npred_short, int npred_short0 );
    void xL3pred_longGeo ( float zgeobuf[2][2 * 256], float *fftbuf,
                           float *pmeas, int npred_long );;

    void map_xform ( float x[], float rrx[], int npolar );
    void epart ( float rr[], float pmeas[], float e[][64], int nsum[68] );
    void spd ( float e[2][64], float ec[2][64], SPD_CNTL c[65], float w[] );
    void L3snrEcho ( float ec[2][64], SNR_TABLE2 tv[64],
                     float ecsave[2][64], int npart );
    void L3snr ( float ec[2][64], SNR_TABLE2 tv[64],
                 float ecsave[2][64], int npart );
    void L3smr ( float e[], float et[], SIG_MASK sm[], MAP_TABLE map[21] );
    void L3mask ( float et[], float mask[], MAP_TABLE map[21] );

// quick acoustic
    void emapLong ( float xr[], float e[64], int nsum[68] );
    void spd_smrLong ( float e[64], float esave[64],
                       SPD_CNTL c[65], float w[], SIG_MASK sm[] );
//void spd_smrLongEcho(float e[64], float esave[64],
//                  SPD_CNTL c[65], float w[],
//                  SIG_MASK sm[]);
    void spd_smrLongEcho ( float e[64], float esave[64],
                           SPD_CNTL c[65], float w[],
                           SIG_MASK sm[], int block_type );

// short block acoustic
//void emapShort(float xr[][192], float e[3][64], int nsum[68]);
    void emapShort ( float xr[], float e[3][64], int nsum[68] );
    void spd_smrShort ( float e[3][64], float ecsave[],
                        SPD_CNTL c[65], float w[],
                        SIG_MASK sm[], int block_type_prev );

    void sbt_L3 ( float *buf1, float *sample );
    void xsbt_L3 ( float *buf1, float *sample );
    void FreqInvert ( float *sample, int nsb_limit );
    void hybrid ( float *sample, float *sample_prev, float *xr,
                  int btype, int nlong, int ntot );
    void hybridLong ( float *sample, float *sample_prev, float *xr,
                      int btype, int nlong, int clear_flag );
    void hybridShort ( float *sample, float *sample_prev, float *xr, int n );
    void antialias ( float x[], int n );
    void xantialias ( float x[], int n );

    float *align16 ( float * );

//int ms_correlation(float x[2][576], int band_limit_stereo);
//int ms_correlation2(float x[2][576]);

    int L3_pack_sf_MPEG1 ( SCALEFACT * sf, int block_type );
    int L3_pack_sf_MPEG1B ( SCALEFACT * sf, int ch, int igr, int *pscfsi );
    int L3_pack_sf_MPEG1B2 ( SCALEFACT * sf, int ch, int igr,
                             int *pscfsi, int not_null );
    int L3_pack_sf_MPEG2 ( SCALEFACT * sf, int is_chan, int nsf_stereo,
                           int nsf_stereo_short, int block_type );
    int L3_pack_huff ( GR * gr_data, int *ix, unsigned char *ixsign );

    void L3_pack_side_MPEG1 ( unsigned char *bs_out,
                              SIDE_INFO * side_info, int nchan );
    void L3_pack_side_MPEG2 ( unsigned char *bs_out,
                              SIDE_INFO * side_info, int nchan, int igr );

    void mpeg_info_string ( MPEG_HEAD * h, char *s, E_CONTROL * ec );

    int setup_header ( E_CONTROL * ec, MPEG_HEAD * h );
    int setup_abcd ( MPEG_HEAD * h );
    int setup_nsb ( MPEG_HEAD * h );
    int setup_nbal ( MPEG_HEAD * h );
    int setup_samprate ( MPEG_HEAD * h );
    int setup_maxbits ( MPEG_HEAD * h, int totbitrate );

    void dummy (  );

    void attack_detect ( float pcm[],
                         int eng[], float filter[2], int flags[2] );
    void attack_detectSBT ( float sample[][576],
                            int eng[], float filter[2], int flags[2] );

    int attack_detect_igr ( float pcm[], int eng[], float filter[2] );

    int attack_detectSBT_igr ( float sample[576],
                               int eng[], int short_flag_prev );

    int attack_detectSBT_igr_MPEG2 ( float sample[576],
                                     int eng[], int short_flag_prev );

//------ test code
    void out_f ( float *x, int n );

//----------------------------------------------  
#ifdef __cplusplus
}
#endif

#endif   // _MP3LOW_H_
