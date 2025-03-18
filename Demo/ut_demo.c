#include "yae_shell.h"
#include "yae_unit.h"

void UnitTest( SH_HANDLE *hnd )
{
	UT_Initialize( hnd );
	
	int i = 0;
	UT_AddTest    ( Test_MustBeFailed, "This test must be failed" ); // Softに同じ
	UT_AddTestHard( Test_HW, "This test is for hardware" );
	UT_AddTestSoft( Test_MustBePassed, "This test must be passed" );
	
	UT_Run();
}

//----------------------------------------------------------------------
//---- Unit test
//----------------------------------------------------------------------

UNIT_TEST( Test_MustBeFailed )
{
	UT_Message( "Test is running" );
	
	int a = 42;
	
	UT_Assert( a == 42 );
	UT_Assert( a != 42 );
}

UNIT_TEST( Test_HW )
{
	UT_Message( "Test is running" );
	
	int a = 42;
	
	UT_Assert( a == 42 );
	UT_Message( " HWテストの確認：結果をキーボードから入力します" );
}

UNIT_TEST( Test_MustBePassed )
{
	UT_Assert( true );
}


