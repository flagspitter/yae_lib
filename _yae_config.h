#ifndef yae_config_h_
#define yae_config_h_

//----------------------------------------------------------------------
//-- Configuration
//----------------------------------------------------------------------

/*------------------
    リソース関係
------------------*/
#define SH_HISTORY_NUM       8 // ヒストリ保存最大数
#define SH_BUF_SIZE        256 // 入力バッファのサイズ→確保されるバッファサイズは、この値 * ( ヒストリ数 + 1 )
#define SH_ESC_DELAY         1 // ここで指定されたTick数後にESCの判定を行う→通信速度が遅い場合は、この数値を大きくします
#define SH_PRINT_BUF_SIZE 1024 // sh_Printf で一度に出力できる最大文字数
#define SH_MAX_TOKEN        32 // トークンの最大数

/*------------------
    特殊な動作
------------------*/
// 不要な機能を無効にすることで、サイズ縮小が可能です

// 効果大
#define SH_ESC_SEQ           1  // ESCキーやエスケープシーケンス全般 → これを無効化すると、SH_CURSOR_MOVING, SH_CSI_RESPONSE は強制無効化されます
#define SH_MON_ENABLED       1  // モニタ機能：変化する状態を常に表示する機能

// 効果中
// ヒストリ機能の無効化は SH_HISTORY_NUM を 0 にしてください
#define SH_CURSOR_MOVING     1  // 左右キー等によるカーソル移動
#define SH_COMPLETE_ENABLED  1  // Tabキーによるコマンド名補完
#define SH_CSI_RESPONSE      0  // 端末側からの応答受信

// 効果小
#define SH_BOOT_KEY_EN       0  // 特定のキー操作で起動とする機能
#define SH_BOOT_KEY    " \x1b\x1b"  // その場合キー操作定義

/*------------------
    カスタマイズ
------------------*/

// 改行コード
#define SH_LINE_BREAK SH_CR_LF
// #define SH_LINE_BREAK SH_LF
// #define SH_LINE_BREAK SH_CR

#define SH_PROMPT FG_L_YELLOW ">" CHAR_DEFAULT " "


//----------------------------------------------------------------------
//-- Comm Configuration
//----------------------------------------------------------------------


#endif
