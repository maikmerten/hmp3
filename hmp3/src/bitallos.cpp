/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallos.cpp,v 1.2 2005/08/09 20:43:41 karll Exp $ 
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

#include "bitallos.h"

extern "C"
{
    extern int iframe; // for testing
}

static int cnt1, cnt2, cnt3, cnt4, cnt5, cnt6, cnt7, cnt8, cnt9;// test vars
static int cnt10, cnt11, cnt12, cnt13, cnt14, cnt15, cnt16;     // test vars

static const int sf_limit[2][12] = {
    {   // scale = 0
        31, 31, 31, 31, 31, 31,
	15, 15, 15, 15, 15, 15,
    },
    {   // scale = 1 
        62, 62, 62, 62, 62, 62,    // use 62 & 30 or 63 & 31 ??
	30, 30, 30, 30, 30, 30,
    },
};

static int sf_upper_limit[2][12] = {    // limit[scale]
    {   // scale = 0,
        30, 30, 30, 30, 30, 30,
	14, 14, 14, 14, 14, 14,
    },
    {   // scale = 1 
        60, 60, 60, 60, 60, 60,
	28, 28, 28, 28, 28, 28,
    },
};

// ATTENTION this will be overwritten later
static int sf_lower_limit[12] = {       // with no preemp limit is 0
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
};

//----------

/*---------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
// Public Functions
///////////////////////////////////////////////////////////////////////////////
CBitAlloShort::CBitAlloShort (  ):
ms_count ( 0 ), ill_is_pos ( 7 ), int_dummy ( 0 )
{
    nsf[0] = nsf[1] = 12;

    memset ( xsxx, 0, sizeof ( xsxx ) );
    memset ( Noise0, 0, sizeof ( Noise0 ) );
    memset ( NT, 0, sizeof ( NT ) );
    memset ( Noise, 0, sizeof ( Noise ) );
    memset ( x34max, 0, sizeof ( x34max ) );
    memset ( ixmax, 0, sizeof ( ixmax ) );
    memset ( ix10xmax, 0, sizeof ( ix10xmax ) );
    memset ( gzero, 0, sizeof ( gzero ) );
    memset ( gmin, 0, sizeof ( gmin ) );
    memset ( gsf, 0, sizeof ( gsf ) );
    memset ( sf, 0, sizeof ( sf ) );
    memset ( G, 0, sizeof ( G ) );
    memset ( active_sf, 0, sizeof ( active_sf ) );
    memset ( x34, 0, sizeof ( x34 ) );
    memset ( ix, 0, sizeof ( ix ) );
    memset ( signx, 0, sizeof ( signx ) );
    memset ( ix10x, 0, sizeof ( ix10x ) );
    memset ( scalefactor_scale, 0, sizeof ( scalefactor_scale ) );
    memset ( snr, 0, sizeof ( snr ) );

}

CBitAlloShort::~CBitAlloShort (  )
{
}

/*===============================================================*/
void
CBitAlloShort::ba_out_stats (  )        // test routine
{
    cnt1 = call_count;
    cnt2 = ms_count;
    fprintf(stderr, "\n ba short %6d %6d %6d %6d %6d %6d %6d %6d %6d",
             cnt1, cnt2, cnt3, cnt4, cnt5, cnt6, cnt7, cnt8, cnt9 );
    fprintf(stderr, "\n ba short  cnt10  %6d %6d %6d %6d %6d %6d %6d",
             cnt10, cnt11, cnt12, cnt13, cnt14, cnt15, cnt16 );
}

