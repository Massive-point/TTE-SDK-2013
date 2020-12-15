//========= Copyright Binka Corporation, All rights reserved. ============//
//
// Purpose: Earrape Generic.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Class Earrape Generic
//-----------------------------------------------------------------------------

class CEarrapeGeneric : public CPointEntity
{
public:
	DECLARE_CLASS(CEarrapeGeneric, CPointEntity);

	void	Spawn(void);
	void	Precache(void);
	void	Activate(void);

	void	PlayEarrape(void);
	void	StopEarrape(void);

	void	RrrLoopThink(void);

	// Input handlers
	void InputPlaySound(inputdata_t &inputdata);
	void InputStopSound(inputdata_t &inputdata);
	void InputToggleSound(inputdata_t &inputdata);
	void InputSetVolume(inputdata_t &inputdata); //Volume

	//variables for spawn flags
	bool m_fLooping;		// true when the sound played will loop
	bool m_fActive = false;		// only true when the entity is playing a looping sound

	DECLARE_DATADESC();

	string_t m_iszSound[4];			// Path/filename of WAV file to play.
	string_t m_sSourceEntName[4];
	string_t m_iszSndForLoop;
	EHANDLE m_hSoundSource[4];
	float earrapevolume[4];
	float m_flERRVolume[4];
	int m_iErrLevel[4];
	float m_flERRLooptime;
	float m_flNextPlayTime = 0;
};

LINK_ENTITY_TO_CLASS(earrape_generic, CEarrapeGeneric);

BEGIN_DATADESC(CEarrapeGeneric)

DEFINE_KEYFIELD(m_iszSound[0], FIELD_SOUNDNAME, "message"),
DEFINE_KEYFIELD(m_iszSound[1], FIELD_SOUNDNAME, "message1"),
DEFINE_KEYFIELD(m_iszSound[2], FIELD_SOUNDNAME, "message2"),
DEFINE_KEYFIELD(m_iszSound[3], FIELD_SOUNDNAME, "message3"),
DEFINE_KEYFIELD(m_sSourceEntName[0], FIELD_STRING, "SourceEntityName"),
DEFINE_KEYFIELD(m_sSourceEntName[1], FIELD_STRING, "SourceEntityName1"),
DEFINE_KEYFIELD(m_sSourceEntName[2], FIELD_STRING, "SourceEntityName2"),
DEFINE_KEYFIELD(m_sSourceEntName[3], FIELD_STRING, "SourceEntityName3"),
DEFINE_KEYFIELD(m_flERRVolume[0], FIELD_FLOAT, "ErrVolume"),
DEFINE_KEYFIELD(m_flERRVolume[1], FIELD_FLOAT, "ErrVolume1"),
DEFINE_KEYFIELD(m_flERRVolume[2], FIELD_FLOAT, "ErrVolume2"),
DEFINE_KEYFIELD(m_flERRVolume[3], FIELD_FLOAT, "ErrVolume3"),
DEFINE_KEYFIELD(m_iErrLevel[0], FIELD_INTEGER, "ErrLevel"),
DEFINE_KEYFIELD(m_iErrLevel[1], FIELD_INTEGER, "ErrLevel1"),
DEFINE_KEYFIELD(m_iErrLevel[2], FIELD_INTEGER, "ErrLevel2"),
DEFINE_KEYFIELD(m_iErrLevel[3], FIELD_INTEGER, "ErrLevel3"),

DEFINE_KEYFIELD(m_flERRLooptime, FIELD_FLOAT, "ErrLooptime"),
DEFINE_KEYFIELD(m_iszSndForLoop, FIELD_SOUNDNAME, "SndForLoop"),

