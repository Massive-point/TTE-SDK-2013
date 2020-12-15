//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef YEET_FRAG_H
#define YEET_FRAG_H
#pragma once

class CBaseGrenade;
struct edict_t;

CBaseGrenade *Fragyeet_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, bool SpawnSiltype);

#endif // ROCK_FRAG_H
