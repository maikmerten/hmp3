/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: setup.c,v 1.2 2005/08/09 20:43:42 karll Exp $ 
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
#include "enc.h"
#include "l3e.h"

static const char *mode_msg[] =
    { "mode 0 STEREO", "mode 1 STEREO", "DUAL", "MONO" };

static const int sr_table[8] =
 { 22050, 24000, 16000, 1, 44100, 48000, 32000, 1 };

/*---  rates by mode - first half table is mpeg2 ----*/
static const int mp_br_table[2][4][16] = {
    0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1,
    0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1,
    0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1,
    0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1,
    0, 0, 0, 0, 64, 0, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1,
    0, 0, 0, 0, 64, 0, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1,
    0, 0, 0, 0, 64, 0, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1,
    0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 0, 0, 0, 0, -1
};

/*--- layer I ---------*/
static const int mp_br_tableL1[2][16] = { 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, -1,      /* mpeg2 */
    0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1
};

/*--- layer III ---------*/
static const int mp_br_tableL3[2][16] = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1,   /* mpeg 2 */
    0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1
};

/*------- max bit rates by layer and id -----*/
static const int max_bitrate[4][2] = {
    160, 320,

    160, 320,  /*-- layer III ---*/

    160, 384,  /*-- layer II  ---*/
    256, 448,

};             /*-- layer I   ---*/

/*---- min per chan bit rates based on frame size considerations --*/

/*---- or min non-free format bitrate -----------------------------*/
static const int min_bitrate[4][2] = {
    4, 8,
    4, 8,
    4, 8,
    16, 32,
};

/*-----------------------------------------------------*/

/*  abcd_index =  lookqt[mode][ratecode][bitratecode]  */
static const char lookqt[4][3][16] = {
    1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,        /*  44ks stereo    */
    0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,        /*  48ks           */
    1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,        /*  32ks           */

    1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,        /*  44ks joint     */
    0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,        /*  48ks           */
    1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,        /*  32ks           */

    1, -1, -1, -1, 2, -1, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1,        /*  44ks dual      */
    0, -1, -1, -1, 2, -1, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1,        /*  48ks           */
    1, -1, -1, -1, 3, -1, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1,        /*  32ks           */

    1, 2, 2, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1,        /* 44ks mono       */
    0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1,        /* 48ks            */
    1, 3, 3, 0, 0, 0, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1
};       /* 32ks            */
static const int look_nsb[6] = { 27, 30, 8, 12, 30, 32 };     /* nsb[abcd] */
static const int look_nbal[6] = { 88, 94, 26, 38, 75, 128 };  /* nbal[abcd] */
static const int look_qnsb[6][4] = {
    3, 8, 12, 4,
    3, 8, 12, 7,
    2, 0, 10, 0,
    2, 0, 6, 0,
    4, 0, 7, 19,        /* mpeg2 */
    32, 0, 0, 0,        /* layer I */
};

/*-- Layer 3 sf band tables --*/
static const struct
{
    int l[23];
    int s[14];
}
sfBandIndexTable[2][3] =
{

/* mpeg-2 */
    {
        {
            {   0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168,
                200, 238, 284, 336, 396, 464, 522, 576},
            {   0, 4, 8, 12, 18, 24, 32, 42, 56, 74, 100, 132, 174, 192}
        },
        {
            {   0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 114, 136, 162,
	        194, 232, 278, 332, 394, 464, 540, 576},
            {   0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 136, 180, 192}
        },
        {
            {   0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168,
	        200, 238, 284, 336, 396, 464, 522, 576},
            {   0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192}
        },
    },

/* mpeg-1 */
    {
        {
            {   0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 52, 62, 74, 90, 110, 134,
	        162, 196, 238, 288, 342, 418, 576},
            {   0, 4, 8, 12, 16, 22, 30, 40, 52, 66, 84, 106, 136, 192}
        },
        {
            {   0, 4, 8, 12, 16, 20, 24, 30, 36, 42, 50, 60, 72, 88, 106, 128,
	        156, 190, 230, 276, 330, 384, 576},
            {   0, 4, 8, 12, 16, 22, 28, 38, 50, 64, 80, 100, 126, 192}
        },
        {
            {   0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 54, 66, 82, 102, 126,
	        156, 194, 240, 296, 364, 448, 550, 576},
            {   0, 4, 8, 12, 16, 22, 30, 42, 58, 78, 104, 138, 180, 192}
        }
    }
};

/*- long block sf bands mpeg2 - 22 same as 16 - this does not look right*/

static int bitrate;

