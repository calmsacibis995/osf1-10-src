#define ImgX_FACILITY 2540
/*                                                                          */
/*+                                                                         */
/* NOTES:                                                                   */
/*                                                                          */
/*	* The .TITLE keyword must be separated by two spaces instead             */
/*	  of a tab so that the .TPU procedure that modifies this file            */
/*	  to produce the IMG_XSTATUS_CODES.OBJ file will be able to              */
/*	  find it.                                                               */
/*                                                                          */
/*	* The prefix string "Img$_" must be in mixed case so that the            */
/*	  CHF message compiler will produce "ImgX_" prefixes also with           */
/*	  mixed case.                                                            */
/*-                                                                         */
/*                                                                          */
/*+                                                                         */
/* General success codes                                                    */
/*-                                                                         */
#define ImgX_NORMAL 166494209
#define ImgX_SUCCESS 166494217
/*                                                                          */
/*+                                                                         */
/* Informational codes.                                                     */
/*-                                                                         */
/*AGGNOTSEG	<aggregate type was not DDIF$_SEG>                              */
#define ImgX_IMGNOTCMP 166495011
#define ImgX_NODATAFND 166495019
#define ImgX_NOSAVDCTX 166495027
/*                                                                          */
/*+                                                                         */
/* Warning codes. (LIB$SIGNAL only)                                         */
/*-                                                                         */
#define ImgX_ATRNOTNOR 166495808
#define ImgX_DXCPYTRUN 166495816
#define ImgX_INVFLGARG 166495824
#define ImgX_INVITMCOD 166495832
#define ImgX_INVSCNLEN 166495840
#define ImgX_MODPRMSNG 166495848
#define ImgX_NOIDUFOU 166495856
#define ImgX_NOPRAFOU 166495864
#define ImgX_NOROOTFOU 166495872
#define ImgX_NOSEGFOU 166495880
#define ImgX_NOSGAFOU 166495888
#define ImgX_NULPTRFND 166495896
#define ImgX_PRVCTXINV 166495904
#define ImgX_RVALTRUNC 166495912
#define ImgX_UNSASPRAT 166495920
#define ImgX_UNSCMPTYP 166495928
#define ImgX_UNSGRDTYP 166495936
#define ImgX_UNSPXLSTR 166495944
#define ImgX_UNSSPCTYP 166495952
#define ImgX_UNXPCTEOB 166495960
/*                                                                          */
/* V3 warning codes                                                         */
/*                                                                          */
#define ImgX_ATRNOTSTD 166495968
#define ImgX_BPCNOTEQL 166495976
#define ImgX_BRXNESTRX 166495984
#define ImgX_BRYNESTRY 166495992
#define ImgX_DPNOTUNC 166496000
#define ImgX_DTYPMISMA 166496008
#define ImgX_FRMNOTFXD 166496016
#define ImgX_FRMNOTUNC 166496024
#define ImgX_INCBPPTOT 166496032
#define ImgX_INCNSARG 166496040
#define ImgX_INSDPSIZE 166496048
#define ImgX_INVATRIDX 166496056
#define ImgX_INVATRVAL 166496064
#define ImgX_INVBRXSZ 166496072
#define ImgX_INVBRYSZ 166496080
#define ImgX_INVDARG 166496088
#define ImgX_INVDPSIZE 166496096
#define ImgX_INVDTYPE 166496104
#define ImgX_INVIDUCNT 166496112
#define ImgX_INVLUTATR 166496120
#define ImgX_INVLUTSIZ 166496128
#define ImgX_INVPLNCNT 166496136
#define ImgX_INVPREDFL 166496144
#define ImgX_INVSCNSTR 166496152
#define ImgX_INVZERVAL 166496160
#define ImgX_NODATAPLA 166496168
#define ImgX_NOIMGFOU 166496176
#define ImgX_NOINPLACE 166496184
#define ImgX_NOMATCH 166496192
#define ImgX_NONSTDBPP 166496200
#define ImgX_VALNOTCON 166496208
#define ImgX_ZERLUTTAB 166496216
/*                                                                          */
/* Fatal codes (LIB$STOP only)                                              */
/*                                                                          */
/*                                                                          */
#define ImgX_ACTNOTDEF 166497412
#define ImgX_ACTNOTSUC 166497420
#define ImgX_AGGNOTSEG 166497428
#define ImgX_AGNOTINFR 166497436
#define ImgX_ATTLSTREQ 166497444
#define ImgX_BLKNOTINT 166497452
#define ImgX_BUFOVRFLW 166497460
#define ImgX_BUFWNOLEN 166497468
#define ImgX_DDIFIMPER 166497476
#define ImgX_DLENGTR32 166497484
#define ImgX_DSCNOTDYN 166497492
#define ImgX_DSTLENZER 166497500
#define ImgX_EDGBOUNDE 166497508
#define ImgX_FRAMIMPER 166497516
#define ImgX_ILLDSTBUF 166497524
#define ImgX_ILLIMGSEG 166497532
#define ImgX_INCACCMOD 166497540
#define ImgX_INCFRMATT 166497548
#define ImgX_INCLSTCNT 166497556
#define ImgX_INSVIRMEM 166497564
#define ImgX_INVAGRSEQ 166497572
#define ImgX_INVALDFID 166497580
#define ImgX_INVALDSID 166497588
#define ImgX_INVALIGNV 166497596
#define ImgX_INVALLSIZ 166497604
#define ImgX_INVARGCNT 166497612
#define ImgX_INVARGCON 166497620
#define ImgX_INVBITPXL 166497628
#define ImgX_INVBITSPC 166497636
#define ImgX_INVBLKTYP 166497644
#define ImgX_INVBPCLST 166497652
#define ImgX_INVBUFLEN 166497660
#define ImgX_INVCMPCNT 166497668
#define ImgX_INVCMPIDX 166497676
#define ImgX_INVCMPMAP 166497684
#define ImgX_INVCMPSTL 166497692
#define ImgX_INVCMPSTR 166497700
#define ImgX_INVCMPTYP 166497708
#define ImgX_INVCODTYP 166497716
#define ImgX_INVCTXUSE 166497724
#define ImgX_INVDDIFMD 166497732
#define ImgX_INVDSCCLA 166497740
#define ImgX_INVDSCLEN 166497748
#define ImgX_INVDSCTYP 166497756
#define ImgX_INVDTHBIT 166497764
#define ImgX_INVDTHLEV 166497772
#define ImgX_INVDTHSPT 166497780
#define ImgX_INVDTHTYP 166497788
#define ImgX_INVFLAORD 166497796
#define ImgX_INVFLBLEN 166497804
#define ImgX_INVFLSORD 166497812
#define ImgX_INVITMBFL 166497820
#define ImgX_INVHSTGRM 166497828
#define ImgX_INVLINPRG 166497836
#define ImgX_INVMODVAL 166497844
#define ImgX_INVPLAIND 166497852
#define ImgX_INVPXLCNT 166497860
#define ImgX_INVPXLIDX 166497868
#define ImgX_INVPXLPTH 166497876
#define ImgX_INVPXLSTR 166497884
#define ImgX_INVROI 166497892
#define ImgX_INVSCLFTR 166497900
#define ImgX_INVSCNIDX 166497908
#define ImgX_INVSRCDSC 166497916
#define ImgX_INVTHRSPC 166497924
#define ImgX_INVTYPCOD 166497932
#define ImgX_IOBREALER 166497940
#define ImgX_ITMCODERR 166497948
#define ImgX_MEMREALER 166497956
#define ImgX_NOIMGAGRP 166497964
#define ImgX_NOIMGDATA 166497972
#define ImgX_NOIOTARGT 166497980
#define ImgX_NORECTDSC 166497988
#define ImgX_NOSPCPRC 166497996
#define ImgX_NOTIMGSEG 166498004
#define ImgX_NOTYPESUP 166498012
#define ImgX_NOTUSRMEM 166498020
#define ImgX_NOUNCDATA 166498028
#define ImgX_NOWRTACC 166498036
#define ImgX_PARAMCONF 166498044
#define ImgX_PVTITMCOD 166498052
#define ImgX_PXLTOOLRG 166498060
#define ImgX_SLCGTRDLC 166498068
#define ImgX_SLENGTR32 166498076
#define ImgX_SPCGTRDPC 166498084
#define ImgX_SPLGTRDPL 166498092
#define ImgX_SPLGTRDPS 166498100
#define ImgX_SRCLENZER 166498108
#define ImgX_UNSCSPORG 166498116
#define ImgX_UNSDATALN 166498124
#define ImgX_UNSDTHMAP 166498132
#define ImgX_UNSDTHCPO 166498140
#define ImgX_UNSOPTION 166498148
/*                                                                          */
/*                                                                          */
/* V3 fatal errors                                                          */
/*                                                                          */
#define ImgX_DCTCMPIDX 166498156
#define ImgX_DCTDECFAI 166498164
#define ImgX_DCTENCFAI 166498172
#define ImgX_DCTFACTER 166498180
#define ImgX_DPREMAPER 166498188
#define ImgX_EOF 166498196
#define ImgX_FAILURE 166498204
#define ImgX_FILCLOSER 166498212
#define ImgX_FILECRERR 166498220
#define ImgX_FILOPENER 166498228
#define ImgX_FILREADER 166498236
#define ImgX_FILWRITER 166498244
#define ImgX_FRAMEDEAL 166498252
#define ImgX_FRMLEVMIS 166498260
#define ImgX_FRMSIZMIS 166498268
#define ImgX_FRMATTMIS 166498276
#define ImgX_IDXRNGEXC 166498284
#define ImgX_INVACCMOD 166498292
#define ImgX_INVACCTYP 166498300
#define ImgX_INVDATCLA 166498308
#define ImgX_INVDCPP 166498316
#define ImgX_INVDMTHARG 166498324
#define ImgX_INVFILACC 166498332
#define ImgX_INVFILBUF 166498340
#define ImgX_INVFRMDEF 166498348
#define ImgX_INVFRMID 166498356
#define ImgX_INVFTYPE 166498364
#define ImgX_INVHROIDM 166498372
#define ImgX_INVLUTDEF 166498380
#define ImgX_INVROIITM 166498388
#define ImgX_INVROIOBJ 166498396
#define ImgX_INVROITYP 166498404
#define ImgX_INVVROIDM 166498412
#define ImgX_INVZERFID 166498420
#define ImgX_INVZERITM 166498428
#define ImgX_LUTDFNCNF 166498436
#define ImgX_LUTTOOSMA 166498444
#define ImgX_MEMALLOC 166498452
#define ImgX_NOTUSRLTD 166498460
#define ImgX_OPARGCONF 166498468
#define ImgX_ROIRECTSZ 166498476
#define ImgX_TABATRCNF 166498484
#define ImgX_TABSZNTFO 166498492
#define ImgX_UNSBITSPC 166498500
#define ImgX_UNSDATACL 166498508
#define ImgX_UNSLTDTYP 166498516
#define ImgX_UNSLTABTY 166498524
#define ImgX_UNSROITYP 166498532
#define ImgX_ZERDFNTYP 166498540
#define ImgX_ZERLUTADR 166498548
#define ImgX_ZERLUTSIZ 166498556
#define ImgX_ZEROROIID 166498564
/*                                                                          */
/*                                                                          */
/* more V3 fatal errors                                                     */
/*                                                                          */
#define ImgX_BADPARAM 166498572
#define ImgX_INVFILTER 166498580
/*                                                                          */
