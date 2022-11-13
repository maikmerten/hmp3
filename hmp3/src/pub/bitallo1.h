/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallo1.h,v 1.1 2005/07/13 17:22:24 rggammon Exp $ 
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

#ifndef _BITALLO1_H_
#define _BITALLO1_H_

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "l3e.h"
#include "bitallo.h"

class CBitAllo1:public CBitAllo
{

  public:
    CBitAllo1 (  );

    ~CBitAllo1 (  );

    void BitAllo ( float xr_arg[][576], SIG_MASK sm_arg[][36],
                   int ch_arg, int nchan_arg,
                   int min_bits_arg, int target_bits_arg, int max_bits_arg,
                   int bit_pool_arg,
                   SCALEFACT sf_out[], GR gr_data[],
                   int ix_arg[][576],
                   unsigned char signx_arg[][576], int ms_flag_arg );

    int BitAlloInit ( BA_CONTROL & ba_control );

    int ms_correlation2 ( float x[2][576], int block_type = 0 );

    void ba_out_stats (  )
    {
    }   // test routine

//-data------------------------------------------------
  private:

    int call_count;
    int nchan;
    int is_flag;
    int ms_flag;
    int ill_is_pos;
    int h_id;
    float running_a;

    int max_bits;       /* must not exceed */
    int min_bits;       /* maybe wasted if less */
    int max_cnt_bits;
    int target_bits;
    int target0_bits;
    int target0_min;
    int target0_max;
    int target_more;
    int bitadjust;      /* running delta counted - estimated */
    int counted_bits;
    int huff_bits[2];

/*-------------*/
    int scalefactor_scale[2];
    int preemp[2];

/*-------------*/
    int nsfb_s;

    int gsf_save[2][21];        /* [ch] */
    int bitadjust_save[2];      /* [ch] */

    typedef float ARRAY576[576];
    ARRAY576 *xr;
    typedef int intARRAY576[576];
    intARRAY576 *ix;

//typedef unsigned char ucharARRAY576[576];
//ucharARRAY576 *signx;

    float x34mm;        /* max all bands (and both chans) */
    float x34[2][576];

    float xsxx[2][21];

    float alpha_nmr;
    float ave_alpha_nmr;        /* running average */

    float mask[2][21];
    float x34max[2][21];
    float noise[2][21];

    int ixmax[2][21];
    int gzero[2][21];   /* quant to zero if gsf >= gzero */
    int gmin[2][22];    // 4/26/99 min val for g, smaller may overflow quant
    int gsf[2][21];     /* G-sf */
    int lastGsf[2][21]; // remember last G - sf, early out

    int sf[2][21];
    int G[2];

    float dGdB; /* delta G/delta bits, dG = dGdB*dB */
    float dBG;  /* 1/dGdB  */

    float look_log_cbw[21];     // log(sf band width) in db

//float look_34igain[128];   // inverse (1.0/gain)**(3/4)
//float look_gain[128];   // gain
//float look_ix43[256];     // inverse quant

    float look_f_ixmax[256];    // noise estimate f(ixmax) in db
    float look_f_ix[256];       // noise estimate f(ix) in db

/* big values of ix */
    float look_f_big_ixmax[256];        // noise estimate f(ixmax) in db
    float look_f_big_ix[256];   // noise estimate f(ix) in db

    int look_bits[256]; // bit estimate x16

    float gz_con0;
    float gz_con1;
    float gz_con2;

    int look_is_pos[34];
    float con707;

// functio_noise selector
    int i_function_noise_cb;

    int int_dummy;

// private functions 
    void gen_atan (  );
    void gen_bit_estimator (  );
    void gen_noise_estimator (  );

    void fnc_sf_final ( int ch );
    void fnc_sf_final_MPEG1 ( int ch );
    void fnc_sf_final_MPEG2 ( int ch );

    void output_sf ( SCALEFACT sf_out[] );
    void compute_x34 (  );

    void smr_adj ( SIG_MASK sm[][36], unsigned char signx[][576] );
    void smr_adj_joint ( SIG_MASK sm[][36], unsigned char signx[][576] );

    int allo_2 (  );
    void fnc_ixmax (  );

    int fnc_bit_est (  );
    int fnc_bit_seek (  );
    int fnc_bit_seek2 (  );
    void fnc_noise (  );
    void fnc_noise_cb ( int i, int ch );
    void fnc_noise2 (  );
    void fnc_noise2_cb ( int i, int ch );
    void fnc_noise2_init (  );
    void fnc_ix_quant (  );
    int fnc_noise_seek (  );

    void function_noise_cb ( int cb, int ch );

    int fnc_scale_factors (  );

};

#endif //  _BITALLO1_H_
