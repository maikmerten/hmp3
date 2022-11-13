/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: bitallo1.cpp,v 1.2 2005/08/09 20:43:41 karll Exp $ 
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

#include "bitallo1.h"

#define g_offset 8
// delta from gzero to gmin,
// theoretical value is 73+, 70 allows for rounding in gsf -> sf
#define GMIN_OFFSET 70

// L3math.c needs these (global) - computed by init, but fixed constants
// C naming convention, cannot be CPP or name mangled
extern "C"
{
    extern float look_gain[128];
    extern float look_34igain[128];
    extern float look_ix43[256];
}

static const int pretable[21] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 2,
    2, 3, 3, 3, 2
};

// ATTENTION -- this table can be overwritten later.
static float Ssb[21] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.10f, 0.10f, 0.10f, 0.10f, 0.10f, 0.10f,
    0.20f, 0.30f, 0.40f, 0.50f, 0.60f, 0.70f, 0.80f, 0.90f, 1.0f, 1.5f,
};
static const float sparse_table_MPEG1[21] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.10f, 0.10f, 0.10f, 0.10f, 0.10f, 0.10f,
    0.20f, 0.30f, 0.40f, 0.50f, 0.60f, 0.70f, 0.80f, 0.90f, 1.0f, 1.5f,
};
static const float sparse_table_MPEG2[21] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.10f, 0.10f, 0.10f, 0.10f, 0.10f, 0.10f,
    0.20f, 0.30f, 0.40f, 0.50f, 0.50f, 0.60f, 0.70f, 0.80f, 0.90f,
};

static const int pre2[21] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    15 + 2 * 1, 15 + 2 * 1, 15 + 2 * 1, 15 + 2 * 1, 15 + 2 * 2,
    15 + 2 * 2, 15 + 2 * 3, 15 + 2 * 3, 15 + 2 * 3, 15 + 2 * 2
};

static const int pre4[21] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    31 + 4 * 1, 31 + 4 * 1, 31 + 4 * 1, 31 + 4 * 1, 31 + 4 * 2,
    31 + 4 * 2, 31 + 4 * 3, 31 + 4 * 3, 31 + 4 * 3, 31 + 4 * 2
};

// function_noise_cb selectors
#define i_fnc_noise_cb   0
#define i_fnc_noise2_cb  1

///////////////////////////////////////////////////////////////////////////////
// Public Functions
///////////////////////////////////////////////////////////////////////////////
CBitAllo1::CBitAllo1 (  ):CBitAllo (  ), ill_is_pos ( 7 ), int_dummy ( 0 )
{
    nsf[0] = nsf[1] = 21;       // in base class
}

CBitAllo1::~CBitAllo1 (  )
{
}

/*===============================================================*/
int
CBitAllo1::BitAlloInit ( BA_CONTROL & bac )
{
    int i, j, k;

    h_id = bac.h_id;    // mpeg1=1
    is_flag = bac.is_flag;      // intensity stereo

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
    nsf[0] = L3init_sfbl_limit2 ( bac.band_limit_left );
    nsf[1] = L3init_sfbl_limit2 ( bac.band_limit_right );

    nsfb_s = L3init_sfbs_limit (  );

    for ( k = 0, i = 0; i < 21; i++ )
    {
        startBand_l[i] = k;
        k += nBand_l[i];
    }
    startBand_l[i] = k;

    for ( k = 0, i = 0; i < 12; i++ )
    {
        startBand_s[i] = k;
        k += nBand_s[i];
    }
    startBand_s[i] = k;

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
        look_log_cbw[i] = ( float ) ( 10.0 * log10 ( (double) nBand_l[i] ) );

    gen_noise_estimator (  );
    gen_bit_estimator (  );

    bitadjust = -100;
    bitadjust_save[1] = bitadjust_save[0] = -100;

    dBG = 0.25f * startBand_l[nsf[0]];
    dGdB = 1.0f / dBG;

/*--------
gzero = 1 + 16/(3*ln(2))*(ln(x34max)-ln(.5946)) + g_offset;
      = con1*ln(x34max) + con2
-------------*/
    gz_con1 = ( float ) ( 16.0 / ( 3.0 * log ( 2.0 ) ) );
    gz_con2 =
        ( float ) ( 1 - ( 16.0 / ( 3.0 * log ( 2.0 ) ) ) * log ( .5946 ) +
                    g_offset );
    gz_con0 = ( float ) ( exp ( ( 0.99 - gz_con2 ) / gz_con1 ) );

    for ( i = 0; i < 2; i++ )
    {
        for ( j = 0; j < nsf[i]; j++ )
            gsf[i][j] = gsf_save[i][j] = 35;
    }

    call_count = 0;
    running_a = ( 1.0f / 20.0f );

    gen_atan (  );      // lookup for joint is position 

    for ( i = 0; i < 21; i++ )
        sf[0][i] = sf[1][i] = 0;

    con707 = ( float ) ( 1.0 / sqrt ( 2.0 ) );

    if ( h_id )
        memmove ( Ssb, sparse_table_MPEG1, sizeof ( Ssb ) );
    else
        memmove ( Ssb, sparse_table_MPEG2, sizeof ( Ssb ) );

    ave_alpha_nmr = 40.0f;

    return nsf[1];      // for use by pack sf mpeg2
}

//==================================================================
void
CBitAllo1::BitAllo ( float xr_arg[][576], SIG_MASK sm_arg[][36],
                     int ch_arg, int nchan_arg,
                     int min_bits_arg, int target_bits_arg, int max_bits_arg,
                     int bit_pool_arg,
                     SCALEFACT sf_out[], GR gr_data[],
                     int ix_arg[][576],
                     unsigned char signx[][576], int ms_flag_arg )

