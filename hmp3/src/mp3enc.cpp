/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: mp3enc.cpp,v 1.2 2005/08/09 20:43:42 karll Exp $ 
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

#include "mp3enc.h"
#include "mp3low.h"     // interface to low level routines
#include "filter2.h"

//#define DOSHOWCONTROL

#ifdef DOSHOWCONTROL
#include <windows.h>
#endif //DOSHOWCONTROL

extern "C"
{
    void start_clock (  );
    void end_clock (  );
}

// 1/27/00 init insurance, constructor clears additional vars

// values of encode selector  iL3_audio_encode_function = 
#define iL3_audio_encode_vbr_MPEG1   0
#define iL3_audio_encode_MPEG1       1
#define iL3_audio_encode_vbr_MPEG2   2
#define iL3_audio_encode_MPEG2       3

// values of encode selector  iencode_function
#define iencode_jointA         0
#define iencode_jointB         1
#define iencode_singleA        2
#define iencode_singleB        3
#define iencode_jointA_MPEG2   4
#define iencode_jointB_MPEG2   5
#define iencode_singleA_MPEG2  6
#define iencode_singleB_MPEG2  7

// for testing
extern "C"
{
  int iframe; // for testing
}

// table to determine current block type 
// current = 
//   block_type_select[prev][short_flag_current][short_flag_next]
// not all combinations are valid (should not happen)
static const int block_type_select[4][2][2] = {
    0, 1, 2, 2,
    3, 2, 2, 2,
    3, 2, 2, 2,
    0, 1, 2, 2,
};

/*---------------
p  cur next    curr
0   0   0       0
0   0   2       1
0   2   0       2x
0   2   2       2x
   
p  cur next
1   0   0       3x
1   0   2       2
1   2   0       2
1   2   2       2

p  cur next
2   0   0       3
2   0   2       2
2   2   0       2
2   2   2       2

p  cur next
3   0   0       0
3   0   2       1
3   2   0       2x
3   2   2       2x
------------------*/

//=========================================================
CMp3Enc::CMp3Enc (  ):
    tot_frames_out ( 0 ),
    tot_bytes_out ( 0 ),
    ave_tot_bytes_out ( 0 ),     // running average
    ivbr_min ( 1 ),              // min and max vbr bitrate_index
    ivbr_max ( 14 ),
    monodual ( 0 ),
    nchan ( 1 ),
    h_id ( 1 ),
    is_flag ( 0 ),
    ms_flag ( 0 ),
    sr_index ( 0 ),
    nband ( 418 ),
    band_limit ( 418 ),
    band_limit_stereo ( 418 ),
    nsb ( 30 ),
    stereo_flag ( 0 ),
    nsb_limit ( 30 ),
    nsbstereo ( 4 ),
    nsbstereo_limit ( 4 ),
    nsf_stereo ( 0 ),
    AveTargetBits ( 0 ),
    sf_bit_max ( 0 ),
    framebytes ( 0 ),
    pad ( 0 ),
    main_framebytes ( 0 ),
    side_bytes ( 0 ),
    padcount ( 44100 ),
    divisor ( 44100 ),
    remainder ( 20700 ),
    padbytes ( 1 ),
    bytesout ( 313 ),
    bytes_in ( 0 ),
//---------------
    totbitrate ( 0 ), samprate ( 0 ),
//---------------
    byte_pool ( 0 ),
    byte_min ( 0 ),
    byte_max ( 0 ),
    igr ( 0 ),
    igr_prev ( 0 ),
    igrx ( 0 ),
    vbr_pool_target ( 255 ),
    BitAllo ( NULL ),
    iL3_audio_encode_function ( 0 ),
    iencode_function ( 0 ),
    cpu_select ( 0 ), short_block_threshold ( 700 ), 
    src_encode ( 0 ), src ( NULL ), src_pcmbuf ( NULL ), int_dummy ( 0 )
{
    head[0] = 0xFF;
    head[1] = 0xFd;
    head[2] = 0x60;
    head[3] = 0xC0;
    memset ( vbr_main_framebytes, 0, sizeof ( vbr_main_framebytes ) );
    memset ( vbr_framebytes, 0, sizeof ( vbr_framebytes ) );
    nsb_limitMS[0] = 0;
    nsb_limitMS[1] = 0;

//---- acoustic model
// amod center points
// additional 576 (one granule) for short decision look ahead 3/14/00
// additional 2*576 (two granule) for short decision look ahead 3/29/00
    amod_audio_buf[0][0] = buf1 + 576 + 240 + 576 + 288 + 1152;
    amod_audio_buf[0][1] = buf1 + 240 + 576 + 288 + 1152;
    amod_audio_buf[1][0] = buf2 + 576 + 240 + 576 + 288 + 1152;
    amod_audio_buf[1][1] = buf2 + 240 + 576 + 288 + 1152;
    memset ( nsum, 0, sizeof ( nsum ) );
    memset ( eng, 0, sizeof ( eng ) );
    memset ( spd_cntl, 0, sizeof ( spd_cntl ) );        // spd, spread fnc
    memset ( w_spd, 0, sizeof ( w_spd ) );      // spd

    memset ( nsumShort, 0, sizeof ( nsumShort ) );
    memset ( spd_cntlShort, 0, sizeof ( spd_cntl ) );   // spd, spread fnc
    memset ( w_spdShort, 0, sizeof ( w_spd ) ); // spd
//----------------

// polyphase done one granule ahead, sbt for next time done after hybrid
    audio_buf[0][0] = buf1 + 576;
    audio_buf[0][1] = buf1;
    audio_buf[1][0] = buf2 + 576;
    audio_buf[1][1] = buf2;

}

/*--------------------------------------------------------------------*/
CMp3Enc::~CMp3Enc (  )
{
    if (BitAllo != NULL)
    { 
        delete BitAllo;
    }

    if ( src != NULL )
    {
        delete src;
    }
    if ( src_pcmbuf != NULL )
    {
        delete[]src_pcmbuf;
    }
}

