/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: hwin.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
#include <string.h>
#include <float.h>
#include <math.h>

//extern int band_limit_nsb;

typedef float ARRAY36[36];

/*-- windows by block type --*/
float win[4][36];

//  win global allows mix and match with xHwin.asm (PIII)
//  xHwin.asm normally uses global from hwin.asm

typedef float ARRAY8[8];

// antialias moved to hwin 11/15/99
// changed for csa[8][2] to csa[2][8] for simd, 11/15/99
float csa[2][8];        /* antialias */

// CSA global for hwin.asm xHwin.asm

/*====================================================================*/
void mdct18 ( float f[], float y[] );   /* 18 point */
void mdct6_3 ( float f[], float y[] );  /* 6 point */
void mdct6_3B ( float f[], float y[] ); /* 6 point output order x[3][192] */

/*====================================================================*/
ARRAY8 *
alias_init_addr (  )
{
    return csa;
}

/*-----------------------------------------------------------*/
ARRAY36 *
hwin_init_addr (  )
{
    return win;
}

/*====================================================================*/
void
hybrid ( float x1[], float x2[], float yout[],
         int btype, int nlong, int ntot )
{
    int i, j;
    float y[18];        /* use short buffer so output can overwrite x1 */

// btype=2 --> 0, short window hardwired, 
// if mixed use window btype index=0 for long portion
    if ( btype == 2 )
        btype = 0;

/*-- do long blocks (if any) --*/

/* nlong = number of dct's to do */
    for ( i = 0; i < nlong; i++ )
    {
        for ( j = 0; j < 9; j++ )
        {
            y[j] =
                win[btype][26 - j] * x2[8 - j] + win[btype][27 + j] * x2[9 +
                                                                         j];
            y[9 + j] =
                win[btype][j] * x1[j] + win[btype][17 - j] * x1[17 - j];
        }
        mdct18 ( y, yout );
        x1 += 18;
        x2 += 18;
        yout += 18;
    }

/*-- do short blocks (if any) --*/
    for ( ; i < ntot; i++ )
    {
        y[0] = win[2][8] * x1[12 + 2] + win[2][9] * x1[12 + 3];
        y[1] = win[2][7] * x1[12 + 1] + win[2][10] * x1[12 + 4];
        y[2] = win[2][6] * x1[12 + 0] + win[2][11] * x1[12 + 5];
        y[3] = win[2][0] * x1[6 + 0] + win[2][5] * x1[6 + 5];
        y[4] = win[2][1] * x1[6 + 1] + win[2][4] * x1[6 + 4];
        y[5] = win[2][2] * x1[6 + 2] + win[2][3] * x1[6 + 3];

        y[6 + 0] = win[2][8] * x2[2] + win[2][9] * x2[3];
        y[6 + 1] = win[2][7] * x2[1] + win[2][10] * x2[4];
        y[6 + 2] = win[2][6] * x2[0] + win[2][11] * x2[5];
        y[6 + 3] = win[2][0] * x1[12 + 0] + win[2][5] * x1[12 + 5];
        y[6 + 4] = win[2][1] * x1[12 + 1] + win[2][4] * x1[12 + 4];
        y[6 + 5] = win[2][2] * x1[12 + 2] + win[2][3] * x1[12 + 3];

        y[12 + 0] = win[2][8] * x2[6 + 2] + win[2][9] * x2[6 + 3];
        y[12 + 1] = win[2][7] * x2[6 + 1] + win[2][10] * x2[6 + 4];
        y[12 + 2] = win[2][6] * x2[6 + 0] + win[2][11] * x2[6 + 5];
        y[12 + 3] = win[2][0] * x2[0] + win[2][5] * x2[5];
        y[12 + 4] = win[2][1] * x2[1] + win[2][4] * x2[4];
        y[12 + 5] = win[2][2] * x2[2] + win[2][3] * x2[3];

        mdct6_3 ( y, yout );
        x1 += 18;
        x2 += 18;
        yout += 18;
    }

}

/*====================================================================*/
void
hybridLong ( float x1[], float x2[], float yout[],
             int btype, int nlong, int clear_flag )
{
    int i, j;
    float y[18];        /* use short buffer so output can overwrite x1 */

// long blocks only  block types  0, 1, 3
// no mixed
//

/* nlong = number of dct's to do */
    for ( i = 0; i < nlong; i++ )
    {
        for ( j = 0; j < 9; j++ )
        {
            y[j] =
                win[btype][26 - j] * x2[8 - j] + win[btype][27 + j] * x2[9 +
                                                                         j];
            y[9 + j] =
                win[btype][j] * x1[j] + win[btype][17 - j] * x1[17 - j];
        }
        mdct18 ( y, yout );
        x1 += 18;
        x2 += 18;
        yout += 18;
    }

// clear_flag indicates remaining yout need to be cleared
// since previous use of yout was for shorts
    if ( clear_flag )
    {
        memset ( yout, 0, sizeof ( float ) * 18 * ( 32 - nlong ) );
    }

}

