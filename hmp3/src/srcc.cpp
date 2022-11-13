/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: srcc.cpp,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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

/*
 * sample rate conversion filter generation/init
 *
 * ncase 0  No conversion
 * ncase 1  Up sample by 1:2.  Can be done quickly
 *         with integer ALU if output type is integer.
 *         Also faster than general m:n with FPU.
 * ncase 2  Up sample by m:n.
 * ncase 3  Down sample by m:n for small n. Interpolation
 *         done as part of bandpass filtering. Requires
 *         n sets of filter coefs.
 * ncase 4  Down sample by general m:n.  Two stage conversion
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>

#include "srcc.h"
#include "hxtypes.h"

/*==================================================================*/

/*==================================================================*/
Csrc::Csrc (  ):
src_bytes_out ( 0 ), src_filter ( 0 )
{
}

/*==================================================================*/

static int common_factor ( int s, int t );
static int cascade_factors ( int source, int target );
static int factor ( int f[], int s );
static int gen_f ( float a[], int ntaps, int ncutoff, int nfilters, int m );

/*==================================================================*/
static int
compute_ntaps ( int source, int target )
{
    int ntaps;

    ntaps = ( 12 * source + target / 2 ) / target;
    if ( ntaps > 48 )
        ntaps = 48;
    if ( ntaps < 1 )
        ntaps = 1;
//ntaps = (ntaps + 1) & (~1);
    ntaps = ( ntaps & ( ~1 ) ) | 1;     // odd for 44.1:16 looks better on sweep
    if ( source <= target )
    {
        ntaps = 1;
    }

    return ntaps;
}

/*==================================================================*/
int
Csrc::gen_src_filter ( int source0, int target )
{
    int n;
    int ncutoff;
    int mem;
    int ntaps;
    int source;
    int source1, target1;
    int ncutoff1;

    source = source0;
    source1 = source;
    target1 = source;
    ntaps = compute_ntaps ( source, target );
    n = target / common_factor ( source, target );
    mem = n * ntaps;

    if ( source == target )
        src.ncase = 0;
    else if ( 2 * source == target )
        src.ncase = 1;
    else if ( source < target )
        src.ncase = 2;
    else if ( mem <= 780 )
        src.ncase = 3;
    else
        src.ncase = 4;

    if ( src.ncase == 4 )
    {
        source = cascade_factors ( source, target );
        if ( source <= 0 )
            return 0;   // fail
        target1 = source;
    }

    src.ntaps1 = compute_ntaps ( source1, target1 );
    src.n1 = target1 / common_factor ( source1, target1 );
    src.k1 = source1 / target1;
    src.m1 = ( src.n1 * source1 - target1 * src.n1 * src.k1 ) / target1;
    src.totcoef1 = src.ntaps1 * src.n1;
    ncutoff1 = ( int ) ( 0.90 * src.ntaps1 * target1 / source1 + 0.50 );
    if ( ncutoff1 > src.ntaps1 )
        ncutoff1 = src.ntaps1;

    src.ntaps = compute_ntaps ( source, target );
    src.n = target / common_factor ( source, target );
    src.k = source / target;
    src.m = ( src.n * source - target * src.n * src.k ) / target;
    src.totcoef = src.ntaps * src.n;
//ncutoff = 0.85*src.ntaps*target/source  + 0.50;
    ncutoff = ( int ) ( 0.90 * src.ntaps * target / source + 0.50 );
//ncutoff = (ncutoff & (~1)) | (src.ntaps & 1);
    if ( ncutoff > src.ntaps )
        ncutoff = src.ntaps;

/*--------------------------------------
//if( (src.n < 50) && (source > target) ) 
//if( (source > target) )
//if( src.ncase == 4 )
{ 
printf("\n%5d to %5d", source0, target);
printf(" ncase=%1d", src.ncase);
//if( src.ncase == 4 ) 
{
    printf("\n  %5d to %5d", source0, source);
    printf("  k+m/n  %1d %3d %4d", src.k1, src.m1, src.n1); 
    printf(" taps = %2d", src.ntaps1);
    printf(" ncoefs %5d", src.totcoef1);
    printf(" c/o = %2d", ncutoff1);
    printf("\n  %5d to %5d", source, target);
}
printf("  k+m/n  %1d %3d %4d", src.k, src.m, src.n); 
printf(" taps = %2d", src.ntaps);
printf(" ncoefs %5d", src.totcoef);
printf(" c/o = %2d", ncutoff);
  //printf(" minb %3d", src.minbuf);
}
------------------------------------*/

// init accum filter delta (down counter wrap if <= 0
    src.am = src.n;
    src.ic = 0; // init 
    src.minbuf =
        ( int ) ( 1152.0 * source0 / target + ( src.ntaps - 1 ) + 1 );
    if ( src.ncase == 4 )
        src.minbuf += ( 128 + 4 );
    src.am1 = src.n1;
    src.ic1 = 0;        // init 
    src.nbuf = 0;

// return failed if not enough memory
    if ( src.totcoef1 > ( sizeof ( src.coef1 ) / sizeof ( float ) ) )
        return 0;
    if ( src.totcoef > ( sizeof ( src.coef ) / sizeof ( float ) ) )
        return 0;

    gen_f ( src.coef1, src.ntaps1, ncutoff1, src.n1, src.m1 );
    gen_f ( src.coef, src.ntaps, ncutoff, src.n, src.m );
//for(i=0;i<n;i++) outf4(src.coef+i*src.ntaps, src.ntaps);

    return src.minbuf;
}

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static void norm ( float x[], int n );
static void fc_window ( float x[], int n );
static void fc_window2 ( float x[], int n, float x0 );
static int gen1 ( float b[], int N, int n, float alpha, float *x );
static int gen2 ( float b[], int N, int n, float alpha, float *x );
static int gen3 ( float b[], int N, int n, float alpha );

