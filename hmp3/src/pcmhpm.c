/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: 2024-03-19, Case
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
 * returns wave type tag in the F_INFO structure.
 *
 * Min of 36 bytes required to obtain format info.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "pcmhpm.h"
#include <assert.h>
#include <memory.h>

static int get_whead_mem_and_file ( FILE *handle, unsigned char *buf, int nbuf, F_INFO * f_info, unsigned int *data_size );

/*---------------------------------------------------------------------*/
int
pcmhead_file ( FILE *handle, unsigned char *buf, int nbuf, F_INFO * f_info, unsigned int *data_size )
{
    int k;

/*-- check for wave first --*/
    k = get_whead_mem_and_file ( handle, buf, nbuf, f_info, data_size );
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
static unsigned int
get_field ( unsigned char f[], int n, int is_msb )
{
    int i;
    unsigned int x;

    x = 0;
    if ( !is_msb ) {
        for ( i = n - 1; i >= 0; i-- )
        {
            x <<= 8;
            x |= f[i];
        }
    } else {
        for ( i = 0; i < n; i++ )
        {
            x <<= 8;
            x |= f[i];
        }
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

typedef struct
{
    unsigned char validbits[2];
    unsigned char channelmask[4];
    unsigned char guid[16];
}
WAVEFMTEXT;

static int whead_read_file ( FILE *handle, unsigned char *buf_in, int nbuf0, int nbuf )
{
    int r, nread;

    while ( nbuf < 0 ) {
        r = -nbuf;
        if ( r > nbuf0 )
            r = nbuf0;
        nread = fread ( buf_in, 1, r, handle );
	    if ( nread <= 0 )
		    return -1;
        nbuf += nread;
    }
    nread = fread ( buf_in, 1, nbuf0, handle );
	if ( nread <= 0 )
		return -1;
    return nread;
}

/*---------------------------WAVE-----------------------------------*/
static int
get_whead_mem_and_file ( FILE *handle, unsigned char *buf_in, int nbuf, F_INFO * f_info, unsigned int *data_size )
{
    unsigned char *buf = buf_in;
    CHUNK *chunk;
    WAVEFMT *w;
    int tag;
    int size;
    int nbuf0;

    nbuf0 = nbuf;

    if ( nbuf < ( signed int ) sizeof ( CHUNK ) )
        return -1;
    f_info->bigendian = 0;
    chunk = ( CHUNK * ) buf;
    if ( cmp ( chunk->name, "RIFF" ) < 0 ) {
        if ( cmp ( chunk->name, "RIFX" ) < 0 )
            return -1;
        f_info->bigendian = 1;
    }
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
        if ( nbuf < ( signed int ) sizeof ( CHUNK ) ) {
            nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	        if ( nbuf < 0 )
		        return -1;
            buf = buf_in;
        }
        chunk = ( CHUNK * ) buf;
        buf += sizeof ( CHUNK );
        nbuf -= sizeof ( CHUNK );
        if ( cmp ( chunk->name, "fmt " ) > 0 )
            break;
        size = get_field ( chunk->size, 4, f_info->bigendian );
        buf += size;
        nbuf -= size;
    }

/* have format chunk */
    if ( nbuf < sizeof ( WAVEFMT ) ) {
        nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	    if ( nbuf < 0 )
		    return -1;
        buf = buf_in;
    }

    w = ( WAVEFMT * ) buf;
    buf += sizeof ( WAVEFMT );
    nbuf -= sizeof ( WAVEFMT );

    f_info->channels = get_field ( w->channels, sizeof ( w->channels ), f_info->bigendian );
    f_info->rate = get_field ( w->rate, sizeof ( w->rate ), f_info->bigendian );
    f_info->bits = get_field ( w->bits, sizeof ( w->bits ), f_info->bigendian );
    if ( f_info->channels > 0 )
        f_info->bits = 8 * ( get_field ( w->align, sizeof ( w->align ), f_info->bigendian ) / f_info->channels );

    tag = get_field ( w->tag, sizeof ( w->tag ), f_info->bigendian );
    if ( tag == 65534 ) { /* WAVE_FORMAT_EXTENSIBLE */
        WAVEFMTEXT *wext;
        int extsize;
        if ( nbuf < 2 + sizeof( WAVEFMTEXT ) ) {
            nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	        if ( nbuf < 0 )
		        return -1;
            buf = buf_in;
        }
        extsize = get_field( buf, 2, f_info->bigendian );
        if ( extsize >= 22 ) {
            static const unsigned char known_guid[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 };
            wext = ( WAVEFMTEXT * )( buf + 2 );
            if ( memcmp ( wext->guid + 2, known_guid + 2, 16 - 2 ) == 0 ) { /* ending of guid matches known types */
                tag = get_field( wext->guid, 2, f_info->bigendian );
            }
            f_info->bits = get_field( wext->validbits, sizeof( wext->validbits ), f_info->bigendian );
        }
    }
    f_info->type = tag;   /* use wave format tag as type  */

/*-----------------------------------------------
Have format data at this point
-----------------------------------------------*/

/* skip any extra format info */
    size = get_field ( chunk->size, sizeof ( chunk->size ), f_info->bigendian ) - 16;
    if ( size < 0 )
        size = 0; /* erroneous size - let's see if ignoring the error is ok */
    buf += size;
    nbuf -= size;

    if ( nbuf < ( signed int ) sizeof ( CHUNK ) ) {
        nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	    if ( nbuf < 0 )
		    return -1;
        buf = buf_in;
    }

    /* look for data chunk */
    for ( ;; )
    {
        if ( nbuf < ( signed int ) sizeof ( CHUNK ) ) {
            nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	        if ( nbuf < 0 )
		        return -1;
            buf = buf_in;
        }
        chunk = ( CHUNK * )buf;
        buf += sizeof ( CHUNK );
        nbuf -= sizeof ( CHUNK );
        if ( cmp ( chunk->name, "data" ) > 0 ) {
            *data_size = get_field ( chunk->size, sizeof(chunk->size), f_info->bigendian ); /* return actual audio data size to the frontend */
            break;
        }
        size = get_field ( chunk->size, 4, f_info->bigendian );
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
static int cvt_is_bigendian;
static int cvt_bits_per_sample;

/*-------------------------------------------------------*/
int
cvt_to_pcm_init ( int nch, int bits, int is_source_bigendian )
{
    volatile unsigned int i = 0x01234567;
    cvt_is_bigendian = ( (*((unsigned char *)(&i))) != 0x67 );

    cvt_flag = ( bits > 8 ) && ( cvt_is_bigendian != is_source_bigendian );

    cvt_bits_per_sample = bits;

    return nch * ( ( bits * 7 ) / 8 );
}

/*-------------------------------------------------------*/
/* little-endian to big-endian conversion                */
/*-------------------------------------------------------*/
int
cvt_to_pcm ( unsigned char *pcm, int bytes_in )
{
    int i, k;
    int nsamp, bytes;
    unsigned char buf[4];
    unsigned char *p;

    if ( !cvt_flag ) return bytes_in; /*-- no conversion required --*/

    bytes = cvt_bits_per_sample / 8;
    nsamp = bytes_in / bytes;

    assert ( nsamp * bytes == bytes_in );

    if ( nsamp <= 0 )
        return 0;

    p = pcm + (nsamp - 1) * bytes;
    for ( i = nsamp - 1; i >= 0; i-- ) {
        for ( k = bytes - 1; k >= 0; k-- )
            buf[k] = p[bytes - 1  - k];
        for ( k = bytes - 1; k >= 0; k-- )
            p[k] = buf[k];
        p -= bytes;
    }
    return nsamp * bytes;
}