/* nchan_agr = chans to allo, can be 1 for stereo */
{
    int i, j, ch;

//static int lbits, rbits, rbits2;

    ms_flag = ms_flag_arg;
    xr = xr_arg;
    ix = ix_arg;

/* TO DO: do this at init time */
    if ( nchan_arg == 1 )
    {
        dBG = 0.25f * startBand_l[nsf[0]];
    }
    else
    {
        dBG = 0.25f * ( startBand_l[nsf[0]] + startBand_l[nsf[1]] );
    }
    dGdB = 1.0f / dBG;

    nchan = nchan_arg;
    if ( nchan_arg == 1 )
        bitadjust = bitadjust_save[ch_arg];

/*----------------------*/
    max_bits = max_bits_arg;    /* must not exceed */
    min_bits = min_bits_arg;
    if ( min_bits < 0 )
        min_bits = 0;
//target_bits = target_bits_arg;
    target_bits = target_bits_arg - ( target_bits_arg >> 4 );
    if ( target_bits < min_bits )
        target_bits = min_bits;

// computes signx - may modify xr
    if ( is_flag == 0 )
        smr_adj ( sm_arg, signx );
    else
        smr_adj_joint ( sm_arg, signx );

    compute_x34 (  );

/* null frame */

/* TO DO - don't compute x34mm, scan x34max for > 3.0 */
    if ( x34mm < 3.0f )
    {
        for ( i = 0; i < nchan; i++ )
        {
            gr_data[i].global_gain = 0;
            gr_data[i].window_switching_flag = 0;
            gr_data[i].block_type = 0;
            gr_data[i].mixed_block_flag = 0;
            gr_data[i].preflag = 0;
            gr_data[i].scalefac_scale = 0;
            gr_data[i].table_select[0] = 0;
            gr_data[i].table_select[1] = 0;
            gr_data[i].table_select[2] = 0;
            gr_data[i].big_values = 0;
            gr_data[i].region0_count = 0;
            gr_data[i].region1_count = 0;
            gr_data[i].count1table_select = 0;  // =0 12/15/98
            gr_data[i].aux_nquads = 0;
            gr_data[i].aux_bits = 0;    /* zero huff bits */
            gr_data[i].aux_not_null = 0;        /* null frame */
            gr_data[i].aux_nreg[0] = 0;
            gr_data[i].aux_nreg[1] = 0;
            gr_data[i].aux_nreg[2] = 0;
            for ( j = 0; j < 21; j++ )
                sf_out[i].l[j] = 0;
        }
        return;
    }

    call_count++;
    if ( call_count <= 20 )
        running_a = 1.0f / call_count;

/*--- target bit computation ------*/
//max_cnt_bits = target_bits + 0.5*extra_avail;
    max_cnt_bits = max_bits;

/* allocate extra if bit pool almost full */

/* why care? can always increase final if going to waste */
//extra_avail = max_bits - target_bits;
//extra = extra_avail - 0.75*(max_bits - min_bits);
//if( extra > 0 ) {
//    target_bits += extra>>1;
//    printf("\n avail extra %3d, %3d ", extra_avail, extra);
//}

    if ( target_bits < min_bits )
    {
        target_bits = min_bits;
    }

//target0_min = (target_bits + min_bits) >> 1;
//target0_min = min_bits;
    target0_min = target_bits >> 1;
    if ( target0_min < min_bits )
        target0_min = min_bits;
    target0_max = ( target_bits + max_bits ) >> 1;

    if ( bitadjust > ( target_bits >> 1 ) )
        bitadjust = target_bits >> 1;
    target0_bits = target_bits - bitadjust;
    target0_min -= bitadjust;
    target0_max -= bitadjust;

/* initial gsf */

/* does this help - just fixed gsf = 35 seems better or just as good */

    if ( nchan_arg == 1 )
    {
        for ( i = 0; i < nsf[0]; i++ )
        {
            gsf[0][i] = gsf_save[ch_arg][i];
            if ( gsf[0][i] > gzero[0][i] )
                gsf[0][i] = gzero[0][i];
        }
    }
    else
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                if ( gsf[ch][i] > gzero[ch][i] )
                    gsf[ch][i] = gzero[ch][i];
            }
        }
    }

/* if nchan = 2 then gsf already has last value */

    counted_bits = allo_2 (  );

    ave_alpha_nmr = ave_alpha_nmr + running_a * ( alpha_nmr - ave_alpha_nmr );

/*------- return data --------*/
    output_sf ( sf_out );

    for ( i = 0; i < nchan; i++ )
    {
        gr_data[i].global_gain = G[i] + ( 4 * 32 + 14 );
        if ( gr_data[i].global_gain > 255 )
            gr_data[i].global_gain = 255;
        gr_data[i].window_switching_flag = 0;
        gr_data[i].block_type = 0;
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

/* save gsf for next time start values */
    if ( nchan_arg == 1 )
    {
        for ( i = 0; i < nsf[0]; i++ )
            gsf_save[ch_arg][i] = gsf[0][i];
    }

    if ( nchan_arg == 1 )
        bitadjust_save[ch_arg] = bitadjust;

    return;
}

