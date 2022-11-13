/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: amodini2.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
#include <assert.h>
#include <string.h>

#include "enc.h"        /* defines peak_input */
#include "amod.h"

#include "balow.h"      // sf band table generator

#include "hxtypes.h"

static const int rate_table[2][3] = {
    { 22050, 24000, 16000 },
    { 44100, 48000, 32000 }
};

#ifdef _MSC_VER
#pragma warning(disable:4305)   // precision loss in tables
#endif

static int f_select;
static int h_id;
static int nsb;

typedef float ( *SPREAD_BARK ) ( float bz0, float bz );
static float spread_barkL2 ( float bz0, float bz );
static float spread_barkL3 ( float bz0, float bz );
static SPREAD_BARK spread_bark = spread_barkL2;

float f_to_bark ( float f );

/*====================================================================*/
static float s[64];

/*-------------------------------------------*/
static float
spread_barkL2old ( float bz0, float bz )
{

    static double a = 0.2302585093;    /*-- 0.1*log(10.0);  --*/
    double temp1, temp2, temp3;
    float s;

/*--- bz0 = signal being spread ---*/

    temp1 = ( bz0 - bz ) * 1.05;
    if ( temp1 >= 0.5 && temp1 <= 2.5 )
    {
        temp2 = temp1 - 0.5;
        temp2 = 8.0 * ( temp2 * temp2 - 2.0 * temp2 );
    }
    else
        temp2 = 0;
    temp1 += 0.474;
    temp3 =
        15.811389 + 7.5 * temp1 -
        17.5 * sqrt ( ( double ) ( 1.0 + temp1 * temp1 ) );
    if ( temp3 <= -100 )
        s = 0;
    else
    {
        temp3 = ( temp2 + temp3 ) * a;
        s = exp ( temp3 );
    }

    return s;
}

/*-------------------------------------------*/

/*-------------------------------------------*/

/*--------spreading function input in bark ------------*/
static float
spread_barkAC3 ( float bz0, float bz )
{
    double dz, t, t2;
    float s;

// conv_coef[i] = [bz = bval[i]]

// only upward masking, i.e. only lower freq masks higher freq

    dz = bz0 - bz;
    if ( dz < 0.0f )
        return 0.0f;    // only upward masking

    t = 7.78 * dz;
    t2 = 1.6 * dz + 22.0;
    if ( t2 < t )
        t = t2;

    s = pow ( 10.0, -0.1 * t );

    return s;
}

/*-------------------------------------------*/

/*-------------------------------------------*/

/*--------spreading function input in bark ------------*/
static float
spread_barkAC3X ( float bz0, float bz )
{
    double dz, t, t2;
    float s;

// conv_coef[i] = [bz = bval[i]]

    dz = bz0 - bz;
    if ( dz < 0.0f )
    {
        dz = -2.5 * dz;
    }

    t = 7.78 * dz;
    t2 = 1.6 * dz + 22.0;
    if ( t2 < t )
        t = t2;

    s = pow ( 10.0, -0.1 * t );

    return s;
}

/*-------------------------------------------*/

/*-------------------------------------------*/

/*--------spreading function input in bark ------------*/
static float
spread_barkPS ( float bz0, float bz )
{

    static double a = 0.2302585093;    /*-- 0.1*log(10.0);  --*/
    double tempx, tempy;
    float s;

// function given by Painter and Spanias (from Schroeder)

// conv_coef[i] = [bz = bval[i]]

    tempx = ( bz0 - bz ) * 1.00;        // P&S
//tempx = (bz0 - bz)*1.05;

    tempx += 0.474;
    tempy = 15.811389 + 7.5 * tempx - 17.5 * sqrt ( 1.0 + tempx * tempx );
    if ( tempy <= -60.0 )
        s = 0.0;
    else
        s = exp ( tempy * a );

    return s;
}

/*-------------------------------------------*/

/*-------------------------------------------*/