/*===============================================================*/
int
CBitAlloShort::BitAlloInit ( BA_CONTROL & bac )
{
    int i, k, w;

    call_count = 0;
    ms_count = 0;
    huff_bits[0] = huff_bits[1] = 0;

    vbr_flag = bac.vbr_flag;
    h_id = bac.h_id;    // mpeg1=1
    is_flag = bac.is_flag;      // intensity stereo

    if ( h_id )
        ill_is_pos = 7;
    else
        ill_is_pos = 999;

// passed band_limit are in terms of long block bands
    L3init_gen_band_table_short ( nBand_s );
    nsf[0] = L3init_sfbs_limit2 ( bac.band_limit_left / 3 - 10 );
    nsf[1] = L3init_sfbs_limit2 ( bac.band_limit_right / 3 - 10 );

    for ( k = 0, i = 0; i < 13; i++ )
    {
        startBand_s[i] = k;
        k += nBand_s[i];
    }
    startBand_s[i] = k;

    nbmax3[0] = nbmax2[0] = nbmax[0] = startBand_s[nsf[0]];
    nbmax3[1] = nbmax2[1] = nbmax[1] = startBand_s[nsf[1]];

    for ( i = 0; i < 12; i++ )
    {
        look_log_cbwmb[i] =
            ( int ) ( 100.0f * dbLog ( ( float ) nBand_s[i] ) );
    }

    gz_con1B = 0.017716950f;    // gz_con1B from AAC 
    gz_con2B = ( 104.585000f - 100.0f + 8.0f ); // aac_g_off = 100, mp3_goff = 8

    for ( i = 0; i < 12; i++ )
        taperNT[i] = 0;
//if( h_id == 1 ) {      // mpeg1
//    for(i=6;i<12;i++) {
//        taperNT[i] = 100 + HX_MIN(150, 2*20*(i-6));
//        taperNT[i] = 0;
//    }
//}
//else {                // mpeg2 (provisons for)
//    for(i=6;i<12;i++) {
//        taperNT[i] = 100 + HX_MIN(150, 2*20*(i-6));
//        taperNT[i] = 0;
//    }
//}

// MNR initial MNR in mb, 1db = 100mb
    initialMNR = MNR = MNR + bac.MNRbias;

    for ( w = 0; w < 3; w++ )
    {
        for ( i = 0; i < 12; i++ )
        {
            active_sf[0][w][i] = active_sf[1][w][i] = 0;
            sf[0][w][i] = sf[1][w][i] = 0;
            ixmax[0][w][i] = ixmax[1][w][i] = 0;
        }
    }

    return 0;
}

//==================================================================
int
CBitAlloShort::BitAllo ( float xr_arg[][3][192],
                         SIG_MASK sm_arg[][3][12],
                         int ch_arg, int nchan_arg,
                         int min_bits_arg, int target_bits_arg,
                         int max_bits_arg, int bit_pool_arg,
                         SCALEFACT sf_out[], GR gr_data[], int ix_arg[][576],
                         unsigned char signx_arg[][576], int ms_flag_arg,
                         int MNR_arg )
{
    int ch, k, i, j, w, n1, n2, n;

    MNR = MNR_arg;      // mnr controlled by caller
    if ( h_id == 0 )
    {   // mpeg2
        MNR = HX_MIN ( MNR, 850 ); // limit mnr to avoid constant bit limiting 
    }

    call_count++;
    if ( ms_flag_arg )
        ms_count++;

    ms_flag = ms_flag_arg;
    xr = xr_arg;
    nchan = nchan_arg;

    maxBits = HX_MIN ( 4000 * nchan, max_bits_arg );       /* must not exceed */
    /* 4000*nchan is for part23 */
    minTargetBits = min_bits_arg;
    if ( minTargetBits < 0 )
        minTargetBits = 0;
    TargetBits = target_bits_arg;
    PoolBits = bit_pool_arg;

// if vbr, callers set TargetBits to max steady state bitrate
    maxTargetBits = TargetBits + ( ( 614 * PoolBits ) >> 10 );  // 2/2/99
// let shorts use more of max
    maxTargetBits = ( maxBits + maxTargetBits ) >> 1;

    maxTargetBits = HX_MIN ( maxBits, maxTargetBits );     // 1/28/99

    if ( ms_flag )
        startup_ms ( sm_arg, signx );
    else
        startup ( sm_arg, signx );

//-----------------------

/* null frame */
    if ( activeBands <= 0 )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            gr_data[ch].global_gain = 0;
            gr_data[ch].window_switching_flag = 1;
            gr_data[ch].block_type = 2;
            gr_data[ch].mixed_block_flag = 0;
            gr_data[ch].preflag = 0;
            gr_data[ch].scalefac_scale = 0;
            gr_data[ch].table_select[0] = 0;
            gr_data[ch].table_select[1] = 0;
            gr_data[ch].table_select[2] = 0;
            gr_data[ch].subblock_gain[0] = 0;
            gr_data[ch].subblock_gain[1] = 0;
            gr_data[ch].subblock_gain[2] = 0;
            gr_data[ch].big_values = 0;
            gr_data[ch].region0_count = 0;
            gr_data[ch].region1_count = 0;
            gr_data[ch].count1table_select = 0; // changed to 0 10/27/98
            gr_data[ch].aux_nquads = 0;
            gr_data[ch].aux_bits = 0;   /* zero huff bits */
            gr_data[ch].aux_not_null = 0;
            gr_data[ch].aux_nreg[0] = 0;
            gr_data[ch].aux_nreg[1] = 0;
            gr_data[ch].aux_nreg[2] = 0;
            for ( w = 0; w < 3; w++ )
            {
                for ( j = 0; j < 12; j++ )
                    sf_out[ch].s[w][j] = 0;
            }
        }
        FeedbackBits = 0;
        return FeedbackBits;
    }
//------------------------

    allocate (  );

