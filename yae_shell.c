/***********************************************************************

  yae_shell.c
  組み込みソフトのデバッグ用簡易シェル

***********************************************************************/

//----------------------------------------------------------------------
//-- Include Files
//----------------------------------------------------------------------
#include "yae_lib.h"

//----------------------------------------------------------------------
//-- Macro / definitions
//----------------------------------------------------------------------

#if SH_CSI_RESPONSE != 0
	#define PROMPT  "\x1B[18t" SH_PROMPT
#else
	#define PROMPT  SH_PROMPT
#endif

//----------------------------------------------------------------------
//-- Local Types
//----------------------------------------------------------------------

typedef void(*SH_PROCESS)(SH_HANDLE *);

typedef struct
{
	char Ch;
	SH_PROCESS Func;
} CHAR_PROCESS;

//----------------------------------------------------------------------
//-- Function Prototrypes
//----------------------------------------------------------------------

static void ProcessHome    ( SH_HANDLE *hnd );
static void ProcessInsert  ( SH_HANDLE *hnd );
static void ProcessDelete  ( SH_HANDLE *hnd );
static void ProcessEnd     ( SH_HANDLE *hnd );
static void ProcessPageUp  ( SH_HANDLE *hnd );
static void ProcessPageDown( SH_HANDLE *hnd );
static void ProcessUp      ( SH_HANDLE *hnd );
static void ProcessDown    ( SH_HANDLE *hnd );
static void ProcessRight   ( SH_HANDLE *hnd );
static void ProcessLeft    ( SH_HANDLE *hnd );

static void ProcessEnter    ( SH_HANDLE *hnd );
static void ProcessBackspace( SH_HANDLE *hnd );
static void ProcessEsc      ( SH_HANDLE *hnd );
static void ProcessDelete   ( SH_HANDLE *hnd );
static void ProcessTab      ( SH_HANDLE *hnd );

static void ProcessExtraEsc  ( SH_HANDLE *hnd );
static void ProcessWindowSize( SH_HANDLE *hnd );

static sh_Ready_Entity( SH_HANDLE *hnd );

//----------------------------------------------------------------------
//-- Global Variables
//----------------------------------------------------------------------

#if SH_ESC_SEQ != 0

typedef struct
{
	SH_PROCESS Func;
	unsigned char Opener;
	unsigned char Terminator;
} ESC_ITEM;

typedef struct
{
	SH_PROCESS Func;
	int Arg;
} ESC_EXTRA_ITEM;

static ESC_ITEM EscTable[] = {
	// CSI : \x1b[
	{ ProcessUp,         '[', 'A' },
	{ ProcessDown,       '[', 'B' },
	{ ProcessRight,      '[', 'C' },
	{ ProcessLeft,       '[', 'D' },
	{ ProcessExtraEsc,   '[', '~' },
	{ ProcessWindowSize, '[', 't' }
};

static ESC_EXTRA_ITEM EscExtTable[] = {
	{ ProcessHome,     1 },
	{ ProcessInsert,   2 },
	{ ProcessDelete,   3 },
	{ ProcessEnd,      4 },
	{ ProcessPageUp,   5 },
	{ ProcessPageDown, 6 }
};

#endif

static CHAR_PROCESS CharProcess[] = {
	#if SH_LINE_BREAK == SH_LF
	{ '\n', ProcessEnter },
	#else
	{ '\r', ProcessEnter },
	#endif
	{ '\b', ProcessBackspace },
	{ 0x1b, ProcessEsc },
	{ 0x7f, ProcessDelete },
	{ '\t', ProcessTab },
	
	{ 0x00, NULL }
};

//----------------------------------------------------------------------
//-- Static Functions
//----------------------------------------------------------------------

/***************************************************************
	Name        SkipSeparator
	Purpose     現在の空白文字から次のトークン先頭へ移動
***************************************************************/
static char *SkipSeparator( char *p )
{
	while( ( *p != '\0' ) && ( *p == ' ' ) )
	{
		p++;
	}
	
	return p;
}

/***************************************************************
	Name        NextSeparator
	Purpose     現在位置から次の空白文字へ移動
***************************************************************/
static char *NextSeparator( char *p )
{
	while( ( *p != '\0' ) && ( *p != ' ' ) )
	{
		p++;
	}
	
	return p;
}

#if SH_HISTORY_NUM > 0
/***************************************************************
	Name        MoveToHistoryListTop
	Purpose     対象のヒストリをリストの先頭に移動
***************************************************************/
static void MoveToHistoryListTop( SH_HANDLE *hnd, SH_HISTORY *target )
{
	if( ( target != NULL ) && ( target != hnd->History.Entry.Next ) )
	{
		if( target->Prev != NULL )
		{
			target->Prev->Next = target->Next;
		}
		
		if( target->Next != NULL )
		{
			target->Next->Prev = target->Prev;
		}
		
		target->Prev = NULL;
		target->Next = hnd->History.Entry.Next;
		
		if( hnd->History.Entry.Next != NULL )
		{
			hnd->History.Entry.Next->Prev = target;
		}
		
		hnd->History.Entry.Next = target;
	}
}

