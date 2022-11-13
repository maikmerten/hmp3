/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: spdsmr.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
 * MPEG audio encoder Layer III
 * acoustic model
 *
 *   Short (and fast long) block, 
 *    spread function directly computes mask
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>

#include "amod.h"
#include "l3e.h"

#include "balow.h"      // mbLogC
#include "hxtypes.h"

extern int iframe;      // for testing

/*--------------------------------------------------------------------*/
void
spd_smrShort ( float e[3][64], float esave[],
               SPD_CNTL c[65], float w[],
               SIG_MASK sm[3][12], int block_type_prev )
{
    int i, j, k, n, p;
    int npart, mpart;
    float s0, s1, s2;
    float ts0, ts1, ts2;        // for testing
    int m;
    float mask[3][12];
    float m0, m1, m2;
    float t, tmp;

//sm.sig is not used in short BitAllo

// compute mask directly via spreading function 
// and map to coder partition

// amod partition is 2x coder partition
// remember - npart may be odd

    npart = c[64].count;

    k = 0;
    for ( m = 0, i = 0; i < npart; i += 2, m++ )
    {
        p = c[i].off;
        n = c[i].count;
        s0 = s1 = s2 = 0.5f;    // quiet threshold
        for ( j = 0; j < n; j++ )
        {
            s0 += w[k] * e[0][p + j];
            s1 += w[k] * e[1][p + j];
            s2 += w[k] * e[2][p + j];
            k++;
        }
        p = c[i + 1].off;
        n = c[i + 1].count;
        ts0 = ts1 = ts2 = 0.5f; // quiet threshold
        for ( j = 0; j < n; j++ )
        {
            ts0 += w[k] * e[0][p + j];
            ts1 += w[k] * e[1][p + j];
            ts2 += w[k] * e[2][p + j];
            k++;
        }
        mask[0][m] = sm[0][m].mask = s0 + ts0;
        mask[1][m] = sm[1][m].mask = s1 + ts1;
        mask[2][m] = sm[2][m].mask = s2 + ts2;

    }

    mpart = ( npart + 1 ) >> 1;

    for ( i = 0; i < mpart; i++ )
    {
        m0 = esave[i];
        m1 = 2.0 * mask[0][i];
        m2 = 2.0 * mask[1][i];
        esave[i] = 2.0 * mask[2][i];

        if ( block_type_prev == 2 )
        {
            t = mask[0][i];
            if ( t > m0 )
            {
                tmp = 0.1f * t;
                if ( m0 > tmp )
                {
                    mask[0][i] = m0;
                }
                else
                {
                    mask[0][i] = tmp;
                }
            }
        }

        t = mask[1][i];
        if ( t > m1 )
        {
            tmp = 0.1f * t;
            if ( m1 > tmp )
            {
                mask[1][i] = m1;
            }
            else
            {
                mask[1][i] = tmp;
            }
        }
        t = mask[2][i];
        if ( t > m2 )
        {
            tmp = 0.1f * t;
            if ( m2 > tmp )
            {
                mask[2][i] = m2;
            }
            else
            {
                mask[2][i] = tmp;
            }
        }

        //sm[0][i].mask  =  mask[0][i];
        //sm[1][i].mask  =  mask[1][i];
        //sm[2][i].mask  =  mask[2][i];

        sm[0][i].mask = mask[0][i];     // add prev from esave? 
        sm[1][i].mask = mask[1][i] + 0.1f * mask[0][i];
        sm[2][i].mask = mask[2][i] + 0.1f * mask[1][i];
        //sm.sig is not used in short BitAllo
        sm[0][i].sig = 0.0f;
        sm[1][i].sig = 0.0f;
        sm[2][i].sig = 0.0f;

    }

/* done */
}

