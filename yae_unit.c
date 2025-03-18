//----------------------------------------------------------------------
//-- Include Files
//----------------------------------------------------------------------
#include "yae_lib.h"

//----------------------------------------------------------------------
//-- Global variables
//----------------------------------------------------------------------
// ・複数のテストは同時に走らせない前提
// ・テストの書きやすさを重視するため、面倒な儀式は不要にしたい
// ・以上の目的で、グローバル変数で制御する
static UT_OBJECT UtObj;

//----------------------------------------------------------------------
//-- Functions
//----------------------------------------------------------------------

/***************************************************************
	Name        UT_Initialize
	Purpose     
***************************************************************/
void UT_Initialize( SH_HANDLE *sh )
{
	UtObj.Shell = sh;
	UtObj.Passed = 0;
	UtObj.Failed = 0;
	
	UtObj.Entry = NULL;
	UtObj.Tail = NULL;
}

/***************************************************************
	Name        UT_AddTestSoft
	Purpose     
***************************************************************/
void UT_AddTest_Entity( UT_UNIT *ut, char *name, UT_FUNC func, bool hw )
{
	if( UtObj.Tail == NULL )
	{
		UtObj.Entry = ut;
	}
	else
	{
		ut->Next = NULL;
		UtObj.Tail->Next = ut;
	}
	UtObj.Tail = ut;
	
	ut->HardwareDependence = hw;
	ut->Func = func;
	ut->Name = name;
}

/***************************************************************
	Name        ShowResult
	Purpose     
***************************************************************/
static void ShowResult( void )
{
	sh_Printf( UtObj.Shell, " Result: " );
	if( UtObj.CurrentTest->Failed != 0 )
	{
		sh_Printf( UtObj.Shell, BG_RED "FAIL" CHAR_DEFAULT "\n" );
		UtObj.Failed++;
	}
	else
	{
		sh_Printf( UtObj.Shell, BG_BLUE "PASS" CHAR_DEFAULT "\n" );
		UtObj.Passed++;
	}
	sh_Printf( UtObj.Shell, "  Passed ... %d\n", UtObj.CurrentTest->Passed );
	sh_Printf( UtObj.Shell, "  Failed ... %d" CHAR_DEFAULT "\n", UtObj.CurrentTest->Failed );
}

/***************************************************************
	Name        Hook
	Purpose     
***************************************************************/
static bool Hook( SH_HANDLE *hnd, unsigned char ch, SH_VIRTUAL_KEYS vk )
{
	UtObj.Key = ch;
	return true;
}

/***************************************************************
	Name        BeginPrompt
	Purpose     
***************************************************************/
static void BeginPrompt( void )
{
	sh_Printf( UtObj.Shell, "  Result? ("
		"[" FG_L_YELLOW "P" CHAR_DEFAULT "]assed "
		"[" FG_L_YELLOW "F" CHAR_DEFAULT "]ailed "
		"[" FG_L_YELLOW "R" CHAR_DEFAULT "]etry " 
		") : " CHAR_DEFAULT );
	UtObj.Prompt = true;
	UtObj.Key = '\0';
	UtObj.OrgHook = sh_RegisterHookInput( UtObj.Shell, Hook );
}

/***************************************************************
	Name        PeriodHardwareTest
	Purpose     
***************************************************************/
static void PeriodHardwareTest( void )
{
	sh_Printf( UtObj.Shell, ERASE_LINE "\x1b[G" );
	ShowResult();
	UtObj.Prompt = false;
	UtObj.CurrentTest = UtObj.CurrentTest->Next;
}

/***************************************************************
	Name        PromptProcess
	Purpose     
***************************************************************/
static bool PromptProcess( struct tagSH_HANDLE *hnd )
{
	if( UtObj.Key != '\0' )
	{
		sh_RegisterHookInput( UtObj.Shell, UtObj.OrgHook );
		
		if( ( UtObj.Key == 'P' ) || ( UtObj.Key == 'p' ) )
		{
			UtObj.CurrentTest->Passed++;
			PeriodHardwareTest();
		}
		else if( ( UtObj.Key == 'F' ) || ( UtObj.Key == 'f' ) )
		{
			UtObj.CurrentTest->Failed++;
			sh_Printf( UtObj.Shell, "\x1b[G" ERASE_LINE FG_L_RED " [!] Hardware test Failed" CHAR_DEFAULT "\n" );
			PeriodHardwareTest();
		}
		else if( ( UtObj.Key == 'R' ) || ( UtObj.Key == 'r' ) )
		{
			sh_Printf( UtObj.Shell, ERASE_LINE "\x1b[G " FG_YELLOW " Retry." CHAR_DEFAULT "\n" );
			UtObj.Prompt = false;
		}
		else
		{
			sh_Printf( UtObj.Shell, "%c" "\x1b[G", UtObj.Key );
			BeginPrompt();
		}
	}
	
	UtObj.Key = '\0';
	return false;
}

