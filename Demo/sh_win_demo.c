#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <stdbool.h>

#include "yae_lib.h"

static void InitShell( void );
static void MainLoop( void );

//----------------------------------------------------------------------
//-- RS-232C example
//----------------------------------------------------------------------

static HANDLE ComPort;
static char ReadBuf[1024];
static int  Rp;
static int  Wp;

/***************************************************************
	Name        InitCom
	Purpose     COMポートの初期化
***************************************************************/
static int InitCom( const char *port )
{
	int ret = 0;
	ComPort = CreateFile(_T(port), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL); // シリアルポートを開く
	
	DCB dcb; // シリアルポートの構成情報が入る構造体
	GetCommState( ComPort, &dcb ); // 現在の設定値を読み込み
	
	dcb.BaudRate     = 115200; // 速度
	dcb.ByteSize     = 8; // データ長
	dcb.Parity       = NOPARITY; // パリティ
	dcb.StopBits     = ONESTOPBIT; // ストップビット長
	dcb.fOutxCtsFlow = false; // 送信時CTSフロー
	dcb.fRtsControl  = RTS_CONTROL_ENABLE; // RTSフロー
	
	SetCommState(ComPort, &dcb); // 変更した設定値を書き込み
	
	if( ComPort == INVALID_HANDLE_VALUE)
	{
		ret = -1;
	}
	
	return ret;
}

/***************************************************************
	Name        Tick_ComPort
	Purpose     定期的に実行して読み込む
***************************************************************/
static void Tick_ComPort( void )
{
	int err;
	COMSTAT st;
	ClearCommError(ComPort, &err, &st);
	int rcvSize = st.cbInQue; // 受信したメッセージ長を取得する
	
	// printf( "Rcv %d\n",rcvSize );
	
	int i;
	int read;
	for( i=0; i<rcvSize; i++ )
	{
		ReadFile( ComPort, &ReadBuf[ Wp ], 1, &read, NULL );
		
		if( read == 0 )
		{
			break;
		}
		// printf( "Fetch 0x%02X\n", ReadBuf[Wp] );
		
		Wp++;
		if( Wp >= sizeof(ReadBuf) )
		{
			Wp = 0;
		}
	}
}

/***************************************************************
	Name        IsDataAvailable
	Purpose     シェルに関数ポインタを渡す：データ有無を確認
***************************************************************/
static bool IsDataAvailable( void )
{
	return ( Rp != Wp ) ? true : false;
}

/***************************************************************
	Name        ReadByte
	Purpose     シェルに関数ポインタを渡す：1バイト読み込み
***************************************************************/
static char ReadByte( void )
{
	char ret = '\0';
	
	if( Rp != Wp )
	{
		ret = ReadBuf[ Rp ];
		// printf( "Read  0x%02X\n", ret );
		Rp++;
		if( Rp >= sizeof(ReadBuf) )
		{
			Rp = 0;
		}
	}
	
	return ret;
}

/***************************************************************
	Name        WriteByte
	Purpose     シェルに関数ポインタを渡す：1バイト書き込み
***************************************************************/
static void WriteByte( char ch )
{
	int written;
	WriteFile( ComPort, &ch, 1, &written, NULL ); // ポートへ送信
	// printf( "Write 0x%02X\n", ch );
}

//----------------------------------------------------------------------
//-- Entry
//----------------------------------------------------------------------