/***************************************************************
	Name        SaveHistory
	Purpose     現在の内容をヒストリに退避
***************************************************************/
static void SaveHistory( SH_HANDLE *hnd )
{
	SH_HISTORY *tmpHist;
	tmpHist = hnd->History.Entry.Next;
	hnd->ReadBuf[ hnd->Wp ] = '\0';
	
	// 同一文字列検索
	while( ( tmpHist != NULL ) && ( misc_strcasecmp( tmpHist->Buf, hnd->ReadBuf ) != 0 ) )
	{
		tmpHist = tmpHist->Next;
	}
	
	if( tmpHist != NULL )
	{
		// 入れ替え
		MoveToHistoryListTop( hnd, tmpHist );
	}
	else
	{
		SH_HISTORY *next = NULL;
		
		// 空きを検索
		int i;
		for( i=0; i<SH_HISTORY_NUM; i++ )
		{
			if( hnd->History.Entity[i].Buf[0] == '\0' )
			{
				next = &hnd->History.Entity[i];
				break;
			}
		}
		
		if( next == NULL )
		{
			// 空き無し→末尾を上書きして先頭に移動
			next = hnd->History.Entry.Next;
			while( ( next != NULL ) && ( next->Next != NULL ) )
			{
				next = next->Next;
			}
			
			if( next != NULL )
			{
				MoveToHistoryListTop( hnd, next );
				strcpy( next->Buf, hnd->ReadBuf );
			}
		}
		else
		{
			strcpy( next->Buf, hnd->ReadBuf );
			
			next->Next = hnd->History.Entry.Next;
			next->Prev = NULL;
			
			if( next->Next != NULL )
			{
				next->Next->Prev = next;
			}
			
			hnd->History.Entry.Next = next;
		}
	}
	
	/// debug
	#if 0
	SH_HISTORY *dt = hnd->History.Entry.Next;
	SH_HISTORY *dt2 = NULL;
	while( dt != NULL )
	{
		sh_Printf( hnd, "NextLink: (0x%08x) %s\r\n", dt, dt->Buf );
		dt2 = dt;
		dt = dt->Next;
	}
	
	while( dt2 != NULL )
	{
		sh_Printf( hnd, "PrevLink: (0x%08x) %s\r\n", dt2, dt2->Buf );
		dt2 = dt2->Prev;
	}
	#endif
}
#endif

/***************************************************************
	Name        SplitTokens
	Purpose     トークン分割
***************************************************************/
static int SplitTokens( char *p, char *argv[], int max )
{
	int argc = 0;
	
	while( *p != '\0' )
	{
		p = SkipSeparator( p );
		if( *p != '\0' )
		{
			argv[ argc ] = p;
			argc++;
		}
		
		p = NextSeparator( p );
		if( *p != '\0' )
		{
			*p = '\0';
			p++;
		}
		
		if( argc >= max )
		{
			break;
		}
	}
	
	return argc;
}

/***************************************************************
	Name        Execute
	Purpose     コマンド解析・実行
***************************************************************/
static void Execute( SH_HANDLE *hnd )
{
	char *argv[ SH_MAX_TOKEN ];
	SH_COMMAND func;
		
	int argc = SplitTokens( hnd->ReadBuf, argv, SH_MAX_TOKEN ); // Space is replaced to '\0'
	
	// Search specified command
	int i = 0;
	while( ( hnd->CmdTable[i].Name != NULL ) && ( misc_strcasecmp( hnd->CmdTable[i].Name, argv[0] ) != 0 ) )
	{
		i++;
	}
	func = hnd->CmdTable[i].Func;
	
	// Execute or error
	if( func != NULL )
	{
		func( hnd, argc, argv );
	}
	else
	{
		sh_Printf( hnd, "\x07" FG_L_RED "Unknown command" CHAR_DEFAULT LINE_BREAK );
	}
}

/***************************************************************
	Name        ProcessEscSeq
	Purpose     エスケープシーケンスの判別
***************************************************************/
#if SH_ESC_SEQ != 0
static void ProcessEscSeq( SH_HANDLE *hnd )
{
	unsigned char ch;
	
	while( hnd->Checker() )
	{
		ch = hnd->Reader();
		
		if( hnd->EscOpenChar == '\0' )
		{
			hnd->EscOpenChar = ch;
			hnd->EscParamPos = 0;
			
			int i;
			for( i=0; i<ArraySize(hnd->EscParams); i++ )
			{
				hnd->EscParams[i] = 0;
			}
		}
		else
		{
			if( ( ch >= '0' ) && ( ch <= '9' ) )
			{
				#if 1
				hnd->EscParams[ hnd->EscParamPos ] =
					hnd->EscParams[ hnd->EscParamPos ] * 10 + ( ch - '0' );
				#else
				int tmp = hnd->EscParams[ hnd->EscParamPos ];
				tmp = ( tmp << 4 ) - ( tmp << 2 ) - ( tmp << 1 );
				tmp += ( ch - '0' );
				hnd->EscParams[ hnd->EscParamPos ] = tmp;
				#endif
			}
			else if( ch == ';' )
			{
				hnd->EscParamPos++;
				if( hnd->EscParamPos >= ArraySize(hnd->EscParams) )
				{
					hnd->EscStatus = false;
				}
			}
			else
			{
				hnd->EscStatus = false;
				
				int i;
				for( i=0; i<ArraySize(EscTable); i++ )
				{
					if( EscTable[i].Terminator == ch )
					{
						EscTable[i].Func( hnd );
						break;
					}
				}
			}
		}
	}
}
#endif

