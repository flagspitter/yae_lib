#ifndef yae_script_h_
#define yae_script_h_

struct tagSCR_HANDLE;

typedef enum
{
	SCR_WAIT,
	SCR_REPEAT,
	SCR_EXIT
} SCR_CONDITION_TYPE;

typedef void(*SCR_ACTION)(struct tagSCR_HANDLE *);
typedef bool(*SCR_CONDITION)(struct tagSCR_HANDLE *);

typedef struct
{
	char *Name;
	SCR_ACTION Function;
} SCR_COMMAND;

typedef struct
{
	char Identifier;
	SCR_FUNCTION Condition;
} SCR_CONDITION;

typedef struct
{
	SCR_FUNCTION *PreConditions;
	SCR_FUNCTION *PostConditions;
	SCR_ACTION Function
} SCR_STATEMENT;

typedef struct tagSCR_HANDLE
{
	int CurLine;
	void *Param;
} SCR_HANDLE;

extern void scr_Compile( SCR_HANDLE *hnd, const char *script );
extern void scr_Execute( SCR_HANDLE *hnd, void *param );
extern void scr_Tick( SCR_HANDLE *hnd );

#endif
