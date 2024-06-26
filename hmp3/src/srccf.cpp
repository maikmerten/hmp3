/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: 2024-03-16, Case
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
 *
 * sample rate conversion filters
 * sample rate conversion class Csrc
 *
 * case 0  No conversion
 * case 1  Up sample by 1:2.  Can be done quickly
 *         with integer ALU if output type is integer.
 * case 2  Up sample by m:n.
 * case 3  Down sample by m:n for small n. Interpolation
 *         done as part of bandpass filtering. Requires
 *         n sets of filter coefs.
 * case 4  Down sample by general m:n.  
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>

#include "srcc.h"

/*====================================================================*/

/*====================================================================*/

/*====================================================================*/
int
Csrc::src_filter_mono_case0 ( float x[], float y[] )
{

/* no filter - application should pass data directly to encoder */

    memmove ( y, x, sizeof ( float ) * 1152 );

    return sizeof ( float ) * 1152;     // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_mono_case1 ( float x[], float y[] )
{
    int i, k;
    int a, b;

/* up sample 1:2 */

    k = 0;
    a = x[0];
    for ( i = 0; i < 576; i += 2, k += 4 )
    {
        b = x[i + 1];
        y[k] = ( float ) ( a );
        y[k + 1] = ( float ) ( ( a + b ) >> 1 );
        a = x[i + 2];
        y[k + 2] = ( float ) ( b );
        y[k + 3] = ( float ) ( ( a + b ) >> 1 );

    }
    return sizeof ( float ) * 576;      // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_mono_case2 ( float x[], float y[] )
{
    int i, k;

/* up sample by m:n */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        y[i] = ( float )
            ( ( float ) x[k] +
              src.coef[src.ic] * ( ( float ) x[k + 1] - ( float ) x[k] ) );
        src.ic++;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
        }
    }
    return sizeof ( float ) * k;        // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_mono_case3 ( float x[], float y[] )
{
    int i, j, k;
    int t;
    float u;

/* down sample by m:n for small n - coef table holds n filters */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * x[k + j];
        /*t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) t;*/
        y[i] = u;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        k += src.k;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
        }
    }
    return sizeof ( float ) * k;        // number bytes taken in
}

/*====================================================================*/
int
Csrc::stage1_mono ( float x[] )
{
    int i, j;

    src.nbuf -= src.kbuf;
    if ( src.nbuf > 0 )
        memmove ( src.buf, src.buf + src.kbuf, sizeof ( float ) * src.nbuf );
    src.kbuf = 0;
    for ( j = 0, i = 0; i < 128; i++ )
    {
        src.buf[src.nbuf++] =
            ( float ) x[j] + src.coef1[src.ic1] * ( ( float ) x[j + 1] -
                                                    ( float ) x[j] );
        src.ic1++;
        if ( src.ic1 >= src.totcoef1 )
            src.ic1 = 0;
        src.am1 -= src.m1;
        if ( src.am1 <= 0 )
        {
            src.am1 += src.n1;
            j++;
        }
    }

    return j;   // samples removed from x[]
}

/*====================================================================*/
int
Csrc::src_filter_mono_case4 ( float x[], float y[] )
{
    int i, j;
    int t;
    float u;
    int thres;
    int k0;

/* two stage down sample by general m:n */

    thres = src.nbuf - src.ntaps;
    k0 = 0;
    for ( i = 0; i < 1152; i++ )
    {
        if ( src.kbuf > thres )
        {       // run stage 1
            k0 += stage1_mono ( x + k0 );
            thres = src.nbuf - src.ntaps;
        }
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * src.buf[src.kbuf + j];
        /*t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) t;*/
        y[i] = u;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.kbuf += src.k;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            src.kbuf++;
        }
    }
    return sizeof ( float ) * k0;       // number bytes taken in
}

/*====================================================================*/

/*================= dual channel =====================================*/

/*====================================================================*/