/*--------spreading function input in bark ------------*/
static float
spread_barkPSX ( float bz0, float bz )
{

    static double a = 0.2302585093;    /*-- 0.1*log(10.0);  --*/
    double tempx, tempy;
    double t1, t2, dt;
    float s;

// modification of function given by Painter and Spanias

// conv_coef[i] = [bz = bval[i]]

    t1 = 1.2;
    t2 = 1.2;

//dt = 0.125*(4.0 - bz0);
    dt = ( 0.5 / 7.0 ) * ( 7.0 - bz0 );
    if ( dt < 0.0 )
        dt = 0.0;
    t1 = t1 + dt;
    t2 = t2 + dt;

// decrease high masking low at high freqs
// this helps dip at 13KHz on white noise
// besides, abs thres starts to soar at 13KHz
    dt = bz0 - 22.5;
    if ( dt < 0.0 )
        dt = 0.0;
    t2 = t2 + dt;

    tempx = ( bz0 - bz );
    if ( tempx > 0.0 )
    {
        tempx = t1 * tempx;     // lower freqs masking higher
    }
    else
    {
        tempx = t2 * tempx;
    }

    tempx += 0.474;
    tempy = 15.811389 + 7.5 * tempx - 17.5 * sqrt ( 1.0 + tempx * tempx );
    if ( tempy <= -60.0 )
        s = 0.0;
    else
        s = exp ( tempy * a );

    return s;
}

/*-------------------------------------------*/

/*-------------------------------------------*/

/*--------spreading function input in bark ------------*/
static float
spread_barkL2 ( float bz0, float bz )
{

    static double a = 0.2302585093;    /*-- 0.1*log(10.0);  --*/
    double tempx, x, tempy, temp;
    float s;

// conv_coef[i] = [bz = bval[i]]

    tempx = ( bz0 - bz ) * 1.05;
//if( bz >= bz0 ) tempx = (bz0 - bz)*3.0;
//else            tempx = (bz0 - bz)*1.5;

    if ( tempx >= 0.5 && tempx <= 2.5 )
    {
        temp = tempx - 0.5;
        x = 8.0 * ( temp * temp - 2.0 * temp );
    }
    else
        x = 0.0;

    tempx += 0.474;
    tempy = 15.811389 + 7.5 * tempx - 17.5 * sqrt ( 1.0 + tempx * tempx );
    if ( tempy <= -60.0 )
        s = 0.0;
    else
        s = exp ( ( x + tempy ) * a );

    return s;
}

/*-------------------------------------------*/

/*-------------------------------------------*/

/*--------spreading function input in bark ------------*/
static float
spread_barkL3 ( float bz0, float bz )
{

    static double a = 0.2302585093;    /*-- 0.1*log(10.0);  --*/
    double tempx, x, tempy, temp;
    float s;

// conv_coef[i] = [bz = bval[i]]

    tempx = ( bz0 - bz ) * 1.05;
    if ( bz >= bz0 )
        tempx = ( bz0 - bz ) * 3.0;
    else
        tempx = ( bz0 - bz ) * 1.5;

    if ( tempx >= 0.5 && tempx <= 2.5 )
    {
        temp = tempx - 0.5;
        x = 8.0 * ( temp * temp - 2.0 * temp );
    }
    else
        x = 0.0;

    tempx += 0.474;
    tempy = 15.811389 + 7.5 * tempx - 17.5 * sqrt ( 1.0 + tempx * tempx );
    if ( tempy <= -60.0 )
        s = 0.0;
    else
        s = exp ( ( x + tempy ) * a );

    return s;
}

/*===================================================================*/

/*===================================================================*/

/*====== fast acoustic ==============================================*/

/*===================================================================*/

/*===================================================================*/
float
f_to_ATH ( float f )    // absolute threshold of hearing in db SPL
{
    float ath;

    f = 0.001f * f;     // input freq in Hz

    ath = ( float ) ( 3.64 * pow ( f, -0.80 )
                      - 6.5 * exp ( -0.6 * ( f - 3.3 ) * ( f - 3.3 ) )
                      + 1.0e-3 * f * f * f * f );

    return ath;
}

/*-----------------------------------------------------*/
float
f_to_bark ( float f )
{
    float t, tt, bz;

/*-- compute bark from freq f in Hz ---*/
    t = ( 1.0f / 1000.0f ) * f;
    tt = ( 1.0f / 7.5f ) * t;
    tt = tt * tt;
    bz = ( float ) ( 13.0 * atan ( 0.76f * t ) + 3.5 * atan ( tt ) );
    return bz;
}

