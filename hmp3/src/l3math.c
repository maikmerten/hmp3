/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: 2022/11/24, Maik Merten
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

extern int iframe; // for testing

#define g_offset 8

//extern float look_f00_pmax[512];      // global  in mb
//extern float look_pigain[128];        // global
extern float look_gain[128];    // global
extern float look_34igain[128];
extern float look_ix43[256];

//------------------------------------------------
// LogSubberTable  size = 84
static const int LogSubberTable[84] = {
    7, 22, 36, 49,
    61, 73, 83, 93,
    103, 112, 120, 128,
    136, 143, 150, 156,
    163, 168, 174, 179,
    184, 189, 194, 198,
    202, 206, 210, 214,
    217, 220, 223, 227,
    229, 232, 235, 237,
    240, 242, 244, 247,
    249, 251, 253, 254,
    256, 258, 259, 261,
    263, 264, 265, 267,
    268, 269, 270, 271,
    273, 274, 275, 276,
    277, 277, 278, 279,
    280, 281, 281, 282,
    283, 283, 284, 285,
    285, 286, 286, 287,
    287, 288, 288, 289,
    289, 290, 290, 290,
};

/* ATTENTION! quant_table[0] is modified */
static float quant_table[32] = {
    0.09460f - 0.4375f, //    0
    0.02799f - 0.4375f, //    1
    0.01671f - 0.4375f, //    2
    0.01192f - 0.4375f, //    3
    0.00927f - 0.4375f, //    4
    0.00758f - 0.4375f, //    5
    0.00641f - 0.4375f, //    6
    0.00556f - 0.4375f, //    7
    0.00490f - 0.4375f, //    8
    0.00439f - 0.4375f, //    9
    0.00397f - 0.4375f, //   10
    0.00362f - 0.4375f, //   11
    0.00333f - 0.4375f, //   12
    0.00309f - 0.4375f, //   13
    0.00287f - 0.4375f, //   14
    0.00269f - 0.4375f, //   15
    0.00253f - 0.4375f, //   16
    0.00238f - 0.4375f, //   17
    0.00225f - 0.4375f, //   18
    0.00214f - 0.4375f, //   19
    0.00203f - 0.4375f, //   20
    0.00194f - 0.4375f, //   21
    0.00185f - 0.4375f, //   22
    0.00177f - 0.4375f, //   23
    0.00170f - 0.4375f, //   24
    0.00163f - 0.4375f, //   25
    0.00157f - 0.4375f, //   26
    0.00152f - 0.4375f, //   27
    0.00146f - 0.4375f, //   28
    0.00141f - 0.4375f, //   29
    0.00136f - 0.4375f, //   30
    0.00132f - 0.4375f, //   31
};

static const float con707 = 0.70710678f;

static const float map_table[] = {
    1.000,
    0.875,
    0.750,
    0.625,
    0.500,
    0.375,
    0.250,
    0.125,
};

/*===============================================================*/
float *
align16 ( float *x )
{
    float *y;

    y = ( float * ) ( ( ( ( int ) x ) + 15 ) & ( ~15 ) );

    return y;
}

/*===============================================================*/
float
dbLog ( float x )
{
    return ( float ) ( 10.0 * log10 ( x ) );
}

/*---------------------------------------------------------------*/
float
dbLogB ( float x )
{
    if ( x <= 1.18e-38f )
        return -379.28f;
    return ( float ) ( 10.0 * log10 ( x ) );
}

/*---------------------------------------------------------------*/
int
mbLog ( float x )
{
    int t;

    if ( x <= 1.18e-38f )
        return -37928;
    t = ( int ) ( 1000.0 * log10 ( x ) + 0.5 );
    return t;
}

/*---------------------------------------------------------------*/
int
mbLogB ( float x )
{
    int t;

    if ( x <= 1.18e-38f )
        return -37928;
    t = ( int ) ( 1000.0 * log10 ( x ) + 0.5 );
    return t;
}

