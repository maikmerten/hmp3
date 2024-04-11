/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: 2024-04-10, Case
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
#include <stdint.h>

static int get_whead_mem_and_file ( FILE *handle, unsigned char *buf, int nbuf, F_INFO * f_info, uint64_t *data_size );

/*---------------------------------------------------------------------*/
int
pcmhead_file ( FILE *handle, unsigned char *buf, int nbuf, F_INFO * f_info, uint64_t *data_size )
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

/*-----------------------------------------------------*/
static uint64_t
get_field64 ( unsigned char f[], int n, int is_msb )
{
    int i;
    uint64_t x;

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

/* Sony Wave64 chunks (*.w64) */
typedef struct
{
    unsigned char guid[16];
    unsigned char size[8];
}
CHUNK64;

/* RF64 DS64 chunk */
typedef struct
{
    unsigned char name[4];
    unsigned char size[4];
    unsigned char riffSize[8];
    unsigned char dataSize[8];
    unsigned char sampleCount[8];
    unsigned char tableLength[4];
}
CHUNKDS64;

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
get_whead_mem_and_file ( FILE *handle, unsigned char *buf_in, int nbuf, F_INFO * f_info, uint64_t *data_size )
{
    /* RIFF: 66666972-912E-11CF-A5D6-28DB04C10000 */
    /* WAVE: 65766177-ACF3-11D3-8CD1-00C04F8EDB8A */
    /* FMT : 20746D66-ACF3-11D3-8CD1-00C04F8EDB8A */
    /* DATA: 61746164-ACF3-11D3-8CD1-00C04F8EDB8A */
    static const unsigned char w64_guid_riff[16] = { 0x72, 0x69, 0x66, 0x66, 0x2E, 0x91, 0xCF, 0x11, 0xA5, 0xD6, 0x28, 0xDB, 0x04, 0xC1, 0x00, 0x00 };
    static const unsigned char w64_guid_wave[16] = { 0x77, 0x61, 0x76, 0x65, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A };
    static const unsigned char w64_guid_fmt[16] =  { 0x66, 0x6D, 0x74, 0x20, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A };
    static const unsigned char w64_guid_data[16] = { 0x64, 0x61, 0x74, 0x61, 0xF3, 0xAC, 0xD3, 0x11, 0x8C, 0xD1, 0x00, 0xC0, 0x4F, 0x8E, 0xDB, 0x8A };
    unsigned char *buf = buf_in;
    CHUNK *chunk;
    CHUNK64 *chunk64;
    CHUNKDS64 *chunk_ds64 = NULL;
    WAVEFMT *w;
    int tag;
    uint64_t size;
    int nbuf0;
    int is_w64 = 0;
    int is_rf64 = 0;
    int chunk_size = ( int ) sizeof ( CHUNK );

    nbuf0 = nbuf;

    if ( nbuf < ( signed int ) sizeof ( CHUNK64 ) )
        return -1;
    f_info->bigendian = 0;
    chunk = ( CHUNK * ) buf;
    chunk64 = ( CHUNK64 * ) buf;
    if ( cmp ( chunk->name, "RIFF" ) < 0 ) {
        if ( cmp ( chunk->name, "RF64" ) == 1 || cmp(chunk->name, "BW64") == 1) {
            is_rf64 = 1;
        } else if ( cmp ( chunk->name, "RIFX" ) == 1 ) {
            f_info->bigendian = 1;
        } else if ( memcmp ( chunk64->guid, w64_guid_riff, sizeof ( w64_guid_riff ) ) == 0 ) {
            is_w64 = 1;
            chunk_size = ( int ) sizeof ( CHUNK64 );
        } else {
            return -1;
        }
    }

    buf += chunk_size;
    nbuf -= chunk_size;

    if ( is_w64 ) {
        if ( nbuf < sizeof ( w64_guid_wave ) || memcmp ( buf, w64_guid_wave, sizeof ( w64_guid_wave) ) != 0 )
            return -1;
        buf += sizeof ( w64_guid_wave );
        nbuf -= sizeof ( w64_guid_wave );
    } else {
        if ( nbuf < 4 || cmp ( buf, "WAVE" ) < 0 )
            return -1;
        buf += 4;
        nbuf -= 4;
    }

    if ( is_rf64 ) {
        if ( nbuf < sizeof ( CHUNKDS64 ) )
            return -1;
        chunk_ds64 = ( CHUNKDS64 * ) buf;
        if ( cmp ( chunk_ds64->name, "ds64" ) < 0 )
            return -1;
        size = get_field ( chunk_ds64->size, sizeof ( chunk_ds64->size ), f_info->bigendian ) + sizeof ( chunk_ds64->name ) + sizeof ( chunk_ds64->size );
        if ( size < sizeof ( CHUNKDS64 ) )
            return -1;
        if ( size % 2 != 0 ) size++;
        buf += size;
        nbuf -= size;
    }

/* look for fmt chunk */
    for ( ;; )
    {
        if ( nbuf < chunk_size ) {
            nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	        if ( nbuf < 0 )
		        return -1;
            buf = buf_in;
        }
        if ( !is_w64 )
            chunk = ( CHUNK * ) buf;
        else
            chunk64 = ( CHUNK64 * ) buf;
        buf += chunk_size;
        nbuf -= chunk_size;
        if ( !is_w64 ) {
            if ( cmp ( chunk->name, "fmt " ) > 0 )
                break;
            size = get_field ( chunk->size, 4, f_info->bigendian );
            if ( size % 2 != 0 ) size++;
        } else {
            if ( memcmp ( chunk64->guid, w64_guid_fmt, sizeof ( w64_guid_fmt ) ) == 0 )
                break;
            size = get_field64 ( chunk64->size, 8, f_info->bigendian );
            if ( size < chunk_size ) /* From specs: the chunk size fields directly following the chunk-GUID and preceeding the chunk body, include the size of the chunk-GUID and the chunk length field itself. */
                return -1;
            size -= chunk_size;
            if ( size % 8 != 0 ) size += 8 - ( size % 8 ); /* From specs: All Wave64 chunks are byte-aligned on 8-byte boundaries, but their chunk size fields do not include any padding if it is necessary. */
        }
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
    if ( !is_w64 ) {
        size = get_field ( chunk->size, sizeof ( chunk->size ), f_info->bigendian );
        if ( size % 2 != 0 ) size++;
    } else {
        size = get_field64 ( chunk64->size, sizeof ( chunk64->size ), f_info->bigendian );
        if ( size < chunk_size )
            return -1;
        size -= chunk_size;
        if ( size % 8 != 0 ) size += 8 - ( size % 8 ); /* All Wave64 chunks are byte-aligned on 8-byte boundaries, but their chunk size fields do not include any padding if it is necessary */
    }
    if ( size < sizeof ( WAVEFMT ) || size >= ( 1UL << 31UL ) ) /* sanity check */
        return -1;
    size -= sizeof ( WAVEFMT );
    buf += size;
    nbuf -= size;

    if ( nbuf < chunk_size ) {
        nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	    if ( nbuf < 0 )
		    return -1;
        buf = buf_in;
    }

    /* look for data chunk */
    for ( ;; )
    {
        if ( nbuf < chunk_size ) {
            nbuf = whead_read_file ( handle, buf_in, nbuf0, nbuf );
	        if ( nbuf < 0 )
		        return -1;
            buf = buf_in;
        }
        if ( !is_w64 )
            chunk = ( CHUNK * ) buf;
        else
            chunk64 = ( CHUNK64 * ) buf;
        buf += chunk_size;
        nbuf -= chunk_size;
        if ( !is_w64 ) {
            size = get_field ( chunk->size, 4, f_info->bigendian );
            if ( size % 2 != 0 ) size++;
            if ( cmp ( chunk->name, "data" ) > 0 ) {
                if ( is_rf64 && size == 0xFFFFFFFF ) {
                    size = get_field64 ( chunk_ds64->dataSize, sizeof ( chunk_ds64->dataSize ), f_info->bigendian );
                    if ( size % 2 != 0 ) size++;
                }
                *data_size = size; /* return actual audio data size to the frontend */
                break;
            }
        } else {
            size = get_field64 ( chunk64->size, 8, f_info->bigendian );
            if ( size < chunk_size )
                return -1;
            size -= chunk_size;
            if ( size % 8 != 0 ) size += 8 - ( size % 8 );
            if ( memcmp ( chunk64->guid, w64_guid_data, sizeof ( w64_guid_data ) ) == 0 ) {
                *data_size = size; /* return actual audio data size to the frontend */
                break;
            }
        }
        if ( size >= ( 1UL << 31UL ) ) /* sanity check */
            return -1;
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