/*====================================================================*/
void
hybridShort_01 ( float x1[], float x2[], float yout[], int n )
{
    int i;
    float y[18];

// short btype=2 , short window only
// no mixed

// n = number of dct's (triplets) to do = number polyphase filter bands

    for ( i = 0; i < n; i++ )
    {
        y[0] = win[2][8] * x1[12 + 2] + win[2][9] * x1[12 + 3];
        y[1] = win[2][7] * x1[12 + 1] + win[2][10] * x1[12 + 4];
        y[2] = win[2][6] * x1[12 + 0] + win[2][11] * x1[12 + 5];
        y[3] = win[2][0] * x1[6 + 0] + win[2][5] * x1[6 + 5];
        y[4] = win[2][1] * x1[6 + 1] + win[2][4] * x1[6 + 4];
        y[5] = win[2][2] * x1[6 + 2] + win[2][3] * x1[6 + 3];

        y[6 + 0] = win[2][8] * x2[2] + win[2][9] * x2[3];
        y[6 + 1] = win[2][7] * x2[1] + win[2][10] * x2[4];
        y[6 + 2] = win[2][6] * x2[0] + win[2][11] * x2[5];
        y[6 + 3] = win[2][0] * x1[12 + 0] + win[2][5] * x1[12 + 5];
        y[6 + 4] = win[2][1] * x1[12 + 1] + win[2][4] * x1[12 + 4];
        y[6 + 5] = win[2][2] * x1[12 + 2] + win[2][3] * x1[12 + 3];

        y[12 + 0] = win[2][8] * x2[6 + 2] + win[2][9] * x2[6 + 3];
        y[12 + 1] = win[2][7] * x2[6 + 1] + win[2][10] * x2[6 + 4];
        y[12 + 2] = win[2][6] * x2[6 + 0] + win[2][11] * x2[6 + 5];
        y[12 + 3] = win[2][0] * x2[0] + win[2][5] * x2[5];
        y[12 + 4] = win[2][1] * x2[1] + win[2][4] * x2[4];
        y[12 + 5] = win[2][2] * x2[2] + win[2][3] * x2[3];

        mdct6_3 ( y, yout );
        x1 += 18;
        x2 += 18;
        yout += 18;
    }

}

/*====================================================================*/
void
hybridShort ( float x1[], float x2[], float yout[], int n )
{
    int i;
    float y[18];

// output ordered for x[3][192]
//
// short btype=2 , short window only
// no mixed
//
// n = number of dct's (triplets) to do = number polyphase filter bands

    for ( i = 0; i < n; i++ )
    {
        y[0] = win[2][8] * x1[12 + 2] + win[2][9] * x1[12 + 3];
        y[1] = win[2][7] * x1[12 + 1] + win[2][10] * x1[12 + 4];
        y[2] = win[2][6] * x1[12 + 0] + win[2][11] * x1[12 + 5];
        y[3] = win[2][0] * x1[6 + 0] + win[2][5] * x1[6 + 5];
        y[4] = win[2][1] * x1[6 + 1] + win[2][4] * x1[6 + 4];
        y[5] = win[2][2] * x1[6 + 2] + win[2][3] * x1[6 + 3];

        y[6 + 0] = win[2][8] * x2[2] + win[2][9] * x2[3];
        y[6 + 1] = win[2][7] * x2[1] + win[2][10] * x2[4];
        y[6 + 2] = win[2][6] * x2[0] + win[2][11] * x2[5];
        y[6 + 3] = win[2][0] * x1[12 + 0] + win[2][5] * x1[12 + 5];
        y[6 + 4] = win[2][1] * x1[12 + 1] + win[2][4] * x1[12 + 4];
        y[6 + 5] = win[2][2] * x1[12 + 2] + win[2][3] * x1[12 + 3];

        y[12 + 0] = win[2][8] * x2[6 + 2] + win[2][9] * x2[6 + 3];
        y[12 + 1] = win[2][7] * x2[6 + 1] + win[2][10] * x2[6 + 4];
        y[12 + 2] = win[2][6] * x2[6 + 0] + win[2][11] * x2[6 + 5];
        y[12 + 3] = win[2][0] * x2[0] + win[2][5] * x2[5];
        y[12 + 4] = win[2][1] * x2[1] + win[2][4] * x2[4];
        y[12 + 5] = win[2][2] * x2[2] + win[2][3] * x2[3];

        mdct6_3B ( y, yout );
        x1 += 18;
        x2 += 18;
        yout += 6;
    }

// must make sure remaining part of buffer is clear
// in case yout was previously written with long data.
    for ( i = 0; i < 6 * ( 32 - n ); i++ )
    {
        yout[i] = 0.0f;
        yout[i + 192] = 0.0f;
        yout[i + 2 * 192] = 0.0f;
    }

}

/*--------------------------------------------------------------------*/
void
FreqInvert ( float y[32][18], int nsb )
{
    int i, j;

    for ( j = 0; j < nsb; j += 2 )
    {
        for ( i = 0; i < 18; i += 2 )
        {
            y[1 + j][1 + i] = -y[1 + j][1 + i];
        }
    }

}

/*--------------------------------------------------------------------*/
void
antialias ( float x[], int n )
{
    int i, k;
    float a, b;

    n--;        /* half size butterfly on last one */
    for ( k = 0; k < n; k++ )
    {
        for ( i = 0; i < 8; i++ )
        {
            a = x[17 - i];
            b = x[18 + i];
            x[17 - i] = a * csa[0][i] + b * csa[1][i];
            x[18 + i] = b * csa[0][i] - a * csa[1][i];
        }
        x += 18;
    }

/* half on last */
    for ( i = 0; i < 8; i++ )
    {
        x[17 - i] = x[17 - i] * csa[0][i];
    }

}

/*===============================================================*/
