/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: xhead.c,v 1.2 2005/08/09 20:43:42 karll Exp $ 
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
#include "xhead.h"
#include "hxtypes.h"

// 4   Xing
// 4   flags
// 4   frames
// 4   bytes
// 100 toc
// 4   vbr scale

static const int sr_table[6] = {
    22050, 24000, 16000,
    44100, 48000, 32000,
};

static const int br_table[2][16] = {
    0, 8, 16, 24, 32, 40, 48, 56,       // mpeg2
    64, 80, 96, 112, 128, 144, 160, 0,
    0, 32, 40, 48, 56, 64, 80, 96,      // mpeg1
    112, 128, 160, 192, 224, 256, 320, 0,
};

#define N 512
static int table[N + 1][2];
static int toc_ptr;
static int toc_n;

/*-------------------------------------------------------------*/
static void
InsertI4 ( unsigned char *buf, int x )
{
// big endian insert

    buf[0] = ( ( unsigned int ) x ) >> 24;
    buf[1] = ( ( unsigned int ) x ) >> 16;
    buf[2] = ( ( unsigned int ) x ) >> 8;
    buf[3] = ( ( unsigned int ) x );

}

/*-------------------------------------------------------------*/
static int
ExtractI4 ( unsigned char *buf )
{
    int x;

// big endian extract

    x = buf[0];
    x <<= 8;
    x |= buf[1];
    x <<= 8;
    x |= buf[2];
    x <<= 8;
    x |= buf[3];

    return x;
}

/*-------------------------------------------------------------*/
static void
BuildTOC ( int tot_frames, int tot_bytes, unsigned char *buf )
{
    int i, k;
    int target, target0, target0b;
    double a, b;
    int index;

    if ( ( tot_frames <= 0 ) || ( tot_bytes <= 0 ) )
    {
        for ( i = 0; i < 100; i++ )
            buf[i] = 0;
        return;
    }

    table[toc_ptr][0] = tot_frames;
    table[toc_ptr][1] = tot_bytes;
    toc_ptr++;

    for ( i = 0; i < toc_ptr; i++ )
    {
        table[i][0] = 100 * table[i][0];
    }

    a = 256.0 / tot_bytes;
    target = 0;
    target0 = 0;
    target0b = 0;
    k = 0;
    for ( i = 0; i < 100; i++ )
    {
        while ( table[k][0] <= target )
        {
            target0 = table[k][0];
            target0b = table[k][1];
            k++;
        }
        assert ( ( table[k][0] - target0 ) > 0 );
        b = target0b +
            ( ( double ) ( target - target0 ) ) *
            ( ( double ) ( table[k][1] - target0b ) ) /
            ( ( double ) ( table[k][0] - target0 ) );
        index = ( int ) ( a * b + 0.5 );
        index = HX_MAX ( index, 0 );       // should not happen
        index = HX_MIN ( index, 255 );     // should not happen
        buf[i] = index;
        target += tot_frames;
    }

//for(i=0;i<100;i++) {
//    if( (i%10) == 0 ) printf("\n");
//    printf(" %3d", (int)buf[i]);
//}

}