/***************************************************************
	Name        ProcessDelete
	Purpose     カーソル位置の文字を削除
***************************************************************/
static void ProcessDelete( SH_HANDLE *hnd )
{
	// sh_Printf( hnd, "Delete\r\n" );
	if( hnd->CurCursor < hnd->Wp )
	{
		int i;
		for( i = hnd->CurCursor; i < hnd->Wp - 1; i++ )
		{
			hnd->ReadBuf[ i ] = hnd->ReadBuf[ i + 1 ];
		}
		hnd->Wp--;
		sh_Printf( hnd, DELETE_CH );
	}
}

#if SH_COMPLETE_ENABLED != 0
/***************************************************************
	Name        SearchComplete
	Purpose     補完で、条件に合致するコマンドを検索する
***************************************************************/
static SH_COMMAND_ELEMENT *SearchComplete( SH_HANDLE *hnd, int length )
{
	SH_COMMAND_ELEMENT *ret = NULL;
	SH_COMMAND_ELEMENT *cur = hnd->Complete.CmdPos;
	
	if( cur == NULL )
	{
		cur = hnd->Complete.CmdPos = hnd->CmdTable;
	}
	
	if( length > 0 )
	{
		do
		{
			// sh_Printf( hnd, "Complete : %s : %s\r\n", hnd->ReadBuf, cur->Name );
			if( misc_strncasecmp( hnd->ReadBuf, cur->Name, length ) == 0 )
			{
				ret = cur;
				break;
			}
			
			cur++;
			if( cur->Name == NULL )
			{
				cur = hnd->CmdTable;
			}
		} while( cur != hnd->Complete.CmdPos );
	}
	else
	{
		ret = cur;
	}
	
	return ret;
}
#endif

/***************************************************************
	Name        ProcessTab
	Purpose     入力補完
***************************************************************/
static void ProcessTab( SH_HANDLE *hnd )
{
	#if SH_COMPLETE_ENABLED != 0
	
	SH_COMMAND_ELEMENT *found = NULL;
	int length;
	
	hnd->ReadBuf[hnd->Wp] = '\0';
	
	if( hnd->Complete.CmdPos == NULL )
	{
		// 新規：現在の入力文字列に合致するコマンドを先頭から検索
		length = strlen( hnd->ReadBuf );
		found = SearchComplete( hnd, length );
		if( found != NULL )
		{
			hnd->Complete.AutoColumn = length;
		}
		else
		{
			hnd->Complete.CmdPos = NULL;
		}
	}
	else
	{
		// 継続
		length = hnd->Complete.AutoColumn;
		found = SearchComplete( hnd, length );
	}
	
	if( found != NULL )
	{
		// 置き換え
		length = strlen( found->Name );
		
		hnd->Complete.CmdPos = found + 1;
		if( hnd->Complete.CmdPos->Name == NULL )
		{
			hnd->Complete.CmdPos = hnd->CmdTable;
		}
		
		hnd->Wp = length;
		hnd->CurCursor = length;
		
		strcpy( hnd->ReadBuf, found->Name );
		
		sh_Printf( hnd, ERASE_LINE "\x1b[0G" SH_PROMPT );
		sh_Printf( hnd, found->Name );
	}
	
	#endif
}

/***************************************************************
	Name        ProcessUp
	Purpose     履歴戻り
***************************************************************/
static void ProcessUp( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		handled = hnd->HookInput( hnd, '\0', SH_VK_UP );
	}
	
	if( handled == false )
	{
		#if SH_HISTORY_NUM > 0
		
		SH_HISTORY *next;
		
		// sh_Printf( hnd, "Up\r\n" );
		if( hnd->History.Current == NULL )
		{
			// ヒストリを表示していない状態なので、
			// 現在の内容をヒストリに退避
			hnd->ReadBuf[ hnd->Wp ] = '\0';
			strcpy( hnd->History.Entry.Buf, hnd->ReadBuf );
			
			next = hnd->History.Entry.Next;
		}
		else
		{
			next = hnd->History.Current->Next;
		}
		
		if( next != NULL )
		{
			hnd->History.Current = next;
			
			strcpy( hnd->ReadBuf, next->Buf );
			hnd->Wp = hnd->CurCursor = strlen( hnd->ReadBuf );
			hnd->ReadBuf[ hnd->Wp ] = '\0';
			sh_Printf( hnd, ERASE_LINE "\x1b[0G" PROMPT "%s", hnd->ReadBuf );
		}
		
		#endif
	}
}

