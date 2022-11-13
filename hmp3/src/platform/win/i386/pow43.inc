; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: pow43.inc,v 1.2 2005/08/09 20:43:45 karll Exp $ 
;   
; Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.  
;       
; The contents of this file, and the files included with this file, 
; are subject to the current version of the RealNetworks Public 
; Source License (the "RPSL") available at 
; http://www.helixcommunity.org/content/rpsl unless you have licensed 
; the file under the current version of the RealNetworks Community 
; Source License (the "RCSL") available at 
; http://www.helixcommunity.org/content/rcsl, in which case the RCSL 
; will apply. You may also obtain the license terms directly from 
; RealNetworks.  You may not use this file except in compliance with 
; the RPSL or, if you have a valid RCSL with RealNetworks applicable 
; to this file, the RCSL.  Please see the applicable RPSL or RCSL for 
; the rights, obligations and limitations governing use of the 
; contents of the file. 
;   
; This file is part of the Helix DNA Technology. RealNetworks is the 
; developer of the Original Code and owns the copyrights in the 
; portions it created. 
;   
; This file, and the files included with this file, is distributed 
; and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY 
; KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS 
; ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES 
; OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET 
; ENJOYMENT OR NON-INFRINGEMENT. 
;  
; Technology Compatibility Kit Test Suite(s) Location:  
;    http://www.helixcommunity.org/content/tck  
;  
; Contributor(s):  
;   
; ***** END LICENSE BLOCK *****  


