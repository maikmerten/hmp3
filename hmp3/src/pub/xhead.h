/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: xhead.h,v 1.1 2005/07/13 17:22:24 rggammon Exp $ 
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

#ifndef _XHEAD_H_
#define _XHEAD_H_

#ifdef __cplusplus
extern "C" {
#endif

// A Xing header may be present in the ancillary
// data field of the first frame of an mp3 bitstream
// The Xing header (optionally) contains
//      frames      total number of audio frames in the bitstream
//      bytes       total number of bytes in the bitstream
//      toc         table of contents

// toc (table of contents) gives seek points
// for random access
// the ith entry determines the seek point for
// i-percent duration
// seek point in bytes = (toc[i]/256.0) * total_bitstream_bytes
// e.g. half duration seek point = (toc[50]/256.0) * total_bitstream_bytes

#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008
#define VBR_RESERVEDA_FLAG  0x0010
#define VBR_RESERVEDB_FLAG  0x0020

//**** see tomp3.c for an application example ****//

int XingHeader ( int samprate, int h_mode, int cr_bit, int original,
                 int flags, int frames, int bs_bytes,
                 int vbr_scale,
                 unsigned char *toc, unsigned char *buf, unsigned char *buf20,
                 unsigned char *buf20B, int nBitRateIndex );
// creates an mp3 frame that contains a Xing header
// return  0 = fail or frame_bytes
// input:
//      samprate    sample rate (e.g. 44100)
//      h_mode      mpeg coding mode
//      cr_bit      copyright bit to set in MPEG frame header
//      original    original/copy bit to set in MPEG frame header
//      vbr_scale   vbr scale value to set in header
//      flags       controls data included in header
//                  e.g. flags = FRAMES_FLAG | BYTES_FLAG
//                  e.g. flags = FRAMES_FLAG | BYTES_FLAG | TOC_FLAG
//      frames      number frames in bitstream, including
//                  the header frame, may be place holder, e.g. 0
//      bytes       number bytes in bitstream, including
//                  the header frame, may be place holder, e.g. 0
//      toc         pointer to caller supplied toc buffer,
//                  may be NULL.  Use NULL for 1. no toc,  
//                  2. place holder  3. toc constructed
//                  by XingHeaderTOC
// output
//      buf         buffer to place the mp3 frame

int XingHeaderUpdate ( int frames, int bs_bytes,
                       int vbr_scale,
                       unsigned char *toc, unsigned char *buf,
                       unsigned char *buf20, unsigned char *buf20B );
// update the information in a previously created mp3 frame 
// that contains a Xing header
// return  0 = fail,  1 = success
// input
//      frames      number frames in bitstream, including
//                  the header frame, may be place holder, e.g. 0
//      bytes       number bytes in bitstream, including
//                  the header frame, may be place holder, e.g. 0
//      toc         pointer to caller supplied toc buffer,
//                  may be NULL.  Use NULL for 1. no toc,  
//                  2. toc constructed by XingHeaderTOC
// input/output
//      buf         buffer containing mp3 frame to update

int XingHeaderTOC ( int frames, int bs_bytes );

// Dynamically (single pass) build TOC
// XingHeader must be called to initialize XingHeaderTOC
// input
//      frames      number of frames currently in bitstream, including
//                  the header frame.
//      bytes       number of bytes currently in bitstream, including
//                  the header frame.
// returns
//      toc_counter

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _XHEAD_H_ */
