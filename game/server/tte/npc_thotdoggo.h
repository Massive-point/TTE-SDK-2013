//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef NPC_THOTDOGGO_H
#define NPC_THOTDOGGO_H


//#include	"hl1_ai_basenpc.h"
#include "hl1_npc_talker.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	THOTDOGGO_AE_ATTACK_RIGHT		0x01
#define	THOTDOGGO_AE_ATTACK_LEFT		0x02
#define	THOTDOGGO_AE_ATTACK_BOTH		0x03

#define THOTDOGGO_FLINCH_DELAY			2		// at most one flinch every n secs

//=========================================================
//=========================================================
class CNPC_Thotdoggo : public CHL1NPCTalker
{
	DECLARE_CLASS(CNPC_Thotdoggo, CHL1NPCTalker);
public:

	DECLARE_DATADESC();

	void Spawn(void);
	void Precache(void);
	float MaxYawSpeed(void);
	Class_T Classify(void);
	void HandleAnimEvent(animevent_t *pEvent);
	//	int IgnoreConditions ( void );

	float m_flNextFlinch;
	float	m_flNextPainSoundTime;

	int TranslateSchedule(int scheduleType);

	void PainSound(const CTakeDamageInfo &info);
	void AlertSound(void);
	//void IdleSound(void);
	void AttackSound(void);

	bool	ShouldGib(const CTakeDamageInfo &info);
	void	DeathSound(const CTakeDamageInfo &info);

	// No range attacks
	BOOL CheckRangeAttack1(float flDot, float flDist) { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) { return FALSE; }
	int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);

	void RemoveIgnoredConditions(void);
	int MeleeAttack1Conditions(float flDot, float flDist);

	enum
	{
		SCHED_THOTDOGGO_CHASE_ENEMY = BaseClass::NEXT_SCHEDULE,
	};

	DEFINE_CUSTOM_AI;
};

#endif //NPC_THOTDOGGO_H