/*---------------------------------------------------------------*/
static const signed int logMbC_table[] = {
0xFFFF6AAE, 0xFFFF6AB0, 0xFFFF6AB1, 0xFFFF6AB3, 0xFFFF6AB5, 0xFFFF6AB6,
0xFFFF6AB8, 0xFFFF6ABA, 0xFFFF6ABB, 0xFFFF6ABD, 0xFFFF6ABE, 0xFFFF6AC0,
0xFFFF6AC2, 0xFFFF6AC3, 0xFFFF6AC5, 0xFFFF6AC7, 0xFFFF6AC8, 0xFFFF6ACA,
0xFFFF6ACB, 0xFFFF6ACD, 0xFFFF6ACE, 0xFFFF6AD0, 0xFFFF6AD2, 0xFFFF6AD3,
0xFFFF6AD5, 0xFFFF6AD6, 0xFFFF6AD8, 0xFFFF6AD9, 0xFFFF6ADB, 0xFFFF6ADC,
0xFFFF6ADE, 0xFFFF6ADF, 0xFFFF6AE1, 0xFFFF6AE2, 0xFFFF6AE4, 0xFFFF6AE5,
0xFFFF6AE7, 0xFFFF6AE8, 0xFFFF6AEA, 0xFFFF6AEB, 0xFFFF6AED, 0xFFFF6AEE,
0xFFFF6AF0, 0xFFFF6AF1, 0xFFFF6AF3, 0xFFFF6AF4, 0xFFFF6AF5, 0xFFFF6AF7,
0xFFFF6AF8, 0xFFFF6AFA, 0xFFFF6AFB, 0xFFFF6AFD, 0xFFFF6AFE, 0xFFFF6AFF,
0xFFFF6B01, 0xFFFF6B02, 0xFFFF6B04, 0xFFFF6B05, 0xFFFF6B06, 0xFFFF6B08,
0xFFFF6B09, 0xFFFF6B0B, 0xFFFF6B0C, 0xFFFF6B0D, 0xFFFF6B0F, 0xFFFF6B10,
0xFFFF6B11, 0xFFFF6B13, 0xFFFF6B14, 0xFFFF6B15, 0xFFFF6B17, 0xFFFF6B18,
0xFFFF6B19, 0xFFFF6B1B, 0xFFFF6B1C, 0xFFFF6B1D, 0xFFFF6B1F, 0xFFFF6B20,
0xFFFF6B21, 0xFFFF6B22, 0xFFFF6B24, 0xFFFF6B25, 0xFFFF6B26, 0xFFFF6B28,
0xFFFF6B29, 0xFFFF6B2A, 0xFFFF6B2B, 0xFFFF6B2D, 0xFFFF6B2E, 0xFFFF6B2F,
0xFFFF6B30, 0xFFFF6B32, 0xFFFF6B33, 0xFFFF6B34, 0xFFFF6B35, 0xFFFF6B37,
0xFFFF6B38, 0xFFFF6B39, 0xFFFF6B3A, 0xFFFF6B3C, 0xFFFF6B3D, 0xFFFF6B3E,
0xFFFF6B3F, 0xFFFF6B40, 0xFFFF6B42, 0xFFFF6B43, 0xFFFF6B44, 0xFFFF6B45,
0xFFFF6B46, 0xFFFF6B48, 0xFFFF6B49, 0xFFFF6B4A, 0xFFFF6B4B, 0xFFFF6B4C,
0xFFFF6B4E, 0xFFFF6B4F, 0xFFFF6B50, 0xFFFF6B51, 0xFFFF6B52, 0xFFFF6B53,
0xFFFF6B55, 0xFFFF6B56, 0xFFFF6B57, 0xFFFF6B58, 0xFFFF6B59, 0xFFFF6B5A,
0xFFFF6B5B, 0xFFFF6B5D, 0xFFFF6B5E, 0xFFFF6B5F, 0xFFFF6B60, 0xFFFF6B61,
0xFFFF6B62, 0xFFFF6B63, 0xFFFF6B64, 0xFFFF6B65, 0xFFFF6B67, 0xFFFF6B68,
0xFFFF6B69, 0xFFFF6B6A, 0xFFFF6B6B, 0xFFFF6B6C, 0xFFFF6B6D, 0xFFFF6B6E,
0xFFFF6B6F, 0xFFFF6B70, 0xFFFF6B72, 0xFFFF6B73, 0xFFFF6B74, 0xFFFF6B75,
0xFFFF6B76, 0xFFFF6B77, 0xFFFF6B78, 0xFFFF6B79, 0xFFFF6B7A, 0xFFFF6B7B,
0xFFFF6B7C, 0xFFFF6B7D, 0xFFFF6B7E, 0xFFFF6B7F, 0xFFFF6B80, 0xFFFF6B81,
0xFFFF6B82, 0xFFFF6B83, 0xFFFF6B85, 0xFFFF6B86, 0xFFFF6B87, 0xFFFF6B88,
0xFFFF6B89, 0xFFFF6B8A, 0xFFFF6B8B, 0xFFFF6B8C, 0xFFFF6B8D, 0xFFFF6B8E,
0xFFFF6B8F, 0xFFFF6B90, 0xFFFF6B91, 0xFFFF6B92, 0xFFFF6B93, 0xFFFF6B94,
0xFFFF6B95, 0xFFFF6B96, 0xFFFF6B97, 0xFFFF6B98, 0xFFFF6B99, 0xFFFF6B9A,
0xFFFF6B9B, 0xFFFF6B9C, 0xFFFF6B9D, 0xFFFF6B9E, 0xFFFF6B9F, 0xFFFF6BA0,
0xFFFF6BA1, 0xFFFF6BA1, 0xFFFF6BA2, 0xFFFF6BA3, 0xFFFF6BA4, 0xFFFF6BA5,
0xFFFF6BA6, 0xFFFF6BA7, 0xFFFF6BA8, 0xFFFF6BA9, 0xFFFF6BAA, 0xFFFF6BAB,
0xFFFF6BAC, 0xFFFF6BAD, 0xFFFF6BAE, 0xFFFF6BAF, 0xFFFF6BB0, 0xFFFF6BB1,
0xFFFF6BB2, 0xFFFF6BB3, 0xFFFF6BB3, 0xFFFF6BB4, 0xFFFF6BB5, 0xFFFF6BB6,
0xFFFF6BB7, 0xFFFF6BB8, 0xFFFF6BB9, 0xFFFF6BBA, 0xFFFF6BBB, 0xFFFF6BBC,
0xFFFF6BBD, 0xFFFF6BBE, 0xFFFF6BBE, 0xFFFF6BBF, 0xFFFF6BC0, 0xFFFF6BC1,
0xFFFF6BC2, 0xFFFF6BC3, 0xFFFF6BC4, 0xFFFF6BC5, 0xFFFF6BC6, 0xFFFF6BC7,
0xFFFF6BC7, 0xFFFF6BC8, 0xFFFF6BC9, 0xFFFF6BCA, 0xFFFF6BCB, 0xFFFF6BCC,
0xFFFF6BCD, 0xFFFF6BCE, 0xFFFF6BCE, 0xFFFF6BCF, 0xFFFF6BD0, 0xFFFF6BD1,
0xFFFF6BD2, 0xFFFF6BD3, 0xFFFF6BD4, 0xFFFF6BD4, 0xFFFF6BD5, 0xFFFF6BD6,
0xFFFF6BD7, 0xFFFF6BD8, 0xFFFF6BD9, 0xFFFF6BDA
} ;