/*====================================================================*/
int
Csrc::src_filter_dual_case0 ( float x[], float y[] )
{

/* no filter - application should pass data directly to encoder */

    memmove ( y, x, sizeof ( float ) * 2 * 1152 );

    return sizeof ( float ) * 2 * 1152; // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_dual_case1 ( float x[][2], float y[][2] )
{
    int i, k;

/* up sample 1:2 */

    k = 0;
    for ( i = 0; i < 576; i++, k += 2 )
    {
        y[k][0] = x[i][0];
        y[k + 1][0] =
            ( float ) ( ( x[i][0] + x[i + 1][0] ) * 0.5 );
        y[k][1] = x[i][1];
        y[k + 1][1] =
            ( float ) ( ( x[i][1] + x[i + 1][1] ) * 0.5 );

    }
    return sizeof ( float ) * 2 * 576;  // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_dual_case2 ( float x[][2], float y[][2] )
{
    int i, k;

/* up sample by m:n */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        y[i][0] = ( float )
            ( ( float ) x[k][0] +
              src.coef[src.ic] * ( ( float ) x[k + 1][0] -
                                   ( float ) x[k][0] ) );
        y[i][1] =
            ( float ) ( ( float ) x[k][1] +
                        src.coef[src.ic] * ( ( float ) x[k + 1][1] -
                                             ( float ) x[k][1] ) );
        src.ic++;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
        }
    }
    return ( sizeof ( float ) * 2 ) * k;        // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_dual_case3 ( float x[][2], float y[][2] )
{
    int i, j, k;
    int tu, tv;
    float u, v;

/* down sample by m:n for small n - coef table holds n filters */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        v = u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
        {
            u += src.coef[src.ic] * x[k + j][0];
            v += src.coef[src.ic++] * x[k + j][1];
        }
        /*tu = ( int ) u;
        tv = ( int ) v;
        if ( tu > 32767 )
            tu = 32767;
        else if ( tu < -32767 )
            tu = -32767;
        if ( tv > 32767 )
            tv = 32767;
        else if ( tv < -32767 )
            tv = -32767;
        y[i][0] = ( short ) tu;
        y[i][1] = ( short ) tv;*/
        y[i][0] = u;
        y[i][1] = v;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        k += src.k;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
        }
    }
    return ( 2 * sizeof ( float ) ) * k;        // number bytes taken in
}

/*====================================================================*/

/*====================================================================*/
int
Csrc::stage1_dual ( float x[][2] )
{
    int i, j;

    src.nbuf -= src.kbuf;
    if ( src.nbuf > 0 )
    {
        memmove ( src.buf, src.buf + src.kbuf, sizeof ( float ) * src.nbuf );
        memmove ( src.buf2, src.buf2 + src.kbuf,
                  sizeof ( float ) * src.nbuf );
    }
    src.kbuf = 0;
    for ( j = 0, i = 0; i < 128; i++ )
    {
        src.buf[src.nbuf] =
            ( float ) x[j][0] + src.coef1[src.ic1] * ( ( float ) x[j + 1][0] -
                                                       ( float ) x[j][0] );
        src.buf2[src.nbuf++] =
            ( float ) x[j][1] + src.coef1[src.ic1] * ( ( float ) x[j + 1][1] -
                                                       ( float ) x[j][1] );
        src.ic1++;
        if ( src.ic1 >= src.totcoef1 )
            src.ic1 = 0;
        src.am1 -= src.m1;
        if ( src.am1 <= 0 )
        {
            src.am1 += src.n1;
            j++;
        }
    }

    return j;   // samples removed from x[][2]
}

/*====================================================================*/
int
Csrc::src_filter_dual_case4 ( float x[][2], float y[][2] )
{
    int i, j;
    int tu, tv;
    float u, v;
    int thres;
    int k0;

/* two stage down sample by general m:n */

    thres = src.nbuf - src.ntaps;
    k0 = 0;
    for ( i = 0; i < 1152; i++ )
    {
        if ( src.kbuf > thres )
        {       // run stage 1
            k0 += stage1_dual ( x + k0 );
            thres = src.nbuf - src.ntaps;
        }
        v = u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
        {
            u += src.coef[src.ic] * src.buf[src.kbuf + j];
            v += src.coef[src.ic++] * src.buf2[src.kbuf + j];
        }
        /*tu = ( int ) u;
        tv = ( int ) v;
        if ( tu > 32767 )
            tu = 32767;
        else if ( tu < -32767 )
            tu = -32767;
        if ( tv > 32767 )
            tv = 32767;
        else if ( tv < -32767 )
            tv = -32767;
        y[i][0] = ( short ) tu;
        y[i][1] = ( short ) tv;*/
        y[i][0] = u;
        y[i][1] = v;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.kbuf += src.k;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            src.kbuf++;
        }
    }
    return ( 2 * sizeof ( float ) ) * k0;       // number bytes taken in
}