//=====================================================================
int
CMp3Enc::L3_audio_encode_init ( E_CONTROL * ec_arg )
{
    int i, j, k;
    int tmp;
    E_CONTROL ec;
    int initialMNR;
    int nsb_limit_user1, nsb_limit_user2, nsb_limit_user, nsb_user_flag;
    int freq_limit;

//int nband_limit_calc;
    BA_CONTROL bac;

//int n1, n2;

#ifdef DOSHOWCONTROL
    char sz[512];

    if ( ec_arg->test2 )
    {
        ec_arg->test2 = 0;

        sprintf ( sz,
                  "mode %d\nbitrate %d\nsamprate %d\nnsbstereo %d\nfilter_select %d\nfreq_limit %d\nnsb_limit %d\nquick %d\nlayer %d\ncr_bit %d\noriginal %d\nhf_flag %d\nvbr_flag %d\nvbr_mnr %d\nvbr_br_limit %d\nvbr_delta_mnr %d\nchan_add_f0 %d\nchan_add_f1 %d\nsparse_scale %d\ntest1 %d\ncpu_select %d\ntest2 %d\ntest3 %d\n",
                  ec_arg->mode, ec_arg->bitrate, ec_arg->samprate,
                  ec_arg->nsbstereo, ec_arg->filter_select,
                  ec_arg->freq_limit, ec_arg->nsb_limit, ec_arg->quick,
                  //ec_arg->npass,         
                  //ec_arg->npred,         
                  ec_arg->layer,
                  ec_arg->cr_bit,
                  ec_arg->original,
                  ec_arg->hf_flag,
                  ec_arg->vbr_flag,
                  ec_arg->vbr_mnr,
                  ec_arg->vbr_br_limit,
                  ec_arg->vbr_delta_mnr,
                  ec_arg->chan_add_f0,
                  ec_arg->chan_add_f1,
                  ec_arg->sparse_scale,
                  ec_arg->test1,
                  ec_arg->cpu_select, ec_arg->test2, ec_arg->test3 );

        MessageBox ( NULL, sz, NULL, 0 );

    }
#endif //DOSHOWCONTROL

    if ( BitAllo != NULL )
    {   // incase multi calls to init
        delete BitAllo;

        BitAllo = NULL;
    }

    igr = 0;
    igr_prev = igr ^ 1;
    igrx = 0;

    memset ( &side_info, 0, sizeof ( side_info ) );

    memset ( attack_buf, 0, sizeof ( attack_buf ) );
    for ( i = 0; i < ( sizeof ( attack_buf[0] ) / sizeof ( int ) ); i++ )
    {
        attack_buf[0][i] = attack_buf[1][i] = 9000;
    }

    iframe = 0;

/*---- make local copy, may change ----*/
    ec = *ec_arg;

/*------- set defaults, limit to valid range -----*/

//-------------- mode 
    if ( ec.mode < 0 )
        ec.mode = 1;
    if ( ec.mode > 3 )
        ec.mode = 3;
//-------------- bitrate
    if ( ec.bitrate < 0 )
    {
        ec.bitrate = 64;
        if ( ec.samprate < 32000 )
        {
            if ( ec.mode == 3 )
                ec.bitrate = 32;
            else
                ec.bitrate = 32;
        }
    }
//-------------- vbr_flag
// vbr requires bitallo2 - as of 9/28/98 handles mpeg1 only
//if( ec.samprate < 32000 ) ec.vbr_flag = 0;
    if ( ec.mode == 2 )
        ec.vbr_flag = 0;
//if( ec.mode == 3 )        ec.vbr_flag = 0;
//-------------- vbr_mnr
    if ( ec.vbr_mnr < 0 )
        ec.vbr_mnr = 0;
    if ( ec.vbr_mnr > 150 )
        ec.vbr_mnr = 150;
//-------------- nsbstereo
    if ( ec.mode != 1 )
        ec.nsbstereo = 0;
    if ( ec.vbr_flag )
        ec.nsbstereo = 0;       // allo2 cannot handle is
//-------------- hf_flag
    if ( ec.mode == 2 )
        ec.hf_flag = 0;
    if ( ec.vbr_flag == 0 )
    {
        if ( ec.bitrate < 96 )
            ec.hf_flag = 0;
    }
    else
    {
        if ( ec.vbr_mnr < 80 )
            ec.hf_flag = 0;
    }
    if ( ec.samprate < 44100 )
        ec.hf_flag = 0;
//-------------- ec.filter_select
    if ( ec.filter_select < 0 )
        ec.filter_select = 0;
//

    if ( ( ec.vbr_flag == 0 ) && ( ec.samprate > 24000 )
         && ( ec.bitrate < 48 ) )
    {
        // for public use, fail ridiculous bitrates
        return 0;
    }

    ec.cr_bit = ec.cr_bit & 1;
    ec.original = ec.original & 1;
//-----------------------------------------------
// limit low rate, with side and sf, no room for audio data
// also need to limit since byte pool could hold more that 31 frames
// (of silence) causing side_p0/side_p1 to wrap
    if ( ec.samprate > 32000 )
    {
        if ( ec.bitrate < 24 )
            ec.bitrate = 24;
    }
    else if ( ec.samprate > 24000 )
    {
        if ( ec.bitrate < 16 )
            ec.bitrate = 16;
    }
    else if ( ec.samprate > 16000 )
    {
        if ( ec.bitrate < 12 )
            ec.bitrate = 12;
    }
    else
    {
        if ( ec.bitrate < 8 )
            ec.bitrate = 8;
    }

    cpu_select = ec.cpu_select;
    short_block_threshold = ec.short_block_threshold;

/*-- min and max bitrates limited by setup_header --*/

/*------- setup encoder parameters -----*/
    totbitrate = setup_header ( &ec, &h );

    if ( h.option != 1 )
        return 0;       // fail init if not Layer III

    pack_head_local ( &h, head );

    side_info.mode = h.mode;
    h_id = h.id;
    monodual = 1;
    if ( h.mode == 3 )
        monodual = 0;
    nchan = monodual + 1;
    sr_index = h.sr_index;

//-------------------------------
    nband = setup_nband_L3 ( &h );
    nsb = ( nband + 17 ) / 18;

//nsbstereo = 4 + 4*h.mode_ext;
    if ( h.id == 0 )
    {   // mpeg2
        nsbstereo = 7 * totbitrate / 16 - 7;
        nsbstereo = HX_MIN ( nsbstereo, 32 );
        nsbstereo = HX_MAX ( nsbstereo, 3 );
        if ( totbitrate >= 48 )
            nsbstereo = 32;     // no intensity >= 48
    }
    else
    {   // mpeg1
        nsbstereo = 12 * totbitrate / 32 - 20;
        nsbstereo = HX_MIN ( nsbstereo, 32 );
        nsbstereo = HX_MAX ( nsbstereo, 3 );
        if ( totbitrate >= 96 )
            nsbstereo = 32;     // no intensity >= 96
    }

    if ( ec.vbr_flag )
    {   // no is for vbr, allo2 cannot handle is
        nsbstereo = 32;
    }

    if ( ec.nsbstereo > 0 )
    {   // allow user any in range 3-30 for Layer III
        nsbstereo = ec.nsbstereo;
        if ( nsbstereo < 3 )
            nsbstereo = 3;
        if ( nsbstereo > 32 )
            nsbstereo = 32;
    }
    if ( nsbstereo > nsb )
        nsbstereo = nsb;

    samprate = setup_samprate ( &h );
    divisor = samprate;

    stereo_flag = 0;
    if ( h.mode != 3 )
        stereo_flag = 1;

/* layer III */
    if ( h.id == 1 )
    {   // MPEG1
        iL3_audio_encode_function = iL3_audio_encode_MPEG1;
        framebytes = 144000 * totbitrate / divisor;
        remainder = ( 144000 * totbitrate ) % divisor;
        padcount = divisor;
        padbytes = 1;
        if ( h.mode == 3 )
            side_bytes = 17;
        else
            side_bytes = 32;
        main_framebytes = framebytes - 4;       /* less header */
        main_framebytes -= side_bytes;  /* less side */
        //sf_bit_max = 4*11+3*10;      // long blocks
        sf_bit_max = 3 * ( 6 * 4 + 6 * 3 );     // short blocks (do dynamically?)
        AveTargetBits = 8 * main_framebytes / 2;        /* bits per gr */
        if ( h.mode != 3 )
            AveTargetBits >>= 1;        /* per channel */
        AveTargetBits -= sf_bit_max;    /* less sf */
    }
    else
    {   // mpeg 2
        iL3_audio_encode_function = iL3_audio_encode_MPEG2;
        framebytes = ( 144000 / 2 ) * totbitrate / divisor;
        remainder = ( ( 144000 / 2 ) * totbitrate ) % divisor;
        padcount = divisor;
        padbytes = 1;
        if ( h.mode == 3 )
            side_bytes = 9;
        else
            side_bytes = 17;
        main_framebytes = framebytes - 4;       /* less header */
        main_framebytes -= side_bytes;  /* less side */
        //sf_bit_max = 4*11+3*10;      // long blocks
        sf_bit_max = 3 * ( 6 * 4 + 6 * 3 );     // short blocks (do dynamically?)
        AveTargetBits = 8 * main_framebytes;    /* bits per gr=frame */
        if ( h.mode != 3 )
            AveTargetBits >>= 1;        /* per channel */
        AveTargetBits -= sf_bit_max;    /* less sf */
    }

/*------------- nsb bandwidth  --*/
    nsb_user_flag = 0;
    nsb_limit_user1 = 32;
    if ( ec.nsb_limit > 0 )
    {
        nsb_limit_user1 = HX_MIN ( ec.nsb_limit, 32 );
        nsb_limit_user1 = HX_MAX ( ec.nsb_limit, ( 64 * 1000 + samprate / 2 ) / samprate );        // 1000Hz
        nsb_user_flag = 1;
    }
    nsb_limit_user2 = 32;
    if ( ec.freq_limit < 24000 )
    {
        nsb_limit_user2 =
            ( 64 * HX_MAX ( ec.freq_limit, 1000 ) + samprate / 2 ) / samprate;
        nsb_user_flag = 1;
    }
    nsb_limit_user = HX_MIN ( nsb_limit_user1, nsb_limit_user2 );
    if ( ec.vbr_flag )
    {
        if ( h_id == 1 )
        {
            // this will set -v50 freq to be desired 23 sf bands (15848)
            freq_limit = 12000 + 80 * ec.vbr_mnr;       // for mpeg1
            if ( ec.vbr_mnr <= 5 )
                freq_limit = 12000;
            freq_limit =
                HX_MIN ( freq_limit,
                      ( ( int ) ( ( 0.96f * 0.5f ) * samprate ) ) );
        }
        else
        {       // mpeg-2
            // splitting mpeg-2 sf bands apparently causing trouble 
            // so try to hit whole sf band
            freq_limit = 7500 + 50 * ec.vbr_mnr;        // for mpeg2
            if ( ec.vbr_mnr <= 5 )
                freq_limit = 7500;
            freq_limit =
                HX_MIN ( freq_limit,
                      ( ( int ) ( ( 0.96f * 0.5f ) * samprate ) ) );
            freq_limit =
                L3freq_nearest_sf_band ( sr_index, h_id, freq_limit );
            // round to nearest polyphase nsb
            tmp = ( 64 * freq_limit + ( samprate / 2 ) ) / samprate;
            freq_limit = ( tmp * samprate ) / 64;
        }
    }
    else
    {
        freq_limit = calc_freq_limit_L3 ( samprate, totbitrate, h.mode );
        if ( ( h_id == 0 ) )
        {       // mpeg2 splitting sf band causing trouble at esp at cbr48
            freq_limit =
                L3freq_nearest_sf_band ( sr_index, h_id, freq_limit );
            // round to nearest polyphase nsb
            tmp = ( 64 * freq_limit + ( samprate / 2 ) ) / samprate;
            freq_limit = ( tmp * samprate ) / 64;
        }
    }

    if ( nsb_user_flag )
    {
        nsb_limit = nsb_limit_user;
    }
    else
    {
        //nband_limit_calc = ((2*576)*freq_limit+samprate/2)/samprate;
        //nsb_limit = (nband_limit_calc+17)/18;  // done like this so 112->full
        //nsb_limit = HX_MIN(nsb_limit, 32);
        nsb_limit =
            ( 64 * HX_MAX ( freq_limit, 1000 ) + samprate / 2 ) / samprate;
    }
    nsb_limit = HX_MIN ( nsb, nsb_limit );

    nsb_limitMS[0] = nsb_limitMS[1] = nsb_limit;
    if ( nsb_limit < nsb )
        ec.hf_flag = 0;
    if ( ec.hf_flag )
    {
        nsb_limitMS[0] = 29;
        if ( nsb_user_flag )
            nsb_limitMS[0] = HX_MIN ( nsb_limit_user, 29 );

    }
    if ( ec.hf_flag & 2 )
    {
        nsb_limitMS[1] = 29;
        if ( nsb_user_flag )
            nsb_limitMS[1] = HX_MIN ( nsb_limit_user, 29 );
    }

    band_limit = 18 * nsb_limit;
    if ( band_limit > nband )
        band_limit = nband;
//----------------------------------
    nsbstereo_limit = HX_MIN ( nsbstereo, nsb_limit );

/* number right chan stereo bands for joint */
    if ( h.mode == 1 )
        band_limit_stereo = 18 * nsbstereo_limit;
    else
        band_limit_stereo = band_limit;
    if ( band_limit_stereo > band_limit )
        band_limit_stereo = band_limit;

/*------- input filter function selection ----*/
    filter2_init ( samprate, ec.filter_select, monodual, &fc2 );

    bytes_in = ( 1 + monodual ) * ( sizeof ( short ) * 1152 );

/*--- init encoder tables ----*/

/*--- init encoder subroutines after table init ---*/
    sbt_init (  );
    xsbt_init (  );     // P3 simd

/*-- init layer III tables */
// must init tables before acoustic 
// amod init need code band table generation
    L3table_init ( sr_index, h_id, band_limit );

// acoustic init
    amod_initLong ( sr_index, nsb_limit, h_id, nsum, spd_cntl, w_spd );

    amod_initShort ( sr_index, nsb_limit, h_id,
                     nsumShort, spd_cntlShort, w_spdShort );

    for ( k = 0; k < 2; k++ )
    {
        for ( j = 0; j < 2; j++ )
        {
            for ( i = 0; i < 64; i++ )
                ecsave[k][j][i] = 1.0e20f;
        }
    }

//=============================================

/*--- bit allo init ---*/
    ms_flag = is_flag = 0;
    if ( h.mode == 1 )
    {
        if ( nsbstereo_limit < nsb_limit )
            is_flag = 1;
        //if( h.id == 0 ) is_flag = 1; // 12/22/98 some problem somewhere
        // must have is_flag=1 if old bitAllo
        //  re. Her Majesty (Abbey17)
        ms_flag = 1;
    }
    if ( is_flag )
        ec.vbr_flag = 0;

//-------- vbr
    if ( h.id == 1 )
    {   // MPEG1
        if ( ec.vbr_flag )
        {
            gen_vbr_table ( h.mode, divisor, 2 * ec.vbr_br_limit );
            // AveTargetBits reset by gen_vbr_table
            iL3_audio_encode_function = iL3_audio_encode_vbr_MPEG1;
        }
    }
    else
    {   // mpeg 2
        if ( ec.vbr_flag )
        {
            gen_vbr_table ( h.mode, divisor, 2 * ec.vbr_br_limit );
            // AveTargetBits reset by gen_vbr_table
            iL3_audio_encode_function = iL3_audio_encode_vbr_MPEG2;
        }
    }

    if ( ec.vbr_flag )
    {
        initialMNR = 10 * ec.vbr_mnr;   // initialMNR in mb
        if ( initialMNR < 210 )
            initialMNR = 210;
        if ( initialMNR > 1500 )
            initialMNR = 1500;
    }
    else
    {
        tmp = totbitrate / nchan;
        if ( h_id == 1 )
        {       // mpeg1
            initialMNR = 125 * ( tmp - 32 ) / 8;
        }
        else
        {       // mpeg2
            initialMNR = 10 * ( ( 30 * tmp ) / 8 - 70 );
        }
        if ( initialMNR < 0 )
            initialMNR = 0;
        if ( initialMNR > 1000 )
            initialMNR = 1000;
    }

    ec.vbr_delta_mnr = HX_MIN ( ec.vbr_delta_mnr, 50 );
    ec.vbr_delta_mnr = HX_MAX ( ec.vbr_delta_mnr, -40 );
    for ( i = 0; i < 21; i++ )
    {
        ec.mnr_adjust[i] = HX_MIN ( ec.mnr_adjust[i], 200 );
        ec.mnr_adjust[i] = HX_MAX ( ec.mnr_adjust[i], -200 );
    }

/*------- select encoder and bitallo routines ------------*/
// bitallo1 - handles dual and intensity, no short blocks, no vbr
// bitallo3 - newer/faster/better. short blocks. vbr. no dual or intensity

    if ( h.id == 1 )
    {   // mpeg1
        switch ( h.mode )
        {
        case 0: // stereo
            // bitallo3
            BitAllo = new CBitAllo3;
            iencode_function = iencode_jointB;
            break;
        case 1: // joint
            if ( is_flag == 0 )
            {
                // bitallo3
                BitAllo = new CBitAllo3;
                iencode_function = iencode_jointB;
            }
            else
            {   // intensity in use
                // bitallo1  
                BitAllo = new CBitAllo1;
                iencode_function = iencode_jointA;
            }
            break;
        case 2: // dual
            // bitallo1
            BitAllo = new CBitAllo1;
            iencode_function = iencode_singleA;
            break;
        case 3: // mono
            // bitallo3
            BitAllo = new CBitAllo3;
            iencode_function = iencode_singleB;
            break;
        }       // end switch
    }
    else
    {   // mpeg2
        BitAllo = new CBitAllo1;
        switch ( h.mode )
        {
        case 0: // stereo
            // bitallo3
            BitAllo = new CBitAllo3;
            iencode_function = iencode_jointB_MPEG2;
            break;
        case 1: // joint
            if ( is_flag == 0 )
            {
                // bitallo3
                BitAllo = new CBitAllo3;
                iencode_function = iencode_jointB_MPEG2;
            }
            else
            {   // intensity in use - bitallo3 can't handle
                // bitallo1  
                BitAllo = new CBitAllo1;
                iencode_function = iencode_jointA_MPEG2;
            }
            break;
        case 2: // dual
            // bitallo1
            BitAllo = new CBitAllo1;
            iencode_function = iencode_singleA_MPEG2;
            break;
        case 3: // mono
            // bitallo3
            BitAllo = new CBitAllo3;
            iencode_function = iencode_singleB_MPEG2;
            break;
        }       // end switch
    }

// init bitallo routine 
    bac.band_limit_left = band_limit;
    bac.band_limit_right = band_limit_stereo;
    bac.is_flag = is_flag;
    bac.h_id = h.id;
    bac.hf_flag = ec.hf_flag;
    bac.initialMNR = initialMNR;
    bac.vbr_flag = ec.vbr_flag;
    bac.MNRbias = 10 * ec.vbr_delta_mnr;
    bac.disable_taper = ec.quick;
    bac.mnr_adjust = ec.mnr_adjust;

    bac.test1 = ec.test1;
    if ( bac.test1 < 0 )
        bac.test1 = 6;  // default 6 re. Neal
    bac.test2 = ec.test2;
    bac.test3 = ec.test3;

    nsf_stereo = BitAllo->BitAlloInit ( bac );

/*--- clear encoder buffers  ----*/
// clear scfsi for second time around, 
//      some sf pack routines assume 0
    side_info.scfsi[0] = side_info.scfsi[1] = 0;
    memset ( sf, 0, sizeof ( sf ) );

    memset ( sig_mask, 0, sizeof ( sig_mask ) );        // clear entire buf short and long
    for ( i = 0; i < 36; i++ )
    {
        sig_mask[0][i].sig = sig_mask[0][i].mask =
            sig_mask[1][i].sig = sig_mask[1][i].mask = 100.0;
    }

    for ( i = 0; i < ( sizeof ( buf1 ) / sizeof ( float ) ); i++ )
        buf1[i] = buf2[i] = 0.0f;
    memset ( sample, 0, sizeof ( sample ) );
    for ( i = 0; i < 576; i++ )
    {
        xr[0][0][i] = xr[0][1][i] = xr[1][0][i] = xr[1][1][i] = 0.0f;
        //sample[0][0][i] = sample[0][1][i] = 
        //                sample[1][0][i] = sample[1][1][i] = 0.0f;
    }
    xr_clear_flag[0][0] = xr_clear_flag[0][1] = 0;
    xr_clear_flag[1][0] = xr_clear_flag[1][1] = 0;

    for ( i = 0; i < 576; i++ )
    {
        ix[0][i] = ix[1][i] = 0;
        signx[0][i] = signx[1][i] = 0;
    }

    for ( i = 0; i < 2; i++ )
    {
        for ( j = 0; j < 2; j++ )
        {
            for ( k = 0; k < 21; k++ )
                sf[i][j].l[k] = 0;
            for ( k = 0; k < 12; k++ )
                sf[i][j].s[2][k] = sf[i][j].s[1][k] = sf[i][j].s[0][k] = 0;
        }
    }

    main_p0 = main_p1 = 0;
    side_p0 = side_p1 = 0;
    main_sent = main_bytes = 0;
    main_tot = mf_tot = 0;

    tot_frames_out = 0;
    tot_bytes_out = 0;
    ave_tot_bytes_out = 0;

/* save as encoded control data for info return */
    ec_global = ec;

    ec_global.mode = h.mode;
    ec_global.bitrate = totbitrate;
    if ( h.mode != 3 )
        ec_global.bitrate /= 2;
    ec_global.samprate = samprate;
    ec_global.nsbstereo = nsbstereo;
    if ( is_flag == 0 )
        ec_global.nsbstereo = 32;       // signal is useage
//ec_global.filter_select = filter_select;


    if(ec.hf_flag)
    {
        ec_global.freq_limit = ec.freq_limit;
    }
    else
    {
        ec_global.freq_limit = nsb_limit * ( samprate / 64 );
    }
        
    ec_global.nsb_limit = nsb_limit;

/* acoustic model */
//ec_global.npass =         2;
//ec_global.npred =         npred;
    ec_global.layer = 4 - h.option;

    return bytes_in;    /* 0 = error */
}