/*------- return data --------*/

    if ( ms_flag )
    {   // compensate for lack of sqrt(2) scaling
        GG[0] -= 2;
        GG[1] -= 2;
    }

    GG[0] = HX_MAX ( GG[0], 0 );
    GG[1] = HX_MAX ( GG[1], 0 );

    for ( ch = 0; ch < nchan; ch++ )
    {
        gr_data[ch].global_gain = GG[ch] + ( 4 * 32 + 14 );
        if ( gr_data[ch].global_gain > 255 )
            gr_data[ch].global_gain = 255;
        gr_data[ch].window_switching_flag = 1;
        gr_data[ch].block_type = 2;
        gr_data[ch].mixed_block_flag = 0;
        gr_data[ch].preflag = 0;
        gr_data[ch].scalefac_scale = scalefactor_scale[ch];
        gr_data[ch].aux_bits = huff_bits[ch];   /* huff bits */
        gr_data[ch].aux_not_null = huff_bits[ch];       /* huff bits */
        gr_data[ch].subblock_gain[0] = subblock_gain[ch][0] >> 3;
        gr_data[ch].subblock_gain[1] = subblock_gain[ch][1] >> 3;
        gr_data[ch].subblock_gain[2] = subblock_gain[ch][2] >> 3;
        output_subdivide2 ( &gr_data[ch], ch );
    }

//gr_data[0].global_gain = HX_MAX(0, gr_data[0].global_gain-30);  // test test test
//gr_data[1].global_gain = HX_MAX(0, gr_data[1].global_gain-30);  // test test test

    if ( is_flag )
    {   // intensity - must encode right sf even if huff_bit = 0
        gr_data[1].aux_not_null = 1;
    }

// scale factors
    output_sf ( sf_out );

// spectral data - reordered for huff coding
    for ( ch = 0; ch < nchan; ch++ )
    {
        memset ( ix_arg[ch], 0, 576 * sizeof ( int ) );
        k = 0;
        // big region
        n = save[ch].cbreg[1];  // to end of big region
        for ( i = 0; i < n; i++ )
        {
            n1 = startBand_s[i];
            n2 = startBand_s[i + 1];
            for ( w = 0; w < 3; w++ )
            {
                for ( j = n1; j < n2; j++ )
                {
                    ix_arg[ch][k] = ix[ch][w][j];
                    signx_arg[ch][k] = signx[ch][w][j];
                    k++;
                }
            }
        }
        // quad region - 
        n = save[ch].cbreg[2];  // to end of quad region
        for ( ; i < n; i++ )
        {
            n1 = startBand_s[i];
            n2 = startBand_s[i + 1];
            for ( w = 0; w < 3; w++ )
            {
                for ( j = n1; j < n2; j++ )
                {
                    ix_arg[ch][k] = ix[ch][w][j];
                    signx_arg[ch][k] = signx[ch][w][j];
                    k++;
                }
            }
        }
    }

    return FeedbackBits;
}

///////////////////////////////////////////////////////////////////////////////
// Private Functions
///////////////////////////////////////////////////////////////////////////////

/*===============================================================*/
int
CBitAlloShort::ms_correlation2Short ( float x[2][3][192] )
{
    int i, j, k, n, w;
    float a, b;
    float s0, s1;
    int d;
    int r;

    d = 0;
    for ( w = 0; w < 3; w++ )
    {
        k = 0;
        for ( i = 0; i < nsf[0]; i++ )
        {
            n = nBand_s[i];
            s0 = s1 = 0.0f;
            for ( j = 0; j < n; j++, k++ )
            {
                a = x[0][w][k] * x[0][w][k];
                b = x[1][w][k] * x[1][w][k];
                s0 += ( a + b );
                a = ( float ) fabs ( a - b );
                s1 += a;
            }
            if ( s1 > 0.80 * s0 )
            {
                d++;
            }
            if ( s1 > 0.95 * s0 )
                d += 2;
        }
    }

    r = ( nsf[0] - d );

    r = r << 10;        // 10/4/00 scale to aprox magnitude of long 
    // for equal weight in mpeg-2 frames;
    return r;
}

/*---------------------------------------------------------------*/
void
CBitAlloShort::output_sf ( SCALEFACT sf_out[] )
{
    int i, ch, w;

/* convert to encoded sf */

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            /* adjust scaling */
            if ( scalefactor_scale[ch] == 0 )
            {
                for ( i = 0; i < nsf[ch]; i++ )
                    sf[ch][w][i] >>= 1;
            }
            else
            {
                for ( i = 0; i < nsf[ch]; i++ )
                    sf[ch][w][i] >>= 2;
            }
        }
    }

// for joint set is_pos in null sf bands
//if( is_flag ) {
//    for(i=nsf[1]-1;i>=0;i--) {
//        if( ixmax[1][i] > 0 ) break;
//        sf[1][i] = ill_is_pos;   // set illegal is_pos
//    }
//}

