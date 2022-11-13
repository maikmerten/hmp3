/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: srccfb.cpp,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
 * sample rate conversion filters - 8 bit input
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
Csrc::src_bfilter_mono_case0 ( unsigned char x[], short y[] )
{
    int i;

    for ( i = 0; i < 1152; i++ )
        y[i] = ( short ) ( ( ( ( int ) x[i] ) - 128 ) << 8 );

    return 1152;        // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_mono_case1 ( unsigned char x[], short y[] )
{
    int i, k;
    int a, b;

/* up sample 1:2 */

    k = 0;
    a = ( ( ( ( int ) x[0] ) - 128 ) << 8 );
    for ( i = 0; i < 576; i += 2, k += 4 )
    {
        b = ( ( ( ( int ) x[i + 1] ) - 128 ) << 8 );
        y[k] = ( short ) ( a );
        y[k + 1] = ( short ) ( ( a + b ) >> 1 );
        a = ( ( ( ( int ) x[i + 2] ) - 128 ) << 8 );
        y[k + 2] = ( short ) ( b );
        y[k + 3] = ( short ) ( ( a + b ) >> 1 );

    }
    return 576; // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_mono_case2 ( unsigned char x[], short y[] )
{
    int i, k;
    int a, b;

/* up sample by m:n */

    k = 0;
    a = ( ( ( ( int ) x[0] ) - 128 ) << 8 );
    b = ( ( ( ( int ) x[0 + 1] ) - 128 ) << 8 ) - a;
    for ( i = 0; i < 1152; i++ )
    {
        y[i] = ( short ) ( a + src.coef[src.ic] * b );
        src.ic++;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
            a = a + b;
            b = ( ( ( ( int ) x[k + 1] ) - 128 ) << 8 ) - a;
        }
    }
    return k;   // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_mono_case3 ( unsigned char x[], short y[] )
{
    int i, j, k;
    int t;
    float u;

/* down sample by m:n for small n - coef table holds n filters */

/* could double buffer 8 bit conversion */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * ( ( ( ( int ) x[k + j] ) - 128 ) << 8 );
        t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) ( t );
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
    return k;   // number bytes taken in
}

/*====================================================================*/
int
Csrc::stage1b_mono ( unsigned char x[] )
{
    int i, j;
    int a, b;

    src.nbuf -= src.kbuf;
    if ( src.nbuf > 0 )
        memmove ( src.buf, src.buf + src.kbuf, sizeof ( float ) * src.nbuf );
    src.kbuf = 0;
    a = ( ( ( ( int ) x[0] ) - 128 ) << 8 );
    b = ( ( ( ( int ) x[0 + 1] ) - 128 ) << 8 ) - a;
    for ( j = 0, i = 0; i < 128; i++ )
    {
        src.buf[src.nbuf++] = a + src.coef1[src.ic1] * b;
        src.ic1++;
        if ( src.ic1 >= src.totcoef1 )
            src.ic1 = 0;
        src.am1 -= src.m1;
        if ( src.am1 <= 0 )
        {
            src.am1 += src.n1;
            j++;
            a = a + b;
            b = ( ( ( ( int ) x[j + 1] ) - 128 ) << 8 ) - a;
        }
    }

    return j;   // samples removed from x[]
}

/*====================================================================*/
int
Csrc::src_bfilter_mono_case4 ( unsigned char x[], short y[] )
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
            k0 += stage1b_mono ( x + k0 );
            thres = src.nbuf - src.ntaps;
        }
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * src.buf[src.kbuf + j];
        t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) ( t );
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
    return k0;  // number bytes taken in
}

/*====================================================================*/

/*================= dual channel =====================================*/

/*====================================================================*/

