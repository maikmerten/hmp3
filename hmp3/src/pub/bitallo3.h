/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallo3.h,v 1.1 2005/07/13 17:22:24 rggammon Exp $ 
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

#ifndef _BITALLO3_H_
#define _BITALLO3_H_

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "l3e.h"
#include "bitallo.h"
#include "bitallos.h"   // short block allocation

#define DISABLE_BA_TAPER

#define g_offset 8
// caution g_offset also defined in L3math!!!

// delta from gzero to gmin,
// theoretical value is 73+, 70 allows for rounding in gsf -> sf
#define GMIN_OFFSET 70

// per ch igr bits limited by part2_3length
#define PART23  4021

//=========================================================
class CBitAllo3:public CBitAllo
{

  public:
    CBitAllo3 (  );

//    ~CBitAllo3() {}
    ~CBitAllo3 (  );

    void BitAllo ( float xr_arg[][576], SIG_MASK sm_arg[][36],
                   int ch_arg, int nchan_arg,
                   int min_bits_arg, int target_bits_arg, int max_bits_arg,
                   int bit_pool_arg,
                   SCALEFACT sf_out[], GR gr_data[],
                   int ix_arg[][576],
                   unsigned char signx_arg[][576], int ms_flag_arg );

    int BitAlloInit ( BA_CONTROL & ba_control );

    int ms_correlation2 ( float x[2][576], int block_type = 0 );
    int ms_correlation201 ( float x[2][576], int block_type = 0 );
    int ms_correlation202 ( float x[2][576], int block_type = 0 );

    void ba_out_stats (  );     // test routine

//-data------------------------------------------------
  private:

    int hq_flag;
    int hf_flag;
    int vbr_flag;
    int hf_quant;
    int hf_quant_stereo[2];
    int gsf_hf;
    int gsf_hf_stereo[2];

    float gz_con1B;
    float gz_con2B;

    int totBits;
    int ms_count;
    int call_count;
    int nchan;
    int is_flag;
    int ms_flag;
    int ill_is_pos;
    int h_id;

    int maxBits;        /* must not exceed */
    int maxTargetBits;
    int minTargetBits;  /* maybe wasted if less */
    int PoolBits;
    int PoolFraction;
    int max_cnt_bits;
    int TargetBits;
    int MNR;    // long term MNR target  in mb
    int deltaMNR;       // 
    int initialMNR;
//int bt1MNR;
    int activeBands;

//int nsf[2];       // in base class
    int nsf2[2];
    int nsf3[2];        // stereo hf allo 

//int nBand_l[22];   // in base class
//int startBand_l[24];
//int nBand_s[12];
//int startBand_s[13];

    int nbmax[2];
    int nbmax2[2];
    int nbmax3[2];
    int huff_bits[2];

    int taperNT[22];

    float rnBand_l[22]; // 1.0/nBand_l

    float dGdB; /* delta G/delta bits, dG = dGdB*dB */

    typedef float ARRAY576[576];
    ARRAY576 *xr;
    typedef int intARRAY576[576];
    intARRAY576 *ix;
    typedef unsigned char ucharARRAY576[576];
    ucharARRAY576 *signx;

    int look_log_cbwmb[21 + 1]; // log(sf band width) in mb

    int snr[2][22];
    float xsxx[2][22];
    float xsxxms[2][22];
    int Noise0[2][22];  // Noise at no-allo (signal)
    int Noise[2][22];   // Noise
    int NT[2][22];      // Noise Target (LR or ms)

    int NTadjust[2][22];        // estimator Noise Target adjust

    float x34max[2][22];
    int ixmax[2][22];
    int ix10xmax[2][22];
    int gzero[2][22];
    int gmin[2][22];    // 3/3/99 min val for g, smaller may overflow quant
    int gsf[2][22];
//int   gsfq[2][22];       // gsf for inverse quant
    int sf[2][22];
    int G[2];
    int active_sf[2][22];
//float xigain34[2][22];       // fwd quant gain factors

    float x34[2][576];
    int ix10x[2][576];

    int preemp[2];
    int scalefactor_scale[2];

    int *psf_upper_limit[2], *psf_lower_limit[2];

//  vars for noise seek actual 
    float *y34, *y;
    int *ysignmask;
    int NTarget, noise, dn, logn;
    int NTarget0, NTarget1;
//--------

    BA_CONTROL ba_control;      // added for testing

    int ms_correlation_memory;

    int int_dummy;

//-functions------------------------------------------------
  private:

    void mnr_feedback ( int activeBands, int bits, int block_type );

    void output_sf ( SCALEFACT sf_out[] );
    void startup ( SIG_MASK sm[][36], unsigned char signx[][576] );
    void startup_ms ( SIG_MASK sm[][36], unsigned char signx[][576] );
    void startup_ms2 ( SIG_MASK sm[][36], unsigned char signx[][576] );
    void startup_adjustNT1 (  );
    void startup_adjustNT1B (  );
    void startup_adjustNT (  );

    int allocate (  );
    int allocate_ms (  );

    void noise_seek_initial2 (  );
    int decrease_noise ( int s0, int n );
    int increase_noise ( int s0, int n );
    void noise_seek_actual (  );

    void lucky_noise (  );
    void big_lucky_noise (  );
    void big_lucky_noise2 (  );
    int sfHeadRoom ( int ch );

    void inverse_sf2 (  );

    void quant ( int gsf[2][22] );
    void quantB ( int gsf[2][22] );
    void quantB10x ( int gsf[2][22] );  // used for thresholding

    void clear_hf (  );
    void clear_hf_main (  );

    void sparse_quad_counted ( int qx[], int n, int sparse_level );
    void quantBhf (  );
    void quantBhf_ms (  );

    int count_bits (  );
    int count_bits2 ( int opti_flag );
    int count_bits_dual (  );

    void fnc_sf_final ( int ch );
    void fnc_sf_final_MPEG1 ( int ch );
    void fnc_sf_final_MPEG2 ( int ch );
    int fnc_scale_factors (  );
    int fnc_scale_factors_ms (  );

    void trade_side (  );
    void trade_main (  );
    void trade_dual (  );

    void ms_sparse (  );
    void ms_sparse_quads (  );

    void hf_adjust (  );
    void hf_adjust_ms (  );

    int increase_bits ( int bits0 );
    int increase_bits_ms01 ( int bits0 );
    int increase_bits_ms ( int bits0 );
    int limit_bits ( int bits0 );
    int limit_part23_bits (  );
    int decrease_bits01 ( int bits0 );
    int decrease_bits ( int bits0 );
    int decrease_bits_ms ( int bits0 );

// short bit allocation
    CBitAlloShort BitAlloShort;

};

#endif //  _BITALLO3_H_