/* return scale factors, (sf[i] = 0 beyond nsf[ch] or joint is pos) */
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            for ( i = 0; i < 12; i++ )
                sf_out[ch].s[w][i] = sf[ch][w][i];
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAlloShort::startup ( SIG_MASK sm[][3][12], unsigned char signx[][3][192] )
{
    int i, n, ch, w;
    float *x, *y;
    unsigned char *s;
    int mask;
    int mnr;
    int tsnr;

    mnr = MNR;

// compute sign and xsxx
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            x = xr[ch][w];
            s = signx[ch][w];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                n = nBand_s[i];
                xsxx[ch][w][i] = vect_sign_sxx ( x, s, n );
                x += n;
                s += n;
            }
        }
    }

//-------------------------------------------
// compute Mask in db and NT (noise target)
    activeBands = 0;
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                Noise0[ch][w][i] =
                    mbLogB ( xsxx[ch][w][i] ) - look_log_cbwmb[i];
                if ( Noise0[ch][w][i] < -2000 )
                {
                    NT[ch][w][i] = Noise0[ch][w][i] + 1000;
                    snr[ch][w][i] = -1000;
                }
                else
                {
                    mask =
                        ( mbLogB ( sm[ch][w][i].mask ) - look_log_cbwmb[i] );
                    NT[ch][w][i] = mask - mnr + taperNT[i];     // noise target
                    // dropout prevention
                    tsnr = Noise0[ch][w][i] - NT[ch][w][i];
                    if ( tsnr < 300 )
                    {   // [-500,300] -> [0,300]
                        tsnr = 187 + ( ( 3 * tsnr ) >> 3 ) - tsnr;
                        NT[ch][w][i] -= tsnr;
                    }
                    snr[ch][w][i] = Noise0[ch][w][i] - NT[ch][w][i];
                    activeBands += nBand_s[i];
                }
            }
        }
    }

//-----------------
    startup_adjustNT (  );
//-----------------

//-------------------------------------------

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            vect_fpow34 ( xr[ch][w], x34[ch][w], nbmax3[ch] );
        }
    }

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            y = x34[ch][w];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                n = nBand_s[i];
                vect_fmax2 ( y, n, x34max[ch][w] + i );
                gzero[ch][w][i] =
                    HX_MAX ( 0,
                          round_to_int ( ( gz_con1B *
                                           mbLogC ( x34max[ch][w][i] ) +
                                           gz_con2B ) ) );
                gmin[ch][w][i] = HX_MAX ( 0, gzero[ch][w][i] - GMIN_OFFSET );
                y += n;
            }
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAlloShort::startup_ms ( SIG_MASK sm[][3][12],
                            unsigned char signx[][3][192] )
{
    int i, n, ch, w;
    float *x, *y;
    unsigned char *s;
    float sxx[4];       // left, right, sum, diff
    int maskL, maskR;
    int cbwmb;
    int xNT, Nsum, Ndiff;
    int Noise0L, Noise0R;
    int NTR, NTL;
    int tsnr;

    activeBands = 0;

    for ( w = 0; w < 3; w++ )
    {
        x = xr[0][w];
        s = signx[0][w];
        for ( i = 0; i < nsf[0]; i++ )
        {
            n = nBand_s[i];
            fnc_sxx ( x, n, sxx );
            fnc_ms_process2 ( x, n, s );        //no sqrt(2) scale
            fnc_sxx ( x, n, sxx + 2 );
            xsxx[0][w][i] = sxx[0];
            xsxx[1][w][i] = sxx[1];

            cbwmb = look_log_cbwmb[i];
            Noise0L = mbLogB ( sxx[0] ) - cbwmb;
            if ( Noise0L < -2000 )
                NTL = 10000;
            else
            {
                maskL = ( mbLogB ( sm[0][w][i].mask ) - cbwmb );
                NTL = maskL - MNR + taperNT[i]; // noise target left

                // dropout prevention
                tsnr = Noise0L - NTL;
                if ( tsnr < 300 )
                {       // [-500,300] -> [0,300]
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
                maskR = ( mbLogB ( sm[1][w][i].mask ) - cbwmb );
                NTR = maskR - MNR + taperNT[i]; // noise target right
                // dropout prevention
                tsnr = Noise0R - NTR;
                if ( tsnr < 300 )
                {       // [-500,300] -> [0,300]
                    tsnr = 187 + ( ( 3 * tsnr ) >> 3 ) - tsnr;
                    NTR -= tsnr;
                }

                activeBands += n;
            }

            Noise0[0][w][i] = Nsum = mbLogB ( sxx[2] ) - cbwmb;
            Noise0[1][w][i] = Ndiff = mbLogB ( sxx[3] ) - cbwmb;

            xNT = HX_MIN ( NTR, NTL ) + 300;       // 300 is scaleing for ms sqrt(2)
            NT[1][w][i] = NT[0][w][i] = xNT;
            if ( Ndiff < xNT )
            {
                NT[0][w][i] = LogSubber ( xNT, Ndiff );
                NT[0][w][i] -= 200;
            }
            if ( Nsum < xNT )
            {
                NT[1][w][i] = LogSubber ( xNT, Nsum );
                NT[1][w][i] -= 200;
            }

            snr[0][w][i] = Noise0[0][w][i] - NT[0][w][i];
            snr[1][w][i] = Noise0[1][w][i] - NT[1][w][i];

            x += n;
            s += n;
        }
    }

//-----------------
    startup_adjustNT (  );
//-----------------

    for ( w = 0; w < 3; w++ )
    {
        vect_fpow34 ( xr[0][w], x34[0][w], nbmax2[0] );
        vect_fpow34 ( xr[1][w], x34[1][w], nbmax2[1] );
    }

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            y = x34[ch][w];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                n = nBand_s[i];
                vect_fmax2 ( y, n, x34max[ch][w] + i );
                gzero[ch][w][i] =
                    HX_MAX ( 0,
                          round_to_int ( ( gz_con1B *
                                           mbLogC ( x34max[ch][w][i] ) +
                                           gz_con2B ) ) );
                gmin[ch][w][i] = HX_MAX ( 0, gzero[ch][w][i] - GMIN_OFFSET );
                y += n;
            }
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAlloShort::startup_adjustNT (  )
{
    int i, w, ch, na, a;

//int ab, nab;

    na = 1;
    a = 0;

//nab = 1;
//ab = 0;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( snr[ch][w][i] > 0 )
                {
                    a += NT[ch][w][i];
                    na++;
                    //ab += nBand_s[i]*NT[ch][w][i];
                    //nab+= nBand_s[i];
                }
            }
        }
    }

    a = a / na;