/*-------------------------------------------------------------*/
int
XingHeader ( int samprate, int h_mode, int cr_bit, int original_bit,
             int head_flags, int frames, int bs_bytes,
             int vbr_scale,
             unsigned char *toc, unsigned char *buf, unsigned char *buf20,
             unsigned char *buf20B, int nBitRateIndex )
{
    int i, k;
    int h_id;
    int sr_index;
    int br_index;
    int side_bytes;
    int bytes_required;
    int frame_bytes;
    int tmp;
    unsigned char *buf0;

    //------------ clear toc table
    for ( i = 0; i < N; i++ )
        table[i][0] = table[i][1] = 0;
    toc_ptr = 0;
    toc_n = 1;

    h_mode = h_mode & 3;
    cr_bit = cr_bit & 1;
    original_bit = original_bit & 1;
    head_flags = head_flags & 63;       // max current flags
    buf0 = buf;

    for ( sr_index = 0; sr_index < 6; sr_index++ )
    {
        if ( samprate == sr_table[sr_index] )
            break;
    }
    if ( sr_index >= 6 )
        return 0;       // fail

    h_id = 0;   // mpeg2

    if ( sr_index >= 3 )
    {
        h_id = 1;       // mpeg1
        sr_index -= 3;
    }

    if ( h_id )
    {   // mpeg1
        side_bytes = 32;
        if ( h_mode == 3 )
            side_bytes = 17;
    }
    else
    {
        // if CBR turn off TOC (it may be too large)
        if ( vbr_scale == -1 )
        {
            if ( head_flags & TOC_FLAG )
                head_flags ^= TOC_FLAG;
        }
        // mpeg2
        side_bytes = 17;
        if ( h_mode == 3 )
            side_bytes = 9;
    }

    // determine required br_index
    bytes_required = 4 + side_bytes + ( 4 + 4 );        // "xing" + flags

    if ( head_flags & FRAMES_FLAG )
        bytes_required += 4;
    if ( head_flags & BYTES_FLAG )
        bytes_required += 4;
    if ( head_flags & TOC_FLAG )
        bytes_required += 100;
    if ( head_flags & VBR_SCALE_FLAG )
        bytes_required += 4;

    if ( head_flags & VBR_RESERVEDA_FLAG )
        bytes_required += 20;
    if ( head_flags & VBR_RESERVEDB_FLAG )
        bytes_required += 20;

    tmp = samprate;
    if ( h_id == 0 )
        tmp += tmp;

    // if TRUE VBR 
    if ( vbr_scale != -1)
    {
        for ( br_index = 1; br_index < 15; br_index++ )
        {
            frame_bytes = 144000 * br_table[h_id][br_index] / tmp;
            if ( frame_bytes >= bytes_required )
                break;
        }
        if ( br_index >= 15 )
            return 0;   //fail, won't fit
    }

    // if VBR CBR version
    else
    {
        if ( nBitRateIndex >= 15 )
            return 0;   //fail, won't fit

        // use the index already determined from head info - you only have one shot 
        frame_bytes = 144000 * br_table[h_id][nBitRateIndex] / tmp;
        if ( frame_bytes < bytes_required )
            return 0;   //fail, won't fit

        // this value is needed below
        br_index = nBitRateIndex;
    }

    buf[0] = 0xFF;
    buf[1] = 0xF3 | ( h_id << 3 );
    buf[2] = ( br_index << 4 ) | ( sr_index << 2 );
    buf[3] = ( h_mode << 6 ) | ( cr_bit << 3 ) | ( original_bit << 2 );
    buf += 4;

    for ( i = 0; i < side_bytes; i++ )
        buf[i] = 0;
    buf += side_bytes;

    buf[0] = 'X';
    buf[1] = 'i';
    buf[2] = 'n';
    buf[3] = 'g';
    buf += 4;

    InsertI4 ( buf, head_flags );
    buf += 4;

    if ( head_flags & FRAMES_FLAG )
    {
        InsertI4 ( buf, frames );
        buf += 4;
    }

    if ( head_flags & BYTES_FLAG )
    {
        InsertI4 ( buf, bs_bytes );
        buf += 4;
    }

    if ( head_flags & TOC_FLAG )
    {
        if ( toc != NULL )
        {
            for ( i = 0; i < 100; i++ )
                buf[i] = toc[i];
        }
        else
        {
            for ( i = 0; i < 100; i++ )
                buf[i] = 0;     // reserve space
        }
        buf += 100;
    }

    if ( head_flags & VBR_SCALE_FLAG )
    {
        InsertI4 ( buf, vbr_scale );
        buf += 4;
    }

    if ( head_flags & VBR_RESERVEDA_FLAG )
    {
        if ( buf20 != NULL )
        {
            for ( i = 0; i < 20; i++ )
                buf[i] = buf20[i];
        }
        else
        {
            for ( i = 0; i < 20; i++ )
                buf[i] = 0;     // reserve space
        }
        buf += 20;
    }

    if ( head_flags & VBR_RESERVEDB_FLAG )
    {
        if ( buf20 != NULL )
        {
            for ( i = 0; i < 20; i++ )
                buf[i] = buf20B[i];
        }
        else
        {
            for ( i = 0; i < 20; i++ )
                buf[i] = 0;     // reserve space
        }
        buf += 20;
    }

    // fill remainder of frame
    k = frame_bytes - ( buf - buf0 );

    for ( i = 0; i < k; i++ )
        buf[i] = 0;

    return frame_bytes;
}