/*======================================================================*/

/*--------------------------------------------------------------------*/
int
printnull ( char *format, ... )
{
    format = format;
    return 0;
}

/*--------------------------------------------------------------------*/
int
setup_header ( E_CONTROL * ec, MPEG_HEAD * h )  /* return total bit rate */
{
    int i, k;
    int dmin, d;

    h->sync = 1;
    h->id = 1;
    h->option = 2;
    h->prot = 1;
    h->br_index = 6;
    h->sr_index = 0;
    h->pad = 0;
    h->private_bit = 0;
    h->mode = 3;
    h->mode_ext = 0;
    h->cr = ec->cr_bit;
    h->original = ec->original;
    h->emphasis = 0;

    h->option = 4 - ec->layer;
    if ( h->option > 3 )
        h->option = 3;
    if ( h->option < 1 )
        h->option = 1;

/*------- id and sample rate ----*/
    k = 0;
    dmin = 99999L;
    for ( i = 0; i < 8; i++ )
    {
        d = labs ( ec->samprate - sr_table[i] );
        if ( d < dmin )
        {
            dmin = d;
            k = i;
        }
    }
    h->id = k >> 2;
    h->sr_index = k & 3;

/*------ mode ----*/
    h->mode = ec->mode;
    h->mode_ext = 0;
    if ( h->mode == 1 )
        h->mode_ext = ec->nsbstereo / 4 - 1;
    if ( h->mode_ext < 0 )
    {
        h->mode_ext = 0;
        if ( h->id == 0 )
            h->mode_ext = 1;
    }
    if ( h->mode_ext > 3 )
        h->mode_ext = 3;

/*------ bit rate ----*/
    bitrate = ec->bitrate;

/*--- per chan min ---*/
    if ( bitrate < min_bitrate[h->option][h->id] )
        bitrate = min_bitrate[h->option][h->id];
    if ( ec->mode != 3 )
        bitrate = 2 * bitrate;

/*--- total max ---*/
    if ( bitrate > max_bitrate[h->option][h->id] )
        bitrate = max_bitrate[h->option][h->id];

    h->br_index = 0;/*--- assume free format ---*/
    if ( h->option == 1 )

    {                      /*--- Layer III ---*/
        for ( i = 1;; i++ )
        {
            if ( mp_br_tableL3[h->id][i] < 0 )
                break;
            if ( mp_br_tableL3[h->id][i] == bitrate )
                h->br_index = i;
        }
    }
    if ( h->option == 2 )

    {                      /*--- Layer II ---*/
        for ( i = 1;; i++ )
        {
            if ( mp_br_table[h->id][h->mode][i] < 0 )
                break;
            if ( mp_br_table[h->id][h->mode][i] == bitrate )
                h->br_index = i;
        }
    }
    if ( h->option == 3 )

    {                      /*--- Layer I ---*/
        for ( i = 1;; i++ )
        {
            if ( mp_br_tableL1[h->id][i] < 0 )
                break;
            if ( mp_br_tableL1[h->id][i] == bitrate )
                h->br_index = i;
        }
    }

    return bitrate;
}

/*------------------------------------------------------------------*/
void
out_setup_header ( MPEG_HEAD * h )      /* info only */
{
    fprintf(stderr, "\n h->sync        %3d", h->sync );
    fprintf(stderr, "\n h->id          %3d", h->id );
    fprintf(stderr, "\n h->option      %3d", h->option );
    fprintf(stderr, "\n h->prot        %3d", h->prot );
    fprintf(stderr, "\n h->br_index    %3d", h->br_index );
    fprintf(stderr, "\n h->sr_index    %3d", h->sr_index );
    fprintf(stderr, "\n h->pad         %3d", h->pad );
    fprintf(stderr, "\n h->private_bit %3d", h->private_bit );
    fprintf(stderr, "\n h->mode        %3d", h->mode );
    fprintf(stderr, "\n h->mode_ext    %3d", h->mode_ext );
    fprintf(stderr, "\n h->cr          %3d", h->cr );
    fprintf(stderr, "\n h->original    %3d", h->original );
    fprintf(stderr, "\n h->emphasis    %3d", h->emphasis );
}

/*------------------------------------------------------------------*/
static char *Layer_msg[4] = {
    "NOT VALID", "III", "II", "I"
};