/***************************************************************
	Name        ProcessDown
	Purpose     履歴進み
***************************************************************/
static void ProcessDown( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		handled = hnd->HookInput( hnd, '\0', SH_VK_DOWN );
	}
	
	if( handled == false )
	{
		#if SH_HISTORY_NUM > 0
		
		// sh_Printf( hnd, "Down\r\n" );
		if( hnd->History.Current != NULL )
		{
			if( hnd->History.Current->Prev != NULL )
			{
				strcpy( hnd->ReadBuf, hnd->History.Current->Prev->Buf );
			}
			else
			{
				strcpy( hnd->ReadBuf, hnd->History.Entry.Buf );
			}
			
			hnd->History.Current = hnd->History.Current->Prev;
			hnd->Wp = hnd->CurCursor = strlen( hnd->ReadBuf );
			sh_Printf( hnd, ERASE_LINE "\x1b[0G" PROMPT "%s", hnd->ReadBuf );
		}
		
		#endif
	}
}

/***************************************************************
	Name        ProcessRight
	Purpose     カーソル右
***************************************************************/
static void ProcessRight( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		hnd->HookInput( hnd, '\0', SH_VK_RIGHT );
	}
	
	if( handled == false )
	{
		#if SH_CURSOR_MOVING != 0
		// sh_Printf( hnd, "Right\r\n" );
		if( hnd->CurCursor < hnd->Wp )
		{
			hnd->CurCursor++;
			sh_Printf( hnd, CURSOR_RIGHT );
		}
		#endif
	}
}

/***************************************************************
	Name        ProcessLeft
	Purpose     カーソル左
***************************************************************/
static void ProcessLeft( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		hnd->HookInput( hnd, '\0', SH_VK_LEFT );
	}
	
	if( handled == false )
	{
		#if SH_CURSOR_MOVING != 0
		// sh_Printf( hnd, "Left\r\n" );
		if( hnd->CurCursor > 0 )
		{
			hnd->CurCursor--;
			sh_Printf( hnd, CURSOR_LEFT );
		}
		#endif
	}
}

/***************************************************************
	Name        ProcessHome
	Purpose     カーソル 0 位置へ
***************************************************************/
static void ProcessHome( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		hnd->HookInput( hnd, '\0', SH_VK_HOME );
	}
	
	if( handled == false )
	{
		#if SH_CURSOR_MOVING != 0
		// sh_Printf( hnd, "Home\r\n" );
		if( hnd->CurCursor > 0 )
		{
			sh_Printf( hnd, "\x1b[%dD", hnd->CurCursor );
			hnd->CurCursor = 0;
		}
		#endif
	}
}

/***************************************************************
	Name        ProcessEnd
	Purpose     カーソル終端位置へ
***************************************************************/
static void ProcessEnd( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		hnd->HookInput( hnd, '\0', SH_VK_END );
	}
	
	if( handled == false )
	{
		#if SH_CURSOR_MOVING != 0
		// sh_Printf( hnd, "End\r\n" );
		if( hnd->Wp != hnd->CurCursor )
		{
			sh_Printf( hnd, "\x1b[%dC", hnd->Wp - hnd->CurCursor );
			hnd->CurCursor = hnd->Wp;
		}
		#endif
	}
}

/***************************************************************
	Name        ProcessPageUp
	Purpose     何もしない
***************************************************************/
static void ProcessPageUp( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		hnd->HookInput( hnd, '\0', SH_VK_PAGE_UP );
	}
	
	if( handled == false )
	{
		// sh_Printf( hnd, "P.Up\r\n" );
	}
}

/***************************************************************
	Name        ProcessPageDown
	Purpose     何もしない
***************************************************************/
static void ProcessPageDown( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		hnd->HookInput( hnd, '\0', SH_VK_PAGE_DOWN );
	}
	
	if( handled == false )
	{
		// sh_Printf( hnd, "P.Down\r\n" );
	}
}

/***************************************************************
	Name        ProcessInsert
	Purpose     上書きトグル
***************************************************************/
static void ProcessInsert( SH_HANDLE *hnd )
{
	bool handled = false;
	if( hnd->HookInput != NULL )
	{
		hnd->HookInput( hnd, '\0', SH_VK_INSERT );
	}
	
	if( handled == false )
	{
		#if SH_CURSOR_MOVING != 0
		// sh_Printf( hnd, "Insert\r\n" );
		hnd->Overwrite = !hnd->Overwrite;
		if( hnd->Overwrite )
		{
			sh_Printf( hnd, CURSOR_BLOCK );
		}
		else
		{
			sh_Printf( hnd, CURSOR_U_LINE );
		}
		#endif
	}
}