int
mbLogC ( float x )
{
#ifndef IEEE_FLOAT
    int t;

    if ( x <= 1.18e-38f )
        return -37928;
    t = ( int ) ( 1000.0 * log10 ( x ) + 0.5 );
    return t;
#else
    union {float f; unsigned int i;} u = {x} ;
    unsigned int exponent = u.i >> 23 ;
    unsigned int mant = (u.i >> (23-8)) & 255 ;
    return logMbC_table[mant] + 301*exponent ;
#endif
}

/*---------------------------------------------------------------*/

#ifdef IEEE_FLOAT
static const union {unsigned int i; float f;} look_mbExpA[] = {
0x03F800000, 0x03FE6C949, 0x040500E4E, 0x040BB9070, 0x04129173F, 0x041986FD6,
0x042096C55, 0x04277C6C4, 0x042DF5F66, 0x043495F57, 0x043B589FE, 0x04423A8B7,
0x044938A43, 0x04505023A, 0x0456FD129, 0x045D8327A, 0x04642E757, 0x046AFB518,
0x0471E66DB, 0x0478ECCF4, 0x04800BC6B, 0x048681D02, 0x048D14091, 0x0493CA489,
0x049AA1026, 0x04A19503A, 0x04A8A36A0, 0x04AF9337F, 0x04B60A835, 0x04BCA87C4,
0x04C369538, 0x04CA499A0, 0x04D146371, 0x04D85C604, 0x04DF1322D, 0x04E5970B9,
0x04EC4063E, 0x04F30B7BD, 0x04F9F5006, 0x0500F9F29, 0x0508179EC, 0x050E972AF,
0x051527497, 0x051BDBA39, 0x0522B0A7C, 0x0529A31E8, 0x0530B0214, 0x0537AA254,
0x053E1F2E8, 0x0544BB1E5, 0x054B7A1FC, 0x055258BEB, 0x055953DDF, 0x056068AEF,
0x056729538, 0x056DAB0CD, 0x0574526CB, 0x057B1BBDE, 0x058203A89, 0x058907293,
0x059023883, 0x0596ACA53, 0x059D3AA62, 0x05A3ED181, 0x05AAC0643, 0x05B1B14E2,
0x05B8BCEB3, 0x05BFC1344, 0x05C633F82, 0x05CCCDDBC, 0x05D38B04C, 0x05DA67F9B,
0x05E16198F, 0x05E8750FC, 0x05EF3FA4E, 0x05F5BF2B8, 0x05FC64901, 0x06032C17F,
0x060A12665, 0x061114734, 0x06182F833, 0x061EC23F0, 0x06254E1F5, 0x062BFEA65,
0x0632D037C, 0x0639BF92B, 0x0640C9C7F, 0x0647D8653, 0x064E48E05, 0x0654E0B4E,
0x065B9C029, 0x0662774B2, 0x06696F682, 0x06708182C, 0x067756172, 0x067DD367C,
0x068476CE2, 0x068B3C8A1, 0x06922139C, 0x069921D0E, 0x06A03B8FD, 0x06A6D7F8B,
0x06AD61B53, 0x06B4104E6, 0x06BAE022A, 0x06C1CDEC3, 0x06C8D6B7B, 0x06CFEFB85,
0x06D65DE75, 0x06DCF3A9B, 0x06E3AD197, 0x06EA86B32, 0x06F17D4BB, 0x06F88E081,
0x06FF6CAA7, 0x0705E7C1D, 0x070C89271, 0x07134D146, 0x071A30231, 0x07212F423,
0x072847AE3, 0x072EEDD25, 0x07357567F, 0x073C22106, 0x0742F024F, 0x0749DC5AE,
0x0750E3BA7, 0x07580396E, 0x0090DBC5C, 0x0097F8D58, 0x009E661EC, 0x00A4FB11F,
0x00ABB3C6F, 0x00B28CB84, 0x00B982B90, 0x00C092EC9, 0x00C7757CB, 0x00CDEFB5B,
0x00D490527, 0x00DB538AF, 0x00E235F6B, 0x00E93482E, 0x00F04C6A7, 0x00F6F65C0,
0x00FD7D1A6, 0x01042900C, 0x010AF6666, 0x0111E1FE9, 0x0118E8CFF, 0x0120082C3,
0x01267B50D, 0x012D0E2D9, 0x0133C500C, 0x013A9C3FC, 0x014190B8F, 0x01489F8B9,
0x014F8C3E3, 0x015604397, 0x015CA2D0F, 0x016364372, 0x016A44FE7, 0x0171420FC,
0x017858A1A, 0x017F0C626, 0x018590F57, 0x018C3AE73, 0x019306897, 0x0199F08AC,
0x01A0F5ED6, 0x01A813FEE, 0x01AE90A21, 0x01B521655, 0x01BBD6540, 0x01C2ABDE2,
0x01C99ECD9, 0x01D0AC3D1, 0x01D7A3213, 0x01DE18DB6, 0x01E4B56AA, 0x01EB74FBE,
0x01F2541C6, 0x01F94FB08, 0x020064EAD, 0x020722893, 0x020DA4EDB, 0x02144CE7F,
0x021B16C44, 0x0221FF2C6, 0x0229031E1, 0x02301FE30, 0x0236A612A, 0x023D34B95,
0x0243E7C0B, 0x024ABB937, 0x0251ACF6D, 0x0258B9015, 0x025FBA25E, 0x02662D9BB,
0x026CC81FB, 0x027385D94, 0x027A63509, 0x02815D655, 0x028871461, 0x028F38D09,
0x0295B9035, 0x029C5F033, 0x02A327170, 0x02AA0DE38, 0x02B110623, 0x02B82BD8B,
0x02BEBBA2D, 0x02C54829D, 0x02CBF9470, 0x02D2CB5FF, 0x02D9BB34F, 0x02E0C5D86,
0x02E7D14C7, 0x02EE427A8, 0x02F4DAF05, 0x02FB96CF8, 0x0302729B2, 0x03096B2E6,
0x03107DB38, 0x03174F38C, 0x031DCD369, 0x032471391, 0x032B3781C, 0x03321CB05,
0x03391DB9D, 0x034037DFE, 0x0346D152C, 0x034D5BB6F, 0x03540AE73, 0x035ADB43B,
0x0361C9881, 0x0368D2C24, 0x036FE8951, 0x037657781, 0x037CEDDCB, 0x0383A7DEB,
0x038A81FC3, 0x0391790BA, 0x03988A333, 0x039F65C1F, 0x03A5E1878, 0x03AC8389D,
0x03B34804A, 0x03BA2B92F, 0x03C12B252, 0x03C843F8E, 0x03CEE722A, 0x03D56F60D,
0x03DC1CA14, 0x03E2EB3ED, 0x03E9D7F04, 0x03F0DFBF3
} ;

