/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: srcc.h,v 1.1 2005/07/13 17:22:24 rggammon Exp $ 
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

#ifndef _SRCC_H_
#define _SRCC_H_

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "encapp.h"     // need IN_OUT

typedef struct
{
    int ncase;  // 
    int minbuf; // number samples required in caller buffer for 1152 output
// stage 1 (if any )
    int nbuf;   // number samples in stage1 output buffer
    int kbuf;   // index into stage1 output = stage2 input
    int k1;     // sample delta
    int m1;     // filter fraction sample delta
    int n1;     // n filters
    int ntaps1;
    int totcoef1;       // total number filter coefs
    int am1;    // accum filter delta
    int ic1;    // coef index
    float coef1[21];
//
    int k;      // sample delta
    int m;      // filter fraction sample delta
    int n;      // n filters
    int ntaps;
    int totcoef;        // total number filter coefs
    int am;     // accum filter delta
    int ic;     // coef index
    float coef[1280];   // 1280 for 11025:32000
// two stage intermediate buffers
    float buf[128 + 64];        // case 4 stage 1 output = stage2 input
    float buf2[128 + 64];       // case 4 stage 1 output = stage2 input
}
SRC_STRUCT;

//typedef struct {
//int in_bytes;
//int out_bytes;
//} IN_OUT;

//=================================================================
class Csrc
{

  public:
    Csrc (  );

    ~Csrc (  )
    {
    }

    int sr_convert_init ( int source, int channels, int bits,
                          int target, int target_channels,
                          int *encode_cutoff_freq );
    IN_OUT sr_convert ( unsigned char xin[], short yout[] );

//-data------------------------------------------------
  private:
    SRC_STRUCT src;

    int src_bytes_out;
    int src_filter;

//-functions------------------------------------------------
    int gen_src_filter ( int source0, int target );
    int gen_f1 ( float a[], int ntaps, int ncutoff, int nfilters, int m );

    int src_filter_mono_case0 ( short x[], short y[] );
    int src_filter_mono_case1 ( short x[], short y[] );
    int src_filter_mono_case2 ( short x[], short y[] );
    int src_filter_mono_case3 ( short x[], short y[] );
    int src_filter_mono_case4 ( short x[], short y[] );

    int src_filter_dual_case0 ( short x[], short y[] );
    int src_filter_dual_case1 ( short x[][2], short y[][2] );
    int src_filter_dual_case2 ( short x[][2], short y[][2] );
    int src_filter_dual_case3 ( short x[][2], short y[][2] );
    int src_filter_dual_case4 ( short x[][2], short y[][2] );

    int src_filter_to_mono_case0 ( short x[][2], short y[] );
    int src_filter_to_mono_case1 ( short x[][2], short y[] );
    int src_filter_to_mono_case2 ( short x[][2], short y[] );
    int src_filter_to_mono_case3 ( short x[][2], short y[] );
    int src_filter_to_mono_case4 ( short x[][2], short y[] );

// 8 bit input
    int src_bfilter_mono_case0 ( unsigned char x[], short y[] );
    int src_bfilter_mono_case1 ( unsigned char x[], short y[] );
    int src_bfilter_mono_case2 ( unsigned char x[], short y[] );
    int src_bfilter_mono_case3 ( unsigned char x[], short y[] );
    int src_bfilter_mono_case4 ( unsigned char x[], short y[] );

    int src_bfilter_dual_case0 ( unsigned char x[][2], short y[][2] );
    int src_bfilter_dual_case1 ( unsigned char x[][2], short y[][2] );
    int src_bfilter_dual_case2 ( unsigned char x[][2], short y[][2] );
    int src_bfilter_dual_case3 ( unsigned char x[][2], short y[][2] );
    int src_bfilter_dual_case4 ( unsigned char x[][2], short y[][2] );

    int src_bfilter_to_mono_case0 ( unsigned char x[][2], short y[] );
    int src_bfilter_to_mono_case1 ( unsigned char x[][2], short y[] );
    int src_bfilter_to_mono_case2 ( unsigned char x[][2], short y[] );
    int src_bfilter_to_mono_case3 ( unsigned char x[][2], short y[] );
    int src_bfilter_to_mono_case4 ( unsigned char x[][2], short y[] );

    int stage1_mono ( short x[] );
    int stage1_dual ( short x[][2] );
    int stage1_to_mono ( short x[][2] );

    int stage1b_mono ( unsigned char x[] );
    int stage1b_dual ( unsigned char x[][2] );
    int stage1b_to_mono ( unsigned char x[][2] );

};

#endif //  _SRCC_H_
