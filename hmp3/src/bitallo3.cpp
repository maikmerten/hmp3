/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallo3.cpp,v 1.2 2005/08/09 20:43:41 karll Exp $ 
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

#include "bitallo3.h"

extern "C"
{
    extern int iframe; // for test
}

//extern "C" {
//    void start_clock();
//    void end_clock();
//}

static int cnt1, cnt2, cnt3, cnt4, cnt5, cnt6, cnt7, cnt8, cnt9;// test vars
static int cnt10, cnt11, cnt12, cnt13, cnt14, cnt15, cnt16;     // test vars

// mnrGold applies to old fft acoustic model
// Neal's "mnrGOLD.txt" selective mnr adjustment 10/25/99
// based on VBR-1  ver 4.0 (v17) X6 (base taperNT given below
// positive adjustments in centiBells.
//    for(i=11;i<22;i++) 
//        taperNT[i] = 100 + HX_MIN(150, 20*(i-11));  // base line as of 8/99
static const int mnrGOLD[22] = {
    -5, 0, 0, 0, 0, 0,
    0, 0, 0, 3, 5, 5, 5,
    5, 3, 0, 0,
    0,
    -1,
    -8,
    -10,
};

// L3math.c needs these (global) - computed by init, but fixed constants
// C naming convention, cannot be CPP or name mangled
extern "C"
{
    float look_gain[128];
    float look_34igain[128];
    float look_ix43[256];
}

static const float sparse_table[22] = {       // based on snr in bands
//0.0014f, 0.0026f, 0.0042f, 0.007f, 0.025f, 
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f,
//0.05f, 0.05f, 0.06f, 0.07f, 0.07f, 0.09f, 0.10f, 0.13f, 0.15f, 0.20f,
    0.05f, 0.05f, 0.06f, 0.07f, 0.07f, 0.09f, 0.15f, 0.15f, 0.15f, 0.20f,
    100.0f
};

static const int sf_limit[6][22] = {
    {   // scale = 0, preemp = 0
     31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
     15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     },
    {   // scale = 0, premp = 1
     31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
     15 + 2 * 1, 15 + 2 * 1, 15 + 2 * 1, 15 + 2 * 1, 15 + 2 * 2,
     15 + 2 * 2, 15 + 2 * 3, 15 + 2 * 3, 15 + 2 * 3, 15 + 2 * 2, 15,
     },

    {   // scale = 1 pre = 0
     62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62,        // use 62 & 30 or 63 & 31 ??
     30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
     },
    {   // scale = 1 pre = 1
     62, 62, 62, 62, 62, 62, 62, 62, 62, 62, 62,
     30 + 4 * 1, 30 + 4 * 1, 30 + 4 * 1, 30 + 4 * 1, 30 + 4 * 2,
     30 + 4 * 2, 30 + 4 * 3, 30 + 4 * 3, 30 + 4 * 3, 30 + 4 * 2, 30,
     },
    {   // scale = 0, premp = 1  lower limits
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     2 * 1, 2 * 1, 2 * 1, 2 * 1, 2 * 2,
     2 * 2, 2 * 3, 2 * 3, 2 * 3, 2 * 2, 0,
     },
    {   // scale = 1 pre = 1
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     4 * 1, 4 * 1, 4 * 1, 4 * 1, 4 * 2,
     4 * 2, 4 * 3, 4 * 3, 4 * 3, 4 * 2, 0,
     },
};

// ATTENTION: might be overwritten later on
static int sf_upper_limit[2][2][22] = { // limit[scale][preemp]
    {{  // scale = 0, preemp = 0
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
      14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
      },
     {  // scale = 0, premp = 1
      30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
      14 + 2 * 1, 14 + 2 * 1, 14 + 2 * 1, 14 + 2 * 1, 14 + 2 * 2,
      14 + 2 * 2, 14 + 2 * 3, 14 + 2 * 3, 14 + 2 * 3, 14 + 2 * 2, 14,
      }},
    {{  // scale = 1 pre = 0
      60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
      28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
      },
     {  // scale = 1 pre = 1
      60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60,
      28 + 4 * 1, 28 + 4 * 1, 28 + 4 * 1, 28 + 4 * 1, 28 + 4 * 2,
      28 + 4 * 2, 28 + 4 * 3, 28 + 4 * 3, 28 + 4 * 3, 28 + 4 * 2, 28}},
};

// ATTENTION: might be overwritten later on
//----------
static int sf_lower_limit[2][2][22] = {
    {{  // scale = 0, preemp = 0
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      },
     {  // scale = 0, premp = 1
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      2 * 1, 2 * 1, 2 * 1, 2 * 1, 2 * 2,
      2 * 2, 2 * 3, 2 * 3, 2 * 3, 2 * 2, 0,
      }},
    {{  // scale = 1 pre = 0
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      },
     {  // scale = 1 pre = 1
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      4 * 1, 4 * 1, 4 * 1, 4 * 1, 4 * 2,
      4 * 2, 4 * 3, 4 * 3, 4 * 3, 4 * 2, 0,
      }},
};

//---------

//static int pre2[21]={
//0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//15+2*1, 15+2*1, 15+2*1, 15+2*1, 15+2*2, 
//15+2*2, 15+2*3, 15+2*3, 15+2*3, 15+2*2
//};
//static int pre4[21]={
//0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//31+4*1, 31+4*1, 31+4*1, 31+4*1, 31+4*2, 
//31+4*2, 31+4*3, 31+4*3, 31+4*3, 31+4*2
//};

static const int pretable[21] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 2,
    2, 3, 3, 3, 2
};

static const int dNthres[22] = {
    250, 250, 250, 250, 250,
    250, 250, 250, 250, 250,
    250, 250, 250, 250, 250,
    300, 400, 400, 500, 500,
    600, 600,
};

/*---------------------------------------------------------------*/
// tables used by trade_main trade_dual
static float factor_table[16] = {
    1.0f / ( 0.5f + 0.09460f ),
    1.0f / ( 1.5f + 0.02799f ),
    1.0f / ( 2.5f + 0.01671f ),
    1.0f / ( 3.5f + 0.01192f ),
    1.0f / ( 4.5f + 0.00927f ),
    1.0f / ( 5.5f + 0.00758f ),
    1.0f / ( 6.5f + 0.00641f ),
    1.0f / ( 7.5f + 0.00556f ),
    1.0f / ( 8.5f + 0.00490f ),
    1.0f / ( 9.5f + 0.00439f ),
    1.0f / ( 10.5f + 0.00397f ),
    1.0f / ( 11.5f + 0.00362f ),
    1.0f / ( 12.5f + 0.00333f ),
    1.0f / ( 13.5f + 0.00309f ),
    1.0f / ( 14.5f + 0.00287f ),
    1.0f / ( 15.5f + 0.00269f ),
};
static int target_table[16] = {
    0,   // 0
    1,   // 1
    2,   // 2
    3,   // 3
    3,   // 4
    5,   // 5
    5,   // 6
    7,   // 7
    7,   // 8
    7,   // 9
    7,   // 10
    15,  // 11
    15,  // 12
    15,  // 13
    15,  // 14
    15,  // 15
};

///////////////////////////////////////////////////////////////////////////////
// Public Functions
///////////////////////////////////////////////////////////////////////////////
CBitAllo3::CBitAllo3 (  ):CBitAllo (  ),
hq_flag ( 1 ),
ill_is_pos ( 7 ), ms_correlation_memory ( 0 ), int_dummy ( 0 )
{
    nsf[0] = nsf[1] = 21;       // in base class
    nsf2[0] = nsf2[1] = 21;
    nsf3[0] = nsf3[1] = 21;     // stereo hf allo 

    memset ( xsxx, 0, sizeof ( xsxx ) );
    memset ( xsxxms, 0, sizeof ( xsxxms ) );
    memset ( Noise0, 0, sizeof ( Noise0 ) );
    memset ( NT, 0, sizeof ( NT ) );
    memset ( Noise, 0, sizeof ( Noise ) );
    memset ( NTadjust, 0, sizeof ( NTadjust ) );
    memset ( snr, 0, sizeof ( snr ) );
    memset ( x34max, 0, sizeof ( x34max ) );
    memset ( ixmax, 0, sizeof ( ixmax ) );
    memset ( ix10xmax, 0, sizeof ( ix10xmax ) );
    memset ( gzero, 0, sizeof ( gzero ) );
    memset ( gmin, 0, sizeof ( gmin ) );
    memset ( gsf, 0, sizeof ( gsf ) );
//memset(gsfq               , 0, sizeof(gsfq             ) );   
    memset ( sf, 0, sizeof ( sf ) );
    memset ( G, 0, sizeof ( G ) );
    memset ( active_sf, 0, sizeof ( active_sf ) );
    memset ( x34, 0, sizeof ( x34 ) );
    memset ( ix10x, 0, sizeof ( ix10x ) );
    memset ( preemp, 0, sizeof ( preemp ) );
    memset ( scalefactor_scale, 0, sizeof ( scalefactor_scale ) );

}

CBitAllo3::~CBitAllo3 (  )
{
}

/*===============================================================*/
void
CBitAllo3::ba_out_stats (  )    // test routine
{

    cnt1 = call_count;
    cnt2 = ms_count;

    cnt13 = ( 100 * cnt2 + ( cnt1 >> 1 ) ) / cnt1;

    fprintf(stderr, "\n ba long  %6d %6d %6d %6d %6d %6d %6d %6d %6d",
             cnt1, cnt2, cnt3, cnt4, cnt5, cnt6, cnt7, cnt8, cnt9 );
    fprintf(stderr, "\n ba long  cnt10  %6d %6d %6d %6d %6d %6d %6d",
             cnt10, cnt11, cnt12, cnt13, cnt14, cnt15, cnt16 );

    BitAlloShort.ba_out_stats (  );
}