/*--------------------------------------------------------------------*/
void
CMp3Enc::pack_head_local ( MPEG_HEAD * h, unsigned char *outbuf )
{
    L3_outbits_init ( outbuf );

    L3_outbits ( 0xFFF, 12 );
    L3_outbits ( h->id, 1 );
    L3_outbits ( h->option, 2 );
    L3_outbits ( h->prot, 1 );

    L3_outbits ( h->br_index, 4 );
    L3_outbits ( h->sr_index, 2 );
    L3_outbits ( h->pad, 1 );
    L3_outbits ( h->private_bit, 1 );

    L3_outbits ( h->mode, 2 );
    L3_outbits ( h->mode_ext, 2 );
    L3_outbits ( h->cr, 1 );
    L3_outbits ( h->original, 1 );
    L3_outbits ( h->emphasis, 2 );

    L3_outbits_flush (  );
}

/*--------------------------------------------------------------------*/
int
CMp3Enc::calc_freq_limit_L3 ( int samprate, int totbitrate, int mode )
{
    float chan_bitrate;
    int flimit;
    static const float factor[4] = { 1.1f, 1.333f, 1.0f, 1.0f };

// mod 11/16/98 

    if ( samprate < 8000 )
        samprate = 8000;

    chan_bitrate = totbitrate;
    if ( mode != 3 )
        chan_bitrate = 0.5 * chan_bitrate;
    chan_bitrate = factor[mode] * chan_bitrate;

    if ( samprate < 32000 )
    {   // 4000@16  7500@32 11000@42.7(64 joint)
        if ( chan_bitrate <= 32.0f )
            flimit = ( int ) ( 752.0 + 203.0 * chan_bitrate );
        else if ( chan_bitrate <= 42.7f )
            flimit = ( int ) ( -2967.0 + 327.0 * chan_bitrate );
        else
            flimit = 11000;
    }
    else
    {
        // 8/11/99 
        // flimit set to match FhG at 44100kHz for mode 1
        // 17 bins at 96kbps  f = 11714
        // 23 bins at 128kbps f = 15848 (max bins at 44.1 is 24 fo f = 16537)
        flimit = ( int ) ( 187.97 * chan_bitrate );
        //flimit = (int)(234.375*chan_bitrate);
    }

    return flimit;
}

/*--------------------------------------------------------------------*/
void
CMp3Enc::acoustic_model ( int igr, int block_type, int block_type_prev )
{
    int i;

/* caller passes start point of 1024 pt fft */
    for ( i = 0; i < nchan; i++ )
    {
        L3acousticQuick ( i, igr, block_type, block_type_prev );
    }

}

/*--------------------------------------------------------------------*/
static const int vbr_br_table[16] = {
    0, 32, 40, 48, 56, 64, 80, 96,
    112, 128, 160, 192, 224, 256, 320,
};

static const int vbr_br_tableMPEG2[16] = {
    0, 8, 16, 24, 32, 40, 48, 56,
    64, 80, 96, 112, 128, 144, 160,
};

/*--------------------------------------------------------------------*/
void
CMp3Enc::gen_vbr_table ( int h_mode, int samplerate, int max_tot_bitrate )
{
    int i;
    int main_bytes;
    int side_bytes;

    if ( h_id == 1 )
    {   // MPEG1
        if ( h_mode == 3 )
            side_bytes = 17;
        else
            side_bytes = 32;

        for ( i = 1; i < 15; i++ )
        {
            main_bytes = 144000 * vbr_br_table[i] / samplerate;
            vbr_framebytes[i] = main_bytes;
            main_bytes -= 4;    /* less header */
            main_bytes -= side_bytes;   /* less side */
            vbr_main_framebytes[i] = main_bytes;
        }
        vbr_framebytes[15] = 9999999;
        vbr_main_framebytes[15] = 9999999;

        vbr_pool_target = 256;  // use more pool with restricted br
        for ( i = 14; i >= 12; i-- )
        {
            if ( max_tot_bitrate >= vbr_br_table[i] )
                break;
            vbr_pool_target = ( vbr_pool_target + 511 ) >> 1;   // use more pool with restricted br

        }
        ivbr_max = i;
        ivbr_min = 1;   // min and max vbr bitrate_index

        //AveTargetBits is a per channel per granule number 
        // used by bitallo2 for bit pool control in vbr mode
        // always comes out less that part23 max
        AveTargetBits =
            ( 8 * vbr_main_framebytes[ivbr_max] / ( 2 * nchan ) ) -
            sf_bit_max;
    }
    else
    {   //  MPEG2
        if ( h.mode == 3 )
            side_bytes = 9;
        else
            side_bytes = 17;

        for ( i = 1; i < 15; i++ )
        {
            main_bytes = 72000 * vbr_br_tableMPEG2[i] / samplerate;
            vbr_framebytes[i] = main_bytes;
            main_bytes -= 4;    /* less header */
            main_bytes -= side_bytes;   /* less side */
            vbr_main_framebytes[i] = main_bytes;
        }
        vbr_framebytes[15] = 9999999;
        vbr_main_framebytes[15] = 9999999;

        vbr_pool_target = 128;  // use more pool with restricted br
        for ( i = 14; i >= 12; i-- )
        {
            if ( max_tot_bitrate >= vbr_br_tableMPEG2[i] )
                break;
            vbr_pool_target = ( vbr_pool_target + 255 ) >> 1;   // use more pool with restricted br
        }
        ivbr_max = i;
        ivbr_min = 1;   // min and max vbr bitrate_index

        //AveTargetBits is a per channel per granule number 
        // used by bitallo2 for bit pool control in vbr mode
        // always comes out less that part23 max
        AveTargetBits =
            ( 8 * vbr_main_framebytes[ivbr_max] / ( nchan ) ) - sf_bit_max;
    }

}

/*====================================================================*/
void
CMp3Enc::transform_igr ( int igr )
{
    int ch;
    int btype;
    int igrx_prev;
    int igrx_ahead;

// transform single selected igr
//
// sbt done one gr ahead of time (for use by short block detector)
// this introduces additional (over older transform) 576 sample
// delay. This has alignment implications for old (fft) acoustic model

// igrx applies to sample buffer only

    igrx_prev = ( igrx - 1 ) & 3;
    igrx_ahead = ( igrx + 2 ) & 3;

    if ( cpu_select != 2 )
    {
        // fpu transform 
        for ( ch = 0; ch < nchan; ch++ )
        {
            FreqInvert ( sample[ch][igrx], nsb_limitMS[0] );
            btype = side_info.gr[igr][ch].block_type;
            if ( btype != 2 )
            {   // long block types
                hybridLong ( sample[ch][igrx_prev], sample[ch][igrx],
                             xr[igr][ch], btype, nsb_limitMS[0],
                             xr_clear_flag[igr][ch] );
                xr_clear_flag[igr][ch] = 0;
                antialias ( xr[igr][ch], nsb_limitMS[0] );
            }
            else
            {   // short block
                hybridShort ( sample[ch][igrx_prev], sample[ch][igrx],
                              xr[igr][ch], nsb_limitMS[0] );
                xr_clear_flag[igr][ch] = 1;     // xr to be cleared of short data
            }
            sbt_L3 ( audio_buf[ch][igr], sample[ch][igrx_ahead] );      // do sbt ahead of time
        }
    }
    else
    {
        // Pentium III  sse transform 
        for ( ch = 0; ch < nchan; ch++ )
        {
            FreqInvert ( sample[ch][igrx], nsb_limitMS[0] );
            btype = side_info.gr[igr][ch].block_type;
            if ( btype != 2 )
            {   // long block types
                hybridLong ( sample[ch][igrx_prev], sample[ch][igrx],
                             xr[igr][ch], btype, nsb_limitMS[0],
                             xr_clear_flag[igr][ch] );
                xr_clear_flag[igr][ch] = 0;
                xantialias ( xr[igr][ch], nsb_limitMS[0] );
            }
            else
            {   // short block
                hybridShort ( sample[ch][igrx_prev], sample[ch][igrx],
                              xr[igr][ch], nsb_limitMS[0] );
                xr_clear_flag[igr][ch] = 1;     // xr to be cleared of short data
            }
            xsbt_L3 ( audio_buf[ch][igr], sample[ch][igrx_ahead] );     // do sbt ahead of time
        }
    }

    igrx = ( igrx + 1 ) & 3;    // inc circ sample buf

}

/*====================================================================*/

/*xxxxxxxxxxxxxxxxxxxxxxxx
void CMp3Enc::transform()
{
int ch;
int btype; 
int igr, igr_prev;

// note this has different alignment (delay) than transform_igr

if( cpu_select != 2 ) {
    // fpu transform 
    for(igr=0;igr<2;igr++) {
        igr_prev = igr^1;
        for(ch=0;ch<nchan;ch++) {
            FreqInvert(sample[ch][igr], nsb_limitMS[0]);
            btype = side_info.gr[igr][ch].block_type;
            if( btype != 2 ) {      // long block types
                hybridLong(sample[ch][igr_prev], sample[ch][igr], xr[igr][ch],
                    btype, nsb_limitMS[0], xr_clear_flag[igr][ch]);
                xr_clear_flag[igr][ch] = 0;
                antialias(xr[igr][ch], nsb_limitMS[0]);
            }
            else {                  // short block
                hybridShort(sample[ch][igr_prev], sample[ch][igr], xr[igr][ch],
                    nsb_limitMS[0]);
                xr_clear_flag[igr][ch] = 1;  // xr will to be cleared of short data
            }
            sbt_L3(audio_buf[ch][igr], sample[ch][igr_prev]);  // do sbt one igr ahead of time
        }
    }
}
else {
    // Pentium III  sse transform 
    for(igr=0;igr<2;igr++) {
        igr_prev = igr^1;
        for(ch=0;ch<nchan;ch++) {
            FreqInvert(sample[ch][igr_prev], nsb_limitMS[0]);
            btype = side_info.gr[igr][ch].block_type;
            if( btype != 2 ) {      // long block types
                hybridLong(sample[ch][igr], sample[ch][igr_prev], xr[igr][ch],
                    btype, nsb_limitMS[0], xr_clear_flag[igr][ch]);
                xr_clear_flag[igr][ch] = 0;
                xantialias(xr[igr][ch], nsb_limitMS[0]);
            }
            else {                  // short block
                hybridShort(sample[ch][igr], sample[ch][igr_prev], xr[igr][ch],
                    nsb_limitMS[0]);
                xr_clear_flag[igr][ch] = 1;  // xr to be cleared of short data
            }
            xsbt_L3(audio_buf[ch][igr], sample[ch][igr]);  // do sbt one igr ahead of time
        }
    }

}

}
//====================================================================
void CMp3Enc::transform_MPEG2()
{
int ch;
int btype; 
int igr_prev;

// note this has different alignment (delay) than transform_igr

igr_prev = igr^1;
for(ch=0;ch<nchan;ch++) {
    sbt_L3(audio_buf[ch][igr], sample[ch][igr_prev]);
    FreqInvert(sample[ch][igr_prev], nsb_limit);
    btype = side_info.gr[igr][ch].block_type;
    if( btype != 2 ) {    // long block types
        hybridLong(sample[ch][igr], sample[ch][igr_prev], xr[igr][ch],
            btype, nsb_limit, xr_clear_flag[igr][ch]);
        xr_clear_flag[igr][ch] = 0;
        antialias(xr[igr][ch], nsb_limit);
    }
    else {
        hybridShort(sample[ch][igr], sample[ch][igr_prev], xr[igr][ch],
            nsb_limit);
        xr_clear_flag[igr][ch] = 1;  // xr to be cleared of short data
    }

}

}
xxxxxxxxxxxxxxxxxxx*/

/*====================================================================*/
int
CMp3Enc::encode_function (  )
{

    switch ( iencode_function )
    {
    case 0:
        return encode_jointA (  );
    case 1:
        return encode_jointB (  );
    case 2:
        return encode_singleA (  );
    case 3:
        return encode_singleB (  );

    case 4:
        return encode_jointA_MPEG2 (  );
    case 5:
        return encode_jointB_MPEG2 (  );
    case 6:
        return encode_singleA_MPEG2 (  );
    case 7:
        return encode_singleB_MPEG2 (  );
    }

    return 0;
}

