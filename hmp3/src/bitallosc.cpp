/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallosc.cpp,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
#include "bitallos.h"
#include "hxtypes.h"
#include "cnttab.h"     //DUPLICATED, SHARE WITH LONG BLOCKS

extern "C"
{
    extern int iframe;  // for testing
}

static const unsigned char count_quada[16] = {
    1, 4 + 1, 4 + 1, 5 + 2, 4 + 1, 6 + 2, 5 + 2, 6 + 3,
    4 + 1, 5 + 2, 5 + 2, 6 + 3, 5 + 2, 6 + 3, 6 + 3, 6 + 4,
};

static int cbreg[5];    /* regions by cb's */
static int rmax[3];     /* ixmax in regions  */
static int tmax[3];     /* table max in regions  */
static int ntable[4][4];        /* table number by [regions][table]  */
static int icase[4];
static void *TablePtr[4];

static int ntable_out[4];       /* table number in regions + quad */
static int nbig;
static int nquads;

//static struct {
//    int ntable_out[4];
//    int cbreg[3];
//    int nbig;
//    int nquads;
//    int bits;
//}   save[2];

typedef struct
{
    int bits;
    int index;
}
BI;

extern "C"
{
    BI CountBits0Short ( void *table, int ix[][192], int n );
    BI CountBits1Short ( void *table, int ix[][192], int n );
    BI CountBits2Short ( void *table, int ix[][192], int n );
    BI CountBits3Short ( void *table, int ix[][192], int n );
    BI CountBits4Short ( void *table, int ix[][192], int n );
    BI CountBits5Short ( void *table, int ix[][192], int n );
    BI CountBitsQuad ( int ix[], int nquads );  // same as long
}

typedef BI ( *COUNTSUB ) ( void *table, int ix[][192], int n );

static const COUNTSUB SubTable[] = {
    CountBits0Short,
    CountBits1Short,
    CountBits2Short,
    CountBits3Short,
    CountBits4Short,
    CountBits5Short,
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
static void
pick_tables (  )
{
    int i, ixmax;

/* select tables */

    for ( i = 0; i < 2; i++ )
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

/*---------------------------------------------------------------*/
int
CBitAlloShort::subdivide2 ( int ixmax[][16], int ix[][192], int ncb, int ch )
{
    int i;
    int bits, n0;
    int k, j, w, n1, n2;
    BI bi;

// for short blocks
//   region 0 count is always 3 short sfb's = 12 short samples = 36 tot samples
//            mpeg 2.5 always 3 short sfb's but 8kHz is 24/72 samples
//            so always determine number samples by using bands tables
//   region 1 includes all bands

    cbreg[0] = 3;

    for ( i = ncb - 1; i >= 0; i-- )
    {
        if ( ixmax[0][i] > 0 )
            break;
        if ( ixmax[1][i] > 0 )
            break;
        if ( ixmax[2][i] > 0 )
            break;
    }
    cbreg[2] = i + 1;   // end of count1  region

    for ( ; i >= 0; i-- )
    {
        if ( ixmax[0][i] > 1 )
            break;
        if ( ixmax[1][i] > 1 )
            break;
        if ( ixmax[2][i] > 1 )
            break;
    }
    cbreg[1] = i + 1;   // end of cb big region

/* keep first 3 sf bands big regions */
    cbreg[1] = HX_MAX ( cbreg[1], 3 );
    cbreg[2] = HX_MAX ( cbreg[2], cbreg[1] );

/* find bigValues */
// bigValues - because of huff transmission order for short blocks
// make bigValues integral scale factor band
    nbig = startBand_s[cbreg[1]];       //  big_values = 3*(nbig/2) 

/* find count 1 values  */
// because of huff transmission order for short blocks
// make quads integral scale factor band and round up to quad
// uqads are cbreg[1]=big to cbreg[2]
// bands may not be divisable by 4
// nquads is total number of quads for all windows and sf bands
// computed below as part of reorder

// find max in each region
// region could be null if cbreg[1]=cbreg[0]
    rmax[0] = 0;
    for ( i = 0; i < cbreg[0]; i++ )
    {
        if ( rmax[0] < ixmax[0][i] )
            rmax[0] = ixmax[0][i];
        if ( rmax[0] < ixmax[1][i] )
            rmax[0] = ixmax[1][i];
        if ( rmax[0] < ixmax[2][i] )
            rmax[0] = ixmax[2][i];
    }
    rmax[1] = 0;
    for ( ; i < cbreg[1]; i++ )
    {
        if ( rmax[1] < ixmax[0][i] )
            rmax[1] = ixmax[0][i];
        if ( rmax[1] < ixmax[1][i] )
            rmax[1] = ixmax[1][i];
        if ( rmax[1] < ixmax[2][i] )
            rmax[1] = ixmax[2][i];
    }

/* choose tables by region */
    pick_tables (  );

/* count the bits */
    n0 = startBand_s[cbreg[0]];

    bi = SubTable[icase[0]] ( TablePtr[0], ix, n0 );
    bits = bi.bits;
    ntable_out[0] = ntable[0][bi.index];

    bi = SubTable[icase[1]] ( TablePtr[1], ( int ( * )[192] ) ( &ix[0][n0] ),
                              nbig - n0 );
    bits += bi.bits;
    ntable_out[1] = ntable[1][bi.index];

    ntable_out[2] = 0;

// reorder quads for bit count
// each sf band is padded to integral number of quads
    k = 0;
    for ( i = cbreg[1]; i < cbreg[2]; i++ )
    {
        n1 = startBand_s[i];
        n2 = startBand_s[i + 1];
        for ( w = 0; w < 3; w++ )
        {
            for ( j = n1; j < n2; j++ )
            {
                ix_reorder[k] = ix[w][j];
                k++;
            }
        }
    }
// could save a few bits by stopping when rest all zero
    ix_reorder[k] = ix_reorder[k + 1] = ix_reorder[k + 2] = 0;
    k = ( k + 3 ) & ( ~3 );     // round up to integral number of quads
    nquads = k >> 2;
    bi = CountBitsQuad ( ix_reorder, nquads );
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
CBitAlloShort::output_subdivide2 ( GR * gr_data, int ch )
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

// mult by 3 windows
    gr_data->big_values = 3 * ( save[ch].nbig >> 1 );
// regions are not coded for short blocks (or bt = 1 or 3 either)
//gr_data->region0_count  = save[ch].cbreg[0] - 1;
//gr_data->region1_count  = (save[ch].cbreg[1] - save[ch].cbreg[0]) - 1;
    gr_data->region0_count = 0;
    gr_data->region1_count = 0;

/* number pairs in huff region */
    n0 = startBand_s[save[ch].cbreg[0]];
    n1 = startBand_s[save[ch].cbreg[1]];
    n2 = startBand_s[save[ch].cbreg[2]];
    if ( n2 > save[ch].nbig )
        n2 = save[ch].nbig;
    if ( n1 > n2 )
        n1 = n2;
    if ( n0 > n1 )
        n0 = n1;
    n2 = n2 - n1;
    n1 = n1 - n0;

    gr_data->aux_nreg[0] = 3 * ( n0 >> 1 );
    gr_data->aux_nreg[1] = 3 * ( n1 >> 1 );
    gr_data->aux_nreg[2] = 3 * ( n2 >> 1 );

    gr_data->aux_nquads = save[ch].nquads;      // save contains total all w and sfb
    assert ( gr_data->region0_count >= 0 );
    assert ( gr_data->region1_count >= 0 );

}

/*---------------------------------------------------------------*/