//ab = ab/nab;
    if ( a <= 500 )
        return;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( snr[ch][w][i] > 0 )
                {
                    NT[ch][w][i] = ( NT[ch][w][i] + a ) >> 1;
                }
            }
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAlloShort::noise_seek_initial2 (  )
{
    int i, ch, w;
    float g4, d, g;

// 
// g4: gsf such that max quantized value = 4
// derived for aac g_offset = 100, mp3 g_offset = 8  adjust by -100+8
//

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                g4 = gz_con1B * mbLogC ( x34max[ch][w][i] ) + ( 88.411238f - 100.0f + 8.0f );   // g4 w/g_offset
                d = ( 1.00f / 110.5f ) * ( 1800 - ( 2 * 8 ) * i -
                                           ( Noise0[ch][w][i] -
                                             NT[ch][w][i] ) );
                g = g4 + d;
                gsf[ch][w][i] = round_to_int ( g );
                gsf[ch][w][i] = HX_MIN ( gsf[ch][w][i], gzero[ch][w][i] );
                gsf[ch][w][i] = HX_MAX ( gsf[ch][w][i], gmin[ch][w][i] );
            }
        }
    }

}

/*===============================================================*/
int
CBitAlloShort::decrease_noise ( int s0, int n )
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
CBitAlloShort::increase_noise ( int s0, int n )
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
CBitAlloShort::noise_seek_actual (  )
{
    int i, ch, w;
    int s, n;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            y34 = x34[ch][w];
            y = xr[ch][w];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                NTarget = NT[ch][w][i];
                n = nBand_s[i];
                s = gsf[ch][w][i];
                if ( Noise0[ch][w][i] > NTarget )
                {
                    logn = look_log_cbwmb[i];
                    noise = ifnc_noise_actual ( y34, y, s, n, logn );
                    dn = noise - NTarget;
                    if ( dn > 100 )
                    {
                        s = decrease_noise ( s, n );
                    }
                    else if ( dn < -100 )
                    {
                        s = increase_noise ( s, n );
                    }
                    gsf[ch][w][i] = s;
                    Noise[ch][w][i] = noise;
                }
                else
                {
                    gsf[ch][w][i] = gzero[ch][w][i] + 5;
                    Noise[ch][w][i] = Noise0[ch][w][i];
                }

                y34 += n;
                y += n;
            }
        }
    }

}

/*===============================================================*/
void
CBitAlloShort::quant (  )
{
    int ch, i, n, w;
    float *x;
    int *qx;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            x = x34[ch][w];
            qx = ix[ch][w];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                n = nBand_s[i];
                ixmax[ch][w][i] = vect_quant ( x, qx, gsf[ch][w][i], n );
                x += n;
                qx += n;
            }
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAlloShort::quantB (  )
{
    int ch, i, n, w;
    float *x;
    int *qx;

// optimum quantizer

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            x = x34[ch][w];
            qx = ix[ch][w];
            for ( i = 0; i < nsf[ch]; i++ )
            {
                n = nBand_s[i];
                ixmax[ch][w][i] =
                    vect_quantB2 ( x, qx, gsf[ch][w][i], n, -.30f );
                //ixmax[ch][w][i] = vect_quantB(x, qx, gsf[ch][w][i], n);
                x += n;
                qx += n;
            }
        }
    }

}