/*===============================================================*/
int
CBitAllo1::ms_correlation2 ( float x[2][576], int block_type )
{
    int i, j, k, n;
    float a, b;
    float s0, s1;
    int d;
    int r;

//if( block_type == 2 ) {
//    return BitAlloShort.ms_correlation2Short((float (*)[3][192])xr_arg);
//}

    r = 0;
    d = 0;
    k = 0;
    for ( i = 0; i < nsf[0]; i++ )
    {
        n = nBand_l[i];
        s0 = s1 = 0.0f;
        for ( j = 0; j < n; j++, k++ )
        {
            a = x[0][k] * x[0][k];
            b = x[1][k] * x[1][k];
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
        //if( s1 > 0.99*s0 ) d+=4;
    }

    r = ( nsf[0] - 3 * d );

    return r;
}

/*===============================================================*/
///////////////////////////////////////////////////////////////////////////////
// Private Functions
///////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------*/
void
CBitAllo1::gen_atan (  )
{
    int i, k;
    double pi;

// gen is_pos lookup

    if ( h_id )
    {   // mpeg 1
        pi = 4.0 * atan ( 1.0 );
        // truncate for slight bias toward more channel separation
        for ( i = 0; i < 34; i++ )
        {
            //look_is_pos[i] = ((12.0/pi)*atan(sqrt(i/32.0))) + 0.5;
            look_is_pos[i] =
                ( int ) ( ( ( 12.0 / pi ) * atan ( sqrt ( i / 32.0 ) ) ) +
                          .25 );
        }
    }
    else
    {   // mpeg 2
        for ( i = 0; i < 34; i++ )
        {
            k = ( int ) ( -log ( ( i + .0001 ) / 32.0 ) / log ( 2.0 ) + 0.5 );
            if ( k < 0 )
                k = 0;
            if ( k > 3 )
                k = 3;
            look_is_pos[i] = k + k;
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo1::gen_bit_estimator (  )
{
    int ixm;

    look_bits[0] = 0;
    for ( ixm = 1; ixm < 256; ixm++ )
    {
        /* mag + sign */
        look_bits[ixm] =
            ( int ) ( 16 *
                      ( 1.4427 * log ( (double)(ixm + 1) ) + ( ixm - 0.6 ) / ixm ) );
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo1::gen_noise_estimator (  )
{
    int i, ix;
    double x0, xh, xl;
    double eps, t;
    double dh, dl;
    double cum;
    double ave_noise;

/* estimate ave noise as function of ixmax */

    cum = 0.0f;
    for ( ix = 0; ix < 256; ix++ )
    {
        t = ix + 0.5;
        xh = t * pow ( t, 1.0 / 3.0 );
        t = ix;
        x0 = t * pow ( t, 1.0 / 3.0 );
        t = ix - 0.5;
        xl = t * pow ( fabs ( t ), 1.0 / 3.0 );
        dh = xh - x0;
        dl = xl - x0;
        eps = ( dh * dh * dh - dl * dl * dl ) / ( 3.0 * ( xh - xl ) );
        cum += eps;
        ave_noise = cum / ( ix + 1 );
        look_f_ix[ix] = ( float ) eps;
        look_f_ixmax[ix] = ( float ) ( 10.0 * log10 ( ave_noise ) );
    }

    cum = 0.0;
    for ( i = 0; i < 256; i++ )
    {
        ix = 32 * i + 16;
        t = ix + 0.5;
        xh = t * pow ( t, 1.0 / 3.0 );
        t = ix;
        x0 = t * pow ( t, 1.0 / 3.0 );
        t = ix - 0.5;
        xl = t * pow ( fabs ( t ), 1.0 / 3.0 );
        dh = xh - x0;
        dl = xl - x0;
        eps = ( dh * dh * dh - dl * dl * dl ) / ( 3.0 * ( xh - xl ) );
        cum += eps;
        ave_noise = cum / ( i + 1 );
        look_f_big_ix[i] = ( float ) ( eps );
        look_f_big_ixmax[i] = ( float ) ( 10.0 * log10 ( ave_noise ) );
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo1::output_sf ( SCALEFACT sf_out[] )
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
                sf[ch][i] -= pretable[i];
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

/*---------------------------------------------------------------*/
void
CBitAllo1::compute_x34 (  )
{
    int i, j, k, n, ch;

    for ( ch = 0; ch < nchan; ch++ )
    {
        n = startBand_l[nsf[ch]];
        //for(i=0;i<n;i++) x34[ch][i] = (float)(pow(xr[ch][i],(3.0/4.0)));
        vect_fpow34 ( xr[ch], x34[ch], n );
    }

/* TO DO - drop x34mm, to decide null frame, just scan x34max */
    x34mm = 0.0f;
    for ( ch = 0; ch < nchan; ch++ )
    {
        k = 0;
        for ( i = 0; i < nsf[ch]; i++ )
        {
            x34max[ch][i] = 0.0f;
            n = nBand_l[i];
            for ( j = 0; j < n; j++, k++ )
            {
                if ( x34max[ch][i] < x34[ch][k] )
                    x34max[ch][i] = x34[ch][k];
            }
            if ( x34mm < x34max[ch][i] )
                x34mm = x34max[ch][i];
            if ( x34max[ch][i] < gz_con0 )
                gzero[ch][i] = 0;
            else
                gzero[ch][i] =
                    ( int ) ( gz_con1 * log ( x34max[ch][i] ) + gz_con2 );
            gmin[ch][i] = HX_MAX ( 0, gzero[ch][i] - GMIN_OFFSET );
            /* gzero should max out at about 64 with     */
            /* g offset = 8, mdct scale = 1/9, peak pcm = 32k */
            /* g >= gzero --> ixmax=0, but not converse! */

        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo1::smr_adj ( SIG_MASK sm[][36], unsigned char signx[][576] )
{
    int i, j, k, n, ch;
    float r;

    for ( ch = 0; ch < nchan; ch++ )
    {
        k = 0;
        for ( i = 0; i < nsf[ch]; i++ )
        {
            xsxx[ch][i] = 1.0e-12f;     // greater than 0 for log
            n = nBand_l[i];
            for ( j = 0; j < n; j++, k++ )
            {
                signx[ch][k] = 0;
                if ( xr[ch][k] < 0.0f )
                {
                    signx[ch][k] = 1;
                    xr[ch][k] = -xr[ch][k];
                }
                xsxx[ch][i] += xr[ch][k] * xr[ch][k];
            }
        }
    }

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            r = sm[ch][i].sig / ( sm[ch][i].mask *
                                  ( 0.1f + 0.0001f * xsxx[ch][i] ) );
            if ( r < 1.0e-10f )
                mask[ch][i] = 100.0f;
            else
                mask[ch][i] =
                    ( float ) ( -10.0 * log10 ( r ) - look_log_cbw[i] );
        }
    }

}

/*---------------------------------------------------------------*/
void
CBitAllo1::smr_adj_joint ( SIG_MASK sm[][36], unsigned char signx[][576] )
{
    int i, j, k, n, ch;
    float r, a, b;

    if ( ms_flag == 0 )
    {
        // stereo portion signs and xsxx
        for ( ch = 0; ch < nchan; ch++ )
        {
            k = 0;
            for ( i = 0; i < nsf[1]; i++ )
            {   // note nsf[1] = chan 2
                xsxx[ch][i] = 1.0e-12f; // greater than 0 for log
                n = nBand_l[i];
                for ( j = 0; j < n; j++, k++ )
                {
                    signx[ch][k] = 0;
                    if ( xr[ch][k] < 0.0f )
                    {
                        signx[ch][k] = 1;
                        xr[ch][k] = -xr[ch][k];
                    }
                    xsxx[ch][i] += xr[ch][k] * xr[ch][k];
                }
            }
        }
    }
    else
    {
        // ms processing
        k = 0;
        for ( i = 0; i < nsf[1]; i++ )
        {       // note nsf[1] = chan 2
            xsxx[0][i] = xsxx[1][i] = 1.0e-12f; // greater than 0 for log
            n = nBand_l[i];
            for ( j = 0; j < n; j++, k++ )
            {
                xsxx[0][i] += xr[0][k] * xr[0][k];
                xsxx[1][i] += xr[1][k] * xr[1][k];
                a = con707 * xr[0][k];
                b = con707 * xr[1][k];
                xr[0][k] = a + b;
                xr[1][k] = a - b;
                signx[0][k] = signx[1][k] = 0;
                if ( xr[0][k] < 0.0f )
                {
                    signx[0][k] = 1;
                    xr[0][k] = -xr[0][k];
                }
                if ( xr[1][k] < 0.0f )
                {
                    signx[1][k] = 1;
                    xr[1][k] = -xr[1][k];
                }
            }
        }
    }

// sparse the side channel in ms mode
    if ( ms_flag )
    {
        k = startBand_l[5];
        for ( i = 5; i < nsf[1]; i++ )
        {       // note nsf[1] = chan 2
            n = nBand_l[i];
            for ( j = 0; j < n; j += 2, k += 2 )
            {
                a = xr[1][k] * xr[1][k] + xr[1][k + 1] * xr[1][k + 1];
                b = Ssb[i] * ( a + xr[0][k] * xr[0][k] +
                               xr[0][k + 1] * xr[0][k + 1] );
                if ( a < b )
                    xr[1][k] = xr[1][k + 1] = 0.0f;
            }
        }
    }

//------------ intensity processing ---
// intensity portion sum L/R, and signs
    if ( is_flag )
    {
        if ( h_id )
        {       /// mpeg1
            for ( i = nsf[1]; i < nsf[0]; i++ )
            {   // note nsf[0] = chan 1
                xsxx[0][i] = xsxx[1][i] = 1.0e-12f;     // greater than 0 for log
                n = nBand_l[i];
                k = startBand_l[i];
                r = 1.0f;
                for ( j = 0; j < n; j++, k++ )
                {
                    xsxx[0][i] += xr[0][k] * xr[0][k];
                    xsxx[1][i] += xr[1][k] * xr[1][k];
                    xr[0][k] = xr[0][k] + xr[1][k];
                    r += xr[0][k] * xr[0][k];
                    signx[0][k] = 0;
                    if ( xr[0][k] < 0.0f )
                    {
                        signx[0][k] = 1;
                        xr[0][k] = -xr[0][k];
                    }
                }
                r = ( float ) ( sqrt ( ( xsxx[0][i] + xsxx[1][i]
                                         +
                                         2.0 * sqrt ( xsxx[0][i] *
                                                      xsxx[1][i] ) ) / r ) );
                if ( r > 1.5f )
                    r = 1.5f;
                k = startBand_l[i];
                for ( j = 0; j < n; j++, k++ )
                    xr[0][k] = r * xr[0][k];
            }
        }
        else
        {       // mpeg2
            for ( i = nsf[1]; i < nsf[0]; i++ )
            {   // note nsf[0] = chan 1
                xsxx[0][i] = xsxx[1][i] = 1.0e-12f;     // greater than 0 for log
                n = nBand_l[i];
                k = startBand_l[i];
                r = 1.0f;
                for ( j = 0; j < n; j++, k++ )
                {
                    xsxx[0][i] += xr[0][k] * xr[0][k];
                    xsxx[1][i] += xr[1][k] * xr[1][k];
                    xr[0][k] = xr[0][k] + xr[1][k];
                    r += xr[0][k] * xr[0][k];
                    signx[0][k] = 0;
                    if ( xr[0][k] < 0.0f )
                    {
                        signx[0][k] = 1;
                        xr[0][k] = -xr[0][k];
                    }
                }
                if ( xsxx[0][i] > xsxx[1][i] )
                    a = xsxx[0][i];
                else
                    a = xsxx[1][i];
                r = ( float ) ( sqrt ( a / r ) );
                if ( r > 1.2f )
                    r = 1.2f;
                k = startBand_l[i];
                for ( j = 0; j < n; j++, k++ )
                    xr[0][k] = r * xr[0][k];
            }
        }
    }   //---- is_flag done

// stereo portion mask
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[1]; i++ )
        {       // note nsf[1] = chan 2
            r = sm[ch][i].sig / ( sm[ch][i].mask *
                                  ( 0.1f + 0.0001f * xsxx[ch][i] ) );
            if ( r < 1.0e-10f )
                mask[ch][i] = 100.0f;
            else
                mask[ch][i] =
                    ( float ) ( -10.0 * log10 ( r ) - look_log_cbw[i] );
        }
    }

//------------ intensity processing ---
    if ( is_flag )
    {
        // intensity portion mask
        for ( i = nsf[1]; i < nsf[0]; i++ )
        {
            r = ( sm[0][i].sig + sm[1][i].sig ) /
                ( ( sm[0][i].mask + sm[1][i].mask )
                  * ( 0.1f + 0.0001f * ( xsxx[0][i] + xsxx[1][i] ) ) );
            if ( r < 1.0e-10f )
                mask[0][i] = 100.0f;
            else
                mask[0][i] =
                    ( float ) ( -10.0 * log10 ( r ) - look_log_cbw[i] );
        }

        // compute is_position
        if ( h_id )
        {       // mpeg1
            for ( i = nsf[1]; i < nsf[0]; i++ )
            {
                if ( xsxx[0][i] <= xsxx[1][i] )
                {
                    k = ( int ) ( 32.0f * xsxx[0][i] / xsxx[1][i] + 0.5f );
                    sf[1][i] = look_is_pos[k];
                }
                else
                {
                    k = ( int ) ( 32.0f * xsxx[1][i] / xsxx[0][i] + 0.5f );
                    sf[1][i] = 6 - look_is_pos[k];
                }
            }
        }
        else
        {       // mpeg2
            for ( i = nsf[1]; i < nsf[0]; i++ )
            {
                if ( xsxx[0][i] <= xsxx[1][i] )
                {
                    k = ( int ) ( 32.0f * xsxx[0][i] / xsxx[1][i] + 0.5f );
                    sf[1][i] = look_is_pos[k];
                    if ( sf[1][i] != 0 )
                        sf[1][i] -= 1;
                }
                else
                {
                    k = ( int ) ( 32.0f * xsxx[1][i] / xsxx[0][i] + 0.5f );
                    sf[1][i] = look_is_pos[k];
                }
            }

        }
    }   //------ is flag done 

    if ( ms_flag )
    {
        for ( i = 0; i < nsf[1]; i++ )
        {
            mask[1][i] = mask[0][i] = 0.5f * ( mask[0][i] + mask[1][i] );
        }
    }

}

/*===============================================================*/
void
CBitAllo1::fnc_ixmax (  )
{
    int i, ch;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            ixmax[ch][i] =
                ( int ) ( ( 0.5f - 0.0946f ) +
                          x34max[ch][i] * look_34igain[gsf[ch][i]] );
        }
    }

}

/*---------------------------------------------------------------*/
int
CBitAllo1::fnc_bit_est (  )
{
    int i, ixm, ch;
    int n, bits;

    n = 0;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            ixm = ixmax[ch][i];
            if ( ixm < 256 )
                bits = look_bits[ixm];
            else if ( ixm < 512 )
                bits = 16 * 11; /* scaled by 16 */
            else if ( ixm < 2048 )
                bits = 16 * 13;
            else
                bits = 16 * 15;
            n += nBand_l[i] * bits;
        }
    }
    return n >> 4;      /* bit table scaled by 16 */
}

/*----------------------------------------------------------------*/
int
CBitAllo1::fnc_bit_seek (  )
{
    int i, j, nbits;
    int delta_bits;
    int mindelta;
    int dG, gz_flag;
    int ch;

/* min < adjust gsf until bits < max */
    fnc_ixmax (  );
    nbits = fnc_bit_est (  );
    delta_bits = nbits - target0_bits;

/* decrease bits */
    if ( delta_bits > 0 )
    {
        for ( i = 0; i < 10; i++ )
        {
            if ( delta_bits <= 0 )
                break;
            dG = ( int ) ( dGdB * delta_bits );
            if ( dG < 1 )
                dG = 1;
            for ( ch = 0; ch < nchan; ch++ )
            {
                for ( j = 0; j < nsf[ch]; j++ )
                {
                    gsf[ch][j] += dG;
                    if ( gsf[ch][j] > gzero[ch][j] )
                        gsf[ch][j] = gzero[ch][j];
                }
            }
            fnc_ixmax (  );
            nbits = fnc_bit_est (  );
            delta_bits = nbits - target0_bits;
        }
        return nbits;
    }

/* increase bits */
    mindelta = target0_bits >> 2;       /* 25% threshold on increase */
    if ( mindelta < 100 )
        mindelta = 100;
    delta_bits = -delta_bits;
    if ( delta_bits < mindelta )
        return nbits;
    for ( i = 0; i < 10; i++ )
    {
        dG = ( int ) ( dGdB * delta_bits );
        if ( dG < 1 )
            dG = 1;
        gz_flag = 0;
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                gsf[ch][i] -= dG;
                if ( gsf[ch][i] < 0 )
                {
                    gsf[ch][i] = 0;
                }
                gz_flag |= gsf[ch][i];
            }
        }
        fnc_ixmax (  );
        nbits = fnc_bit_est (  );
        delta_bits = target0_bits - nbits;
        if ( delta_bits < mindelta )
            break;
        if ( gz_flag == 0 )
            break;      /* can't increase any more */
    }
    return nbits;
}

/*----------------------------------------------------------------*/
int
CBitAllo1::fnc_bit_seek2 (  )
{
    int i, j, ch, nbits;
    int delta_bits;
    int mindelta;
    int dG, gz_flag;
    int target;

/* min < adjust gsf until bits < max */

/* target adjusted for current noise */

//target = target0_bits;

    target =
        ( int ) ( target0_bits + 0.5f * dBG * ( alpha_nmr - ave_alpha_nmr ) );
    if ( target > target0_max )
        target = target0_max;
    else if ( target < target0_min )
        target = target0_min;

    fnc_ixmax (  );
    nbits = fnc_bit_est (  );
    delta_bits = nbits - target;

/* decrease bits */
    if ( delta_bits > 0 )
    {
        for ( i = 0; i < 10; i++ )
        {
            if ( delta_bits <= 0 )
                break;
            dG = ( int ) ( dGdB * delta_bits );
            if ( dG < 1 )
                dG = 1;
            for ( ch = 0; ch < nchan; ch++ )
            {
                for ( j = 0; j < nsf[ch]; j++ )
                {
                    gsf[ch][j] += dG;
                    if ( gsf[ch][j] > gzero[ch][j] )
                        gsf[ch][j] = gzero[ch][j];
                }
            }
            fnc_ixmax (  );
            nbits = fnc_bit_est (  );
            delta_bits = nbits - target;
        }
        return nbits;
    }

/* increase bits */
    mindelta = target >> 2;     /* 25% threshold on increase */
    if ( mindelta < 100 )
        mindelta = 100;
    delta_bits = -delta_bits;
    if ( delta_bits < mindelta )
        return nbits;
    for ( i = 0; i < 10; i++ )
    {
        dG = ( int ) ( dGdB * delta_bits );
        if ( dG < 1 )
            dG = 1;
        gz_flag = 0;
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                gsf[ch][i] -= dG;
                if ( gsf[ch][i] < 0 )
                {
                    gsf[ch][i] = 0;
                }
                gz_flag |= gsf[ch][i];
            }
        }
        fnc_ixmax (  );
        nbits = fnc_bit_est (  );
        delta_bits = target - nbits;
        if ( delta_bits < mindelta )
            break;
        if ( gz_flag == 0 )
            break;      /* can't increase any more */
    }
    return nbits;
}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_noise (  )
{
    int i, ixm, ch;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            ixm = ixmax[ch][i];
            if ( ixm < 256 )
                noise[ch][i] = look_f_ixmax[ixm] + 1.505f * gsf[ch][i];
            else
            {
                ixm >>= 5;      // scale by 32 and use big table
                if ( ixm > 255 )
                    ixm = 255;
                noise[ch][i] = look_f_big_ixmax[ixm] + 1.505f * gsf[ch][i];
            }
        }
    }
}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_noise_cb ( int i, int ch )
{
    int ixm;

    ixmax[ch][i] = ixm =
        ( int ) ( ( 0.5f - 0.0946f + 0.002f ) +
                  x34max[ch][i] * look_34igain[gsf[ch][i]] );
    if ( ixm < 256 )
        noise[ch][i] = look_f_ixmax[ixm] + 1.505f * ( gsf[ch][i] );
    else
    {
        ixm >>= 5;      // scale by 32 and use big table
        if ( ixm > 255 )
            ixm = 255;
        noise[ch][i] = look_f_big_ixmax[ixm] + 1.505f * ( gsf[ch][i] );
    }

}

