/***********************************************************************

  yae_menu.c
  

***********************************************************************/

//----------------------------------------------------------------------
//-- Include Files
//----------------------------------------------------------------------
#include "yae_lib.h"

#if SH_ESC_SEQ == 0
#error SH_ESC_SEQ is required to use yae_menu
#endif


//----------------------------------------------------------------------
//-- Type definitions
//----------------------------------------------------------------------
typedef void(*VK_PROCESS)( struct tagSH_HANDLE *shHnd );


//----------------------------------------------------------------------
//-- Prototypes
//----------------------------------------------------------------------
static void WriteMajorItem( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, int i );

//----------------------------------------------------------------------
//-- Controller
//----------------------------------------------------------------------

/***************************************************************
	Name        CallStatusChangedCallback
	Purpose     
***************************************************************/
static void CallStatusChangedCallback( SH_HANDLE *shHnd, MENU_HANDLE *mHnd )
{
	if( mHnd != NULL )
	{
		if( mHnd->Callback_StatusChanged != NULL )
		{
			mHnd->Callback_StatusChanged( shHnd, mHnd );
		}
	}
}

/***************************************************************
	Name        CallValueChangedCallback
	Purpose     
***************************************************************/
static void CallValueChangedCallback( SH_HANDLE *shHnd, MENU_HANDLE *mHnd )
{
	if( mHnd != NULL )
	{
		MENU_ITEM *curItem = &mHnd->Items[ mHnd->Sel ];
		
		if( ( curItem->Type == MENU_DECIMAL ) ||
		    ( curItem->Type == MENU_HEX ) ||
		    ( curItem->Type == MENU_STRING ) )
		{
			if( curItem->CallbackValueChanged != NULL )
			{
				 curItem->CallbackValueChanged( shHnd, mHnd, curItem, curItem->Value );
			}
		}
		
		if( curItem->Type == MENU_SELECTION )
		{
			if( curItem->SubItems[ curItem->Value ].CallbackValueChanged != NULL )
			{
				curItem->SubItems[ curItem->Value ].CallbackValueChanged( shHnd, mHnd, curItem, curItem->Value );
			}
		}
	}
}

/***************************************************************
	Name        ProcessEnter
	Purpose     
***************************************************************/
static void ProcessEnter( struct tagSH_HANDLE *shHnd )
{
	MENU_HANDLE *hnd = shHnd->Menu;
	int sel = hnd->Sel;
	MENU_ITEM *curItem = &hnd->Items[sel];
	
	if( curItem->Callback_Enter != NULL )
	{
		curItem->Callback_Enter( shHnd, hnd, curItem, curItem->Value );
	}
	
	if( ( curItem->SubItems != NULL ) && ( curItem->Value < curItem->SubCount ) )
	{
		if( curItem->SubItems[ curItem->Value ].Callback_Enter != NULL )
		{
			curItem->SubItems[ curItem->Value ].Callback_Enter( shHnd, hnd, &curItem->SubItems[ curItem->Value ], curItem->Value );
		}
	}
	
	CallStatusChangedCallback( shHnd, shHnd->Menu );
}

/***************************************************************
	Name        ProcessBackspce
	Purpose     
***************************************************************/
static void ProcessBackspce( struct tagSH_HANDLE *shHnd )
{
	MENU_HANDLE *mHnd = shHnd->Menu;
	int sel = mHnd->Sel;
	MENU_ITEM *curItem = &mHnd->Items[sel];
	
	if( curItem->Type == MENU_DECIMAL )
	{
		curItem->Value /= 10;
		WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
		CallStatusChangedCallback( shHnd, shHnd->Menu );
		CallValueChangedCallback( shHnd, shHnd->Menu );
	}
	
	if( curItem->Type == MENU_HEX )
	{
		curItem->Value /= 16;
		WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
		CallStatusChangedCallback( shHnd, shHnd->Menu );
		CallValueChangedCallback( shHnd, shHnd->Menu );
	}
	
	if( curItem->Type == MENU_STRING )
	{
		if( curItem->Tmp > 0 )
		{
			curItem->Tmp--;
			curItem->Buf[ curItem->Tmp ] = '\0';
			
			WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
			CallStatusChangedCallback( shHnd, shHnd->Menu );
			CallValueChangedCallback( shHnd, shHnd->Menu );
		}
	}
}