/*------------------------------------------------------------------*/
float
finterp_fnc ( const float xy[][2], float x )
{
    int i;
    float y;

    for ( i = 1; i < 100; i++ )
    {
        if ( x <= xy[i][0] )
            break;
    }

    y = xy[i - 1][1] +
        ( x - xy[i - 1][0] ) * ( ( xy[i][1] - xy[i - 1][1] ) ) / ( xy[i][0] -
                                                                   xy[i -
                                                                      1][0] );

    return y;
}

//-----------------------------------------------------

/*--------generate spreading function ------------*/
static int
spread_fncB ( int j, float bval[], int npart )
{
    int i;
    float sumstest;

    sumstest = 0.0;

    for ( i = 0; i < 64; i++ )
        s[i] = 0.0;
    for ( i = 0; i < npart; i++ )
    {
        s[i] = spread_bark ( bval[j], bval[i] );
    }

    sumstest = 1.0 / sumstest;

    return 1;
}

/*-------------------------------------------*/
static int
norm_spreadB ( int npart )
{
    int j;
    double sum, a;

    sum = 0.0;
    for ( j = 0; j < npart; j++ )
        sum += s[j];
    a = 1.0 / sum;
    for ( j = 0; j < npart; j++ )
        s[j] = a * s[j];
    return 1;
}

/*-------------------------------------------*/
static float
spread_norm_factor ( int npart )
{
    int j;
    float sum, a;

    sum = 0.0;
    for ( j = 0; j < npart; j++ )
        sum += s[j];
    a = 1.0 / sum;
    return a;
}

/*---------filter spreading function-----------*/
static int
filter_spreadB ( float thres, int npart )
{
    int j, n;

    n = 0;
    for ( j = 0; j < npart; j++ )
    {
        if ( s[j] > thres )
            break;
        s[j] = 0.0;
    }
    for ( ; j < npart; j++ )
    {
        if ( s[j] <= thres )
            break;
        n++;
    }
    for ( ; j < npart; j++ )
        s[j] = 0.0;

    return n;
}

/*--------------------------------------------------------------------*/

/*-------------generate spreading function------------*/
static int
gen_spreadB ( SPD_CNTL cntl[], float w[],
              float bval[], float snr_factor[], int npart )
{
    int i, j;
    int maxpart;
    int ntot, n;
    int count, nj;
    float thres;
    float r_norm;
    float r_norm0;

    thres = 1.0e-6;
    r_norm = 1.0f;      // use typical norm factor instead of norm_spreadB
    maxpart = npart;

    for ( i = 0; i < 64; i++ )
    {   // zero out unused
        cntl[i].count = 0;
        cntl[i].off = 0;
    }

    ntot = 0;
    for ( i = 0; i < npart; i++ )
    {
        spread_fncB ( i, bval, npart );
        r_norm0 = spread_norm_factor ( npart );
        r_norm = 0.35f; // fixed factor 0.35 for L2 spread, 0.6 for L3 
        n = filter_spreadB ( thres, npart );

  /*-- gen tables ---*/
        count = 0;
        for ( j = 0; j < npart; j++ )
            if ( s[j] != 0.0f )
                break;
        nj = j;
        if ( nj >= maxpart )
            break;
        for ( ; j < maxpart; j++ )
        {
            if ( s[j] == 0.0 )
                break;
            count++;
            ntot++;
            *w++ = r_norm * snr_factor[i] * s[j];
        }
        cntl[i].count = count;
        cntl[i].off = nj;
        snr_factor[i] = r_norm * snr_factor[i]; // return total norm factor
        /*  printf("\n spread n nj  %2d %2d ", count, nj); */
    }
    cntl[64].count = i;

    return ntot;
}

/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/

/*-------------generate spreading function------------*/
static int
gen_spreadB2 ( SPD_CNTL cntl[], float w[],
               float bval[], float snr_factor[], float athres[], int npart )
{
    int i, j;
    int maxpart;
    int ntot, n;
    int count, nj;
    float thres;
    float r_norm;

    thres = 1.0e-6;
    maxpart = npart;

    for ( i = 0; i < 64; i++ )
    {   // zero out unused
        cntl[i].count = 0;
        cntl[i].off = 0;
    }

    ntot = 0;
    for ( i = 0; i < npart; i++ )
    {
        spread_fncB ( i, bval, npart );
        r_norm = spread_norm_factor ( npart );
        //r_norm = 0.35f;   // fixed factor 
        n = filter_spreadB ( thres, npart );

  /*-- gen tables ---*/
        count = 0;
        for ( j = 0; j < npart; j++ )
            if ( s[j] != 0.0f )
                break;
        nj = j;
        if ( nj >= maxpart )
            break;
        for ( ; j < maxpart; j++ )
        {
            if ( s[j] == 0.0 )
                break;
            count++;
            ntot++;
            *w++ = snr_factor[i] * s[j];
        }
        cntl[i].count = count;
        cntl[i].off = nj;
    }
    cntl[64].count = i;

    return ntot;
}