/*===============================================================*/
int
CBitAlloShort::count_bits (  )
{
    int ch, bits;

    for ( bits = 0, ch = 0; ch < nchan; ch++ )
    {
        huff_bits[ch] = subdivide2 ( ixmax[ch], ix[ch], nsf[ch], ch );
        bits += huff_bits[ch];
    }
    return bits;
}

/*===============================================================*/
void
CBitAlloShort::fnc_sf_final ( int ch )
{
    int i, w;
    int scale_flag;
    int sp0;

// determine if sf scale required

    sp0 = 0;
    for ( i = 0; i < nsf[ch]; i++ )
    {
        for ( w = 0; w < 3; w++ )
        {
            if ( active_sf[ch][w][i] )
            {
                sp0 |= ( sf_limit[0][i] - sf[ch][w][i] );
            }
        }
    }

    scale_flag = 1;     // assume scale flag needed (could be out of range also)
    if ( sp0 >= 0 )
    {
        scale_flag = 0;
    }

    scalefactor_scale[ch] = scale_flag;

}

/*----------------------------------------------------------------*/
void
CBitAlloShort::fnc_scale_factors01 (  )
{
    int i, ch, w;
    int Gtmp;
    int s, d, dN;

// routine used by V5T2 (tested by Neal) and V5T3

    for ( ch = 0; ch < nchan; ch++ )
    {
        scalefactor_scale[ch] = 0;
        for ( w = 0; w < 3; w++ )
        {
            /* compute G */
            Gtmp = -1;
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( Noise0[ch][w][i] <= NT[ch][w][i] )
                {       // not active sf
                    active_sf[ch][w][i] = 0;
                    gsf[ch][w][i] = gzero[ch][w][i];
                }
                else
                {
                    active_sf[ch][w][i] = -1;   // -1 provision for branchless logic
                    gsf[ch][w][i] = HX_MAX ( gsf[ch][w][i], gmin[ch][w][i] );      // quant overflow protect
                    Gtmp = HX_MAX ( Gtmp, gsf[ch][w][i] );
                }
            }
            G[ch][w] = Gtmp;    // neg value will flag null w group
        }       // w loop

        GG[ch] = HX_MAX ( G[ch][0], G[ch][1] );
        GG[ch] = HX_MAX ( GG[ch], G[ch][2] );

        for ( w = 0; w < 3; w++ )
        {
            Gtmp = G[ch][w];
            if ( Gtmp < 0 )
            {   /* null w group for ch */
                subblock_gain[ch][w] = 0;
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    sf[ch][w][i] = 0;
                    gsf[ch][w][i] = gzero[ch][w][i];
                }
            }
            else
            {   /* not null w group */
                subblock_gain[ch][w] = ( GG[ch] - Gtmp ) & ( ~7 );
                subblock_gain[ch][w] = HX_MIN ( subblock_gain[ch][w], 7 * 8 );     // 3 bit code 
                Gtmp = GG[ch] - subblock_gain[ch][w];
                G[ch][w] = Gtmp;
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    sf[ch][w][i] = 0;
                    if ( active_sf[ch][w][i] )
                        sf[ch][w][i] = Gtmp - gsf[ch][w][i];
                }
            }
        }       // w loop

        /* choose scalefactor_scale  */
        fnc_sf_final ( ch );

        if ( scalefactor_scale[ch] == 0 )
        {
            for ( w = 0; w < 3; w++ )
            {
                if ( G[ch][w] < 0 )
                    continue;   // null group
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    //if( (i<6) && (Noise[ch][w][i] > NT[ch][w][i]) ) {
                    if ( ( Noise[ch][w][i] > NT[ch][w][i] ) )
                    {
                        sf[ch][w][i]++;
                    }
                    sf[ch][w][i] = HX_MIN ( G[ch][w], sf[ch][w][i] );      // prevent negative gsf
                    sf[ch][w][i] &= ( ~1 );

                }
            }
        }
        else
        {       //  scalefactor_scale[ch] == 1
            for ( w = 0; w < 3; w++ )
            {
                if ( G[ch][w] < 0 )
                    continue;   // null group
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    s = sf[ch][w][i] & ( ~3 );
                    d = sf[ch][w][i] - s;
                    dN = Noise[ch][w][i] - NT[ch][w][i] + 150 * d;
                    if ( ( dN > 250 ) )
                    {
                        s = s + 4;
                        s = HX_MIN ( G[ch][w], s ) & ( ~3 );       // prevent negative gsf
                    }
                    sf[ch][w][i] = s;
                }
            }
        }

        //apply hard limits
        //save limit pointers for possible limiting later
        psf_upper_limit[ch] = sf_upper_limit[scalefactor_scale[ch]];
        for ( w = 0; w < 3; w++ )
        {
            if ( G[ch][w] >= 0 )
            {   // not a null group
                vect_limits ( sf[ch][w], psf_upper_limit[ch], sf_lower_limit,   // this all 0 for short
                              nsf[ch] );
            }
        }

        /* back compute gsf */
        for ( w = 0; w < 3; w++ )
        {
            if ( G[ch][w] < 0 )
                continue;       // null w group, skip
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( active_sf[ch][w][i] )
                {
                    gsf[ch][w][i] = G[ch][w] - sf[ch][w][i];
                    if ( gsf[ch][w][i] >= gzero[ch][w][i] )
                    {
                        gsf[ch][w][i] = gzero[ch][w][i];
                        sf[ch][w][i] = 0;
                    }
                }
            }
        }

    }   // end ch loop 

}

