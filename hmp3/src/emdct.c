/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: emdct.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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

/****************************************************************************
 *
 * $Id: emdct.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $
 *
 * Copyright (c) 1996-2001 RealNetworks Inc.
 * All Rights Reserved

/*
 *
 * C++ 5.0 bug if settings are optimize speed + blend or Pentium
 *       optimize speed + Pentium Pro is OK        
 *
 * Layer III encode
 *
 * cos transform for n=18, n=6
 *
 *  computes  c[k] =  Sum( cos((pi/4*n)*(2*k+1)*(2*p+1))*f[p] )
 *         k = 0, ...n-1,  p = 0...n-1
 *
 * inplace ok.
 *
 * Note: this inverts itself except for constant scaling.
 * Encode and decode the same except for ordering
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

/*------ 18 point xform -------*/
static float w[18];
static float w2[9];
static float coef[9][4];

static float v[6];
static float v2[3];
static float coef87;

typedef struct
{
    float *w;
    float *w2;
    void *coef;
}
IMDCT_INIT_BLOCK;

static IMDCT_INIT_BLOCK mdct_info_18 = { w, w2, coef };
static IMDCT_INIT_BLOCK mdct_info_6 = { v, v2, &coef87 };

/*====================================================================*/
IMDCT_INIT_BLOCK *
mdct_init_addr_18 (  )
{
    return &mdct_info_18;
}

IMDCT_INIT_BLOCK *
mdct_init_addr_6 (  )
{
    return &mdct_info_6;
}

/*--------------------------------------------------------------------*/
void
mdct18 ( float f[], float y[] ) /* 18 point */
{
    int p;
    float a[9], b[9];
    float ap, bp, a8p, b8p;
    float g1, g2;

    for ( p = 0; p < 4; p++ )
    {
        g1 = w[p] * f[p];
        g2 = w[17 - p] * f[17 - p];
        ap = g1 + g2;   // a[p]
        bp = w2[p] * ( g1 - g2 );       // b[p]
        g1 = w[8 - p] * f[8 - p];
        g2 = w[9 + p] * f[9 + p];
        a8p = g1 + g2;  // a[8-p]
        b8p = w2[8 - p] * ( g1 - g2 );  // b[8-p]
        a[p] = ap + a8p;
        a[5 + p] = ap - a8p;
        b[p] = bp + b8p;
        b[5 + p] = bp - b8p;
    }
    g1 = w[p] * f[p];
    g2 = w[17 - p] * f[17 - p];
    a[p] = g1 + g2;
    b[p] = w2[p] * ( g1 - g2 );

    y[0] = 0.5f * ( a[0] + a[1] + a[2] + a[3] + a[4] );
    y[1] = 0.5f * ( b[0] + b[1] + b[2] + b[3] + b[4] );

    y[2] = coef[1][0] * a[5] + coef[1][1] * a[6] + coef[1][2] * a[7]
        + coef[1][3] * a[8];
    y[3] = coef[1][0] * b[5] + coef[1][1] * b[6] + coef[1][2] * b[7]
        + coef[1][3] * b[8] - y[1];
    y[1] = y[1] - y[0];
    y[2] = y[2] - y[1];

    y[4] = coef[2][0] * a[0] + coef[2][1] * a[1] + coef[2][2] * a[2]
        + coef[2][3] * a[3] - a[4];
    y[5] = coef[2][0] * b[0] + coef[2][1] * b[1] + coef[2][2] * b[2]
        + coef[2][3] * b[3] - b[4] - y[3];
    y[3] = y[3] - y[2];
    y[4] = y[4] - y[3];

    y[6] = coef[3][0] * ( a[5] - a[7] - a[8] );
    y[7] = coef[3][0] * ( b[5] - b[7] - b[8] ) - y[5];
    y[5] = y[5] - y[4];
    y[6] = y[6] - y[5];

    y[8] = coef[4][0] * a[0] + coef[4][1] * a[1] + coef[4][2] * a[2]
        + coef[4][3] * a[3] + a[4];
    y[9] = coef[4][0] * b[0] + coef[4][1] * b[1] + coef[4][2] * b[2]
        + coef[4][3] * b[3] + b[4] - y[7];
    y[7] = y[7] - y[6];
    y[8] = y[8] - y[7];

    y[10] = coef[5][0] * a[5] + coef[5][1] * a[6] + coef[5][2] * a[7]
        + coef[5][3] * a[8];
    y[11] = coef[5][0] * b[5] + coef[5][1] * b[6] + coef[5][2] * b[7]
        + coef[5][3] * b[8] - y[9];
    y[9] = y[9] - y[8];
    y[10] = y[10] - y[9];

    y[12] = 0.5f * ( a[0] + a[2] + a[3] ) - a[1] - a[4];
    y[13] = 0.5f * ( b[0] + b[2] + b[3] ) - b[1] - b[4] - y[11];
    y[11] = y[11] - y[10];
    y[12] = y[12] - y[11];

    y[14] = coef[7][0] * a[5] + coef[7][1] * a[6] + coef[7][2] * a[7]
        + coef[7][3] * a[8];
    y[15] = coef[7][0] * b[5] + coef[7][1] * b[6] + coef[7][2] * b[7]
        + coef[7][3] * b[8] - y[13];
    y[13] = y[13] - y[12];
    y[14] = y[14] - y[13];

    y[16] = coef[8][0] * a[0] + coef[8][1] * a[1] + coef[8][2] * a[2]
        + coef[8][3] * a[3] + a[4];
    y[17] = coef[8][0] * b[0] + coef[8][1] * b[1] + coef[8][2] * b[2]
        + coef[8][3] * b[3] + b[4] - y[15];
    y[15] = y[15] - y[14];
    y[16] = y[16] - y[15];
    y[17] = y[17] - y[16];

    return;
}