/*==================================================================*/
static int
cascade_factors ( int source, int target )
{
    int i;
    int cf;
    int s, t;
    int fs1, ft1;
    int fs2, ft2;
    int ntaps, ncutoff;
    int source2;
    int mem;

    if ( source <= target )
        return source;

    cf = common_factor ( source, target );
    s = source / cf;
    t = target / cf;

    ft1 = 0;    // init to eliminate compiler warning
    fs1 = 0;
    for ( i = 7; i < t; i++ )
    {
        if ( s != i * ( s / i ) )
            continue;
        if ( t != ( i + 1 ) * ( t / ( i + 1 ) ) )
            continue;
        fs1 = i;
        ft1 = i + 1;
        ft2 = t / ft1;
        source2 = ft1 * source / fs1;
        ntaps = compute_ntaps ( source2, target );
        mem = ntaps * ft2;
        if ( mem <= 780 )
            break;
    }

    if ( fs1 == 0 )
    {
        //printf("\n cascade fail");
        return 0;       // failed
    }

//printf("\n fs1 ft1 %3d %3d", fs1, ft1);

    fs2 = s / fs1;
    ft2 = t / ft1;
//printf("    fs2 ft2 %3d %3d", fs2, ft2);

    source2 = ft1 * source / fs1;
    ntaps = ( 12 * source2 + target / 2 ) / target;

    ncutoff = ( int ) ( 0.85 * ntaps * target / source2 + 0.50 );

//printf("\n stage 2  source %5d", source2);
//printf("   taps = %3d", ntaps);
//printf("   c/o  = %3d", ncutoff);
    mem = ntaps * ft2;
//printf("   mem  = %3d", mem);

    return source2;
}

/*==================================================================*/
static int
common_factor ( int s, int t )
{
    int i;
    int cf;

    cf = 1;
    for ( i = 2; i <= t; i++ )
    {
        if ( s != i * ( s / i ) )
            continue;
        if ( t != i * ( t / i ) )
            continue;
        cf = i * cf;
        s = s / i;
        t = t / i;
        i = 1;
    }

    return cf;
}

/*==================================================================*/
static int
factor ( int f[], int s )
{
    int i, m, n;

    n = 0;
    m = ( s + 1 ) / 2;
    for ( i = 2; i <= m; i++ )
    {
        if ( s != i * ( s / i ) )
            continue;
        f[n++] = i;
        s = s / i;
        m = ( s + 1 ) / 2;
        i = 1;
    }
    f[n++] = s;

    return n;
}