/***************************************************************
	Name        ProcessOtherKey
	Purpose     
***************************************************************/
static void ProcessOtherKey( struct tagSH_HANDLE *shHnd, unsigned char key )
{
	MENU_HANDLE *mHnd = shHnd->Menu;
	int sel = mHnd->Sel;
	MENU_ITEM *curItem = &mHnd->Items[sel];
	int next;
	
	if( curItem->Type == MENU_DECIMAL )
	{
		if( ( key >= '0' ) && ( key <= '9' ) )
		{
			next = curItem->Value * 10;
			
			if( next >= 0 )
			{
				next += misc_char2int( key );
			}
			else
			{
				next -= misc_char2int( key );
			}
			
			if( curItem->Tmp < 0 )
			{
				next *= -1;
			}
			curItem->Tmp = 0;
			curItem->Value = next;
			WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
			CallStatusChangedCallback( shHnd, shHnd->Menu );
			CallValueChangedCallback( shHnd, shHnd->Menu );
		}
		
		if( key == '-' )
		{
			if( curItem->Value == 0 )
			{
				curItem->Tmp = -1;
			}
			else
			{
				curItem->Value *= -1;
				WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
				CallStatusChangedCallback( shHnd, shHnd->Menu );
				CallValueChangedCallback( shHnd, shHnd->Menu );
			}
		}
	}
	
	if( curItem->Type == MENU_HEX )
	{
		if( ( ( key >= '0' ) && ( key <= '9' ) ) ||
		    ( ( key >= 'A' ) && ( key <= 'F' ) ) ||
		    ( ( key >= 'a' ) && ( key <= 'f' ) ) )
		{
			next = curItem->Value * 16 + misc_char2int( key );
			curItem->Value = next;
			WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
			CallStatusChangedCallback( shHnd, shHnd->Menu );
			CallValueChangedCallback( shHnd, shHnd->Menu );
		}
	}
	
	if( ( curItem->Type == MENU_STRING ) && ( key >= 0x20 ) && ( key < 0x7f ) )
	{
		if( curItem->Tmp < curItem->BufLength - 1 )
		{
			curItem->Buf[ curItem->Tmp ] = key;
			curItem->Tmp++;
			curItem->Buf[ curItem->Tmp ] = '\0';
			
			WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
			CallStatusChangedCallback( shHnd, shHnd->Menu );
			CallValueChangedCallback( shHnd, shHnd->Menu );
		}
	}
}

/***************************************************************
	Name        ProcessKey
	Purpose     
***************************************************************/
static void ProcessKey( struct tagSH_HANDLE *shHnd, unsigned char key )
{
	if( key == LINE_BREAK_CODE )
	{
		ProcessEnter( shHnd );
	}
	else if( key == '\b' )
	{
		ProcessBackspce( shHnd );
	}
	else
	{
		ProcessOtherKey( shHnd, key );
	}
}

/***************************************************************
	Name        ProcessUp
	Purpose     
***************************************************************/
static void ProcessUp( struct tagSH_HANDLE *shHnd )
{
	int unsel = shHnd->Menu->Sel;
	int next = shHnd->Menu->Sel;
	
	do {
		next--;
		if( next < 0 )
		{
			next = shHnd->Menu->ItemCount - 1;
		}
	} while( shHnd->Menu->Items[ next ].Type == MENU_CAPTION );
	
	shHnd->Menu->Sel = next;
	
	WriteMajorItem( shHnd, shHnd->Menu, unsel );
	WriteMajorItem( shHnd, shHnd->Menu, next );
	CallStatusChangedCallback( shHnd, shHnd->Menu );
}

/***************************************************************
	Name        ProcessDown
	Purpose     
***************************************************************/
static void ProcessDown( struct tagSH_HANDLE *shHnd )
{
	int unsel = shHnd->Menu->Sel;
	int next = shHnd->Menu->Sel;
	
	do {
		next++;
		if( next >= shHnd->Menu->ItemCount )
		{
			next = 0;
		}
	} while( shHnd->Menu->Items[ next ].Type == MENU_CAPTION );
	
	shHnd->Menu->Sel = next;
	
	WriteMajorItem( shHnd, shHnd->Menu, unsel );
	WriteMajorItem( shHnd, shHnd->Menu, next );
	CallStatusChangedCallback( shHnd, shHnd->Menu );
}

