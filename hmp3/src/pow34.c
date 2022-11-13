/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: pow34.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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

static const union {signed int i; float f;} e34[] = {
0x000000000, 0x0103504F3, 0x0109837F0, 0x011000000,
0x0115744FD, 0x011B504F3, 0x0121837F0, 0x012800000,
0x012D744FD, 0x0133504F3, 0x0139837F0, 0x014000000,
0x0145744FD, 0x014B504F3, 0x0151837F0, 0x015800000,
0x015D744FD, 0x0163504F3, 0x0169837F0, 0x017000000,
0x0175744FD, 0x017B504F3, 0x0181837F0, 0x018800000,
0x018D744FD, 0x0193504F3, 0x0199837F0, 0x01A000000,
0x01A5744FD, 0x01AB504F3, 0x01B1837F0, 0x01B800000,
0x01BD744FD, 0x01C3504F3, 0x01C9837F0, 0x01D000000,
0x01D5744FD, 0x01DB504F3, 0x01E1837F0, 0x01E800000,
0x01ED744FD, 0x01F3504F3, 0x01F9837F0, 0x020000000,
0x0205744FD, 0x020B504F3, 0x0211837F0, 0x021800000,
0x021D744FD, 0x0223504F3, 0x0229837F0, 0x023000000,
0x0235744FD, 0x023B504F3, 0x0241837F0, 0x024800000,
0x024D744FD, 0x0253504F3, 0x0259837F0, 0x026000000,
0x0265744FD, 0x026B504F3, 0x0271837F0, 0x027800000,
0x027D744FD, 0x0283504F3, 0x0289837F0, 0x029000000,
0x0295744FD, 0x029B504F3, 0x02A1837F0, 0x02A800000,
0x02AD744FD, 0x02B3504F3, 0x02B9837F0, 0x02C000000,
0x02C5744FD, 0x02CB504F3, 0x02D1837F0, 0x02D800000,
0x02DD744FD, 0x02E3504F3, 0x02E9837F0, 0x02F000000,
0x02F5744FD, 0x02FB504F3, 0x0301837F0, 0x030800000,
0x030D744FD, 0x0313504F3, 0x0319837F0, 0x032000000,
0x0325744FD, 0x032B504F3, 0x0331837F0, 0x033800000,
0x033D744FD, 0x0343504F3, 0x0349837F0, 0x035000000,
0x0355744FD, 0x035B504F3, 0x0361837F0, 0x036800000,
0x036D744FD, 0x0373504F3, 0x0379837F0, 0x038000000,
0x0385744FD, 0x038B504F3, 0x0391837F0, 0x039800000,
0x039D744FD, 0x03A3504F3, 0x03A9837F0, 0x03B000000,
0x03B5744FD, 0x03BB504F3, 0x03C1837F0, 0x03C800000,
0x03CD744FD, 0x03D3504F3, 0x03D9837F0, 0x03E000000,
0x03E5744FD, 0x03EB504F3, 0x03F1837F0, 0x03F800000,
0x03FD744FD, 0x0403504F3, 0x0409837F0, 0x041000000,
0x0415744FD, 0x041B504F3, 0x0421837F0, 0x042800000,
0x042D744FD, 0x0433504F3, 0x0439837F0, 0x044000000,
0x0445744FD, 0x044B504F3, 0x0451837F0, 0x045800000,
0x045D744FD, 0x0463504F3, 0x0469837F0, 0x047000000,
0x0475744FD, 0x047B504F3, 0x0481837F0, 0x048800000,
0x048D744FD, 0x0493504F3, 0x0499837F0, 0x04A000000,
0x04A5744FD, 0x04AB504F3, 0x04B1837F0, 0x04B800000,
0x04BD744FD, 0x04C3504F3, 0x04C9837F0, 0x04D000000,
0x04D5744FD, 0x04DB504F3, 0x04E1837F0, 0x04E800000,
0x04ED744FD, 0x04F3504F3, 0x04F9837F0, 0x050000000,
0x0505744FD, 0x050B504F3, 0x0511837F0, 0x051800000,
0x051D744FD, 0x0523504F3, 0x0529837F0, 0x053000000,
0x0535744FD, 0x053B504F3, 0x0541837F0, 0x054800000,
0x054D744FD, 0x0553504F3, 0x0559837F0, 0x056000000,
0x0565744FD, 0x056B504F3, 0x0571837F0, 0x057800000,
0x057D744FD, 0x0583504F3, 0x0589837F0, 0x059000000,
0x0595744FD, 0x059B504F3, 0x05A1837F0, 0x05A800000,
0x05AD744FD, 0x05B3504F3, 0x05B9837F0, 0x05C000000,
0x05C5744FD, 0x05CB504F3, 0x05D1837F0, 0x05D800000,
0x05DD744FD, 0x05E3504F3, 0x05E9837F0, 0x05F000000,
0x05F5744FD, 0x05FB504F3, 0x0601837F0, 0x060800000,
0x060D744FD, 0x0613504F3, 0x0619837F0, 0x062000000,
0x0625744FD, 0x062B504F3, 0x0631837F0, 0x063800000,
0x063D744FD, 0x0643504F3, 0x0649837F0, 0x065000000,
0x0655744FD, 0x065B504F3, 0x0661837F0, 0x066800000,
0x066D744FD, 0x0673504F3, 0x0679837F0, 0x068000000,
0x0685744FD, 0x068B504F3, 0x0691837F0, 0x069800000,
0x069D744FD, 0x06A3504F3, 0x06A9837F0, 0x06B000000,
0x06B5744FD, 0x06BB504F3, 0x06C1837F0, 0x06C800000,
0x06CD744FD, 0x06D3504F3, 0x06D9837F0, 0x06E000000,
0x06E5744FD, 0x06EB504F3, 0x06F1837F0, 0x07F800000
} ;