/*--------------------------------------------------------------------*/
static int
gen_f ( float a[], int ntaps, int ncutoff, int nfilters, int m )
{
    int i;
    float x, alpha;
    int am;

// linear interp in order used by filter
    if ( ntaps == 1 )
    {
        am = 0;
        for ( i = 0; i < nfilters; i++ )
        {
            alpha = ( ( float ) am ) / nfilters;
            a[0] = alpha;       // compute as y = f0+alpha*f1-f0);
            am += m;
            if ( am >= nfilters )
                am = am - nfilters;
            a += ntaps;
        }
        return 0;
    }

    if ( ntaps == 2 )
    {   // linear interpolation
        am = 0;
        for ( i = 0; i < nfilters; i++ )
        {
            alpha = ( ( float ) am ) / nfilters;
            a[0] = 1.0f - alpha;
            a[1] = alpha;
            am += m;
            if ( am >= nfilters )
                am = am - nfilters;
            a += ntaps;
        }
        return 0;
    }

    am = 0;
    for ( i = 0; i < nfilters; i++ )
    {
        alpha = ( ( float ) am ) / nfilters;
        alpha = alpha + 0.5f / nfilters - 0.5f; // filter center = 0.0
        //gen1(a, ntaps, ncutoff, alpha, &x);  // lbr using gen2
        gen2 ( a, ntaps, ncutoff, alpha, &x );  // lbr using gen2
        //gen3(a, ntaps, ncutoff, alpha);    // never tested

        fc_window ( a, ntaps );
        norm ( a, ntaps );
        //fc_window2(a, ntaps, x);

        am += m;
        if ( am >= nfilters )
            am = am - nfilters;

//printf("\n alpha = %8.4f --------", alpha);
//outf4b(a, ntaps);

        a += ntaps;
    }

    return 0;
}

/*--------------------------------------------------------------------*/
int
Csrc::gen_f1 ( float a[], int ntaps, int ncutoff, int nfilters, int m )
{
    int i, j;
    float x, alpha;
    float a0[64];
    int nt;
    int am, dk;

// build filter from linear interp of pair of filter;

    nt = ntaps; // ntaps should be even?
    if ( nfilters > 1 )
        nt--;   // ntaps should be even?
    if ( ncutoff > nt )
        ncutoff = nt;

    if ( ntaps <= 2 )
    {   // linear interpolation
        a0[0] = 0.0f;
        a0[1] = 1.0f;
        a0[2] = 0.0f;
    }
    else
    {
        a0[ntaps] = a0[0] = 0.0f;
        //gen1(a0+1, nt, ncutoff, 0.0f, &x);
        gen2 ( a0 + 1, nt, ncutoff, 0.0f, &x ); // lbr using gen2
        //gen3(a0+1, nt, ncutoff, 0.0f);  // lbr using gen2
        fc_window ( a0 + 1, nt );
        norm ( a0 + 1, nt );    // ??
    }

/*---------------
for(i=0;i<nfilters;i++) {
  //alpha = ((float)i + 0.5)/nfilters;
  alpha = ((float)i)/nfilters;
  for(j=0;j<ntaps;j++) a[j] = a0[j+1] + alpha*(a0[j]-a0[j+1]);
  a+=ntaps;
}
---------------*/

// in order used by filter
    am = 0;
    for ( i = 0; i < nfilters; i++ )
    {
        alpha = ( ( float ) am ) / nfilters;
        for ( j = 0; j < ntaps; j++ )
            a[j] = a0[j + 1] + alpha * ( a0[j] - a0[j + 1] );
        if ( ntaps == 1 )
            a[0] = alpha;       // compute as y = f0+alpha*f1-f0);
        //if( ntaps == 1 ) a[0] = alpha + 0.5/nfilters;  // compute as y = f0+alpha*f1-f0);
        a += ntaps;
        dk = src.k;     // sample delta for next filtered output
        am += m;
        if ( am >= nfilters )
        {
            am = am - nfilters;
            dk++;
        }
    }

    return ntaps;
}

/*-------------------------------------------------------------*/
static int
gen1 ( float b[], int N, int n, float alpha, float *xout )
{
    int k, p;
    double pi, t;
    double x, scale;
    double wk;

//x = N/2.0 - 1 + alpha;
    x = ( N - 1 ) / 2.0 + alpha;
    *xout = ( float ) x;

    pi = 4.0 * atan ( 1.0 );
    t = pi / ( 2 * N );
    scale = 1.0 / N;

    for ( p = 0; p < N; p++ )
        b[p] = 0.0f;

    for ( p = 0; p < N; p++ )
    {
        for ( k = 0; k < n; k++ )
        {
            wk = 2.0;
            if ( k == 0 )
                wk = 1.0;
            b[p] +=
                ( float ) ( scale * wk * cos ( t * k * ( 2 * x + 1 ) ) *
                            cos ( t * k * ( 2 * p + 1 ) ) );
        }
    }

    return N;
}