;;--- table pow 4/3  coefs ordered [b, a] bits =8 ntable = 256
pow43_bits = 8
look_pow43 label dword
  dd   03FAAC716h,   0BEAB1C58h
  dd   03FAAFFD4h,   0BEAC0032h
  dd   03FAB386Ch,   0BEACE457h
  dd   03FAB70DFh,   0BEADC8C8h
  dd   03FABA92Dh,   0BEAEAD84h
  dd   03FABE156h,   0BEAF928Ch
  dd   03FAC195Ah,   0BEB077DDh
  dd   03FAC513Ah,   0BEB15D7Ah
  dd   03FAC88F6h,   0BEB24361h
  dd   03FACC08Eh,   0BEB32992h
  dd   03FACF802h,   0BEB4100Dh
  dd   03FAD2F53h,   0BEB4F6D2h
  dd   03FAD6680h,   0BEB5DDE1h
  dd   03FAD9D8Bh,   0BEB6C539h
  dd   03FADD473h,   0BEB7ACDAh
  dd   03FAE0B38h,   0BEB894C5h
  dd   03FAE41DAh,   0BEB97CF8h
  dd   03FAE785Bh,   0BEBA6575h
  dd   03FAEAEB9h,   0BEBB4E39h
  dd   03FAEE4F6h,   0BEBC3747h
  dd   03FAF1B11h,   0BEBD209Ch
  dd   03FAF510Bh,   0BEBE0A3Ah
  dd   03FAF86E4h,   0BEBEF41Fh
  dd   03FAFBC9Ch,   0BEBFDE4Ch
  dd   03FAFF233h,   0BEC0C8C1h
  dd   03FB027A9h,   0BEC1B37Dh
  dd   03FB05CFFh,   0BEC29E80h
  dd   03FB09235h,   0BEC389CAh
  dd   03FB0C74Bh,   0BEC4755Bh
  dd   03FB0FC41h,   0BEC56133h
  dd   03FB13118h,   0BEC64D51h
  dd   03FB165CFh,   0BEC739B6h
  dd   03FB19A66h,   0BEC82661h
  dd   03FB1CEDFh,   0BEC91352h
  dd   03FB20339h,   0BECA0089h
  dd   03FB23774h,   0BECAEE05h
  dd   03FB26B90h,   0BECBDBC7h
  dd   03FB29F8Fh,   0BECCC9CFh
  dd   03FB2D36Eh,   0BECDB81Ch
  dd   03FB30730h,   0BECEA6AEh
  dd   03FB33AD4h,   0BECF9584h
  dd   03FB36E5Bh,   0BED084A0h
  dd   03FB3A1C4h,   0BED17400h
  dd   03FB3D50Fh,   0BED263A5h
  dd   03FB4083Dh,   0BED3538Eh
  dd   03FB43B4Fh,   0BED443BBh
  dd   03FB46E43h,   0BED5342Ch
  dd   03FB4A11Bh,   0BED624E1h
  dd   03FB4D3D6h,   0BED715DAh
  dd   03FB50675h,   0BED80716h
  dd   03FB538F7h,   0BED8F896h
  dd   03FB56B5Dh,   0BED9EA58h
  dd   03FB59DA8h,   0BEDADC5Fh
  dd   03FB5CFD6h,   0BEDBCEA8h
  dd   03FB601E9h,   0BEDCC134h
  dd   03FB633E1h,   0BEDDB402h
  dd   03FB665BDh,   0BEDEA713h
  dd   03FB6977Eh,   0BEDF9A67h
  dd   03FB6C924h,   0BEE08DFDh
  dd   03FB6FAAFh,   0BEE181D5h
  dd   03FB72C1Fh,   0BEE275EFh
  dd   03FB75D74h,   0BEE36A4Ah
  dd   03FB78EB0h,   0BEE45EE8h
  dd   03FB7BFD0h,   0BEE553C7h
  dd   03FB7F0D7h,   0BEE648E8h
  dd   03FB821C3h,   0BEE73E49h
  dd   03FB85296h,   0BEE833ECh
  dd   03FB8834Eh,   0BEE929D0h
  dd   03FB8B3EDh,   0BEEA1FF5h
  dd   03FB8E473h,   0BEEB165Bh
  dd   03FB914DFh,   0BEEC0D01h
  dd   03FB94532h,   0BEED03E8h
  dd   03FB9756Bh,   0BEEDFB0Fh
  dd   03FB9A58Ch,   0BEEEF277h
  dd   03FB9D593h,   0BEEFEA1Eh
  dd   03FBA0582h,   0BEF0E205h
  dd   03FBA3558h,   0BEF1DA2Dh
  dd   03FBA6516h,   0BEF2D294h
  dd   03FBA94BBh,   0BEF3CB3Ah
  dd   03FBAC448h,   0BEF4C421h
  dd   03FBAF3BDh,   0BEF5BD46h
  dd   03FBB231Ah,   0BEF6B6ABh
  dd   03FBB525Fh,   0BEF7B04Eh
  dd   03FBB818Ch,   0BEF8AA31h
  dd   03FBBB0A1h,   0BEF9A453h
  dd   03FBBDF9Fh,   0BEFA9EB3h
  dd   03FBC0E86h,   0BEFB9952h
  dd   03FBC3D55h,   0BEFC942Fh
  dd   03FBC6C0Ch,   0BEFD8F4Bh
  dd   03FBC9AADh,   0BEFE8AA4h
  dd   03FBCC937h,   0BEFF863Ch
  dd   03FBCF7A9h,   0BF004109h
  dd   03FBD2605h,   0BF00BF13h
  dd   03FBD544Ah,   0BF013D3Ch
  dd   03FBD8279h,   0BF01BB84h
  dd   03FBDB091h,   0BF0239EAh
  dd   03FBDDE93h,   0BF02B86Fh
  dd   03FBE0C7Fh,   0BF033713h
  dd   03FBE3A54h,   0BF03B5D5h
  dd   03FBE6814h,   0BF0434B6h
  dd   03FBE95BDh,   0BF04B3B5h
  dd   03FBEC351h,   0BF0532D3h
  dd   03FBEF0CFh,   0BF05B20Fh
  dd   03FBF1E37h,   0BF063169h
  dd   03FBF4B89h,   0BF06B0E2h
  dd   03FBF78C7h,   0BF073079h
  dd   03FBFA5EFh,   0BF07B02Eh
  dd   03FBFD301h,   0BF083001h
  dd   03FBFFFFFh,   0BF08AFF2h
  dd   03FC02CE7h,   0BF093001h
  dd   03FC059BBh,   0BF09B02Eh
  dd   03FC0867Ah,   0BF0A3078h
  dd   03FC0B324h,   0BF0AB0E1h
  dd   03FC0DFB9h,   0BF0B3167h
  dd   03FC10C3Ah,   0BF0BB20Bh
  dd   03FC138A6h,   0BF0C32CDh
  dd   03FC164FEh,   0BF0CB3ACh
  dd   03FC19141h,   0BF0D34A9h
  dd   03FC1BD70h,   0BF0DB5C3h
  dd   03FC1E98Ch,   0BF0E36FBh
  dd   03FC21593h,   0BF0EB850h
  dd   03FC24186h,   0BF0F39C2h
  dd   03FC26D65h,   0BF0FBB52h
  dd   03FC29931h,   0BF103CFFh
  dd   03FC2C4E9h,   0BF10BEC9h
  dd   03FC2F08Dh,   0BF1140B0h
  dd   03FC31C1Eh,   0BF11C2B5h
  dd   03FC3479Ch,   0BF1244D6h
  dd   03FC37306h,   0BF12C714h
  dd   03FC39E5Dh,   0BF13496Fh
  dd   03FC3C9A0h,   0BF13CBE7h
  dd   03FC3F4D1h,   0BF144E7Ch
  dd   03FC41FEEh,   0BF14D12Eh
  dd   03FC44AF9h,   0BF1553FCh
  dd   03FC475F1h,   0BF15D6E7h
  dd   03FC4A0D6h,   0BF1659EFh
  dd   03FC4CBA8h,   0BF16DD13h
  dd   03FC4F668h,   0BF176054h
  dd   03FC52115h,   0BF17E3B1h
  dd   03FC54BB0h,   0BF18672Ah
  dd   03FC57638h,   0BF18EAC0h
  dd   03FC5A0AEh,   0BF196E73h
  dd   03FC5CB12h,   0BF19F241h
  dd   03FC5F564h,   0BF1A762Ch
  dd   03FC61FA4h,   0BF1AFA33h
  dd   03FC649D1h,   0BF1B7E56h
  dd   03FC673EDh,   0BF1C0296h
  dd   03FC69DF7h,   0BF1C86F1h
  dd   03FC6C7EFh,   0BF1D0B68h
  dd   03FC6F1D6h,   0BF1D8FFCh
  dd   03FC71BABh,   0BF1E14ABh
  dd   03FC7456Eh,   0BF1E9976h
  dd   03FC76F20h,   0BF1F1E5Dh
  dd   03FC798C0h,   0BF1FA35Fh
  dd   03FC7C24Fh,   0BF20287Eh
  dd   03FC7EBCDh,   0BF20ADB8h
  dd   03FC8153Ah,   0BF21330Dh
  dd   03FC83E95h,   0BF21B87Fh
  dd   03FC867E0h,   0BF223E0Ch
  dd   03FC89119h,   0BF22C3B4h
  dd   03FC8BA42h,   0BF234978h
  dd   03FC8E35Ah,   0BF23CF57h
  dd   03FC90C61h,   0BF245552h
  dd   03FC93557h,   0BF24DB68h
  dd   03FC95E3Ch,   0BF256199h
  dd   03FC98711h,   0BF25E7E5h
  dd   03FC9AFD6h,   0BF266E4Dh
  dd   03FC9D88Ah,   0BF26F4D0h
  dd   03FCA012Dh,   0BF277B6Eh
  dd   03FCA29C1h,   0BF280227h
  dd   03FCA5244h,   0BF2888FBh
  dd   03FCA7AB6h,   0BF290FEAh
  dd   03FCAA319h,   0BF2996F4h
  dd   03FCACB6Ch,   0BF2A1E18h
  dd   03FCAF3AEh,   0BF2AA558h
  dd   03FCB1BE1h,   0BF2B2CB3h
  dd   03FCB4404h,   0BF2BB428h
  dd   03FCB6C17h,   0BF2C3BB8h
  dd   03FCB941Ah,   0BF2CC363h
  dd   03FCBBC0Dh,   0BF2D4B28h
  dd   03FCBE3F1h,   0BF2DD308h
  dd   03FCC0BC5h,   0BF2E5B03h
  dd   03FCC338Ah,   0BF2EE318h
  dd   03FCC5B3Fh,   0BF2F6B48h
  dd   03FCC82E5h,   0BF2FF392h
  dd   03FCCAA7Ch,   0BF307BF6h
  dd   03FCCD203h,   0BF310475h
  dd   03FCCF97Bh,   0BF318D0Eh
  dd   03FCD20E4h,   0BF3215C2h
  dd   03FCD483Dh,   0BF329E8Fh
  dd   03FCD6F88h,   0BF332777h
  dd   03FCD96C3h,   0BF33B07Ah
  dd   03FCDBDF0h,   0BF343996h
  dd   03FCDE50Eh,   0BF34C2CCh
  dd   03FCE0C1Dh,   0BF354C1Dh
  dd   03FCE331Dh,   0BF35D587h
  dd   03FCE5A0Eh,   0BF365F0Bh
  dd   03FCE80F1h,   0BF36E8AAh
  dd   03FCEA7C5h,   0BF377262h
  dd   03FCECE8Bh,   0BF37FC34h
  dd   03FCEF542h,   0BF388620h
  dd   03FCF1BEAh,   0BF391026h
  dd   03FCF4284h,   0BF399A45h
  dd   03FCF6910h,   0BF3A247Fh
  dd   03FCF8F8Eh,   0BF3AAED1h
  dd   03FCFB5FDh,   0BF3B393Eh
  dd   03FCFDC5Eh,   0BF3BC3C4h
  dd   03FD002B1h,   0BF3C4E64h
  dd   03FD028F5h,   0BF3CD91Dh
  dd   03FD04F2Ch,   0BF3D63F0h
  dd   03FD07555h,   0BF3DEEDCh
  dd   03FD09B70h,   0BF3E79E2h
  dd   03FD0C17Ch,   0BF3F0501h
  dd   03FD0E77Bh,   0BF3F9039h
  dd   03FD10D6Dh,   0BF401B8Bh
  dd   03FD13350h,   0BF40A6F6h
  dd   03FD15926h,   0BF41327Ah
  dd   03FD17EEEh,   0BF41BE17h
  dd   03FD1A4A8h,   0BF4249CEh
  dd   03FD1CA55h,   0BF42D59Dh
  dd   03FD1EFF5h,   0BF436186h
  dd   03FD21587h,   0BF43ED88h
  dd   03FD23B0Bh,   0BF4479A3h
  dd   03FD26082h,   0BF4505D7h
  dd   03FD285ECh,   0BF459224h
  dd   03FD2AB49h,   0BF461E89h
  dd   03FD2D098h,   0BF46AB08h
  dd   03FD2F5DAh,   0BF47379Fh
  dd   03FD31B0Fh,   0BF47C450h
  dd   03FD34037h,   0BF485119h
  dd   03FD36552h,   0BF48DDFBh
  dd   03FD38A60h,   0BF496AF5h
  dd   03FD3AF60h,   0BF49F809h
  dd   03FD3D454h,   0BF4A8534h
  dd   03FD3F93Bh,   0BF4B1279h
  dd   03FD41E16h,   0BF4B9FD6h
  dd   03FD442E3h,   0BF4C2D4Ch
  dd   03FD467A4h,   0BF4CBADAh
  dd   03FD48C58h,   0BF4D4881h
  dd   03FD4B0FFh,   0BF4DD640h
  dd   03FD4D59Ah,   0BF4E6417h
  dd   03FD4FA28h,   0BF4EF207h
  dd   03FD51EAAh,   0BF4F8010h
  dd   03FD5431Fh,   0BF500E30h
  dd   03FD56787h,   0BF509C69h
  dd   03FD58BE4h,   0BF512ABAh
  dd   03FD5B034h,   0BF51B924h
  dd   03FD5D477h,   0BF5247A5h
  dd   03FD5F8AFh,   0BF52D63Fh
  dd   03FD61CDAh,   0BF5364F1h
  dd   03FD640F8h,   0BF53F3BBh
  dd   03FD6650Bh,   0BF54829Dh
  dd   03FD68912h,   0BF551197h
  dd   03FD6AD0Ch,   0BF55A0A9h
  dd   03FD6D0FBh,   0BF562FD3h
  dd   03FD6F4DDh,   0BF56BF15h
