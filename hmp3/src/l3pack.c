/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: l3pack.c,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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

#include "l3e.h"
#include "hxtypes.h"

extern int iframe;      // for testing

static int ip1, ip2, ip3;       // static just to eliminate warnings
static int is_sfmax2, is_sfmax3;// static just to eliminate warnings

static unsigned char *buf, *buf0;
static int room, bitbuf;

static int bit_pos_start, bit_pos_end;

static const unsigned char look_sf_compress[5][4] = {   /* [slen1][slen2] */
    0, 1, 2, 3,
    5, 5, 6, 7,
    8, 8, 9, 10,
    4, 11, 12, 13,
    14, 14, 14, 15,
};
static const unsigned char slen_table[16][2] = {
    0, 0, 0, 1,
    0, 2, 0, 3,
    3, 0, 1, 1,
    1, 2, 1, 3,
    2, 1, 2, 2,
    2, 3, 3, 1,
    3, 2, 3, 3,
    4, 2, 4, 3,
};

/* huff coding tables */
static const struct
{
    unsigned char code;
    unsigned char len;
}
huff_quada[16] =
{
    1, 1,
    5, 4,
    4, 4,
    5, 5,
    6, 4,
    5, 6, 4, 5, 4, 6, 7, 4, 3, 5, 6, 5, 0, 6, 7, 5, 2, 6, 3, 6, 1, 6,};

#include "htable.h"

int L3_pack_sf_short_MPEG1 ( SCALEFACT * sf );
int L3_pack_sf_short_MPEG2 ( SCALEFACT * sf,
                             int is_chan, int nsf_stereo_short );

/*================================================================*/
void
L3_outbits_init ( unsigned char *outbuf )
{
    buf0 = buf = outbuf;
    bitbuf = 0;
    room = 32;
}

/*--------------------------------------------------------------------*/
void
L3_outbits ( unsigned int x, int nbits )
{
    if ( room < nbits )
    {
        while ( room < 24 )
        {       /* table 13 max code len = 19 */
            *buf++ = ( unsigned char ) ( bitbuf >> ( 24 - room ) );
            room += 8;
        }
    }
    bitbuf = ( bitbuf << nbits ) | x;
    room -= nbits;
}

/*--------------------------------------------------------------------*/
static void
outbits ( unsigned int x, int nbits )
{
    if ( room < nbits )
    {
        while ( room < 24 )
        {       /* table 13 max code len = 19 */
            *buf++ = ( unsigned char ) ( bitbuf >> ( 24 - room ) );
            room += 8;
        }
    }
    bitbuf = ( bitbuf << nbits ) | x;
    room -= nbits;
}

/*--------------------------------------------------------------------*/
int
L3_outbits_flush (  )   /* flush to byte boundary, return byte count */
{
    while ( room < 24 )
    {
        *buf++ = ( unsigned char ) ( bitbuf >> ( 24 - room ) );
        room += 8;
    }
    if ( room < 32 )
    {
        *buf++ = ( unsigned char ) ( bitbuf << ( room - 24 ) );
    }
    room = 32;

    return ( buf - buf0 );
}

/*====================================================================*/
int
L3_pack_sf_MPEG1 ( SCALEFACT * sf, int block_type )
{
    int i, n;
    int scalefac_compress;
    int sfmax1, sfmax2;
    int slen1, slen2;

    if ( block_type == 2 )
    {
        return L3_pack_sf_short_MPEG1 ( sf );
    }

/* save pos for part2_3_len returned by huff */
    bit_pos_start = ( ( buf - buf0 ) << 3 ) + ( 32 - room );

/* determine slen1 slen2, scalefact_compress */
    sfmax2 = sfmax1 = 0;
    for ( i = 0; i < 11; i++ )
    {
        if ( sf->l[i] > sfmax1 )
            sfmax1 = sf->l[i];
    }

    for ( ; i < 21; i++ )
    {
        if ( sf->l[i] > sfmax2 )
            sfmax2 = sf->l[i];
    }

    n = 1;
    sfmax1++;
    for ( slen1 = 0; slen1 < 4; slen1++ )
    {
        if ( sfmax1 <= n )
            break;
        n += n;
    }   /* if fall through slen1 = 4 */

    n = 1;
    sfmax2++;
    for ( slen2 = 0; slen2 < 3; slen2++ )
    {
        if ( sfmax2 <= n )
            break;
        n += n;
    }   /* if fall through slen2 = 3 */

    scalefac_compress = look_sf_compress[slen1][slen2];
    slen1 = slen_table[scalefac_compress][0];
    slen2 = slen_table[scalefac_compress][1];

    for ( i = 0; i < 11; i++ )
        outbits ( sf->l[i], slen1 );
    for ( ; i < 21; i++ )
        outbits ( sf->l[i], slen2 );

    return scalefac_compress;
}