static const union {unsigned int i; float f;} look_mbExpB[] = {
0x03F800000, 0x03F804B8A, 0x03F809740, 0x03F80E323, 0x03F812F33, 0x03F817B70,
0x03F81C7D9, 0x03F821470, 0x03F826134, 0x03F82AE25, 0x03F82FB44, 0x03F834890,
0x03F83960A, 0x03F83E3B1, 0x03F843187, 0x03F847F8A, 0x03F84CDBB, 0x03F851C1B,
0x03F856AA8, 0x03F85B964, 0x03F86084F, 0x03F865768, 0x03F86A6B0, 0x03F86F626,
0x03F8745CC, 0x03F8795A0, 0x03F87E5A4, 0x03F8835D7, 0x03F888639, 0x03F88D6CA,
0x03F89278B, 0x03F89787C, 0x03F89C99D, 0x03F8A1AED, 0x03F8A6C6D, 0x03F8ABE1E,
0x03F8B0FFF, 0x03F8B6210, 0x03F8BB451, 0x03F8C06C3, 0x03F8C5966, 0x03F8CAC39,
0x03F8CFF3D, 0x03F8D5273, 0x03F8DA5D9, 0x03F8DF971, 0x03F8E4D3A, 0x03F8EA134,
0x03F8EF560, 0x03F8F49BE, 0x03F8F9E4D, 0x03F8FF30E, 0x03F904802, 0x03F909D27,
0x03F90F27F, 0x03F914809, 0x03F919DC5, 0x03F91F3B4, 0x03F9249D6, 0x03F92A02B,
0x03F92F6B3, 0x03F934D6D, 0x03F93A45B, 0x03F93FB7C, 0x03F9452D1, 0x03F94AA59,
0x03F950215, 0x03F955A04, 0x03F95B228, 0x03F960A7F, 0x03F96630B, 0x03F96BBCB,
0x03F9714BF, 0x03F976DE8, 0x03F97C745, 0x03F9820D7, 0x03F987A9E, 0x03F98D49A,
0x03F992ECB, 0x03F998931, 0x03F99E3CD, 0x03F9A3E9E, 0x03F9A99A5, 0x03F9AF4E1,
0x03F9B5053, 0x03F9BABFC, 0x03F9C07DA, 0x03F9C63EE, 0x03F9CC039, 0x03F9D1CBB,
0x03F9D7972, 0x03F9DD661, 0x03F9E3387, 0x03F9E90E3, 0x03F9EEE76, 0x03F9F4C41,
0x03F9FAA43, 0x03FA0087D, 0x03FA066EE, 0x03FA0C597, 0x03FA12478, 0x03FA18391,
0x03FA1E2E1, 0x03FA2426B, 0x03FA2A22C, 0x03FA30226, 0x03FA36259, 0x03FA3C2C4,
0x03FA42369, 0x03FA48446, 0x03FA4E55C, 0x03FA546AC, 0x03FA5A836, 0x03FA609F8,
0x03FA66BF5, 0x03FA6CE2B, 0x03FA7309C, 0x03FA79346, 0x03FA7F62B, 0x03FA8594A,
0x03FA8BCA3, 0x03FA92037, 0x03FA98406, 0x03FA9E810, 0x03FAA4C55, 0x03FAAB0D5,
0x03FAB1590, 0x03FAB7A87, 0x03FABDFB9, 0x03FAC4527, 0x03FACAAD1, 0x03FAD10B7,
0x03FAD76D9, 0x03FADDD37, 0x03FAE43D2, 0x03FAEAAA9, 0x03FAF11BD, 0x03FAF790E,
0x03FAFE09C, 0x03FB04866, 0x03FB0B06E, 0x03FB118B4, 0x03FB18137, 0x03FB1E9F8,
0x03FB252F6, 0x03FB2BC33, 0x03FB325AD, 0x03FB38F66, 0x03FB3F95E, 0x03FB46393,
0x03FB4CE08, 0x03FB538BB, 0x03FB5A3AD, 0x03FB60EDF, 0x03FB67A4F, 0x03FB6E5FF,
0x03FB751EF, 0x03FB7BE1E, 0x03FB82A8D, 0x03FB8973C, 0x03FB9042C, 0x03FB9715B,
0x03FB9DECB, 0x03FBA4C7C, 0x03FBABA6D, 0x03FBB289F, 0x03FBB9712, 0x03FBC05C7,
0x03FBC74BC, 0x03FBCE3F4, 0x03FBD536C, 0x03FBDC327, 0x03FBE3324, 0x03FBEA362,
0x03FBF13E3, 0x03FBF84A6, 0x03FBFF5AC, 0x03FC066F5, 0x03FC0D880, 0x03FC14A4E,
0x03FC1BC60, 0x03FC22EB5, 0x03FC2A14D, 0x03FC31429, 0x03FC38749, 0x03FC3FAAD,
0x03FC46E54, 0x03FC4E240, 0x03FC55671, 0x03FC5CAE6, 0x03FC63F9F, 0x03FC6B49E,
0x03FC729E2, 0x03FC79F6B, 0x03FC81539, 0x03FC88B4D, 0x03FC901A6, 0x03FC97845,
0x03FC9EF2B, 0x03FCA6656, 0x03FCADDC8, 0x03FCB5580, 0x03FCBCD7F, 0x03FCC45C5,
0x03FCCBE51, 0x03FCD3725, 0x03FCDB040, 0x03FCE29A3, 0x03FCEA34D, 0x03FCF1D3F,
0x03FCF9779, 0x03FD011FB, 0x03FD08CC6, 0x03FD107D9, 0x03FD18334, 0x03FD1FED8,
0x03FD27AC6, 0x03FD2F6FC, 0x03FD3737C, 0x03FD3F045, 0x03FD46D58, 0x03FD4EAB5,
0x03FD5685B, 0x03FD5E64C, 0x03FD66487, 0x03FD6E30D, 0x03FD761DD, 0x03FD7E0F8,
0x03FD8605E, 0x03FD8E010, 0x03FD9600C, 0x03FD9E054, 0x03FDA60E8, 0x03FDAE1C8,
0x03FDB62F4, 0x03FDBE46C, 0x03FDC6631, 0x03FDCE842, 0x03FDD6A9F, 0x03FDDED4A,
0x03FDE7042, 0x03FDEF387, 0x03FDF771A, 0x03FDFFAFA, 0x03FE07F28, 0x03FE103A4,
0x03FE1886E, 0x03FE20D87, 0x03FE292EE, 0x03FE318A4, 0x03FE39EA9, 0x03FE424FD,
0x03FE4ABA0, 0x03FE53293, 0x03FE5B9D5, 0x03FE64167
} ;
#endif

