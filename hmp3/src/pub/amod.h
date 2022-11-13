/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: amod.h,v 1.1 2005/07/13 17:22:24 rggammon Exp $ 
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

#ifdef _WINDOWS
/*---------------------------------------------
MS C++ 4.1, disable warning of possible precision 
loss in floating point data tables
and conversions by assignment
-------------------------------------------------*/
#pragma warning(disable:4244)
#endif

typedef struct
{
    float x;
    float y;
}
CPAIR;

typedef struct
{
    float r2;
    float t2;
    float r1;
    float t1;
    float r;
    float t;
}
RTHETA_BUF;

typedef struct
{
    float x0;
    float y0;
    float rr0;
    float r0;

    float x1;
    float y1;
    float rr1;
    float r1;

    float x2;
    float y2;
    float rr2;
    float r2;
}
L3RTHETA_BUF;

/* --------- spd.c --------------*/
typedef struct
{
    int count;
    int off;
}
SPD_CNTL;

typedef struct
{
    float thres;        /* threshold for table gen, thres >= 0.0 */
    SPD_CNTL *control;
    float *coef;
    int size;
}
SPD_SETUP;

SPD_SETUP *spd_init_addr ( void );

/* --------- snr.c --------------*/
typedef struct
{
    float tmn;
    float valmin;
}
SNR_TABLE;
typedef struct
{
    float tmn;
    float valmin;
    float qthres;
}
SNR_TABLE2;

typedef struct
{
    SNR_TABLE *tv;
    int *npart;
    float *qthres;      /* layer 3 thres in quiet (abs thres) */
}
SNR_SETUP;

SNR_SETUP *snr_init_addr ( void );

/*---- Layer III smr, map calc partition to sf band --*/
typedef struct
{
    int n;
    float w1;
    float w2;
}
MAP_TABLE;

/* --------- ets.c --------------*/
typedef struct
{
    float ivsum;
    int nsum;
}
ETS_CNTL;

typedef struct
{
    int *npart1;
    int *npart2;
    ETS_CNTL *nsum;
    float *tat;
}
ETS_SETUP;

ETS_SETUP *ets_init_addr (  );

/*----------------------------------------*/
void fft1024 ( float *pcm, CPAIR * buf );
void fft512 ( float *pcm, CPAIR * buf );

/*---------------------------------------------------------------------*/
