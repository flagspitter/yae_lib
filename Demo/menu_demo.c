/***********************************************************************

  menu_demo.c
  

***********************************************************************/

//----------------------------------------------------------------------
//-- Include Files
//----------------------------------------------------------------------
#include "yae_lib.h"

//----------------------------------------------------------------------
//-- Global variables
//----------------------------------------------------------------------
static unsigned char BufString[16] = {0};
static MENU_HANDLE MenuHandle;

//----------------------------------------------------------------------
//-- Callback Functions
//----------------------------------------------------------------------

/***************************************************************
	Name        FuncEnter
	Purpose     "Enterで実行" の項目でEnterが押された
***************************************************************/
static void FuncEnter( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "FuncEnter\n" );
}

/***************************************************************
	Name        FuncDecimal
	Purpose     "10進数入力" の項目でEnterが押された
***************************************************************/
static void FuncDecimal( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "FuncDecimal\n" );
}

/***************************************************************
	Name        FuncHex
	Purpose     "16進数入力" の項目でEnterが押された
***************************************************************/
static void FuncHex( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "FuncHex\n" );
}

/***************************************************************
	Name        FuncString
	Purpose     "文字列入力" の項目でEnterが押された
***************************************************************/
static void FuncString( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "FuncString\n" );
}

/***************************************************************
	Name        FuncSubItem
	Purpose     "選択する項目" の項目で(選択肢に関わらず)Enterが押された
***************************************************************/
static void FuncSubItem( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "%s\n", item->Caption );
}

/***************************************************************
	Name        ExitMenu
	Purpose     "終了する" でEnter → メニューを終了する
***************************************************************/
static void ExitMenu( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	menu_Exit( shHnd );
}

/***************************************************************
	Name        KeyCallback
	Purpose     選択に関わらず、何らかのキーが押された
***************************************************************/
static void KeyCallback( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, unsigned char key, SH_VIRTUAL_KEYS vk )
{
	printf( "Key %d, %d\n", key, vk );
}

/***************************************************************
	Name        TransitCallback
	Purpose     選択状態や値等、何らかの状態が変化した
***************************************************************/
static void TransitCallback( SH_HANDLE *shHnd, MENU_HANDLE *mHnd )
{
	printf( "Transit\n" );
}

/***************************************************************
	Name        VcSel1
	Purpose     "選択肢[1]"が選択された
***************************************************************/
static void VcSel1( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "->選択肢[1] %d\n", val );
}

/***************************************************************
	Name        VcSel2
	Purpose     "選択肢[2]"が選択された
***************************************************************/
static void VcSel2( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "->選択肢[2] %d\n", val );
}

/***************************************************************
	Name        VcSel3
	Purpose     "選択肢[3]"が選択された
***************************************************************/
static void VcSel3( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "->選択肢[3] %d\n", val );
}

/***************************************************************
	Name        VcDec
	Purpose     "10進数入力"の値が変化した
***************************************************************/
static void VcDec( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "->DEC %d\n", val );
}

/***************************************************************
	Name        VcHex
	Purpose     "16進数入力"の値が変化した
***************************************************************/
static void VcHex( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "->HEX %d\n", val );
}

/***************************************************************
	Name        VcStr
	Purpose     "文字列入力"の入力内容が変化した
***************************************************************/
static void VcStr( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item, int val )
{
	printf( "->STR %s\n", item->Buf );
}

static int InitSub( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, MENU_ITEM *item )
{
	return 1;
}

//----------------------------------------------------------------------
//-- Menu Definition
//----------------------------------------------------------------------

static MENU_ITEM SubItems[] = {
	MENU_TEMPLATE_SUBITEM( "選択肢[1]", FuncSubItem, VcSel1,  NULL ),
	MENU_TEMPLATE_SUBITEM( "選択肢[2]", FuncSubItem, VcSel2,  NULL ),
	MENU_TEMPLATE_SUBITEM( "選択肢[3]", FuncSubItem, VcSel3,  NULL ),
	MENU_ITME_END
};

#define SUBITEM_POS 17
static MENU_ITEM Items[] = {
	MENU_TEMPLATE_CAPTION  ( "<<< Menu Demo >>>", NULL ),
	MENU_TEMPLATE_CAPTION  ( "-------------------------------------------------------------------", NULL ),
	MENU_TEMPLATE_ENTER    ( " Enterで実行    ", FuncEnter, NULL ),
	MENU_TEMPLATE_CAPTION  ( "-------------------------------------------------------------------", NULL ),
	MENU_TEMPLATE_SELECTION( " 選択する項目 : ", InitSub, SubItems,    SUBITEM_POS, NULL ),
	MENU_TEMPLATE_DECIMAL  ( " 10進数入力   : ", NULL, FuncDecimal, VcDec, SUBITEM_POS, NULL ),
	MENU_TEMPLATE_HEX      ( " 16進数入力   : ", NULL, FuncHex,     VcHex, SUBITEM_POS, NULL ),
	MENU_TEMPLATE_STRING   ( " 文字列入力   : ", NULL, FuncString,  VcStr, SUBITEM_POS, BufString, NULL ),
	MENU_TEMPLATE_CAPTION  ( "-------------------------------------------------------------------", NULL ),
	MENU_TEMPLATE_ENTER    ( " 終了する       ", ExitMenu, NULL ),
	MENU_TEMPLATE_CAPTION  ( "-------------------------------------------------------------------", NULL ),
	MENU_ITME_END
};

//----------------------------------------------------------------------
//-- Entry
//----------------------------------------------------------------------

void EnterMenu( SH_HANDLE *shHnd )
{
	menu_Initialize( &MenuHandle, Items );
	
	MenuHandle.Callback_AnyKey = KeyCallback;
	MenuHandle.Callback_StatusChanged = TransitCallback;
	
	menu_Ready( shHnd, &MenuHandle );
}