/*====================================================================*/
int
L3_pack_sf_short_MPEG1 ( SCALEFACT * sf )
{
    int i, n;
    int scalefac_compress;
    int sfmax1, sfmax2;
    int slen1, slen2;

// short, block_type = 2 only

/* save pos for part2_3_len returned by huff */
    bit_pos_start = ( ( buf - buf0 ) << 3 ) + ( 32 - room );

/* determine slen1 slen2, scalefact_compress */
    sfmax2 = sfmax1 = 0;
    for ( i = 0; i < 6; i++ )
    {
        sfmax1 = HX_MAX ( sfmax1, sf->s[0][i] );
        sfmax1 = HX_MAX ( sfmax1, sf->s[1][i] );
        sfmax1 = HX_MAX ( sfmax1, sf->s[2][i] );
    }
    for ( ; i < 12; i++ )
    {
        sfmax2 = HX_MAX ( sfmax2, sf->s[0][i] );
        sfmax2 = HX_MAX ( sfmax2, sf->s[1][i] );
        sfmax2 = HX_MAX ( sfmax2, sf->s[2][i] );
    }

    n = 1;
    sfmax1++;
    for ( slen1 = 0; slen1 < 4; slen1++ )
    {
        if ( sfmax1 <= n )
            break;
        n += n;
    }   /* if fall through slen1 = 4 */

    n = 1;
    sfmax2++;
    for ( slen2 = 0; slen2 < 3; slen2++ )
    {
        if ( sfmax2 <= n )
            break;
        n += n;
    }   /* if fall through slen2 = 3 */

    scalefac_compress = look_sf_compress[slen1][slen2];
    slen1 = slen_table[scalefac_compress][0];
    slen2 = slen_table[scalefac_compress][1];

    for ( i = 0; i < 6; i++ )
    {
        outbits ( sf->s[0][i], slen1 );
        outbits ( sf->s[1][i], slen1 );
        outbits ( sf->s[2][i], slen1 );
    }
    for ( ; i < 12; i++ )
    {
        outbits ( sf->s[0][i], slen2 );
        outbits ( sf->s[1][i], slen2 );
        outbits ( sf->s[2][i], slen2 );
    }

    return scalefac_compress;
}

/*====================================================================*/

