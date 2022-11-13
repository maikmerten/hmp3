/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: mhead.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
 *  mpeg audio
 *  extract info from mpeg header
 *  portable version (adapted from c:\eco\mhead.c
 *
 *  add Layer III
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "mhead.h"      /* mpeg header structure */

static const int mp_br_table[2][16] = {
    0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0,
    0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, 0
};
static const int mp_sr20_table[2][4] = {
    441, 480, 320, -999, 882, 960, 640, -999
};

static const int mp_br_tableL1[2][16] = {
    /* mpeg2 */
    0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, 0,
    0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0
};

static const int mp_br_tableL3[2][16] = {
    /* mpeg 2 */
    0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, 0,
    0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0
};

static int find_sync ( unsigned char *buf, int n );
static int sync_scan ( unsigned char *buf, int n, int i0 );
static int sync_test ( unsigned char *buf, int n, int isync, int padbytes );

/*--------------------------------------------------------------*/
int
head_info ( unsigned char *buf, unsigned int n, MPEG_HEAD * h )
{
    int framebytes;

    if ( n > 10000 )
        n = 10000;      /* limit scan for free format */

    h->sync = 0;
    if ( buf[0] != 0xFF )
        return 0;
    if ( ( buf[1] & 0xF0 ) != 0xF0 )
        return 0;

// mpeg 2.5 sync
//if( ((buf[1] & 0xF0) != 0xF0) && ((buf[1] & 0xF0) != 0xE0)  ) return 0;

    h->sync = 1;

    h->id = ( buf[1] & 0x08 ) >> 3;
    h->option = ( buf[1] & 0x06 ) >> 1;
    h->prot = ( buf[1] & 0x01 );

    h->br_index = ( buf[2] & 0xf0 ) >> 4;
    h->sr_index = ( buf[2] & 0x0c ) >> 2;
    h->pad = ( buf[2] & 0x02 ) >> 1;
    h->private_bit = ( buf[2] & 0x01 );
    h->mode = ( buf[3] & 0xc0 ) >> 6;
    h->mode_ext = ( buf[3] & 0x30 ) >> 4;
    h->cr = ( buf[3] & 0x08 ) >> 3;
    h->original = ( buf[3] & 0x04 ) >> 2;
    h->emphasis = ( buf[3] & 0x03 );

/* compute framebytes for Layer I, II, III */
    if ( h->option < 1 )
        return 0;
    if ( h->option > 3 )
        return 0;

    framebytes = 0;

    if ( h->br_index > 0 )
    {
        if ( h->option == 3 )
        {       /* layer I */
            framebytes =
                240 * mp_br_tableL1[h->id][h->br_index]
                / mp_sr20_table[h->id][h->sr_index];
            framebytes = 4 * framebytes;
        }
        else if ( h->option == 2 )
        {       /* layer II */
            framebytes =
                2880 * mp_br_table[h->id][h->br_index]
                / mp_sr20_table[h->id][h->sr_index];
        }
        else if ( h->option == 1 )
        {       /* layer III */
            if ( h->id )
                framebytes =
                    2880 * mp_br_tableL3[h->id][h->br_index]
                    / mp_sr20_table[h->id][h->sr_index];
            else
                framebytes =
                    1440 * mp_br_tableL3[h->id][h->br_index]
                    / mp_sr20_table[h->id][h->sr_index];

        }
    }
    else
        framebytes = find_sync ( buf, n );      /* free format */

    return framebytes;
}

/*--------------------------------------------------------------*/
int
head_info2 ( unsigned char *buf, unsigned int n, MPEG_HEAD * h, int *br )
{
    int framebytes;

/*---  return br (in bits/sec) in addition to frame bytes ---*/

    *br = 0;     /*-- assume fail --*/
    framebytes = head_info ( buf, n, h );

    if ( framebytes == 0 )
        return 0;

    if ( h->option == 1 )
    {   /* layer III */
        if ( h->br_index > 0 )
            *br = 1000 * mp_br_tableL3[h->id][h->br_index];
        else
        {
            if ( h->id )        // mpeg1
                *br =
                    1000 * framebytes * mp_sr20_table[h->id][h->sr_index] /
                    ( 144 * 20 );
            else
                *br =
                    1000 * framebytes * mp_sr20_table[h->id][h->sr_index] /
                    ( 72 * 20 );
        }
    }
    if ( h->option == 2 )
    {   /* layer II */
        if ( h->br_index > 0 )
            *br = 1000 * mp_br_table[h->id][h->br_index];
        else
            *br = 1000 * framebytes * mp_sr20_table[h->id][h->sr_index]
                / ( 144 * 20 );
    }
    if ( h->option == 3 )
    {   /* layer I */
        if ( h->br_index > 0 )
            *br = 1000 * mp_br_tableL1[h->id][h->br_index];
        else
            *br = 1000 * framebytes * mp_sr20_table[h->id][h->sr_index]
                / ( 48 * 20 );
    }

    return framebytes;
}

/*--------------------------------------------------------------*/
static int
compare ( unsigned char *buf, unsigned char *buf2 )
{
    if ( buf[0] != buf2[0] )
        return 0;
    if ( buf[1] != buf2[1] )
        return 0;
    return 1;
}

/*----------------------------------------------------------*/

/*-- does not scan for initial sync, initial sync assumed --*/
static int
find_sync ( unsigned char *buf, int n )
{
    int i0, isync, nmatch, pad;
    int padbytes, option;

/* mod 4/12/95 i0 change from 72, allows as low as 8kbits for mpeg1 */
    i0 = 24;
    padbytes = 1;
    option = ( buf[1] & 0x06 ) >> 1;
    if ( option == 3 )
    {
        padbytes = 4;
        i0 = 24;        /* for shorter layer I frames */
    }

    pad = ( buf[2] & 0x02 ) >> 1;

    n -= 3;     /*  need 3 bytes of header  */

    while ( i0 < 2000 )
    {
        isync = sync_scan ( buf, n, i0 );
        i0 = isync + 1;
        isync -= pad;
        if ( isync <= 0 )
            return 0;
        nmatch = sync_test ( buf, n, isync, padbytes );
        if ( nmatch > 0 )
            return isync;
    }

    return 0;
}

/*------------------------------------------------------*/

/*---- scan for next sync, assume start is valid -------*/

/*---- return number bytes to next sync ----------------*/
static int
sync_scan ( unsigned char *buf, int n, int i0 )
{
    int i;

    for ( i = i0; i < n; i++ )
        if ( compare ( buf, buf + i ) )
            return i;

    return 0;
}

/*------------------------------------------------------*/

/*- test consecutative syncs, input isync without pad --*/
static int
sync_test ( unsigned char *buf, int n, int isync, int padbytes )
{
    int i, nmatch, pad;

    nmatch = 0;
    for ( i = 0;; )
    {
        pad = padbytes * ( ( buf[i + 2] & 0x02 ) >> 1 );
        i += ( pad + isync );
        if ( i > n )
            break;
        if ( !compare ( buf, buf + i ) )
            return -nmatch;
        nmatch++;
    }
    return nmatch;
}
