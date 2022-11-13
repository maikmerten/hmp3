/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: l3e.h,v 1.1 2005/07/13 17:22:24 rggammon Exp $ 
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

#ifndef _L3E_H_
#define _L3E_H_

#define GLOBAL_GAIN_SCALE (4*15)

/* #define GLOBAL_GAIN_SCALE 0 */

/*--------------------------------------------------------------*/
typedef struct
{
    float sig;
    float mask;
}
SIG_MASK;

/*--------------------------------------------------------------*/
typedef struct
{
    int l[23];
    int s[14];
}
BAND_TABLE;

/*--------------------------------------------------------------*/
typedef struct
{
    unsigned char *bs_ptr;
    unsigned char *bs_ptr0;
    unsigned int bitbuf;
    int bits;
}
BITDAT;

/*-- side info ---*/
typedef struct
{
    int part2_3_length;
    int big_values;
    int global_gain;
    int scalefac_compress;
    int window_switching_flag;
    int block_type;
    int mixed_block_flag;
    int table_select[3];
    int subblock_gain[3];
    int region0_count;
    int region1_count;
    int preflag;
    int scalefac_scale;
    int count1table_select;
    int aux_nquads;     // ba aux data
    int aux_bits;       // ba huff data bit count
    int aux_not_null;   // signals to encode sf+huff (even if huff=0)
    int aux_nreg[3];    // pairs in huff regions
    int block_type_prev;        // aux, previous block type 0,1,2,
    int short_flag_current;     // aux, 0/1 use short blocks
    int short_flag_next;        // aux, 0/1 use short blocks
}
GR;
typedef struct
{
    int mode;
    int mode_ext;

    /*---------------*/
    int main_data_begin;
    int private_bits;

    /*---------------*/
    int scfsi[2];       /* 4 bit flags [ch] */
    GR gr[2][2];        /* [gran][ch] */
}
SIDE_INFO;

/*-----------------------------------------------------------*/

/*-- scale factors ---*/
typedef struct
{
    int l[23];  /* [cb] */
    int s[3][13];       /* [window][cb] */
}
SCALEFACT;

/*-----------------------------------------------------------*/
typedef struct
{
    int code;
    int len;
}
HUFF_STRUCT;

//typedef struct {
//    unsigned char code;
//    unsigned char len;
//} BYTE_HUFF_STRUCT;
typedef struct
{
    int code;
    int len;
}
BYTE_HUFF_STRUCT;

///*-----------------------------------------------------------*/
//typedef union {
//    int s;
//    float x;
//} SAMPLE;
///*-----------------------------------------------------------*/

#endif //  _L3E_H_