/*-------------------------------------------------------------*/
static int
gen2 ( float b[], int N, int n, float alpha, float *xout )
{
    int k, p;
    double pi, t;
    double x, scale;
    double wp;

//x = N/2 - 1 + alpha;
//x = N/2.0 - 1 + alpha;  //  ???
    x = ( N - 1 ) / 2.0 + alpha;        //  ???
    *xout = ( float ) x;

    pi = 4.0 * atan ( 1.0 );
    t = pi / ( 2 * N );
    scale = 1.0 / N;

    for ( p = 0; p < N; p++ )
        b[p] = 0.0f;

    for ( p = 0; p < N; p++ )
    {
        wp = 2.0;
        if ( p == 0 )
            wp = 1.0;
        for ( k = 0; k < n; k++ )
        {
            b[p] +=
                ( float ) ( scale * wp * cos ( t * x * ( 2 * k + 1 ) ) *
                            cos ( t * p * ( 2 * k + 1 ) ) );
        }
    }

    return N;
}

/*-------------------------------------------------------------*/

/*-------------------------------------------------------------*/
static int
gen3 ( float b[], int N, int n, float alpha )
{
    int k, p;
    double pi, t;
    double x, scale;

// this never tested

    x = ( N - 1 ) / 2 + alpha;

    pi = 4.0 * atan ( 1.0 );
    t = pi / ( 4 * N );
    scale = 1.0 / N;

    for ( p = 0; p < N; p++ )
        b[p] = 0.0f;

    for ( p = 0; p < N; p++ )
    {
        for ( k = 0; k < n; k++ )
        {
            b[p] +=
                ( float ) ( scale *
                            cos ( t * ( 2 * x + 1 ) * ( 2 * k + 1 ) ) *
                            cos ( t * ( 2 * p + 1 ) * ( 2 * k + 1 ) ) );
        }
    }

    return N;
}

/*-------------------------------------------------------------*/
static void
fc_window ( float x[], int n )
{
    int i;
    double pi, t, w;

    pi = 4.0 * atan ( 1.0 );
    t = 2.0 * pi / n;

    for ( i = 0; i < n; i++ )
    {
        w = 0.5 * ( 1.0 - cos ( ( i + 0.5 ) * t ) );
        w = .5 + .5 * w;
        x[i] = ( float ) ( w * x[i] );

    }

}

/*-------------------------------------------------------------*/
static void
fc_window2 ( float x[], int n, float x0 )
{
    int i;
    double pi, t, w, w0;

    pi = 4.0 * atan ( 1.0 );
    t = 2.0 * pi / n;

    w0 = 0.5 * ( 1.0 - cos ( ( x0 + 0.5 ) * t ) );
    w0 = .5 + .5 * w0;
    w0 = 1.0 / w0;

    for ( i = 0; i < n; i++ )
    {
        w = 0.5 * ( 1.0 - cos ( ( i + 0.5 ) * t ) );
        w = .5 + .5 * w;
        x[i] = ( float ) ( w0 * w * x[i] );

    }

}

/*--------------------------------------------------------------------*/
static void
norm ( float x[], int n )
{
    int i;
    float sum;

    sum = 0.0f;
    for ( i = 0; i < n; i++ )
        sum += x[i];
    for ( i = 0; i < n; i++ )
        x[i] = x[i] / sum;

    return;
}

/*--------------------------------------------------------------------*/

/********************************************************************/

/********************************************************************/

/********************************************************************/

/********************************************************************/
//extern SRC_STRUCT src;
//static int src_bytes_out;

//typedef int (*F_FUNCTION)(void *, void *);
//static F_FUNCTION src_filter;