/***************************************************************
	Name        ProcessLeft
	Purpose     
***************************************************************/
static void ProcessLeft( struct tagSH_HANDLE *shHnd )
{
	MENU_HANDLE *mHnd = shHnd->Menu;
	MENU_ITEM *item = &mHnd->Items[ mHnd->Sel ];
	int val = mHnd->Items[ mHnd->Sel ].Value;
	
	if( item->Type == MENU_SELECTION )
	{
		val--;
		if( val < 0 )
		{
			val = item->SubCount - 1;
		}
		mHnd->Items[ mHnd->Sel ].Value = val;
		CallStatusChangedCallback( shHnd, shHnd->Menu );
		CallValueChangedCallback( shHnd, shHnd->Menu );
	}
	
	if( ( item->Type == MENU_DECIMAL ) || ( item->Type == MENU_HEX ) )
	{
		val--;
		mHnd->Items[ mHnd->Sel ].Value = val;
		CallStatusChangedCallback( shHnd, shHnd->Menu );
		CallValueChangedCallback( shHnd, shHnd->Menu );
	}
	
	WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
}

/***************************************************************
	Name        ProcessRight
	Purpose     
***************************************************************/
static void ProcessRight( struct tagSH_HANDLE *shHnd )
{
	MENU_HANDLE *mHnd = shHnd->Menu;
	MENU_ITEM *item = &mHnd->Items[ mHnd->Sel ];
	int val = mHnd->Items[ mHnd->Sel ].Value;
	
	if( mHnd->Items[ mHnd->Sel ].Type == MENU_SELECTION )
	{
		val++;
		if( val >= item->SubCount )
		{
			val = 0;
		}
		mHnd->Items[ mHnd->Sel ].Value = val;
		CallStatusChangedCallback( shHnd, shHnd->Menu );
		CallValueChangedCallback( shHnd, shHnd->Menu );
	}
	
	if( ( item->Type == MENU_DECIMAL ) || ( item->Type == MENU_HEX ) )
	{
		val++;
		mHnd->Items[ mHnd->Sel ].Value = val;
		CallStatusChangedCallback( shHnd, shHnd->Menu );
		CallValueChangedCallback( shHnd, shHnd->Menu );
	}
	
	WriteMajorItem( shHnd, shHnd->Menu, mHnd->Sel );
}

/***************************************************************
	Name        ProcessVirtualKey
	Purpose     
***************************************************************/
struct
{
	SH_VIRTUAL_KEYS Vk;
	VK_PROCESS Func;
} VkTable[] = {
	{ SH_VK_UP,        ProcessUp },
	{ SH_VK_DOWN,      ProcessDown },
	{ SH_VK_LEFT,      ProcessLeft },
	{ SH_VK_RIGHT,     ProcessRight },
	
	{ SH_VK_NONE, NULL }
};

static void ProcessVirtualKey( struct tagSH_HANDLE *shHnd, SH_VIRTUAL_KEYS vk )
{
	int i = 0;
	while( ( VkTable[i].Func != NULL ) && ( VkTable[i].Vk != vk ) )
	{
		i++;
	}
	
	if( VkTable[i].Func != NULL )
	{
		VkTable[i].Func( shHnd );
	}
}

/***************************************************************
	Name        Hook
	Purpose     
***************************************************************/
static bool Hook( struct tagSH_HANDLE *shHnd, unsigned char key, SH_VIRTUAL_KEYS vk )
{
	if( vk == SH_VK_NONE )
	{
		ProcessKey( shHnd, key );
	}
	else
	{
		ProcessVirtualKey( shHnd, vk );
	}
	
	if( ( shHnd->Menu != NULL ) && ( shHnd->Menu->Callback_AnyKey != NULL ) )
	{
		shHnd->Menu->Callback_AnyKey( shHnd, shHnd->Menu, key, vk );
	}
	
	return true;
}

static void WriteSubItems( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, int i )
{
	int j;
	
	if( mHnd->Items[i].Type == MENU_SELECTION )
	{
		j = 0;
		while( mHnd->Items[i].SubItems[j].Caption != NULL )
		{
			if( j == mHnd->Items[i].Value )
			{
				if( i == mHnd->Sel )
				{
					sh_Printf( shHnd, mHnd->Selected );
				}
				else
				{
					sh_Printf( shHnd, mHnd->UnfocusedSel );
				}
			}
			else
			{
				sh_Printf( shHnd, mHnd->Unselected );
			}
			sh_Printf( shHnd, mHnd->Items[i].SubItems[j].Caption );
			sh_Printf( shHnd, CHAR_DEFAULT " " );
			
			j++;
		}
	}
	
	if( mHnd->Items[i].Type == MENU_DECIMAL )
	{
		sh_Printf( shHnd, "%d", mHnd->Items[i].Value );
	}
	
	if( mHnd->Items[i].Type == MENU_HEX )
	{
		sh_Printf( shHnd, "0x%08X", mHnd->Items[i].Value );
	}
	
	if( mHnd->Items[i].Type == MENU_STRING )
	{
		sh_Printf( shHnd, mHnd->Items[i].Buf );
	}
}