static void ProcessExtraEsc( SH_HANDLE *hnd )
{
	#if SH_ESC_SEQ != 0
	int i;
	for( i=0; i<ArraySize(EscTable); i++ )
	{
		if( EscExtTable[i].Arg == hnd->EscParams[0] )
		{
			EscExtTable[i].Func( hnd );
			break;
		}
	}
	#endif
}

static void ProcessWindowSize( SH_HANDLE *hnd )
{
	#if SH_CSI_RESPONSE != 0
	if( hnd->EscParams[0] == 8 )
	{
		hnd->TerminalY = hnd->EscParams[1];
		hnd->TerminalX = hnd->EscParams[2];
		
		printf( "TerminalSize = (%d,%d)\n", hnd->TerminalX, hnd->TerminalY );
	}
	#endif
}

/***************************************************************
	Name        ProcessBackspace
	Purpose     カーソルの前の文字を削除
***************************************************************/
static void ProcessBackspace( SH_HANDLE *hnd )
{
	if( hnd->CurCursor > 0 )
	{
		hnd->CurCursor--;
		sh_Printf( hnd, CURSOR_LEFT );
		ProcessDelete( hnd );
	}
}

/***************************************************************
	Name        ProcessEsc
	Purpose     ESCキー単独入力判定
***************************************************************/
static void ProcessEsc( SH_HANDLE *hnd )
{
	// Esc
	#if SH_ESC_SEQ != 0
	hnd->EscDelayCount = SH_ESC_DELAY;
	hnd->EscOpenChar = '\0';
	#endif
}

/***************************************************************
	Name        ProcessEsc_Delayed
	Purpose     ESCキー単独入力判定：ESC入力後少ししてから実行
***************************************************************/
#if SH_ESC_SEQ != 0
static void ProcessEsc_Delayed( SH_HANDLE *hnd )
{
	if( !hnd->Checker() )
	{
		// ESCが単独で入力された場合は全消去
		hnd->Wp = 0;
		hnd->CurCursor = 0;
		
		#if SH_HISTORY_NUM > 0
		hnd->History.Current = NULL;
		#endif
		
		bool handled = false;
		if( hnd->HookInput != NULL )
		{
			handled = hnd->HookInput( hnd, '\0', SH_VK_ESC );
		}
		
		if( handled == false )
		{
			sh_Printf( hnd, ERASE_LINE "\x1b[0G" PROMPT );
		}
	}
	else
	{
		// ESCから別のコードが続く場合はエスケープシーケンス認識
		hnd->EscStatus = true;
		ProcessEscSeq( hnd );
	}
}
#endif

/***************************************************************
	Name        ProcessEnter
	Purpose     コマンド実行
***************************************************************/
static void ProcessEnter( SH_HANDLE *hnd )
{
	sh_Printf( hnd, LINE_BREAK );
	if( hnd->Wp != 0 )
	{
		hnd->ReadBuf[hnd->Wp] = '\0';
		if( *SkipSeparator( hnd->ReadBuf ) != '\0' )
		{
			#if SH_HISTORY_NUM > 0
			SaveHistory( hnd );
			#endif
			
			Execute( hnd );
		}
		hnd->Wp = 0;
	}
	hnd->CurCursor = 0;
	hnd->Overwrite = false;
	
	#if SH_HISTORY_NUM > 0
	hnd->History.Current = NULL;
	#endif
	
	if( hnd->SkipPrompt == false )
	{
		sh_Printf( hnd, CURSOR_U_LINE PROMPT );
	}
	hnd->SkipPrompt = false;
}

/***************************************************************
	Name        ProcessAscii
	Purpose     通常の文字入力
***************************************************************/
static void ProcessAscii( SH_HANDLE *hnd, char val )
{
	#if SH_CURSOR_MOVING != 0
	if( hnd->Wp == hnd->CurCursor )
	#endif
	{
		// 末尾に追加する
		hnd->Writer( val );
		hnd->ReadBuf[hnd->CurCursor] = val;
		hnd->Wp++;
		hnd->CurCursor++;
	}
	#if SH_CURSOR_MOVING != 0
	else
	{
		// 行の中に追加する場合は、上書きモードと挿入モードで異なる動作
		if( hnd->Overwrite == false )
		{
			int i;
			sh_Printf( hnd, BACKSPACE );
			hnd->Writer( val );
			for( i=hnd->Wp; i>=hnd->CurCursor; i-- )
			{
				hnd->ReadBuf[i] = hnd->ReadBuf[i-1];
			}
			
			hnd->ReadBuf[ hnd->CurCursor ] = val;
			hnd->CurCursor++;
			hnd->Wp++;
		}
		else
		{
			hnd->Writer( val );
			hnd->ReadBuf[hnd->CurCursor] = val;
			hnd->CurCursor++;
		}
	}
	#endif
}

