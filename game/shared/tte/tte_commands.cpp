#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar money("money", 0, FCVAR_ARCHIVE | FCVAR_REPLICATED, "Money."); //Money Convar.

#ifdef CLIENT_DLL
//Create new chudmoney class here
//Also update vpc scripts to have new files. Kill me.
#endif