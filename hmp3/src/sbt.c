/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: sbt.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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

/*-- window ---*/

/*------------- window scaled for idct ! ----------*/
static const float wincoef[264] = {   /* window coefs  */
#include "tableaw2.h"
};

/*-- idct ---*/
static float coef[32];  /* 32 pt inverse dct coefs */
static float a[32];     /* ping pong buffers */
static float b[32];

/*--------------------------------------------------------------------*/
void
window ( const float vbuf[] )
{
    int i, j, k;
    const float *si, *bx;
    const float *coef;
    float sum1, sum2;

/*-- output to dct ping-pong buf b --*/
    si = vbuf + 16;
    bx = vbuf + 15;

    k = 0;
    coef = wincoef;

/*-- first 1 --*/
    sum1 = 0.0f;
    for ( j = 0; j < 512; j += 64 )
    {
        sum1 += ( *coef++ ) * si[j];
    }
    si++;
    b[k++] = sum1 ;

/*-- 16 --*/
    for ( i = 0; i < 16; i++ )
    {
        sum1 = sum2 = 0.0f;
        for ( j = 0; j < 512; j += 64 )
        {
            sum1 += ( *coef++ ) * si[j];
            sum2 += ( *coef++ ) * bx[j];
        }
        si++;
        bx--;
        b[k++] = sum1+sum2;
    }

/*-- last 15 --*/
    coef = wincoef + 247;
    bx += 64;
    for ( i = 0; i < 15; i++ )
    {
        sum1 = sum2 = 0.0f;
        for ( j = 0; j < 512; j += 64 )
        {
            sum1 += ( *coef-- ) * bx[j];
            sum2 -= ( *coef-- ) * si[j];
        }
        si++;
        bx--;
        b[k++] = sum1+sum2;
    }
}

/*======================================================================*/
int
fidct_init (  ) /* gen coef for N=32 */
{
    int p, n, i, k;
    double t, pi;

    pi = 4.0 * atan ( 1.0 );
    n = 16;
    k = 0;
    for ( i = 0; i < 5; i++, n = n / 2 )
    {
        for ( p = 0; p < n; p++, k++ )
        {
            t = ( pi / ( 4 * n ) ) * ( 2 * p + 1 );
            coef[k] = ( float ) ( 2.0 * cos ( t ) );
        }
    }
    return 1;
}

/*--------------------------------------------------------------*/
static void
forward_ibf ( int m, int n, float x[], float f[] )
{
    int i, j, n2, n21;
    int p, q, p0;

    p0 = n - 1;
    n2 = n / 2;
    n21 = n2 - 1;
    for ( i = 0; i < m; i++, p0 += n )
    {
        p = p0;
        q = p0;
        f[q] = x[p--];
        f[q - n2] = x[p--];
        q--;
        for ( j = 0; j < n21; j++ )
        {
            f[q] = x[p--] - f[q + 1];
            f[q - n2] = x[p--];
            q--;
        }
    }
}

/*--------------------------------------------------------------*/
static void
back_ibf ( int m, int n, const float x[], float f[], const float coef[] )
{
    int i, j, n2;
    int p, q, p0, k;

    p0 = 0;
    n2 = n / 2;
    for ( i = 0; i < m; i++, p0 += n )
    {
        k = 0;
        p = p0;
        q = p + n - 1;
        for ( j = 0; j < n2; j++, p++, q--, k++ )
        {
            float tmp = coef[k] * x[p + n2];
            float t = x[p] ;
            f[p] = t + tmp;
            f[q] = t - tmp;
        }
    }
}

/*--------------------------------------------------------------*/

/*---------------- special last stage interleaved output -------*/
static void
back_ibf_last ( float x[], float f[] )
{
    int p, q, k;
    float tmp;

    p = 0;
    q = 2 * 31;
    for ( k = 0; k < 16; k++, p += 2, q -= 2 )
    {
        tmp = coef[k] * x[k + 16];
        f[p] = x[k] + tmp;
        f[q] = x[k] - tmp;
    }
}

/*--------------------------------------------------------------*/

/*---- special last stage Layer III -------*/
static void
back_ibf_last_L3 ( float x[], float f[] )
{
    int p, q, k;
    float tmp;

    p = 0;
    q = 18 * 31;
    for ( k = 0; k < 16; k++, p += 18, q -= 18 )
    {
        tmp = coef[k] * x[k + 16];
        f[p] = x[k] + tmp;
        f[q] = x[k] - tmp;
    }
}

/*--------------------------------------------------------------*/
void
fidct ( float c[] )
{

/*--- idct input must be scaled by window ------------------------*/

/*--- b[0]=x[0];  for(i=1;i<32;i++) b[i] = 0.5*x[i];  ----*/

    forward_ibf ( 1, 32, b, a );
    forward_ibf ( 2, 16, a, b );
    forward_ibf ( 4, 8, b, a );
    forward_ibf ( 8, 4, a, b );
    back_ibf ( 16, 2, b, a, coef + 16 + 8 + 4 + 2 );
    back_ibf ( 8, 4, a, b, coef + 16 + 8 + 4 );
    back_ibf ( 4, 8, b, a, coef + 16 + 8 );
    back_ibf ( 2, 16, a, b, coef + 16 );
    back_ibf_last ( b, c );
}

/*--------------------------------------------------------------*/
void
fidct_L3 ( float c[] )
{

/*--- idct input must be scaled by window ------------------------*/

/*--- b[0]=x[0];  for(i=1;i<32;i++) b[i] = 0.5*x[i];  ----*/

    forward_ibf ( 1, 32, b, a );
    forward_ibf ( 2, 16, a, b );
    forward_ibf ( 4, 8, b, a );
    forward_ibf ( 8, 4, a, b );
    back_ibf ( 16, 2, b, a, coef + 16 + 8 + 4 + 2 );
    back_ibf ( 8, 4, a, b, coef + 16 + 8 + 4 );
    back_ibf ( 4, 8, b, a, coef + 16 + 8 );
    back_ibf ( 2, 16, a, b, coef + 16 );
    back_ibf_last_L3 ( b, c );
}

/*====================================================================*/
void
sbt_init (  )
{
    static int first_pass = 1;

    if ( first_pass )
    {
        fidct_init (  );
        first_pass = 0;
    }
}

/*====================================================================*/
void
sbt ( float vbuf[], float samp[] )
{
    int i;

    vbuf += 1152;
    for ( i = 0; i < 36; i++ )
    {
        vbuf -= 32;
        window ( vbuf );
        fidct ( samp );
        samp += 64;     /* interleaved output */
    }

/*-- done  --*/
}

/*--------------------------------------------------------------------*/
void
sbt_L3 ( float vbuf[], float samp[] )
{
    int i;

/* do one granule */

    vbuf += 576;
    for ( i = 0; i < 18; i++ )
    {
        vbuf -= 32;
        window ( vbuf );
        fidct_L3 ( samp );
        samp++;
    }

/*-- done  --*/
}

/*--------------------------------------------------------------------*/