/*--------------------------------------------------------------------*/

/* does 3, 6 pt dct */
void
mdct6_3 ( float f[], float c[] )        /* 6 point */
{
    int w;
    float buf[18];
    float *a;
    float g1, g2;
    float a02, b02;

    a = buf;
    for ( w = 0; w < 3; w++ )
    {
        g1 = v[0] * f[0];
        g2 = v[5] * f[5];
        a[0] = g1 + g2;
        a[3 + 0] = v2[0] * ( g1 - g2 );

        g1 = v[1] * f[1];
        g2 = v[4] * f[4];
        a[1] = g1 + g2;
        a[3 + 1] = v2[1] * ( g1 - g2 );

        g1 = v[2] * f[2];
        g2 = v[3] * f[3];
        a[2] = g1 + g2;
        a[3 + 2] = v2[2] * ( g1 - g2 );

        a += 6;
        f += 6;
    }

    a = buf;
    for ( w = 0; w < 3; w++ )
    {
        a02 = ( a[0] + a[2] );
        b02 = ( a[3 + 0] + a[3 + 2] );
        c[0] = a02 + a[1];
        c[1] = b02 + a[3 + 1];
        c[2] = coef87 * ( a[0] - a[2] );
        c[3] = coef87 * ( a[3 + 0] - a[3 + 2] ) - c[1];
        c[1] = c[1] - c[0];
        c[2] = c[2] - c[1];
        c[4] = a02 - a[1] - a[1];
        c[5] = b02 - a[3 + 1] - a[3 + 1] - c[3];
        c[3] = c[3] - c[2];
        c[4] = c[4] - c[3];
        c[5] = c[5] - c[4];
        a += 6;
        c += 6;
    }

    return;
}

/*--------------------------------------------------------------------*/

/* does 3, 6 pt dct */
// output ordered for x[3][192]
void
mdct6_3B ( float f[], float c[] )       /* 6 point */
{
    int w;
    float buf[18];
    float *a;
    float g1, g2;
    float a02, b02;

    a = buf;
    for ( w = 0; w < 3; w++ )
    {
        g1 = v[0] * f[0];
        g2 = v[5] * f[5];
        a[0] = g1 + g2;
        a[3 + 0] = v2[0] * ( g1 - g2 );

        g1 = v[1] * f[1];
        g2 = v[4] * f[4];
        a[1] = g1 + g2;
        a[3 + 1] = v2[1] * ( g1 - g2 );

        g1 = v[2] * f[2];
        g2 = v[3] * f[3];
        a[2] = g1 + g2;
        a[3 + 2] = v2[2] * ( g1 - g2 );

        a += 6;
        f += 6;
    }

    a = buf;
    for ( w = 0; w < 3; w++ )
    {
        a02 = ( a[0] + a[2] );
        b02 = ( a[3 + 0] + a[3 + 2] );
        c[0] = a02 + a[1];
        c[1] = b02 + a[3 + 1];
        c[2] = coef87 * ( a[0] - a[2] );
        c[3] = coef87 * ( a[3 + 0] - a[3 + 2] ) - c[1];
        c[1] = c[1] - c[0];
        c[2] = c[2] - c[1];
        c[4] = a02 - a[1] - a[1];
        c[5] = b02 - a[3 + 1] - a[3 + 1] - c[3];
        c[3] = c[3] - c[2];
        c[4] = c[4] - c[3];
        c[5] = c[5] - c[4];
        a += 6;
        c += 192;
    }

    return;
}

/*--------------------------------------------------------------------*/
