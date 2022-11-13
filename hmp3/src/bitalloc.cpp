/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitalloc.cpp,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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

#include "l3e.h"
#include "bitallo.h"

#include "hxtypes.h"

#include "cnttab.h"

extern "C" {
    extern int iframe; // for test
}

static unsigned char count_quada[16] = {
    1, 4 + 1, 4 + 1, 5 + 2, 4 + 1, 6 + 2, 5 + 2, 6 + 3,
    4 + 1, 5 + 2, 5 + 2, 6 + 3, 5 + 2, 6 + 3, 6 + 3, 6 + 4,
};

/*----------
extern int nBand_l[22];
extern int startBand_l[24];
extern int nBand_s[12];
extern int startBand_s[13];
-----------*/

static int cbreg[5];    /* regions by cb's */
static int rmax[3];     /* ixmax in regions  */
static int tmax[3];     /* table max in regions  */
static int ntable[4][4];        /* table number by [regions][table]  */
static int icase[4];
static void *TablePtr[4];

static int ntable_out[4];       /* table number in regions + quad */
static int nbig;
static int nquads;

static struct
{
    int ntable_out[4];
    int cbreg[3];
    int nbig;
    int nquads;
    int bits;
}
save[2];

typedef struct
{
    int bits;
    int index;
}
BI;

//typedef struct {
//    int a;
//    int b;
//}   INTPAIR;

extern "C"
{
    BI CountBits0 ( void *table, int ix[], int n );
    BI CountBits1 ( void *table, int ix[], int n );
    BI CountBits2 ( void *table, int ix[], int n );
    BI CountBits3 ( void *table, int ix[], int n );
    BI CountBits4 ( void *table, int ix[], int n );
    BI CountBits5 ( void *table, int ix[], int n );
    BI CountBitsQuad ( int ix[], int nquads );
}

typedef BI ( *COUNTSUB ) ( void *table, int ix[], int n );

static const COUNTSUB SubTable[] = {
    CountBits0,
    CountBits1,
    CountBits2,
    CountBits3,
    CountBits4,
    CountBits5,
};

static int cbreg0, cbreg1, cbreg2;

static const struct
{
    unsigned reg0;
    unsigned reg1;
}
region_table[24] =
{
    {
    1, 1}
    ,   /* 0 bands */
    {
    1, 1}
    ,   /* 1 bands */
    {
    1, 1}
    ,   /* 2 bands */
    {
    1, 1}
    ,   /* 3 bands */
    {
    1, 1}
    ,   /* 4 bands */
    {
    1, 2}
    ,   /* 5 bands */
    {
    2, 2}
    ,   /* 6 bands */
    {
    2, 2}
    ,   /* 7 bands */
    {
    2, 3}
    ,   /* 8 bands */
    {
    3, 3}
    ,   /* 9 bands */
    {
    3, 4}
    ,   /* 10 bands */
    {
    3, 5}
    ,   /* 11 bands */
    {
    4, 5}
    ,   /* 12 bands */
    {
    4, 5}
    ,   /* 13 bands */
    {
    4, 5}
    ,   /* 14 bands */
    {
    5, 6}
    ,   /* 15 bands */
    {
    5, 6}
    ,   /* 16 bands */
    {
    5, 7}
    ,   /* 17 bands */
    {
    6, 7}
    ,   /* 18 bands */
    {
    6, 7}
    ,   /* 19 bands */
    {
    6, 8}
    ,   /* 20 bands */
    {
    7, 8}
    ,   /* 21 bands */
    {
    7, 8}
    ,   /* 22 bands */
    {
    7, 8}
    ,   /* 23 bands */
};

static void pick_tables (  );