/*====================================================================*/
int
L3_pack_sf_MPEG1B ( SCALEFACT * sf, int chan, int igr, int *pscfsi )
{
    int i, n;
    int scalefac_compress;
    int sfmax1, sfmax2;
    int slen1, slen2;
    int scfsi, t;
    static int sf_save[2][21];
    static int tcnt, dcnt, dcnt1, dcnt2, dcnt3, dcnt4;

    scfsi = 0;
    if ( igr == 0 )
    {
        for ( i = 0; i < 21; i++ )
        {
            sf_save[chan][i] = sf->l[i];
        }
    }
    else
    {
        for ( t = 0, i = 0; i < 6; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;

        for ( t = 0; i < 11; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;

        for ( t = 0; i < 16; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;

        for ( t = 0; i < 21; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;
    }

/* save pos for part2_3_len returned by huff */
    bit_pos_start = ( ( buf - buf0 ) << 3 ) + ( 32 - room );

/* determine slen1 slen2, scalefact_compress */
    sfmax2 = sfmax1 = 0;
    if ( ( scfsi & 8 ) == 0 )
    {
        for ( i = 0; i < 6; i++ )
        {
            if ( sf->l[i] > sfmax1 )
                sfmax1 = sf->l[i];
        }
    }
    if ( ( scfsi & 4 ) == 0 )
    {
        for ( i = 6; i < 11; i++ )
        {
            if ( sf->l[i] > sfmax1 )
                sfmax1 = sf->l[i];
        }
    }
    if ( ( scfsi & 2 ) == 0 )
    {
        for ( i = 11; i < 16; i++ )
        {
            if ( sf->l[i] > sfmax2 )
                sfmax2 = sf->l[i];
        }
    }
    if ( ( scfsi & 1 ) == 0 )
    {
        for ( i = 16; i < 21; i++ )
        {
            if ( sf->l[i] > sfmax2 )
                sfmax2 = sf->l[i];
        }
    }
    n = 1;
    sfmax1++;
    for ( slen1 = 0; slen1 < 4; slen1++ )
    {
        if ( sfmax1 <= n )
            break;
        n += n;
    }   /* if fall through slen1 = 4 */

    n = 1;
    sfmax2++;
    for ( slen2 = 0; slen2 < 3; slen2++ )
    {
        if ( sfmax2 <= n )
            break;
        n += n;
    }   /* if fall through slen2 = 3 */

    scalefac_compress = look_sf_compress[slen1][slen2];
    slen1 = slen_table[scalefac_compress][0];
    slen2 = slen_table[scalefac_compress][1];

    if ( ( scfsi & 8 ) == 0 )
    {
        for ( i = 0; i < 6; i++ )
            outbits ( sf->l[i], slen1 );
    }
    if ( ( scfsi & 4 ) == 0 )
    {
        for ( i = 6; i < 11; i++ )
            outbits ( sf->l[i], slen1 );
    }
    if ( ( scfsi & 2 ) == 0 )
    {
        for ( i = 11; i < 16; i++ )
            outbits ( sf->l[i], slen2 );
    }
    if ( ( scfsi & 1 ) == 0 )
    {
        for ( i = 16; i < 21; i++ )
            outbits ( sf->l[i], slen2 );
    }

    *pscfsi = scfsi;

    return scalefac_compress;
}

/*====================================================================*/

/*====================================================================*/
int
L3_pack_sf_MPEG1B2 ( SCALEFACT * sf, int chan, int igr,
                     int *pscfsi, int not_null )
{
    int i, n;
    int scalefac_compress;
    int sfmax1, sfmax2;
    int slen1, slen2;
    int scfsi, t;
    static int sf_save[2][21];

//static int tcnt, dcnt, dcnt1, dcnt2, dcnt3, dcnt4;

    scalefac_compress = 0;
    scfsi = 0;
    if ( igr == 0 )
    {
        for ( i = 0; i < 21; i++ )
        {
            sf_save[chan][i] = sf->l[i];
        }
    }
    else
    {
        for ( t = 0, i = 0; i < 6; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;

        for ( t = 0; i < 11; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;

        for ( t = 0; i < 16; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;

        for ( t = 0; i < 21; i++ )
            t |= ( sf_save[chan][i] - sf->l[i] );
        scfsi <<= 1;
        if ( t == 0 )
            scfsi |= 1;
    }

/* save pos for part2_3_len returned by huff */
    bit_pos_start = ( ( buf - buf0 ) << 3 ) + ( 32 - room );

    if ( not_null == 0 )
        goto exit;

/* determine slen1 slen2, scalefact_compress */
    sfmax2 = sfmax1 = 0;
    if ( ( scfsi & 8 ) == 0 )
    {
        for ( i = 0; i < 6; i++ )
        {
            if ( sf->l[i] > sfmax1 )
                sfmax1 = sf->l[i];
        }
    }
    if ( ( scfsi & 4 ) == 0 )
    {
        for ( i = 6; i < 11; i++ )
        {
            if ( sf->l[i] > sfmax1 )
                sfmax1 = sf->l[i];
        }
    }
    if ( ( scfsi & 2 ) == 0 )
    {
        for ( i = 11; i < 16; i++ )
        {
            if ( sf->l[i] > sfmax2 )
                sfmax2 = sf->l[i];
        }
    }
    if ( ( scfsi & 1 ) == 0 )
    {
        for ( i = 16; i < 21; i++ )
        {
            if ( sf->l[i] > sfmax2 )
                sfmax2 = sf->l[i];
        }
    }
    n = 1;
    sfmax1++;
    for ( slen1 = 0; slen1 < 4; slen1++ )
    {
        if ( sfmax1 <= n )
            break;
        n += n;
    }   /* if fall through slen1 = 4 */

    n = 1;
    sfmax2++;
    for ( slen2 = 0; slen2 < 3; slen2++ )
    {
        if ( sfmax2 <= n )
            break;
        n += n;
    }   /* if fall through slen2 = 3 */

    scalefac_compress = look_sf_compress[slen1][slen2];
    slen1 = slen_table[scalefac_compress][0];
    slen2 = slen_table[scalefac_compress][1];

    if ( ( scfsi & 8 ) == 0 )
    {
        for ( i = 0; i < 6; i++ )
            outbits ( sf->l[i], slen1 );
    }
    if ( ( scfsi & 4 ) == 0 )
    {
        for ( i = 6; i < 11; i++ )
            outbits ( sf->l[i], slen1 );
    }
    if ( ( scfsi & 2 ) == 0 )
    {
        for ( i = 11; i < 16; i++ )
            outbits ( sf->l[i], slen2 );
    }
    if ( ( scfsi & 1 ) == 0 )
    {
        for ( i = 16; i < 21; i++ )
            outbits ( sf->l[i], slen2 );
    }

  exit:

    *pscfsi = scfsi;

    return scalefac_compress;
}

/*====================================================================*/
int
L3_pack_sf_MPEG2 ( SCALEFACT * sf, int is_chan, int nsf_stereo,
                   int nsf_stereo_short, int block_type )
{
    int i, n;
    int scalefac_compress;
    int sfmax1, sfmax2, sfmax3, sfmax4;
    int slen1, slen2, slen3, slen4;
    int ip_value;

    if ( block_type == 2 )
    {
        return L3_pack_sf_short_MPEG2 ( sf, is_chan, nsf_stereo_short );
    }

/* save pos for part2_3_len returned by huff */
    bit_pos_start = ( ( buf - buf0 ) << 3 ) + ( 32 - room );

/* determine slen1 slen2, scalefact_compress */
    sfmax4 = sfmax3 = sfmax2 = sfmax1 = 0;
    if ( is_chan )
    {   // intensity stereo channel
        ip1 = ip2 = ip3 = 0;    // illegal is position flag
        is_sfmax2 = is_sfmax3 = -1;
        for ( i = 0; i < 7; i++ )
        {
            if ( sf->l[i] >= 999 )
                ip1 = 1;
            else if ( ( sf->l[i] > sfmax1 ) )
                sfmax1 = sf->l[i];
        }
        for ( ; i < 14; i++ )
        {
            if ( sf->l[i] >= 999 )
            {
                ip2 = 1;
                continue;
            }
            if ( ( sf->l[i] > sfmax2 ) )
                sfmax2 = sf->l[i];
            if ( i < nsf_stereo )
                continue;
            if ( sf->l[i] > is_sfmax2 )
                is_sfmax2 = sf->l[i];
        }
        for ( ; i < 21; i++ )
        {
            if ( sf->l[i] >= 999 )
            {
                ip3 = 1;
                continue;
            }
            if ( ( sf->l[i] > sfmax3 ) )
                sfmax3 = sf->l[i];
            if ( i < nsf_stereo )
                continue;
            if ( sf->l[i] > is_sfmax3 )
                is_sfmax3 = sf->l[i];
        }
    }
    else
    {
        for ( i = 0; i < 6; i++ )
            if ( sf->l[i] > sfmax1 )
                sfmax1 = sf->l[i];
        for ( ; i < 11; i++ )
            if ( sf->l[i] > sfmax2 )
                sfmax2 = sf->l[i];
        for ( ; i < 16; i++ )
            if ( sf->l[i] > sfmax3 )
                sfmax3 = sf->l[i];
        for ( ; i < 21; i++ )
            if ( sf->l[i] > sfmax4 )
                sfmax4 = sf->l[i];
    }

    n = 1;
    sfmax1++;
    for ( slen1 = 0; slen1 < 4; slen1++ )
    {
        if ( sfmax1 <= n )
            break;
        n += n;
    }   /* if fall through slen1 = 4 */
    n = 1;
    sfmax2++;
    for ( slen2 = 0; slen2 < 4; slen2++ )
    {
        if ( sfmax2 <= n )
            break;
        n += n;
    }   /* if fall through slen2 = 4 */
    n = 1;
    sfmax3++;
    for ( slen3 = 0; slen3 < 3; slen3++ )
    {
        if ( sfmax3 <= n )
            break;
        n += n;
    }   /* if fall through slen3 = 3 */
    n = 1;
    sfmax4++;
    for ( slen4 = 0; slen4 < 3; slen4++ )
    {
        if ( sfmax4 <= n )
            break;
        n += n;
    }   /* if fall through slen4 = 3 */

    if ( is_chan )
    {
        // check is_pos, cannot have valid is_pos take on illegal is_value
        if ( is_sfmax2 == ( ( 1 << slen2 ) - 1 ) )
        {
            slen2++;
        }
        if ( is_sfmax3 == ( ( 1 << slen3 ) - 1 ) )
        {
            slen3++;
        }
        // set illegal is pos
        if ( ip1 )
        {
            ip_value = ( 1 << slen1 ) - 1;
            for ( i = 0; i < 7; i++ )
                if ( sf->l[i] >= 999 )
                    sf->l[i] = ip_value;
        }
        if ( ip2 )
        {
            ip_value = ( 1 << slen2 ) - 1;
            for ( i = 7; i < 14; i++ )
                if ( sf->l[i] >= 999 )
                    sf->l[i] = ip_value;
        }
        if ( ip3 )
        {
            ip_value = ( 1 << slen3 ) - 1;
            for ( i = 14; i < 21; i++ )
                if ( sf->l[i] >= 999 )
                    sf->l[i] = ip_value;
        }

        scalefac_compress = slen3 + 6 * slen2 + 36 * slen1;
        //       intensity_scale ;
        scalefac_compress = scalefac_compress + scalefac_compress + 1;
        for ( i = 0; i < 7; i++ )
            outbits ( sf->l[i], slen1 );
        for ( ; i < 14; i++ )
            outbits ( sf->l[i], slen2 );
        for ( ; i < 21; i++ )
            outbits ( sf->l[i], slen3 );
    }
    else
    {
        scalefac_compress = slen4
            + ( slen3 << 2 ) + ( ( slen2 + 5 * slen1 ) << 4 );
        for ( i = 0; i < 6; i++ )
            outbits ( sf->l[i], slen1 );
        for ( ; i < 11; i++ )
            outbits ( sf->l[i], slen2 );
        for ( ; i < 16; i++ )
            outbits ( sf->l[i], slen3 );
        for ( ; i < 21; i++ )
            outbits ( sf->l[i], slen4 );
    }

    return scalefac_compress;
}

/*====================================================================*/
int
L3_pack_sf_short_MPEG2 ( SCALEFACT * sf, int is_chan, int nsf_stereo_short )
{
    int i, n;
    int scalefac_compress;
    int sfmax1, sfmax2, sfmax3, sfmax4;
    int slen1, slen2, slen3, slen4;
    int ip_value;
    int w;

// short, block_type = 2 only

/* save pos for part2_3_len returned by huff */
    bit_pos_start = ( ( buf - buf0 ) << 3 ) + ( 32 - room );

/* determine slen1 slen2, scalefact_compress */
    sfmax4 = sfmax3 = sfmax2 = sfmax1 = 0;
    if ( is_chan )
    {   // intensity stereo channel
        ip1 = ip2 = ip3 = 0;    // illegal is position flag
        is_sfmax2 = is_sfmax3 = -1;
        for ( w = 0; w < 3; w++ )
        {
            for ( i = 0; i < 4; i++ )
            {
                if ( sf->s[w][i] >= 999 )
                    ip1 = 1;
                else if ( ( sf->s[w][i] > sfmax1 ) )
                    sfmax1 = sf->s[w][i];
            }
            for ( ; i < 8; i++ )
            {
                if ( sf->s[w][i] >= 999 )
                {
                    ip2 = 1;
                    continue;
                }
                if ( ( sf->s[w][i] > sfmax2 ) )
                    sfmax2 = sf->s[w][i];
                if ( i < nsf_stereo_short )
                    continue;
                if ( sf->s[w][i] > is_sfmax2 )
                    is_sfmax2 = sf->s[w][i];
            }
            for ( ; i < 12; i++ )
            {
                if ( sf->s[w][i] >= 999 )
                {
                    ip3 = 1;
                    continue;
                }
                if ( ( sf->s[w][i] > sfmax3 ) )
                    sfmax3 = sf->s[w][i];
                if ( i < nsf_stereo_short )
                    continue;
                if ( sf->s[w][i] > is_sfmax3 )
                    is_sfmax3 = sf->s[w][i];
            }
        }
    }
    else
    {
        for ( w = 0; w < 3; w++ )
        {
            for ( i = 0; i < 3; i++ )
                if ( sf->s[w][i] > sfmax1 )
                    sfmax1 = sf->s[w][i];
            for ( ; i < 6; i++ )
                if ( sf->s[w][i] > sfmax2 )
                    sfmax2 = sf->s[w][i];
            for ( ; i < 9; i++ )
                if ( sf->s[w][i] > sfmax3 )
                    sfmax3 = sf->s[w][i];
            for ( ; i < 12; i++ )
                if ( sf->s[w][i] > sfmax4 )
                    sfmax4 = sf->s[w][i];
        }
    }

    n = 1;
    sfmax1++;
    for ( slen1 = 0; slen1 < 4; slen1++ )
    {
        if ( sfmax1 <= n )
            break;
        n += n;
    }   /* if fall through slen1 = 4 */
    n = 1;
    sfmax2++;
    for ( slen2 = 0; slen2 < 4; slen2++ )
    {
        if ( sfmax2 <= n )
            break;
        n += n;
    }   /* if fall through slen2 = 4 */
    n = 1;
    sfmax3++;
    for ( slen3 = 0; slen3 < 3; slen3++ )
    {
        if ( sfmax3 <= n )
            break;
        n += n;
    }   /* if fall through slen3 = 3 */
    n = 1;
    sfmax4++;
    for ( slen4 = 0; slen4 < 3; slen4++ )
    {
        if ( sfmax4 <= n )
            break;
        n += n;
    }   /* if fall through slen4 = 3 */

    if ( is_chan )
    {
        // check is_pos, cannot have valid is_pos take on illegal is_value
        if ( is_sfmax2 == ( ( 1 << slen2 ) - 1 ) )
        {
            slen2++;
        }
        if ( is_sfmax3 == ( ( 1 << slen3 ) - 1 ) )
        {
            slen3++;
        }
        // set illegal is pos
        for ( w = 0; w < 3; w++ )
        {
            if ( ip1 )
            {
                ip_value = ( 1 << slen1 ) - 1;
                for ( i = 0; i < 4; i++ )
                    if ( sf->s[w][i] >= 999 )
                        sf->s[w][i] = ip_value;
            }
            if ( ip2 )
            {
                ip_value = ( 1 << slen2 ) - 1;
                for ( i = 4; i < 8; i++ )
                    if ( sf->s[w][i] >= 999 )
                        sf->s[w][i] = ip_value;
            }
            if ( ip3 )
            {
                ip_value = ( 1 << slen3 ) - 1;
                for ( i = 8; i < 12; i++ )
                    if ( sf->s[w][i] >= 999 )
                        sf->s[w][i] = ip_value;
            }
        }

        scalefac_compress = slen3 + 6 * slen2 + 36 * slen1;
        //       intensity_scale ;
        scalefac_compress = scalefac_compress + scalefac_compress + 1;
        for ( i = 0; i < 4; i++ )
        {
            outbits ( sf->s[0][i], slen1 );
            outbits ( sf->s[1][i], slen1 );
            outbits ( sf->s[2][i], slen1 );
        }
        for ( ; i < 8; i++ )
        {
            outbits ( sf->s[0][i], slen2 );
            outbits ( sf->s[1][i], slen2 );
            outbits ( sf->s[2][i], slen2 );
        }
        for ( ; i < 12; i++ )
        {
            outbits ( sf->s[0][i], slen3 );
            outbits ( sf->s[1][i], slen3 );
            outbits ( sf->s[2][i], slen3 );
        }
    }
    else
    {
        scalefac_compress = slen4
            + ( slen3 << 2 ) + ( ( slen2 + 5 * slen1 ) << 4 );
        for ( i = 0; i < 3; i++ )
        {
            outbits ( sf->s[0][i], slen1 );
            outbits ( sf->s[1][i], slen1 );
            outbits ( sf->s[2][i], slen1 );
        }
        for ( ; i < 6; i++ )
        {
            outbits ( sf->s[0][i], slen2 );
            outbits ( sf->s[1][i], slen2 );
            outbits ( sf->s[2][i], slen2 );
        }
        for ( ; i < 9; i++ )
        {
            outbits ( sf->s[0][i], slen3 );
            outbits ( sf->s[1][i], slen3 );
            outbits ( sf->s[2][i], slen3 );
        }
        for ( ; i < 12; i++ )
        {
            outbits ( sf->s[0][i], slen4 );
            outbits ( sf->s[1][i], slen4 );
            outbits ( sf->s[2][i], slen4 );
        }
    }

    return scalefac_compress;
}

/*====================================================================*/
typedef BYTE_HUFF_STRUCT BHUFF2[2];
typedef BYTE_HUFF_STRUCT BHUFF4[4];
typedef BYTE_HUFF_STRUCT BHUFF8[8];
typedef BYTE_HUFF_STRUCT BHUFF16[16];

typedef HUFF_STRUCT HUFF16[16];
typedef int iARRAY4[4];
typedef unsigned char ucARRAY4[4];

int
L3_pack_huff ( GR * gr_data, int ix[][2], unsigned char ixsign[][2] )
{
    int i, j, k, n;
    int x, y;
    int linbits, icase;
    void *table;
    BHUFF2 *ht1;
    BHUFF4 *ht2;
    BHUFF8 *ht3;
    BHUFF16 *ht4;
    HUFF16 *ht5;
    iARRAY4 *qix;
    ucARRAY4 *qixsign;
    int bits;

#ifdef _DEBUG
    int bitsh, tmp;

    bitsh = ( ( ( buf - buf0 ) << 3 ) + ( 32 - room ) );
#endif

    for ( i = 0; i < 3; i++ )
    {
        n = gr_data->aux_nreg[i];
        k = gr_data->table_select[i];
        icase = huff_case_table[k].icase;
        linbits = huff_case_table[k].linbits;
        table = huff_case_table[k].table;
        switch ( icase )
        {
        case 0:
            ix += n;
            ixsign += n;
            break;
        case 1:
            ht1 = table;
            for ( j = 0; j < n; j++ )
            {
                x = ix[j][0];
                y = ix[j][1];
                outbits ( ht1[x][y].code, ht1[x][y].len );
                if ( x )
                    outbits ( ixsign[j][0], 1 );
                if ( y )
                    outbits ( ixsign[j][1], 1 );
            }
            ix += n;
            ixsign += n;
            break;
        case 2:
            ht2 = table;
            for ( j = 0; j < n; j++ )
            {
                x = ix[j][0];
                y = ix[j][1];
                outbits ( ht2[x][y].code, ht2[x][y].len );
                if ( x )
                    outbits ( ixsign[j][0], 1 );
                if ( y )
                    outbits ( ixsign[j][1], 1 );
            }
            ix += n;
            ixsign += n;
            break;
        case 3:
            ht3 = table;
            for ( j = 0; j < n; j++ )
            {
                x = ix[j][0];
                y = ix[j][1];
                outbits ( ht3[x][y].code, ht3[x][y].len );
                if ( x )
                    outbits ( ixsign[j][0], 1 );
                if ( y )
                    outbits ( ixsign[j][1], 1 );
            }
            ix += n;
            ixsign += n;
            break;
        case 4:
            ht4 = table;
            for ( j = 0; j < n; j++ )
            {
                x = ix[j][0];
                y = ix[j][1];
                outbits ( ht4[x][y].code, ht4[x][y].len );
                if ( x )
                    outbits ( ixsign[j][0], 1 );
                if ( y )
                    outbits ( ixsign[j][1], 1 );
            }
            ix += n;
            ixsign += n;
            break;
        case 5: /* have linbits */
            ht5 = table;
            for ( j = 0; j < n; j++ )
            {
                x = ix[j][0];
                y = ix[j][1];
                if ( x > 15 )
                    x = 15;
                if ( y > 15 )
                    y = 15;
                outbits ( ht5[x][y].code, ht5[x][y].len );
                if ( x >= 15 )
                    outbits ( ix[j][0] - 15, linbits );
                if ( x )
                    outbits ( ixsign[j][0], 1 );
                if ( y >= 15 )
                    outbits ( ix[j][1] - 15, linbits );
                if ( y )
                    outbits ( ixsign[j][1], 1 );
            }
            ix += n;
            ixsign += n;
            break;
        }       /* end switch */

    }   /* end region loop */

/*--------- quads ------*/

    qix = ( void * ) ix;
    qixsign = ( void * ) ixsign;

    n = gr_data->aux_nquads;

    if ( gr_data->count1table_select == 1 )
    {   /* quad b */
        for ( j = 0; j < n; j++ )
        {
            x = ( ( qix[j][0] << 3 ) + ( qix[j][1] << 2 ) +
                  ( qix[j][2] << 1 ) + qix[j][3] ) ^ 15;
            outbits ( x, 4 );
            if ( ( x & 8 ) == 0 )
                outbits ( qixsign[j][0], 1 );
            if ( ( x & 4 ) == 0 )
                outbits ( qixsign[j][1], 1 );
            if ( ( x & 2 ) == 0 )
                outbits ( qixsign[j][2], 1 );
            if ( ( x & 1 ) == 0 )
                outbits ( qixsign[j][3], 1 );
        }
    }
    else
    {   /* quada */
        for ( j = 0; j < n; j++ )
        {
            x = ( qix[j][0] << 3 ) + ( qix[j][1] << 2 ) + ( qix[j][2] << 1 ) +
                qix[j][3];
            outbits ( huff_quada[x].code, huff_quada[x].len );
            if ( ( x & 8 ) )
                outbits ( qixsign[j][0], 1 );
            if ( ( x & 4 ) )
                outbits ( qixsign[j][1], 1 );
            if ( ( x & 2 ) )
                outbits ( qixsign[j][2], 1 );
            if ( ( x & 1 ) )
                outbits ( qixsign[j][3], 1 );
        }
    }

    bits = ( ( ( buf - buf0 ) << 3 ) + ( 32 - room ) ) - bit_pos_start;

#ifdef _DEBUG
    tmp = ( ( ( buf - buf0 ) << 3 ) + ( 32 - room ) ) - bitsh;
    assert ( ( ( tmp =
                 ( ( ( buf - buf0 ) << 3 ) + ( 32 - room ) ) ) - bitsh ) ==
             gr_data->aux_bits );
#endif

    return bits;
}

/*====================================================================*/
void
L3_pack_side_MPEG1 ( unsigned char *bs_out, SIDE_INFO * side_info, int nchan )
{
    int igr, ch;

    L3_outbits_init ( bs_out );

    if ( side_info->mode != 1 )
        side_info->mode_ext = 0;

/* outbits(side_info->main_data_begin,9);  */

/* main data begin set later */
    outbits ( 0, 9 );
    if ( side_info->mode == 3 )
    {
        outbits ( side_info->private_bits, 5 );
    }
    else
    {
        outbits ( side_info->private_bits, 3 );
    }
    for ( ch = 0; ch < nchan; ch++ )
        outbits ( side_info->scfsi[ch], 4 );

/* this always 0 (both igr) for short blocks */

    for ( igr = 0; igr < 2; igr++ )
    {
        for ( ch = 0; ch < nchan; ch++ )
        {
            outbits ( side_info->gr[igr][ch].part2_3_length, 12 );
            outbits ( side_info->gr[igr][ch].big_values, 9 );
            outbits ( side_info->gr[igr][ch].global_gain, 8 );
            outbits ( side_info->gr[igr][ch].scalefac_compress, 4 );
            outbits ( side_info->gr[igr][ch].window_switching_flag, 1 );
            if ( side_info->gr[igr][ch].window_switching_flag )
            {
                outbits ( side_info->gr[igr][ch].block_type, 2 );
                outbits ( side_info->gr[igr][ch].mixed_block_flag, 1 );
                outbits ( side_info->gr[igr][ch].table_select[0], 5 );
                outbits ( side_info->gr[igr][ch].table_select[1], 5 );
                outbits ( side_info->gr[igr][ch].subblock_gain[0], 3 );
                outbits ( side_info->gr[igr][ch].subblock_gain[1], 3 );
                outbits ( side_info->gr[igr][ch].subblock_gain[2], 3 );
            }
            else
            {
                outbits ( side_info->gr[igr][ch].table_select[0], 5 );
                outbits ( side_info->gr[igr][ch].table_select[1], 5 );
                outbits ( side_info->gr[igr][ch].table_select[2], 5 );
                outbits ( side_info->gr[igr][ch].region0_count, 4 );
                outbits ( side_info->gr[igr][ch].region1_count, 3 );
            }
            outbits ( side_info->gr[igr][ch].preflag, 1 );
            outbits ( side_info->gr[igr][ch].scalefac_scale, 1 );
            outbits ( side_info->gr[igr][ch].count1table_select, 1 );
        }
    }

    L3_outbits_flush (  );

/* done */
}

/*====================================================================*/
void
L3_pack_side_MPEG2 ( unsigned char *bs_out,
                     SIDE_INFO * side_info, int nchan, int igr )
{
    int ch;

    L3_outbits_init ( bs_out );

    if ( side_info->mode != 1 )
        side_info->mode_ext = 0;

/* outbits(side_info->main_data_begin,9);  */

/* main data begin set later */
    outbits ( 0, 8 );
    if ( side_info->mode == 3 )
    {
        outbits ( side_info->private_bits, 1 );
    }
    else
    {
        outbits ( side_info->private_bits, 2 );
    }

    for ( ch = 0; ch < nchan; ch++ )
    {
        outbits ( side_info->gr[igr][ch].part2_3_length, 12 );
        outbits ( side_info->gr[igr][ch].big_values, 9 );
        outbits ( side_info->gr[igr][ch].global_gain, 8 );
        outbits ( side_info->gr[igr][ch].scalefac_compress, 9 );
        outbits ( side_info->gr[igr][ch].window_switching_flag, 1 );
        if ( side_info->gr[igr][ch].window_switching_flag )
        {
            outbits ( side_info->gr[igr][ch].block_type, 2 );
            outbits ( side_info->gr[igr][ch].mixed_block_flag, 1 );
            outbits ( side_info->gr[igr][ch].table_select[0], 5 );
            outbits ( side_info->gr[igr][ch].table_select[1], 5 );
            outbits ( side_info->gr[igr][ch].subblock_gain[0], 3 );
            outbits ( side_info->gr[igr][ch].subblock_gain[1], 3 );
            outbits ( side_info->gr[igr][ch].subblock_gain[2], 3 );
        }
        else
        {
            outbits ( side_info->gr[igr][ch].table_select[0], 5 );
            outbits ( side_info->gr[igr][ch].table_select[1], 5 );
            outbits ( side_info->gr[igr][ch].table_select[2], 5 );
            outbits ( side_info->gr[igr][ch].region0_count, 4 );
            outbits ( side_info->gr[igr][ch].region1_count, 3 );
        }
        outbits ( side_info->gr[igr][ch].scalefac_scale, 1 );
        outbits ( side_info->gr[igr][ch].count1table_select, 1 );
    }

    L3_outbits_flush (  );

/* done */
}

/*====================================================================*/