/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/

/*====================================================================*/
void
amod_initShort ( int f_select_arg, int nsb_arg, int h_id_arg, int nsumShort[],  // epart 
                 SPD_CNTL spd_cntlShort[65], float w_spdShort[] )       // spd
{
    int i, t, m;
    int nBand_s[14];
    int partShort[32];  // cumulative partition part[0] = 0
    float snr_factor[32], x;
    float bval[32];
    int nbin, npart;
    float freq;
    int ntot;
    float b0, f0;
    float freq_tab[32];

//static float freq_snr[][2] = {    // based on spec table C.7.e
//     0.0f, 0.15f,
//   861.0f, 0.15f,
//  2584.0f, 0.18f,
//  5857.0f, 0.20f,
//  9302.0f, 0.25f,
// 13092.0f, 0.30f,
// 15500.0f, 0.35f,
// 99999.0f, 0.50f,
//};
    static float freq_dbsnrShort[][2] = {
        { 0.0f, 12.0f },
        { 861.0f, 10.0f },
        { 2584.0f, 8.0f },
        { 5857.0f, 7.0f },
        { 9302.0f, 5.0f },
        { 13092.0f, 4.0f },
        { 15500.0f, 3.0f },
        { 99999.0f, -2.0f} ,
    };

// these should be the same as long block init
    h_id = h_id_arg;
    f_select = f_select_arg & 3;
    if ( f_select == 3 )
        f_select = 0;
    nsb = nsb_arg;      // cutoff in terms of polyphase bins

    for ( i = 0; i < 64; i++ )
    {   // clear
        spd_cntlShort[i].count = 0;
        spd_cntlShort[i].off = 0;
    }

// this short model works directly on short mdct bins (192 bins)

// gen scale factor band table 
    for ( i = 0; i < ( sizeof ( nBand_s ) / sizeof ( int ) ); i++ )
        nBand_s[i] = 0;
    L3init_gen_band_table_short ( nBand_s );
//
//
#define maxpart (sizeof(partShort)/sizeof(int))
    for ( i = 0; i < maxpart; i++ )
        partShort[i] = 192;

// gen model partition = 2x sf band
    t = 0;
    for ( i = 0; i < 14; i++ )
    {
        partShort[2 * i] = t;
        m = nBand_s[i] / 2;
        t = t + m;
        partShort[2 * i + 1] = t;
        m = nBand_s[i] - m;
        t = t + m;
    }
//
    nbin = 6 * nsb;
    for ( i = 0; i < maxpart; i++ )
    {
        if ( partShort[i] >= nbin )
            break;
    }
    npart = i;
    npart = HX_MIN ( npart, 2 * 12 );      // limit to max coder sf band 

// note that npart can be odd!
//printf("\n amod npart short %3d", npart);

// compute snr factors
// gen bark values
    x = 0.5f * rate_table[h_id][f_select] / 192;
    for ( i = 0; i < maxpart - 1; i++ )
    {
        freq = x * 0.5f * ( partShort[i] + partShort[i + 1] );
        freq_tab[i] = freq;     // for debug/test
        if ( h_id == 1 )
        {       // mpeg1
            snr_factor[i] =
                0.7 * pow ( 10.0,
                            -0.1 * finterp_fnc ( freq_dbsnrShort, freq ) );
        }
        else
        {       // mpeg2
            snr_factor[i] =
                2.8 * pow ( 10.0,
                            -0.1 * finterp_fnc ( freq_dbsnrShort, freq ) );
        }
        bval[i] = f_to_bark ( freq );
    }
    snr_factor[i] = 1.0f;
    bval[i] = bval[i - 1];

// select spreading function
//   L2 sounds better than spec L3 on sqr at same short ave bits
//   spread_bark = spread_barkL2;      // baseline
//   decided to use PS over L2
//
    spread_bark = spread_barkPS;
// generate spreading function
// norm factor returned in snr_factor
    ntot = gen_spreadB ( spd_cntlShort, w_spdShort, bval, snr_factor, npart );
    assert ( ntot <= 1000 );

// generate nsum mapping (from transform xr to amod partition)
    for ( i = 0; i < 64; i++ )
        nsumShort[i] = 0;       // set unused to 0
    for ( i = 0; i < npart; i++ )
    {
        nsumShort[i] = partShort[i + 1] - partShort[i];
    }
    nsumShort[64] = 0;
    nsumShort[65] = 0;
    nsumShort[66] = npart;

// test code
    f0 = b0 = 0;
//for(i=0;i<24;i++) {
////    printf("\n %3d %6.0f %8.2f %8.2f", i, 
////        freq_tab[i], bval[i], snr_factor[i]);
//    x = 0.5*(bval[i+1] - b0);  // band width
//    freq = 0.5*(freq_tab[i+1] - f0);  // band width
//    b0 = bval[i];
//    f0 = freq_tab[i];
//    printf("\n %3d %6.0f %6.0f %8.2f %8.2f", i, 
//        freq_tab[i], freq,  bval[i], x);
//}

//ntot = 0;
//x = 0.5f*rate_table[h_id][f_select] / 192;
//for(i=0;i<12;i++) {
//    ntot += nBand_s[i];
//    freq = x*ntot; 
//    printf("\n %3d %6.0f", i, freq);
//}

    return;
}

