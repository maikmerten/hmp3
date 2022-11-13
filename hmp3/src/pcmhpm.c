/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: pcmhpm.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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
 * parse from memory buffer - portable version pcmhead
 * adapted from pcmhp.c
 *
 * Check buffer for pcm input, looks for wave,
 * If recognized returns offset to start of pcm data.
 * Valid offset always > 0, (evaluates to TRUE)
 * Fail return 0 (FALSE).
 * Can also have soft failure: Format found but can't find start of data.
 * This returns nbuf.
 *
 * int pcmhead_mem(unsigned char *buf, F_INFO *f_info)
 *
 * If file type recognized but not a known pcm type, 
 * returns file type == 1 in the F_INFO structure. Linear
 * pcm return type=0, u-law returns type = 10
 *
 * Min of 36 bytes required to obtain format info.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "pcmhpm.h"

static int get_whead_mem ( unsigned char *buf, int nbuf, F_INFO * f_info );

/*---------------------------------------------------------------------*/
int
pcmhead_mem ( unsigned char *buf, int nbuf, F_INFO * f_info )
{
    int k;

/*-- check for wave first --*/
    k = get_whead_mem ( buf, nbuf, f_info );
    if ( k > 0 )
        return k;       /* full success */
    if ( k == 0 )
        return nbuf;    /* have wave format but can't find start */

    return 0;
}

/*-----------------------------------------------------------------*/
static int
cmp ( unsigned char *s, char *t )
{
    int i;

    for ( i = 0; i < 4; i++ )
        if ( s[i] != ( unsigned char ) t[i] )
            return -1;
    return 1;
}

/*-----------------------------------------------------*/
static int
get_field ( unsigned char f[], int n )
{
    int i;
    int x;

    x = 0;
    for ( i = n - 1; i >= 0; i-- )
    {
        x <<= 8;
        x |= f[i];
    }
    return x;
}

/*-----------------------------------------------------------------*/
typedef struct
{
    unsigned char name[4];
    unsigned char size[4];
}
CHUNK;

typedef struct
{
    unsigned char tag[2];
    unsigned char channels[2];
    unsigned char rate[4];
    unsigned char avebps[4];
    unsigned char align[2];
    unsigned char bits[2];
}
WAVEFMT;

/*---------------------------WAVE-----------------------------------*/
static int
get_whead_mem ( unsigned char *buf, int nbuf, F_INFO * f_info )
{
    CHUNK *chunk;
    WAVEFMT *w;
    int tag, channels;
    int size;
    int nbuf0;

    nbuf0 = nbuf;

    if ( nbuf < ( signed int ) sizeof ( CHUNK ) )
        return -1;
    chunk = ( CHUNK * ) buf;
    if ( cmp ( chunk->name, "RIFF" ) < 0 )
        return -1;
    buf += sizeof ( CHUNK );
    nbuf -= sizeof ( CHUNK );

    if ( nbuf < 4 )
        return -1;
    if ( cmp ( buf, "WAVE" ) < 0 )
        return -1;
    buf += 4;
    nbuf -= 4;

/* look for fmt chunk */
    for ( ;; )
    {
        if ( nbuf < ( signed int ) sizeof ( CHUNK ) )
            return -1;
        chunk = ( CHUNK * ) buf;
        buf += sizeof ( CHUNK );
        nbuf -= sizeof ( CHUNK );
        if ( cmp ( chunk->name, "fmt " ) > 0 )
            break;
        size = get_field ( chunk->size, 4 );
        buf += size;
        nbuf -= size;
    }

/* have format chunk */
    if ( nbuf < sizeof ( WAVEFMT ) )
        return -1;
    w = ( WAVEFMT * ) buf;
    buf += sizeof ( WAVEFMT );
    nbuf -= sizeof ( WAVEFMT );

    f_info->channels = get_field ( w->channels, sizeof ( w->channels ) );
    f_info->bits = 0;
    channels = get_field ( w->channels, sizeof ( w->channels ) );
    if ( channels > 0 )
        f_info->bits =
            8 * ( get_field ( w->align, sizeof ( w->align ) ) / channels );
    f_info->rate = get_field ( w->rate, sizeof ( w->rate ) );

    f_info->type = 1;   /* don't know what encoding */
    tag = get_field ( w->tag, sizeof ( w->tag ) );
    if ( tag == 1 )
        f_info->type = 0;       /* linear pcm */
    if ( tag == 7 )
        f_info->type = 10;      /* u-law */

/*-----------------------------------------------
Have format data at this point so it should
be a wave file and playable.  If fail after
this point because start of data not found, return 0, instead of -1.  
With this failure, audio is still playable but may pop at start.
-----------------------------------------------*/

/* skip any extra format info */
    size = get_field ( chunk->size, sizeof ( chunk->size ) ) - 16;
    if ( size < 0 )
        return 0;
    buf += size;
    nbuf -= size;

/* look for data chunk */
    for ( ;; )
    {
        if ( nbuf < ( signed int ) sizeof ( CHUNK ) )
            return 0;
        chunk = ( CHUNK * ) buf;
        buf += sizeof ( CHUNK );
        nbuf -= sizeof ( CHUNK );
        if ( cmp ( chunk->name, "data" ) > 0 )
            break;
        size = get_field ( chunk->size, 4 );
        buf += size;
        nbuf -= size;
    }

/*-- return offset to start of data ---*/
    return ( nbuf0 - nbuf );
}

/*-------------------------------------------------------------*/
/* convert wave data to pcm                                    */
/*-------------------------------------------------------------*/

static int cvt_flag;
#if defined(_M_IX86) || defined(__i386__)
#define mustConvert() 0 // we know we're little-endian, so no cvt
#else
#define mustConvert() cvt_flag
#endif

/*-------------------------------------------------------*/
int
cvt_to_pcm_init ( int bits )
{
    static const union {int i;char c;} big_ender = {1} ;

    cvt_flag = ( bits > 8 ) && big_ender.c ;

    if ( bits > 8 )
        return sizeof ( short );
    else
        return 2;
}

/*-------------------------------------------------------*/
/* little-endian to big-endian conversion                */
/*-------------------------------------------------------*/
int
cvt_to_pcm ( unsigned char *pcm, int bytes_in )
{
    int i, k;
    int nsamp;
    short *w;

    if (!mustConvert() )
        return bytes_in;               /*-- no conversion required --*/

    nsamp = bytes_in >> 1;
    if ( nsamp <= 0 )
        return 0;

    w = ( short * ) pcm;
    for ( i = nsamp - 1, k = bytes_in - 2; i >= 0; i--, k -= 2 )
        w[i] =
            ( short ) ( ( ( unsigned char ) pcm[k] ) +
                        ( ( ( int ) pcm[k + 1] ) << 8 ) );

    return sizeof ( short ) * nsamp;
    /*--- return bytes of short ---*/
}

