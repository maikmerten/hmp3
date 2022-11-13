/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallo.h,v 1.1 2005/07/13 17:22:24 rggammon Exp $ 
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

#ifndef _BITALLO_H_
#define _BITALLO_H_

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>

#include "l3e.h"

#include "balow.h"      // low level routines
#include "hxtypes.h"

typedef struct
{
    int band_limit_left;
    int band_limit_right;
    int is_flag;
    int h_id;
    int hf_flag;
    int initialMNR;
    int vbr_flag;
    int MNRbias;
    int disable_taper;
    int *mnr_adjust;
    int test1;
    int test2;
    int test3;
}
BA_CONTROL;

class CBitAllo
{

  public:
    CBitAllo (  );

    ~CBitAllo (  );

    virtual void BitAllo ( float xr_arg[][576], SIG_MASK sm_arg[][36],
                           int ch_arg, int nchan_arg,
                           int min_bits_arg, int target_bits_arg,
                           int max_bits_arg, int bit_pool_arg,
                           SCALEFACT sf_out[], GR gr_data[],
                           int ix_arg[][576], unsigned char signx_arg[][576],
                           int ms_flag_arg ) = 0;

    virtual int BitAlloInit ( BA_CONTROL & ba_control ) = 0;

    virtual int ms_correlation2 ( float x[2][576], int block_type = 0 ) = 0;

    int ms_correlation ( float x[2][576], int n );

    virtual void ba_out_stats (  ) = 0; // test routine

//-data------------------------------------------------
  protected:

    int nsf[2];
    int nBand_l[22];
    int startBand_l[24];
    int nBand_s[13];    // extended to 192 lines
    int startBand_s[14];

    int block_type;

//-functions------------------------------------------------

// bit counter L3bacC.cpp
    INTPAIR subdivide2_quadregion ( int ixmax[], int ix[], int ncb, int ch );
// returns (nbig,nquads)
    void output_subdivide2 ( GR * gr_data, int ch );
// long block type 0
    int subdivide2 ( int ixmax[], int ix[], int ncb, int opti_flag, int ch );
// long block types 1/3
    int subdivide2BT13 ( int ixmax[], int ix[], int ncb, int opti_flag,
                         int ch );
    int region_aux ( int ixmax[], int ix[] );
    int divide_region3 ( int c, int ixmax[], int ix[] );

};

#endif //  _BITALLO_H_
