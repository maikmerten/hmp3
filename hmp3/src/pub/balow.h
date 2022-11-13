/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: balow.h,v 1.1 2005/07/13 17:34:48 rggammon Exp $ 
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

#ifndef _BALOW_H_
#define _BALOW_H_

#ifdef __cplusplus
extern "C"
{
#endif
//----------------------------------------------  
    typedef struct
    {
        int a;
        int b;
    }
    INTPAIR;
    float dbLog ( float x );    // full precision, needs 0.0 protection
    float dbLogB ( float x );   // max eps  0.01, no 0.0 protection required
    int mbLog ( float x );      // full precision, needs 0.0 protection

#define mbLogB mbLogC
    int mbLogB ( float x );     // max eps 1mb, relative eps 5.8e-5
    // no 0.0 protection required
    int mbLogC ( float x );     // max eps = 4mb, relative eps = 1.5e-4
    // no 0.0 protection required

    float mbExp ( int mb );     // inverse mb log

    int round_to_int ( float x );

    float pos_fmax ( float x, float y );
    float pos_fmin ( float x, float y );

    int LogSubber ( int mbNoise, int mbNoise2 );

    void vect_fpow34 ( float x[], float x34[], int n );
    float vect_fmax1 ( float x[], int n );
    void vect_fmax2 ( float x[], int n, float *yout );
    float vect_sign_sxx ( float x[], unsigned char sign[], int n );

    int ifnc_noise_actual ( float x34[], float x[],
                            int gsf, int n, int log_of_n );
    int ifnc_ixnoise_actual ( int ix[], float x[], int sf, int n, int logn );

    int vect_quant ( float x34[], int ix[], int gsf, int n );
    int vect_quantB ( float x34[], int ix[], int gsf, int n );
    int vect_quantB2 ( float x34[], int ix[], int gsf, int n, float qadjust );
    int vect_quantB10x ( float x34[], int ix[], int gsf, int n );
    void vect_ixmax_quantB ( float x34max[], int ixmax[], int gsf[], int n );
    void vect_ix10xmax_quantB ( float x34max[], int ixmax[], int gsf[],
                                int n );
    int vect_quantX ( float x34[], int ix[], float igain34, int n );
    int vect_quant_clip1 ( float x34[], int ix[], int gsf, int n );
    void vect_limits ( int x[], int upper[], int lower[], int n );
    void shave_01pairs ( float x34[], int ix[], int g, int n, float thres );

    void fnc_ms_process ( float *xr, int n, unsigned char *ixsign );
    void fnc_ms_process2 ( float *xr, int n, unsigned char *ixsign );   // no sqrt(2) scale
    void fnc_ms_sparse ( float *xr, int n, float thres );
    void fnc_ms_sparse_sum ( float *xr, int n, float thres );
    void fnc_sxx ( float *xr, int n, float *sxx );
    int vect_imax ( int x[], int n );

    INTPAIR ifnc_noise_actual_ms ( float *x34,
                                   float *x, int *sign_mask,
                                   int gsf0, int gsf1, int n, int logn_mb );

    int ifnc_inverse_gsf_snr2 ( int *qx, float *y, int n );
    int ifnc_inverse_gsf_xfer2 ( int *qx, float *y, int n );

    void L3init_gen_band_table_long ( int *nBand_l );
    void L3init_gen_band_table_short ( int *nBand_s );
    int L3init_sfbl_limit2 ( int band_limit );
    int L3init_sfbs_limit2 ( int band_limit );

/*--------
INTPAIR subdivide2_quadregion(int ixmax[], int ix[], int ncb, int ch);
// returns (nbig,nquads)
void output_subdivide2(GR *gr_data, int ch);
int subdivide2(int ixmax[], int ix[], int ncb, int opti_flag, int ch);
----------*/

// bitallo1
    int L3init_sfbs_limit (  );

//----------------------------------------------  
#ifdef __cplusplus
}
#endif

#endif   // _BALOW_H_