/*-------
extern int src_filter_mono_case0( short x[], short y[]);
extern int src_filter_mono_case1( short x[], short y[]);
extern int src_filter_mono_case2( short x[], short y[]);
extern int src_filter_mono_case3( short x[], short y[]);
extern int src_filter_mono_case4( short x[], short y[]);

extern int src_filter_dual_case0( short x[], short y[]);
extern int src_filter_dual_case1( short x[], short y[]);
extern int src_filter_dual_case2( short x[], short y[]);
extern int src_filter_dual_case3( short x[], short y[]);
extern int src_filter_dual_case4( short x[], short y[]);

extern int src_filter_to_mono_case0( short x[], short y[]);
extern int src_filter_to_mono_case1( short x[], short y[]);
extern int src_filter_to_mono_case2( short x[], short y[]);
extern int src_filter_to_mono_case3( short x[], short y[]);
extern int src_filter_to_mono_case4( short x[], short y[]);

// 8 bit input
extern int src_bfilter_mono_case0( unsigned char x[], short y[]);
extern int src_bfilter_mono_case1( unsigned char x[], short y[]);
extern int src_bfilter_mono_case2( unsigned char x[], short y[]);
extern int src_bfilter_mono_case3( unsigned char x[], short y[]);
extern int src_bfilter_mono_case4( unsigned char x[], short y[]);

extern int src_bfilter_dual_case0( unsigned char x[], short y[]);
extern int src_bfilter_dual_case1( unsigned char x[], short y[]);
extern int src_bfilter_dual_case2( unsigned char x[], short y[]);
extern int src_bfilter_dual_case3( unsigned char x[], short y[]);
extern int src_bfilter_dual_case4( unsigned char x[], short y[]);

extern int src_bfilter_to_mono_case0( unsigned char x[], short y[]);
extern int src_bfilter_to_mono_case1( unsigned char x[], short y[]);
extern int src_bfilter_to_mono_case2( unsigned char x[], short y[]);
extern int src_bfilter_to_mono_case3( unsigned char x[], short y[]);
extern int src_bfilter_to_mono_case4( unsigned char x[], short y[]);
-----*/

/*---------------------------------
static const F_FUNCTION src_filter_table[2][3][5] = {
src_filter_mono_case0,
src_filter_mono_case1,
src_filter_mono_case2,
src_filter_mono_case3,
src_filter_mono_case4,

src_filter_dual_case0,
src_filter_dual_case1,
src_filter_dual_case2,
src_filter_dual_case3,
src_filter_dual_case4,

src_filter_to_mono_case0,
src_filter_to_mono_case1,
src_filter_to_mono_case2,
src_filter_to_mono_case3,
src_filter_to_mono_case4,

// 8 bit input
src_bfilter_mono_case0,
src_bfilter_mono_case1,
src_bfilter_mono_case2,
src_bfilter_mono_case3,
src_bfilter_mono_case4,

src_bfilter_dual_case0,
src_bfilter_dual_case1,
src_bfilter_dual_case2,
src_bfilter_dual_case3,
src_bfilter_dual_case4,

src_bfilter_to_mono_case0,
src_bfilter_to_mono_case1,
src_bfilter_to_mono_case2,
src_bfilter_to_mono_case3,
src_bfilter_to_mono_case4,

};
--------------------*/

/*====================================================================*/
int
Csrc::sr_convert_init ( int source, int channels, int bits,
                        int target, int target_channels,
                        int *encode_cutoff_freq )
{
    int min_samps, min_inbuf_bytes;
    int kfilter, byte_input;

    memset ( &src, 0, sizeof ( src ) ); // mod 12/15/98 for identical bitstreams
    // on second time around encodes

    if ( ( bits != 16 ) && ( bits != 8 ) )
        return 0;
    if ( channels < 1 )
        return 0;
    if ( channels > 2 )
        return 0;
    if ( source < 8000 )
        return 0;
    if ( source > 48000 )
        return 0;
    if ( target < 5000 )
        return 0;
    if ( target > 50400 )
        return 0;

    if ( target_channels < 1 )
        target_channels = 1;
    if ( target_channels > channels )
        target_channels = channels;

    kfilter = 0;
    if ( ( channels == 2 ) && ( target_channels == 2 ) )
        kfilter = 1;
    if ( ( channels == 2 ) && ( target_channels == 1 ) )
        kfilter = 2;

    byte_input = 0;
    if ( bits == 8 )
        byte_input = 1;

    min_samps = gen_src_filter ( source, target );      // return 0 if can't handle
    if ( min_samps <= 0 )
        return 0;

    min_inbuf_bytes = min_samps * channels * bits / 8;

    src_bytes_out = sizeof ( short ) * target_channels * 1152;

//src_filter = src_filter_table[byte_input][kfilter][src.ncase];
    src_filter = ( 3 * 5 ) * byte_input + 5 * kfilter + src.ncase;

/* return cutoff frequency for encoder */
    *encode_cutoff_freq = ( int ) ( 0.90f * HX_MIN ( target, source ) / 2 );

    return min_inbuf_bytes;
}