/*===============================================================*/
int
CBitAllo3::BitAlloInit ( BA_CONTROL & bac )
{
    int i, k;

    cnt7 = 9999;

    ba_control = bac;   // copy to local, access to test1/2/3

// init short block allocation
    BitAlloShort.BitAlloInit ( bac );

    call_count = 0;
    ms_count = 0;
    huff_bits[0] = huff_bits[1] = 0;

    MNR = initialMNR = bac.initialMNR;  // MNR initial MNR in mb, 1db = 100mb
    vbr_flag = bac.vbr_flag;
    h_id = bac.h_id;    // mpeg1=1
    is_flag = bac.is_flag;      // intensity stereo
    hf_flag = bac.hf_flag;
//hf_flag = 0;  // no hf, bad for quality, also needs testing with this BitAllo

    PoolFraction = 614; // fraction*1024 
    if ( vbr_flag == 0 )
        PoolFraction = 0;       // fraction*1024 

    if ( h_id )
    {   // mpeg1
        ill_is_pos = 7;
    }
    else
    {   // mpeg2
        ill_is_pos = 999;
    }

    L3init_gen_band_table_long ( nBand_l );
    L3init_gen_band_table_short ( nBand_s );
    nsf3[0] = nsf2[0] = nsf[0] = L3init_sfbl_limit2 ( bac.band_limit_left );
    nsf3[1] = nsf2[1] = nsf[1] = L3init_sfbl_limit2 ( bac.band_limit_right );
    if ( hf_flag )
    {
        nsf2[0] = 22;
        nBand_l[21] = 100;
    }

    if ( hf_flag & 2 )
    {
        nsf3[0] = 22;
        nsf3[1] = 22;
    }

    for ( k = 0, i = 0; i < 22; i++ )
    {
        startBand_l[i] = k;
        k += nBand_l[i];
    }
    startBand_l[i] = k;
    startBand_l[i + 1] = 576;   // [22] and [23] are 576

    for ( k = 0, i = 0; i < 13; i++ )
    {   // only needed for short
        startBand_s[i] = k;
        k += nBand_s[i];
    }
    startBand_s[i] = k;

    nbmax3[0] = nbmax2[0] = nbmax[0] = startBand_l[nsf[0]];
    nbmax3[1] = nbmax2[1] = nbmax[1] = startBand_l[nsf[1]];
    if ( hf_flag )
    {
        nbmax2[0] = startBand_l[nsf2[0]];
    }
    if ( hf_flag & 2 )
    {
        nbmax3[0] = startBand_l[nsf3[0]];
        nbmax3[1] = startBand_l[nsf3[1]];
    }

    for ( i = 0; i < 128; i++ )
    {
        look_gain[i] = ( float ) ( pow ( 2.0, 0.25 * ( i - g_offset ) ) );      /* note offset, 8=4x */
        look_34igain[i] =
            ( float ) ( 1.0 / pow ( (double) look_gain[i], (double)( 3.0 / 4.0 ) ) );
    }

/* inverse quant */
    for ( i = 0; i < 256; i++ )
        look_ix43[i] = ( float ) ( i * pow ( (double) i, (double)( 1.0 / 3.0 ) ) );

    for ( i = 0; i < 21; i++ )
    {
        look_log_cbwmb[i] =
            ( int ) ( 100.0f * dbLog ( ( double ) nBand_l[i] ) );
    }

/*--------
gzero = 1 + 16/(3*ln(2))*(ln(x34max)-ln(.5946)) + g_offset;
      = con1*ln(x34max) + con2
-------------*/
//gz_con1 = (float)(16.0/(3.0*log(2.0)));
//gz_con2 = (float)(1 - (16.0/(3.0*log(2.0)))*log(.5946) + g_offset);
//gz_con0 = (float)(exp( (0.99-gz_con2)/gz_con1));

// gzero = con1*dbLog(x14max) + con2
//gz_con1 = (float)(16.0/dbLog(2.0));
//gz_con2 = (float)(1.000001 - (16.0/(3.0*dbLog(2.0f)))*dbLog(.5946f) + g_offset);
//gz_con0 = (float)(exp( ((0.99-gz_con2)/gz_con1) * (log(10.0)/10.0) ) );

/*-------------
con2r for mbLogC + round_to_int(), includes round up + mbLogC eps
con0 not used with mbLogC implementation
uses x34 with goffset = 100
   0 con1 con2   0.017716950 104.000000  con2r  104.585000
   1 con1 con2   0.017716950  96.737957  con2r   97.322957
   2 con1 con2   0.017716950  92.898457  con2r   93.483457
   3 con1 con2   0.017716950  90.334612  con2r   90.919612
   4 con1 con2   0.017716950  88.411238  con2r   88.996238
   5 con1 con2   0.017716950  86.872435  con2r   87.457435
   6 con1 con2   0.017716950  85.590068  con2r   86.175068
   7 con1 con2   0.017716950  84.490884  con2r   85.075884
gz_con1 = 0.017716950f;
gz_con2 = 104.585000f;
---------------*/
    gz_con1B = 0.017716950f;    // gz_con1B from AAC 
    gz_con2B = ( 104.585000f - 100.0f + 8.0f );

    for ( i = 0; i < 22; i++ )
        taperNT[i] = 0;

    if ( bac.disable_taper )
    {
        if ( h_id == 0 )
        {       // mpeg2
            for ( i = 0; i < 22; i++ )
                taperNT[i] = 0;
        }
    }
    else
    {   // for old/slow/fft acoustic model
        for ( i = 11; i < 22; i++ )
        {
            taperNT[i] = 100 + HX_MIN ( 150, 20 * ( i - 11 ) );    // base line as of 8/99
        }
        if ( vbr_flag )
        {
            for ( i = 11; i < 22; i++ )
            {
                taperNT[i] = HX_MIN ( taperNT[i], initialMNR );
            }
        }
        // add in mnr adjustment - arg is in centiBells, + means increase mnr
        for ( i = 0; i < 21; i++ )
        {
            //taperNT[i] -= 10*bac.mnr_adjust[i];
            taperNT[i] -= 10 * mnrGOLD[i];
        }
    }

    if ( h_id == 1 )
    {   // mpeg-1
        initialMNR = MNR = MNR + bac.MNRbias;
    }
    else
    {   //  mpeg-2 need lower mnr for vbr values relative to nominal bitrates 
        initialMNR = MNR = MNR + bac.MNRbias - 300;
    }

    for ( i = 0; i < 22; i++ )
    {
        active_sf[0][i] = active_sf[1][i] = 0;
        sf[0][i] = sf[1][i] = 0;
        ixmax[0][i] = ixmax[1][i] = 0;

    }

    for ( i = 0; i < 22; i++ )
    {
        if ( nBand_l[i] != 0 )
            rnBand_l[i] = ( 1.0f / nBand_l[i] );
    }

    hf_quant = 0;
    hf_quant_stereo[0] = hf_quant_stereo[1] = 0;
    gsf_hf = -1;
    gsf_hf_stereo[0] = gsf_hf_stereo[1] = -1;

    for ( i = 0; i < 22; i++ )
    {
        NTadjust[0][i] = NTadjust[1][i] = 0;
    }

    return nsf[1];      // for use by pack sf mpeg2
}

//==================================================================
void
CBitAllo3::BitAllo ( float xr_arg[][576], SIG_MASK sm_arg[][36],
                     int ch_arg, int nchan_arg,
                     int min_bits_arg, int target_bits_arg, int max_bits_arg,
                     int bit_pool_arg,
                     SCALEFACT sf_out[], GR gr_data[],
                     int ix_arg[][576],
                     unsigned char signx_arg[][576], int ms_flag_arg )

/* nchan_agr = chans to allo, can be 1 for stereo  */
{
    int i, j;
    int FeedbackBits;
    int MNR0;
    int tbits, t;

// at this time block_type must be same in both channels
    block_type = gr_data[0].block_type;

    call_count++;       // this used by mnr_feedback

    deltaMNR = 0;

    if ( block_type == 1 )
    {
        if ( MNR > initialMNR )
        {
            MNR = ( MNR + initialMNR ) >> 1;
            MNR = HX_MIN ( MNR, initialMNR + 500 );
        }
        cnt10++;
        //MNR = HX_MIN(MNR, initialMNR);
    }
    else if ( block_type == 3 )
    {
        MNR = ( MNR + initialMNR ) >> 1;
        MNR = HX_MIN ( MNR, initialMNR + 500 );
        // clear any garbage left by short block
        // really only need to clear a few values past last sfb
        // (quad could extend beyond sfb)
        memset ( ix_arg, 0, nchan_arg * 576 * sizeof ( int ) );
    }

    if ( block_type == 2 )
    {
        MNR0 = MNR;
        if ( vbr_flag == 0 )
        {
            MNR0 =
                MNR - ( HX_MAX ( MNR - initialMNR, 0 ) >> 1 ) -
                ( HX_MAX ( MNR - initialMNR - 400, 0 ) >> 2 );
            MNR0 = HX_MAX ( initialMNR + 400, MNR0 );
            //MNR0 = 400 + HX_MAX(initialMNR, MNR0);
        }
        else
        {
            MNR0 = initialMNR + 400;
        }
        FeedbackBits =
            BitAlloShort.BitAllo ( ( float ( * )[3][192] ) xr_arg,
                                   ( SIG_MASK ( * )[3][12] ) sm_arg, ch_arg,
                                   nchan_arg, min_bits_arg, target_bits_arg,
                                   max_bits_arg, bit_pool_arg, sf_out,
                                   gr_data, ix_arg, signx_arg, ms_flag_arg,
                                   MNR0 );
        if ( vbr_flag == 0 )
        {
            mnr_feedback ( nchan * startBand_l[nsf[0]], FeedbackBits,
                           block_type );
        }
        return;
    }

    if ( ms_flag_arg )
        ms_count++;

//-----------------------
    ms_flag = ms_flag_arg;
    xr = xr_arg;
    signx = signx_arg;
    ix = ix_arg;
    nchan = nchan_arg;

    maxBits = HX_MIN ( 4000 * nchan, max_bits_arg );       /* must not exceed */
    /* 4000*nchan is for part23 */
    minTargetBits = min_bits_arg;
    if ( minTargetBits < 0 )
        minTargetBits = 0;
    TargetBits = target_bits_arg;
    PoolBits = bit_pool_arg;

// if vbr, callers set TargetBits to max steady state bitrate
    if ( vbr_flag == 0 )
    {
        PoolFraction = HX_MIN ( PoolFraction + 50, 614 );
        if ( block_type != 0 )
            PoolFraction = 0;
    }
    tbits = ( ( PoolFraction * PoolBits ) >> 10 );
    if ( vbr_flag == 0 )
    {   // don't allow very high mnrs to use bit pool
        t = HX_MAX ( ( 2050 - 500 ) + initialMNR - MNR, 200 );
        tbits = HX_MIN ( tbits, t );
    }
    maxTargetBits = TargetBits + tbits;
    maxTargetBits = HX_MIN ( maxBits, maxTargetBits );
    if ( MNR < -200 )
    {   //
        minTargetBits = HX_MAX ( minTargetBits, ( 3 * TargetBits ) >> 2 );
    }
    maxTargetBits = HX_MAX ( minTargetBits, maxTargetBits );
    minTargetBits = HX_MIN ( minTargetBits, maxTargetBits - 100 );

    cnt7 = HX_MIN ( MNR, cnt7 );

    if ( ms_flag )
    {
        startup_ms2 ( sm_arg, signx );
    }
    else
    {
        startup ( sm_arg, signx );
    }

//-----------------------

/* null frame */
    if ( activeBands <= 0 )
    {
        for ( i = 0; i < nchan; i++ )
        {
            gr_data[i].global_gain = 0;
            gr_data[i].window_switching_flag = ( block_type != 0 );
            gr_data[i].block_type = block_type;
            gr_data[i].mixed_block_flag = 0;
            gr_data[i].preflag = 0;
            gr_data[i].scalefac_scale = 0;
            gr_data[i].table_select[0] = 0;
            gr_data[i].table_select[1] = 0;
            gr_data[i].table_select[2] = 0;
            gr_data[i].big_values = 0;
            gr_data[i].region0_count = 0;
            gr_data[i].region1_count = 0;
            gr_data[i].count1table_select = 0;  // changed to 0 10/27/98
            gr_data[i].aux_nquads = 0;
            gr_data[i].aux_bits = 0;    /* zero huff bits */
            gr_data[i].aux_not_null = 0;
            gr_data[i].aux_nreg[0] = 0;
            gr_data[i].aux_nreg[1] = 0;
            gr_data[i].aux_nreg[2] = 0;
            for ( j = 0; j < 21; j++ )
                sf_out[i].l[j] = 0;
        }
        return;
    }
//------------------------

    if ( ms_flag )
        FeedbackBits = allocate_ms (  );
    else
        FeedbackBits = allocate (  );

// mnr feedback for CBR
    if ( vbr_flag == 0 )
        mnr_feedback ( activeBands, FeedbackBits, block_type );

/*------- return data --------*/
    output_sf ( sf_out );

    if ( ms_flag )
    {   // compensate for lack of sqrt(2) scaling
        G[0] -= 2;
        G[1] -= 2;
    }

    for ( i = 0; i < nchan; i++ )
    {
        gr_data[i].global_gain = G[i] + ( 4 * 32 + 14 );
        if ( gr_data[i].global_gain > 255 )
            gr_data[i].global_gain = 255;
        gr_data[i].window_switching_flag = ( block_type != 0 );
        gr_data[i].block_type = block_type;
        gr_data[i].mixed_block_flag = 0;
        gr_data[i].preflag = preemp[i];
        gr_data[i].scalefac_scale = scalefactor_scale[i];
        gr_data[i].aux_bits = huff_bits[i];     /* huff bits */
        gr_data[i].aux_not_null = huff_bits[i]; /* huff bits */
        output_subdivide2 ( &gr_data[i], i );
    }

    if ( is_flag )
    {   // must encode right sf even if huff_bit = 0
        gr_data[1].aux_not_null = 1;
    }

}