/*====================================================================*/
int
CMp3Enc::encode_jointA (  )
{
    int ch;
    int bits;
    int bit_min, bit_max;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int TargetBits;
    int sf_bits;
    int m1, m2;
    int ms_allocate;
    int null_bit_pool = 0;      // not used by bitall1

// bitallo1

/* simultaneous channel encoding - two channel only */
    TargetBits = AveTargetBits + AveTargetBits;

/* compute both chan, per gr, bit parameters */
    bit_max = ( byte_max << 2 );
    bit_min = ( byte_min << 2 );

    ba_bit_max = bit_max;
    if ( ba_bit_max > 4095 )
        ba_bit_max = 4095;      /* limited by part2_3 */
    ba_bit_min = bit_min;
    sf_bits = sf_bit_max + sf_bit_max;
    ba_bit_max -= sf_bits;
    ba_bit_min -= sf_bits;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

//----

    transform_igr ( 0 );        // igr = 0
    transform_igr ( 1 );        // igr = 1

    ms_allocate = 0;
    if ( ms_flag )
    {   // decide ms or stereo encoding 
        m1 = BitAllo->ms_correlation2 ( xr[0], 0 );
        m2 = BitAllo->ms_correlation2 ( xr[1], 0 );
        if ( ( m1 + m2 ) >= 0 )
            ms_allocate = 1;
    }

    for ( igr = 0; igr < 2; igr++ )
    {
        acoustic_model ( igr );
        BitAllo->BitAllo ( xr[igr], sig_mask, 0, 2,     // nchan=2
                           ba_min, TargetBits, ba_max, null_bit_pool,
                           &sf[igr][0], &side_info.gr[igr][0], ix, signx,
                           ms_allocate );
        //BitAllo(xr[igr][0], sig_mask[0], 0, 2,  // nchan=2
        //           ba_min, TargetBits, ba_max,
        //            &sf[igr][0], &side_info.gr[igr][0], signx[0],
        //            ms_allocate);
        for ( ch = 0; ch < nchan; ch++ )
        {
            bits = 0;
            side_info.gr[igr][ch].scalefac_compress =
                L3_pack_sf_MPEG1B2 ( &sf[igr][ch], ch, igr,
                                     &side_info.scfsi[ch],
                                     side_info.gr[igr][ch].aux_not_null );
            if ( side_info.gr[igr][ch].aux_not_null )
            {   /* pack main data */
                //side_info.gr[igr][ch].scalefac_compress = 
                //L3_pack_sf_MPEG1(&sf[igr][ch]);
                //   L3_pack_sf_MPEG1B(&sf[igr][ch], ch, igr, &side_info.scfsi[ch]);
                bits =
                    L3_pack_huff ( &side_info.gr[igr][ch], ix[ch],
                                   signx[ch] );
            }
            ba_min -= bits;
            ba_max -= bits;
            side_info.gr[igr][ch].part2_3_length = bits;
        }
        ba_min += ba_bit_min + sf_bits;
        ba_max += ba_bit_max + sf_bits;
    }

    return ms_allocate;

/* done */
}

/*====================================================================*/
void
CMp3Enc::blocktype_selectB_igr_single ( int igr )
{
    int igrx_ahead;
    int igr_prev;
    int prev, cur, next;
    int short_flag;
    int t;

    igr_prev = igr ^ 1;
    igrx_ahead = ( igrx + 1 ) & 3;      // applies to sample only
// igrx_ahead points to most recent sbt done by transform_igr

    t = attack_detectSBT_igr ( sample[0][igrx_ahead], attack_buf[0],
                               side_info.gr[igr_prev][0].short_flag_next );
    short_flag = 0;
    if ( t > short_block_threshold ) 
    {
        short_flag = 1;
    }

// block_type same in both channels for now, this most bit efficient
// most of the time anyway.

    side_info.gr[igr][0].short_flag_next = short_flag;
    side_info.gr[igr][0].short_flag_current =
        side_info.gr[igr_prev][0].short_flag_next;
    side_info.gr[igr][0].block_type_prev =
        side_info.gr[igr_prev][0].block_type;
    prev = side_info.gr[igr][0].block_type_prev;
    cur = side_info.gr[igr][0].short_flag_current;
    next = side_info.gr[igr][0].short_flag_next;
    side_info.gr[igr][0].block_type = side_info.gr[igr][1].block_type =
        block_type_select[prev][cur][next];

}

/*====================================================================*/
void
CMp3Enc::blocktype_selectB_igr_single_MPEG2 ( int igr )
{
    int igrx_ahead;
    int igr_prev;
    int prev, cur, next;
    int short_flag;
    int t;

    igr_prev = igr ^ 1;
    igrx_ahead = ( igrx + 1 ) & 3;      // applies to sample only

    t = attack_detectSBT_igr_MPEG2 ( sample[0][igrx_ahead], attack_buf[0],
                                     side_info.gr[igr_prev][0].
                                     short_flag_next );
    short_flag = 0;
    if ( t > short_block_threshold ) 
    {
        short_flag = 1;
    }

    side_info.gr[igr][0].short_flag_next = short_flag;
    side_info.gr[igr][0].short_flag_current =
        side_info.gr[igr_prev][0].short_flag_next;
    side_info.gr[igr][0].block_type_prev =
        side_info.gr[igr_prev][0].block_type;
    prev = side_info.gr[igr][0].block_type_prev;
    cur = side_info.gr[igr][0].short_flag_current;
    next = side_info.gr[igr][0].short_flag_next;
    side_info.gr[igr][0].block_type = side_info.gr[igr][1].block_type =
        block_type_select[prev][cur][next];

}

/*====================================================================*/
void
CMp3Enc::blocktype_selectB_igr_dual ( int igr )
{
    int igrx_ahead;
    int igr_prev;
    int prev, cur, next;
    int short_flag;
    int v1, v2;

    igr_prev = igr ^ 1;
    igrx_ahead = ( igrx + 1 ) & 3;      // applies to sample only

    v1 = attack_detectSBT_igr ( sample[0][igrx_ahead], attack_buf[0],
                                side_info.gr[igr_prev][0].short_flag_next );
    v2 = attack_detectSBT_igr ( sample[1][igrx_ahead], attack_buf[1],
                                side_info.gr[igr_prev][0].short_flag_next );
    short_flag = 0;


    if( v1 > short_block_threshold )
    {
        short_flag = 1;
    }
    if( v2 > short_block_threshold )
    {
        short_flag = 1;
    }


// block_type same in both channels for now, this most bit efficient
// most of the time anyway.

    side_info.gr[igr][0].short_flag_next = short_flag;
    side_info.gr[igr][0].short_flag_current =
        side_info.gr[igr_prev][0].short_flag_next;
    side_info.gr[igr][0].block_type_prev =
        side_info.gr[igr_prev][0].block_type;
    prev = side_info.gr[igr][0].block_type_prev;
    cur = side_info.gr[igr][0].short_flag_current;
    next = side_info.gr[igr][0].short_flag_next;
    side_info.gr[igr][0].block_type = side_info.gr[igr][1].block_type =
        block_type_select[prev][cur][next];

}

/*====================================================================*/
void
CMp3Enc::blocktype_selectB_igr_dual_MPEG2 ( int igr )
{
    int igrx_ahead;
    int igr_prev;
    int prev, cur, next;
    int short_flag;
    int v1, v2;

    igr_prev = igr ^ 1;
    igrx_ahead = ( igrx + 1 ) & 3;      // applies to sample only

    v1 = attack_detectSBT_igr_MPEG2 ( sample[0][igrx_ahead], attack_buf[0],
                                      side_info.gr[igr_prev][0].
                                      short_flag_next );
    v2 = attack_detectSBT_igr_MPEG2 ( sample[1][igrx_ahead], attack_buf[1],
                                      side_info.gr[igr_prev][0].
                                      short_flag_next );
    short_flag = 0;

    if( v1 > short_block_threshold )
    {
        short_flag = 1;
    }
    if( v2 > short_block_threshold )
    {
        short_flag = 1;
    }


// block_type same in both channels for now, this most bit efficient
// most of the time anyway.
    side_info.gr[igr][0].short_flag_next = short_flag;
    side_info.gr[igr][0].short_flag_current =
        side_info.gr[igr_prev][0].short_flag_next;
    side_info.gr[igr][0].block_type_prev =
        side_info.gr[igr_prev][0].block_type;
    prev = side_info.gr[igr][0].block_type_prev;
    cur = side_info.gr[igr][0].short_flag_current;
    next = side_info.gr[igr][0].short_flag_next;
    side_info.gr[igr][0].block_type = side_info.gr[igr][1].block_type =
        block_type_select[prev][cur][next];

}

/*====================================================================*/

/*====================================================================*/
int
CMp3Enc::encode_jointB (  )
{
    int ch;
    int bits;
    int bit_pool, bit_min, bit_max;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int TargetBits;
    int sf_bits;
    int m1, m2;
    int ms_allocate;
    int dba_max;
    int shortblock_frame;

// bitallo2
// bitallo2 does its own part23 bit limiting 

/* simultaneous channel encoding - two channel only */

    TargetBits = AveTargetBits + AveTargetBits;

/* compute both chan, per gr, bit parameters */
    bit_pool = byte_pool << 2;
    bit_max = byte_max << 2;
    bit_min = byte_min << 2;

    sf_bits = sf_bit_max + sf_bit_max;
    ba_bit_max = bit_max - sf_bits;
    ba_bit_min = bit_min - sf_bits;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

    dba_max = bit_pool >> 2;    // 
    ba_max = ba_max + dba_max;

// determine block type and transform
    blocktype_selectB_igr_dual ( 0 );
    transform_igr ( 0 );        // igr = 0
    blocktype_selectB_igr_dual ( 1 );
    transform_igr ( 1 );        // both igr
// just check ch=0, for now block_type same in both channels
    shortblock_frame = ( side_info.gr[0][0].block_type == 2 ) |
        ( side_info.gr[1][0].block_type == 2 );

    ms_allocate = 0;
    if ( ms_flag )
    {   // decide ms or stereo encoding, block_type from ch=0 
        m1 = BitAllo->ms_correlation2 ( xr[0],
                                        side_info.gr[0][0].block_type );
        m2 = BitAllo->ms_correlation2 ( xr[1],
                                        side_info.gr[1][0].block_type );
        if ( ( m1 + m2 ) >= 0 )
            ms_allocate = 1;
    }

    for ( igr = 0; igr < 2; igr++ )
    {
        acoustic_model ( igr, side_info.gr[igr][0].block_type,
                         side_info.gr[igr][0].block_type_prev );
        BitAllo->BitAllo ( xr[igr], sig_mask, 0, 2,     // nchan=2
                           ba_min, TargetBits, ba_max, bit_pool,
                           &sf[igr][0], &side_info.gr[igr][0], ix, signx,
                           ms_allocate );
        for ( ch = 0; ch < nchan; ch++ )
        {
            bits = 0;
            side_info.gr[igr][ch].scalefac_compress = 0;	// Added 2005-12-20 as bugfix
            if ( shortblock_frame )
            {   // have a short, scfsi = 0
                side_info.scfsi[ch] = 0;
                if ( side_info.gr[igr][ch].aux_not_null )
                {
                    side_info.gr[igr][ch].scalefac_compress =
                        L3_pack_sf_MPEG1 ( &sf[igr][ch],
                                           side_info.gr[igr][ch].block_type );
                }
            }
            else
            {   // all granules are long,  use scfsi
                side_info.gr[igr][ch].scalefac_compress =
                    L3_pack_sf_MPEG1B2 ( &sf[igr][ch], ch, igr,
                                         &side_info.scfsi[ch],
                                         side_info.gr[igr][ch].aux_not_null );

            }
            if ( side_info.gr[igr][ch].aux_not_null )
            {   /* pack main data */
                bits =
                    L3_pack_huff ( &side_info.gr[igr][ch], ix[ch],
                                   signx[ch] );
            }
            ba_min -= bits;
            ba_max -= bits;
            side_info.gr[igr][ch].part2_3_length = bits;
        }
        ba_min += ba_bit_min + sf_bits;
        ba_max = ba_max - dba_max;
        ba_max += ba_bit_max + sf_bits;

    }

    return ms_allocate;

/* done */
}

/*====================================================================*/
int
CMp3Enc::encode_singleA (  )
{
    int ch;
    int bits;
    int bit_min, bit_max;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int null_bit_pool = 0;

/* independent channel encoding */
// can do mono or dual channel
// bitallo1 required for dual channel

// use L3_pack_sf_MPEG1B2 ???

/* compute per chan, per gr, bit parameters */
    if ( stereo_flag )
    {
        bit_max = ( byte_max << 1 );
        bit_min = ( byte_min << 1 );
    }
    else
    {
        bit_max = ( byte_max << 2 );
        bit_min = ( byte_min << 2 );
    }

    ba_bit_max = bit_max;
    if ( ba_bit_max > 4095 )
        ba_bit_max = 4095;      /* limited by part2_3 */
    ba_bit_min = bit_min;
    ba_bit_max -= sf_bit_max;
    ba_bit_min -= sf_bit_max;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

    transform_igr ( 0 );        // igr = 0
    transform_igr ( 1 );        // igr = 1

    for ( igr = 0; igr < 2; igr++ )
    {
        acoustic_model ( igr );
        for ( ch = 0; ch < nchan; ch++ )
        {
            BitAllo->BitAllo ( xr[igr] + ch, sig_mask + ch, ch, 1,      // nchan=1
                               ba_min, AveTargetBits, ba_max, null_bit_pool,
                               &sf[igr][ch], &side_info.gr[igr][ch],
                               ix + ch, signx + ch, ms_flag );
            bits = 0;
            side_info.gr[igr][ch].scalefac_compress = 0;
            if ( side_info.gr[igr][ch].aux_bits )
            {   /* pack main data */
                side_info.gr[igr][ch].scalefac_compress =
                    L3_pack_sf_MPEG1 ( &sf[igr][ch],
                                       side_info.gr[igr][ch].block_type );
                bits =
                    L3_pack_huff ( &side_info.gr[igr][ch], ix[ch],
                                   signx[ch] );
            }
            ba_min += ba_bit_min + sf_bit_max - bits;
            ba_max += ba_bit_max + sf_bit_max - bits;
            side_info.gr[igr][ch].part2_3_length = bits;
        }

    }

    return 0;   // no ms mode/allocation

/* done */
}

