#ifndef esc_seq_h_
#define esc_seq_h_

//----------------------------------------------------------------------
//-- Colors
//----------------------------------------------------------------------

#define CHAR_DEFAULT "\x1b[0m"

#define FG_BLACK     "\x1b[30m"
#define FG_RED       "\x1b[31m"
#define FG_GREEN     "\x1b[32m"
#define FG_YELLOW    "\x1b[33m"
#define FG_BLUE      "\x1b[34m"
#define FG_MAGENTA   "\x1b[35m"
#define FG_CYAN      "\x1b[36m"
#define FG_WHITE     "\x1b[37m"
#define FG_GRAY      "\x1b[90m"
#define FG_L_RED     "\x1b[91m"
#define FG_L_GREEN   "\x1b[92m"
#define FG_L_YELLOW  "\x1b[93m"
#define FG_L_BLUE    "\x1b[94m"
#define FG_L_MAGENTA "\x1b[95m"
#define FG_L_CYAN    "\x1b[96m"
#define FG_L_GRAY    "\x1b[97m"

#define BG_BLACK     "\x1b[40m"
#define BG_RED       "\x1b[41m"
#define BG_GREEN     "\x1b[42m"
#define BG_YELLOW    "\x1b[43m"
#define BG_BLUE      "\x1b[44m"
#define BG_MAGENTA   "\x1b[45m"
#define BG_CYAN      "\x1b[46m"
#define BG_WHITE     "\x1b[47m"
#define BG_GRAY      "\x1b[100m"
#define BG_L_RED     "\x1b[101m"
#define BG_L_GREEN   "\x1b[102m"
#define BG_L_YELLOW  "\x1b[103m"
#define BG_L_BLUE    "\x1b[104m"
#define BG_L_MAGENTA "\x1b[105m"
#define BG_L_CYAN    "\x1b[106m"
#define BG_L_GRAY    "\x1b[107m"

#define CURSOR_UP    "\x1b[A"
#define CURSOR_DOWN  "\x1b[B"
#define CURSOR_RIGHT "\x1b[C"
#define CURSOR_LEFT  "\x1b[D"

#define CURSOR_HOME  "\x1b[1;1H"
#define ERASE_LINE   "\x1b[2K"
#define ERASE_ALL    "\x1b[2J"
#define HIDE_CURSOR  "\x1b[?25l"
#define SHOW_CURSOR  "\x1b[?25h"
#define LINE_TERM    "\x1b[0K\r\n"
#define LINE_TERM_C  "\x1b[0K"
#define CLEAR_SCREEN  "\x1b[2J"

#define SAVE_CURSOR    "\x1b[s"
#define RESTORE_CURSOR "\x1b[u"

#define DELETE_CH "\x1b[P"
#define BACKSPACE "\x1b[@"

#define CURSOR_BLOCK  "\x1b[0 q"
#define CURSOR_U_LINE "\x1b[3 q"
#define CURSOR_V_LINE "\x1b[5 q"

#define TERMINAL_INITIALIZER \
	"\r\n" \
	"\xff\xfb\x01" \
	"\x1b[12h" \
	"\x1b[20h" \
	"\x1b]11;#000000\x07" \
	"\x1b]10;#FFFFFF\x07" \
	CHAR_DEFAULT \
	SHOW_CURSOR \
	"\x1b[!p" \
	"\x1b[1G" \
	ERASE_LINE

#endif
