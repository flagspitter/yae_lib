#ifndef yae_comm_h_
#define yae_comm_h_

//----------------------------------------------------------------------
//--  Include
//----------------------------------------------------------------------
#include "_yae_config.h"
#include "yae_shell.h"

//----------------------------------------------------------------------
//-- Types
//----------------------------------------------------------------------
struct tagCOMM_HANDLE;

typedef void(*COMM_WRITER)(struct tagCOMM_HANDLE *hnd, unsigned char ch);

typedef struct
{
	unsigned char *Buf;
	int BufSize;
	int Wp;
	int Rp;
	int Overrun;
} COMM_BUFFER;

typedef struct tagCOMM_HANDLE
{
	void *Param;
	COMM_BUFFER Receive;
	COMM_BUFFER Send;
	
	COMM_WRITER Writer;
} COMM_HANDLE;

//----------------------------------------------------------------------
//-- Functions
//----------------------------------------------------------------------

// System
extern void comm_Open( COMM_HANDLE *hnd, unsigned char *rBuf, int rBufSize, unsigned char *wBuf, int wBufSize, COMM_WRITER writer, void *param );

// Called from low level / debug / test
extern void comm_SetReceivedData( COMM_HANDLE *hnd, unsigned char val );
extern void comm_SetReceivedDataRange( COMM_HANDLE *hnd, const unsigned char *src, int length );
extern bool comm_IsSendingDataAvailable( COMM_HANDLE *hnd );
extern unsigned char comm_GetSendingData( COMM_HANDLE *hnd );

// Basic read
extern bool comm_IsDataAvailable( COMM_HANDLE *hnd );
extern unsigned char comm_ReadByte( COMM_HANDLE *hnd );

// Basic write
extern void comm_WriteByte( COMM_HANDLE *hnd, unsigned char val );
extern void comm_Write( COMM_HANDLE *hnd, const unsigned char *buf, int length );

#endif