/*====================================================================*/
int
CMp3Enc::encode_singleB (  )
{
    int bits;
    int bit_min, bit_max, bit_pool;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int shortblock_frame;

/* mono only no dual chan for bitallo2 */

/* compute per chan, per gr, bit parameters */
    bit_pool = byte_pool << 2;
    bit_max = byte_max << 2;
    bit_min = byte_min << 2;

    ba_bit_max = bit_max;
    if ( ba_bit_max > 4095 )
        ba_bit_max = 4095;      /* limited by part2_3 */
    ba_bit_min = bit_min;
    ba_bit_max -= sf_bit_max;
    ba_bit_min -= sf_bit_max;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

// determine block type and transform
    blocktype_selectB_igr_dual ( 0 );
    transform_igr ( 0 );        // igr = 0
    blocktype_selectB_igr_dual ( 1 );
    transform_igr ( 1 );        // igr = 1
    shortblock_frame = ( side_info.gr[0][0].block_type == 2 ) |
        ( side_info.gr[1][0].block_type == 2 );

    for ( igr = 0; igr < 2; igr++ )
    {
        acoustic_model ( igr, side_info.gr[igr][0].block_type,
                         side_info.gr[igr][0].block_type_prev );
        BitAllo->BitAllo ( xr[igr], sig_mask, 0, 1,     // nchan=1
                           ba_min, AveTargetBits, ba_max, bit_pool,
                           &sf[igr][0], &side_info.gr[igr][0], ix, signx, 0 );

        bits = 0;
        side_info.gr[igr][0].scalefac_compress = 0;	// Added 2005-12-20 as bugfix
        if ( shortblock_frame )
        {       // have a short, scfsi = 0
            side_info.scfsi[0] = 0;
            if ( side_info.gr[igr][0].aux_not_null )
            {
                side_info.gr[igr][0].scalefac_compress =
                    L3_pack_sf_MPEG1 ( &sf[igr][0],
                                       side_info.gr[igr][0].block_type );
            }
        }
        else
        {       // all granules are long,  use scfsi
            side_info.gr[igr][0].scalefac_compress =
                L3_pack_sf_MPEG1B2 ( &sf[igr][0], 0, igr,
                                     &side_info.scfsi[0],
                                     side_info.gr[igr][0].aux_not_null );

        }

        if ( side_info.gr[igr][0].aux_bits )
        {       /* pack main data */
            bits = L3_pack_huff ( &side_info.gr[igr][0], ix[0], signx[0] );
        }
        ba_min += ba_bit_min + sf_bit_max - bits;
        ba_max += ba_bit_max + sf_bit_max - bits;
        side_info.gr[igr][0].part2_3_length = bits;
    }

    return 0;   // no ms mode/allocation

/* done */
}

/*====================================================================*/
int
CMp3Enc::encode_jointA_MPEG2 (  )
{
    int ch;
    int bits;
    int bit_min, bit_max;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int TargetBits;
    int sf_bits;
    int m1;
    int ms_allocate;
    int null_bit_pool = 0;

// bitallo1  -  no short blocks, may be intensity

/* simultaneous channel encoding - two channel only */
    TargetBits = AveTargetBits + AveTargetBits;

/* compute both chan, one gr, bit parameters */
    bit_max = ( byte_max << 3 );
    bit_min = ( byte_min << 3 );

    if ( byte_pool > 245 )
        bit_min += 40;

    ba_bit_max = bit_max;
    if ( ba_bit_max > 4095 )
        ba_bit_max = 4095;      /* limited by part2_3 */
    ba_bit_min = bit_min;
    sf_bits = sf_bit_max + sf_bit_max;
    ba_bit_max -= sf_bits;
    ba_bit_min -= sf_bits;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

    transform_igr ( igr );      // one igr

    ms_allocate = 0;
    if ( ms_flag )
    {   // decide ms or stereo encoding 
        //m1 = BitAllo->ms_correlation(xr[igr], band_limit_stereo);
        //if( (m1 < 80) ) ms_allocate = 1;
        m1 = BitAllo->ms_correlation2 ( xr[igr] );
        if ( m1 >= 0 )
            ms_allocate = 1;
    }

    acoustic_model ( igr );
    BitAllo->BitAllo ( xr[igr], sig_mask, 0, 2, // nchan=2
                       ba_min, TargetBits, ba_max, null_bit_pool,
                       &sf[igr][0], &side_info.gr[igr][0],
                       ix, signx, ms_allocate );
//BitAllo(xr[igr][0], sig_mask[0], 0, 2,  // nchan=2
//            ba_min, TargetBits, ba_max,
//            &sf[igr][0], &side_info.gr[igr][0], signx[0],
//            ms_allocate);
    for ( ch = 0; ch < nchan; ch++ )
    {
        bits = 0;
        side_info.gr[igr][ch].scalefac_compress = 0;
        if ( side_info.gr[igr][ch].aux_not_null )
        {       /* pack main data */
            side_info.gr[igr][ch].scalefac_compress =
                L3_pack_sf_MPEG2 ( &sf[igr][ch], ch & is_flag,
                                   nsf_stereo, 12,
                                   side_info.gr[igr][0].block_type );
            bits = L3_pack_huff ( &side_info.gr[igr][ch], ix[ch], signx[ch] );
        }
        side_info.gr[igr][ch].part2_3_length = bits;
    }

    return ms_allocate;

/* done */
}

/*====================================================================*/
int
CMp3Enc::encode_jointB_MPEG2 (  )
{
    int ch;
    int bits;
    int bit_min, bit_max;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int TargetBits;
    int sf_bits;
    int m1;
    int ms_allocate;
    int bit_pool;

// bitallo2/3  may be short blocks, no intensity

/* simultaneous channel encoding - two channel only */
    TargetBits = AveTargetBits + AveTargetBits;

/* compute both chan, one gr, bit parameters */
    bit_pool = byte_pool << 3;
    bit_max = ( byte_max << 3 );
    bit_min = ( byte_min << 3 );

    if ( byte_pool > 245 )
        bit_min += 40;

    ba_bit_max = bit_max;
    if ( ba_bit_max > 4095 )
        ba_bit_max = 4095;      /* limited by part2_3 */
    ba_bit_min = bit_min;
    sf_bits = sf_bit_max + sf_bit_max;
    ba_bit_max -= sf_bits;
    ba_bit_min -= sf_bits;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

    blocktype_selectB_igr_dual_MPEG2 ( igr );
    transform_igr ( igr );      // one igr

    ms_allocate = 0;
    if ( ms_flag )
    {   // decide ms or stereo encoding, block_type from ch=0 
        m1 = BitAllo->ms_correlation2 ( xr[igr],
                                        side_info.gr[igr][0].block_type );
        if ( m1 >= 0 )
            ms_allocate = 1;
        //ms_allocate = 1;
    }

    acoustic_model ( igr, side_info.gr[igr][0].block_type,
                     side_info.gr[igr][0].block_type_prev );

    BitAllo->BitAllo ( xr[igr], sig_mask, 0, 2, // nchan=2
                       ba_min, TargetBits, ba_max, bit_pool,
                       &sf[igr][0], &side_info.gr[igr][0], ix, signx,
                       ms_allocate );
    for ( ch = 0; ch < nchan; ch++ )
    {
        bits = 0;
        side_info.gr[igr][ch].scalefac_compress = 0;
        if ( side_info.gr[igr][ch].aux_not_null )
        {       /* pack main data */
            side_info.gr[igr][ch].scalefac_compress =
                L3_pack_sf_MPEG2 ( &sf[igr][ch], ch & is_flag,
                                   nsf_stereo, 12,
                                   side_info.gr[igr][0].block_type );
            bits = L3_pack_huff ( &side_info.gr[igr][ch], ix[ch], signx[ch] );
        }
        side_info.gr[igr][ch].part2_3_length = bits;
    }

    return ms_allocate;

/* done */
}

/*====================================================================*/
int
CMp3Enc::encode_singleA_MPEG2 (  )
{
    int ch;
    int bits;
    int bit_min, bit_max;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int null_bit_pool = 0;

// L3enc.c encode_single_MPEG2
// bitallo1 - no short blocks 

/* independent channel encoding - mono or dual */

/* compute per chan, bit parameters */
    if ( stereo_flag )
    {
        bit_max = ( byte_max << 2 );
        bit_min = ( byte_min << 2 );
    }
    else
    {
        bit_max = ( byte_max << 3 );
        bit_min = ( byte_min << 3 );
    }

    ba_bit_max = bit_max;
    if ( ba_bit_max > 4095 )
        ba_bit_max = 4095;      /* limited by part2_3 */
    ba_bit_min = bit_min;
    ba_bit_max -= sf_bit_max;
    ba_bit_min -= sf_bit_max;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

    transform_igr ( igr );      // one igr

    acoustic_model ( igr );
    for ( ch = 0; ch < nchan; ch++ )
    {
        BitAllo->BitAllo ( xr[igr] + ch, sig_mask + ch, ch, 1,  // nchan=1
                           ba_min, AveTargetBits, ba_max, null_bit_pool,
                           &sf[igr][ch], &side_info.gr[igr][ch],
                           ix + ch, signx + ch, ms_flag );
        bits = 0;
        side_info.gr[igr][ch].scalefac_compress = 0;
        if ( side_info.gr[igr][ch].aux_bits )
        {       /* pack main data */
            side_info.gr[igr][ch].scalefac_compress =
                L3_pack_sf_MPEG2 ( &sf[igr][ch], 0, 21, 12,
                                   side_info.gr[igr][0].block_type );
            bits = L3_pack_huff ( &side_info.gr[igr][ch], ix[ch], signx[ch] );
        }
        ba_min += ba_bit_min + sf_bit_max - bits;
        ba_max += ba_bit_max + sf_bit_max - bits;
        side_info.gr[igr][ch].part2_3_length = bits;
    }

    return 0;   // no ms mode/allocation

/* done */
}

/*====================================================================*/
int
CMp3Enc::encode_singleB_MPEG2 (  )
{
    int bits;
    int bit_min, bit_max;
    int ba_bit_min, ba_bit_max;
    int ba_min, ba_max;
    int bit_pool;

/* mono only - no dual - bitallo2/3 */

/* compute one igr bit parameters */
    bit_pool = byte_pool << 3;
    bit_max = ( byte_max << 3 );
    bit_min = ( byte_min << 3 );

    ba_bit_max = bit_max;
    if ( ba_bit_max > 4095 )
        ba_bit_max = 4095;      /* limited by part2_3 */
    ba_bit_min = bit_min;
    ba_bit_max -= sf_bit_max;
    ba_bit_min -= sf_bit_max;

    ba_min = ba_bit_min;
    ba_max = ba_bit_max;

// determine block type and transform
    blocktype_selectB_igr_single_MPEG2 ( igr );
    transform_igr ( igr );      // one igr

    acoustic_model ( igr, side_info.gr[igr][0].block_type,
                     side_info.gr[igr][0].block_type_prev );

    BitAllo->BitAllo ( xr[igr], sig_mask, 0, 1, // nchan=1
                       ba_min, AveTargetBits, ba_max, bit_pool,
                       &sf[igr][0], &side_info.gr[igr][0],
                       ix, signx, ms_flag );
    bits = 0;
    side_info.gr[igr][0].scalefac_compress = 0;
    if ( side_info.gr[igr][0].aux_bits )
    {   /* pack main data */
        side_info.gr[igr][0].scalefac_compress =
            L3_pack_sf_MPEG2 ( &sf[igr][0], 0, 21, 12,
                               side_info.gr[igr][0].block_type );
        bits = L3_pack_huff ( &side_info.gr[igr][0], ix[0], signx[0] );
    }
    side_info.gr[igr][0].part2_3_length = bits;

    return 0;   // no ms mode/allocation

/* done */
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode ( short *pcm, unsigned char *bs_out )
{

    switch ( iL3_audio_encode_function )
    {
    default:
    case 0:
        return L3_audio_encode_vbr_MPEG1 ( pcm, bs_out );
    case 1:
        return L3_audio_encode_MPEG1 ( pcm, bs_out );
    case 2:
        return L3_audio_encode_vbr_MPEG2 ( pcm, bs_out );
    case 3:
        return L3_audio_encode_MPEG2 ( pcm, bs_out );
    }

}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_Packet ( short *pcm,
                                  unsigned char *bs_out,
                                  unsigned char *packet, int nbytes_out[2] )
{

    switch ( iL3_audio_encode_function )
    {
    default:
    case 0:
        return L3_audio_encode_vbr_MPEG1Packet ( pcm, bs_out, packet,
                                                 nbytes_out );
    case 1:
        return L3_audio_encode_MPEG1Packet ( pcm, bs_out, packet,
                                             nbytes_out );
    case 2:
        return L3_audio_encode_vbr_MPEG2Packet ( pcm, bs_out, packet,
                                                 nbytes_out );
    case 3:
        return L3_audio_encode_MPEG2Packet ( pcm, bs_out, packet,
                                             nbytes_out );
    }

}

/*====================================================================*/
int
CMp3Enc::L3_pack_head ( unsigned char *bs_out, int pad_flag, int mode_ext )
{
    bs_out[0] = head[0];
    bs_out[1] = head[1];
    bs_out[2] = head[2];
    bs_out[3] = head[3];
    if ( pad_flag )
        bs_out[2] |= 2;
    bs_out[3] = ( bs_out[3] & 0xCF ) | ( mode_ext << 4 );
    return 4;
}

/*--------------------------------------------------------------------*/
int
CMp3Enc::L3_pack_head_vbr ( unsigned char *bs_out,
                            int mode_ext, int bitrate_index )
{
    bs_out[0] = head[0];
    bs_out[1] = head[1];
    bs_out[2] = head[2];
    bs_out[3] = head[3];
    bs_out[3] = ( bs_out[3] & 0xCF ) | ( mode_ext << 4 );
    bs_out[2] = ( bs_out[2] & 0x0F ) | ( bitrate_index << 4 );

    return 4;
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_vbr_MPEG1 ( short *pcm, unsigned char *bs_out )
{
    IN_OUT x;
    int bytes;
    int main_data_begin;
    unsigned char *bs_out0;
    int ms_encoded;
    int bytes2, bytes3;
    int ibr;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

/* no pad for vbr */
    pad = 0;

/* save the frame info */
    frame_info[side_p1].main_pos = main_tot;

    byte_pool = ( mf_tot - main_tot );

    byte_max = vbr_main_framebytes[ivbr_max] + byte_pool;
    byte_min = vbr_main_framebytes[ivbr_min] + byte_pool - 511;

    L3_outbits_init ( main_buf + main_p1 );     // init main data packing

/*===============================================*/

/*===============================================*/
    ms_encoded = encode_function (  );  // will do single or joint

/*=========================================================*/
    mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

    bytes = L3_outbits_flush (  );      /* bytes of main data for this frame */

    assert ( bytes <= byte_max );

// determine br index for frame
// how much should byte pool be used??? none at all if max = 320kbps?
    bytes2 = bytes - byte_pool; // will use entire pool
//bytes3 = bytes2 + 511;          // would fill entire pool
//bytes3 = bytes2 + 256;          // would fill half  pool
    bytes3 = bytes2 + vbr_pool_target;  // would fill pool to vbr_pool_target
    for ( ibr = ivbr_min; ibr <= ivbr_max; ibr++ )
    {   // find min required
        if ( bytes2 <= vbr_main_framebytes[ibr] )
            break;
    }
    for ( ; ibr <= ivbr_max; ibr++ )
    {   // find required for pool fill
        if ( bytes3 < vbr_main_framebytes[ibr + 1] )
            break;
    }
    if ( ibr > ivbr_max )
        ibr = ivbr_max;

    br_index_buf[side_p1] = ibr;
    frame_info[side_p1].mf_bytes = vbr_main_framebytes[ibr];

    if ( bytes < byte_min )
    {   /* pad if necessary */
        memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
        bytes = byte_min;
    }

/* pack side info */
    L3_pack_side_MPEG1 ( side_buf[side_p1], &side_info, nchan );

    main_tot += bytes;
    main_bytes += bytes;
    main_p1 += bytes;
    mf_tot += vbr_main_framebytes[ibr];
    side_p1 = ( side_p1 + 1 ) & 31;     /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
    bs_out0 = bs_out;
    for ( ; side_p0 != side_p1; )
    {
        if ( main_bytes < frame_info[side_p0].mf_bytes )
            break;
        tot_frames_out++;
        main_data_begin = main_sent - frame_info[side_p0].main_pos;
        main_sent += frame_info[side_p0].mf_bytes;
        /* output the data */
        bs_out += L3_pack_head_vbr ( bs_out,
                                     mode_ext_buf[side_p0],
                                     br_index_buf[side_p0] );
        side_buf[side_p0][0] = main_data_begin >> 1;
        side_buf[side_p0][1] |= ( main_data_begin & 1 ) << 7;
        memmove ( bs_out, side_buf[side_p0], side_bytes );
        bs_out += side_bytes;
        memmove ( bs_out, main_buf + main_p0, frame_info[side_p0].mf_bytes );
        bs_out += frame_info[side_p0].mf_bytes;

        main_bytes -= frame_info[side_p0].mf_bytes;
        main_p0 += frame_info[side_p0].mf_bytes;
        side_p0 = ( side_p0 + 1 ) & 31;
    }
    bytesout = bs_out - bs_out0;
    tot_bytes_out += bytesout;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( bytesout << 8 ) - ave_tot_bytes_out ) ) >> 7 );