/*====================================================================*/
int
Csrc::src_bfilter_dual_case0 ( unsigned char x[][2], short y[][2] )
{
    int i;

    for ( i = 0; i < 1152; i++ )
    {
        y[i][0] = ( short ) ( ( ( ( int ) x[i][0] ) - 128 ) << 8 );
        y[i][1] = ( short ) ( ( ( ( int ) x[i][1] ) - 128 ) << 8 );
    }

    return 2 * 1152;    // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_dual_case1 ( unsigned char x[][2], short y[][2] )
{
    int i, k;
    int a0, a1, b;

/* up sample 1:2 */

    k = 0;
    a0 = ( ( ( ( int ) x[0][0] ) - 128 ) << 8 );
    a1 = ( ( ( ( int ) x[0][1] ) - 128 ) << 8 );
    for ( i = 0; i < 576; i += 2, k += 4 )
    {
        b = ( ( ( ( int ) x[i + 1][0] ) - 128 ) << 8 );
        y[k][0] = ( short ) ( a0 );
        y[k + 1][0] = ( short ) ( ( a0 + b ) >> 1 );
        a0 = ( ( ( ( int ) x[i + 2][0] ) - 128 ) << 8 );
        y[k + 2][0] = ( short ) ( b );
        y[k + 3][0] = ( short ) ( ( a0 + b ) >> 1 );

        b = ( ( ( ( int ) x[i + 1][1] ) - 128 ) << 8 );
        y[k][1] = ( short ) ( a1 );
        y[k + 1][1] = ( short ) ( ( a1 + b ) >> 1 );
        a1 = ( ( ( ( int ) x[i + 2][1] ) - 128 ) << 8 );
        y[k + 2][1] = ( short ) ( b );
        y[k + 3][1] = ( short ) ( ( a1 + b ) >> 1 );
    }

    return 2 * 576;     // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_dual_case2 ( unsigned char x[][2], short y[][2] )
{
    int i, k;
    int a0, b0, a1, b1;

/* up sample by m:n */

    k = 0;
    a0 = ( ( ( ( int ) x[0][0] ) - 128 ) << 8 );
    a1 = ( ( ( ( int ) x[0][1] ) - 128 ) << 8 );
    b0 = ( ( ( ( int ) x[0 + 1][0] ) - 128 ) << 8 ) - a0;
    b1 = ( ( ( ( int ) x[0 + 1][1] ) - 128 ) << 8 ) - a1;
    for ( i = 0; i < 1152; i++ )
    {
        y[i][0] = ( short ) ( a0 + src.coef[src.ic] * b0 );
        y[i][1] = ( short ) ( a1 + src.coef[src.ic] * b1 );
        src.ic++;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
            a0 = a0 + b0;
            a1 = a1 + b1;
            b0 = ( ( ( ( int ) x[k + 1][0] ) - 128 ) << 8 ) - a0;
            b1 = ( ( ( ( int ) x[k + 1][1] ) - 128 ) << 8 ) - a1;
        }
    }

    return 2 * k;       // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_dual_case3 ( unsigned char x[][2], short y[][2] )
{
    int i, j, k;
    int tu, tv;
    float u, v;

/* down sample by m:n for small n - coef table holds n filters */

/* could double buffer 8 bit conversion */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        v = u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
        {
            u += src.coef[src.ic] *
                ( ( ( ( int ) x[k + j][0] ) - 128 ) << 8 );
            v += src.coef[src.ic++] *
                ( ( ( ( int ) x[k + j][1] ) - 128 ) << 8 );
        }
        tu = ( int ) u;
        tv = ( int ) v;
        if ( tu > 32767 )
            tu = 32767;
        else if ( tu < -32767 )
            tu = -32767;
        if ( tv > 32767 )
            tv = 32767;
        else if ( tv < -32767 )
            tv = -32767;
        y[i][0] = ( short ) ( tu );
        y[i][1] = ( short ) ( tv );
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
    return 2 * k;       // number bytes taken in
}

/*====================================================================*/

/*====================================================================*/
int
Csrc::stage1b_dual ( unsigned char x[][2] )
{
    int i, j;
    int a0, b0, a1, b1;

    src.nbuf -= src.kbuf;
    if ( src.nbuf > 0 )
    {
        memmove ( src.buf, src.buf + src.kbuf, sizeof ( float ) * src.nbuf );
        memmove ( src.buf2, src.buf2 + src.kbuf,
                  sizeof ( float ) * src.nbuf );
    }
    src.kbuf = 0;
    a0 = ( ( ( ( int ) x[0][0] ) - 128 ) << 8 );
    b0 = ( ( ( ( int ) x[0 + 1][0] ) - 128 ) << 8 ) - a0;
    a1 = ( ( ( ( int ) x[0][1] ) - 128 ) << 8 );
    b1 = ( ( ( ( int ) x[0 + 1][1] ) - 128 ) << 8 ) - a1;
    for ( j = 0, i = 0; i < 128; i++ )
    {
        src.buf[src.nbuf] = a0 + src.coef1[src.ic1] * b0;
        src.buf2[src.nbuf++] = a1 + src.coef1[src.ic1] * b1;
        src.ic1++;
        if ( src.ic1 >= src.totcoef1 )
            src.ic1 = 0;
        src.am1 -= src.m1;
        if ( src.am1 <= 0 )
        {
            src.am1 += src.n1;
            j++;
            a0 = a0 + b0;
            b0 = ( ( ( ( int ) x[j + 1][0] ) - 128 ) << 8 ) - a0;
            a1 = a1 + b1;
            b1 = ( ( ( ( int ) x[j + 1][1] ) - 128 ) << 8 ) - a1;
        }
    }

    return j;   // samples removed from x[][2]
}

/*====================================================================*/
int
Csrc::src_bfilter_dual_case4 ( unsigned char x[][2], short y[][2] )
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
            k0 += stage1b_dual ( x + k0 );
            thres = src.nbuf - src.ntaps;
        }
        v = u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
        {
            u += src.coef[src.ic] * src.buf[src.kbuf + j];
            v += src.coef[src.ic++] * src.buf2[src.kbuf + j];
        }
        tu = ( int ) u;
        tv = ( int ) v;
        if ( tu > 32767 )
            tu = 32767;
        else if ( tu < -32767 )
            tu = -32767;
        if ( tv > 32767 )
            tv = 32767;
        else if ( tv < -32767 )
            tv = -32767;
        y[i][0] = ( short ) ( tu );
        y[i][1] = ( short ) ( tv );
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
    return 2 * k0;      // number bytes taken in
}

