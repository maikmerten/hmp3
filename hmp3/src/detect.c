/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: detect.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
#include <string.h>     // memmove proto

#include "balow.h"      // mbLogC

#include "hxtypes.h"

//extern int iframe;   // for testing

//====================================================================
// detect based on SBT (Polyphase filter) output
int
attack_detectSBT_igr ( float sample[576], int eng[32], int short_flag_prev )
{
    int i, j, k;
    float x, sum;
    int a, a0, a1, a2;
    int m;

//int v;
    float *y, *yy;

// transient detector - input is sbt data, 
// sbt data used as bandpass filter
//
// The commented out parameter v may have some (future)
// application in detecting passages (possibly speech) 
// that need more bits if encoded with long blocks

#undef STARTBIN
#undef NBINS

// baseline is start=4 n = 10
#define STARTBIN 4
#define NBINS 14

// SAMPLE ORGANIZED [32][18]  [FREQ][TIME]
// (sample is output from polyphase filter)
// computes one granule
//
// eng[32] bigger than necessary

// shift energy history
    memmove ( eng, eng + 9, 23 * sizeof ( int ) );

    j = 23;
    yy = sample + ( 18 * STARTBIN );
    for ( k = 0; k < 9; k++, j++ )
    {
        y = yy;
        sum = 7.0e4f;   // 
        for ( i = 0; i < NBINS; i++ )
        {       // two time bins per iter
            x = y[0] * y[0];
            sum += x;
            x = y[1] * y[1];
            sum += x;
            //
            y += 18;    // next freq bin
        }
        eng[j] = mbLogC ( sum );
        yy += 2;        // next pair time bins

    }

#define JA (17)
#define JB (29)

//v = 0;
    m = 0;

    if ( short_flag_prev == 0 )
    {
        for ( j = JA; j < JB; j++ )
        {
            a0 = HX_MAX ( eng[j - 6], eng[j - 7] );
            a1 = HX_MAX ( eng[j - 4], eng[j - 5] );
            a2 = HX_MAX ( eng[j - 2], eng[j - 3] );
            a1 = HX_MAX ( a1, a0 );
            a = HX_MAX ( a1, a2 );
            m = HX_MAX ( m, eng[j] - a );

            //v += abs(eng[j] - eng[j-1]);
        }
    }
    else
    {   // prev block was short
        for ( j = ( JA + 1 ); j < JB; j++ )
        {
            a0 = HX_MAX ( eng[j - 6], eng[j - 7] );
            a1 = HX_MAX ( eng[j - 4], eng[j - 5] );
            a2 = HX_MAX ( eng[j - 2], eng[j - 3] );
            a1 = HX_MAX ( a1, a0 );
            a = HX_MAX ( a1, a2 );
            m = HX_MAX ( m, eng[j] - a );

            //v += abs(eng[j] - eng[j-1]);
        }
    }

    return m;
}

//====================================================================
// detect based on SBT (Polyphase filter) output
int
attack_detectSBT_igr_MPEG2 ( float sample[576], int eng[32],
                             int short_flag_prev )
{
    int i, j, k;
    float x, sum;
    int a, a1, a2;
    int m;

//int v;
    float *y, *yy;

// transient detector - input is sbt data, 
// sbt data used as bandpass filter
//
// The commented out parameter v may have some (future)
// application in detecting passages (possibly speech) 
// that need more bits if encoded with long blocks

#undef STARTBIN
#undef NBINS

#define STARTBIN 8
#define NBINS 20

// SAMPLE ORGANIZED [32][18]  [FREQ][TIME]
// (sample is output from polyphase filter)
// computes one granule
//
// eng[32] bigger than necessary

// shift energy history
    memmove ( eng, eng + 9, 23 * sizeof ( int ) );

    j = 23;
    yy = sample + ( 18 * STARTBIN );
    for ( k = 0; k < 9; k++, j++ )
    {
        y = yy;
        sum = 7.0e4f;   // x^2
        for ( i = 0; i < NBINS; i++ )
        {       // two time bins per iter
            x = y[0] * y[0];
            sum += x;
            x = y[1] * y[1];
            sum += x;
            //
            y += 18;    // next freq bin
        }
        eng[j] = mbLogC ( sum );
        yy += 2;        // next pair time bins
    }

#define JA (17)
#define JB (29)

//v = 0;
    m = 0;

    if ( short_flag_prev == 0 )
    {
        for ( j = JA; j < JB; j++ )
        {
            a1 = HX_MAX ( eng[j - 4], eng[j - 5] );
            a2 = HX_MAX ( eng[j - 2], eng[j - 3] );
            a = HX_MAX ( a1, a2 );
            m = HX_MAX ( m, eng[j] - a );
            //v += abs(eng[j] - eng[j-1]);
        }
    }
    else
    {   // prev block was short
        for ( j = ( JA + 1 ); j < JB; j++ )
        {
            a1 = HX_MAX ( eng[j - 4], eng[j - 5] );
            a2 = HX_MAX ( eng[j - 2], eng[j - 3] );
            a = HX_MAX ( a1, a2 );
            m = HX_MAX ( m, eng[j] - a );
            //v += abs(eng[j] - eng[j-1]);
        }
    }

    return m;
}

//====================================================================