/*===============================================================*/
int
CBitAllo3::ms_correlation2 ( float x[2][576], int block_type )
{
    int i, j, k, n;
    float a, b;
    float t, c, el, er, es, ed;
    int mblr, mbsd;

//int vlr[21], vsd[21], v[21];
    int cm, cm0, cm1;
    int psd, totpsd;

    if ( block_type == 2 )
    {
        ms_correlation_memory = 0;
        return BitAlloShort.
            ms_correlation2Short ( ( float ( * )[3][192] ) x );
    }

    totpsd = 0;
    cm = cm0 = cm1 = 0;
    k = 0;
    for ( i = 0; i < nsf[0]; i++ )
    {
        n = nBand_l[i];
        el = er = 100.0f;
        t = 0.0f;
        for ( j = 0; j < n; j++, k++ )
        {
            a = x[0][k] * x[0][k];
            b = x[1][k] * x[1][k];
            c = x[0][k] * x[1][k];
            el += a;
            er += b;
            t += c;
        }
        es = ed = el + er;
        t = t + t;
        es = es + t;
        ed = ed - t;

        mblr = mbLogC ( el + er ) - mbLogC ( pos_fmax ( el, er ) );
        //vlr[i] = mblr;

        mbsd = mbLogC ( es + ed ) - mbLogC ( pos_fmax ( es, ed ) );
        //vsd[i] = mbsd;

        //v[i] = mblr - mbsd;

        cm0 += mblr;
        cm1 += mbsd;

        psd = HX_MAX ( 75 - abs ( mblr - 120 ), 0 );
        totpsd += nBand_l[i] * psd;

        //mbsd = HX_MIN(mbsd, ((3*mbsd)>>2) + 50);
        //
        mbsd = HX_MIN ( mbsd, ( mbsd >> 1 ) + 120 );
        mbsd += psd;

        cm += nBand_l[i] * ( mblr - mbsd );

    }

    cm += ms_correlation_memory;

    ms_correlation_memory = -5000;
    if ( cm > 0 )
    {
        ms_correlation_memory = 5000;
    }

    return cm;
}

/*===============================================================*/
///////////////////////////////////////////////////////////////////////////////
// Private Functions
///////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------*/
void
CBitAllo3::output_sf ( SCALEFACT sf_out[] )
{
    int i, ch;

/* convert to encoded sf */

    for ( ch = 0; ch < nchan; ch++ )
    {
        /* adjust scaling */
        if ( scalefactor_scale[ch] == 0 )
        {
            for ( i = 0; i < nsf[ch]; i++ )
                sf[ch][i] >>= 1;
        }
        else
        {
            for ( i = 0; i < nsf[ch]; i++ )
                sf[ch][i] >>= 2;
        }
        /* adjust for preemp */
        if ( preemp[ch] )
        {
            for ( i = 11; i < nsf[ch]; i++ )
            {
                sf[ch][i] -= pretable[i];
                assert ( sf[ch][i] >= 0 );
            }
        }

    }

// for joint set is_pos in null sf bands
    if ( is_flag )
    {
        for ( i = nsf[1] - 1; i >= 0; i-- )
        {
            if ( ixmax[1][i] > 0 )
                break;
            sf[1][i] = ill_is_pos;      // set illegal is_pos
        }
    }

/* return scale factors, (sf[i] = 0 beyond nsf[ch] or joint is pos) */
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < 21; i++ )
            sf_out[ch].l[i] = sf[ch][i];
    }

}

/*===============================================================*/
void
CBitAllo3::startup ( SIG_MASK sm[][36], unsigned char signx[][576] )
{
    int i, n, ch;
    float *x, *y;
    unsigned char *s;
    int mask;
    int mnr;
    int tsnr;

//mnr = MNR;
    mnr = MNR + 100;

// compute sign and xsxx
    for ( ch = 0; ch < nchan; ch++ )
    {
        x = xr[ch];
        s = signx[ch];
        for ( i = 0; i < nsf3[ch]; i++ )
        {
            n = nBand_l[i];
            xsxx[ch][i] = vect_sign_sxx ( x, s, n );
            x += n;
            s += n;
        }
    }
//-------------------------------------------
// compute Mask in db and NT (noise target)
    activeBands = 0;
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            Noise0[ch][i] = mbLogB ( xsxx[ch][i] ) - look_log_cbwmb[i];
            if ( Noise0[ch][i] < -2000 )
            {
                NT[ch][i] = Noise0[ch][i] + 1000;
            }
            else
            {
                activeBands += nBand_l[i];
                mask = mbLogB ( sm[ch][i].mask ) - look_log_cbwmb[i];
                NT[ch][i] = mask - mnr + taperNT[i];    // noise target
                // dropout prevention
                tsnr = Noise0[ch][i] - NT[ch][i];
                if ( tsnr < 300 )
                {       // [-500,300] -> [0,300]
                    tsnr = 187 + ( ( 3 * tsnr ) >> 3 ) - tsnr;
                    NT[ch][i] -= tsnr;
                }
            }
            snr[ch][i] = Noise0[ch][i] - NT[ch][i];
        }
    }

//-------------------------------------------
    startup_adjustNT1B (  );
//-------------------------------------------