/*====================================================================*/

/*====================================================================*/

/*================= dual input convert to mono =======================*/

/*====================================================================*/

/*====================================================================*/
int
Csrc::src_bfilter_to_mono_case0 ( unsigned char x[][2], short y[] )
{
    int i;

    for ( i = 0; i < 1152; i++ )
    {
        y[i] =
            ( short ) ( ( ( ( int ) x[i][0] + ( int ) x[i][1] ) -
                          256 ) << 7 );
    }

    return 2 * 1152;    // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_to_mono_case1 ( unsigned char x[][2], short y[] )
{
    int i, k;
    int a, b;

/* up sample 1:2 */

    k = 0;
    a = ( ( ( int ) x[0][1] + ( int ) x[0][1] ) - 256 );
    for ( i = 0; i < 576; i += 2, k += 4 )
    {
        b = ( ( ( int ) x[i + 1][0] + ( int ) x[i + 1][1] ) - 256 );
        y[k] = ( short ) ( a << 7 );
        y[k + 1] = ( short ) ( ( a + b ) << 6 );
        a = ( ( ( int ) x[i + 2][0] + ( int ) x[i + 2][1] ) - 256 );
        y[k + 2] = ( short ) ( b << 7 );
        y[k + 3] = ( short ) ( ( a + b ) << 6 );

    }

    return 2 * 576;     // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_to_mono_case2 ( unsigned char x[][2], short y[] )
{
    int i, k;
    int a, b;

/* up sample by m:n */

    k = 0;
    a = ( ( ( ( int ) x[0][0] + ( int ) x[0][1] ) - 256 ) << 7 );
    b = ( ( ( ( int ) x[0 + 1][0] + ( int ) x[0 + 1][1] ) - 256 ) << 7 ) - a;
    for ( i = 0; i < 1152; i++ )
    {
        y[i] = ( short ) ( a + src.coef[src.ic] * b );
        src.ic++;
        if ( src.ic >= src.totcoef )
            src.ic = 0;
        src.am -= src.m;
        if ( src.am <= 0 )
        {
            src.am += src.n;
            k++;
            a = a + b;
            b = ( ( ( ( int ) x[k + 1][0] + ( int ) x[k + 1][1] ) -
                    256 ) << 7 ) - a;
        }
    }

    return 2 * k;       // number bytes taken in
}

/*====================================================================*/
int
Csrc::src_bfilter_to_mono_case3 ( unsigned char x[][2], short y[] )
{
    int i, j, k;
    int t;
    float u;

/* down sample by m:n for small n - coef table holds n filters */

/* better to double buffer sum channel */

    k = 0;
    for ( i = 0; i < 1152; i++ )
    {
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * ( ( ( int ) x[k + j][0] +
                                          ( int ) x[k + j][1] - 256 ) << 7 );
        t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) ( t );
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
    return 2 * k;       // number bytes taken in
}

/*====================================================================*/
int
Csrc::stage1b_to_mono ( unsigned char x[][2] )
{
    int i, j;
    int a, b;

    src.nbuf -= src.kbuf;
    if ( src.nbuf > 0 )
        memmove ( src.buf, src.buf + src.kbuf, sizeof ( float ) * src.nbuf );
    src.kbuf = 0;
    a = ( ( ( int ) x[0][0] + ( int ) x[0][1] - 256 ) << 7 );
    b = ( ( ( int ) x[1][0] + ( int ) x[1][1] - 256 ) << 7 );
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
            b = ( ( ( int ) x[j + 1][0] + ( int ) x[j + 1][1] - 256 ) << 7 );
        }
    }

    return j;   // samples removed from x[]
}

/*====================================================================*/
int
Csrc::src_bfilter_to_mono_case4 ( unsigned char x[][2], short y[] )
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
            k0 += stage1b_to_mono ( x + k0 );
            thres = src.nbuf - src.ntaps;
        }
        u = 0.0f;
        for ( j = 0; j < src.ntaps; j++ )
            u += src.coef[src.ic++] * src.buf[src.kbuf + j];
        t = ( int ) u;
        if ( t > 32767 )
            t = 32767;
        else if ( t < -32767 )
            t = -32767;
        y[i] = ( short ) ( t );
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
    return 2 * k0;      // number bytes taken in
}

/*====================================================================*/