/***************************************************************
	Name        ProcessChar
	Purpose     1文字分の処理
***************************************************************/
static bool ProcessChar( SH_HANDLE *hnd, unsigned char val )
{
	bool handled = false;
	
	// 制御文字の判定
	CHAR_PROCESS *p = CharProcess;
	while( p->Ch != 0x00 )
	{
		if( val == p->Ch )
		{
			if( p->Func != NULL )
			{
				p->Func( hnd );
			}
			handled = true;
			break;
		}
		p++;
	}
	
	// 通常の文字のみを処理する
	if( ( handled == false ) && ( val >= 0x20 ) && ( val < 0x80 ) )
	{
		ProcessAscii( hnd, val );
		handled = true;
	}
	
	return handled;
}

/***************************************************************
	Name        ProcessInput
	Purpose     1文字入力
***************************************************************/
static void ProcessInput( SH_HANDLE *hnd, unsigned char val )
{
	bool handled = false;
	
	// フック（上位が入力をかすめ取る）の判定
	if( hnd->HookInput != NULL )
	{
		if( val == 0x1b )
		{
			handled = true;
			ProcessEsc( hnd );
		}
		else
		{
			handled = hnd->HookInput( hnd, val, SH_VK_NONE );
		}
	}
	
	// フックされた場合、trueが返ったらその入力は破棄する
	if( handled == false )
	{
		ProcessChar( hnd, val );
	}
	
	// タブ入力以外の場合は補完の状態をリセットする
	#if SH_COMPLETE_ENABLED != 0
	if( val != '\t' )
	{
		hnd->Complete.CmdPos = NULL;
		hnd->Complete.AutoColumn = 0;
	}
	#endif
}

#if SH_MON_ENABLED != 0
/***************************************************************
	Name        WriteMonitorLabel
	Purpose     モニタ機能：ラベル出力
***************************************************************/
static int WriteAndPositionMonitorLabel( SH_HANDLE *hnd )
{
	int CurRow = 0;
	int i = 0;
	SH_MON_ITEM *mon = hnd->Monitor.TopItem;
	while( ( mon[i].Label != NULL ) || ( mon[i].Writer != NULL ) )
	{
		mon[i].Lb_x  = ( mon[i].Lb_x  < 0 ) ? 0 : mon[i].Lb_x;
		mon[i].Lb_y  = ( mon[i].Lb_y  < 0 ) ? CurRow      : mon[i].Lb_y;
		mon[i].Val_y = ( mon[i].Val_y < 0 ) ? mon[i].Lb_y : mon[i].Val_y;
		
		CurRow = mon[i].Val_y + 1;
		
		if( ( mon[i].Val_x < 0 ) && ( mon[i].Label != NULL ) )
		{
			mon[i].Val_x = mon[i].Lb_x + strlen( mon[i].Label );
		}
		
		if( mon[i].Label != NULL )
		{
			sh_Printf( hnd, "\x1b[%d;%dH", mon[i].Lb_y + 1, mon[i].Lb_x + 1 );
			sh_Printf( hnd, mon[i].Label, mon[i].Param );
		}
		
		i++;
	}
	
	return CurRow;
}

static void WriteMonitorLabel( SH_HANDLE *hnd )
{
	int i = 0;
	SH_MON_ITEM *mon = hnd->Monitor.TopItem;
	while( ( mon[i].Label != NULL ) || ( mon[i].Writer != NULL ) )
	{
		if( mon[i].Label != NULL )
		{
			sh_Printf( hnd, "\x1b[%d;%dH", mon[i].Lb_y + 1, mon[i].Lb_x + 1 );
			sh_Printf( hnd, mon[i].Label, mon[i].Param );
		}
		
		i++;
	}
}

/***************************************************************
	Name        TickMonitor
	Purpose     モニタ機能：定期的に実行
***************************************************************/
static TickMonitor( SH_HANDLE *hnd )
{
	SH_MON_HANDLE *mon = &hnd->Monitor;
	
	if( mon->CurItem != NULL )
	{
		// ラベルを再出力
		if( mon->LabelInterval > 0 )
		{
			sh_Printf( hnd, SAVE_CURSOR HIDE_CURSOR );
			
			mon->LabelCount++;
			if( mon->LabelCount >= mon->LabelInterval )
			{
				WriteMonitorLabel( hnd );
				mon->LabelCount = 0;
			}
			
			if( hnd->Menu == NULL )
			{
				sh_Printf( hnd, RESTORE_CURSOR SHOW_CURSOR );
			}
			else
			{
				sh_Printf( hnd, RESTORE_CURSOR );
			}
		}
		
		// データを出力
		mon->UpdateCount++;
		if( mon->UpdateCount >= mon->UpdateInterval )
		{
			sh_Printf( hnd, SAVE_CURSOR HIDE_CURSOR );
			
			SH_MON_ITEM *cur = mon->CurItem;
			
			// 同じバンド番号の項目はまとめて出力する
			int baseBand = mon->CurItem->Band;
			do
			{
				if( cur->Writer != NULL )
				{
					sh_Printf( hnd, "\x1b[%d;%dH", cur->Val_y + 1, cur->Val_x + 1 );
					cur->Writer( hnd, cur->Param );
				}
				
				cur++;
				if( ( cur->Label == NULL ) && ( cur->Writer == NULL ) )
				{
					// 一周したらバンド番号に関わらず中断
					cur = mon->TopItem;
					break;
				}
			} while( baseBand == cur->Band );
			
			if( hnd->Menu == NULL )
			{
				sh_Printf( hnd, RESTORE_CURSOR SHOW_CURSOR );
			}
			else
			{
				sh_Printf( hnd, RESTORE_CURSOR );
			}
			
			mon->CurItem = cur;
			mon->UpdateCount = 0;
		}
	}
}
#endif