//-------------------------------------------
//
//  gzero:  gsf >= gzero -> ixmax = 0 (i.e. will quant to zero)
//          but note converse does not apply 
//  gmin:   gsf < gmin may exceed max allowed quantized value
// 

    for ( ch = 0; ch < nchan; ch++ )
    {
        vect_fpow34 ( xr[ch], x34[ch], nbmax3[ch] );
        y = x34[ch];
        for ( i = 0; i < nsf3[ch]; i++ )
        {
            n = nBand_l[i];
            vect_fmax2 ( y, n, x34max[ch] + i );
            gzero[ch][i] =
                HX_MAX ( 0,
                      round_to_int ( ( gz_con1B * mbLogC ( x34max[ch][i] ) +
                                       gz_con2B ) ) );
            gmin[ch][i] = HX_MAX ( 0, gzero[ch][i] - GMIN_OFFSET );
            y += n;
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::startup_ms2 ( SIG_MASK sm[][36], unsigned char signx[][576] )
{
    int i, n, ch;
    int mnr;
    float *x, *y;
    unsigned char *s;
    float sxx[4];       // left, right, sum, diff
    int maskL, maskR;
    int cbwmb;
    int xNT, Nsum, Ndiff;
    int Noise0L, Noise0R;
    int NTR, NTL;
    int tsnr;

//int yNT;
//int d;

// this probably should change for different bitrates
// slightly better stats with fewer calls to increase_bits
    if ( vbr_flag == 0 )
    {
        if ( call_count > 10 )
        {
            if ( ( TargetBits - minTargetBits ) < 100 )
            {
                MNR = HX_MIN ( MNR + 50, 2050 );
            }
        }
    }

    mnr = MNR;

    activeBands = 0;
    x = xr[0];
    s = signx[0];

    for ( i = 0; i < nsf[0]; i++ )
    {
        n = nBand_l[i];
        fnc_sxx ( x, n, sxx );
        fnc_ms_process2 ( x, n, s );    //no sqrt(2) scale
        fnc_sxx ( x, n, sxx + 2 );

        xsxx[0][i] = sxx[0];    // using this
        xsxx[1][i] = sxx[1];    // this also
        xsxxms[0][i] = sxx[2];  // using this
        xsxxms[1][i] = sxx[3];  // this also

        cbwmb = look_log_cbwmb[i];
        Noise0L = mbLogB ( sxx[0] ) - cbwmb;
        if ( Noise0L < -2000 )
            NTL = 10000;
        else
        {
            maskL = ( mbLogB ( sm[0][i].mask ) - cbwmb );
            NTL = maskL - mnr + taperNT[i];     // noise target left

            // dropout prevention
            tsnr = Noise0L - NTL;
            if ( tsnr < 300 )
            {   // [-500,300] -> [0,300]
                tsnr = 187 + ( ( 3 * tsnr ) >> 3 ) - tsnr;
                NTL -= tsnr;
            }

            activeBands += n;
        }
        Noise0R = mbLogB ( sxx[1] ) - cbwmb;
        if ( Noise0R < -2000 )
            NTR = 10000;
        else
        {
            maskR = ( mbLogB ( sm[1][i].mask ) - cbwmb );
            NTR = maskR - mnr + taperNT[i];     // noise target right

            // dropout prevention
            tsnr = Noise0R - NTR;
            if ( tsnr < 300 )
            {   // [-500,300] -> [0,300]
                tsnr = 187 + ( ( 3 * tsnr ) >> 3 ) - tsnr;
                NTR -= tsnr;
            }

            activeBands += n;
        }

        NT[0][i] = NTL;
        NT[1][i] = NTR;
        snr[0][i] = Noise0L - NTL;
        snr[1][i] = Noise0R - NTR;

        Noise0[0][i] = mbLogB ( sxx[2] ) - cbwmb;
        Noise0[1][i] = mbLogB ( sxx[3] ) - cbwmb;

        x += n;
        s += n;
    }

    if ( hf_flag )
    {
        n = nBand_l[21];
        fnc_ms_process2 ( x, n, s );
    }

//-------------------------------------------
    startup_adjustNT1B (  );
//-------------------------------------------

    for ( i = 0; i < nsf[0]; i++ )
    {

        NTL = NT[0][i];
        NTR = NT[1][i];
        Nsum = Noise0[0][i];
        Ndiff = Noise0[1][i];

        xNT = HX_MIN ( NTL, NTR ) + 300;   // scale by 300 for ms sqrt

        NT[1][i] = NT[0][i] = xNT;
        if ( Ndiff < xNT )
        {
            NT[0][i] = LogSubber ( xNT, Ndiff );
            if ( i < 16 )
                NT[0][i] -= 200;        // x7 baseline 10/25/99
        }
        if ( Nsum < xNT )
        {
            NT[1][i] = LogSubber ( xNT, Nsum );
        }

        snr[0][i] = Nsum - NT[0][i];
        snr[1][i] = Ndiff - NT[1][i];

        x += n;
        s += n;
    }

//-----------------------------------------

    vect_fpow34 ( xr[0], x34[0], nbmax2[0] );
    vect_fpow34 ( xr[1], x34[1], nbmax2[1] );
//
//  gzero:  gsf >= gzero -> ixmax = 0 (i.e. will quant to zero)
//          but note converse does not apply 
//  gmin:   gsf < gmin may exceed max allowed quantized value
// 
    for ( ch = 0; ch < nchan; ch++ )
    {
        y = x34[ch];
        for ( i = 0; i < nsf2[ch]; i++ )
        {
            n = nBand_l[i];
            //x34max[ch][i] = vect_fmax1(y, n);
            vect_fmax2 ( y, n, x34max[ch] + i );
            gzero[ch][i] =
                HX_MAX ( 0,
                      round_to_int ( ( gz_con1B * mbLogC ( x34max[ch][i] ) +
                                       gz_con2B ) ) );
            gmin[ch][i] = HX_MAX ( 0, gzero[ch][i] - GMIN_OFFSET );
            y += n;
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::startup_adjustNT1B (  )
{
    int ch, i, a, na;
    int ab, nab;
    int d, dmax;
    int f;
    static int sthres[21] = {
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        100, 100, 100, 200, 300, 300, 300,
    };

//#define STHRES  300
//#define STHRES  0

    if ( ba_control.test1 == 0 )
        return;
    f = ba_control.test1;

    for ( ch = 0; ch < nchan; ch++ )
    {
        na = 1;
        a = 0;
        nab = 1;
        ab = 0;
        for ( i = 0; i < nsf[ch]; i++ )
        {
            //if( snr[ch][i] > STHRES ) {
            if ( snr[ch][i] > sthres[i] )
            {
                a += NT[ch][i];
                na++;
                ab += nBand_l[i] * NT[ch][i];
                nab += nBand_l[i];
            }
        }
        a = a / na;
        ab = ab / nab;

        if ( na < 5 )
            continue;

        for ( i = 0; i < nsf[ch]; i++ )
        {
            //if( snr[ch][i] > STHRES ) {
            if ( snr[ch][i] > sthres[i] )
            {
                dmax = HX_MAX ( snr[ch][i] - 400, 0 );
                //d = (ab - NT[ch][i]) >> 1;
                d = ( f * ( ab - NT[ch][i] ) ) >> 4;
                d = HX_MIN ( d, dmax );
                NT[ch][i] = NT[ch][i] + d;
            }
        }

    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::noise_seek_initial2 (  )
{
    int i;
    int ch;
    float g4, d, g;

// from AAC code
// g4: gsf such that max quantized value = 4
// aac g_offset = 100, mp3 g_offset = 8

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            NTadjust[ch][i] = HX_MAX ( NTadjust[ch][i], -400 );
            //NTadjust[ch][i] = HX_MIN(NTadjust[ch][i],  100 );
            NTadjust[ch][i] = HX_MIN ( NTadjust[ch][i], 400 );
            g4 = gz_con1B * mbLogC ( x34max[ch][i] ) + ( 88.411238f - 100.0f + 8.0f );  // g4 w/g_offset
            //d = (1.00f/110.5f)*(1800-4*i-(Noise0[ch][i]-NT[ch][i]));
            //d = (1.00f/110.5f)*(1800-8*i-(Noise0[ch][i]-NT[ch][i]));
            d = ( 1.00f / 110.5f ) * ( 1800 - 8 * i -
                                       ( Noise0[ch][i] - NT[ch][i] +
                                         NTadjust[ch][i] ) );
            g = g4 + d;
            gsf[ch][i] = round_to_int ( g );
            gsf[ch][i] = HX_MIN ( gsf[ch][i], gzero[ch][i] );
            gsf[ch][i] = HX_MAX ( gsf[ch][i], gmin[ch][i] );
        }
    }

}

/*===============================================================*/
int
CBitAllo3::decrease_noise ( int s0, int n )
{
    int i;
    int s, smin;
    int tnoise, tnmin, thres;
    int niter;
    int absdn, absdnmin;

    s = s0 - 1;

    thres = NTarget;

    absdnmin = abs ( dn );
    tnmin = noise;
    smin = s0;
    niter = HX_MIN ( s, 20 );

    for ( i = 0; i < niter; i++ )
    {
        tnoise = ifnc_noise_actual ( y34, y, s, n, logn );
        absdn = abs ( tnoise - NTarget );
        if ( absdn < absdnmin )
        {
            absdnmin = absdn;
            tnmin = tnoise;
            smin = s;
        }
        if ( tnoise <= thres )
            break;
        s--;
    }

    noise = tnmin;

    return smin;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::increase_noise ( int s0, int n )
{
    int i;
    int s, smin;
    int tnoise, tnmin, thres;
    int absdn, absdnmin;

// dn < 0

    absdnmin = abs ( dn );

    s = s0;

    thres = NTarget;
    tnmin = noise;
    smin = s0;

    for ( i = 0; i < 20; i++ )
    {
        s++;
        tnoise = ifnc_noise_actual ( y34, y, s, n, logn );
        absdn = abs ( tnoise - NTarget );
        if ( absdn < absdnmin )
        {
            absdnmin = absdn;
            tnmin = tnoise;
            smin = s;
        }
        if ( tnoise >= thres )
            break;
    }

    noise = tnmin;

    return smin;
}

/*---------------------------------------------------------------*/
void
CBitAllo3::noise_seek_actual (  )
{
    int i;
    int ch;
    int s, n;

// ifnc_noise_actualB (opti fwd quant) no help

    for ( ch = 0; ch < nchan; ch++ )
    {
        y34 = x34[ch];
        y = xr[ch];
        for ( i = 0; i < nsf[ch]; i++ )
        {
            NTarget = NT[ch][i];
            n = nBand_l[i];
            s = gsf[ch][i];
            if ( Noise0[ch][i] > NTarget )
            {
                logn = look_log_cbwmb[i];
                noise = ifnc_noise_actual ( y34, y, s, n, logn );
                dn = noise - NTarget;
                NTadjust[ch][i] = NTadjust[ch][i] + ( dn >> 3 );
                if ( dn > 100 )
                {
                    s = decrease_noise ( s, n );
                }
                else if ( dn < -100 )
                {
                    s = increase_noise ( s, n );
                }
                gsf[ch][i] = s;
                Noise[ch][i] = noise;
            }
            else
            {
                //gsf[ch][i] = gzero[ch][i];
                gsf[ch][i] = gzero[ch][i] + 5;  // for increase_bits
                Noise[ch][i] = Noise0[ch][i];
            }

            y34 += n;
            y += n;
        }
    }

}

/*===============================================================*/
void
CBitAllo3::lucky_noise (  )
{
    int i;
    int ch;
    int s, n;
    int sdelta;

    for ( ch = 0; ch < nchan; ch++ )
    {
        sdelta = 2 * ( 1 + scalefactor_scale[ch] );
        y34 = x34[ch];
        y = xr[ch];
        for ( i = 0; i < 11; i++ )
        {
            n = nBand_l[i];
            if ( active_sf[ch][i] && sf[ch][i]
                 && ( gsf[ch][i] < ( gzero[ch][i] - 5 ) ) )
            {
                s = gsf[ch][i] + sdelta;
                logn = look_log_cbwmb[i];
                noise = ifnc_noise_actual ( y34, y, s, n, logn );
                //noise   = ifnc_noise_actualB(y34, y, s, n, logn);
                if ( noise <= Noise[ch][i] )
                {
                    Noise[ch][i] = noise;
                    sf[ch][i] -= sdelta;
                    gsf[ch][i] = s;
                }
            }
            y34 += n;
            y += n;
        }
    }

}

/*---------------------------------------------------------------*/
int
CBitAllo3::sfHeadRoom ( int ch )
{
    int i, n, h;
    int *s, *limit;

    s = sf[ch];
    limit = psf_upper_limit[ch];
    n = nsf[ch];
    h = limit[0];
    for ( i = 0; i < n; i++ )
    {
        h = HX_MIN ( h, limit[i] - s[i] );
    }
    return h;
}

/*---------------------------------------------------------------*/
void
CBitAllo3::big_lucky_noise (  )
{
    int i, m;
    int ch;
    int s, g, n, s0, GG, smin, g0;
    int sdelta;

    for ( ch = 0; ch < nchan; ch++ )
    {
        sdelta = 2 * ( 1 + scalefactor_scale[ch] );
        GG = G[ch];
        y34 = x34[ch];
        y = xr[ch];
        m = HX_MIN ( 13, nsf[ch] );
        //for(i=0;i<nsf[ch];i++) {   // no help
        //for(i=0;i<11;i++) {
        for ( i = 0; i < m; i++ )
        {
            n = nBand_l[i];
            // no need to check very near gzero
            if ( active_sf[ch][i] && ( gsf[ch][i] < ( gzero[ch][i] - 5 ) ) )
            {
                smin = sf[ch][i];
                g0 = gzero[ch][i] - 4;
                s = psf_upper_limit[ch][i];
                s = HX_MIN ( sf[ch][i] - sdelta, s );
                s0 = psf_lower_limit[ch][i];
                logn = look_log_cbwmb[i];
                for ( ; s >= s0; s -= sdelta )
                {
                    g = GG - s;
                    if ( g >= g0 )
                        break;
                    noise = ifnc_noise_actual ( y34, y, g, n, logn );
                    //noise   = ifnc_noise_actualB(y34, y, g, n, logn);
                    if ( noise <= NT[ch][i] )
                    {
                        Noise[ch][i] = noise;
                        smin = s;
                    }
                }
                sf[ch][i] = smin;
                // need to limit gsf, original sf may be disconnected
                // making G-sf[ch][i] < 0
                gsf[ch][i] = HX_MAX ( GG - smin, 0 );
            }
            y34 += n;
            y += n;
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::big_lucky_noise2 (  )
{
    int i;
    int ch;
    int s, g, n, s0, GG, smin, g0;
    int sdelta;
    int h, Gdelta;

// essentially no snr improvement over big_lucky_noise

    for ( ch = 0; ch < nchan; ch++ )
    {
        sdelta = 2 * ( 1 + scalefactor_scale[ch] );
        GG = G[ch];
        y34 = x34[ch];
        y = xr[ch];
        h = sfHeadRoom ( ch );
        Gdelta = 0;
        //for(i=0;i<nsf[ch];i++) {
        for ( i = 0; i < 11; i++ )
        {
            n = nBand_l[i];
            // no need to check very near gzero
            if ( active_sf[ch][i] && ( gsf[ch][i] < ( gzero[ch][i] - 5 ) ) )
            {
                smin = sf[ch][i];
                g0 = gzero[ch][i] - 4;
                s = psf_upper_limit[ch][i];
                s = HX_MIN ( sf[ch][i] - sdelta, s );
                s0 = psf_lower_limit[ch][i] - h;
                logn = look_log_cbwmb[i];
                for ( ; s >= s0; s -= sdelta )
                {
                    g = GG - s;
                    if ( g >= g0 )
                        break;
                    noise = ifnc_noise_actual ( y34, y, g, n, logn );
                    //noise   = ifnc_noise_actualB(y34, y, g, n, logn);
                    if ( noise <= NT[ch][i] )
                    {
                        Noise[ch][i] = noise;
                        smin = s;
                    }
                }
                sf[ch][i] = smin;
                Gdelta = HX_MIN ( Gdelta, smin );
                // need to limit gsf, original sf may be disconnected
                // making G-sf[ch][i] < 0
                gsf[ch][i] = HX_MAX ( GG - smin, 0 );
            }
            y34 += n;
            y += n;
        }
        if ( Gdelta < 0 )
        {
            G[ch] -= Gdelta;
            for ( i = 0; i < nsf[ch]; i++ )
            {
                sf[ch][i] -= Gdelta;
            }
        }

    }   // ch loop

}

/*===============================================================*/
void
CBitAllo3::inverse_sf2 (  )
{
    int ch, i, n, t;
    float *y;
    int *qx;
    int s, GG, Gscale;

// value returns by ifnc_inverse_gsf_snr2 is scaled by 8*1024

// maybe snr sounds better than xfer sinatra @ p0 t-30
// switched to xfer for elevator.wav (first few seconds) 7/27/00

// ver 2 a little better that inverse_sf
// doing ixmax=1,2 better that just ixmax=1, doing all ixmax no help

    for ( ch = 0; ch < nchan; ch++ )
    {
        GG = G[ch];
        Gscale = GG << 13;
        if ( scalefactor_scale[ch] == 0 )
        {
            y = xr[ch];
            qx = ix[ch];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                n = nBand_l[i];
                //if( ixmax[ch][i] == 1 ) {
                if ( ( ixmax[ch][i] == 1 ) || ( ixmax[ch][i] == 2 ) )
                {
                    //t = ifnc_inverse_gsf_snr2(qx, y, n);
                    t = ifnc_inverse_gsf_xfer2 ( qx, y, n );
                    s = ( ( Gscale - t +
                            ( 1 << 13 ) ) & ( ~( ( 1 << 14 ) - 1 ) ) ) >> 13;
                    s = HX_MIN ( s, psf_upper_limit[ch][i] );
                    s = HX_MAX ( s, psf_lower_limit[ch][i] );
                    sf[ch][i] = s;
                }
                y += n;
                qx += n;
            }
        }
        else
        {       //  scalefactor_scale[ch] == 1
            y = xr[ch];
            qx = ix[ch];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                n = nBand_l[i];
                //if( ixmax[ch][i] == 1 ) {
                if ( ( ixmax[ch][i] == 1 ) || ( ixmax[ch][i] == 2 ) )
                {
                    //t = ifnc_inverse_gsf_snr2(qx, y, n);
                    t = ifnc_inverse_gsf_xfer2 ( qx, y, n );
                    s = ( ( Gscale - t +
                            ( 1 << 14 ) ) & ( ~( ( 1 << 15 ) - 1 ) ) ) >> 13;
                    s = HX_MIN ( s, psf_upper_limit[ch][i] );
                    s = HX_MAX ( s, psf_lower_limit[ch][i] );
                    sf[ch][i] = s;
                }
                y += n;
                qx += n;
            }
        }
    }

}

/*===============================================================*/
void
CBitAllo3::quant ( int gsf[2][22] )
{
    int ch, i, n;
    float *x;
    int *qx;

    for ( ch = 0; ch < nchan; ch++ )
    {
        x = x34[ch];
        qx = ix[ch];
        for ( i = 0; i < nsf[ch]; i++ )
        {
            n = nBand_l[i];
            ixmax[ch][i] = vect_quant ( x, qx, gsf[ch][i], n );
            x += n;
            qx += n;
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::quantB ( int gsf[2][22] )
{
    int ch, i, n;
    float *x;
    int *qx;

// optimum quantizer

    for ( ch = 0; ch < nchan; ch++ )
    {
        x = x34[ch];
        qx = ix[ch];
        for ( i = 0; i < nsf[ch]; i++ )
        {
            n = nBand_l[i];
            ixmax[ch][i] = vect_quantB ( x, qx, gsf[ch][i], n );
            x += n;
            qx += n;
        }
    }

/*--------------------
for(ch=0;ch<nchan;ch++) {
    x  = x34[ch];
    qx = ix[ch];
    for(i=0;i<nsf[ch];i++) {
        n = nBand_l[i];
        //ixmax[ch][i] = vect_quantB(x, qx, gsf[ch][i], n);
        //ixmax[ch][i] = vect_quantB2(x, qx, gsf[ch][i], n, -.3429f);
        ixmax[ch][i] = vect_quantB2(x, qx, gsf[ch][i], n, -.30f);
        shave_01pairs(x, qx, gsf[ch][i], n, 0.75f );
        if( ixmax[ch][i] == 1 ) { // 12/1/99 needed for shave to zero
            ixmax[ch][i] = vect_imax(qx, n);
        }
        x  += n;
        qx += n;
    }
}
----------------*/

}

/*---------------------------------------------------------------*/
void
CBitAllo3::quantB10x ( int gsf[2][22] ) // used for thresholding
{
    int ch, i, n;
    float *x;
    int *qx;

// optimum quantizer 10x actual
// result can be negative

    for ( ch = 0; ch < nchan; ch++ )
    {
        x = x34[ch];
        qx = ix10x[ch];
        for ( i = 0; i < nsf[ch]; i++ )
        {
            n = nBand_l[i];
            ix10xmax[ch][i] = vect_quantB10x ( x, qx, gsf[ch][i], n );
            x += n;
            qx += n;
        }
    }

}

/*===============================================================*/
void
CBitAllo3::clear_hf_main (  )
{
    int i, n;
    int *qx;

    qx = ix[0] + startBand_l[21];
    n = nBand_l[21];
    for ( i = 0; i < n; i++ )
        qx[i] = 0;

}

/*---------------------------------------------------------------*/
void
CBitAllo3::clear_hf (  )
{
    int i, n, ch;
    int *qx;

    for ( ch = 0; ch < nchan; ch++ )
    {
        qx = ix[ch] + startBand_l[21];
        n = nBand_l[21];
        for ( i = 0; i < n; i++ )
            qx[i] = 0;
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::sparse_quad_counted ( int qx[], int n, int sparse_level )
{
    int i, c, scnt, m;

// sparse_level = 0 to 16;
// sparse_level/16 fraction allowed to be removed
//
    c = 0;
    for ( i = 0; i < n; i++ )
    {
        c += qx[i];
    }
    c = ( sparse_level * c ) >> 4;
    if ( c <= 0 )
        return;

    scnt = 0;
    for ( i = n - 4; i >= 0; i -= 4 )
    {
        m = qx[i] + qx[i + 1] + qx[i + 2] + qx[i + 3];
        if ( m == 1 )
        {
            qx[i] = qx[i + 1] = qx[i + 2] = qx[i + 3] = 0;
            scnt++;
            if ( scnt >= c )
                break;
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::quantBhf (  )
{
    int ch, n;
    float *x;
    int *qx;

// quant hf region

    for ( ch = 0; ch < nchan; ch++ )
    {
        if ( hf_quant_stereo[ch] )
        {
            x = x34[ch] + startBand_l[21];
            qx = ix[ch] + startBand_l[21];
            n = nBand_l[21];
            ixmax[ch][21] = vect_quantB2 ( x, qx, G[ch], n, -.30f );
            sparse_quad_counted ( qx, n, 4 );
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::quantBhf_ms (  )
{
    int n;
    float *x;
    int *qx;
    int itmp = 999;
    static int dcnt;

// quant hf region

    x = x34[0] + startBand_l[21];
    qx = ix[0] + startBand_l[21];
    n = nBand_l[21];
    ixmax[0][21] = vect_quantB2 ( x, qx, G[0], n, -.30f );

}

/*===============================================================*/
int
CBitAllo3::count_bits (  )
{
    int ch, bits;

    for ( bits = 0, ch = 0; ch < nchan; ch++ )
    {
        huff_bits[ch] = subdivide2 ( ixmax[ch], ix[ch], nsf2[ch], 1, ch );
        bits += huff_bits[ch];
    }
    return bits;
}

/*----------------------------------------------------------------*/
int
CBitAllo3::count_bits_dual (  )
{
    int ch, bits;

    for ( bits = 0, ch = 0; ch < nchan; ch++ )
    {
        huff_bits[ch] = subdivide2 ( ixmax[ch], ix[ch], nsf3[ch], 1, ch );
        bits += huff_bits[ch];
    }
    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::count_bits2 ( int opti_flag )
{
    int ch, bits;

    for ( bits = 0, ch = 0; ch < nchan; ch++ )
    {
        huff_bits[ch] =
            subdivide2 ( ixmax[ch], ix[ch], nsf2[ch], opti_flag, ch );
        bits += huff_bits[ch];
    }
    return bits;
}

/*===============================================================*/
void
CBitAllo3::fnc_sf_final ( int ch )
{
    if ( h_id )
        fnc_sf_final_MPEG1 ( ch );
    else
        fnc_sf_final_MPEG2 ( ch );
}

/*===============================================================*/
void
CBitAllo3::fnc_sf_final_MPEG1 ( int ch )
{
    int i;
    int pre_flag, scale_flag;
    int sp0, sp1, sp2, sp3;
    int s;

//*************************************
// probably be better to do more clamping
// and less sf_scale = 1 
//
// if preemp would work except for neg sf, better to clamp?
//
// shift gmax to lower value if gmax near gzero in low 
// freq region?
//
//***************************

    sp0 = sp1 = sp2 = sp3 = 0;
    for ( i = 0; i < nsf[ch]; i++ )
    {
        if ( active_sf[ch][i] )
        {
            s = sf[ch][i];
            sp0 |= ( sf_limit[0][i] - s );
            sp1 |= ( sf_limit[1][i] - s );
            sp2 |= ( sf_limit[2][i] - s );
            sp3 |= ( sf_limit[3][i] - s );
            // disallow negative sf's in pre-emp region
            sp1 |= ( s - sf_limit[4][i] );
            sp3 |= ( s - sf_limit[5][i] );
        }
    }

    if ( sp0 >= 0 )
    {
        scale_flag = 0;
        pre_flag = 0;
    }
    else if ( sp1 >= 0 )
    {
        scale_flag = 0;
        pre_flag = 1;
    }
    else if ( sp2 >= 0 )
    {
        scale_flag = 1;
        pre_flag = 0;
    }
    else if ( sp3 >= 0 )
    {
        scale_flag = 1;
        pre_flag = 1;
    }
    else
    {   // default     out of range - need to be clamped
        scale_flag = 1;
        pre_flag = 0;
    }

    preemp[ch] = pre_flag;
    scalefactor_scale[ch] = scale_flag;

}

/*----------------------------------------------------------------*/
void
CBitAllo3::fnc_sf_final_MPEG2 ( int ch )
{
    int i;
    int scale_flag;
    int sp0;
    int s;

// no pre-emp for mpeg2 - if want pre in mpeg2, can only use 2 bit sf's

    sp0 = 0;
    for ( i = 0; i < nsf[ch]; i++ )
    {
        if ( active_sf[ch][i] )
        {
            s = sf[ch][i];
            sp0 |= ( sf_limit[0][i] - s );
        }
    }

    scale_flag = 1;     // assume scale flag needed (could be out of range also)
    if ( sp0 >= 0 )
    {
        scale_flag = 0;
    }

    preemp[ch] = 0;
    scalefactor_scale[ch] = scale_flag;

}

/*----------------------------------------------------------------*/
int
CBitAllo3::fnc_scale_factors (  )
{
    int i, ch;
    int Gtmp, Gtmpmin;
    int s, d, dN;
    int dsf;

// no adjustments made for gsf near gzero, 
//  handled by inverse sf routine
//

    Gtmpmin = 999;
    for ( ch = 0; ch < nchan; ch++ )
    {

/* compute G */
//Gtmp = -1;
        Gtmp = gsf_hf_stereo[ch];
        for ( i = 0; i < nsf[ch]; i++ )
        {
            gsf[ch][i] = HX_MAX ( gsf[ch][i], gmin[ch][i] );       // quant overflow protect
            //if( Noise0[ch][i] <= NT[ch][i] ) {
            //    gsf[ch][i] = gzero[ch][i];
            //}
            active_sf[ch][i] = 0;
            if ( gsf[ch][i] < gzero[ch][i] )
            {
                active_sf[ch][i] = -1;
                Gtmp = HX_MAX ( Gtmp, gsf[ch][i] );
            }
        }

        if ( Gtmp < 0 )
        {       /* null frame for ch */
            for ( i = 0; i < nsf[ch]; i++ )
            {
                sf[ch][i] = 0;
                gsf[ch][i] = gzero[ch][i];
                Gtmp = HX_MAX ( Gtmp, gsf[ch][i] );
            }
            preemp[ch] = 0;
            scalefactor_scale[ch] = 0;
            G[ch] = Gtmp;
            if ( Gtmpmin > 100 )
                Gtmpmin = 100;
            psf_upper_limit[ch] = sf_upper_limit[0][0];
            psf_lower_limit[ch] = sf_lower_limit[0][0];
            continue;   // continue channel loop
        }

/* scale factors */

        for ( i = 0; i < nsf[ch]; i++ )
        {
            sf[ch][i] = 0;
            if ( active_sf[ch][i] )
                sf[ch][i] = Gtmp - gsf[ch][i];
        }

/* choose scale preemp etc. */
        fnc_sf_final ( ch );

        if ( scalefactor_scale[ch] == 0 )
        {
            dsf = 2;    // save for possible use later
            for ( i = 0; i < nsf[ch]; i++ )
            {
                //if( Noise[ch][i] > NT[ch][i] ) sf[ch][i]++;
                if ( ( i < 11 ) && ( Noise[ch][i] > NT[ch][i] ) )
                    sf[ch][i]++;
                sf[ch][i] &= ( ~1 );    // non-active will remain 0
            }
        }
        else
        {
            dsf = 4;    // save for possible use later
            for ( i = 0; i < nsf[ch]; i++ )
            {
                //if( (i<11) && (Noise[ch][i] > NT[ch][i]) ) sf[ch][i]++;
                //sf[ch][i] = (sf[ch][i]+1) & (~3);
                s = sf[ch][i] & ( ~3 );
                d = sf[ch][i] - s;
                dN = Noise[ch][i] - NT[ch][i] + 150 * d;
                if ( dN > dNthres[i] )
                    s = s + 4;
                sf[ch][i] = s & active_sf[ch][i];       // non-active set to zero
            }
        }

// apply hard limits
// non-active will be set to lower_limit
// save limit pointers
        psf_upper_limit[ch] =
            sf_upper_limit[scalefactor_scale[ch]][preemp[ch]],
            psf_lower_limit[ch] =
            sf_lower_limit[scalefactor_scale[ch]][preemp[ch]],
            vect_limits ( sf[ch], psf_upper_limit[ch], psf_lower_limit[ch],
                          nsf[ch] );

/* back compute gsf */
        for ( i = 0; i < nsf[ch]; i++ )
        {
            if ( active_sf[ch][i] )
            {
                gsf[ch][i] = Gtmp - sf[ch][i];
                if ( gsf[ch][i] < 0 )
                {       // sf up rounding could make gsf < 0
                    gsf[ch][i] += dsf;  // in extreem cases of low signals
                    sf[ch][i] -= dsf;   // 
                    assert ( sf[ch][i] >= psf_lower_limit[ch][i] );
                }
                if ( gsf[ch][i] >= gzero[ch][i] )
                {       // sf may have become inactive
                    gsf[ch][i] = gzero[ch][i] + 5;      // set to g+5 for increase_bits
                    sf[ch][i] = psf_lower_limit[ch][i]; // set to lower, not zero, incase preemp
                }
            }
        }

        G[ch] = Gtmp;
        if ( Gtmp < Gtmpmin )
            Gtmpmin = Gtmp;

    }   // end ch loop 

    return Gtmpmin;
}

/*----------------------------------------------------------------*/
int
CBitAllo3::fnc_scale_factors_ms (  )
{
    int i, ch;
    int Gtmp, Gtmpmin;
    int s, d, dN;
    int dsf;

// no adjustments made for gsf near gzero, 
//  handled by inverse sf routine
//

    Gtmp = -1;
    if ( hf_quant )
    {
        Gtmp = gsf_hf;
    }

    Gtmpmin = 999;
    for ( ch = 0; ch < nchan; ch++ )
    {

/* compute G */
        for ( i = 0; i < nsf[ch]; i++ )
        {
            gsf[ch][i] = HX_MAX ( gsf[ch][i], gmin[ch][i] );       // quant overflow protect
            //if( Noise0[ch][i] <= NT[ch][i] ) {
            //    gsf[ch][i] = gzero[ch][i];
            //}
            active_sf[ch][i] = 0;
            if ( gsf[ch][i] < gzero[ch][i] )
            {
                active_sf[ch][i] = -1;
                Gtmp = HX_MAX ( Gtmp, gsf[ch][i] );
            }

        }

        if ( Gtmp < 0 )
        {       /* null frame for ch */
            for ( i = 0; i < nsf[ch]; i++ )
            {
                sf[ch][i] = 0;
                gsf[ch][i] = gzero[ch][i];
                Gtmp = HX_MAX ( Gtmp, gsf[ch][i] );
            }
            preemp[ch] = 0;
            scalefactor_scale[ch] = 0;
            G[ch] = Gtmp;
            Gtmpmin = HX_MIN ( Gtmpmin, 100 );
            psf_upper_limit[ch] = sf_upper_limit[0][0];
            psf_lower_limit[ch] = sf_lower_limit[0][0];
            continue;   // continue channel loop
        }

/* scale factors */

        for ( i = 0; i < nsf[ch]; i++ )
        {
            //sf[ch][i] = 0;
            //if( active_sf[ch][i] ) sf[ch][i] = Gtmp - gsf[ch][i];
            sf[ch][i] = ( Gtmp - gsf[ch][i] ) & active_sf[ch][i];
        }

/* choose scale preemp etc. */
        fnc_sf_final ( ch );

        if ( scalefactor_scale[ch] == 0 )
        {
            dsf = 2;    // save sf delta for possible use later
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( active_sf[ch][i] )
                {       // for gzero stuff
                    if ( ( gzero[ch][i] - gsf[ch][i] ) < 5 )
                    {
                        sf[ch][i]++;
                    }
                    else if ( ( i < 11 ) && ( Noise[ch][i] > NT[ch][i] ) )
                    {
                        sf[ch][i]++;
                    }
                    sf[ch][i] &= ( ~1 );
                }
                //if( (i<11) && (Noise[ch][i] > NT[ch][i]) ) sf[ch][i]++;
                //sf[ch][i] &= (~1);
            }
        }
        else
        {
            dsf = 4;    // save sf delta for possible use later
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( active_sf[ch][i] )
                {       // not needed? for gzero stuff?
                    s = sf[ch][i] & ( ~3 );
                    d = sf[ch][i] - s;
                    dN = Noise[ch][i] - NT[ch][i] + 150 * d;
                    if ( dN > dNthres[i] )
                    {
                        s = s + 4;
                    }
                    else if ( ( gzero[ch][i] - gsf[ch][i] - d ) < 5 )
                    {
                        s = s + 4;      // do not drop bands near gzero
                    }
                    sf[ch][i] = s;
                }
            }
        }

// apply hard limits
// non-active will be set to lower_limit
// save limit pointers
        psf_upper_limit[ch] =
            sf_upper_limit[scalefactor_scale[ch]][preemp[ch]],
            psf_lower_limit[ch] =
            sf_lower_limit[scalefactor_scale[ch]][preemp[ch]],
            vect_limits ( sf[ch], psf_upper_limit[ch], psf_lower_limit[ch],
                          nsf[ch] );

/* back compute gsf */
        for ( i = 0; i < nsf[ch]; i++ )
        {
            if ( active_sf[ch][i] )
            {
                gsf[ch][i] = Gtmp - sf[ch][i];
                if ( gsf[ch][i] < 0 )
                {       // sf up rounding could make gsf < 0 
                    gsf[ch][i] += dsf;  // in extreem cases of low signals   
                    sf[ch][i] -= dsf;   //                                   
                    assert ( sf[ch][i] >= psf_lower_limit[ch][i] );
                }
                if ( gsf[ch][i] >= gzero[ch][i] )
                {
                    gsf[ch][i] = gzero[ch][i] + 5;      // set to gz+5 for increase_bits
                    sf[ch][i] = psf_lower_limit[ch][i]; // set to lower, not zero, incase preemp
                }
            }
        }

        G[ch] = Gtmp;
        Gtmpmin = HX_MIN ( Gtmpmin, Gtmp );

        Gtmp = -1;
    }   // end ch loop 

    return Gtmpmin;
}

/*===============================================================*/
void
CBitAllo3::trade_side (  )
{
    int i;
    int g, dg;
    float xg;
    static int tcnt, dcnt, dcnt2;
    int thres;

// shave/flatten side channel ixmax peaks in huff table region2

// compute ixmax - only need to compute regions of interest
//vect_ixmax_quantB(x34max[0], ixmax[0], gsf[0], nsf[0]);
    vect_ixmax_quantB ( x34max[1], ixmax[1], gsf[1], nsf[1] );

//vect_ix10xmax_quantB(x34max[0], ix10xmax[0], gsf[0], nsf[0]);
    vect_ix10xmax_quantB ( x34max[1], ix10xmax[1], gsf[1], nsf[1] );

// should not let sf function increase ixmax in processed region 
// need sf_lock for use when sf scale = 1

    thres = 30;
    for ( i = nsf[1] - 1; i >= 13; i-- )
    {
        if ( ix10xmax[1][i] > thres )
            break;
        if ( ixmax[1][i] == 2 )
        {
            thres -= 2;
            xg = 1.7717f * dbLog ( x34max[1][i] *
                                   ( 1.0f / ( 1.5f + 0.02799f ) ) );
            g = ( int ) ( xg + 1.0f );
            g = g + g_offset;
            dg = g - gsf[1][i];
            gsf[1][i] = g;
        }
        thres--;
        thres = HX_MAX ( 16, thres );
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::trade_dual (  )
{
    int i, ch;
    int ixmax0;
    int g, dg;
    float xg;
    float eixmax;
    float feixmax, fetot;
    int ixtarget;
    float factor, ftmp;
    int k0, k1;

    for ( ch = 0; ch < nchan; ch++ )
    {
// compute ixmax - only need to compute regions of interest
        vect_ixmax_quantB ( x34max[ch], ixmax[ch], gsf[ch], nsf[ch] );
        vect_ix10xmax_quantB ( x34max[ch], ix10xmax[ch], gsf[ch], nsf[ch] );

        for ( i = nsf[ch] - 1; i >= 11; i-- )
        {
            if ( ix10xmax[ch][i] > 16 )
                break;
            if ( ixmax[ch][i] == 2 )
            {
                xg = 1.7717f * dbLog ( x34max[ch][i] *
                                       ( 1.0f / ( 1.5f + 0.02799f ) ) );
                g = ( int ) ( xg + 1.0f );
                g = g + g_offset;
                dg = g - gsf[ch][i];
                gsf[ch][i] = g;
            }
        }
        k1 = i + 1;
        if ( k1 < 9 )
            continue;
        k0 = ( 3 * k1 ) >> 2;
        if ( k0 < 11 )
            k0 = 11;    // will need sf lock
        if ( k0 >= k1 )
            continue;

// should not let sf function increase ixmax in processed region 
        ixmax0 = vect_imax ( ixmax[ch] + k0, k1 - k0 );
        if ( ixmax0 <= 2 )
            continue;

        fetot = 0;
        feixmax = 0;
        for ( i = k0; i < k1; i++ )
        {
            ftmp = rnBand_l[i] * xsxx[ch][i];
            fetot += ftmp;
            feixmax += ftmp * ix10xmax[ch][i];
        }

        eixmax = feixmax / ( 1.0f + fetot );
        ixtarget = ( int ) ( 0.1f * eixmax + 0.65f );

        if ( ixtarget < 2 )
            ixtarget = 2;
        if ( ixmax0 <= ixtarget )
            continue;
        if ( ixtarget > 15 )
            continue;

        ixtarget = target_table[ixtarget];
        factor = factor_table[ixtarget];

        for ( i = k0; i < k1; i++ )
        {
            if ( ixmax[ch][i] > ixtarget )
            {
                xg = 1.7717f * dbLog ( x34max[ch][i] * factor );
                g = ( int ) ( xg + 1.0f );
                g = g + g_offset;
                dg = g - gsf[ch][i];
                gsf[ch][i] = g;
            }
        }

    }   //ch loop

    return;
}

/*===============================================================*/
void
CBitAllo3::ms_sparse (  )
{
    int i;

// 5 changed to 14, 6/5/00 for tmpsax2.wav and gspi35_1
// n=5 damages snr, these files have extreem tonality with high
// channel correlation.  Some day: Could adapt sparsing factors
// depending on snr, high snr (very tonal) get little sparsing,
// noisy (low snr) get high sparsing.

    for ( i = nsf[0] - 1; i >= 14; i-- )
    {
        if ( xsxxms[1][i] > 0.5 * sparse_table[i] * xsxxms[0][i] )
            break;
        gsf[1][i] = gzero[1][i];
        ixmax[1][i] = 0;
    }

    for ( ; i >= 14; i-- )
    {
        if ( xsxxms[1][i] < 0.25 * sparse_table[i] * xsxxms[0][i] )
        {
            gsf[1][i] = gzero[1][i];
            ixmax[1][i] = 0;
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo3::ms_sparse_quads (  )
{
    int i, j, n, m, m0;
    int nbig, nquads;
    INTPAIR nq;
    float *x;
    int *qx;
    static int tcnt, dcnt, dcnt2;
    int iband;

// drop for gspi35_2 - not beneficial

    if ( block_type )
        return;

    nq = subdivide2_quadregion ( ixmax[1], ix[1], nsf[1], 1 );
// (nbig,nquads)
// nbig = num samples
    nbig = nq.a;
    nquads = nq.b;

    if ( nquads <= 0 )
        return;

// optimum quantizer 10x actual
// quantB10x result can be negative!
    x = x34[1];
    qx = ix10x[1];
    iband = 999;
    for ( i = 0; i < nsf[1]; i++ )
    {
        n = nBand_l[i];
        if ( nbig < startBand_l[i + 1] )
        {
            ix10xmax[1][i] = vect_quantB10x ( x, qx, gsf[1][i], n );
            iband = HX_MIN ( iband, i );
        }
        x += n;
        qx += n;
    }

    i = nbig;
    for ( j = 0; j < nquads; j++, i += 4 )
    {
        m0 = ix[0][i] + ix[0][i + 1] + ix[0][i + 2] + ix[0][i + 3];
        m = ix[1][i] + ix[1][i + 1] + ix[1][i + 2] + ix[1][i + 3];
        if ( ( m == 1 ) )
        {
            if ( m0 >= 4 )
            {
                if ( ix10x[1][i] <= 9 )
                    ix[1][i] = 0;
                if ( ix10x[1][i + 1] <= 9 )
                    ix[1][i + 1] = 0;
                if ( ix10x[1][i + 2] <= 9 )
                    ix[1][i + 2] = 0;
                if ( ix10x[1][i + 3] <= 9 )
                    ix[1][i + 3] = 0;
            }
            else
            {
                if ( ix10x[1][i] <= 7 )
                    ix[1][i] = 0;
                if ( ix10x[1][i + 1] <= 7 )
                    ix[1][i + 1] = 0;
                if ( ix10x[1][i + 2] <= 7 )
                    ix[1][i + 2] = 0;
                if ( ix10x[1][i + 3] <= 7 )
                    ix[1][i + 3] = 0;
            }

        }

    }

    qx = ix[1] + startBand_l[iband];
    for ( i = iband; i < nsf[1]; i++ )
    {
        n = nBand_l[i];
        ixmax[1][i] = vect_imax ( qx, n );
        qx += n;
    }

}

/*===============================================================*/
void
CBitAllo3::hf_adjust (  )
{
    int i, ch;
    int gmax, gmax0, gmax1, ig;
    int ixmax0, ixg;
    int gtar, gtar2, gset;

    gsf_hf_stereo[0] = gsf_hf_stereo[1] = -1;

    for ( ch = 0; ch < nchan; ch++ )
    {
        // don't attempt on low hf signals
        if ( gzero[ch][21] <= 8 )
            continue;   // 3/3/99
        gmax0 = 0;
        ig = 0;
        for ( i = 0; i < 11; i++ )
        {
            if ( gsf[ch][i] < gzero[ch][i] )
            {
                if ( gsf[ch][i] > gmax0 )
                {
                    gmax0 = gsf[ch][i];
                    ig = i;
                }
            }
        }
        gmax1 = 0;
        for ( i = 11; i < nsf[ch]; i++ )
        {
            if ( gsf[ch][i] < gzero[ch][i] )
            {
                if ( gsf[ch][i] > gmax1 )
                {
                    gmax1 = gsf[ch][i];
                }
            }
        }

        ixmax0 = vect_imax ( ixmax[ch], 11 );
        ixg = ixmax[ch][ig];

        gtar = HX_MAX ( 0, gzero[ch][21] - 5 );
        gtar2 = HX_MAX ( 0, gzero[ch][21] - 7 );
        gmax = HX_MAX ( gmax0, gmax1 );

        if ( gtar >= gmax )
        {
            hf_quant_stereo[ch] = 1;
            gsf_hf_stereo[ch] = gtar2;
        }
        else if ( gmax0 > gmax1 )
        {
            gset = HX_MAX ( gtar, gmax1 );
            if ( gzero[ch][21] > gset )
            {
                for ( i = 0; i < 11; i++ )
                {
                    if ( gsf[ch][i] < gzero[ch][i] )
                    {
                        if ( gsf[ch][i] > gset )
                            gsf[ch][i] = gset;
                    }
                }
                hf_quant_stereo[ch] = 1;
            }
        }

    }

    hf_quant = hf_quant_stereo[0] | hf_quant_stereo[1];

}

/*---------------------------------------------------------------*/
void
CBitAllo3::hf_adjust_ms (  )
{
    int i;
    int gmax, gmax0, gmax1, ig;
    int ixmax0, ixg;
    int gtar, gtar2, gset;

// don't attempt on low level hf signals
    if ( gzero[0][21] <= 8 )
    {
        return; // 3/3/99
    }

    gmax0 = 0;
    ig = 0;
    for ( i = 0; i < 11; i++ )
    {
        if ( gsf[0][i] < gzero[0][i] )
        {
            if ( gsf[0][i] > gmax0 )
            {
                gmax0 = gsf[0][i];
                ig = i;
            }
        }
    }

    gmax1 = 0;
    for ( i = 11; i < nsf[0]; i++ )
    {
        if ( gsf[0][i] < gzero[0][i] )
        {
            if ( gsf[0][i] > gmax1 )
            {
                gmax1 = gsf[0][i];
            }
        }
    }

    ixmax0 = vect_imax ( ixmax[0], 11 );
    ixg = ixmax[0][ig];

    gtar = HX_MAX ( 0, gzero[0][21] - 5 );
    gtar2 = HX_MAX ( 0, gzero[0][21] - 7 );
    gmax = HX_MAX ( gmax0, gmax1 );

    if ( gtar >= gmax )
    {
        hf_quant = 1;
        gsf_hf = gtar2;
    }
    else if ( gmax0 > gmax1 )
    {
        gset = HX_MAX ( gtar, gmax1 );
        if ( gzero[0][21] > gset )
        {
            for ( i = 0; i < 11; i++ )
            {
                if ( gsf[0][i] < gzero[0][i] )
                {
                    if ( gsf[0][i] > gset )
                        gsf[0][i] = gset;
                }
            }
            hf_quant = 1;
        }
    }

}

/*===============================================================*/
int
CBitAllo3::increase_bits ( int bits0 )
{
    int i, k, ch;
    int bits;
    int thres;
    int g[2][21];

// need to code hf2 to prevent dropouts in the white noise game

    thres = minTargetBits - ( minTargetBits >> 4 );
    if ( bits0 > thres )
        return bits0;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
            g[ch][i] = gsf[ch][i];
    }

    for ( k = 0; k < 10; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
                gsf[ch][i] = g[ch][i] = HX_MAX ( g[ch][i] - 1, gmin[ch][i] );
        }
        if ( hf_flag & 2 )
        {
            hf_quant = 0;
            hf_quant_stereo[0] = hf_quant_stereo[1] = 0;
            gsf_hf_stereo[0] = gsf_hf_stereo[1] = -1;
            ixmax[0][21] = ixmax[1][21] = 0;
            hf_adjust (  );
        }
        fnc_scale_factors (  );
        quantB ( gsf );
        if ( hf_quant )
            quantBhf (  );
        bits = count_bits_dual (  );
        if ( bits >= thres )
            break;
    }

// avoid having both increase and decrease
// this would invalidate deltaMNR computed in decrease
//
    if ( bits > maxTargetBits )
    {
        // go back to previous allocation
        // may happen with restricted bit pool fraction
        // min and max bits may be very close
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
                gsf[ch][i] = g[ch][i] + 1;
        }
        if ( hf_flag & 2 )
        {
            hf_quant = 0;
            hf_quant_stereo[0] = hf_quant_stereo[1] = 0;
            gsf_hf_stereo[0] = gsf_hf_stereo[1] = -1;
            ixmax[0][21] = ixmax[1][21] = 0;
            hf_adjust (  );
        }
        fnc_scale_factors (  );
        quantB ( gsf );
        if ( hf_quant )
            quantBhf (  );
        bits = count_bits_dual (  );
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::increase_bits_ms ( int bits0 )
{
    int i, k, ch;
    int bits;
    int thres;
    int g[2][21];

    thres = minTargetBits - ( minTargetBits >> 4 );
    if ( bits0 > thres )
        return bits0;

    for ( i = 0; i < nsf[0]; i++ )
    {
        g[0][i] = gsf[0][i];
        g[1][i] = gsf[1][i];
    }

    for ( k = 0; k < 10; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                gsf[ch][i] = g[ch][i] = HX_MAX ( g[ch][i] - 1, gmin[ch][i] );
            }
        }
        hf_quant = 0;
        ixmax[0][21] = 0;
        gsf_hf = -1;
        clear_hf_main (  );
        if ( hf_flag )
            hf_adjust_ms (  );
        fnc_scale_factors_ms (  );
        quantB ( gsf );
        ixmax[0][21] = 0;
        if ( hf_quant )
            quantBhf_ms (  );
        //ms_sparse_quads();
        bits = count_bits (  );
        if ( bits >= thres )
            break;

    }

// avoid having both increase and decrease
// this would invalidate deltaMNR computed in decrease
//

    if ( bits > maxTargetBits )
    {
        // may happen with restricted bit pool fraction
        // min and max bits may be very close
        // go back to previous allocation
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                gsf[ch][i] = g[ch][i] + 1;
            }
        }
        hf_quant = 0;
        ixmax[0][21] = 0;
        gsf_hf = -1;
        clear_hf_main (  );
        if ( hf_flag )
            hf_adjust_ms (  );
        fnc_scale_factors_ms (  );
        quantB ( gsf );
        ixmax[0][21] = 0;
        if ( hf_quant )
            quantBhf_ms (  );
        //ms_sparse_quads();
        bits = count_bits (  );
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::limit_bits ( int bits0 )
{
    int i, k, ch;
    int bits;

    for ( k = 0; k < 100; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
                gsf[ch][i] = HX_MIN ( 127, gsf[ch][i] + 1 );
        }
        fnc_scale_factors (  );
        quant ( gsf );
        bits = count_bits (  );
        if ( bits <= maxBits )
            break;
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::limit_part23_bits (  )
{
    int i, k, ch;
    int bits;

    for ( k = 0; k < 100; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            if ( huff_bits[ch] > PART23 )
            {
                for ( i = 0; i < nsf[ch]; i++ )
                    gsf[ch][i] = HX_MIN ( 127, gsf[ch][i] + 1 );
            }
        }
        fnc_scale_factors (  );
        quant ( gsf );
        bits = count_bits (  );
        if ( ( huff_bits[0] <= PART23 ) && ( huff_bits[1] <= PART23 ) )
            break;
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::decrease_bits01 ( int bits0 )
{
    int i, k, ch;
    int bits;
    int thres;
    int deltaN;

//deltaN = (int)(75.0f*(150.0f/(0.20f*(activeBands + 10))));
    deltaN =
        round_to_int ( 75.0f *
                       ( 150.0f / ( 0.20f * ( activeBands + 10 ) ) ) );
    deltaN = HX_MIN ( deltaN, 200 );
    deltaN = HX_MAX ( deltaN, 40 );

    thres = maxTargetBits;

    for ( k = 0; k < 10; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                NT[ch][i] += deltaN;
            }
        }
        noise_seek_actual (  );
        fnc_scale_factors (  );
        quant ( gsf );
        bits = count_bits (  );
        if ( bits <= thres )
            break;
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::decrease_bits ( int bits0 )
{
    int i, k, ch;
    int bits;
    int deltaN;
    int f;

//deltaN = (int)(75.0f*(150.0f/(0.20f*(activeBands + 10))));
//deltaN = 500*(bits0-maxTargetBits)/(activeBands+10);

    f = ( 250 * 1024 ) / ( activeBands + 10 );
    deltaN = ( f * ( bits0 - maxTargetBits ) ) >> 10;
    deltaN = HX_MAX ( deltaN, 40 );

    deltaMNR = 0;

    for ( k = 0; k < 10; k++ )
    {
        deltaMNR += deltaN;
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                NT[ch][i] += deltaN;
            }
        }
        noise_seek_actual (  );
        fnc_scale_factors (  );
        quant ( gsf );
        //quantB(gsf);
        bits = count_bits (  );
        if ( bits <= maxTargetBits )
            break;
        deltaN = ( f * ( bits - maxTargetBits ) ) >> 10;
        deltaN = HX_MAX ( deltaN, 40 );
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::decrease_bits_ms ( int bits0 )
{
    int i, k, ch;
    int bits;
    int deltaN;
    int f;

    f = ( 250 * 1024 ) / ( activeBands + 10 );
    deltaN = ( f * ( bits0 - maxTargetBits ) ) >> 10;
    deltaN = HX_MAX ( deltaN, 40 );

    deltaMNR = 0;

    for ( k = 0; k < 10; k++ )
    {
        deltaMNR += deltaN;
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                NT[ch][i] += deltaN;
            }
        }
        noise_seek_actual (  );
        //ms_sparse();
        fnc_scale_factors_ms (  );
        quant ( gsf );
        //quantB(gsf);
        //ms_sparse_quads();
        bits = count_bits (  );
        if ( bits <= maxTargetBits )
            break;
        deltaN = ( f * ( bits - maxTargetBits ) ) >> 10;
        deltaN = HX_MAX ( deltaN, 40 );
    }

    return bits;
}

/*===============================================================*/
void
CBitAllo3::mnr_feedback ( int activeBands, int bits, int block_type )
{
    int mnr, mnr2, mnr0, mnrp;
    float deltaNB;
    int dmnr, maxdmnr;
    int dPool, dBits;
    int ddmnr;

    if ( block_type == 2 )
    {
        //MNR = HX_MIN(MNR, initialMNR);
        return;
    }

    if ( call_count > 10 )
    {
        deltaNB = 150.0f / ( 0.20f * ( activeBands + 10 ) );
        mnr = ( int ) ( 0.05 * deltaNB * ( bits - TargetBits ) );
        mnr2 = ( int ) ( 0.05 * deltaNB * HX_MAX ( ( bits - maxBits ), 0 ) );

        dPool = HX_MAX ( TargetBits - bits, 0 );
        dBits = ( ( ( 2044 + 8 * 5 ) - PoolBits ) >> 4 ) - dPool;
        dBits = HX_MAX ( dBits, 0 );
        dBits = HX_MIN ( dBits, 200 );
        mnrp = ( int ) ( deltaNB * dBits );

        mnr0 =
            ( int ) ( 0.2 * deltaNB * HX_MAX ( ( minTargetBits - bits ), 0 ) );
        dmnr = mnr + mnr2 + mnrp - mnr0;
        maxdmnr = HX_MAX ( MNR - initialMNR, TargetBits >> 3 );    // limit for decrease 
        dmnr = HX_MIN ( dmnr, maxdmnr );
        if ( deltaMNR )
        {
            ddmnr = ( deltaMNR >> 1 );
            dmnr = HX_MAX ( dmnr, ddmnr );
        }
        MNR = MNR - dmnr;
        MNR = HX_MIN ( MNR, 2000 );        // for tones 
        if ( bits > ( TargetBits + 2000 ) )
            MNR = HX_MIN ( MNR, initialMNR );
    }

//if( block_type == 1 ) {
//    MNR = HX_MIN(MNR, initialMNR-100);
//    return;
//}

}

/*===============================================================*/
int
CBitAllo3::allocate (  )
{
    int ch;
    int bits, bits0;

//int i;

// moved
//if( MNR < -200 )  {   // re harp.wav
//    minTargetBits = HX_MAX(minTargetBits, (3*TargetBits)>>2);
//}

    if ( hf_flag )
    {
        hf_quant = 0;
        hf_quant_stereo[0] = hf_quant_stereo[1] = 0;
        gsf_hf_stereo[0] = gsf_hf_stereo[1] = -1;
        ixmax[0][21] = ixmax[1][21] = 0;
        clear_hf (  );
    }

    noise_seek_initial2 (  );

    noise_seek_actual (  );

// this may be ok re gspi35-2.wav -b64
    trade_dual (  );

    if ( hf_flag & 2 )
        hf_adjust (  );

    fnc_scale_factors (  );

    if ( hq_flag == 0 )
        lucky_noise (  );
    else
        big_lucky_noise (  );

//quant(gsf);
    quantB ( gsf );

    if ( hf_quant )
        quantBhf (  );

    bits0 = bits = count_bits_dual (  );
//bits0 = bits = count_bits2(3);  // do a faster version of this?

    if ( bits < minTargetBits )
    {
        if ( MNR < 2000 )
        {
            bits = increase_bits ( bits );
        }
    }

    if ( hf_flag )
    {
        hf_quant = 0;
        hf_quant_stereo[0] = hf_quant_stereo[1] = 0;
        gsf_hf_stereo[0] = gsf_hf_stereo[1] = -1;
        ixmax[0][21] = ixmax[1][21] = 0;
    }

    if ( bits > maxTargetBits )
    {
        clear_hf (  );
        bits = decrease_bits ( bits );
    }

    if ( bits > maxBits )
    {
        clear_hf (  );
        bits = limit_bits ( bits );
    }

    if ( bits > PART23 )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            if ( huff_bits[ch] > PART23 )
            {
                clear_hf (  );
                bits = limit_part23_bits (  );
                break;  // limit_part23 checks both channels
            }
        }
    }

    if ( hq_flag )
        inverse_sf2 (  );       // does not change audio bit count - do last

    cnt3++;
    cnt4 += bits;
    cnt5 = cnt4 / cnt3;

    return bits0;
}

/*---------------------------------------------------------------*/
int
CBitAllo3::allocate_ms (  )
{
    int ch;
    int bits;
    int bits0;

// moved
//if( MNR < -200 )  {   // re harp.wav
//    minTargetBits = HX_MAX(minTargetBits, (3*TargetBits)>>2);
//}

    if ( hf_flag )
    {
        hf_quant = 0;
        ixmax[0][21] = ixmax[1][21] = 0;
        gsf_hf = -1;
        clear_hf (  );  // clear both chans incase last igr was dual
    }

    noise_seek_initial2 (  );

    noise_seek_actual (  );

// ms_sparse for i<14 hurts tmpsax2
// 6/30/00 all sparse and trade functions dropped for gspi35_2.wav -b64
// ms_sparse hurts diff.wav, 
// sparse-quads and trades don't seem to make much difference
//ms_sparse();
//trade_side();
//trade_main();

    if ( hf_flag )
        hf_adjust_ms (  );

    fnc_scale_factors_ms (  );

    if ( hq_flag == 0 )
        lucky_noise (  );
    else
        big_lucky_noise (  );

//quant(gsf);
    quantB ( gsf );

    ixmax[0][21] = 0;   // ???
    if ( hf_quant )
        quantBhf_ms (  );

// drop for gspi35_2
//ms_sparse_quads();  // does own quantB10x in required region

    bits0 = bits = count_bits (  );
//bits0 = bits = count_bits2(3);  // do a faster version of this?

    if ( bits < minTargetBits )
    {
        if ( MNR < 2000 )
        {
            bits = increase_bits_ms ( bits );
        }
    }

// clear hf - no hf on decreases
    hf_quant = 0;
    ixmax[0][21] = 0;
    gsf_hf = -1;

    if ( bits > maxTargetBits )
    {
        clear_hf_main (  );
        bits = decrease_bits ( bits );
        //bits = decrease_bits_ms(bits);
    }

    if ( bits > maxBits )
    {
        clear_hf_main (  );
        bits = limit_bits ( bits );
    }

    if ( bits > PART23 )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            if ( huff_bits[ch] > PART23 )
            {
                clear_hf_main (  );
                bits = limit_part23_bits (  );
                break;  // limit_part23 checks both channels
            }
        }
    }

    if ( hq_flag )
        inverse_sf2 (  );       // does not change audio bit count - do last

    cnt3++;
    cnt4 += bits;
    cnt5 = cnt4 / cnt3;

    return bits0;
}

/*---------------------------------------------------------------*/
