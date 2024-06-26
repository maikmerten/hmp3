/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: 2024-03-14, Case
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

#include "bitallo.h"

///////////////////////////////////////////////////////////////////////////////
// Public Functions
///////////////////////////////////////////////////////////////////////////////
CBitAllo::CBitAllo (  ):
block_type ( 0 )
{
}

/*CBitAllo::~CBitAllo (  )
{
}*/

//======================================================
int
CBitAllo::ms_correlation ( float x[2][576], int n )
{
    int i;
    float a, b;
    float ss, sd;
    int r;

    sd = 0.0f;
    ss = 1.0f;
    for ( i = 0; i < n; i++ )
    {
        a = x[0][i] * x[0][i];
        b = x[1][i] * x[1][i];
        ss += ( a + b );
        a = a - b;
        if ( a < 0.0f )
            a = -a;
        sd += a;
    }

/*-------
sd = 0.0f;
ss = 100.0f;
for(i=0;i<n;i++) {
    a = x[0][i] - x[1][i];
    b = x[0][i] + x[1][i];
    sd += a*a;
    ss += b*b;
}
--------*/

    r = ( int ) ( 100.0f * sd / ss );

    return r;
}

/*===============================================================*/
