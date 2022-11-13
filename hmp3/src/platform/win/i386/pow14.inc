; ***** BEGIN LICENSE BLOCK *****  
; Source last modified: $Id: pow14.inc,v 1.2 2005/08/09 20:43:45 karll Exp $ 
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


;;------------- pow14 coefs ordered [b, a]
look_pow14 label dword
  dd   03E7FA039h,   03F4017F2h
  dd   03E7EE186h,   03F4047CEh
  dd   03E7E241Fh,   03F407787h
  dd   03E7D67FFh,   03F40A71Ch
  dd   03E7CAD22h,   03F40D68Eh
  dd   03E7BF386h,   03F4105DDh
  dd   03E7B3B26h,   03F413509h
  dd   03E7A8401h,   03F416413h
  dd   03E79CE11h,   03F4192FBh
  dd   03E791955h,   03F41C1C1h
  dd   03E7865C9h,   03F41F065h
  dd   03E77B369h,   03F421EE7h
  dd   03E770233h,   03F424D48h
  dd   03E765224h,   03F427B88h
  dd   03E75A339h,   03F42A9A7h
  dd   03E74F56Fh,   03F42D7A5h
  dd   03E7448C2h,   03F430583h
  dd   03E739D30h,   03F433341h
  dd   03E72F2B7h,   03F4360DEh
  dd   03E724953h,   03F438E5Ch
  dd   03E71A101h,   03F43BBBAh
  dd   03E70F9BFh,   03F43E8F9h
  dd   03E70538Bh,   03F441618h
  dd   03E6FAE61h,   03F444318h
  dd   03E6F0A3Fh,   03F446FF9h
  dd   03E6E6722h,   03F449CBCh
  dd   03E6DC508h,   03F44C960h
  dd   03E6D23EEh,   03F44F5E6h
  dd   03E6C83D2h,   03F45224Eh
  dd   03E6BE4B1h,   03F454E98h
  dd   03E6B468Ah,   03F457AC4h
  dd   03E6AA959h,   03F45A6D2h
  dd   03E6A0D1Ch,   03F45D2C3h
  dd   03E6971D1h,   03F45FE97h
  dd   03E68D775h,   03F462A4Eh
  dd   03E683E07h,   03F4655E8h
  dd   03E67A584h,   03F468166h
  dd   03E670DEAh,   03F46ACC6h
  dd   03E667737h,   03F46D80Bh
  dd   03E65E168h,   03F470333h
  dd   03E654C7Bh,   03F472E40h
  dd   03E64B86Fh,   03F475930h
  dd   03E642542h,   03F478405h
  dd   03E6392F0h,   03F47AEBEh
  dd   03E630179h,   03F47D95Ch
  dd   03E6270DAh,   03F4803DFh
  dd   03E61E111h,   03F482E47h
  dd   03E61521Dh,   03F485893h
  dd   03E60C3FBh,   03F4882C5h
  dd   03E6036A9h,   03F48ACDDh
  dd   03E5FAA26h,   03F48D6DAh
  dd   03E5F1E70h,   03F4900BDh
  dd   03E5E9385h,   03F492A86h
  dd   03E5E0963h,   03F495434h
  dd   03E5D8009h,   03F497DC9h
  dd   03E5CF774h,   03F49A744h
  dd   03E5C6FA3h,   03F49D0A6h
  dd   03E5BE894h,   03F49F9EEh
  dd   03E5B6246h,   03F4A231Dh
  dd   03E5ADCB6h,   03F4A4C33h
  dd   03E5A57E4h,   03F4A7530h
  dd   03E59D3CDh,   03F4A9E14h
  dd   03E595070h,   03F4AC6E0h
  dd   03E58CDCBh,   03F4AEF93h
  dd   03E584BDDh,   03F4B182Dh
  dd   03E57CAA4h,   03F4B40AFh
  dd   03E574A1Eh,   03F4B6919h
  dd   03E56CA4Bh,   03F4B916Bh
  dd   03E564B28h,   03F4BB9A5h
  dd   03E55CCB4h,   03F4BE1C7h
  dd   03E554EEEh,   03F4C09D2h
  dd   03E54D1D4h,   03F4C31C5h
  dd   03E545564h,   03F4C59A1h
  dd   03E53D99Eh,   03F4C8165h
  dd   03E535E80h,   03F4CA913h
  dd   03E52E408h,   03F4CD0A9h
  dd   03E526A35h,   03F4CF828h
  dd   03E51F106h,   03F4D1F91h
  dd   03E517879h,   03F4D46E3h
  dd   03E51008Dh,   03F4D6E1Eh
  dd   03E508941h,   03F4D9543h
  dd   03E501294h,   03F4DBC52h
  dd   03E4F9C83h,   03F4DE34Ah
  dd   03E4F270Fh,   03F4E0A2Ch
  dd   03E4EB235h,   03F4E30F9h
  dd   03E4E3DF4h,   03F4E57AFh
  dd   03E4DCA4Ch,   03F4E7E50h
  dd   03E4D573Ah,   03F4EA4DBh
  dd   03E4CE4BEh,   03F4ECB51h
  dd   03E4C72D7h,   03F4EF1B1h
  dd   03E4C0182h,   03F4F17FCh
  dd   03E4B90C1h,   03F4F3E32h
  dd   03E4B2090h,   03F4F6452h
  dd   03E4AB0EEh,   03F4F8A5Eh
  dd   03E4A41DCh,   03F4FB055h
  dd   03E49D357h,   03F4FD637h
  dd   03E49655Fh,   03F4FFC04h
  dd   03E48F7F2h,   03F5021BDh
  dd   03E488B10h,   03F504761h
  dd   03E481EB7h,   03F506CF1h
  dd   03E47B2E6h,   03F50926Dh
  dd   03E47479Ch,   03F50B7D4h
  dd   03E46DCD8h,   03F50DD28h
  dd   03E467299h,   03F510267h
  dd   03E4608DFh,   03F512793h
  dd   03E459FA7h,   03F514CABh
  dd   03E4536F2h,   03F5171AFh
  dd   03E44CEBEh,   03F51969Fh
  dd   03E446709h,   03F51BB7Ch
  dd   03E43FFD4h,   03F51E046h
  dd   03E43991Dh,   03F5204FCh
  dd   03E4332E4h,   03F52299Fh
  dd   03E42CD27h,   03F524E2Fh
  dd   03E4267E5h,   03F5272ACh
  dd   03E42031Dh,   03F529716h
  dd   03E419ED0h,   03F52BB6Eh
  dd   03E413AFAh,   03F52DFB2h
  dd   03E40D79Dh,   03F5303E4h
  dd   03E4074B7h,   03F532803h
  dd   03E401246h,   03F534C10h
  dd   03E3FB04Bh,   03F53700Ah
  dd   03E3F4EC4h,   03F5393F2h
  dd   03E3EEDB1h,   03F53B7C8h
  dd   03E3E8D10h,   03F53DB8Bh
  dd   03E3E2CE1h,   03F53FF3Dh
  dd   03E3DCD23h,   03F5422DCh
  dd   03E3D6DD5h,   03F544669h
  dd   03E3D0EF6h,   03F5469E5h
  dd   03E3CB087h,   03F548D4Fh
  dd   03E3C5284h,   03F54B0A7h
  dd   03E3BF4EFh,   03F54D3EEh
  dd   03E3B97C6h,   03F54F723h
  dd   03E3B3B09h,   03F551A47h
  dd   03E3ADEB6h,   03F553D5Ah
  dd   03E3A82CDh,   03F55605Bh
  dd   03E3A274Eh,   03F55834Bh
  dd   03E39CC37h,   03F55A62Ah
  dd   03E397187h,   03F55C8F7h
  dd   03E39173Fh,   03F55EBB4h
  dd   03E38BD5Dh,   03F560E60h
  dd   03E3863E1h,   03F5630FBh
  dd   03E380AC9h,   03F565386h
  dd   03E37B216h,   03F5675FFh
  dd   03E3759C6h,   03F569868h
  dd   03E3701D9h,   03F56BAC1h
  dd   03E36AA4Eh,   03F56DD09h
  dd   03E365324h,   03F56FF41h
  dd   03E35FC5Ch,   03F572168h
  dd   03E35A5F3h,   03F574380h
  dd   03E354FEAh,   03F576587h
  dd   03E34FA40h,   03F57877Eh
  dd   03E34A4F4h,   03F57A965h
  dd   03E345006h,   03F57CB3Ch
  dd   03E33FB74h,   03F57ED03h
  dd   03E33A73Fh,   03F580EBAh
  dd   03E335366h,   03F583061h
  dd   03E32FFE8h,   03F5851F9h
  dd   03E32ACC4h,   03F587381h
  dd   03E3259FAh,   03F5894FAh
  dd   03E320789h,   03F58B663h
  dd   03E31B571h,   03F58D7BDh
  dd   03E3163B1h,   03F58F907h
  dd   03E311249h,   03F591A42h
  dd   03E30C138h,   03F593B6Eh
  dd   03E30707Dh,   03F595C8Bh
  dd   03E302018h,   03F597D98h
  dd   03E2FD008h,   03F599E97h
  dd   03E2F804Dh,   03F59BF87h
  dd   03E2F30E6h,   03F59E067h
  dd   03E2EE1D3h,   03F5A0139h
  dd   03E2E9312h,   03F5A21FCh
  dd   03E2E44A5h,   03F5A42B0h
  dd   03E2DF689h,   03F5A6356h
  dd   03E2DA8BFh,   03F5A83EDh
  dd   03E2D5B46h,   03F5AA475h
  dd   03E2D0E1Dh,   03F5AC4EFh
  dd   03E2CC144h,   03F5AE55Ah
  dd   03E2C74BBh,   03F5B05B7h
  dd   03E2C2881h,   03F5B2606h
  dd   03E2BDC95h,   03F5B4647h
  dd   03E2B90F7h,   03F5B6679h
  dd   03E2B45A6h,   03F5B869Dh
  dd   03E2AFAA2h,   03F5BA6B3h
  dd   03E2AAFEBh,   03F5BC6BBh
  dd   03E2A6580h,   03F5BE6B5h
  dd   03E2A1B61h,   03F5C06A1h
  dd   03E29D18Ch,   03F5C267Fh
  dd   03E298802h,   03F5C4650h
  dd   03E293EC3h,   03F5C6612h
  dd   03E28F5CCh,   03F5C85C7h
  dd   03E28AD20h,   03F5CA56Fh
  dd   03E2864BCh,   03F5CC508h
  dd   03E281CA0h,   03F5CE494h
  dd   03E27D4CCh,   03F5D0413h
  dd   03E278D40h,   03F5D2384h
  dd   03E2745FAh,   03F5D42E8h
  dd   03E26FEFBh,   03F5D623Fh
  dd   03E26B843h,   03F5D8188h
  dd   03E2671D0h,   03F5DA0C4h
  dd   03E262BA2h,   03F5DBFF3h
  dd   03E25E5B9h,   03F5DDF14h
  dd   03E25A015h,   03F5DFE29h
  dd   03E255AB5h,   03F5E1D30h
  dd   03E251599h,   03F5E3C2Bh
  dd   03E24D0BFh,   03F5E5B18h
  dd   03E248C29h,   03F5E79F9h
  dd   03E2447D5h,   03F5E98CDh
  dd   03E2403C3h,   03F5EB794h
  dd   03E23BFF3h,   03F5ED64Eh
  dd   03E237C64h,   03F5EF4FCh
  dd   03E233916h,   03F5F139Dh
  dd   03E22F608h,   03F5F3232h
  dd   03E22B33Bh,   03F5F50B9h
  dd   03E2270ADh,   03F5F6F35h
  dd   03E222E5Fh,   03F5F8DA4h
  dd   03E21EC50h,   03F5FAC06h
  dd   03E21AA7Fh,   03F5FCA5Ch
  dd   03E2168EDh,   03F5FE8A6h
  dd   03E212798h,   03F6006E4h
  dd   03E20E682h,   03F602515h
  dd   03E20A5A8h,   03F60433Ah
  dd   03E20650Bh,   03F606153h
  dd   03E2024ABh,   03F607F60h
  dd   03E1FE487h,   03F609D61h
  dd   03E1FA49Fh,   03F60BB56h
  dd   03E1F64F2h,   03F60D93Fh
  dd   03E1F2580h,   03F60F71Ch
  dd   03E1EE64Ah,   03F6114EDh
  dd   03E1EA74Dh,   03F6132B2h
  dd   03E1E688Bh,   03F61506Ch
  dd   03E1E2A03h,   03F616E19h
  dd   03E1DEBB4h,   03F618BBBh
  dd   03E1DAD9Eh,   03F61A952h
  dd   03E1D6FC1h,   03F61C6DDh
  dd   03E1D321Ch,   03F61E45Ch
  dd   03E1CF4B0h,   03F6201CFh
  dd   03E1CB77Ch,   03F621F38h
  dd   03E1C7A7Fh,   03F623C94h
  dd   03E1C3DBAh,   03F6259E5h
  dd   03E1C012Bh,   03F62772Bh
  dd   03E1BC4D3h,   03F629466h
  dd   03E1B88B2h,   03F62B195h
  dd   03E1B4CC6h,   03F62CEB9h
  dd   03E1B1110h,   03F62EBD2h
  dd   03E1AD590h,   03F6308E0h
  dd   03E1A9A45h,   03F6325E2h
  dd   03E1A5F2Fh,   03F6342DAh
  dd   03E1A244Dh,   03F635FC6h
  dd   03E19E9A0h,   03F637CA7h
  dd   03E19AF27h,   03F63997Eh
  dd   03E1974E1h,   03F63B649h
  dd   03E193ACFh,   03F63D309h
  dd   03E1900F0h,   03F63EFBFh
  dd   03E18C744h,   03F640C6Ah
  dd   03E188DCAh,   03F64290Ah
  dd   03E185483h,   03F64459Fh
