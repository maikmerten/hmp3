/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: l3init.c,v 1.2 2005/08/09 20:43:42 karll Exp $ 
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

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

/* get rid of precision loss warnings on conversion */
#ifdef _MSC_VER
#pragma warning(disable:4244 4056)
#endif

static const int sr_table[2][3] =     // [h_id][sr_index]
{
    { 22050, 24000, 16000 },
    { 44100, 48000, 32000 }
};

/*-- Layer 3 sf band tables --*/
static const struct
{
    int l[23];
    int s[14];
}
sfBandTable[2][3] =
{       // [h_id][sr_index]

/* mpeg-2 */
    {
        {
            {   0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168,
                200, 238, 284, 336, 396, 464, 522, 576},
            {   0, 4, 8, 12, 18, 24, 32, 42, 56, 74, 100, 132, 174, 192}
        },
        {
            {   0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 114, 136, 162,
                194, 232, 278, 332, 394, 464, 540, 576},
            {   0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 136, 180, 192}
        },
        {
            {   0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168,
	        200, 238, 284, 336, 396, 464, 522, 576},
            {   0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
        },
    },

/* mpeg-1 */
    {
        {
            {   0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 52, 62, 74, 90, 110, 134,
                162, 196, 238, 288, 342, 418, 576},
            {   0, 4, 8, 12, 16, 22, 30, 40, 52, 66, 84, 106, 136, 192}
        },
        {
            {   0, 4, 8, 12, 16, 20, 24, 30, 36, 42, 50, 60, 72, 88, 106, 128,
                156, 190, 230, 276, 330, 384, 576},
            {   0, 4, 8, 12, 16, 22, 28, 38, 50, 64, 80, 100, 126, 192}
        },
        {
            {   0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 54, 66, 82, 102, 126,
                156, 194, 240, 296, 364, 448, 550, 576},
            {   0, 4, 8, 12, 16, 22, 30, 42, 58, 78, 104, 138, 180, 192}
        }
    }
};

/*- long block sf bands mpeg2 - 22 same as 16 */

static int sr_index;
static int h_id;
static int band_limit;

///*---------- quant ---------------------------------*/
///* 8 bit lookup x = pow(2.0, 0.25*(global_gain-210)) */
//float *quant_init_global_addr();
//
//
///* x = pow(2.0, -0.5*(1+scalefact_scale)*scalefac + preemp) */
//typedef float LS[4][16];
//LS *quant_init_scale_addr();
//
//
//float *quant_init_pow_addr();
//float *quant_init_subblock_addr();
//
//typedef int iARRAY21[21];
//iARRAY21 *quant_init_band_addr();
//

/*---------- antialias ---------------------------------*/
typedef float ARRAY8[8];

ARRAY8 *alias_init_addr (  );

static const float Ci[8] = {
    -0.6f, -0.535f, -0.33f, -0.185f, -0.095f, -0.041f, -0.0142f, -0.0037f
};

void hwin_init (  );    // hybrid windows
void mdct_init (  );
typedef struct
{
    float *w;
    float *w2;
    void *coef;
}
IMDCT_INIT_BLOCK;

/*=============================================================*/

/*=============================================================*/
int
L3freq_nearest_sf_band ( int sr_index_arg, int h_id_arg, int freq )
{
    int i, samprate, f, fout;
    int delta, deltamin;
    float a;

    samprate = sr_table[h_id_arg][sr_index_arg];
    a = samprate / ( 2.0f * 576.0f );

    deltamin = 999999;
    fout = freq;
    for ( i = 0; i < 21; i++ )
    {
        f = ( int ) ( a * sfBandTable[h_id_arg][sr_index_arg].l[i + 1] +
                      0.5f );
        delta = abs ( f - freq );
        if ( delta < deltamin )
        {
            deltamin = delta;
            fout = f;
        }
    }

    return fout;
}

/*=============================================================*/
void
L3table_init ( int sr_index_arg, int h_id_arg, int band_limit_arg )
{
    int i;
    ARRAY8 *csa;

/* save for band table generator */
    sr_index = sr_index_arg;
    h_id = h_id_arg;
    band_limit = band_limit_arg;

/*================ antialias ===============================*/
    csa = alias_init_addr (  );
    for ( i = 0; i < 8; i++ )
    {
        csa[0][i] = ( float ) ( 1.0 / sqrt ( 1.0 + Ci[i] * Ci[i] ) );
        csa[1][i] = ( float ) ( Ci[i] / sqrt ( 1.0 + Ci[i] * Ci[i] ) );
    }

/*================ mdct ===============================*/
    mdct_init (  );

/*--- hybrid windows ------------*/
    hwin_init (  );

}

/*====================================================================*/
typedef float ARRAY36[36];
ARRAY36 *hwin_init_addr (  );

/*--------------------------------------------------------------------*/
void
hwin_init (  )
{
    int i, j;
    double pi;
    ARRAY36 *win;

    win = hwin_init_addr (  );

    pi = 4.0 * atan ( 1.0 );

/* type 0 */
    for ( i = 0; i < 36; i++ )
        win[0][i] = ( float ) sin ( pi / 36 * ( i + 0.5 ) );

/* type 1*/
    for ( i = 0; i < 18; i++ )
        win[1][i] = ( float ) sin ( pi / 36 * ( i + 0.5 ) );
    for ( i = 18; i < 24; i++ )
        win[1][i] = 1.0F;
    for ( i = 24; i < 30; i++ )
        win[1][i] = ( float ) sin ( pi / 12 * ( i + 0.5 - 18 ) );
    for ( i = 30; i < 36; i++ )
        win[1][i] = 0.0F;

/* type 3*/
    for ( i = 0; i < 6; i++ )
        win[3][i] = 0.0F;
    for ( i = 6; i < 12; i++ )
        win[3][i] = ( float ) sin ( pi / 12 * ( i + 0.5 - 6 ) );
    for ( i = 12; i < 18; i++ )
        win[3][i] = 1.0F;
    for ( i = 18; i < 36; i++ )
        win[3][i] = ( float ) sin ( pi / 36 * ( i + 0.5 ) );

/* type 2*/
    for ( i = 0; i < 12; i++ )
        win[2][i] = ( float ) sin ( pi / 12 * ( i + 0.5 ) );
    for ( i = 12; i < 36; i++ )
        win[2][i] = 0.0F;

/*--- invert signs by region to match mdct 18pt --> 36pt mapping */
    for ( j = 0; j < 4; j++ )
    {
        if ( j == 2 )
            continue;
        for ( i = 9; i < 36; i++ )
            win[j][i] = -win[j][i];
    }

/*-- invert signs for short blocks --*/
    for ( i = 3; i < 12; i++ )
        win[2][i] = -win[2][i];

/*--- scale window so mdct(imdct(x)) = x (aprox 32k max) --*/
    for ( j = 0; j < 4; j++ )
    {
        if ( j == 2 )
            continue;
        for ( i = 0; i < 36; i++ )
            win[j][i] = ( 1.0f / 9.0f ) * win[j][i];
    }
    for ( i = 0; i < 36; i++ )
        win[2][i] = ( 1.0f / 3.0f ) * win[2][i];

/*--------
for(j=0;j<4;j++) {
    if( j == 2 ) continue;
    for(i=0;i<36;i++ ) win[j][i] = (1.0f/8.0f)*win[j][i];
}
for(i=0;i<36;i++ ) win[2][i] = (1.0f/8.0f)*win[2][i];
-----------*/

    return;
}

/*=============================================================*/
typedef float ARRAY4[4];
IMDCT_INIT_BLOCK *mdct_init_addr_18 (  );
IMDCT_INIT_BLOCK *mdct_init_addr_6 (  );

/*-------------------------------------------------------------*/
void
mdct_init (  )
{
    int k, p, n;
    double t, pi;
    IMDCT_INIT_BLOCK *addr;
    float *w, *w2;
    float *v, *v2, *coef87;
    ARRAY4 *coef;

/*--- 18 point --*/
    addr = mdct_init_addr_18 (  );
    w = addr->w;
    w2 = addr->w2;
    coef = addr->coef;

/*----*/
    n = 18;
    pi = 4.0 * atan ( 1.0 );
    t = pi / ( 4 * n );
    for ( p = 0; p < n; p++ )
        w[p] = ( float ) ( 2.0 * cos ( t * ( 2 * p + 1 ) ) );
    for ( p = 0; p < 9; p++ )
        w2[p] = ( float ) 2.0 *cos ( 2 * t * ( 2 * p + 1 ) );

    t = pi / ( 2 * n );
    for ( k = 0; k < 9; k++ )
    {
        for ( p = 0; p < 4; p++ )
            coef[k][p] = ( float ) cos ( t * ( 2 * k ) * ( 2 * p + 1 ) );
    }

/*--- 6 point */
    addr = mdct_init_addr_6 (  );
    v = addr->w;
    v2 = addr->w2;
    coef87 = addr->coef;

/*----*/
    n = 6;
    pi = 4.0 * atan ( 1.0 );
    t = pi / ( 4 * n );
    for ( p = 0; p < n; p++ )
        v[p] = ( float ) 2.0 *cos ( t * ( 2 * p + 1 ) );

    for ( p = 0; p < 3; p++ )
        v2[p] = ( float ) 2.0 *cos ( 2 * t * ( 2 * p + 1 ) );

    t = pi / ( 2 * n );
    k = 1;
    p = 0;
    *coef87 = ( float ) cos ( t * ( 2 * k ) * ( 2 * p + 1 ) );

/* adjust scaling to save a few mults */
    for ( p = 0; p < 6; p++ )
        v[p] = v[p] / 2.0f;
    *coef87 = ( float ) 2.0 *( *coef87 );

    return;
}

/*===============================================================*/
void
L3init_gen_band_table_long ( int nBand_long[] )
{
    int i;

/* call table init first for valid sr_index and h_id */

// note: table gen extended to 22

    for ( i = 0; i < 22; i++ )
        nBand_long[i] =
            sfBandTable[h_id][sr_index].l[i + 1]
            - sfBandTable[h_id][sr_index].l[i];

}

/*-------------------------------------------------------------*/
void
L3init_gen_band_table_short ( int nBand[] )
{
    int i;

/* call table init first for valid sr_index and h_id */
// extended to 13
    for ( i = 0; i < 13; i++ )
        nBand[i] =
            sfBandTable[h_id][sr_index].s[i + 1]
            - sfBandTable[h_id][sr_index].s[i];

}

/*-------------------------------------------------------------*/
int
L3init_sfbl_limit (  )
{
    int i;

/* call table init first for valid sr_index and h_id */

/* return max sfb long */
    for ( i = 0; i < 23; i++ )
    {
        if ( band_limit <= sfBandTable[h_id][sr_index].l[i] )
            break;
    }
    if ( i > 21 )
        i = 21;
    return i;
}

/*-------------------------------------------------------------*/
int
L3init_sfbl_limit2 ( int band_limit )
{
    int i;

/* call table init first for valid sr_index and h_id */

/* return max sfb long */
    for ( i = 0; i < 23; i++ )
    {
        if ( band_limit <= sfBandTable[h_id][sr_index].l[i] )
            break;
    }
    if ( i > 21 )
        i = 21;
    return i;
}

/*-------------------------------------------------------------*/
int
L3init_sfbs_limit (  )
{
    int i;

/* call table init first for valid sr_index and h_id */

/* return max sfb short */
    for ( i = 0; i < 14; i++ )
    {
        if ( band_limit <= sfBandTable[h_id][sr_index].s[i] )
            break;
    }
    if ( i > 12 )
        i = 12;
    return i;
}

/*-------------------------------------------------------------*/
int
L3init_sfbs_limit2 ( int band_limit )
{
    int i;

/* call table init first for valid sr_index and h_id */

/* return max sfb short */
    for ( i = 0; i < 14; i++ )
    {
        if ( band_limit <= sfBandTable[h_id][sr_index].s[i] )
            break;
    }
    if ( i > 12 )
        i = 12;
    return i;
}

/*-------------------------------------------------------------*/