static void InitHandleElement( SH_HANDLE *hnd )
{
	hnd->Wp = 0;
	hnd->CurCursor = 0;
	hnd->Overwrite = 0;
	hnd->Tick = NULL;
	hnd->SkipPrompt = false;
	
	#if SH_HISTORY_NUM > 0
	hnd->History.Count = 0;
	hnd->History.Current = NULL;
	hnd->History.Entry.Prev = NULL;
	hnd->History.Entry.Next = NULL;
	
	int i;
	for( i=0; i<SH_HISTORY_NUM; i++ )
	{
		hnd->History.Entity[i].Buf[0] = '\0';
	}
	#endif
	
	#if SH_COMPLETE_ENABLED != 0
	hnd->Complete.CmdPos = hnd->CmdTable;
	hnd->Complete.AutoColumn = 0;
	#endif
	
	#if SH_MON_ENABLED != 0
	hnd->Monitor.CurItem = NULL;
	hnd->Monitor.TopItem = NULL;
	#endif
	
	#if SH_ESC_SEQ != 0
		hnd->EscParamPos = 0;
		hnd->EscDelayCount = 0;
		hnd->EscStatus = false;
	#endif
	
	#if SH_CSI_RESPONSE != 0
		hnd->TerminalX = 0;
		hnd->TerminalY = 0;
	#endif
	
	#if SH_BOOT_KEY_EN != 0
		hnd->Boot = false;
	#endif
}

/***************************************************************
	Name        InitHandle
	Purpose     ハンドルを初期化する
***************************************************************/
static void InitHandle( SH_HANDLE *hnd, SH_ACTION ini, SH_READER r, SH_WRITER w, SH_READ_CHECKER rc )
{
	hnd->Initializer = ini;
	hnd->Reader = r;
	hnd->Writer = w;
	hnd->Checker = rc;
	
	InitHandleElement( hnd );
}

//----------------------------------------------------------------------
//-- Public Functions : shell controller
//----------------------------------------------------------------------

/***************************************************************
	Name        sh_Open
	Purpose     ハンドルを指定して、準備する
***************************************************************/
void sh_Open( SH_HANDLE *hnd, SH_COMMAND_ELEMENT *c, SH_ACTION ini, SH_READER r, SH_WRITER w, SH_READ_CHECKER rc )
{
	hnd->CmdTable = c;
	InitHandle( hnd, ini, r, w, rc );
}

/***************************************************************
	Name        sh_Clone
	Purpose     指定ハンドルの使用を終了する
***************************************************************/
void sh_Clone( const SH_HANDLE *src, SH_HANDLE *dst, SH_ACTION ini, SH_READER r, SH_WRITER w, SH_READ_CHECKER rc )
{
	*dst = *src;
	InitHandle( dst, ini, r, w, rc );
}

/***************************************************************
	Name        sh_Tick
	Purpose     上位は、この関数をハンドルごとに定期的に実行しなければならない
***************************************************************/
void sh_Tick( SH_HANDLE *hnd )
{
	bool handled = false;
	
	#if SH_BOOT_KEY_EN != 0
	
	if( hnd->Boot == false )
	{
		char ch;
		while( hnd->Checker() )
		{
			ch = hnd->Reader();
			if( ch == SH_BOOT_KEY[ hnd->Wp ] )
			{
				hnd->Wp++;
				if( SH_BOOT_KEY[ hnd->Wp ] == '\0' )
				{
					sh_Ready_Entity( hnd );
					hnd->Wp = 0;
					hnd->Boot = true;
				}
			}
			else
			{
				hnd->Wp = 0;
			}
		}
		
		handled = true;
	}
	
	#endif
	
	// Tickコールバックが登録されている場合
	if( ( handled == false ) && ( hnd->Tick != NULL ) )
	{
		handled = hnd->Tick( hnd );
	}
	
	// Tickコールバックがtrueを返した場合は、これ以上処理をするなと言う意味
	if( handled == false )
	{
		#if SH_MON_ENABLED != 0
		if( hnd->Monitor.CurItem != NULL )
		{
			TickMonitor( hnd );
		}
		#endif
		
		#if SH_ESC_SEQ != 0
		if( hnd->EscDelayCount > 0 )
		{
			// ESC入力後、少ししてからエスケープシーケンス判定
			hnd->EscDelayCount--;
			if( hnd->EscDelayCount == 0 )
			{
				ProcessEsc_Delayed( hnd );
			}
		}
		else if( hnd->EscStatus )
		{
			ProcessEscSeq( hnd );
		}
		else
		#endif
		{
			// 入力が続く限り処理
			while( hnd->Checker() )
			{
				ProcessInput( hnd, hnd->Reader() );
				if( hnd->EscDelayCount > 0 )
				{
					break;
				}
			}
		}
	}
}