/*--------------------------------------------------------------------*/
void
spd_smrLongEcho ( float e[64], float esave[64],
                  SPD_CNTL c[65], float w[], SIG_MASK sm[], int block_type )
{
    int i, j, k, n, p, m;
    int npart, npart2;
    float s1, s2, t, x, a;
    float s;
    int snr, snr0, totsnr, nsnr, d, d0, dm0, dm;
    float xtab[42];
    float stab[42];
    float etab[42];
    int mbetab[42];
    int snrvar, dv;
    int itmp;
    int mbe;
    float emax;

//
// compute mask directly via spreading function 
// and map to coder partition
//
// need to do even number to map amod partition to coder partition
// amod partition is 2x coder partition
// last c[i].count computed may be null
//
//
//  w starts at w+128,  w = absolute thres  w64-128 reserved
//

// non-linear summation - must match value used in amodini
#define alpha 0.30f

    npart = c[64].count;
// round up to even, npart can be odd, 
// must have valid etab for mapping below to coder partition
// that uses etab[i] etab[i+1]
    npart2 = ( npart + 1 ) & ( ~1 );
    for ( i = 0; i < npart2; i++ )
    {
        t = w[i] + e[i];
        etab[i] = t;
        mbe = mbLogC ( t );
        mbetab[i] = mbe;
        xtab[i] = mbExp ( alpha * mbe );
    }

    nsnr = 0;
    snrvar = 0;
    totsnr = 0;
    snr0 = 0;
    k = 128;    // w table starts at 128
    for ( i = 0; i < npart; i++ )
    {
        p = c[i].off;
        n = c[i].count;
        s = 0.1f;
        for ( j = 0; j < n; j++ )
        {
            s += w[k] * xtab[p + j];
            k++;
        }
        s = ( 0.03f * 0.1f * 0.35f ) * mbExp ( ( 1.0f / alpha ) * mbLogC ( s ) ) + w[i];        // plus qthres
        stab[i] = s;
        // adding abs thres here again tends to raise (via d adjust below) 
        // vbr bitrate on gspi35, spfe, and piano.
        snr = mbetab[i] - mbLogC ( w[i] + s );

        if ( snr > 0 )
            nsnr++;
        totsnr += HX_MAX ( -200, snr );
        snrvar += abs ( snr - snr0 );
        snr0 = snr;
    }

    d = 0;
    if ( nsnr > 0 )
    {
        d0 = round_to_int ( 1.3f * ( totsnr / npart ) - 850 );
        itmp = snrvar / npart;
        dv = HX_MIN ( 500 - itmp, 0 );
        d = d0 + dv;
        d = HX_MAX ( d, -2000 );
        d = HX_MIN ( d, 600 );
    }
    d += 300;   // 8/16/00 bit rate adjust
    dm0 = ( 300 - d ) >> 4;     // 8/16/00

    m = 0;
    itmp = 0;
    for ( i = 0; i < npart; i += 2 )
    {
        dm = HX_MAX ( dm0 * HX_MAX ( m - 13, 0 ), 0 );
        a = mbExp ( d + dm );

        s1 = a * stab[i];
        t = esave[i];
        esave[i] = 2.0 * s1;
        if ( block_type != 3 )
        {
            if ( s1 > t )
            {   // pre-echo
                x = 0.1f * s1;
                s1 = t;
                if ( s1 < x )
                    s1 = x;
            }
        }

        s2 = a * stab[i + 1];
        t = esave[i + 1];
        esave[i + 1] = 2.0f * s2;
        if ( block_type != 3 )
        {
            if ( s2 > t )
            {
                x = 0.1f * s2;
                s2 = t;
                if ( s2 < x )
                    s2 = x;
            }
        }

        emax = etab[i];
        if ( emax < etab[i + 1] )
            emax = etab[i + 1];
        s = ( etab[i] * s1 + etab[i + 1] * s2 ) / emax;
        sm[m].sig = etab[i] + etab[i + 1];      // for use by bitallo1
        sm[m].mask = s;
        m++;
    }

/* done */
}

/*--------------------------------------------------------------------*/