float
mbExp ( int x )
{
#ifndef IEEE_FLOAT
// return anti-log of x, x in milli-bells

    return ( float ) exp ( 0.00230258509299 * x );
#else
    unsigned short u1 = (unsigned)x & 0xff ;
    unsigned short u2 = ((unsigned)x & 0xff00) >> 8 ;
    float t = look_mbExpB[u1].f * look_mbExpA[u2].f ;
    
    if (x > 32000)       return 1.0E32f ;
    else if (x < -32000) return 1.0E-32f ;
    else return t ;
#endif
}

/*---------------------------------------------------------------*/
int
round_to_int ( float x )
{
    return (int)(x + copysignf(0.5f, x));
}

/*---------------------------------------------------------------*/
int
LogSubber ( int mbNoise, int mbNoise2 )
{
    int a, k;

// mbNoise in milli-bells
// mbNoise > mbNoise2
//  computes mbLog(2*antilog(mbNoise) - antilog(mbNoise2))

    k = ( mbNoise - mbNoise2 ) >> 4;
    if ( k > 83 )
        k = 83;
    a = mbNoise + LogSubberTable[k];

    return a;
}

/*---------------------------------------------------------------*/
float
pos_fmax ( float x, float y )
{
// return HX_MAX(x,y), will only be used non-negative x, y

    if ( x > y )
        return x;
    else
        return y;

}

/*---------------------------------------------------------------*/
void
vect_pow3414 ( float xr[], float x34[], float x14[], int n )
{
    int i;
    float t;

    for ( i = 0; i < n; i++ )
    {
        t = ( float ) sqrt ( sqrt ( fabs ( xr[i] ) ) );
        x14[i] = t;
        x34[i] = t * t * t;
    }

}

/*---------------------------------------------------------------*/
float
vect_fmax1 ( float x[], int n )
{
    int i;
    float xmax;

    xmax = 0.0f;
    for ( i = 0; i < n; i++ )
    {
        if ( x[i] > xmax )
            xmax = x[i];
    }

    return xmax;
}

/*---------------------------------------------------------------*/
void
vect_fmax2 ( float x[], int n, float *yout )
{
    int i;
    float xmax;

    xmax = 0.0f;
    for ( i = 0; i < n; i++ )
    {
        if ( x[i] > xmax )
            xmax = x[i];
    }

    *yout = xmax;
}

/*---------------------------------------------------------------*/
float
vect_sign_sxx ( float x[], char sign[], int n )
{
    int i;
    float sxx;

    sxx = 0.0f;
    for ( i = 0; i < n; i++ )
    {
        if ( x[i] >= 0.0f )
        {
            sign[i] = 0;
        }
        else
        {
            sign[i] = 1;
            x[i] = -x[i];
        }
        sxx += x[i] * x[i];
    }

    return sxx;
}

/*---------------------------------------------------------------*/

/*xxxxxxxx
int ixfnc_noise0P(float x14max, float xgsf)
{
int gsf, p, noise;
float igain, sum;

//  integer noise in milli-bells
;;
gsf = (int)(xgsf + 0.5f);
igain = look_pigain[gsf];
p   = (int)(0.5f + x14max*igain);
sum = look_f00_pmax[p];    // in mb
noise = (int)(sum + 110.0*(xgsf-gsf) + 150.515f*(gsf-g_offset));  
// asm version rounds final result to nearest (may be negative)

return noise;
}
xxxxxxxxxxxxx*/