/*-------------------------------------------------------------*/
int
XingHeaderUpdate ( int frames, int bs_bytes,
                   int vbr_scale,
                   unsigned char *toc, unsigned char *buf,
                   unsigned char *buf20, unsigned char *buf20B )
{
    int i, head_flags;
    int h_id, h_mode;

// update info in header
// using logic similar to logic that might be used by a decoder

    h_id = ( buf[1] >> 3 ) & 1;
    h_mode = ( buf[3] >> 6 ) & 3;

    if ( h_id )
    {   // mpeg1
        if ( h_mode != 3 )
            buf += ( 32 + 4 );
        else
            buf += ( 17 + 4 );
    }
    else
    {   // mpeg2
        if ( h_mode != 3 )
            buf += ( 17 + 4 );
        else
            buf += ( 9 + 4 );
    }

    if ( buf[0] != 'X' )
        return 0;
    if ( buf[1] != 'i' )
        return 0;
    if ( buf[2] != 'n' )
        return 0;
    if ( buf[3] != 'g' )
        return 0;
    buf += 4;

    head_flags = ExtractI4 ( buf );
    buf += 4;   // get flags

    if ( head_flags & FRAMES_FLAG )
    {
        InsertI4 ( buf, frames );
        buf += 4;
    }
    if ( head_flags & BYTES_FLAG )
    {
        InsertI4 ( buf, bs_bytes );
        buf += 4;
    }

    if ( head_flags & TOC_FLAG )
    {
        if ( toc != NULL )
        {       // user supplied toc
            for ( i = 0; i < 100; i++ )
                buf[i] = toc[i];
        }
        else
        {
            BuildTOC ( frames, bs_bytes, buf );
        }
        buf += 100;
    }
    if ( head_flags & VBR_SCALE_FLAG )
    {
        InsertI4 ( buf, vbr_scale );
        buf += 4;
    }

    if ( head_flags & VBR_RESERVEDA_FLAG )
    {
        if ( buf20 != NULL )
        {       // user supplied buf20
            for ( i = 0; i < 20; i++ )
                buf[i] = buf20[i];
        }
        else
        {
            for ( i = 0; i < 20; i++ )
                buf[i] = 0;     // reserve space
        }
        buf += 20;
    }

    if ( head_flags & VBR_RESERVEDB_FLAG )
    {
        if ( buf20 != NULL )
        {       // user supplied buf20
            for ( i = 0; i < 20; i++ )
                buf[i] = buf20B[i];
        }
        else
        {
            for ( i = 0; i < 20; i++ )
                buf[i] = 0;     // reserve space
        }
        buf += 20;
    }

    return 1;   // success
}

/*-------------------------------------------------------------*/
int
XingHeaderTOC ( int frames, int bs_bytes )
{
    int i, k;

// build toc

    table[toc_ptr][0] = frames;
    table[toc_ptr][1] = bs_bytes;
    toc_ptr++;
    if ( toc_ptr < N )
        return toc_n;

// decimate table
    for ( k = 1, i = 0; i < ( N / 2 ); i++, k += 2 )
    {
        table[i][0] = table[k][0];
        table[i][1] = table[k][1];
    }
    toc_ptr = ( N / 2 );
    toc_n += toc_n;

    return toc_n;
}

/*-------------------------------------------------------------*/