/*----------------------------------------------------------------*/
void
CBitAlloShort::fnc_scale_factors (  )
{
    int i, ch, w;
    int Gtmp;
    int s, d, dN;

// upgrade of v5t2 function

    for ( ch = 0; ch < nchan; ch++ )
    {
        scalefactor_scale[ch] = 0;
        for ( w = 0; w < 3; w++ )
        {
            /* compute G */
            Gtmp = -1;
            for ( i = 0; i < nsf[ch]; i++ )
            {
                gsf[ch][w][i] = HX_MAX ( gsf[ch][w][i], gmin[ch][w][i] );  // quant overflow protect
                active_sf[ch][w][i] = 0;
                if ( gsf[ch][w][i] < gzero[ch][w][i] )
                {
                    active_sf[ch][w][i] = -1;   // -1 provision for branchless
                    Gtmp = HX_MAX ( Gtmp, gsf[ch][w][i] );
                }
            }
            G[ch][w] = Gtmp;    // neg value will flag null w group
        }       // w loop

        GG[ch] = HX_MAX ( G[ch][0], G[ch][1] );
        GG[ch] = HX_MAX ( GG[ch], G[ch][2] );

        for ( w = 0; w < 3; w++ )
        {
            Gtmp = G[ch][w];
            if ( Gtmp < 0 )
            {   /* null w group for ch */
                subblock_gain[ch][w] = 0;
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    sf[ch][w][i] = 0;
                    gsf[ch][w][i] = gzero[ch][w][i];
                }
            }
            else
            {   /* not null w group */
                subblock_gain[ch][w] = ( GG[ch] - Gtmp ) & ( ~7 );
                subblock_gain[ch][w] = HX_MIN ( subblock_gain[ch][w], 7 * 8 );     // 3 bit code 
                Gtmp = GG[ch] - subblock_gain[ch][w];
                G[ch][w] = Gtmp;
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    sf[ch][w][i] = 0;
                    if ( active_sf[ch][w][i] )
                        sf[ch][w][i] = Gtmp - gsf[ch][w][i];
                }
            }
        }       // w loop

        /* choose scalefactor_scale  */
        fnc_sf_final ( ch );

        if ( scalefactor_scale[ch] == 0 )
        {
            for ( w = 0; w < 3; w++ )
            {
                if ( G[ch][w] < 0 )
                    continue;   // null group
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    if ( ( Noise[ch][w][i] > NT[ch][w][i] ) )
                    {
                        sf[ch][w][i]++;
                    }
                    sf[ch][w][i] = HX_MIN ( G[ch][w], sf[ch][w][i] );      // prevent negative gsf
                    sf[ch][w][i] &= ( ~1 );

                }
            }
        }
        else
        {       //  scalefactor_scale[ch] == 1
            for ( w = 0; w < 3; w++ )
            {
                if ( G[ch][w] < 0 )
                    continue;   // null group
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    s = sf[ch][w][i] & ( ~3 );
                    d = sf[ch][w][i] - s;
                    dN = Noise[ch][w][i] - NT[ch][w][i] + 150 * d;
                    if ( ( dN > 250 ) )
                    {
                        s = s + 4;
                        s = HX_MIN ( G[ch][w], s ) & ( ~3 );       // prevent negative gsf
                    }
                    sf[ch][w][i] = s;
                }
            }
        }

        //apply hard limits
        //save limit pointers for possible limiting later
        psf_upper_limit[ch] = sf_upper_limit[scalefactor_scale[ch]];
        for ( w = 0; w < 3; w++ )
        {
            if ( G[ch][w] >= 0 )
            {   // not a null group
                vect_limits ( sf[ch][w], psf_upper_limit[ch], sf_lower_limit,   // this all 0 for short
                              nsf[ch] );
            }
        }

        /* back compute gsf */
        for ( w = 0; w < 3; w++ )
        {
            if ( G[ch][w] < 0 )
                continue;       // null w group, skip
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( active_sf[ch][w][i] )
                {
                    gsf[ch][w][i] = G[ch][w] - sf[ch][w][i];
                    if ( gsf[ch][w][i] >= gzero[ch][w][i] )
                    {
                        gsf[ch][w][i] = gzero[ch][w][i];
                        sf[ch][w][i] = 0;
                    }
                }
            }
        }

    }   // end ch loop 

}