/* shift main buffer */
    if ( main_p1 > MB_TRIGGER )
    {
        main_p1 = main_p1 - main_p0;
        memmove ( main_buf, main_buf + main_p0, main_p1 );
        main_p0 = 0;
    }

/*---------------------------------------*/

    x.in_bytes = bytes_in;
    x.out_bytes = bytesout;

    return x;
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_MPEG1 ( short *pcm, unsigned char *bs_out )
{
    IN_OUT x;
    int bytes;
    int main_data_begin;
    unsigned char *bs_out0;
    int ms_encoded;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

/* compute pad */
    pad = 0;
    padcount -= remainder;
    if ( padcount <= 0 )
    {
        padcount += divisor;
        pad = 1;
    }

/* save the frame info */
    frame_info[side_p1].main_pos = main_tot;
    frame_info[side_p1].mf_bytes = main_framebytes + pad;

    byte_pool = ( mf_tot - main_tot );

    byte_max = main_framebytes + pad + byte_pool;
    byte_min = byte_max - 511;  // 511 is MPEG1

    L3_outbits_init ( main_buf + main_p1 );     // init main data packing

/*===============================================*/

/*===============================================*/
    ms_encoded = encode_function (  );  // does single or joint

/*=========================================================*/
    mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

    bytes = L3_outbits_flush (  );      /* bytes of main data for this frame */

    assert ( bytes <= byte_max );

    if ( bytes < byte_min )
    {   /* pad if necessary */
        memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
        bytes = byte_min;
    }

/* pack side info */

    L3_pack_side_MPEG1 ( side_buf[side_p1], &side_info, nchan );

    main_tot += bytes;
    main_bytes += bytes;
    main_p1 += bytes;
    mf_tot += main_framebytes + pad;
    side_p1 = ( side_p1 + 1 ) & 31;     /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
    bs_out0 = bs_out;
    for ( ; side_p0 != side_p1; )
    {
        if ( main_bytes < frame_info[side_p0].mf_bytes )
            break;
        tot_frames_out++;
        main_data_begin = main_sent - frame_info[side_p0].main_pos;
        main_sent += frame_info[side_p0].mf_bytes;
        /* output the data */
        bs_out += L3_pack_head ( bs_out,
                                 frame_info[side_p0].mf_bytes -
                                 main_framebytes, mode_ext_buf[side_p0] );
        side_buf[side_p0][0] = main_data_begin >> 1;
        side_buf[side_p0][1] |= ( main_data_begin & 1 ) << 7;
        memmove ( bs_out, side_buf[side_p0], side_bytes );
        bs_out += side_bytes;
        memmove ( bs_out, main_buf + main_p0, frame_info[side_p0].mf_bytes );
        bs_out += frame_info[side_p0].mf_bytes;
        main_bytes -= frame_info[side_p0].mf_bytes;
        main_p0 += frame_info[side_p0].mf_bytes;
        side_p0 = ( side_p0 + 1 ) & 31;
    }
    bytesout = bs_out - bs_out0;
    tot_bytes_out += bytesout;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( bytesout << 8 ) - ave_tot_bytes_out ) ) >> 7 );

/* shift main buffer */
    if ( main_p1 > MB_TRIGGER )
    {
        main_p1 = main_p1 - main_p0;
        memmove ( main_buf, main_buf + main_p0, main_p1 );
        main_p0 = 0;
    }

/*---------------------------------------*/

    x.in_bytes = bytes_in;
    x.out_bytes = bytesout;

    return x;
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_vbr_MPEG2 ( short *pcm, unsigned char *bs_out )
{
    IN_OUT x;
    int bytes, bytes2, bytes3;
    int main_data_begin;
    unsigned char *bs_out0, *bs_out00;
    int ms_encoded;
    int ibr;
    int side_dp;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

    bs_out00 = bs_out;
    x.out_bytes = 0;

    for ( igr = 0; igr < 2; igr++ )
    {   // two frames two igr

/* compute pad - no pad for vbr */
        pad = 0;

/* save the frame info */
        frame_info[side_p1].main_pos = main_tot;

        byte_pool = mf_tot - main_tot;
        byte_max = vbr_main_framebytes[ivbr_max] + byte_pool;
        byte_min = vbr_main_framebytes[ivbr_min] + byte_pool - 255;

        L3_outbits_init ( main_buf + main_p1 ); // init main data packing

/*===============================================*/

/*===============================================*/
        ms_encoded = encode_function (  );      // does single or joint

/*=========================================================*/
        mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

        bytes = L3_outbits_flush (  );  /* bytes of main data for this frame */

        assert ( bytes <= byte_max );

// determine br index for frame
        bytes2 = bytes - byte_pool;     // will use entire pool
        bytes3 = bytes2 + vbr_pool_target;      // would fill pool to vbr_pool_target
        for ( ibr = ivbr_min; ibr <= ivbr_max; ibr++ )
        {       // find min required
            if ( bytes2 <= vbr_main_framebytes[ibr] )
                break;
        }
// limit number of frames in pool
// side_dp indicates number of frames spanned by pool
// min mpeg2 vbr_main_framebytes is small enough to wrap side pointers
// if side_dp is large, consume pool to limit side_dp
        side_dp = ( side_p1 - side_p0 ) & 31;
        if ( side_dp < 10 )
        {       // no pool fill if side_dp is large 
            for ( ; ibr <= ivbr_max; ibr++ )
            {   // find required for pool fill
                if ( bytes3 < vbr_main_framebytes[ibr + 1] )
                    break;
            }
        }
        else if ( side_dp > 15 )
        {       // consume pool with padding via byte_min
            if ( side_dp > 24 )
            {   // consume entire pool
                byte_min = vbr_main_framebytes[ivbr_min] + byte_pool;
            }
            else
            {   // consume some of pool
                byte_min = vbr_main_framebytes[ivbr_min] + ( byte_pool >> 4 );
            }
        }

        if ( ibr > ivbr_max )
            ibr = ivbr_max;
        br_index_buf[side_p1] = ibr;
        frame_info[side_p1].mf_bytes = vbr_main_framebytes[ibr];

        if ( bytes < byte_min )
        {       /* pad if necessary */
            memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
            bytes = byte_min;
        }

/* pack side info */
        L3_pack_side_MPEG2 ( side_buf[side_p1], &side_info, nchan, igr );

        main_tot += bytes;
        main_bytes += bytes;
        main_p1 += bytes;
        mf_tot += vbr_main_framebytes[ibr];
        side_p1 = ( side_p1 + 1 ) & 31; /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
        bs_out0 = bs_out;
        for ( ; side_p0 != side_p1; )
        {
            if ( main_bytes < frame_info[side_p0].mf_bytes )
                break;
            tot_frames_out++;
            main_data_begin = main_sent - frame_info[side_p0].main_pos;
            assert ( main_data_begin < 256 );
            assert ( main_data_begin >= 0 );
            main_sent += frame_info[side_p0].mf_bytes;
            /* output the data */
            bs_out += L3_pack_head_vbr ( bs_out,
                                         mode_ext_buf[side_p0],
                                         br_index_buf[side_p0] );
            side_buf[side_p0][0] = main_data_begin;
            memmove ( bs_out, side_buf[side_p0], side_bytes );
            bs_out += side_bytes;
            memmove ( bs_out, main_buf + main_p0,
                      frame_info[side_p0].mf_bytes );
            bs_out += frame_info[side_p0].mf_bytes;
            main_bytes -= frame_info[side_p0].mf_bytes;
            main_p0 += frame_info[side_p0].mf_bytes;
            side_p0 = ( side_p0 + 1 ) & 31;
        }

/* shift main buffer */
        if ( main_p1 > MB_TRIGGER )
        {
            main_p1 = main_p1 - main_p0;
            memmove ( main_buf, main_buf + main_p0, main_p1 );
            main_p0 = 0;
        }

/*---------------------------------------*/

    }   // end igr loop

    x.in_bytes = bytes_in;
    x.out_bytes = bs_out - bs_out00;
    tot_bytes_out += x.out_bytes;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( x.out_bytes << 8 ) - ave_tot_bytes_out ) ) >> 6 );
    return x;
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_MPEG2 ( short *pcm, unsigned char *bs_out )
{
    IN_OUT x;
    int bytes;
    int main_data_begin;
    unsigned char *bs_out0, *bs_out00;
    int ms_encoded;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

    bs_out00 = bs_out;
    x.out_bytes = 0;

    for ( igr = 0; igr < 2; igr++ )
    {   // two frames two igr

/* compute pad */
        pad = 0;
        padcount -= remainder;
        if ( padcount <= 0 )
        {
            padcount += divisor;
            pad = 1;
        }

/* save the frame info */
        frame_info[side_p1].main_pos = main_tot;
        frame_info[side_p1].mf_bytes = main_framebytes + pad;

        byte_pool = mf_tot - main_tot;

        byte_max = main_framebytes + pad + byte_pool;
        byte_min = byte_max - 255;      // 255 is MPEG2

        L3_outbits_init ( main_buf + main_p1 ); // init main data packing

/*===============================================*/

/*===============================================*/
        ms_encoded = encode_function (  );      // does single or joint

/*=========================================================*/
        mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

        bytes = L3_outbits_flush (  );  /* bytes of main data for this frame */

        assert ( bytes <= byte_max );

        if ( bytes < byte_min )
        {       /* pad if necessary */
            memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
            bytes = byte_min;
        }

/* pack side info */
        L3_pack_side_MPEG2 ( side_buf[side_p1], &side_info, nchan, igr );

        main_tot += bytes;
        main_bytes += bytes;
        main_p1 += bytes;
        mf_tot += main_framebytes + pad;
        side_p1 = ( side_p1 + 1 ) & 31; /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
        bs_out0 = bs_out;
        for ( ; side_p0 != side_p1; )
        {
            if ( main_bytes < frame_info[side_p0].mf_bytes )
                break;
            tot_frames_out++;
            main_data_begin = main_sent - frame_info[side_p0].main_pos;
            assert ( main_data_begin < 256 );
            assert ( main_data_begin >= 0 );
            main_sent += frame_info[side_p0].mf_bytes;
            /* output the data */
            bs_out += L3_pack_head ( bs_out,
                                     frame_info[side_p0].mf_bytes -
                                     main_framebytes, mode_ext_buf[side_p0] );
            side_buf[side_p0][0] = main_data_begin;
            memmove ( bs_out, side_buf[side_p0], side_bytes );
            bs_out += side_bytes;
            memmove ( bs_out, main_buf + main_p0,
                      frame_info[side_p0].mf_bytes );
            bs_out += frame_info[side_p0].mf_bytes;
            main_bytes -= frame_info[side_p0].mf_bytes;
            main_p0 += frame_info[side_p0].mf_bytes;
            side_p0 = ( side_p0 + 1 ) & 31;
        }

/* shift main buffer */
        if ( main_p1 > MB_TRIGGER )
        {
            main_p1 = main_p1 - main_p0;
            memmove ( main_buf, main_buf + main_p0, main_p1 );
            main_p0 = 0;
        }

/*---------------------------------------*/

    }   // end igr loop

    x.in_bytes = bytes_in;
    x.out_bytes = bs_out - bs_out00;
    tot_bytes_out += x.out_bytes;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( x.out_bytes << 8 ) - ave_tot_bytes_out ) ) >> 6 );
    return x;
}