/*---------------------------------------------------------------*/
#ifdef _M_IX86	/* Asm versions */
/*__inline long
RoundFtoL(float f) {
	long l;
	__asm fld	f
	__asm fistp	l
	return l;
}*/
#else /* C versions */
/*long // FIXME: removed __inline to fix linker bug
RoundFtoL(float f) {
	long l = (long)(f < 0.0f ? f - 0.5f : f + 0.5f);
	return l;
}*/
#endif /* _M_IX86 */


int
ifnc_noise_actual ( const float x34[], const float x[], int gsf, int n, int log_of_n )
{
    int i, qx, noise;
    float igain, gain, sxx, xhat, tmp;

    sxx = 0.0f;
    igain = look_34igain[gsf];
    gain = look_gain[gsf];

    for ( i = 0; i < n; i++ )
    {
//        qx = ( int ) ( igain * x34[i] + ( 0.5f - 0.0946f ) );
//        qx = RoundFtoL ( igain * x34[i] + ( 0.0f - 0.0946f ) ) ;
        tmp = ( igain * x34[i] + ( 0.0f - 0.0946f ) ) ;
        qx = (int)(tmp + copysignf(0.5f, tmp));
        if ( qx < 256 )
        {
            xhat = gain * look_ix43[qx];
        }
        else
        {
            xhat = ( float ) ( gain * pow ( qx, ( 4.0 / 3.0 ) ) );
        }
        tmp = x[i] - xhat;
        sxx += tmp * tmp;
    }

//    noise = ( int ) ( 1000.0 * log10 ( 1.0e-12f + sxx ) + 0.5 );
//    noise = noise - log_of_n;
    noise = mbLogC(1.0e-12f + sxx) - log_of_n ;

    return noise;
}

/*---------------------------------------------------------------*/

// unused, currently

int
ifnc_noise_actual2 ( float x34[], float x[],
                     int gsf, int n, int log_of_n, int sf )
{
    int i, qx, noise;
    float igain, gain, sxx, xhat, tmp;

// gsf = forward quant, sf = dequant

    sxx = 0.0f;
    igain = look_34igain[gsf];
    gain = look_gain[sf];

    for ( i = 0; i < n; i++ )
    {
        qx = ( int ) ( igain * x34[i] + ( 0.5f - 0.0946f ) );
        if ( qx < 256 )
        {
            xhat = gain * look_ix43[qx];
        }
        else
        {
            xhat = ( float ) ( gain * pow ( qx, ( 4.0 / 3.0 ) ) );
        }
        tmp = x[i] - xhat;
        sxx += tmp * tmp;
    }

    noise = ( int ) ( 1000.0 * log10 ( 1.0e-12f + sxx ) + 0.5 );

    noise = noise - log_of_n;

    return noise;
}

/*---------------------------------------------------------------*/
int
ifnc_noise_actualX2 ( float x34[], float x[],
                      float igain, int n, int log_of_n, int sf )
{
    int i, qx, noise;
    float gain, sxx, xhat, tmp;

// igain = forward quant, sf = dequant

    sxx = 0.0f;
//igain = look_34igain[gsf];
    gain = look_gain[sf];

    for ( i = 0; i < n; i++ )
    {
        qx = ( int ) ( igain * x34[i] + ( 0.5f - 0.0946f ) );
        if ( qx < 256 )
        {
            xhat = gain * look_ix43[qx];
        }
        else
        {
            xhat = ( float ) ( gain * pow ( qx, ( 4.0 / 3.0 ) ) );
        }
        tmp = x[i] - xhat;
        sxx += tmp * tmp;
    }

    noise = ( int ) ( 1000.0 * log10 ( 1.0e-12f + sxx ) + 0.5 );

    noise = noise - log_of_n;

    return noise;
}

/*---------------------------------------------------------------*/
int
ifnc_ixnoise_actual ( int ix[], float x[], int sf, int n, int logn )
{
    int i, noise;
    float gain, sxx, xhat, tmp;

// ix = quantized value,  sf = dequant

    sxx = 0.0f;
    gain = look_gain[sf];

    for ( i = 0; i < n; i++ )
    {
        if ( ix[i] < 256 )
        {
            xhat = gain * look_ix43[ix[i]];
        }
        else
        {
            xhat = ( float ) ( gain * pow ( ix[i], ( 4.0 / 3.0 ) ) );
        }
        tmp = x[i] - xhat;
        sxx += tmp * tmp;
    }

    noise = ( int ) ( 1000.0 * log10 ( 1.0e-12f + sxx ) + 0.5 );

    noise = noise - logn;

    return noise;
}

/*---------------------------------------------------------------*/
int
vect_quant ( float x34[], int ix[], int gsf, int n )
{
    int i, ixmax;
    float igain;

    igain = look_34igain[gsf];
    ixmax = 0;
    for ( i = 0; i < n; i++ )
    {
        ix[i] = ( int ) ( igain * x34[i] + ( 0.5f - 0.0946f ) );
        if ( ix[i] > ixmax )
            ixmax = ix[i];
    }

    return ixmax;
}

/*---------------------------------------------------------------*/
int
vect_quantB ( float x34[], int ix[], int gsf, int n )
{
    int i, iq, ixmax;
    float igain, t;

    igain = look_34igain[gsf];
    ixmax = 0;
    for ( i = 0; i < n; i++ )
    {
        t = igain * x34[i] + ( 0.5f - 0.4375f );
        iq = ( int ) t;
        if ( iq > 31 )
            iq = 31;
        ix[i] = ( int ) ( t - quant_table[iq] );
        if ( ix[i] > ixmax )
            ixmax = ix[i];
    }

    return ixmax;
}