/*------------------------------------------------------------------*/
void
out_setup ( MPEG_HEAD * h )     /* info only */
{
    fprintf(stderr, "\n Layer %s ", Layer_msg[h->option] );

    fprintf(stderr, "  %s ", mode_msg[h->mode] );
    fprintf(stderr, "  %ldkHz ", sr_table[4 * h->id + h->sr_index] );
    fprintf(stderr, "  %dkbps ", bitrate );
    if ( h->mode == 1 )
        fprintf(stderr, "  %d stereo bands ", 4 + 4 * h->mode_ext );

}

/*------------------------------------------------------------------*/

/* info only */
void
mpeg_info_string ( MPEG_HEAD * h, char *s, E_CONTROL * ec )
{

    s += sprintf ( s, "Layer %s ", Layer_msg[h->option] );

    s += sprintf ( s, "  %s ", mode_msg[h->mode] );

    if ( ( h->mode == 1 ) && ( ec->nsbstereo < 32 ) )
        s += sprintf ( s, " IS-%d ", ec->nsbstereo );

    s += sprintf ( s, "  %ldHz ", sr_table[4 * h->id + h->sr_index] );

    if ( ec->vbr_flag == 0 )
    {
        s += sprintf ( s, "  %dkbps ", bitrate );
    }
    else
    {
        s += sprintf ( s, " VBR-%d", ec->vbr_mnr );
        if ( ec->vbr_delta_mnr )
            s += sprintf ( s, "(%d)", ec->vbr_delta_mnr );

    }

    if ( ec->hf_flag )
    {
        s += sprintf ( s, "  hf" );
        if ( ec->hf_flag & 2 )
            s += sprintf ( s, "2" );
    }

}

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
int

setup_abcd ( MPEG_HEAD * h )          /*--- return abcd index ----*/
{

    if ( h->option == 3 )

        return 5;                  /*-- Layer i --*/
    if ( h->id == 0 )
        return 4;       /* mpeg 2 -- */
    return lookqt[h->mode][h->sr_index][h->br_index];
}

/*------------------------------------------------------------------*/
int

setup_nsb ( MPEG_HEAD * h )          /*--- return nsb ----*/
{
    return look_nsb[setup_abcd ( h )];
}

/*------------------------------------------------------------------*/
int

setup_nband_L3 ( MPEG_HEAD * h )          /*--- return nband ----*/
{
    return sfBandIndexTable[h->id][h->sr_index].l[21];
}

/*------------------------------------------------------------------*/
int

setup_nbal ( MPEG_HEAD * h )          /*--- return nbal ----*/
{
    return look_nbal[setup_abcd ( h )];
}

/*------------------------------------------------------------------*/
int

setup_samprate ( MPEG_HEAD * h )          /*--- return samprate ----*/
{
    return sr_table[4 * h->id + h->sr_index];
}

/*------------------------------------------------------------------*/
int

setup_maxbits ( MPEG_HEAD * h, int totbitrate )   /*--- return maxbits ----*/
{
    int i, k;
    int t, nbal, maxbits, abcd_index, nsbstereo;

/* compute bits for bitallocation */

/*  mono/dual per chan,  stereo/ joint totoal */

    maxbits = 0;

    if ( h->option == 3 )  /*-- Layer I --*/
        t = ( 4 * 8 ) * ( ( 12000 * totbitrate ) / setup_samprate ( h ) );

    else                       /*-- Layer II --*/
        t = 8 * ( ( 144000 * totbitrate ) / setup_samprate ( h ) );

    nbal = setup_nbal ( h );

    switch ( h->mode )
    {
    case 0:     /* stereo */
        maxbits = t - 32 - 2 * nbal;
        break;
    case 1:     /* joint */
        abcd_index = setup_abcd ( h );
        nsbstereo = 4 + 4 * h->mode_ext;
        k = 0;
        for ( i = 0; i < look_qnsb[abcd_index][0]; i++, k++ )
            if ( k < nsbstereo )
                nbal += 4;
        for ( i = 0; i < look_qnsb[abcd_index][1]; i++, k++ )
            if ( k < nsbstereo )
                nbal += 4;
        for ( i = 0; i < look_qnsb[abcd_index][2]; i++, k++ )
            if ( k < nsbstereo )
                nbal += 3;
        for ( i = 0; i < look_qnsb[abcd_index][3]; i++, k++ )
            if ( k < nsbstereo )
                nbal += 2;
        maxbits = t - 32 - nbal;
        break;
    case 2:     /* dual */
        maxbits = ( t - 32 - 2 * nbal ) / 2;
        break;
    case 3:     /* mono */
        maxbits = t - 32 - nbal;
        break;
    }

    return maxbits;
}

/*------------------------------------------------------------------*/
void
dummy (  )
{
}