static void WriteMajorItem( SH_HANDLE *shHnd, MENU_HANDLE *mHnd, int i )
{
	const char *col = ( mHnd->Sel == i ) ? mHnd->Selected : mHnd->Unselected;
	
	sh_Printf( shHnd, "\x1b[%dH\x1b[G", mHnd->TopPos+i+1 );
	sh_Printf( shHnd, col );
	sh_Printf( shHnd, mHnd->Items[i].Caption );
	
	if( mHnd->Items[i].Type == MENU_CAPTION )
	{
		sh_Printf( shHnd, CHAR_DEFAULT "\r\n" );
	}
	else if( mHnd->Items[i].SubItemPos != 0 )
	{
		sh_Printf( shHnd, "\x1b[%dG" CHAR_DEFAULT, mHnd->Items[i].SubItemPos + 1 );
		WriteSubItems( shHnd, mHnd, i );
		sh_Printf( shHnd, LINE_TERM );
	}
	else
	{
		sh_Printf( shHnd, CHAR_DEFAULT "\r\n" );
	}
}

//----------------------------------------------------------------------
//-- Public functions
//----------------------------------------------------------------------

/***************************************************************
	Name        menu_Initialize
	Purpose     
***************************************************************/
void menu_Initialize( MENU_HANDLE *mHnd, MENU_ITEM *mItem )
{
	mHnd->Unselected = CHAR_DEFAULT;
	mHnd->Selected = FG_BLACK BG_YELLOW;
	mHnd->UnfocusedSel = FG_WHITE BG_GRAY;
	
	mHnd->Items = mItem;
	mHnd->TopPos = 0;
}

/***************************************************************
	Name        menu_Ready
	Purpose     
***************************************************************/
void menu_Ready( SH_HANDLE *shHnd, MENU_HANDLE *mHnd )
{
	int i;
	
	shHnd->Menu = mHnd;
	mHnd->OldHook = shHnd->HookInput;
	shHnd->HookInput = Hook;
	
	// 項目初期化
	menu_InitItems( shHnd, mHnd );
	
	// 選択
	i = 0;
	while( mHnd->Items[i].Type == MENU_CAPTION )
	{
		i++;
	}
	mHnd->Sel = i;
	
	// 初期画面描画
	menu_WriteInitialImage( shHnd, mHnd );
}

void menu_ShowOtherMenu( SH_HANDLE *shHnd, MENU_HANDLE *mHnd )
{
	int i;
	
	shHnd->Menu = mHnd;
	
	// 項目初期化
	menu_InitItems( shHnd, mHnd );
	
	// 選択
	i = 0;
	while( mHnd->Items[i].Type == MENU_CAPTION )
	{
		i++;
	}
	mHnd->Sel = i;
	
	// 初期画面描画
	menu_WriteInitialImage( shHnd, mHnd );
}

void menu_InitItems( SH_HANDLE *shHnd, MENU_HANDLE *mHnd )
{
	int i = 0;
	while( mHnd->Items[i].Caption != NULL )
	{
		if( mHnd->Items[i].Buf != NULL )
		{
			mHnd->Items[i].Buf[0] = '\0';
		}
		mHnd->Items[i].Value = 0;
		
		if( mHnd->Items[i].Initializer != NULL )
		{
			mHnd->Items[i].Value = mHnd->Items[i].Initializer( shHnd, mHnd, &mHnd->Items[i] );
		}
		
		i++;
	}
	mHnd->ItemCount = i;
}

void menu_WriteInitialImage( SH_HANDLE *shHnd, MENU_HANDLE *mHnd )
{
	sh_Printf( shHnd, CLEAR_SCREEN HIDE_CURSOR );
	int i = 0;
	while( mHnd->Items[i].Caption != NULL )
	{
		WriteMajorItem( shHnd, mHnd, i );
		i++;
	}
}

/***************************************************************
	Name        menu_Exit
	Purpose     
***************************************************************/
void menu_Exit( SH_HANDLE *shHnd )
{
	MENU_HANDLE *mHnd = shHnd->Menu;
	
	sh_Printf( shHnd, CLEAR_SCREEN CURSOR_HOME SHOW_CURSOR SH_PROMPT );
	
	shHnd->HookInput = mHnd->OldHook;
	shHnd->Menu = NULL;
}
