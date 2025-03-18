#ifndef yae_menu_h_
#define yae_menu_h_

struct tagMENU_ITEM;
typedef void(*MENU_ACTION)( SH_HANDLE *, struct tagMENU_HANDLE *, struct tagMENU_ITEM *, int);
typedef void(*MENU_KEY_ACTION)( SH_HANDLE *, struct tagMENU_HANDLE *, unsigned char, SH_VIRTUAL_KEYS );
typedef void(*MENU_TRANSIT_ACTION)( SH_HANDLE *, struct tagMENU_HANDLE *);
typedef int(*MENU_ITEM_INITIALIZER)( SH_HANDLE *, struct tagMENU_HANDLE *, struct tagMENU_ITEM * );

typedef enum
{
	MENU_CAPTION,   // 選択できず、飛ばす項目
	MENU_ENTER,     // エンターキーで実行するのみの項目
	MENU_SELECTION, // 左右で選択する項目
	MENU_DECIMAL,   // 数値を入力する項目
	MENU_HEX,       // 16進数の数値を入力する項目
	MENU_STRING,    // 任意の文字列を入力する項目
	MENU_SUBITEM,   // MENU_SELECTIONの場合の子項目
	
	MENU_ITEM_TYPE_MAX
} MENU_ITEM_TYPE;
                                                       //                         Caption                                            PushedEnter
                                                       //   Type                  |    Tag Val, Tmp, Pos, Buf          BufLng  Init  |      ValChg SubItems
#define MENU_TEMPLATE_CAPTION(cap,tag)                     { MENU_CAPTION,        cap, tag,  0,   0,   0, NULL,             0, NULL, NULL,  NULL, NULL, 0 }
#define MENU_TEMPLATE_ENTER(cap,ae,tag)                    { MENU_ENTER,          cap, tag,  0,   0,   0, NULL,             0, NULL, ae,    NULL, NULL, 0 }
#define MENU_TEMPLATE_SELECTION(cap,init,sub,pos,tag)      { MENU_SELECTION,      cap, tag,  0,   0, pos, NULL,             0, init, NULL,  NULL, sub, ArraySize(sub)-1 }
#define MENU_TEMPLATE_DECIMAL(cap,init,ae,vc,pos,tag)      { MENU_DECIMAL,        cap, tag,  0,   0, pos, NULL,             0, init, ae,    vc,   NULL, 0 }
#define MENU_TEMPLATE_HEX(cap,init,ae,vc,pos,tag)          { MENU_HEX,            cap, tag,  0,   0, pos, NULL,             0, init, ae,    vc,   NULL, 0 }
#define MENU_TEMPLATE_STRING(cap,init,ae,vc,pos,buf,tag)   { MENU_STRING,         cap, tag,  0,   0, pos, buf, ArraySize(buf), init, ae,    vc,   NULL, 0 }
#define MENU_TEMPLATE_SUBITEM(cap,ae,vc,tag)               { MENU_SUBITEM,        cap, tag,  0,   0,   0, NULL,             0, NULL, ae,    vc,   NULL, 0 }
#define MENU_ITME_END                                      { MENU_ITEM_TYPE_MAX, NULL,NULL,  0,   0,   0, NULL,             0, NULL, NULL,  NULL, NULL, 0 }

typedef struct tagMENU_ITEM
{
	MENU_ITEM_TYPE Type;
	char *Caption;  // その項目の表題
	void *Tag;      // ユーザーが任意に使用できる値
	int Value;      // 付随する数値
	int Tmp;        // ライブラリ側で使用する一時領域
	int SubItemPos; // 子項目の表示桁
	char *Buf;      // 文字列/数値入力がある場合に使用する
	int BufLength;  // Bufの長さ
	
	MENU_ITEM_INITIALIZER Initializer;          // 起動時、初期化用に、最初に実行される
	MENU_ACTION           Callback_Enter;       // その項目でEnterが押された場合に実行する関数
	MENU_ACTION           CallbackValueChanged; // 値が変化したときに実行する関数
	
	struct tagMENU_ITEM *SubItems; // ぶら下げる子アイテム
	int SubCount; // 子アイテムの数
} MENU_ITEM;

typedef struct tagMENU_HANDLE
{
	char *Unselected; // 非選択時
	char *Selected;   // 選択時
	char *UnfocusedSel;   // 非選択時の子項目
	
	int Sel; // 選択中のインデックス
	
	MENU_ITEM *Items; // 全項目一覧
	int ItemCount;    // 項目数
	int TopPos;       // 最上段の位置
	
	MENU_KEY_ACTION     Callback_AnyKey;  // 何らかのキーが押された場合に実行する関数
	MENU_TRANSIT_ACTION Callback_StatusChanged; // 状態が変化したときに実行する関数
	
	SH_HOOK_INPUT OldHook;
} MENU_HANDLE;

extern void menu_Initialize( MENU_HANDLE *mHnd, MENU_ITEM *mItem );
extern void menu_Ready( SH_HANDLE *shHnd, MENU_HANDLE *mHnd );
extern void menu_ShowOtherMenu( SH_HANDLE *shHnd, MENU_HANDLE *mHnd );
extern void menu_InitItems( SH_HANDLE *shHnd, MENU_HANDLE *mHnd );
extern void menu_WriteInitialImage( SH_HANDLE *shHnd, MENU_HANDLE *mHnd );
extern void menu_Exit( SH_HANDLE *shHnd );

#endif