/*===============================================================*/
int
CBitAlloShort::increase_bits ( int bits0 )
{
    int i, k, ch, w;
    int bits;

    for ( k = 0; k < 10; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( w = 0; w < 3; w++ )
            {
                for ( i = 0; i < nsf[ch]; i++ )
                    gsf[ch][w][i] = HX_MAX ( gsf[ch][w][i] - 1, 0 );
            }
        }
        fnc_scale_factors (  );
        quantB (  );
        bits = count_bits (  );
        if ( bits >= minTargetBits )
            break;
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAlloShort::limit_bits ( int bits0 )
{
    int i, k, ch, w;
    int bits;

// used if decrease bits not able to bring into range

    for ( k = 0; k < 100; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( w = 0; w < 3; w++ )
            {
                for ( i = 0; i < nsf[ch]; i++ )
                    gsf[ch][w][i] = HX_MIN ( 127, gsf[ch][w][i] + 1 );
            }
        }
        fnc_scale_factors (  );
        quant (  );
        bits = count_bits (  );
        if ( bits <= maxBits )
            break;
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAlloShort::limit_part23_bits (  )
{
    int i, k, ch, w;
    int bits;

    for ( k = 0; k < 100; k++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            if ( huff_bits[ch] > PART23 )
            {
                for ( w = 0; w < 3; w++ )
                {
                    for ( i = 0; i < nsf[ch]; i++ )
                        gsf[ch][w][i] = HX_MIN ( 127, gsf[ch][w][i] + 1 );
                }
            }
        }
        fnc_scale_factors (  );
        quant (  );
        bits = count_bits (  );
        if ( ( huff_bits[0] <= PART23 ) && ( huff_bits[1] <= PART23 ) )
            break;
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAlloShort::decrease_bits01 ( int bits0 )
{
    int i, k, ch, w;
    int bits;
    int thres;
    int deltaN;

    deltaN =
        round_to_int ( 75.0f *
                       ( 150.0f / ( 0.20f * ( activeBands + 10 ) ) ) );
    deltaN = HX_MIN ( deltaN, 200 );
    deltaN = HX_MAX ( deltaN, 40 );

    thres = maxTargetBits;

    deltaMNR = 0;

    for ( k = 0; k < 10; k++ )
    {
        deltaMNR += deltaN;
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( w = 0; w < 3; w++ )
            {
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    NT[ch][w][i] += deltaN;
                }
            }
        }
        noise_seek_actual (  );
        fnc_scale_factors (  );
        quant (  );
        bits = count_bits (  );
        if ( bits <= thres )
            break;
    }

    return bits;
}

/*---------------------------------------------------------------*/
int
CBitAlloShort::decrease_bits ( int bits0 )
{
    int i, k, ch, w;
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
            for ( w = 0; w < 3; w++ )
            {
                for ( i = 0; i < nsf[ch]; i++ )
                {
                    NT[ch][w][i] += deltaN;
                }
            }
        }
        noise_seek_actual (  );
        fnc_scale_factors (  );
        quant (  );
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
CBitAlloShort::allocate (  )
{
    int ch;
    int bits;
    int bits0;

    if ( MNR < -200 )
    {
        minTargetBits = HX_MAX ( minTargetBits, ( 3 * TargetBits ) >> 2 );
    }

    noise_seek_initial2 (  );
    noise_seek_actual (  );
    fnc_scale_factors (  );
//quant();
    quantB (  );

    FeedbackBits = bits0 = bits = count_bits (  );

    if ( bits < minTargetBits )
    {
        bits = increase_bits ( bits );
        cnt9++;
    }

    if ( bits > maxTargetBits )
    {
        bits = decrease_bits ( bits );
        cnt7++;
    }

    if ( bits > maxBits )
    {
        bits = limit_bits ( bits );
        cnt8++;
    }

    if ( bits > PART23 )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            if ( huff_bits[ch] > PART23 )
            {
                bits = limit_part23_bits (  );
                break;  // limit_part23 checks both channels
            }
        }
    }

    cnt3++;
    cnt4 += bits;
    cnt5 = cnt4 / cnt3;

}

/*---------------------------------------------------------------*/