/***************************************************************
	Name        TestProcess
	Purpose     
***************************************************************/
static bool TestProcess( struct tagSH_HANDLE *hnd )
{
	bool ret = true;
	
	sh_Printf( UtObj.Shell, "\n" BG_GRAY "[ %s ]" CHAR_DEFAULT "\n", UtObj.CurrentTest->Name );
	UtObj.CurrentTest->Passed = 0;
	UtObj.CurrentTest->Failed = 0;
	UtObj.CurrentTest->Func();
	if( UtObj.CurrentTest->HardwareDependence )
	{
		BeginPrompt();
		ret = false;
	}
	else
	{
		ShowResult();
		UtObj.CurrentTest = UtObj.CurrentTest->Next;
	}
	
	return ret;
}

/***************************************************************
	Name        PostProcess
	Purpose     
***************************************************************/
static bool PostProcess( struct tagSH_HANDLE *hnd )
{
	sh_Printf( UtObj.Shell, "\n" BG_GRAY "--------- Finished ---------" CHAR_DEFAULT "\n" );
	sh_Printf( UtObj.Shell, " Total Result: " );
	if( UtObj.Failed != 0 )
	{
		sh_Printf( UtObj.Shell, BG_RED "FAIL" CHAR_DEFAULT "\n" );
	}
	else
	{
		sh_Printf( UtObj.Shell, BG_BLUE "PASS" CHAR_DEFAULT "\n" );
	}
	sh_Printf( UtObj.Shell, "  Passed ... %d\n", UtObj.Passed );
	sh_Printf( UtObj.Shell, "  Failed ... %d\n", UtObj.Failed );
	sh_Printf( UtObj.Shell, "\n" );
	
	sh_RegisterTick( UtObj.Shell, UtObj.OldTick );
	sh_Prompt( UtObj.Shell );
	
	return true;
}

/***************************************************************
	Name        Run
	Purpose     
***************************************************************/
static bool Run( struct tagSH_HANDLE *hnd )
{
	bool ret = true;
	
	if( UtObj.Prompt )
	{
		ret = PromptProcess( hnd );
	}
	else if( UtObj.CurrentTest != NULL )
	{
		ret = TestProcess( hnd );
	}
	else
	{
		ret = PostProcess( hnd );
	}
	
	return ret;
}

/***************************************************************
	Name        UT_Run
	Purpose     
***************************************************************/
void UT_Run( void )
{
	sh_Printf( UtObj.Shell, "------- " UT_INFORMATION " -------\n" );
	UtObj.CurrentTest = UtObj.Entry;
	UtObj.Prompt = false;
	
	UtObj.Shell->SkipPrompt = true;
	UtObj.OldTick = UtObj.Shell->Tick;
	sh_RegisterTick( UtObj.Shell, Run );
}

/***************************************************************
	Name        UT_Assert_Entity
	Purpose     
***************************************************************/
void UT_Assert_Entity( const char *file, const char *func, int line, const char *cndStr, bool condition )
{
	if( !condition )
	{
		// Fail
		UtObj.CurrentTest->Failed++;
		sh_Printf( UtObj.Shell, FG_L_RED " [!] Assertion Failed in %s - %s(%d) :" CHAR_DEFAULT " ( %s )\n",
			func,
			file,
			line,
			cndStr
		);
	}
	else
	{
		UtObj.CurrentTest->Passed++;
	}
}

/***************************************************************
	Name        UT_Message
	Purpose     
***************************************************************/
void UT_Message( const char *msg )
{
	sh_Printf( UtObj.Shell, msg );
	sh_Printf( UtObj.Shell, "\n" );
}