/*---------------------------------------------------------------*/
int
vect_quantB2 ( float x34[], int ix[], int gsf, int n, float qadjust )
{
    int i, iq, ixmax;
    float igain, t;
    float save;

    save = quant_table[0];
    quant_table[0] = qadjust;

    igain = look_34igain[gsf];
    ixmax = 0;
    for ( i = 0; i < n; i++ )
    {
        t = igain * x34[i] + ( 0.5f - 0.4375f );
        iq = ( int ) t;
        if ( iq > 31 )
            iq = 31;
        ix[i] = ( int ) ( t - quant_table[iq] );
        if ( ix[i] > ixmax )
            ixmax = ix[i];
    }

    quant_table[0] = save;

    return ixmax;
}

/*---------------------------------------------------------------*/
int
vect_quantB10x ( float x34[], int ix[], int gsf, int n )
{
    int i, iq, ixmax;
    float igain, t;

// quant at 10x actual - 
// asm ix result can be negative (rounding)
// in this routine ix non-negative

    igain = look_34igain[gsf];
    ixmax = 0;
    for ( i = 0; i < n; i++ )
    {
        t = igain * x34[i] + ( 0.5f - 0.4375f );
        iq = ( int ) t;
        if ( iq > 31 )
            iq = 31;
        ix[i] = ( int ) ( 10.0f * ( t - quant_table[iq] ) + ( 0.5f - 5.0f ) );
        if ( ix[i] > ixmax )
            ixmax = ix[i];
    }

    return ixmax;
}

/*---------------------------------------------------------------*/
void
vect_ixmax_quantB ( float x34max[], int ixmax[], int gsf[], int n )
{
    int i, iq;
    float t;

    for ( i = 0; i < n; i++ )
    {
        t = look_34igain[gsf[i]] * x34max[i] + ( 0.5f - 0.4375f );
        iq = ( int ) t;
        if ( iq > 31 )
            iq = 31;
        ixmax[i] = ( int ) ( t - quant_table[iq] );
    }

}

/*---------------------------------------------------------------*/
void
vect_ix10xmax_quantB ( float x34max[], int ixmax[], int gsf[], int n )
{
    int i, iq;
    float t;

// quant at 10x actual - 
// asm  result can be negative (rounding)
// in this routine non-negative

    for ( i = 0; i < n; i++ )
    {
        t = look_34igain[gsf[i]] * x34max[i] + ( 0.5f - 0.4375f );
        iq = ( int ) t;
        if ( iq > 31 )
            iq = 31;
        ixmax[i] =
            ( int ) ( 10.0f * ( t - quant_table[iq] ) + ( 0.5f - 5.0f ) );
    }

}

/*---------------------------------------------------------------*/
int
vect_quantX ( float x34[], int ix[], float igain, int n )
{

    int i, ixmax;

//igain = look_34igain[gsf];
    ixmax = 0;
    for ( i = 0; i < n; i++ )
    {
        ix[i] = ( int ) ( igain * x34[i] + ( 0.5f - 0.0946f ) );
        if ( ix[i] > ixmax )
            ixmax = ix[i];
    }

    return ixmax;
}

/*---------------------------------------------------------------*/
int
vect_quant_clip1 ( float x34[], int ix[], int gsf, int n )
{
    int i, ixmax;
    float igain;

    igain = look_34igain[gsf];
    ixmax = 0;
    for ( i = 0; i < n; i++ )
    {
        ix[i] = ( int ) ( igain * x34[i] + ( 0.5f - 0.0946f ) );
        if ( ix[i] > 1 )
            ix[i] = 1;
        ixmax |= ix[i];
    }

    return ixmax;
}

/*---------------------------------------------------------------*/
void
vect_limits ( int x[], int upper[], int lower[], int n )
{
    int i;

    for ( i = 0; i < n; i++ )
    {
        if ( x[i] > upper[i] )
            x[i] = upper[i];
        else if ( x[i] < lower[i] )
            x[i] = lower[i];
    }

}

/*---------------------------------------------------------------*/
void
shave_01pairs ( float x34[], int ix[], int g, int n, float thres )
{
    int i;
    float t;

    t = thres / look_34igain[g];

    for ( i = 0; i < n; i += 2 )
    {
        if ( ( ix[i] == 1 ) && ( ix[i + 1] == 0 ) )
        {
            if ( x34[i] < t )
                ix[i] = 0;
        }
        else if ( ( ix[i] == 0 ) && ( ix[i + 1] == 1 ) )
        {
            if ( x34[i + 1] < t )
                ix[i + 1] = 0;
        }
    }

}

/*---------------------------------------------------------------*/
void
fnc_ms_process ( float xr[][576], int n, char ixsign[][576] )
{
    int i;
    float x0, x1;

    for ( i = 0; i < n; i++ )
    {
        x0 = con707 * ( xr[0][i] + xr[1][i] );
        x1 = con707 * ( xr[0][i] - xr[1][i] );
        ixsign[0][i] = ixsign[1][i] = 0;
        if ( x0 < 0.0f )
        {
            ixsign[0][i] = 1;
            x0 = -x0;
        }
        if ( x1 < 0.0f )
        {
            ixsign[1][i] = 1;
            x1 = -x1;
        }
        xr[0][i] = x0;
        xr[1][i] = x1;
    }

}