;;------------- pow14 exponent table
look_exp14 label dword
  dd   000000000h 
  dd   02FB504F3h 
  dd   02FD744FDh 
  dd   030000000h 
  dd   0301837F0h 
  dd   0303504F3h 
  dd   0305744FDh 
  dd   030800000h 
  dd   0309837F0h 
  dd   030B504F3h 
  dd   030D744FDh 
  dd   031000000h 
  dd   0311837F0h 
  dd   0313504F3h 
  dd   0315744FDh 
  dd   031800000h 
  dd   0319837F0h 
  dd   031B504F3h 
  dd   031D744FDh 
  dd   032000000h 
  dd   0321837F0h 
  dd   0323504F3h 
  dd   0325744FDh 
  dd   032800000h 
  dd   0329837F0h 
  dd   032B504F3h 
  dd   032D744FDh 
  dd   033000000h 
  dd   0331837F0h 
  dd   0333504F3h 
  dd   0335744FDh 
  dd   033800000h 
  dd   0339837F0h 
  dd   033B504F3h 
  dd   033D744FDh 
  dd   034000000h 
  dd   0341837F0h 
  dd   0343504F3h 
  dd   0345744FDh 
  dd   034800000h 
  dd   0349837F0h 
  dd   034B504F3h 
  dd   034D744FDh 
  dd   035000000h 
  dd   0351837F0h 
  dd   0353504F3h 
  dd   0355744FDh 
  dd   035800000h 
  dd   0359837F0h 
  dd   035B504F3h 
  dd   035D744FDh 
  dd   036000000h 
  dd   0361837F0h 
  dd   0363504F3h 
  dd   0365744FDh 
  dd   036800000h 
  dd   0369837F0h 
  dd   036B504F3h 
  dd   036D744FDh 
  dd   037000000h 
  dd   0371837F0h 
  dd   0373504F3h 
  dd   0375744FDh 
  dd   037800000h 
  dd   0379837F0h 
  dd   037B504F3h 
  dd   037D744FDh 
  dd   038000000h 
  dd   0381837F0h 
  dd   0383504F3h 
  dd   0385744FDh 
  dd   038800000h 
  dd   0389837F0h 
  dd   038B504F3h 
  dd   038D744FDh 
  dd   039000000h 
  dd   0391837F0h 
  dd   0393504F3h 
  dd   0395744FDh 
  dd   039800000h 
  dd   0399837F0h 
  dd   039B504F3h 
  dd   039D744FDh 
  dd   03A000000h 
  dd   03A1837F0h 
  dd   03A3504F3h 
  dd   03A5744FDh 
  dd   03A800000h 
  dd   03A9837F0h 
  dd   03AB504F3h 
  dd   03AD744FDh 
  dd   03B000000h 
  dd   03B1837F0h 
  dd   03B3504F3h 
  dd   03B5744FDh 
  dd   03B800000h 
  dd   03B9837F0h 
  dd   03BB504F3h 
  dd   03BD744FDh 
  dd   03C000000h 
  dd   03C1837F0h 
  dd   03C3504F3h 
  dd   03C5744FDh 
  dd   03C800000h 
  dd   03C9837F0h 
  dd   03CB504F3h 
  dd   03CD744FDh 
  dd   03D000000h 
  dd   03D1837F0h 
  dd   03D3504F3h 
  dd   03D5744FDh 
  dd   03D800000h 
  dd   03D9837F0h 
  dd   03DB504F3h 
  dd   03DD744FDh 
  dd   03E000000h 
  dd   03E1837F0h 
  dd   03E3504F3h 
  dd   03E5744FDh 
  dd   03E800000h 
  dd   03E9837F0h 
  dd   03EB504F3h 
  dd   03ED744FDh 
  dd   03F000000h 
  dd   03F1837F0h 
  dd   03F3504F3h 
  dd   03F5744FDh 
  dd   03F800000h 
  dd   03F9837F0h 
  dd   03FB504F3h 
  dd   03FD744FDh 
  dd   040000000h 
  dd   0401837F0h 
  dd   0403504F3h 
  dd   0405744FDh 
  dd   040800000h 
  dd   0409837F0h 
  dd   040B504F3h 
  dd   040D744FDh 
  dd   041000000h 
  dd   0411837F0h 
  dd   0413504F3h 
  dd   0415744FDh 
  dd   041800000h 
  dd   0419837F0h 
  dd   041B504F3h 
  dd   041D744FDh 
  dd   042000000h 
  dd   0421837F0h 
  dd   0423504F3h 
  dd   0425744FDh 
  dd   042800000h 
  dd   0429837F0h 
  dd   042B504F3h 
  dd   042D744FDh 
  dd   043000000h 
  dd   0431837F0h 
  dd   0433504F3h 
  dd   0435744FDh 
  dd   043800000h 
  dd   0439837F0h 
  dd   043B504F3h 
  dd   043D744FDh 
  dd   044000000h 
  dd   0441837F0h 
  dd   0443504F3h 
  dd   0445744FDh 
  dd   044800000h 
  dd   0449837F0h 
  dd   044B504F3h 
  dd   044D744FDh 
  dd   045000000h 
  dd   0451837F0h 
  dd   0453504F3h 
  dd   0455744FDh 
  dd   045800000h 
  dd   0459837F0h 
  dd   045B504F3h 
  dd   045D744FDh 
  dd   046000000h 
  dd   0461837F0h 
  dd   0463504F3h 
  dd   0465744FDh 
  dd   046800000h 
  dd   0469837F0h 
  dd   046B504F3h 
  dd   046D744FDh 
  dd   047000000h 
  dd   0471837F0h 
  dd   0473504F3h 
  dd   0475744FDh 
  dd   047800000h 
  dd   0479837F0h 
  dd   047B504F3h 
  dd   047D744FDh 
  dd   048000000h 
  dd   0481837F0h 
  dd   0483504F3h 
  dd   0485744FDh 
  dd   048800000h 
  dd   0489837F0h 
  dd   048B504F3h 
  dd   048D744FDh 
  dd   049000000h 
  dd   0491837F0h 
  dd   0493504F3h 
  dd   0495744FDh 
  dd   049800000h 
  dd   0499837F0h 
  dd   049B504F3h 
  dd   049D744FDh 
  dd   04A000000h 
  dd   04A1837F0h 
  dd   04A3504F3h 
  dd   04A5744FDh 
  dd   04A800000h 
  dd   04A9837F0h 
  dd   04AB504F3h 
  dd   04AD744FDh 
  dd   04B000000h 
  dd   04B1837F0h 
  dd   04B3504F3h 
  dd   04B5744FDh 
  dd   04B800000h 
  dd   04B9837F0h 
  dd   04BB504F3h 
  dd   04BD744FDh 
  dd   04C000000h 
  dd   04C1837F0h 
  dd   04C3504F3h 
  dd   04C5744FDh 
  dd   04C800000h 
  dd   04C9837F0h 
  dd   04CB504F3h 
  dd   04CD744FDh 
  dd   04D000000h 
  dd   04D1837F0h 
  dd   04D3504F3h 
  dd   04D5744FDh 
  dd   04D800000h 
  dd   04D9837F0h 
  dd   04DB504F3h 
  dd   04DD744FDh 
  dd   04E000000h 
  dd   04E1837F0h 
  dd   04E3504F3h 
  dd   04E5744FDh 
  dd   04E800000h 
  dd   04E9837F0h 
  dd   04EB504F3h 
  dd   04ED744FDh 
  dd   04F000000h 
  dd   04F1837F0h 
  dd   04F3504F3h 
  dd   04F5744FDh 
  dd   04F802666h 

