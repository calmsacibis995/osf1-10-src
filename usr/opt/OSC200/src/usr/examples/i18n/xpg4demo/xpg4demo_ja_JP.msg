$ 
$ *****************************************************************
$ *                                                               *
$ *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
$ *                                                               *
$ *   All Rights Reserved.  Unpublished rights  reserved  under   *
$ *   the copyright laws of the United States.                    *
$ *                                                               *
$ *   The software contained on this media  is  proprietary  to   *
$ *   and  embodies  the  confidential  technology  of  Digital   *
$ *   Equipment Corporation.  Possession, use,  duplication  or   *
$ *   dissemination of the software and media is authorized only  *
$ *   pursuant to a valid written license from Digital Equipment  *
$ *   Corporation.                                                *
$ *                                                               *
$ *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
$ *   by the U.S. Government is subject to restrictions  as  set  *
$ *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
$ *   or  in  FAR 52.227-19, as applicable.                       *
$ *                                                               *
$ *****************************************************************
$
$ HISTORY
$ 
$ @(#)$RCSfile: xpg4demo_ja_JP.msg,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/25 22:04:33 $
$ 
$ /*
$  * XPG4 demo program message catalogue
$  *
$  */

$quote "

$set MSGError

E_COM_EXISTBADGE	"社員番号 %ld は既に存在します"
E_COM_FINDBADGE		"社員番号 %ld が見つかりません"
E_COM_INPUT		"入力できません"
E_COM_MODIFY		"変更するエントリが存在しません"
E_COM_NOENT		"エントリが存在しません"
E_COM_NOTDEL		"削除するエントリが存在しません"
E_COM_UNKNOWN		"未定義コマンドです"
E_DB_NOMEMORY		"社員データ"
E_DB_WRITEOPEN		"出力ファイル %s がオープンできません"
E_MES_ABORT		". 強制終了します。\n"
E_MES_FATAL2		". 異常終了します\n"
E_MES_NOMEMORY		"%s 用メモリがありません"
E_SCR_INV_BADGE		"社員番号が正しくありません"
E_SCR_INV_CC		"コストセンターが正しくありません"
E_SCR_INV_JOD		"日付が正しくありません"
E_SCR_2LONG_1STNM	"名前が長すぎます"
E_SCR_NOMEMORY		"スクリーン構造体"
E_SCR_2LONG_SURNM	"姓名が長すぎます"
E_SCR_UNKOWN_TERM	"ターミナル名が分かりません. \"ansi\" で行います"
E_SCR_TERMCAP_OPEN	"termcap/terminfo データベースがオープンできません"
E_SCR_TERMNOMEM		"termcap バッファ"
E_SCR_UNDEFTERM		"ターミナル・タイプ %s が定義されていません"
E_UTL_NOMEMORY		"メッセージ・データ"
E_XPG_DBREAD		"データベースは読み取り専用です。出力ファイルは作れません"
E_XPG_INIT_COM		"コマンド環境を初期化できません"
E_XPG_INIT_SCR		"スクリーンの初期化ができません"
E_XPG_NEWFILE		"新規ファイル"
E_XPG_NOMEMORY		"メモリが足りません"
E_XPG_READ		"社員データベース %s が読めません"
E_XPG_USAGE		"使用方法： %s [社員データベース名]\n"
E_XPG_WRITE		"データベースに書き込むことができませんでした"

$set MSGInfo

I_COM_NEWEMP		"新規登録"
I_COM_YN_DELETE		"表示中のデータを削除しますか？"
I_COM_YN_MODIFY		"表示中のデータを変更しますか？"
I_COM_YN_REPLACE	"表示中のデータに変更しますか？"
I_COM_ASK_MOD_FMT	"変更するデータの%sを入力してください。[省略時 %ld]: "
I_COM_ASK_DEL_FMT	"削除するデータの%sを入力してください。[省略時 %ld]: "
I_COM_DISP_LIST_FMT	"%1$7ld  %3$30S %2$-20S  %4$3S  %5$10s\n"
$ I_COM_MOD_BADGE_FMT	""
I_DB_SURNAME1ST		"y"
I_MES_YN		" (y/n) [y]: "
I_XPG_CONTINUE		"Return キーを押してください"
I_SCR_PROMPT_BADGE_DEF	"%s [省略時 %ld]? "
I_SCR_PROMPT_BADGE	"%s? "
I_SCR_PROMPT_NAME_DEF	"%s [省略時 %S]? "
I_SCR_PROMPT_NAME	"%s? "
I_SCR_PROMPT_CC_DEF	"%s [省略時 %S]? "
I_SCR_PROMPT_CC		"%s? "
I_SCR_PROMPT_DOJ_DEF	"%s [省略時 %s]? "
I_SCR_PROMPT_DOJ	"%s? "
I_SCR_IN_DATE_FMT	"%1$d/%2$d/%3$d"

$set MSGString

S_COM_CREATE		"c"
S_COM_DELETE		"d"
S_COM_EXIT		"e"
S_COM_LIST		"l"
S_COM_MODIFY		"m"
S_COM_LIST_TITLE	"社員番号               姓名               名前              ｺｽﾄｾﾝﾀ-   入社日\n"
S_COM_LIST_LINE		"───────────────────────────────────────\n"
S_SCR_BADGE		"社員番号"
S_SCR_CC		"コストセンター"
S_SCR_DOJ		"入社日"
S_SCR_DOJ_FMT		"入社日 (年/月/日)"
S_SCR_MENU		"1-変更(M) 2-新規登録(C) 3-削除(D) 4-一覧(L) 5-終了(E) :",
S_SCR_NAME		"名前"
S_SCR_SNAME1ST		"y"
S_SCR_SURNAME		"姓名"
S_SCR_PROMPT		""
S_DATE_FMT		"%x"
S_SCR_DATE_FMT		"%x"