/*===============================================================*/
void
CBitAllo1::fnc_noise2 (  )
{
    int i, j, k, n, ixm, ch;
    float sum, igain;

    for ( ch = 0; ch < nchan; ch++ )
    {

        for ( i = 0; i < nsf[ch]; i++ )
        {
            if ( gsf[ch][i] == lastGsf[ch][i] )
                continue;
            lastGsf[ch][i] = gsf[ch][i];
            n = nBand_l[i];
            k = startBand_l[i];
            igain = look_34igain[gsf[ch][i]];
            sum = 0.0f;
            for ( j = 0; j < n; j++, k++ )
            {
                ixm = ( int ) ( ( 0.5f - 0.0946f ) + x34[ch][k] * igain );
                if ( ixm < 256 )
                    sum += look_f_ix[ixm];
                else
                {
                    ixm >>= 5;  // scale by 32 and use big table
                    if ( ixm > 255 )
                        ixm = 255;
                    sum += look_f_big_ix[ixm];
                }
            }
            noise[ch][i] =
                ( float ) ( 10.0f * log10 ( sum ) - look_log_cbw[i] +
                            1.505f * ( gsf[ch][i] ) );
        }

    }
}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_noise2_cb ( int i, int ch )
{
    int j, k, n, ixm;
    float sum, igain;

    if ( gsf[ch][i] == lastGsf[ch][i] )
        return;
    lastGsf[ch][i] = gsf[ch][i];
    k = startBand_l[i];
    n = nBand_l[i];
    igain = look_34igain[gsf[ch][i]];
    sum = 0.0f;
    for ( j = 0; j < n; j++, k++ )
    {
        ixm = ( int ) ( ( 0.5f - 0.0946f ) + x34[ch][k] * igain );
        if ( ixm < 256 )
            sum += look_f_ix[ixm];
        else
        {
            ixm >>= 5;  // scale by 32 and use big table
            if ( ixm > 255 )
                ixm = 255;
            sum += look_f_big_ix[ixm];
        }
    }
    noise[ch][i] =
        ( float ) ( 10.0f * log10 ( sum ) - look_log_cbw[i] +
                    1.505f * gsf[ch][i] );

}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_noise2_init (  )
{
    int i, ch;

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < 21; i++ )
            lastGsf[ch][i] = -9999;
    }
}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_ix_quant (  )    // done by noise3 within allo loops
{
    int i, j, k, n, ch;
    float igain;

    for ( ch = 0; ch < nchan; ch++ )
    {

        for ( i = 0; i < nsf[ch]; i++ )
        {
            if ( gsf[ch][i] == lastGsf[ch][i] )
                continue;
            lastGsf[ch][i] = gsf[ch][i];
            n = nBand_l[i];
            k = startBand_l[i];
            if ( ixmax[ch][i] <= 0 )
            {
                for ( j = 0; j < n; j++, k++ )
                    ix[ch][k] = 0;
            }
            else
            {
                igain = look_34igain[gsf[ch][i]];
                for ( j = 0; j < n; j++, k++ )
                {
                    ix[ch][k] =
                        ( int ) ( ( 0.5f - 0.0946f ) + x34[ch][k] * igain );
                }
            }
        }

    }
}