DEFINE_FIELD(m_fLooping, FIELD_BOOLEAN),
DEFINE_FIELD(m_fActive, FIELD_BOOLEAN),
DEFINE_FIELD(earrapevolume[0], FIELD_FLOAT),
DEFINE_FIELD(earrapevolume[1], FIELD_FLOAT),
DEFINE_FIELD(earrapevolume[2], FIELD_FLOAT),
DEFINE_FIELD(earrapevolume[3], FIELD_FLOAT),
//DEFINE_FIELD(m_flNextPlayTime, FIELD_TIME),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "PlaySound", InputPlaySound),
DEFINE_INPUTFUNC(FIELD_VOID, "StopSound", InputStopSound),
DEFINE_INPUTFUNC(FIELD_VOID, "ToggleSound", InputToggleSound),
//DEFINE_INPUTFUNC(FIELD_INTEGER, "SetVolume", InputSetVolume),
//4 sounds R G B A
DEFINE_INPUTFUNC(FIELD_COLOR32, "SetVolume", InputSetVolume),

// Function Pointers
DEFINE_FUNCTION(RrrLoopThink),

END_DATADESC()

#define SF_EARRAPE_LOOP			1
#define SF_EARRAPE_NOT_PAUSE	16
//#define SF_AMBIENT_SOUND_NOT_LOOPING		32
//#define SF_AMBIENT_SOUND_NOT_PAUSE			64

void CEarrapeGeneric::Spawn(void)
{
	if (FBitSet(m_spawnflags, SF_EARRAPE_LOOP))
	{
		m_fLooping = true;
	}
	else
	{
		m_fLooping = false;
	}

	for (int i = 0; i < 4; i++)
	{
		earrapevolume[i] = m_flERRVolume[i];
	}

	if (m_iszSndForLoop != NULL_STRING)
	{
		char *szSoundLoopFile = (char *)STRING(m_iszSndForLoop);
		m_flERRLooptime = enginesound->GetSoundDuration(szSoundLoopFile) + 0.02;
		//Warning("Earrape length: %5.5f \n", m_flERRLooptime);
	}

	BaseClass::Spawn();
	Precache();
}

void CEarrapeGeneric::Precache(void)
{
	for (int i = 0; i < 4; i++)
	{
		char *szSoundFile = (char *)STRING(m_iszSound[i]);
		if (m_iszSound[i] != NULL_STRING && strlen(szSoundFile) > 1)
		{
			if (*szSoundFile != '!')
			{
				PrecacheScriptSound(szSoundFile);
			}
		}
	}


}

void CEarrapeGeneric::InputPlaySound(inputdata_t &inputdata)
{
	PlayEarrape();
}
void CEarrapeGeneric::InputStopSound(inputdata_t &inputdata)
{
	StopEarrape();
}
void CEarrapeGeneric::InputToggleSound(inputdata_t &inputdata)
{
	if (m_fActive)
	{
		StopEarrape();
	}
	else
	{
		PlayEarrape();
	}
}

void CEarrapeGeneric::RrrLoopThink(void)
{
	PlayEarrape();
}


void CEarrapeGeneric::PlayEarrape(void)
{
	//Warning("Earrape length: %5.5f \n", m_flERRLooptime);
	if ((m_flNextPlayTime <= gpGlobals->curtime + 0.1) && m_fLooping && (!FBitSet(m_spawnflags, SF_EARRAPE_NOT_PAUSE)) && m_flERRLooptime >= 0)
	{
		SetThink(&CEarrapeGeneric::RrrLoopThink);
		SetNextThink(gpGlobals->curtime + m_flERRLooptime);
		m_flNextPlayTime = gpGlobals->curtime + m_flERRLooptime;
	}

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	int flags = SND_NOFLAGS;

	flags |= (SND_CHANGE_PITCH | SND_CHANGE_VOL);

	if (!FBitSet(m_spawnflags, SF_EARRAPE_NOT_PAUSE))
	{
		flags |= (SND_SHOULDPAUSE);
	}
	
	for (int i = 0; i < 4; i++)
	{
		if (m_iszSound[i] != NULL_STRING)
		{
			char *szSoundFile = (char *)STRING(m_iszSound[i]);
			if (m_sSourceEntName[i] != NULL_STRING)
			{
				m_hSoundSource[i] = gEntList.FindEntityByName(NULL, m_sSourceEntName[i]);
				if (m_hSoundSource[i] != NULL)
				{
					UTIL_EmitAmbientSound(m_hSoundSource[i]->entindex(), m_hSoundSource[i]->GetAbsOrigin(), szSoundFile, earrapevolume[i], (soundlevel_t)m_iErrLevel[i], flags, 100);
				}
			}
			else
			{
				UTIL_EmitAmbientSound(pPlayer->entindex(), pPlayer->GetAbsOrigin(), szSoundFile, earrapevolume[i], SNDLVL_NONE, flags, 100);
			}
			
		}
	}
	
	m_fActive = true;
}