/*====================================================================*/

/*====================================================================*/

/*================= dual input convert to mono =======================*/

/*====================================================================*/

/*====================================================================*/
int
Csrc::src_filter_to_mono_case0 ( float x[][2], float y[] )
{
    int i;

    for ( i = 0; i < 1152; i++ )
    {
        y[i] = ( float ) ( ( x[i][0] + x[i][1] ) * 0.5 );
    }

    return 2 * sizeof ( float ) * 1152; // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_to_mono_case1 ( float x[][2], float y[] )
{
    int i, k;
    float a, b;

/* up sample 1:2 */

    k = 0;
    a = x[0][0] + x[0][1];
    for ( i = 0; i < 576; i += 2, k += 4 )
    {
        b = x[i + 1][0] + x[i + 1][1];
        y[k + 1] = ( float ) ( ( a + b ) * 0.25 );
        y[k] = ( float ) ( a * 0.5 );
        a = x[i + 2][0] + x[i + 2][1];
        y[k + 3] = ( float ) ( ( a + b ) * 0.25 );
        y[k + 2] = ( float ) ( b * 0.5 );

    }
    return 2 * sizeof ( float ) * 576;  // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_to_mono_case2 ( float x[][2], float y[] )
{
    int i, k;
    float a, b;

/* up sample by m:n */

    k = 0;
    a = ( x[0][0] + x[0][1] ) * 0.5;
    b = ( ( x[0 + 1][0] + x[0 + 1][1] ) * 0.5 ) - a;
    for ( i = 0; i < 1152; i++ )
    {
        y[i] = ( float ) ( a + src.coef[src.ic] * b );
        src.ic++;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
            a = a + b;
            b = ( ( x[k + 1][0] + x[k + 1][1] ) * 0.5 ) - a;
        }
    }
    return ( 2 * sizeof ( float ) ) * k;        // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_filter_to_mono_case3 ( float x[][2], float y[] )
{
    int i, j, k;
    int t;
    float u;

/* down sample by m:n for small n - coef table holds n filters */

/* better to double buffer integer sum channel */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * ( ( x[k + j][0] +
                                          x[k + j][1] ) * 0.5 );
        /*t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) t;*/
        y[i] = u;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        k += src.k;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
        }
    }
    return ( 2 * sizeof ( float ) ) * k;        // number bytes taken in
}

/*====================================================================*/
int
Csrc::stage1_to_mono ( float x[][2] )
{
    int i, j;
    float a, b;

    src.nbuf -= src.kbuf;
    if ( src.nbuf > 0 )
        memmove ( src.buf, src.buf + src.kbuf, sizeof ( float ) * src.nbuf );
    src.kbuf = 0;
    a = ( x[0][0] + x[0][1] ) * 0.5;
    b = ( x[1][0] + x[1][1] ) * 0.5;
    for ( j = 0, i = 0; i < 128; i++ )
    {
        src.buf[src.nbuf++] = a + src.coef1[src.ic1] * ( b - a );
        src.ic1++;
        if ( src.ic1 >= src.totcoef1 )
            src.ic1 = 0;
        src.am1 -= src.m1;
        if ( src.am1 <= 0 )
        {
            src.am1 += src.n1;
            j++;
            a = b;
            b = ( x[j + 1][0] + x[j + 1][1] ) * 0.5;
        }
    }

    return j;   // samples removed from x[]
}

/*====================================================================*/
int
Csrc::src_filter_to_mono_case4 ( float x[][2], float y[] )
{
    int i, j;
    int t;
    float u;
    int thres;
    int k0;

/* two stage down sample by general m:n */

    thres = src.nbuf - src.ntaps;
    k0 = 0;
    for ( i = 0; i < 1152; i++ )
    {
        if ( src.kbuf > thres )
        {       // run stage 1
            k0 += stage1_to_mono ( x + k0 );
            thres = src.nbuf - src.ntaps;
        }
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * src.buf[src.kbuf + j];
        /*t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) t;*/
        y[i] = u;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.kbuf += src.k;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            src.kbuf++;
        }
    }
    return ( 2 * sizeof ( float ) ) * k0;       // number bytes taken in
}

/*====================================================================*/