/*----------------------------------------------------------------*/
int
CBitAllo1::fnc_noise_seek (  )
{
    int i, cb, ch;
    float a;
    float dn;
    int dg;
    int dgmax;
    float dn0;
    int gsf0, gsf00;
    float asum;
    int n;

    n = 0;
    asum = 0.0f;
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            if ( ( gsf[ch][i] > 0 ) && ( gsf[ch][i] < gzero[ch][i] ) )
            {
                asum += noise[ch][i] - mask[ch][i];
                n++;
            }
        }
    }
    if ( n <= 1 )
        return 0;       /* stuck at boundaries */
    a = asum / n;
    alpha_nmr = a;

    dgmax = 0;
    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( cb = 0; cb < nsf[ch]; cb++ )
        {
            dn = noise[ch][cb] - mask[ch][cb] - a;
            if ( dn > 1.0 )
            {   /* too much noise, decrease gsf */
                if ( gsf[ch][cb] <= 0 )
                    continue;   /* can't decrease noise */
                dn0 = dn;
                gsf00 = gsf0 = gsf[ch][cb];
                for ( i = 0; i < 50; i++ )
                {
                    if ( gsf[ch][cb] <= 0 )
                        break;
                    dg = ( int ) ( 0.5f * dn + 0.5f );
                    if ( dg <= 0 )
                        break;
                    gsf[ch][cb] -= dg;
                    if ( gsf[ch][cb] < 0 )
                        gsf[ch][cb] = 0;
                    function_noise_cb ( cb, ch );
                    dn = noise[ch][cb] - mask[ch][cb] - a;
                    if ( dn < -1.0f )
                    {   /* gone way too far */
                        dn = dn0 = 0.5f * dn0;
                        gsf[ch][cb] = gsf0;
                        continue;
                    }
                    dn0 = dn;
                    gsf0 = gsf[ch][cb];
                }
                dg = gsf00 - gsf[ch][cb];
                if ( dg > dgmax )
                    dgmax = dg;
            }
            else if ( dn < -1.0f )
            {   /* not enough noise, increase gsf */
                if ( gsf[ch][cb] >= gzero[ch][cb] )
                    continue;
                dn0 = dn;
                gsf00 = gsf0 = gsf[ch][cb];
                for ( i = 0; i < 50; i++ )
                {
                    if ( gsf[ch][cb] >= gzero[ch][cb] )
                        break;
                    dg = ( int ) ( -0.5f * dn );
                    if ( dg <= 0 )
                        break;
                    gsf[ch][cb] += dg;
                    if ( gsf[ch][cb] >= gzero[ch][cb] )
                        gsf[ch][cb] = gzero[ch][cb];
                    function_noise_cb ( cb, ch );
                    dn = noise[ch][cb] - mask[ch][cb] - a;
                    if ( dn > 1.0f )
                    {
                        dn = dn0 = 0.5f * dn0;  /* gone way too far */
                        gsf[ch][cb] = gsf0;
                        continue;
                    }
                    dn0 = dn;
                    gsf0 = gsf[ch][cb];
                }
                dg = gsf[ch][cb] - gsf00;
                if ( dg > dgmax )
                    dgmax = dg;
            }

        }       // cb
    }   // ch