/*--------------------------------------------------------------------*/
IN_OUT
Csrc::sr_convert ( unsigned char xin[], short yout[] )
{
    IN_OUT x;

    typedef short short_pair[2];
    typedef unsigned char uchar_pair[2];

//x.in_bytes  = src_filter(xin, yout);

    switch ( src_filter )
    {
    case 0:
        x.in_bytes = src_filter_mono_case0 ( ( short * ) xin, yout );
        break;
    case 1:
        x.in_bytes = src_filter_mono_case1 ( ( short * ) xin, yout );
        break;
    case 2:
        x.in_bytes = src_filter_mono_case2 ( ( short * ) xin, yout );
        break;
    case 3:
        x.in_bytes = src_filter_mono_case3 ( ( short * ) xin, yout );
        break;
    case 4:
        x.in_bytes = src_filter_mono_case4 ( ( short * ) xin, yout );
        break;
//----
    case 5:
        x.in_bytes = src_filter_dual_case0 ( ( short * ) xin, yout );
        break;
    case 6:
        x.in_bytes =
            src_filter_dual_case1 ( ( short_pair * ) xin,
                                    ( short_pair * ) yout );
        break;
    case 7:
        x.in_bytes =
            src_filter_dual_case2 ( ( short_pair * ) xin,
                                    ( short_pair * ) yout );
        break;
    case 8:
        x.in_bytes =
            src_filter_dual_case3 ( ( short_pair * ) xin,
                                    ( short_pair * ) yout );
        break;
    case 9:
        x.in_bytes =
            src_filter_dual_case4 ( ( short_pair * ) xin,
                                    ( short_pair * ) yout );
        break;
//---
    case 10:
        x.in_bytes = src_filter_to_mono_case0 ( ( short_pair * ) xin, yout );
        break;
    case 11:
        x.in_bytes = src_filter_to_mono_case1 ( ( short_pair * ) xin, yout );
        break;
    case 12:
        x.in_bytes = src_filter_to_mono_case2 ( ( short_pair * ) xin, yout );
        break;
    case 13:
        x.in_bytes = src_filter_to_mono_case3 ( ( short_pair * ) xin, yout );
        break;
    case 14:
        x.in_bytes = src_filter_to_mono_case4 ( ( short_pair * ) xin, yout );
        break;

// 8 bit input
    case 15:
        x.in_bytes = src_bfilter_mono_case0 ( ( unsigned char * ) xin, yout );
        break;
    case 16:
        x.in_bytes = src_bfilter_mono_case1 ( ( unsigned char * ) xin, yout );
        break;
    case 17:
        x.in_bytes = src_bfilter_mono_case2 ( ( unsigned char * ) xin, yout );
        break;
    case 18:
        x.in_bytes = src_bfilter_mono_case3 ( ( unsigned char * ) xin, yout );
        break;
    case 19:
        x.in_bytes = src_bfilter_mono_case4 ( ( unsigned char * ) xin, yout );
        break;
//---
    case 20:
        x.in_bytes =
            src_bfilter_dual_case0 ( ( uchar_pair * ) xin,
                                     ( short_pair * ) yout );
        break;
    case 21:
        x.in_bytes =
            src_bfilter_dual_case1 ( ( uchar_pair * ) xin,
                                     ( short_pair * ) yout );
        break;
    case 22:
        x.in_bytes =
            src_bfilter_dual_case2 ( ( uchar_pair * ) xin,
                                     ( short_pair * ) yout );
        break;
    case 23:
        x.in_bytes =
            src_bfilter_dual_case3 ( ( uchar_pair * ) xin,
                                     ( short_pair * ) yout );
        break;
    case 24:
        x.in_bytes =
            src_bfilter_dual_case4 ( ( uchar_pair * ) xin,
                                     ( short_pair * ) yout );
        break;
//---
    case 25:
        x.in_bytes = src_bfilter_to_mono_case0 ( ( uchar_pair * ) xin, yout );
        break;
    case 26:
        x.in_bytes = src_bfilter_to_mono_case1 ( ( uchar_pair * ) xin, yout );
        break;
    case 27:
        x.in_bytes = src_bfilter_to_mono_case2 ( ( uchar_pair * ) xin, yout );
        break;
    case 28:
        x.in_bytes = src_bfilter_to_mono_case3 ( ( uchar_pair * ) xin, yout );
        break;
    case 29:
        x.in_bytes = src_bfilter_to_mono_case4 ( ( uchar_pair * ) xin, yout );
        break;

    }

    x.out_bytes = src_bytes_out;
    return x;
}

/*--------------------------------------------------------------------*/