/***************************************************************
	Name        main
	Purpose     
***************************************************************/
int main( int argc, char *argv[] )
{
	char tmp[256];
	
	strcpy( tmp, "\\\\.\\" );
	strcat( tmp, argv[1] );
	
	if( InitCom( tmp ) != 0 )
	{
		printf( "Handle Error\n" );
	}
	else
	{
		// Flash input
		Tick_ComPort();
		while( IsDataAvailable() )
		{
			ReadByte();
		}
		
		MainLoop();
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//-- Demo

//----------------------------------------------------------------------
//---- Prototypes
//----------------------------------------------------------------------
static void Initializer( SH_HANDLE *hnd );

//----------------------------------------------------------------------
//---- Global objects : Shell handle
//----------------------------------------------------------------------
static SH_HANDLE shHnd;

//----------------------------------------------------------------------
//---- Command functions
//----------------------------------------------------------------------
static void cmd_Hello( SH_HANDLE *hnd, int argc, char *argv[] )
{
	sh_Printf( hnd, "Hello,world\n" );
}

static void cmd_Exit( SH_HANDLE *hnd, int argc, char *argv[] )
{
	sh_Printf( hnd, "Bye\n" );
	exit(0);
}

extern void EnterMenu( SH_HANDLE *shHnd );
static void cmd_Dummy( SH_HANDLE *hnd, int argc, char *argv[] )
{
	EnterMenu( hnd );
}

static void cmd_LongName( SH_HANDLE *hnd, int argc, char *argv[] )
{
	// To input long command name, tab key is useful.
	sh_Printf( hnd, "%d\n", strlen( argv[0] ) );
}

static void cmd_TerminalSize( SH_HANDLE *hnd, int argc, char *argv[] )
{
	// sh_Printf( hnd, "%d, %d\n", hnd->TerminalX, hnd->TerminalY );
}

extern void UnitTest( SH_HANDLE *hnd );
static void cmd_UnitTest( SH_HANDLE *hnd, int argc, char *argv[] )
{
	UnitTest( hnd );
}

static void cmd_Monitor( SH_HANDLE *hnd, int argc, char *argv[] );

//----------------------------------------------------------------------
//---- Command Registration Table
//----------------------------------------------------------------------
static SH_COMMAND_ELEMENT CmdTable[] = {
	{ "hello",  cmd_Hello },
	{ "exit",   cmd_Exit },
	{ "bye",    cmd_Exit },
	{ "test1",  cmd_Dummy },
	{ "test2",  cmd_Dummy },
	{ "ts",     cmd_TerminalSize },
	{ "answer_to_life_the_universe_and_everything",  cmd_LongName },
	{ "unittest", cmd_UnitTest },
	
	{ "mon",    cmd_Monitor },
	
	{ NULL, NULL }
};

//----------------------------------------------------------------------
//---- Functions
//----------------------------------------------------------------------

/***************************************************************
	Name        MainLoop
	Purpose     最低限このように実行すればよい
***************************************************************/
static void MainLoop( void )
{
	// (1) Openする
	// 必要な関数をそれぞれ指定する
	sh_Open( &shHnd, CmdTable, Initializer, ReadByte, WriteByte, IsDataAvailable );
	
	// (2) Ready
	// プロンプトが表示され、初期化時の関数が実行される
	// 例えば、TCP/IP接続時に実行するなどして、Openとは別タイミングで実行しても良い
	sh_Ready( &shHnd );
	
	// プログラム全体のメインループでも良いし、
	// 何らかのタスクでも良い
	while(1)
	{
		// (3) Tick関数を定期的に実行する
		sh_Tick( &shHnd );
		
		// これは通信の例なので、通信の実装形態によって必要かもしれない程度
		Tick_ComPort();
		
		// 実行間隔は、OSなどの実装形態に依存する
		Sleep(1);
	}
}

/***************************************************************
	Name        Initializer
	Purpose     シェル起動時に実行される
***************************************************************/
static void Initializer( SH_HANDLE *hnd )
{
	sh_SetWindowTitle( hnd, SH_INFORMATION " Sample implementation" );
	// sh_Printf( hnd, "\n [ " SH_INFORMATION " ]\n  Sample implementation\n\n" );
}


//----------------------------------------------------------------------
//---- Monitor function
//----------------------------------------------------------------------

#if SH_MON_ENABLED != 0
static void mon_Heartbeat( SH_HANDLE *hnd, unsigned int param )
{
	static int count;
	const char rc[] = { '/', '-', '\\', '|' };
	count++;
	count &= 0x03;
	sh_Printf( hnd, " %c", rc[count] );
}

static void mon_Test1( SH_HANDLE *hnd, unsigned int param )
{
	static int i;
	sh_Printf( hnd, "%d,%d", param, i++ );
}

static void mon_Test2( SH_HANDLE *hnd, unsigned int param )
{
	static int i;
	sh_Printf( hnd, "%d", i+=2 );
}

static SH_MON_ITEM MonitorItems[] = {
	// Writer          Label   Value  Param      Band
	//                 x   y   x,  y  
	{  mon_Heartbeat,  0,  0, -1, -1, 0x00000000, 0, "<< Monitor Mode >>" },
	{  NULL,           4, -1, -1, -1, 0x00000000, 0, "This item is label only" },
	{  mon_Test1,      0, -1, -1, -1, 0x00000001, 1, "Test1[1] = " },
	{  mon_Test1,      0, -1, -1, -1, 0x00000002, 1, "Test1[2] = " },
	{  mon_Test2,      0, -1, -1, -1, 0x00000000, 2, "Test2 = " },
	{  NULL,           0, -1, -1, -1, 0x00000000, 3, FG_GRAY "------------------------ Execute \"mon\" command again to stop monitor." CHAR_DEFAULT },
	
	{  NULL,   0,   0,  0,  0,  0,  0, NULL }
};

#endif

static void cmd_Monitor( SH_HANDLE *hnd, int argc, char *argv[] )
{
	#if SH_MON_ENABLED != 0
	sh_ToggleMonitor( hnd, MonitorItems, 10, 1000 );
	#else
	sh_Printf( hnd, "Monitor function is disabled in config file.\n" );
	#endif
}