/*====================================================================*/
void
CMp3Enc::L3acousticQuick ( int ch, int igr,
                           int block_type, int block_type_prev )
{

// uses xform energy only, pcm not used 

    if ( block_type != 2 )
    {   // long block types
        emapLong ( xr[igr][ch], eng[0], nsum );
        spd_smrLongEcho ( eng[0], ecsave[ch][0],
                          spd_cntl, w_spd, sig_mask[ch], block_type );
    }
    else
    {
        emapShort ( xr[igr][ch], eng, nsumShort );
        spd_smrShort ( eng, ecsave[ch][0],
                       spd_cntlShort, w_spdShort, sig_mask[ch],
                       block_type_prev );
    }

}

/*====================================================================*/

/*==================================================================*/

/*===== encode with rate conversion layer, 8 or 16 bit input =======*/

/*==================================================================*/

/*==================================================================*/
static const int rate_table[6] = {
    22050, 24000, 16000, 44100, 48000, 32000
};

static int
find_nearest ( const int *table, int n, int x )
{
    int i, d;
    int d0, i0;

    i0 = 0;
    d0 = abs ( table[0] - x );
    for ( i = 0; i < n; i++ )
    {
        d = abs ( table[i] - x );
        if ( d < d0 )
        {
            d0 = d;
            i0 = i;
        }
    }

    return table[i0];
}

/*==================================================================*/
int
CMp3Enc::MP3_audio_encode_init ( E_CONTROL * ec_arg, int input_type,    // 0=16 bit linear,1=8 bitlinear
                                 int mpeg_select, int mono_convert )
{
    E_CONTROL ec;
    E_CONTROL ec2;
    int source, target;
    int source_chan, target_chan;
    int source_bits;
    int convert_flag;
    int encode_input_bytes;
    int encode_cutoff_freq;
    int min_input_bytes = 0;
    int nsb_limit;

// cleanup in case of multiple calls to init 
    if ( src != NULL )
    {
        delete src;

        src = NULL;
    }
    if ( src_pcmbuf != NULL )
    {
        delete[] src_pcmbuf;
        src_pcmbuf = NULL;
    }

//
    ec = *ec_arg;
    source = ec.samprate;

/*-- limit source samp rate to some semi-reasonable values --*/

/*-- conversion will attempt any, but init may fail on uncommon rates */
    if ( source < 4000 )
        return 0;
    if ( source > 48000 )
        return 0;

/*-- source channels determined from ec.mode */
    source_chan = 2;
    if ( ec.mode == 3 )
        source_chan = 1;
    target_chan = source_chan;
    if ( mono_convert )
        target_chan = 1;

    source_bits = 16;
    if ( input_type == 1 )
        source_bits = 8;

/*------ determine encode sample rate --------*/
    if ( mpeg_select < 0 )
        mpeg_select = 0;
    switch ( mpeg_select )
    {
    case 0:     // track input 
        if ( source < 16000 )
        {       // look for 2x up convert
            target = find_nearest ( rate_table, 3, 2 * source );
            if ( target == 2 * source )
                break;
        }
        target = find_nearest ( rate_table, 6, source );
        break;
    case 1:     // encode mpeg1 rate
        if ( source < 16000 )
        {       // look for 4x up convert
            target = find_nearest ( rate_table + 3, 3, 4 * source );
            if ( target == 4 * source )
                break;
        }
        if ( source < 32000 )
        {       // look for 2x up convert
            target = find_nearest ( rate_table + 3, 3, 2 * source );
            if ( target == 2 * source )
                break;
        }
        target = find_nearest ( rate_table + 3, 3, source );
        break;
    case 2:     // encode mpeg2 rate
        if ( source < 16000 )
        {       // look for 2x up convert
            target = find_nearest ( rate_table, 3, 2 * source );
            if ( target == 2 * source )
                break;
        }
        if ( source > 24000 )
        {       // look for 2x down convert
            target = find_nearest ( rate_table, 3, source / 2 );
            if ( 2 * target == source )
                break;
        }
        target = find_nearest ( rate_table, 3, source );
        break;
    default:    // user selected rate (mpeg_select is desired samprate)
        target = find_nearest ( rate_table, 6, mpeg_select );
        if ( target != mpeg_select )
            return 0;
        break;
    }

/*--- determine if conversion required */
    convert_flag = 0;   // assume no conversion
    if ( source_bits != 16 )
        convert_flag = 1;
    if ( source != target )
        convert_flag = 1;
    if ( source_chan != target_chan )
        convert_flag = 1;

/*--- init sample rate conversion ---*/
    nsb_limit = -1;     // use nsb limit because of L3 low rate auto limit logic
    if ( convert_flag )
    {
        src = new Csrc; // sample rate conversion
        min_input_bytes =
            src->sr_convert_init ( source, source_chan, source_bits, target,
                                   target_chan, &encode_cutoff_freq );
        if ( min_input_bytes <= 0 )
            return 0;   // conversion init failed
        nsb_limit = ( 64 * encode_cutoff_freq + target / 2 ) / target;
        if ( nsb_limit > 30 )
            nsb_limit = 30;
    }

/*--- init encoder ---*/
    ec2 = ec;
    ec2.samprate = target;
    if ( target_chan == 1 )
        ec2.mode = 3;
    if ( source < target )
    {
        if ( ec2.nsb_limit <= 0 )
            ec2.nsb_limit = 30;
        if ( ec2.nsb_limit > nsb_limit )
            ec2.nsb_limit = nsb_limit;
    }

    ec2.layer = 3;
//src_encode = L3_audio_encode;
    src_encode = 0;
    encode_input_bytes = L3_audio_encode_init ( &ec2 );

    if ( encode_input_bytes <= 0 )
        return 0;       // encoder init failed
    if ( convert_flag == 0 )
        return encode_input_bytes;      //done if no convert

/*-- select converter/encoder combination --*/
//encode = L3_convert_encode;
    src_encode = 1;
    src_pcmbuf = new short[2304];
    memset ( src_pcmbuf, 0, 2304 * sizeof ( short ) );

    return min_input_bytes;
}

/*------------------------------------------------------------------*/
IN_OUT
CMp3Enc::MP3_audio_encode ( unsigned char *pcm, unsigned char *bs_out )
{
    IN_OUT x, x2;

    if ( src_encode == 0 )
    {
        return L3_audio_encode ( ( short * ) pcm, bs_out );
    }
    else
    {
        x = src->sr_convert ( pcm, src_pcmbuf );
        x2 = L3_audio_encode ( src_pcmbuf, bs_out );
        x.out_bytes = x2.out_bytes;
        return x;
    }

}

/*------------------------------------------------------------------*/
IN_OUT
CMp3Enc::MP3_audio_encode_Packet ( unsigned char *pcm,
                                   unsigned char *bs_out,
                                   unsigned char *packet, int nbytes_out[2] )
{
    IN_OUT x, x2;

    if ( src_encode == 0 )
    {
        return L3_audio_encode_Packet ( ( short * ) pcm, bs_out,
                                        packet, nbytes_out );
    }
    else
    {
        x = src->sr_convert ( pcm, src_pcmbuf );
        x2 = L3_audio_encode_Packet ( src_pcmbuf, bs_out,
                                      packet, nbytes_out );
        x.out_bytes = x2.out_bytes;
        return x;
    }

}

/*====================================================================*/

/*====================================================================*/

/*====================================================================*/

/*======== REFORMATTED FRAME =========================================*/

/*====================================================================*/

/*====================================================================*/

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_vbr_MPEG1Packet ( short *pcm,
                                           unsigned char *bs_out,
                                           unsigned char *packet,
                                           int nbytes_out[2] )
{
    IN_OUT x;
    int bytes;
    int main_data_begin;
    unsigned char *bs_out0;
    int ms_encoded;
    int bytes2, bytes3;
    int ibr;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

/* no pad for vbr */
    pad = 0;

/* save the frame info */
    frame_info[side_p1].main_pos = main_tot;

    byte_pool = ( mf_tot - main_tot );

    byte_max = vbr_main_framebytes[ivbr_max] + byte_pool;
    byte_min = vbr_main_framebytes[ivbr_min] + byte_pool - 511;

//byte_max = HX_MIN(byte_max, 960);

    L3_outbits_init ( main_buf + main_p1 );     // init main data packing

/*===============================================*/

/*===============================================*/
    ms_encoded = encode_function (  );  // will do single or joint

/*=========================================================*/
    mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

    bytes = L3_outbits_flush (  );      /* bytes of main data for this frame */

    assert ( bytes <= byte_max );

// determine br index for frame ---
// how much should byte pool be used??? none at all if max = 320kbps?
    bytes2 = bytes - byte_pool; // will use entire pool
//bytes3 = bytes2 + 511;          // would fill entire pool
//bytes3 = bytes2 + 256;          // would fill half  pool
    bytes3 = bytes2 + vbr_pool_target;  // would fill pool to vbr_pool_target
    for ( ibr = ivbr_min; ibr <= ivbr_max; ibr++ )
    {   // find min required
        if ( bytes2 <= vbr_main_framebytes[ibr] )
            break;
    }
    for ( ; ibr <= ivbr_max; ibr++ )
    {   // find required for pool fill
        if ( bytes3 < vbr_main_framebytes[ibr + 1] )
            break;
    }
    if ( ibr > ivbr_max )
        ibr = ivbr_max;

    br_index_buf[side_p1] = ibr;
    frame_info[side_p1].mf_bytes = vbr_main_framebytes[ibr];
//--------------

/* pack side info */
    L3_pack_side_MPEG1 ( side_buf[side_p1], &side_info, nchan );

// reformatted frame
    if ( packet != NULL )
    {   // output reformatted frame
        L3_pack_head ( packet, pad, mode_ext_buf[side_p1] );
        memmove ( packet + 4, side_buf[side_p1], side_bytes );
        memmove ( packet + 4 + side_bytes, main_buf + main_p1, bytes );
        nbytes_out[0] = 4 + side_bytes + bytes;
        nbytes_out[1] = 0;
    }
//

/* pad if necessary */
    if ( bytes < byte_min )
    {
        memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
        bytes = byte_min;
    }

    main_tot += bytes;
    main_bytes += bytes;
    main_p1 += bytes;
    mf_tot += vbr_main_framebytes[ibr];
    side_p1 = ( side_p1 + 1 ) & 31;     /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
    bs_out0 = bs_out;
    for ( ; side_p0 != side_p1; )
    {
        if ( main_bytes < frame_info[side_p0].mf_bytes )
            break;
        tot_frames_out++;
        main_data_begin = main_sent - frame_info[side_p0].main_pos;
        main_sent += frame_info[side_p0].mf_bytes;
        /* output the data */
        if ( bs_out != NULL )
        {
            bs_out += L3_pack_head_vbr ( bs_out,
                                         mode_ext_buf[side_p0],
                                         br_index_buf[side_p0] );

            side_buf[side_p0][0] = main_data_begin >> 1;
            side_buf[side_p0][1] |= ( main_data_begin & 1 ) << 7;
            memmove ( bs_out, side_buf[side_p0], side_bytes );
            bs_out += side_bytes;
            memmove ( bs_out, main_buf + main_p0,
                      frame_info[side_p0].mf_bytes );
            bs_out += frame_info[side_p0].mf_bytes;
        }
        main_bytes -= frame_info[side_p0].mf_bytes;
        main_p0 += frame_info[side_p0].mf_bytes;
        side_p0 = ( side_p0 + 1 ) & 31;
    }
    bytesout = bs_out - bs_out0;
    tot_bytes_out += bytesout;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( bytesout << 8 ) - ave_tot_bytes_out ) ) >> 7 );

/* shift main buffer */
    if ( main_p1 > MB_TRIGGER )
    {
        main_p1 = main_p1 - main_p0;
        memmove ( main_buf, main_buf + main_p0, main_p1 );
        main_p0 = 0;
    }

/*---------------------------------------*/

    x.in_bytes = bytes_in;
    x.out_bytes = bytesout;

    return x;
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_MPEG1Packet ( short *pcm,
                                       unsigned char *bs_out,
                                       unsigned char *packet,
                                       int nbytes_out[2] )
{
    IN_OUT x;
    int bytes;
    int main_data_begin;
    unsigned char *bs_out0;
    int ms_encoded;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

/* compute pad */
    pad = 0;
    padcount -= remainder;
    if ( padcount <= 0 )
    {
        padcount += divisor;
        pad = 1;
    }

/* save the frame info */
    frame_info[side_p1].main_pos = main_tot;
    frame_info[side_p1].mf_bytes = main_framebytes + pad;

    byte_pool = ( mf_tot - main_tot );

    byte_max = main_framebytes + pad + byte_pool;
    byte_min = byte_max - 511;  // 511 is MPEG1

    L3_outbits_init ( main_buf + main_p1 );     // init main data packing

/*===============================================*/

/*===============================================*/
    ms_encoded = encode_function (  );  // does single or joint

/*=========================================================*/
    mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

    bytes = L3_outbits_flush (  );      /* bytes of main data for this frame */
    assert ( bytes <= byte_max );

/* pack side info */
    L3_pack_side_MPEG1 ( side_buf[side_p1], &side_info, nchan );

// reformatted frame
    if ( packet != NULL )
    {   // output reformatted frame
        L3_pack_head ( packet, pad, mode_ext_buf[side_p1] );
        memmove ( packet + 4, side_buf[side_p1], side_bytes );
        memmove ( packet + 4 + side_bytes, main_buf + main_p1, bytes );
        nbytes_out[0] = 4 + side_bytes + bytes;
        nbytes_out[1] = 0;
    }
//

    if ( bytes < byte_min )
    {   /* pad if necessary */
        memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
        bytes = byte_min;
    }

    main_tot += bytes;
    main_bytes += bytes;
    main_p1 += bytes;
    mf_tot += main_framebytes + pad;
    side_p1 = ( side_p1 + 1 ) & 31;     /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
    bs_out0 = bs_out;
    for ( ; side_p0 != side_p1; )
    {
        if ( main_bytes < frame_info[side_p0].mf_bytes )
            break;
        tot_frames_out++;
        main_data_begin = main_sent - frame_info[side_p0].main_pos;
        main_sent += frame_info[side_p0].mf_bytes;
        /* output the data */
        if ( bs_out != NULL )
        {
            bs_out += L3_pack_head ( bs_out,
                                     frame_info[side_p0].mf_bytes -
                                     main_framebytes, mode_ext_buf[side_p0] );
            side_buf[side_p0][0] = main_data_begin >> 1;
            side_buf[side_p0][1] |= ( main_data_begin & 1 ) << 7;
            memmove ( bs_out, side_buf[side_p0], side_bytes );
            bs_out += side_bytes;
            memmove ( bs_out, main_buf + main_p0,
                      frame_info[side_p0].mf_bytes );
            bs_out += frame_info[side_p0].mf_bytes;
        }
        main_bytes -= frame_info[side_p0].mf_bytes;
        main_p0 += frame_info[side_p0].mf_bytes;
        side_p0 = ( side_p0 + 1 ) & 31;
    }
    bytesout = bs_out - bs_out0;
    tot_bytes_out += bytesout;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( bytesout << 8 ) - ave_tot_bytes_out ) ) >> 7 );