/*====================================================================*/
void
amod_initLong ( int f_select_arg, int nsb_arg, int h_id_arg, int nsumLong[],    // epart 
                SPD_CNTL spd_cntlLong[65], float w_spdLong[] )  // spd
{
    int i, t, m;
    int nBand_l[24];
    int partLong[64];   // cumulative partition part[0] = 0
    float snr_factor[64], x;
    float bval[64];
    int nbin, npart;
    float freq;
    int ntot;
    float athres[64];
    float freq_tab[64];
    float b0, f0;

    static const float freq_dbsnrLong[][2] = {        // non-linear
        { 0,   0.0f },      //
        { 38,  0.0f },      //     0  mp3 amod bin centers at 44.1
        { 115, 0.0f },      //     1
        { 191, 0.0f },      //     2
        { 268, 0.0f },      //     3
        { 345, 0.0f },      //     4
        { 421, 0.0f },      //     5
        { 498, 0.0f },      //     6
        { 574, 0.0f },      //     7
        { 651, 1.0f },      //     8
        { 727, 1.0f },      //     9
        { 804, 2.5f },      //    10
        { 880, 2.5f },      //    11
        { 976, 1.5f },      //    12
        { 1091, 1.5f },     //    13
        { 1206, 2.0f },     //    14
        { 1321, 2.0f },     //    15
        { 1455, 2.0f },     //    16
        { 1608, 3.0f },     //    17
        { 1761, 3.0f },     //    18
        { 1914, 3.0f },     //    19
        { 2086, 3.0f },     //    20
        { 2278, 3.0f },     //    21
        { 2488, 1.0f },     //    22
        { 2718, 1.0f },     //    23
        { 2986, 0.0f },     //    24
        { 3292, 0.0f },     //    25
        { 3637, 0.0f },     //    26
        { 4020, 0.0f },     //    27
        { 4441, 0.0f },     //    28
        { 4900, 0.0f },     //    29
        { 5398, 0.0f },     //    30
        { 5934, 0.0f },     //    31
        { 6527, 0.0f },     //    32
        { 7178, 0.0f },     //    33
        { 7905, 0.0f },     //    34
        { 8709, 0.0f },     //    35
        { 9589, 0.0f },     //    36
        { 10546, 0.0f },    //    37
        { 11542, 0.0f },    //    38
        { 12575, 0.0f },    //    39
        { 13820, -2.0f },   //    40
        { 15274, -2.0f },   //    41
        { 99999, 0.0f },    //
    };

    static const float freq_absthres[][2] = {
        { 0.0f,     5.0f },
        { 350.0f,   0.03f },
        { 2584.0f,  0.01f },
        { 5857.0f,  0.01f },
        { 9302.0f,  0.03f },
        { 13092.0f, 0.5f },
        { 15500.0f, 5.0f },
        { 99999.0f, 100.0f },
    };

    memset ( athres, 0, sizeof ( athres ) );

// these should be the same as long block init
    h_id = h_id_arg;
    f_select = f_select_arg & 3;
    if ( f_select == 3 )
        f_select = 0;
    nsb = nsb_arg;      // cutoff in terms of polyphase bins

    for ( i = 0; i < 64; i++ )
    {   // clear
        spd_cntlLong[i].count = 0;
        spd_cntlLong[i].off = 0;
    }

// model works directly on long mdct bins (576 bins)

// gen scale factor band table 
    for ( i = 0; i < ( sizeof ( nBand_l ) / sizeof ( int ) ); i++ )
        nBand_l[i] = 0;
    L3init_gen_band_table_long ( nBand_l );
//
//
#undef  maxpart
#define maxpart (sizeof(partLong)/sizeof(int))
    for ( i = 0; i < maxpart; i++ )
        partLong[i] = 576;

// gen model partition = 2x sf band
    t = 0;
    for ( i = 0; i < 22; i++ )
    {
        partLong[2 * i] = t;
        m = nBand_l[i] / 2;
        t = t + m;
        partLong[2 * i + 1] = t;
        m = nBand_l[i] - m;
        t = t + m;
    }
//
    nbin = 18 * nsb;
    for ( i = 0; i < maxpart; i++ )
    {
        if ( partLong[i] >= nbin )
            break;
    }
    npart = i;
    npart = HX_MIN ( npart, 2 * 21 );      // limit to max coder sf band 

// note that npart can be odd!
//printf("\n amod npart %3d", npart);

// compute snr factors
// gen bark values
    x = 0.5f * rate_table[h_id][f_select] / 576;
    for ( i = 0; i < maxpart - 1; i++ )
    {
        freq = x * 0.5f * ( partLong[i] + partLong[i + 1] );
        freq_tab[i] = freq;     // test/debug
        snr_factor[i] =
            pow ( 10.0, -0.1 * finterp_fnc ( freq_dbsnrLong, freq ) );
        bval[i] = f_to_bark ( freq );
        athres[i] =
            finterp_fnc ( freq_absthres,
                          freq ) * ( partLong[i + 1] - partLong[i] );
    }
    snr_factor[i] = 1.0f;
    bval[i] = bval[i - 1];

//  select spreading function
//spread_bark = spread_barkPS;
    spread_bark = spread_barkPSX;       // current choice
//spread_bark = spread_barkL2;
//spread_bark = spread_barkL3;
//spread_bark = spread_barkAC3;
//spread_bark = spread_barkAC3X;

// generate spreading function
// norm factor returned in snr_factor
//ntot = gen_spreadB(spd_cntlLong, w_spdLong+128, bval, snr_factor, npart);
    ntot = gen_spreadB2 ( spd_cntlLong, w_spdLong + 128,
                          bval, snr_factor, athres, npart );
    assert ( ntot <= ( 2200 - 128 ) );

// adjust coefs for non-linear summation
#define alpha 0.30      // alpha factor must agree with SpdSmr.c
    for ( i = 128; i < ( ntot + 128 ); i++ )
    {
        if ( w_spdLong[i] > 0.0f )
            w_spdLong[i] = pow ( w_spdLong[i], alpha );
    }

// insert norm factor in w_spd
// set entire array, not just npart, to allow access beyond on odd npart
    for ( i = 0; i < 64; i++ )
    {
        w_spdLong[i] = athres[i];
    }

// generate nsum mapping (from transform xr to amod partition)
    for ( i = 0; i < 64; i++ )
        nsumLong[i] = 0;        // set unused to 0
    for ( i = 0; i < npart; i++ )
    {
        nsumLong[i] = partLong[i + 1] - partLong[i];
    }
    nsumLong[64] = 0;
    nsumLong[65] = 0;
    nsumLong[66] = npart;

// test code
    f0 = b0 = 0;
//for(i=0;i<42;i++) {
//    printf("\n %3d %6.0f %8.2f %8.2f", i, 
//        freq_tab[i], bval[i], snr_factor[i]);
//    x = 0.5*(bval[i+1] - b0);  // band width
//    freq = 0.5*(freq_tab[i+1] - f0);  // band width
//    b0 = bval[i];
//    f0 = freq_tab[i];
//    printf("\n %3d %6.0f %6.0f %8.2f %8.2f", i, 
//        freq_tab[i], freq,  bval[i], x);
//}

    return;
}

/*====================================================================*/
