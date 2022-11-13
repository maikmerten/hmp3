/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallos.h,v 1.1 2005/07/13 17:34:48 rggammon Exp $ 
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

#ifndef _BITALLOS_H_
#define _BITALLOS_H_

// short BitAllo

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "l3e.h"

#include "balow.h"      // low level routines
#include "hxtypes.h"
#include "bitallo.h"    // ba control structure

#define g_offset 8
// caution g_offset also defined in L3math!!!

// delta from gzero to gmin,
// theoretical value is 73+, 70 allows for rounding in gsf -> sf
#define GMIN_OFFSET 70

// per ch igr bits limited by part2_3length
#define PART23  4021

//=========================================================
class CBitAlloShort
{

  public:
    CBitAlloShort (  );

    ~CBitAlloShort (  );

    // returns feedback bits (huff bits before increasse/decrease)
    // for external MNR eedback adjustment
    int BitAllo ( float xr_arg[][3][192], SIG_MASK sm_arg[][3][12],
                  int ch_arg, int nchan_arg,
                  int min_bits_arg, int target_bits_arg, int max_bits_arg,
                  int bit_pool_arg,
                  SCALEFACT sf_out[], GR gr_data[],
                  int ix_arg[][576],
                  unsigned char signx_arg[][576], int ms_flag_arg, int MNR );

    int BitAlloInit ( BA_CONTROL & bac );

    int ms_correlation2Short ( float x[2][3][192] );

    void ba_out_stats (  );     // test routine

//-data------------------------------------------------
  private:

    int vbr_flag;

    float gz_con1B;
    float gz_con2B;

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
    int max_cnt_bits;
    int TargetBits;
    int FeedbackBits;   // for external mnr adjust
    int MNR;    // long term MNR target  in mb
    int deltaMNR;       // 
    int initialMNR;
    int activeBands;
    int huff_bits[2];

    int nsf[2];

    int nBand_s[13];    // 12 + band to Nyquist
    int startBand_s[14];

    int nbmax[2];
    int nbmax2[2];
    int nbmax3[2];

    int taperNT[12];

    float dGdB; /* delta G/delta bits, dG = dGdB*dB */

    typedef float ARRAY3_192[3][192];
    ARRAY3_192 *xr;

//typedef int intARRAY3_192[3][192];
//intARRAY3_192 *ix;
//typedef unsigned char ucharARRAY3_192[3][192];
//ucharARRAY3_192 *signx;

    int ix[2][3][192];
    unsigned char signx[2][3][192];

    int look_log_cbwmb[16];     // log(sf band width) in mb

    float xsxx[2][3][16];
    int Noise0[2][3][16];       // Noise at no-allo (signal)
    int NT[2][3][16];   // Noise Target (LR or ms)
    int Noise[2][3][16];        // Noise
    int snr[2][3][16];  // 

    float x34max[2][3][16];
    int ixmax[2][3][16];
    int ix10xmax[2][3][16];
    int gzero[2][3][16];
    int gmin[2][3][16]; // 3/3/99 min val for g, smaller may overflow quant
    int gsf[2][3][16];
    int sf[2][3][16];
    int active_sf[2][3][16];

    int subblock_gain[2][3];
    int G[2][3];        // 
    int GG[2];

    float x34[2][3][192];
    int ix10x[2][3][192];

// no preemp on short blocks
    int scalefactor_scale[2];

    int *psf_upper_limit[2], *psf_lower_limit[2];

//  vars for noise seek actual 
    float *y34, *y;
    int *ysignmask;
    int NTarget, noise, dn, logn;
    int NTarget0, NTarget1;
//--------

    int int_dummy;

//-functions------------------------------------------------
  private:

    void output_sf ( SCALEFACT sf_out[] );
    void startup ( SIG_MASK sm[][3][12], unsigned char signx[][3][192] );
    void startup_ms ( SIG_MASK sm[][3][12], unsigned char signx[][3][192] );
    void startup_ms2 ( SIG_MASK sm[][3][12], unsigned char signx[][3][192] );
    void startup_adjustNT (  );
    void startup_adjustNT1 (  );

    void allocate (  );

    void noise_seek_initial2 (  );
    int decrease_noise ( int s0, int n );
    int increase_noise ( int s0, int n );
    void noise_seek_actual (  );

    void quant (  );
    void quantB (  );

    int count_bits (  );
    int count_bits_dual (  );

    void fnc_sf_final ( int ch );
    void fnc_scale_factors01 (  );
    void fnc_scale_factors (  );

    int increase_bits ( int bits0 );
    int increase_bits_ms ( int bits0 );
    int limit_bits ( int bits0 );
    int limit_part23_bits (  );
    int decrease_bits01 ( int bits0 );
    int decrease_bits ( int bits0 );
    int decrease_bits_ms ( int bits0 );

// bit counter functions
    int subdivide2 ( int ixmax[][16], int ix[][192], int ncb, int ch );
    void output_subdivide2 ( GR * gr_data, int ch );
    int ix_reorder[576];        // 
    struct
    {
        int ntable_out[4];
        int cbreg[3];
        int nbig;
        int nquads;
        int bits;
    }
    save[2];

};

#endif //  _BITALLOS_H_