//printf("\n %3d %3d %3d", sfmin, sfmax, sfmax-sfmin);
    return dgmax;
}

/*----------------------------------------------------------------*/
void
CBitAllo1::function_noise_cb ( int cb, int ch )
{

    switch ( i_function_noise_cb )
    {
    default:
    case i_fnc_noise_cb:
        fnc_noise_cb ( cb, ch );
        return;

    case i_fnc_noise2_cb:
        fnc_noise2_cb ( cb, ch );
        return;
    }

}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_sf_final ( int ch )
{
    if ( h_id )
        fnc_sf_final_MPEG1 ( ch );
    else
        fnc_sf_final_MPEG2 ( ch );
}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_sf_final_MPEG1 ( int ch )
{
    int i, n;
    int pre_flag, scale_flag;

    pre_flag = scale_flag = 0;

/* region of 4 bit scale factors */
    n = 11;
    if ( n > nsf[ch] )
        n = nsf[ch];
    for ( i = 0; i < n; i++ )
        if ( sf[ch][i] > 31 )
        {
            scale_flag = 1;
            break;
        }

/* region of 3 bit scale factors */
    if ( scale_flag == 0 )
        for ( i = 11; i < nsf[ch]; i++ )
            if ( sf[ch][i] > pre2[i] )
            {
                scale_flag = 1;
                break;
            }

    if ( scale_flag == 0 )
    {
        for ( i = 11; i < nsf[ch]; i++ )
            if ( sf[ch][i] > 15 )
            {
                pre_flag = 1;
                break;
            }
        if ( pre_flag )
        {       /* protect against negative sf */
            for ( i = 11; i < nsf[ch]; i++ )
                if ( ( sf[ch][i] >> 1 ) < pretable[i] )
                {
                    pre_flag = 0;
                    break;
                }
        }
    }
    else
    {
        for ( i = 11; i < nsf[ch]; i++ )
            if ( sf[ch][i] > 31 )
            {
                pre_flag = 1;
                break;
            }
        if ( pre_flag )
        {       /* protect against negative sf */
            for ( i = 11; i < nsf[ch]; i++ )
                if ( ( sf[ch][i] >> 2 ) < pretable[i] )
                {
                    pre_flag = 0;
                    break;
                }
        }
    }

/*-------limit sf ---------------*/
    if ( scale_flag == 0 )
    {
        for ( i = 0; i < n; i++ )
            if ( sf[ch][i] > 31 )
                sf[ch][i] = 31;
        if ( pre_flag == 0 )
        {
            for ( i = 11; i < nsf[ch]; i++ )
                if ( sf[ch][i] > 15 )
                    sf[ch][i] = 15;
        }
        else
        {
            for ( i = 11; i < nsf[ch]; i++ )
                if ( sf[ch][i] > pre2[i] )
                    sf[ch][i] = pre2[i];
        }
    }
    else
    {
        for ( i = 0; i < n; i++ )
            if ( sf[ch][i] > 63 )
                sf[ch][i] = 63;
        if ( pre_flag == 0 )
        {
            for ( i = 11; i < nsf[ch]; i++ )
                if ( sf[ch][i] > 31 )
                    sf[ch][i] = 31;
        }
        else
        {
            for ( i = 11; i < nsf[ch]; i++ )
                if ( sf[ch][i] > pre4[i] )
                    sf[ch][i] = pre4[i];
        }
    }

/*------- drop low bits -------*/

/* done by fnc_scale_factors */
//if( scale_flag == 0 ) for(i=0;i<nsf[ch];i++) sf[i] &= (~1);
//else                         for(i=0;i<nsf[ch];i++) sf[i] &= (~3);

    preemp[ch] = pre_flag;
    scalefactor_scale[ch] = scale_flag;

    return;
}