static const union {signed int i; float f;} ab[] = {
0x03E82F5BA, 0x03F3E8806, 0x03E88E2EA, 0x03F3BBE0E,
0x03E8EB69B, 0x03F392714, 0x03E94763F, 0x03F36BB71,
0x03E9A231E, 0x03F34764B, 0x03E9FBE5A, 0x03F32538A,
0x03EA548FA, 0x03F304FAD, 0x03EAAC3E8, 0x03F2E67B3,
0x03EB02FFB, 0x03F2C9902, 0x03EB58DF2, 0x03F2AE156,
0x03EBADE80, 0x03F293EB5, 0x03EC02244, 0x03F27AF5E,
0x03EC559D4, 0x03F2631C7, 0x03ECA85B7, 0x03F24C491,
0x03ECFA66D, 0x03F236683, 0x03ED3DD93, 0x03F225006
} ;

/*------------------------------------------------------------------*/
float
fpow34 ( float x )
{
//return (float)pow(x, 0.75);
//return (float)sqrt(sqrt(x*x*x));
    return ( float ) sqrt ( x * sqrt ( x ) );
}

/*------------------------------------------------------------------*/
void
vect_fpow34 ( const float x[], float y[], int n )
{
    int i;

//for(i=0;i<n;i++) y[i] = (float)pow(x[i], 0.75);
//for(i=0;i<n;i++) y[i] = (float)sqrt(sqrt(x[i]*x[i]*x[i]));
#ifndef IEEE_FLOAT
    for ( i = 0; i < n; i++ )
        y[i] = ( float ) sqrt ( x[i] * sqrt ( x[i] ) );
#else
    /* compute fabs(y)**0.75. max error relative aprox 0.5e-4 */
    /* modelled after pow34.asm */

    const unsigned int* px = (const unsigned int*)x ;
    for (i = 0 ; i < n ; i++)
    {
        union { unsigned int i; float f;} mant = {(px[i] & 0x7FFFFFUL) | (127UL << 23)} ;
        unsigned int exponent = px[i] >> 19 ;
        unsigned int e2 = exponent >> (23-19) ;
        exponent &= 15 ;

        y[i] = (mant.f * ab[2*exponent + 1].f + ab[2*exponent].f) * e34[e2].f ;
    }
#endif
}

/*------------------------------------------------------------------*/
float
vect_fmax ( const float x[], int n )  // HX_MAX(x[i])  xmax >= 0.0
{
#ifndef IEEE_FLOAT
    int i;
    float xmax;

    xmax = 0.0f;
    for ( i = 0; i < n; i++ )
        if ( x[i] > xmax )
            xmax = x[i];
    return xmax;
#else
    /* reinterpret floating point as integer and compare. */
    int i;
    signed int xmax = 0 ;
    union {signed int i; float f;} u ;

    signed int *p = (signed int *)x ;

    for ( i = 0; i < n; i++ )
        if ( p[i] > xmax )
            xmax = p[i];
    u.i = xmax ;

    return u.f;
#endif
}

/*------------------------------------------------------------------*/