/*---------------------------------------------------------------*/
int
CBitAllo::region_aux ( int ixmax[], int ix[] )
{
    int i, bits;
    int n, n0;
    BI bi;

/* find max in each region */
    rmax[0] = 0;
    for ( i = 0; i < cbreg[0]; i++ )
        if ( rmax[0] < ixmax[i] )
            rmax[0] = ixmax[i];
    rmax[1] = 0;
    for ( ; i < cbreg[1]; i++ )
        if ( rmax[1] < ixmax[i] )
            rmax[1] = ixmax[i];
    rmax[2] = 0;
    for ( ; i < cbreg[2]; i++ )
        if ( rmax[2] < ixmax[i] )
            rmax[2] = ixmax[i];

/* choose tables by region */
    pick_tables (  );

/* count the bits */
    n0 = startBand_l[cbreg[0]];
    n = startBand_l[cbreg[1]];

    bi = SubTable[icase[0]] ( TablePtr[0], ix, n0 );
    bits = bi.bits;

    bi = SubTable[icase[1]] ( TablePtr[1], ix + n0, n - n0 );
    bits += bi.bits;

    bi = SubTable[icase[2]] ( TablePtr[2], ix + n, nbig - n );
    bits += bi.bits;

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo::divide_region3 ( int c, int ixmax[], int ix[] )
{
    int a, a0, a1;
    int b, b0, b1;
    int amin, bmin;
    int x, xmin;

// need to adjust for partial cbreg[2] (bigval)

    if ( c <= 2 )
    {
        amin = 1;
        bmin = 2;
        cbreg[0] = 1;
        cbreg[1] = 2;
        cbreg0 = amin;
        cbreg1 = bmin;
        cbreg2 = c;
        xmin = region_aux ( ixmax, ix );

        return xmin;
    }

    a0 = 1;
    a1 = HX_MIN ( c - 2, 17 );
    a1 = HX_MAX ( a1, 2 );
    xmin = 9999999;
    amin = a0;
    bmin = a0 + 1;
    for ( a = a0; a < a1; a++ )
    {
        b0 = a + 1;
        //b1 = HX_MIN(a+9, c-2);
        b1 = HX_MIN ( a + 9, c - 1 );
        cbreg[0] = a;
        for ( b = b0; b < b1; b++ )
        {
            cbreg[1] = b;
            x = region_aux ( ixmax, ix );
            if ( x < xmin )
            {
                amin = a;
                bmin = b;
                xmin = x;
            }
        }
    }

    cbreg[0] = amin;
    cbreg[1] = bmin;

    cbreg0 = amin;
    cbreg1 = bmin;
    cbreg2 = c;

    return xmin;
}

/*---------------------------------------------------------------*/
static void
pick_tables (  )
{
    int i, ixmax;

/* select tables */

    for ( i = 0; i < 3; i++ )
    {
        ixmax = rmax[i];
        ntable[i][2] = ntable[i][3] = 0;
        if ( ixmax <= 22 )
        {
            ntable[i][0] = SelectTable[ixmax][0];
            ntable[i][1] = SelectTable[ixmax][1];
            ntable[i][2] = SelectTable[ixmax][2];
            ntable[i][3] = SelectTable[ixmax][3];
            tmax[i] = SelectTable[ixmax][4];
            icase[i] = CountCase[ixmax].icase;
            TablePtr[i] = CountCase[ixmax].table;
        }
        else if ( ixmax <= 30 )
        {
            icase[i] = CountCase[23].icase;
            TablePtr[i] = CountCase[23].table;
            tmax[i] = 30;
            ntable[i][0] = 19;
            ntable[i][1] = 24;
        }
        else if ( ixmax <= 46 )
        {
            icase[i] = CountCase[24].icase;
            TablePtr[i] = CountCase[24].table;
            tmax[i] = 46;
            ntable[i][0] = 25;
            ntable[i][1] = 20;
        }
        else if ( ixmax <= 78 )
        {
            icase[i] = CountCase[25].icase;
            TablePtr[i] = CountCase[25].table;
            tmax[i] = 78;
            ntable[i][0] = 20;
            ntable[i][1] = 26;
        }
        else if ( ixmax <= 142 )
        {
            icase[i] = CountCase[26].icase;
            TablePtr[i] = CountCase[26].table;
            tmax[i] = 142;
            ntable[i][0] = 27;
            ntable[i][1] = 21;
        }
        else if ( ixmax <= 270 )
        {
            icase[i] = CountCase[27].icase;
            TablePtr[i] = CountCase[27].table;
            tmax[i] = 270;
            ntable[i][0] = 21;
            ntable[i][1] = 28;
        }
        else if ( ixmax <= 526 )
        {
            icase[i] = CountCase[28].icase;
            TablePtr[i] = CountCase[28].table;
            tmax[i] = 526;
            ntable[i][0] = 29;
            ntable[i][1] = 22;
        }
        else if ( ixmax <= 1038 )
        {
            icase[i] = CountCase[29].icase;
            TablePtr[i] = CountCase[29].table;
            tmax[i] = 1038;
            ntable[i][0] = 22;
            ntable[i][1] = 30;
        }
        else if ( ixmax <= 2062 )
        {
            icase[i] = CountCase[30].icase;
            TablePtr[i] = CountCase[30].table;
            tmax[i] = 2062;
            ntable[i][0] = 30;
            ntable[i][1] = 23;
        }
        else
        {
            icase[i] = CountCase[31].icase;
            TablePtr[i] = CountCase[31].table;
            tmax[i] = 8206;
            ntable[i][0] = 31;
            ntable[i][1] = 23;
        }
    }

}

/*---------------------------------------------------------------*/

/*---------------------------------------------------------------*/
INTPAIR
CBitAllo::subdivide2_quadregion ( int ixmax[], int ix[], int ncb, int ch )
{
    int i, j, n;
    INTPAIR x;

    for ( i = ncb - 1; i >= 0; i-- )
        if ( ixmax[i] > 0 )
            break;
    cbreg[3] = i + 1;   // end of count1  region
    for ( ; i >= 0; i-- )
        if ( ixmax[i] > 1 )
            break;
    cbreg[2] = i + 1;   // end of cb big region

/* to simply things, keep first 2 sf band big regioons */
    if ( cbreg[2] < 2 )
    {
        cbreg[2] = 2;
        if ( cbreg[3] < cbreg[2] )
            cbreg[3] = cbreg[2];
    }

/* find bigValues */
    j = startBand_l[cbreg[2]];
    n = nBand_l[cbreg[2] - 1];
    for ( i = 0; i < n; i++ )
    {
        j--;
        if ( ix[j] > 1 )
            break;
    }
    nbig = ( j + ( 1 + 1 ) ) & ( ~1 );  /* big_values = nbig/2  */

/* keep first 2 sf bands full big */
    if ( nbig < startBand_l[2] )
        nbig = startBand_l[2];

/* find count 1 values  */
    j = startBand_l[cbreg[3]];
    n = nBand_l[cbreg[3] - 1];
    for ( i = 0; i < n; i++ )
    {
        j--;
        if ( ix[j] > 0 )
            break;
    }

    nquads = ( j + ( 1 + 3 ) - nbig ) >> 2;     /* num of quads in quad region */

    x.a = nbig;
    x.b = nquads;

//bi = CountBitsQuad(ix+nbig, nquads);
//if( bi.index ) x.b = 0;

    return x;
}

/*---------------------------------------------------------------*/
int
CBitAllo::subdivide2 ( int ixmax[], int ix[], int ncb, int opti_flag, int ch )
{
    int i, j;
    int bits, n0, n;
    BI bi;

// long blocks only, call only for long blocks

    if ( block_type )
    {   // long block types 1,3, use fixed regions
        return subdivide2BT13 ( ixmax, ix, ncb, opti_flag, ch );
    }

    for ( i = ncb - 1; i >= 0; i-- )
        if ( ixmax[i] > 0 )
            break;
    cbreg[3] = i + 1;   // end of count1  region
    for ( ; i >= 0; i-- )
        if ( ixmax[i] > 1 )
            break;
    cbreg[2] = i + 1;   // end of cb big region

/* to simply things, keep first 2 sf band big regions */
    if ( cbreg[2] < 2 )
    {
        cbreg[2] = 2;
        if ( cbreg[3] < cbreg[2] )
            cbreg[3] = cbreg[2];
    }

/* find bigValues */
    j = startBand_l[cbreg[2]];
    n = nBand_l[cbreg[2] - 1];
    for ( i = 0; i < n; i++ )
    {
        j--;
        if ( ix[j] > 1 )
            break;
    }
    nbig = ( j + ( 1 + 1 ) ) & ( ~1 );  /* big_values = nbig/2  */

/* keep first 2 sf bands full big */
    if ( nbig < startBand_l[2] )
        nbig = startBand_l[2];

/* find count 1 values  */
    j = startBand_l[cbreg[3]];
    n = nBand_l[cbreg[3] - 1];
    for ( i = 0; i < n; i++ )
    {
        j--;
        if ( ix[j] > 0 )
            break;
    }

    nquads = ( j + ( 1 + 3 ) - nbig ) >> 2;     /* num of quads in quad region */

/* len reg0 limited to 16,  reg1 to  8 */
//cbreg[0] = (int)(0.33f*cbreg[2]);
//cbreg[1] = (int)(0.75f*cbreg[2]);

    cbreg[0] = region_table[cbreg[2]].reg0;
    cbreg[1] = region_table[cbreg[2]].reg0 + region_table[cbreg[2]].reg1;

//if( (opti_flag & 2)) {
    //divide_region3(cbreg[2], ixmax, ix);
//}

    if ( cbreg[0] < 1 )
        cbreg[0] = 1;
    if ( cbreg[1] <= cbreg[0] )
        cbreg[1] = cbreg[0] + 1;
    if ( cbreg[1] > cbreg[0] + 8 )
        cbreg[1] = cbreg[0] + 8;

/* find max in each region */
    rmax[0] = 0;
    for ( i = 0; i < cbreg[0]; i++ )
    {
        if ( rmax[0] < ixmax[i] )
            rmax[0] = ixmax[i];
    }
    rmax[1] = 0;
    for ( ; i < cbreg[1]; i++ )
    {
        if ( rmax[1] < ixmax[i] )
            rmax[1] = ixmax[i];
    }
    rmax[2] = 0;
    for ( ; i < cbreg[2]; i++ )
    {
        if ( rmax[2] < ixmax[i] )
            rmax[2] = ixmax[i];
    }

/* choose tables by region */
    pick_tables (  );

/* optimize/adjust regions */

/* only a few bits per frame, but helps mnr snr a little */
    if ( ( opti_flag & 1 ) )
    {
        // shrink r1 grow r2 
        if ( tmax[2] < tmax[1] )
        {
            // keep at least one cb in reg1 cbreg[1]>cbreg[0] 
            for ( j = cbreg[1] - 1; j > cbreg[0]; j-- )
            {
                if ( ixmax[j] > tmax[2] )
                    break;
            }
            cbreg[1] = j + 1;
        }
        // shrink r0 grow r1 (but not bigger that 8 cbs) 
        if ( tmax[1] < tmax[0] )
        {
            // no more than 8 in cbreg[1], cbreg[0] > 0 
            n = cbreg[1] - 8;
            if ( n < 1 )
                n = 1;
            for ( j = cbreg[0] - 1; j > n; j-- )
            {
                if ( ixmax[j] > tmax[1] )
                    break;
            }
            cbreg[0] = j + 1;
        }
    }

/* count the bits */
    n0 = startBand_l[cbreg[0]];
    n = startBand_l[cbreg[1]];

    bi = SubTable[icase[0]] ( TablePtr[0], ix, n0 );
    bits = bi.bits;
    ntable_out[0] = ntable[0][bi.index];

    bi = SubTable[icase[1]] ( TablePtr[1], ix + n0, n - n0 );
    bits += bi.bits;
    ntable_out[1] = ntable[1][bi.index];

    bi = SubTable[icase[2]] ( TablePtr[2], ix + n, nbig - n );
    bits += bi.bits;
    ntable_out[2] = ntable[2][bi.index];

    bi = CountBitsQuad ( ix + nbig, nquads );
    bits += bi.bits;
    ntable_out[3] = bi.index;

/* save essential info for output */
    save[ch].ntable_out[0] = ntable_out[0];
    save[ch].ntable_out[1] = ntable_out[1];
    save[ch].ntable_out[2] = ntable_out[2];
    save[ch].ntable_out[3] = ntable_out[3];
    save[ch].cbreg[0] = cbreg[0];
    save[ch].cbreg[1] = cbreg[1];
    save[ch].cbreg[2] = cbreg[2];
    save[ch].nbig = nbig;
    save[ch].nquads = nquads;
    save[ch].bits = bits;

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo::subdivide2BT13 ( int ixmax[], int ix[], int ncb, int opti_flag,
                           int ch )
{
    int i, j;
    int bits, n0, n;
    BI bi;

// for block types 1 and 3 (window switching flag = 1)
// regions not coded
// fixed region0 = 3 short sf bands or  8 long sf bands
// region1 covers all bands

// region0_sfb fixed at 8 for bt=1,3
#define region0_sfb 8
    cbreg[0] = region0_sfb;     // region0_sfb fixed at 8 for bt=1,3

    for ( i = ncb - 1; i >= 0; i-- )
        if ( ixmax[i] > 0 )
            break;
    cbreg[3] = i + 1;   // end of count1  region
    for ( ; i >= 0; i-- )
        if ( ixmax[i] > 1 )
            break;
    cbreg[2] = i + 1;   // end of cb big region

/* keep region0_sfb  bands big regions */
    cbreg[2] = HX_MAX ( cbreg[2], region0_sfb );
    cbreg[3] = HX_MAX ( cbreg[3], cbreg[2] );

    cbreg[1] = cbreg[0];

/* find bigValues */
    j = startBand_l[cbreg[2]];
    n = nBand_l[cbreg[2] - 1];
    for ( i = 0; i < n; i++ )
    {
        j--;
        if ( ix[j] > 1 )
            break;
    }
    nbig = ( j + ( 1 + 1 ) ) & ( ~1 );  /* big_values = nbig/2  */

/* keep region0_sfb sf bands full big */
    if ( nbig < startBand_l[region0_sfb] )
        nbig = startBand_l[region0_sfb];

/* find count 1 values  */
    j = startBand_l[cbreg[3]];
    n = nBand_l[cbreg[3] - 1];
    for ( i = 0; i < n; i++ )
    {
        j--;
        if ( ix[j] > 0 )
            break;
    }

    nquads = ( j + ( 1 + 3 ) - nbig ) >> 2;     /* num of quads in quad region */
// can be computed negative for subdivide2BT13
// since nBand at region0_sfb > 4
    nquads = HX_MAX ( nquads, 0 );

/* find max in each region */
    rmax[0] = 0;
    for ( i = 0; i < cbreg[0]; i++ )
    {
        if ( rmax[0] < ixmax[i] )
            rmax[0] = ixmax[i];
    }
    rmax[1] = 0;        // set zero and skip since cbreg[1] = cbreg[0]
//for(   ;i<cbreg[1];i++) {if( rmax[1] < ixmax[i] ) rmax[1] = ixmax[i];}
    rmax[2] = 0;
    for ( ; i < cbreg[2]; i++ )
    {
        if ( rmax[2] < ixmax[i] )
            rmax[2] = ixmax[i];
    }

/* choose tables by region */
    pick_tables (  );

/* count the bits */
    n0 = startBand_l[cbreg[0]];
    n = startBand_l[cbreg[1]];  // cbreg[1]=cbreg[0]

    bi = SubTable[icase[0]] ( TablePtr[0], ix, n0 );
    bits = bi.bits;
    ntable_out[0] = ntable[0][bi.index];

//bi = SubTable[icase[1]](TablePtr[1], ix+n0, n-n0);
//bits += bi.bits;
//ntable_out[1] = ntable[1][bi.index];

    bi = SubTable[icase[2]] ( TablePtr[2], ix + n, nbig - n );
    bits += bi.bits;
    ntable_out[2] = ntable[2][bi.index];
// set to same as big region
// only two tables transmitted for window_switching_flag =1, 
// (only two regions region0 and big)
    ntable_out[1] = ntable_out[2];

    bi = CountBitsQuad ( ix + nbig, nquads );
    bits += bi.bits;
    ntable_out[3] = bi.index;

/* save essential info for output */
    save[ch].ntable_out[0] = ntable_out[0];
    save[ch].ntable_out[1] = ntable_out[1];
    save[ch].ntable_out[2] = ntable_out[2];
    save[ch].ntable_out[3] = ntable_out[3];
    save[ch].cbreg[0] = cbreg[0];
    save[ch].cbreg[1] = cbreg[1];
    save[ch].cbreg[2] = cbreg[2];
    save[ch].nbig = nbig;
    save[ch].nquads = nquads;
    save[ch].bits = bits;

    return bits;
}

/*---------------------------------------------------------------*/
void
CBitAllo::output_subdivide2 ( GR * gr_data, int ch )
{
    int n0, n1, n2;

    if ( save[ch].bits <= 0 )
    {   // if no bits set null data
        gr_data->table_select[0] = 0;
        gr_data->table_select[1] = 0;
        gr_data->table_select[2] = 0;
        gr_data->big_values = 0;
        gr_data->region0_count = 0;
        gr_data->region1_count = 0;
        gr_data->aux_nreg[0] = 0;
        gr_data->aux_nreg[1] = 0;
        gr_data->aux_nreg[2] = 0;
        gr_data->aux_nquads = 0;
        gr_data->count1table_select = 0;
        return;
    }

    gr_data->table_select[0] = save[ch].ntable_out[0];
    gr_data->table_select[1] = save[ch].ntable_out[1];
    gr_data->table_select[2] = save[ch].ntable_out[2];
    gr_data->count1table_select = save[ch].ntable_out[3];

    gr_data->big_values = save[ch].nbig >> 1;
    gr_data->region0_count = save[ch].cbreg[0] - 1;
    gr_data->region1_count = ( save[ch].cbreg[1] - save[ch].cbreg[0] ) - 1;

// for BT13 test
    gr_data->region1_count = HX_MAX ( gr_data->region1_count, 0 );

/* number pairs in huff region */
    n0 = startBand_l[save[ch].cbreg[0]];
    n1 = startBand_l[save[ch].cbreg[1]];
    n2 = startBand_l[save[ch].cbreg[2]];
    if ( n2 > save[ch].nbig )
        n2 = save[ch].nbig;
    if ( n1 > n2 )
        n1 = n2;
    if ( n0 > n1 )
        n0 = n1;
    n2 = n2 - n1;
    n1 = n1 - n0;

    gr_data->aux_nreg[0] = n0 >> 1;
    gr_data->aux_nreg[1] = n1 >> 1;
    gr_data->aux_nreg[2] = n2 >> 1;
    gr_data->aux_nquads = save[ch].nquads;

    assert ( gr_data->region0_count >= 0 );
    assert ( gr_data->region1_count >= 0 );     // may fail for BT13

}

/*---------------------------------------------------------------*/
