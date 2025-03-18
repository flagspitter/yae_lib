#include "yae_lib.h"

#define IncRing(p,size) ( ( (p)+1 > size ) ? 0 : (p)+1 )

/***************************************************************
	Name        comm_Open
	Purpose     入出力の準備
	
	Note
		最初にハンドルと入出力バッファを上位が用意して、
		このこの関数を実行することで、入出力のバッファリング処理を請け負う。
		出力に使用する関数をwriter引数指定する。
		writerは、1バイト出力する関数。必要であれば、writer関数内でブロックしても良い。
		param引数は、ユーザーが任意で使用可能。
		
		1バイト単位で関数を実行することからわかるように、
		速度よりも保守と再利用を重視している。
***************************************************************/
void comm_Open( COMM_HANDLE *hnd, unsigned char *rBuf, int rBufSize, unsigned char *wBuf, int wBufSize, COMM_WRITER writer, void *param )
{
	hnd->Receive.Buf = rBuf;
	hnd->Receive.BufSize = rBufSize;
	hnd->Receive.Wp = 0;
	hnd->Receive.Rp = 0;
	hnd->Receive.Overrun = 0;
	
	hnd->Send.Buf = wBuf;
	hnd->Send.BufSize = wBufSize;
	hnd->Send.Wp = 0;
	hnd->Send.Rp = 0;
	hnd->Send.Overrun = 0;
	
	hnd->Writer = writer;
	hnd->Param = param;
}

/***************************************************************
	Name        PutBuffer
	Purpose     バッファに1バイト追加
***************************************************************/
static void PutBuffer( COMM_BUFFER *bufHnd, unsigned char val )
{
	int next = IncRing( bufHnd->Wp, bufHnd->BufSize );
	
	if( next == bufHnd->Rp )
	{
		bufHnd->Overrun++;
	}
	else
	{
		bufHnd->Buf[ bufHnd->Wp ] = val;
		bufHnd->Wp = next;
	}
}

/***************************************************************
	Name        GetBuffer
	Purpose     バッファから1バイト取得
***************************************************************/
static unsigned char GetBuffer( COMM_BUFFER *bufHnd )
{
	unsigned char ret;
	
	if( bufHnd->Wp != bufHnd->Rp )
	{
		ret = bufHnd->Buf[ bufHnd->Rp ];
		bufHnd->Rp = IncRing( bufHnd->Rp, bufHnd->BufSize );
	}
	else
	{
		ret = '\0';
	}
	
	return ret;
}

/***************************************************************
	Name        IsReceived
	Purpose     バッファから取得できるデータの有無
***************************************************************/
static bool IsReceived( COMM_BUFFER *bufHnd )
{
	return ( IncRing( bufHnd->Wp, bufHnd->BufSize ) != bufHnd->Rp ) ? true : false;
}

/***************************************************************
	Name        IsWritable
	Purpose     バッファの空きスペース有無
***************************************************************/
static bool IsWritable( COMM_BUFFER *bufHnd )
{
	return ( bufHnd->Wp != bufHnd->Rp ) ? true : false;
}


/***************************************************************
	Name        comm_SetReceivedData
	Purpose     受信割り込みルーチンなどがデータをバッファに格納
***************************************************************/
void comm_SetReceivedData( COMM_HANDLE *hnd, unsigned char val )
{
	PutBuffer( &hnd->Receive, val );
}

void comm_SetReceivedDataRange( COMM_HANDLE *hnd, const unsigned char *src, int length )
{
	int i;
	for( i=0; i<length; i++ )
	{
		PutBuffer( &hnd->Receive, src[i] );
	}
}

/***************************************************************
	Name        comm_IsDataAvailable
	Purpose     通常ルーチンで、データを受信可能か判別する
***************************************************************/
bool comm_IsSendingDataAvailable( COMM_HANDLE *hnd )
{
	return IsReceived( &hnd->Send );
}

/***************************************************************
	Name        comm_GetSendingData
	Purpose     送信割り込みなどがデータをバッファから取得
***************************************************************/
unsigned char comm_GetSendingData( COMM_HANDLE *hnd )
{
	return GetBuffer( &hnd->Send );
}

// ---

/***************************************************************
	Name        comm_IsDataAvailable
	Purpose     通常ルーチンで、データを受信可能か判別する
***************************************************************/
bool comm_IsDataAvailable( COMM_HANDLE *hnd )
{
	return IsReceived( &hnd->Receive );
}

/***************************************************************
	Name        comm_ReadByte
	Purpose     通常ルーチンで、受信したデータを受け取る
***************************************************************/
unsigned char comm_ReadByte( COMM_HANDLE *hnd )
{
	return GetBuffer( &hnd->Receive );
}

/***************************************************************
	Name        comm_WriteByte
	Purpose     通常ルーチンで、データの送信を予約する
***************************************************************/
void comm_WriteByte( COMM_HANDLE *hnd, unsigned char val )
{
	PutBuffer( &hnd->Send, val );
}

void comm_Write( COMM_HANDLE *hnd, const unsigned char *buf, int length )
{
	int i;
	for( i=0; i<length; i++ )
	{
		PutBuffer( &hnd->Send, buf[i] );
	}
}