void CEarrapeGeneric::StopEarrape(void)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	SetThink(NULL);
	m_flNextPlayTime = 0;

	for (int i = 0; i < 4; i++)
	{
		if (m_iszSound[i] != NULL_STRING)
		{
			char *szSoundFile = (char *)STRING(m_iszSound[i]);
			if (m_sSourceEntName[i] != NULL_STRING)
			{
				m_hSoundSource[i] = gEntList.FindEntityByName(NULL, m_sSourceEntName[i]);
				if (m_hSoundSource[i] != NULL)
				{
					UTIL_EmitAmbientSound(m_hSoundSource[i]->entindex(), m_hSoundSource[i]->GetAbsOrigin(), szSoundFile, 0, SNDLVL_NONE, SND_STOP, 100);
				}
			}
			else
			{
				UTIL_EmitAmbientSound(pPlayer->entindex(), pPlayer->GetAbsOrigin(), szSoundFile, 0, SNDLVL_NONE, SND_STOP, 100);
			}

		}
	}

	m_fActive = false;
}

void CEarrapeGeneric::Activate(void)
{
	BaseClass::Activate();

	if (m_fLooping && m_fActive)
	{
		m_flNextPlayTime = 0;
		//PlayEarrape();
		//Play the sound after small delay.
		SetThink(&CEarrapeGeneric::RrrLoopThink);
		SetNextThink(gpGlobals->curtime + 0.2);
	}
}


void CEarrapeGeneric::InputSetVolume(inputdata_t &inputdata)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}
	color32 clr = inputdata.value.Color32();
	earrapevolume[0] = (clr.r * (1.0f / 100.0f));
	earrapevolume[1] = (clr.g * (1.0f / 100.0f));
	earrapevolume[2] = (clr.b * (1.0f / 100.0f));
	earrapevolume[3] = (clr.a * (1.0f / 100.0f));

	//Warning("Earrape volume: %5.2f %5.2f %5.2f %5.2f \n", earrapevolume[0], earrapevolume[1], earrapevolume[2], earrapevolume[3]);
	//If the sound is not active only change the volume, not play.
	if (!m_fActive)
	{
		return;
	}

	for (int i = 0; i < 4; i++)
	{
		if (m_iszSound[i] != NULL_STRING)
		{
			char *szSoundFile = (char *)STRING(m_iszSound[i]);
			if (m_sSourceEntName[i] != NULL_STRING)
			{
				m_hSoundSource[i] = gEntList.FindEntityByName(NULL, m_sSourceEntName[i]);
				if (m_hSoundSource[i] != NULL)
				{
					UTIL_EmitAmbientSound(m_hSoundSource[i]->entindex(), m_hSoundSource[i]->GetAbsOrigin(), szSoundFile, earrapevolume[i], (soundlevel_t)m_iErrLevel[i], SND_CHANGE_VOL, 100);
				}
			}
			else
			{
				UTIL_EmitAmbientSound(pPlayer->entindex(), pPlayer->GetAbsOrigin(), szSoundFile, earrapevolume[i], SNDLVL_NONE, SND_CHANGE_VOL, 100);
			}

		}
	}

}