/*---------------------------------------------------------------*/
void
fnc_ms_process2 ( float xr[][576], int n, char ixsign[][576] )
{
    int i;
    float x0, x1;

// no sqrt(2) scaling, bitallo adjusts scale factors
    for ( i = 0; i < n; i++ )
    {
        x0 = ( xr[0][i] + xr[1][i] );
        x1 = ( xr[0][i] - xr[1][i] );
        ixsign[0][i] = ixsign[1][i] = 0;
        if ( x0 < 0.0f )
        {
            ixsign[0][i] = 1;
            x0 = -x0;
        }
        if ( x1 < 0.0f )
        {
            ixsign[1][i] = 1;
            x1 = -x1;
        }
        xr[0][i] = x0;
        xr[1][i] = x1;
    }

}

/*---------------------------------------------------------------*/
void
fnc_ms_sparse ( float xr[][576], int n, float thres )
{
    int i;
    float ss, sd, t;

// not used, not tested?

    for ( i = 0; i < n; i += 2 )
    {
        ss = xr[0][i] * xr[0][i] + xr[0][i + 1] * xr[0][i + 1];
        sd = xr[1][i] * xr[1][i] + xr[1][i + 1] * xr[1][i + 1];
        t = thres * ( ss + sd );
        if ( sd < t )
        {
            xr[1][i] = 0.0f;
            xr[1][i + 1] = 0.0f;
        }
    }

}

/*---------------------------------------------------------------*/
void
fnc_ms_sparse_sum ( float xr[][576], int n, float thres )
{
    int i;
    float ss, sd, t;

// not used, not tested?

    for ( i = 0; i < n; i += 2 )
    {
        ss = xr[0][i] * xr[0][i] + xr[0][i + 1] * xr[0][i + 1];
        sd = xr[1][i] * xr[1][i] + xr[1][i + 1] * xr[1][i + 1];
        t = thres * ( ss + sd );
        if ( ss < t )
        {
            xr[0][i] = 0.0f;
            xr[0][i + 1] = 0.0f;
        }
    }

}

/*---------------------------------------------------------------*/
void
fnc_sxx ( float xr[][576], int n, float sxx[] )
{
    int i;

    sxx[0] = sxx[1] = 0.0f;
    for ( i = 0; i < n; i++ )
    {
        sxx[0] += xr[0][i] * xr[0][i];
        sxx[1] += xr[1][i] * xr[1][i];
    }

}

/*---------------------------------------------------------------*/
int
vect_imax ( int x[], int n )
{
    int i, imax;

//
// neg x ok, but returns imax >= 0

    imax = 0;
    for ( i = 0; i < n; i++ )
    {
        if ( x[i] > imax )
            imax = x[i];
    }

    return imax;
}

/*---------------------------------------------------------------*/
void
map_xform ( float x[], float rrx[], int npolar )
{
    int i, k;

// map transform energy into FFT bins

    for ( k = 0, i = 0; i < npolar; i += 8, k += 9 )
    {
        rrx[i] =
            map_table[0] * x[k] * x[k] + map_table[7] * x[k + 1] * x[k + 1];
        rrx[i + 1] =
            map_table[1] * x[k + 1] * x[k + 1] + map_table[6] * x[k +
                                                                  2] * x[k +
                                                                         2];
        rrx[i + 2] =
            map_table[2] * x[k + 2] * x[k + 2] + map_table[5] * x[k +
                                                                  3] * x[k +
                                                                         3];
        rrx[i + 3] =
            map_table[3] * x[k + 3] * x[k + 3] + map_table[4] * x[k +
                                                                  4] * x[k +
                                                                         4];
        rrx[i + 4] =
            map_table[4] * x[k + 4] * x[k + 4] + map_table[3] * x[k +
                                                                  5] * x[k +
                                                                         5];
        rrx[i + 5] =
            map_table[5] * x[k + 5] * x[k + 5] + map_table[2] * x[k +
                                                                  6] * x[k +
                                                                         6];
        rrx[i + 6] =
            map_table[6] * x[k + 6] * x[k + 6] + map_table[1] * x[k +
                                                                  7] * x[k +
                                                                         7];
        rrx[i + 7] =
            map_table[7] * x[k + 7] * x[k + 7] + map_table[0] * x[k +
                                                                  8] * x[k +
                                                                         8];
    }

}

/*---------------------------------------------------------------*/
int
ifnc_inverse_gsf_snr2 ( int *qx, float *x, int n )
{
    int i, g;
    float sqq, sqx, q;

// do not call if ixmax = 0 !

    sqq = sqx = 0;
    for ( i = 0; i < n; i++ )
    {
        if ( qx[i] < 256 )
        {
            q = look_ix43[qx[i]];
        }
        else
        {
            q = ( float ) ( pow ( qx[i], ( 4.0 / 3.0 ) ) );
        }
        sqq += q * q;
        sqx += q * x[i];
    }

// result scaled by 8*1024, (8<<13) = scaled g_offset 
    g = 109 * mbLogC ( sqx / sqq ) + ( 8 << 13 );

    return g;
}

/*---------------------------------------------------------------*/
int
ifnc_inverse_gsf_xfer2 ( int *qx, float *x, int n )
{
    int i, g;
    float sqq, sxx, q;

// do not call if ixmax = 0 !

    sqq = sxx = 0;
    for ( i = 0; i < n; i++ )
    {
        if ( qx[i] < 256 )
        {
            q = look_ix43[qx[i]];
        }
        else
        {
            q = ( float ) ( pow ( qx[i], ( 4.0 / 3.0 ) ) );
        }
        sqq += q * q;
        sxx += x[i] * x[i];
    }

// result scaled by 8*1024, (8<<13) = scaled g_offset 
    g = 54 * mbLogC ( sxx / sqq ) + ( 8 << 13 );

    return g;
}

/*---------------------------------------------------------------*/