;;--- pow43 exponent table CAUTION table starts at x = 1.0
look_exp43 label dword
  dd   03F800000h 
  dd   040214518h 
  dd   040CB2FF5h 
  dd   041800000h 
  dd   042214518h 
  dd   042CB2FF5h 
  dd   043800000h 
  dd   044214518h 
  dd   044CB2FF5h 
  dd   045800000h 
  dd   046214518h 
  dd   046CB2FF5h 
  dd   047800000h 
  dd   048214518h 
  dd   048CB2FF5h 
  dd   049800000h 
  dd   04A214518h 
  dd   04ACB2FF5h 
  dd   04B800000h 
  dd   04C214518h 
  dd   04CCB2FF5h 
  dd   04D800000h 
  dd   04E214518h 
  dd   04ECB2FF5h 
  dd   04F800000h 
  dd   050214518h 
  dd   050CB2FF5h 
  dd   051800000h 
  dd   052214518h 
  dd   052CB2FF5h 
  dd   053800000h 
  dd   054214518h 
  dd   054CB2FF5h 
  dd   055800000h 
  dd   056214518h 
  dd   056CB2FF5h 
  dd   057800000h 
  dd   058214518h 
  dd   058CB2FF5h 
  dd   059800000h 
  dd   05A214518h 
  dd   05ACB2FF5h 
  dd   05B800000h 
  dd   05C214518h 
  dd   05CCB2FF5h 
  dd   05D800000h 
  dd   05E214518h 
  dd   05ECB2FF5h 
  dd   05F800000h 
  dd   060214518h 
  dd   060CB2FF5h 
  dd   061800000h 
  dd   062214518h 
  dd   062CB2FF5h 
  dd   063800000h 
  dd   064214518h 
  dd   064CB2FF5h 
  dd   065800000h 
  dd   066214518h 
  dd   066CB2FF5h 
  dd   067800000h 
  dd   068214518h 
  dd   068CB2FF5h 
  dd   069800000h 
  dd   06A214518h 
  dd   06ACB2FF5h 
  dd   06B800000h 
  dd   06C214518h 
  dd   06CCB2FF5h 
  dd   06D800000h 
  dd   06E214518h 
  dd   06ECB2FF5h 
  dd   06F800000h 
  dd   070214518h 
  dd   070CB2FF5h 
  dd   071800000h 
  dd   072214518h 
  dd   072CB2FF5h 
  dd   073800000h 
  dd   074214518h 
  dd   074CB2FF5h 
  dd   075800000h 
  dd   076214518h 
  dd   076CB2FF5h 
  dd   077800000h 
  dd   078214518h 
  dd   078CB2FF5h 
  dd   079800000h 
  dd   07A214518h 
  dd   07ACB2FF5h 
  dd   07B800000h 
  dd   07C214518h 
  dd   07CCB2FF5h 
  dd   07D800000h 
  dd   07E214518h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
  dd   07ECB2FF5h 