/***************************************************************
	Name        sh_Ready
	Purpose     指定ハンドルを実行可能な状態にする
***************************************************************/
static sh_Ready_Entity( SH_HANDLE *hnd )
{
	InitHandleElement( hnd );
	
	if( hnd->Initializer != NULL )
	{
		hnd->Initializer( hnd );
	}
	
	sh_Printf( hnd, PROMPT );
}

void sh_Ready( SH_HANDLE *hnd )
{
	sh_Printf( hnd, TERMINAL_INITIALIZER );
	sh_Printf( hnd, SHOW_CURSOR CURSOR_U_LINE );
	
	#if SH_BOOT_KEY_EN == 0
	sh_Ready_Entity( hnd );
	#endif
}

/***************************************************************
	Name        sh_Prompt
	Purpose     プロンプトを表示する
***************************************************************/
void sh_Prompt( SH_HANDLE *hnd )
{
	sh_Printf( hnd, SHOW_CURSOR CURSOR_U_LINE PROMPT );
}

/***************************************************************
	Name        sh_Printf
	Purpose     ハンドルに関連付けられた出力に対して、文字列出力
***************************************************************/
int sh_Printf( SH_HANDLE *hnd, const char *fmt, ... )
{
	int i;
	va_list va;
	
	va_start( va, fmt );
	
	vsprintf( hnd->PrintBuf, fmt, va );
	
	i = 0;
	while( hnd->PrintBuf[i] != '\0' )
	{
		hnd->Writer( hnd->PrintBuf[i] );
		i++;
	}
	
	va_end( va );
	
	return i;
}

/***************************************************************
	Name        sh_RegisterTick
	Purpose     シェル側から定期的に実行する関数を登録
***************************************************************/
void sh_RegisterTick( SH_HANDLE *hnd, SH_TICK func )
{
	// sh_Tickのタイミングで実行するコールバックで、ユーザーが定義する
	//  typedef bool(*SH_TICK)(struct tagSH_HANDLE *);
	// 通常は false を返すように実装する
	// trueを返した場合、そのTickでのシェル側の処理を行わない
	hnd->Tick = func;
}

/***************************************************************
	Name        sh_RegisterHookInput
	Purpose     入力のかすめ取り関数を登録
***************************************************************/
SH_HOOK_INPUT sh_RegisterHookInput( SH_HANDLE *hnd, SH_HOOK_INPUT func )
{
	// 1文字入力されたタイミングで実行するコールバックで、ユーザーが定義する
	//   typedef bool(*SH_HOOK_INPUT)(struct tagSH_HANDLE *, unsigned char);
	// 通常は false を返すように実装する
	// trueを返した場合、入力された文字を完全にかすめ取って、シェルはその入力を処理しない
	SH_HOOK_INPUT ret = hnd->HookInput;
	hnd->HookInput = func;
	return ret;
}

#if SH_MON_ENABLED != 0

// Tickコールバックやコマンドから変更することを前提とし
// シェルの管理外から設定されることは想定しない
// →対応しようとすると、OSの補助などが必要になる

/***************************************************************
	Name        sh_ToggleMonitor
	Purpose     モニタ機能を ON / OFF トグルする
***************************************************************/
void sh_ToggleMonitor( SH_HANDLE *hnd, SH_MON_ITEM *mon, int updateInt, int labelInt )
{
	if( hnd->Monitor.TopItem == NULL )
	{
		// 現在の表示はすべてクリア
		sh_Printf( hnd, ERASE_ALL HIDE_CURSOR );
		sh_Printf( hnd, CURSOR_HOME );
		
		hnd->Monitor.TopItem = hnd->Monitor.CurItem = mon;
		hnd->Monitor.UpdateCount = 0;
		hnd->Monitor.LabelCount = 0;
		hnd->Monitor.UpdateInterval = updateInt;
		hnd->Monitor.LabelInterval = labelInt;
		
		hnd->Monitor.BottomRow = WriteAndPositionMonitorLabel( hnd );
		
		sh_Printf( hnd, LINE_TERM "\x1b[%dr\x1b[%d;1H", hnd->Monitor.BottomRow + 1, hnd->Monitor.BottomRow + 1 );
	}
	else
	{
		hnd->Monitor.TopItem = hnd->Monitor.CurItem = NULL;
		sh_Printf( hnd, SAVE_CURSOR "\x1b[r" RESTORE_CURSOR, hnd->Monitor.BottomRow );
	}
}

#endif

void sh_SetWindowTitle( SH_HANDLE *hnd, char *s )
{
	sh_Printf( hnd, "\x1b]0;%s\x07", s );
}