/* shift main buffer */
    if ( main_p1 > MB_TRIGGER )
    {
        main_p1 = main_p1 - main_p0;
        memmove ( main_buf, main_buf + main_p0, main_p1 );
        main_p0 = 0;
    }

/*---------------------------------------*/

    x.in_bytes = bytes_in;
    x.out_bytes = bytesout;

    return x;
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_vbr_MPEG2Packet ( short *pcm,
                                           unsigned char *bs_out,
                                           unsigned char *packet,
                                           int nbytes_out[2] )
{
    IN_OUT x;
    int bytes, bytes2, bytes3;
    int main_data_begin;
    unsigned char *bs_out0, *bs_out00;
    int ms_encoded;
    int ibr;
    int side_dp;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

    bs_out00 = bs_out;
    x.out_bytes = 0;

    for ( igr = 0; igr < 2; igr++ )
    {   // two frames two igr

/* compute pad - no pad for vbr */
        pad = 0;

/* save the frame info */
        frame_info[side_p1].main_pos = main_tot;

        byte_pool = mf_tot - main_tot;
        byte_max = vbr_main_framebytes[ivbr_max] + byte_pool;
        byte_min = vbr_main_framebytes[ivbr_min] + byte_pool - 255;

        L3_outbits_init ( main_buf + main_p1 ); // init main data packing

/*===============================================*/

/*===============================================*/
        ms_encoded = encode_function (  );      // does single or joint

/*=========================================================*/
        mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

        bytes = L3_outbits_flush (  );  /* bytes of main data for this frame */

        assert ( bytes <= byte_max );

// determine br index for frame
        bytes2 = bytes - byte_pool;     // will use entire pool
        bytes3 = bytes2 + vbr_pool_target;      // would fill pool to vbr_pool_target
        for ( ibr = ivbr_min; ibr <= ivbr_max; ibr++ )
        {       // find min required
            if ( bytes2 <= vbr_main_framebytes[ibr] )
                break;
        }
// limit number of frames in pool
// side_dp indicates number of frames spanned by pool
// min mpeg2 vbr_main_framebytes is small enough to wrap side pointers
// if side_dp is large, consume pool to limit side_dp
        side_dp = ( side_p1 - side_p0 ) & 31;
        if ( side_dp < 10 )
        {       // no pool fill if side_dp is large 
            for ( ; ibr <= ivbr_max; ibr++ )
            {   // find required for pool fill
                if ( bytes3 < vbr_main_framebytes[ibr + 1] )
                    break;
            }
        }
        else if ( side_dp > 15 )
        {       // consume pool with padding via byte_min
            if ( side_dp > 24 )
            {   // consume entire pool
                byte_min = vbr_main_framebytes[ivbr_min] + byte_pool;
            }
            else
            {   // consume some of pool
                byte_min = vbr_main_framebytes[ivbr_min] + ( byte_pool >> 4 );
            }
        }

        if ( ibr > ivbr_max )
            ibr = ivbr_max;
        br_index_buf[side_p1] = ibr;
        frame_info[side_p1].mf_bytes = vbr_main_framebytes[ibr];

/* pack side info */
        L3_pack_side_MPEG2 ( side_buf[side_p1], &side_info, nchan, igr );

// output reformatted packet
        if ( packet != NULL )
        {
            L3_pack_head ( packet, pad, mode_ext_buf[side_p1] );
            packet += 4;
            memmove ( packet, side_buf[side_p1], side_bytes );
            packet += side_bytes;
            memmove ( packet, main_buf + main_p1, bytes );
            packet += bytes;
            nbytes_out[igr] = 4 + side_bytes + bytes;
        }

        if ( bytes < byte_min )
        {       /* pad if necessary */
            memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
            bytes = byte_min;
        }

        main_tot += bytes;
        main_bytes += bytes;
        main_p1 += bytes;
        mf_tot += vbr_main_framebytes[ibr];
        side_p1 = ( side_p1 + 1 ) & 31; /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
        bs_out0 = bs_out;
        for ( ; side_p0 != side_p1; )
        {
            if ( main_bytes < frame_info[side_p0].mf_bytes )
                break;
            tot_frames_out++;
            main_data_begin = main_sent - frame_info[side_p0].main_pos;
            assert ( main_data_begin < 256 );
            assert ( main_data_begin >= 0 );
            main_sent += frame_info[side_p0].mf_bytes;
            /* output the data */
            if ( bs_out != NULL )
            {
                bs_out += L3_pack_head_vbr ( bs_out,
                                             mode_ext_buf[side_p0],
                                             br_index_buf[side_p0] );
                side_buf[side_p0][0] = main_data_begin;
                memmove ( bs_out, side_buf[side_p0], side_bytes );
                bs_out += side_bytes;
                memmove ( bs_out, main_buf + main_p0,
                          frame_info[side_p0].mf_bytes );
                bs_out += frame_info[side_p0].mf_bytes;
            }
            main_bytes -= frame_info[side_p0].mf_bytes;
            main_p0 += frame_info[side_p0].mf_bytes;
            side_p0 = ( side_p0 + 1 ) & 31;
        }

/* shift main buffer */
        if ( main_p1 > MB_TRIGGER )
        {
            main_p1 = main_p1 - main_p0;
            memmove ( main_buf, main_buf + main_p0, main_p1 );
            main_p0 = 0;
        }

/*---------------------------------------*/

    }   // end igr loop

    x.in_bytes = bytes_in;
    x.out_bytes = bs_out - bs_out00;
    tot_bytes_out += x.out_bytes;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( x.out_bytes << 8 ) - ave_tot_bytes_out ) ) >> 6 );
    return x;
}

/*====================================================================*/
IN_OUT
CMp3Enc::L3_audio_encode_MPEG2Packet ( short *pcm,
                                       unsigned char *bs_out,
                                       unsigned char *packet,
                                       int nbytes_out[2] )
{
    IN_OUT x;
    int bytes;
    int main_data_begin;
    unsigned char *bs_out0, *bs_out00;
    int ms_encoded;

    iframe++;
    filter2 ( pcm, buf1, buf2, &fc2 );

    bs_out00 = bs_out;
    x.out_bytes = 0;

    for ( igr = 0; igr < 2; igr++ )
    {   // two frames two igr

/* compute pad */
        pad = 0;
        padcount -= remainder;
        if ( padcount <= 0 )
        {
            padcount += divisor;
            pad = 1;
        }

/* save the frame info */
        frame_info[side_p1].main_pos = main_tot;
        frame_info[side_p1].mf_bytes = main_framebytes + pad;

        byte_pool = mf_tot - main_tot;

        byte_max = main_framebytes + pad + byte_pool;
        byte_min = byte_max - 255;      // 255 is MPEG2

        L3_outbits_init ( main_buf + main_p1 ); // init main data packing

/*===============================================*/

/*===============================================*/
        ms_encoded = encode_function (  );      // does single or joint

/*=========================================================*/
        mode_ext_buf[side_p1] = ms_encoded + ms_encoded + is_flag;

        bytes = L3_outbits_flush (  );  /* bytes of main data for this frame */
        assert ( bytes <= byte_max );

/* pack side info */
        L3_pack_side_MPEG2 ( side_buf[side_p1], &side_info, nchan, igr );

        if ( packet != NULL )
        {       // output reformatted packet
            L3_pack_head ( packet, pad, mode_ext_buf[side_p1] );
            packet += 4;
            memmove ( packet, side_buf[side_p1], side_bytes );
            packet += side_bytes;
            memmove ( packet, main_buf + main_p1, bytes );
            packet += bytes;
            nbytes_out[igr] = 4 + side_bytes + bytes;
        }

        if ( bytes < byte_min )
        {       /* pad if necessary */
            memset ( main_buf + main_p1 + bytes, 0, byte_min - bytes );
            bytes = byte_min;
        }

        main_tot += bytes;
        main_bytes += bytes;
        main_p1 += bytes;
        mf_tot += main_framebytes + pad;
        side_p1 = ( side_p1 + 1 ) & 31; /* inc circ pointer */

/*---------------------------------------*/

/* try to output one or more frames */
        bs_out0 = bs_out;
        for ( ; side_p0 != side_p1; )
        {
            if ( main_bytes < frame_info[side_p0].mf_bytes )
                break;
            tot_frames_out++;
            main_data_begin = main_sent - frame_info[side_p0].main_pos;
            assert ( main_data_begin < 256 );
            assert ( main_data_begin >= 0 );
            main_sent += frame_info[side_p0].mf_bytes;
            /* output the data */
            if ( bs_out != NULL )
            {
                bs_out += L3_pack_head ( bs_out,
                                         frame_info[side_p0].mf_bytes -
                                         main_framebytes,
                                         mode_ext_buf[side_p0] );
                side_buf[side_p0][0] = main_data_begin;
                memmove ( bs_out, side_buf[side_p0], side_bytes );
                bs_out += side_bytes;
                memmove ( bs_out, main_buf + main_p0,
                          frame_info[side_p0].mf_bytes );
                bs_out += frame_info[side_p0].mf_bytes;
            }
            main_bytes -= frame_info[side_p0].mf_bytes;
            main_p0 += frame_info[side_p0].mf_bytes;
            side_p0 = ( side_p0 + 1 ) & 31;
        }

/* shift main buffer */
        if ( main_p1 > MB_TRIGGER )
        {
            main_p1 = main_p1 - main_p0;
            memmove ( main_buf, main_buf + main_p0, main_p1 );
            main_p0 = 0;
        }

/*---------------------------------------*/

    }   // end igr loop

    x.in_bytes = bytes_in;
    x.out_bytes = bs_out - bs_out00;
    tot_bytes_out += x.out_bytes;
    ave_tot_bytes_out = ave_tot_bytes_out +
        ( ( ( ( x.out_bytes << 8 ) - ave_tot_bytes_out ) ) >> 6 );
    return x;
}

/*====================================================================*/

/*====================================================================*/

/*====================================================================*/

/*==================================================================*/

/*====================================================================*/

/*==== Public Aux Routines ==================================================*/

/*====================================================================*/
int
CMp3Enc::L3_audio_encode_get_bitrate (  )
{
    int br;

    if ( tot_frames_out <= 0 )
        return 0;

    if ( h_id == 1 )
    {
        br = ( int ) ( ( 0.001 * 8.0 ) * tot_bytes_out * samprate /
                       ( 1152.0 * tot_frames_out ) + 0.5 );
    }
    else
    {
        br = ( int ) ( ( 0.001 * 8.0 ) * tot_bytes_out * samprate /
                       ( 576.0 * tot_frames_out ) + 0.5 );
    }
    return br;

}

/*--------------------------------------------------------------------*/
float
CMp3Enc::L3_audio_encode_get_bitrate_float (  )
{
    float br;

    if ( tot_frames_out <= 0 )
        return 0.0f;

    if ( h_id == 1 )
    {
        br = ( ( 0.001f * 8.0f ) * tot_bytes_out * samprate /
               ( 1152.0f * tot_frames_out ) );
    }
    else
    {
        br = ( ( 0.001f * 8.0f ) * tot_bytes_out * samprate /
               ( 576.0f * tot_frames_out ) );
    }
    return br;

}

/*--------------------------------------------------------------------*/
float
CMp3Enc::L3_audio_encode_get_bitrate2_float (  )
{
    float br;

    if ( tot_frames_out <= 0 )
        return 0.0f;

// mpeg2 encodes 2 frames per call 
    br = ( 0.001f * 8.0f / ( 1152.0 * 256.0 ) ) * ave_tot_bytes_out *
        samprate;

    return br;

}

/*--------------------------------------------------------------------*/
int
CMp3Enc::L3_audio_encode_get_frames (  )
{
    return tot_frames_out;      // actual number frame written to output
}

/*--------------------------------------------------------------------*/
void
CMp3Enc::L3_audio_encode_info_ec ( E_CONTROL * ec )     /* info only */
{
    *ec = ec_global;
}

/*--------------------------------------------------------------------*/
void
CMp3Enc::L3_audio_encode_info_head ( MPEG_HEAD * head ) /* info only */
{
    *head = h;
}

/*--------------------------------------------------------------------*/
void
CMp3Enc::L3_audio_encode_info_string ( char *s )        /* info only */
{
    mpeg_info_string ( &h, s, &ec_global );
}

/*--------------------------------------------------------------------*/
INT_PAIR
CMp3Enc::L3_audio_encode_get_frames_bytes (  )
{
    INT_PAIR x;

    x.a = tot_frames_out;
    x.b = tot_bytes_out;
    return x;
}

/*--------------------------------------------------------------------*/
void
CMp3Enc::out_stats (  ) // test routine
{

    BitAllo->ba_out_stats (  );

}

/*--------------------------------------------------------------------*/
