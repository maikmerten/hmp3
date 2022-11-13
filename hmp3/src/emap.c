/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: emap.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
 * mpeg audio encoder
 * acoustic model 
 *
 * map transform bins into acoustic model energy partition
 *
 * short blocks and quick acoustic long blocks
 *
 * This file contains no tabs. Indentation is 4 spaces.
 *
 * Original author: Tom Dwyer
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

extern int iframe;      // for testing

/*--------------------------------------------------------------------*/
void
emapShort ( float xr[][192], float e[3][64], int nsum[68] )
{
    int i, j, k;
    float s0, s1, s2;
    int n, nn;

    i = 0;
    n = nsum[64] + nsum[65] + nsum[66]; // only nsum[66] should be set
    for ( j = 0; j < n; j++ )
    {
        s0 = s1 = s2 = 0.0f;
        nn = nsum[j];
        for ( k = 0; k < nn; k++, i++ )
        {
            s0 += xr[0][i] * xr[0][i];
            s1 += xr[1][i] * xr[1][i];
            s2 += xr[2][i] * xr[2][i];
        }
        e[0][j] = s0;
        e[1][j] = s1;
        e[2][j] = s2;
    }

    for ( ; j < 64; j++ )
    {   // this necessary?
        e[0][j] = 0.0f;
        e[1][j] = 0.0f;
        e[2][j] = 0.0f;
    }

/*--- done ----*/
}

/*--------------------------------------------------------------------*/
void
emapLong ( float xr[576], float e[64], int nsum[68] )
{
    int i, j, k;
    float s;
    int n, nn;

    i = 0;
    n = nsum[64] + nsum[65] + nsum[66]; // only nsum[66] should be set
    for ( j = 0; j < n; j++ )
    {
        s = 0.0f;
        nn = nsum[j];
        for ( k = 0; k < nn; k++, i++ )
        {
            s += xr[i] * xr[i];
        }
        e[j] = s;
    }

    for ( ; j < 64; j++ )
    {   // this necessary?
        e[j] = 0.0f;
    }

/*--- done ----*/
}

/*--------------------------------------------------------------------*/