/*----------------------------------------------------------------*/
void
CBitAllo1::fnc_sf_final_MPEG2 ( int ch )
{
    int i, n;
    int pre_flag, scale_flag;

    pre_flag = scale_flag = 0;

/* region of 4 bit scale factors */
    n = 11;
    if ( n > nsf[ch] )
        n = nsf[ch];
    for ( i = 0; i < n; i++ )
        if ( sf[ch][i] > 31 )
        {
            scale_flag = 1;
            break;
        }

/* region of 3 bit scale factors */
    if ( scale_flag == 0 )
        for ( i = 11; i < nsf[ch]; i++ )
            if ( sf[ch][i] > 15 )
            {
                scale_flag = 1;
                break;
            }

/*-------limit sf ---------------*/
    if ( scale_flag == 0 )
    {
        for ( i = 0; i < n; i++ )
            if ( sf[ch][i] > 31 )
                sf[ch][i] = 31;
        for ( i = 11; i < nsf[ch]; i++ )
            if ( sf[ch][i] > 15 )
                sf[ch][i] = 15;
    }
    else
    {
        for ( i = 0; i < n; i++ )
            if ( sf[ch][i] > 63 )
                sf[ch][i] = 63;
        for ( i = 11; i < nsf[ch]; i++ )
            if ( sf[ch][i] > 31 )
                sf[ch][i] = 31;
    }

    preemp[ch] = 0;
    scalefactor_scale[ch] = scale_flag;

    return;
}

