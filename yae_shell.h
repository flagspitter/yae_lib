#ifndef yae_shell_h_
#define yae_shell_h_

// Yet Another Embedded Shell
#define SH_INFORMATION "yae-Shell Ver 1.4"

//----------------------------------------------------------------------
//-- (System) Include Files
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

//----------------------------------------------------------------------
//-- Macro for configuration
//----------------------------------------------------------------------
#define SH_CR_LF 0
#define SH_LF    1
#define SH_CR    2

//----------------------------------------------------------------------
//-- Include Files
//----------------------------------------------------------------------
#include "_yae_config.h"

#if ( SH_ESC_SEQ == 0 ) && ( SH_CURSOR_MOVING != 0 )
	#undef  SH_CURSOR_MOVING
	#define SH_CURSOR_MOVING 0
#endif

#if ( SH_ESC_SEQ == 0 ) && ( SH_CSI_RESPONSE != 0 )
	#undef SH_CSI_RESPONSE
	#define SH_CSI_RESPONSE 0
#endif 

//----------------------------------------------------------------------
//-- Definitions / Macros
//----------------------------------------------------------------------
#define ArraySize(a) ((int)( sizeof(a) / sizeof((a)[0]) ))

#if SH_LINE_BREAK == SH_CR
	#define LINE_BREAK "\r"
	#define LINE_BREAK_CODE '\r'
#elif SH_LINE_BREAK == SH_LF
	#define LINE_BREAK "\n"
	#define LINE_BREAK_CODE '\n'
#else
	#define LINE_BREAK "\r\n"
	#define LINE_BREAK_CODE '\r'
#endif

//----------------------------------------------------------------------
//-- Pre-definitions
//----------------------------------------------------------------------
struct tagSH_HANDLE;

//----------------------------------------------------------------------
//-- Public Types
//----------------------------------------------------------------------

typedef enum
{
	SH_VK_NONE,
	SH_VK_ESC,
	SH_VK_LEFT,
	SH_VK_DOWN,
	SH_VK_UP,
	SH_VK_RIGHT,
	SH_VK_INSERT,
	SH_VK_HOME,
	SH_VK_END,
	SH_VK_PAGE_UP,
	SH_VK_PAGE_DOWN,
	
	SH_VK_MAX
} SH_VIRTUAL_KEYS;

typedef void(*SH_ACTION)(struct tagSH_HANDLE *);
typedef char(*SH_READER)(void);
typedef void(*SH_WRITER)(char ch);
typedef bool(*SH_READ_CHECKER)(void);
typedef bool(*SH_TICK)(struct tagSH_HANDLE *);
typedef bool(*SH_HOOK_INPUT)(struct tagSH_HANDLE *, unsigned char, SH_VIRTUAL_KEYS);
typedef void(*SH_MON_ACTION)(struct tagSH_HANDLE *, unsigned int);

#if SH_HISTORY_NUM > 0

// ヒストリ保存用のリスト構造
typedef struct tagSH_HISTORY
{
	char Buf[SH_BUF_SIZE];  // 上下でヒストリ
	
	struct tagSH_HISTORY *Next;
	struct tagSH_HISTORY *Prev;
} SH_HISTORY;

// ヒストリの管理用
typedef struct
{
	SH_HISTORY Entity[SH_HISTORY_NUM];
	int Count;
	SH_HISTORY Entry;
	SH_HISTORY *Current;
} SH_HISTORY_HANDLE;

#endif

#if SH_COMPLETE_ENABLED != 0
// コマンド補完の管理用
typedef struct
{
	struct tag_SH_COMMAND_ELEMENT *CmdPos;
	int AutoColumn;
} SH_COMPLETE_HANDLE;
#endif

// モニタ機能の管理用
typedef struct
{
	SH_MON_ACTION Writer;
	int Lb_x;
	int Lb_y;
	int Val_x;
	int Val_y;
	unsigned int Param;
	int Band;
	char *Label;
} SH_MON_ITEM;

typedef struct
{
	SH_MON_ITEM *TopItem;
	SH_MON_ITEM *CurItem;
	
	int  UpdateInterval;
	int  UpdateCount;
	
	int  LabelInterval;
	int  LabelCount;
	
	int  BottomRow;
} SH_MON_HANDLE;

struct tagMENU_HANDLE;

// シェルのハンドル
typedef struct tagSH_HANDLE
{
	char PrintBuf[SH_PRINT_BUF_SIZE];
	char ReadBuf[SH_BUF_SIZE];
	int  Wp;
	
	#if SH_HISTORY_NUM > 0
	SH_HISTORY_HANDLE  History;
	#endif
	
	#if SH_COMPLETE_ENABLED != 0
	SH_COMPLETE_HANDLE Complete;
	#endif
	
	#if SH_MON_ENABLED != 0
		SH_MON_HANDLE Monitor;
	#endif
	
	#if SH_ESC_SEQ != 0
		unsigned char EscOpenChar;
		int EscParamPos;
		int EscParams[16];
	#endif
	
	#if SH_CSI_RESPONSE != 0
		int TerminalX;
		int TerminalY;
	#endif
	
	#if SH_BOOT_KEY_EN != 0
		bool Boot;
	#endif
	
	struct tagMENU_HANDLE *Menu;
	
	int CurCursor;
	int EscDelayCount;
	bool Overwrite;        // 上書き入力モード
	bool EscStatus;
	bool SkipPrompt;
	
	void *Param;           // 実装側が任意に使用可能
	
	SH_ACTION Initializer;   // 最初に実行される関数
	SH_READER Reader;        // 1文字読み込む関数
	SH_WRITER Writer;        // 1文字出力する関数
	SH_READ_CHECKER Checker; // 入力が存在する場合にtrueを返す関数
	SH_TICK   Tick;          // 定期的に実行
	SH_HOOK_INPUT HookInput; // 入力のフック
	
	struct tag_SH_COMMAND_ELEMENT *CmdTable;
} SH_HANDLE;

// コマンド定義用
typedef void(*SH_COMMAND)( SH_HANDLE *hnd, int argc, char *argv[]);

typedef struct tag_SH_COMMAND_ELEMENT
{
	char *Name;
	SH_COMMAND Func;
} SH_COMMAND_ELEMENT;

//----------------------------------------------------------------------
//-- Public Function-Prototypes
//----------------------------------------------------------------------
extern void sh_Open ( SH_HANDLE *hnd, SH_COMMAND_ELEMENT *c, SH_ACTION ini, SH_READER r, SH_WRITER w, SH_READ_CHECKER rc );
extern void sh_Clone( const SH_HANDLE *src, SH_HANDLE *dst, SH_ACTION ini, SH_READER r, SH_WRITER w, SH_READ_CHECKER rc );
extern void sh_Tick( SH_HANDLE *hnd );
extern void sh_Ready( SH_HANDLE *hnd );
extern void sh_Prompt( SH_HANDLE *hnd );
extern int  sh_Printf( SH_HANDLE *hnd, const char *fmt, ... );
extern void sh_RegisterTick( SH_HANDLE *hnd, SH_TICK func );
extern SH_HOOK_INPUT sh_RegisterHookInput( SH_HANDLE *hnd, SH_HOOK_INPUT func );
extern void sh_ToggleMonitor( SH_HANDLE *hnd, SH_MON_ITEM *mon, int updateInt, int labelInt );
extern void sh_SetWindowTitle( SH_HANDLE *hnd, char *s );

#endif
