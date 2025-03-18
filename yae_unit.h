#ifndef yae_unit_h_
#define yae_unit_h_

#define UT_INFORMATION "yae-Unit Ver 0.1"

//----------------------------------------------------------------------
//-- Include files
//----------------------------------------------------------------------
#include "yae_shell.h"

//----------------------------------------------------------------------
//-- Types
//----------------------------------------------------------------------

typedef void(*UT_FUNC)(void);

struct tagUT_UNIT;

typedef struct
{
	SH_HANDLE *Shell;
	int Passed;
	int Failed;
	
	SH_TICK OldTick;
	
	struct tagUT_UNIT *Entry;
	struct tagUT_UNIT *Tail;
	
	// Status
	struct tagUT_UNIT *CurrentTest;
	bool Prompt;
	char Key;
	SH_HOOK_INPUT OrgHook;
} UT_OBJECT;

typedef struct tagUT_UNIT
{
	bool HardwareDependence;
	int Passed;
	int Failed;
	
	UT_FUNC Func;
	const char *Name;
	
	struct tagUT_UNIT *Next;
} UT_UNIT;

//----------------------------------------------------------------------
//-- Function prototypes and Wrappers
//----------------------------------------------------------------------

extern void UT_Initialize( SH_HANDLE *sh );

#define UT_AddTest( f, n )     extern UT_UNIT hnd_##f; extern void f(void); UT_AddTest_Entity( &hnd_##f, n, f, false );
#define UT_AddTestSoft( f, n ) extern UT_UNIT hnd_##f; extern void f(void); UT_AddTest_Entity( &hnd_##f, n, f, false );
#define UT_AddTestHard( f, n ) extern UT_UNIT hnd_##f; extern void f(void); UT_AddTest_Entity( &hnd_##f, n, f, true );
extern void UT_AddTest_Entity( UT_UNIT *ut, char *name, UT_FUNC func, bool hw );

extern void UT_Run( void );

#define UT_Assert( cnd ) UT_Assert_Entity( __FILE__, __func__, __LINE__, #cnd, cnd )
extern void UT_Assert_Entity( const char *file, const char *func, int line, const char *cndStr, bool condition );

#define UNIT_TEST(f) UT_UNIT hnd_##f; void f( void )

extern void UT_Message( const char *msg );

#endif