/*----------------------------------------------------------------*/
int
CBitAllo1::fnc_scale_factors (  )
{
    int i, ch;
    int Gtmp, Gtmpmin;

    Gtmpmin = 999;
    for ( ch = 0; ch < nchan; ch++ )
    {

/* compute G */
        Gtmp = -1;
        for ( i = 0; i < nsf[ch]; i++ )
        {
            gsf[ch][i] = HX_MAX ( gsf[ch][i], gmin[ch][i] );       // quant overflow protect
            if ( ( ixmax[ch][i] > 0 ) && ( gsf[ch][i] > Gtmp ) )
                Gtmp = gsf[ch][i];
        }
        if ( Gtmp < 0 )
        {       /* null frame for ch */
            for ( i = 0; i < nsf[ch]; i++ )
            {
                sf[ch][i] = 0;
                gsf[ch][i] = gzero[ch][i];
                if ( gsf[ch][i] > Gtmp )
                    Gtmp = gsf[ch][i];
            }
            preemp[ch] = 0;
            scalefactor_scale[ch] = 0;
            G[ch] = Gtmp;
            if ( 100 < Gtmpmin )
                Gtmpmin = 100;
            continue;   // continue channel loop
        }

/* scale factors */
        for ( i = 0; i < nsf[ch]; i++ )
        {
            sf[ch][i] = 0;
            if ( ixmax[ch][i] > 0 )
                sf[ch][i] = Gtmp - gsf[ch][i];
        }

/* choose scale preemp etc. */
        fnc_sf_final ( ch );

/* which better ? don't know */

/* drop low bits */
// number 1
        if ( scalefactor_scale[ch] == 0 )
            for ( i = 0; i < nsf[ch]; i++ )
                sf[ch][i] &= ( ~1 );
        else
            for ( i = 0; i < nsf[ch]; i++ )
                sf[ch][i] &= ( ~3 );

/* back compute gsf */
        for ( i = 0; i < nsf[ch]; i++ )
        {
            gsf[ch][i] = Gtmp - sf[ch][i];
            if ( gsf[ch][i] > gzero[ch][i] )
                gsf[ch][i] = gzero[ch][i];      /* not needed if quiting ba */
        }

        G[ch] = Gtmp;
        if ( Gtmp < Gtmpmin )
            Gtmpmin = Gtmp;

    }   // end ch loop 

    return Gtmpmin;
}

/*----------------------------------------------------------------*/
int
CBitAllo1::allo_2 (  )
{
    int i, j, ch;
    int nbits;
    int dsf;
    int GG, dG;
    int bits;

//static int totbits, ntotbits, totdelta;
    int gz_flag;
    int tmp;
    static int n_increase;

    fnc_noise2_init (  );

/* stage 1 ----------------------------*/
//outi(gzero, 21);

    i_function_noise_cb = i_fnc_noise_cb;
    nbits = fnc_bit_seek (  );
    for ( i = 0; i < 4; i++ )
    {
        fnc_noise (  );
        dsf = fnc_noise_seek (  );
        if ( dsf <= 0 )
            break;      /* no change, bit seek not needed */
        nbits = fnc_bit_seek (  );
        //printf("\nA1 %2d dsf %3d  bits %4d", i, dsf, nbits);
        if ( dsf < 2 )
            break;
    }

/* stage 2 ----------------------------*/

    i_function_noise_cb = i_fnc_noise2_cb;
    for ( i = 0; i < 4; i++ )
    {   // base iter = 4
        fnc_noise2 (  );
        dsf = fnc_noise_seek (  );
        if ( dsf <= 0 )
            break;      /* no change, bit seek not needed */
        //nbits = fnc_bit_seek();
        nbits = fnc_bit_seek2 (  );
        //printf("\nA2 %2d dsf %3d  bits %4d", i, dsf, nbits);
        if ( dsf < 2 )
            break;
    }

    fnc_noise2_init (  );       // re-init lastGsf for fnc_ix_quant
    fnc_scale_factors (  );
    fnc_ixmax (  );
    fnc_ix_quant (  );

    for ( bits = 0, ch = 0; ch < nchan; ch++ )
        bits += huff_bits[ch] =
            subdivide2 ( ixmax[ch], ix[ch], nsf[ch], 1, ch );

    bitadjust = bitadjust + ( ( bits - nbits - bitadjust ) >> 3 );      // feedback 

    if ( ( tmp = min_bits - bits ) > 0 )
    {
        if ( tmp > 200 )
        {
            tmp = 200;
        }
        bitadjust = bitadjust - ( tmp >> 2 );
    }

    if ( ( min_bits - bits ) >= 50 )
    {
        n_increase++;
    }

/* increase bits if < min */
    for ( j = 0; j < 3; j++ )
    {
        if ( ( min_bits - bits ) < 50 )
            break;
//printf("\n %3d bit increase %3d %3d G %3d %3d", j, bits, min_bits, G[0], G[1]);
        dG = ( int ) ( dGdB * ( min_bits - bits ) );
        if ( dG < 1 )
            dG = 1;
        gz_flag = 0;
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
            {
                gsf[ch][i] -= dG;
                if ( gsf[ch][i] < 0 )
                    gsf[ch][i] = 0;
                gz_flag |= gsf[ch][i];
            }
        }
        fnc_scale_factors (  ); // shortcut some of this ? 
        fnc_ixmax (  );
        fnc_ix_quant (  );
        for ( bits = 0, ch = 0; ch < nchan; ch++ )
            bits += huff_bits[ch] =
                subdivide2 ( ixmax[ch], ix[ch], nsf[ch], 1, ch );
        if ( gz_flag == 0 )
            break;      /* can't increase any more */
    }

/* decrease bits if > max */
    for ( j = 0; j < 100; j++ )
    {
        if ( bits <= max_cnt_bits )
            break;
//printf("\n bit decrease %3d %3d %3d", bits, max_cnt_bits, max_bits);
        dG = ( int ) ( dGdB * ( bits - max_cnt_bits ) );
        if ( dG < 1 )
            dG = 1;
        for ( ch = 0; ch < nchan; ch++ )
        {
            for ( i = 0; i < nsf[ch]; i++ )
                gsf[ch][i] += dG;
        }
        GG = fnc_scale_factors (  );    // shortcut some of this ? 
        fnc_ixmax (  );
        fnc_ix_quant (  );
        for ( bits = 0, ch = 0; ch < nchan; ch++ )
            bits += huff_bits[ch] =
                subdivide2 ( ixmax[ch], ix[ch], nsf[ch], 1, ch );
        if ( GG >= 100 )
            break;
    }

    for ( ch = 0; ch < nchan; ch++ )
    {
        for ( i = 0; i < nsf[ch]; i++ )
        {
            if ( ( ixmax[ch][i] <= 0 ) )
                sf[ch][i] = 0;
        }
    }

    return bits;
}

/*----------------------------------------------------------------*/
