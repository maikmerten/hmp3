/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: enc.h,v 1.2 2005/08/09 20:43:45 karll Exp $ 
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

#include "encapp.h"     // encoder application interface
                    // protos, E_CONTROL structure, MPEG_HEAD structure




#ifdef _WINDOWS
/*---------------------------------------------
MS C++ disable warning of possible precision 
loss in floating point data tables
and conversions by assignment
-------------------------------------------------*/
#pragma warning(disable:4244)
#endif

#define PEAK_INPUT 32768.0F

/*--- to eliminate printf:  --*/
//#define printf printnull

int printnull ( char *format, ... );

/* --------- bit allocation --------------*/

typedef struct
{        /* init tbaii tables */
    int nsb, nbal;      /* nsb = maxsb+1 */
    int nsbi[4];
    int deltasnr[4][16];
    int deltabits[4][16];
    int basteps[4][16];
}
TBAII;

typedef struct
{        /* ba.c */
    int deltasnr;
    int deltabits;
}
BAT;

BAT *ba_init_addr ( void );

/* --------- pack --------------*/
int *pack_init_addr ( int nsbi[4] );

/*---------------------setup ------------------------------------------*/

/*---
int setup_header(E_CONTROL *ec,  MPEG_HEAD *h);
int setup_abcd( MPEG_HEAD *h);
int setup_nsb( MPEG_HEAD *h);
int setup_nbal( MPEG_HEAD *h);
int setup_samprate( MPEG_HEAD *h);
int setup_maxbits( MPEG_HEAD *h, int totbitrate);
-------*/

/*-----------------mhead------------------------------------------------*/
int head_info ( unsigned char *buf, unsigned int n, MPEG_HEAD * h );

/*---------------------------------------------------------------------*/
