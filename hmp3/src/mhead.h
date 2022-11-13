/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: mhead.h,v 1.1 2005/07/13 17:22:20 rggammon Exp $ 
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


/* mpeg audio header   */
typedef struct
{
    int sync;   /* 1 if valid sync */
    int id;
    int option;
    int prot;
    int br_index;
    int sr_index;
    int pad;
    int private_bit;
    int mode;
    int mode_ext;
    int cr;
    int original;
    int emphasis;
}
MPEG_HEAD;

/* portable mpeg audio decoder, decoder functions */
typedef struct
{
    int in_bytes;
    int out_bytes;
}
IN_OUT;

typedef struct
{
    int channels;
    int outvalues;
    long samprate;
    int bits;
    int framebytes;
    int type;
}
DEC_INFO;

#ifdef __cplusplus
extern "C"
{
#endif

    int head_info ( unsigned char *buf, unsigned int n, MPEG_HEAD * h );
    int head_info2 ( unsigned char *buf,
                     unsigned int n, MPEG_HEAD * h, int *br );

/* head_info returns framebytes > 0 for success */

/* audio_decode_init returns 1 for success, 0 for fail */

/* audio_decode returns in_bytes = 0 on sync loss */

    int audio_decode_init ( MPEG_HEAD * h, int framebytes_arg,
                            int reduction_code, int transform_code,
                            int convert_code, int freq_limit );
    void audio_decode_info ( DEC_INFO * info );
    IN_OUT audio_decode ( unsigned char *bs, short *pcm );

    int audio_decode8_init ( MPEG_HEAD * h, int framebytes_arg,
                             int reduction_code, int transform_code,
                             int convert_code, int freq_limit );
    void audio_decode8_info ( DEC_INFO * info );
    IN_OUT audio_decode8 ( unsigned char *bs, short *pcmbuf );

/*-- integer decode --*/
    int i_audio_decode_init ( MPEG_HEAD * h, int framebytes_arg,
                              int reduction_code, int transform_code,
                              int convert_code, int freq_limit );
    void i_audio_decode_info ( DEC_INFO * info );
    IN_OUT i_audio_decode ( unsigned char *bs, short *pcm );

#ifdef __cplusplus
}
#endif
