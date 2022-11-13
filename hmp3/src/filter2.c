/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: filter2.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
 * Layer III MPEG audio encoder 
 * encoder input short->float cast, optional DC blocking filter
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <string.h>     // memmove proto

#include "filter2.h"

// additional delay for Layer III
// overlap = bufsize - 1152
//#define OVERLAP 480
//#define OVERLAP (2192-1152)
// 3/14/00 buf increased one granule
// 3/27/00 buf increased two granule
#define OVERLAP ((2192+1152)-1152)

/*====================================================================*/
void
filter2_init ( int samprate, int filter_select,
               int monodual, FILTER2_CONTROL * fc2 )
{

    fc2->alpha = ( float ) ( 0.001 * 44100.0 / samprate );      /* set dc filter coef */
    fc2->d = fc2->d2 = 0.0f;

    if ( filter_select < 0 )
        filter_select = 0;      // default = 0
    if ( filter_select > 1 )
        filter_select = 1;      // default = 1

    fc2->select = 2 * filter_select + monodual;

}

/*--------------------------------------------------------------------*/
void
filter2 ( short pcm[], float *buf1, float *buf2, FILTER2_CONTROL * fc2 )
{
    int i;
    float *x, *y;
    float t, alpha, d, d2;

    switch ( fc2->select )
    {
    case 0:
        // filter_mono0
        x = buf1 + 1152;
        memmove ( x, buf1, OVERLAP * sizeof ( float ) );
        for ( i = 0; i < 1152; i++ )
            *--x = pcm[i];
        return;

    case 1:
        //filter_dual0
        x = buf1 + 1152;
        y = buf2 + 1152;
        memmove ( x, buf1, OVERLAP * sizeof ( float ) );
        memmove ( y, buf2, OVERLAP * sizeof ( float ) );
        for ( i = 0; i < 2304; i += 2 )
        {
            *--x = pcm[i];
            *--y = pcm[i + 1];
        }
        return;

    case 2:
        //filter_mono_dc
        alpha = fc2->alpha;
        d = fc2->d;

        x = buf1 + 1152;
        memmove ( x, buf1, OVERLAP * sizeof ( float ) );
        for ( i = 0; i < 1152; i++ )
        {
            t = ( pcm[i] - d );
            d = d + alpha * t;
            *--x = t;
        }

        fc2->alpha = alpha;
        fc2->d = d;
        return;

    case 3:
        //filter_dual_dc
        alpha = fc2->alpha;
        d = fc2->d;
        d2 = fc2->d2;

        x = buf1 + 1152;
        y = buf2 + 1152;
        memmove ( x, buf1, OVERLAP * sizeof ( float ) );
        memmove ( y, buf2, OVERLAP * sizeof ( float ) );
        for ( i = 0; i < 2304; i += 2 )
        {
            t = ( float ) ( pcm[i] - d );
            d = d + alpha * t;
            *--x = t;
            t = pcm[i + 1] - d2;
            d2 = d2 + alpha * t;
            *--y = t;
        }

        fc2->alpha = alpha;
        fc2->d = d;
        fc2->d2 = d2;
        return;
    }   // end switch

}

/*====================================================================*/
