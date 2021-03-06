//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_basehlcombatweapon.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "c_weapon__stubs.h"
#include "materialsystem/imaterial.h"
#include "clienteffectprecachesystem.h"
#include "beamdraw.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectMemeCannon )
CLIENTEFFECT_MATERIAL( "effects/stunstick" )
CLIENTEFFECT_REGISTER_END()

class C_WeaponMeme : public C_HLSelectFireMachineGun
{
	DECLARE_CLASS( C_WeaponMeme, C_HLSelectFireMachineGun );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	int DrawModel( int flags )
	{
		//FIXME: This sucks, but I can't easily create temp ents...

		if ( m_bActive )
		{
			Vector	vecOrigin;
			QAngle	vecAngles;
			float	color[3];

			color[0] = color[1] = color[2] = random->RandomFloat( 0.1f, 0.2f );

			GetAttachment( 1, vecOrigin, vecAngles );

			Vector	vForward;
			AngleVectors( vecAngles, &vForward );

			Vector vEnd = vecOrigin - vForward * 1.0f;

			IMaterial *pMaterial = materials->FindMaterial( "effects/stunstick", NULL, false );

			CMatRenderContextPtr pRenderContext( materials );
			pRenderContext->Bind( pMaterial );
			DrawHalo( pMaterial, vEnd, random->RandomFloat( 4.0f, 6.0f ), color );

			color[0] = color[1] = color[2] = random->RandomFloat( 0.9f, 1.0f );

			DrawHalo( pMaterial, vEnd, random->RandomFloat( 2.0f, 3.0f ), color );
		}

		return BaseClass::DrawModel( flags );
	}

	// Do part of our effect
	void ClientThink( void )
	{
		// Update our effects
		if ( m_bActive && 
			gpGlobals->frametime != 0.0f &&
			( random->RandomInt( 0, 5 ) == 0 ) )
		{
			/*Vector	vecOrigin;
			QAngle	vecAngles;

			GetAttachment( 1, vecOrigin, vecAngles );

			Vector	vForward;
			AngleVectors( vecAngles, &vForward );

			Vector vEnd = vecOrigin - vForward * 1.0f;

			// Inner beams
			BeamInfo_t beamInfo;

			beamInfo.m_vecStart = vEnd;
			Vector	offset = RandomVector( -6, 2 );

			offset += Vector(2,2,2);
			beamInfo.m_vecEnd = vecOrigin + offset;

			beamInfo.m_pStartEnt= cl_entitylist->GetEnt( BEAMENT_ENTITY( entindex() ) );
			beamInfo.m_pEndEnt	= cl_entitylist->GetEnt( BEAMENT_ENTITY( entindex() ) );
			beamInfo.m_nStartAttachment = 1;
			beamInfo.m_nEndAttachment = 2;
			
			beamInfo.m_nType = TE_BEAMTESLA;
			beamInfo.m_pszModelName = "sprites/physbeam.vmt";
			beamInfo.m_flHaloScale = 0.0f;
			beamInfo.m_flLife = 0.01f;
			beamInfo.m_flWidth = random->RandomFloat( 0.5f, 2.0f );
			beamInfo.m_flEndWidth = 0;
			beamInfo.m_flFadeLength = 0.0f;
			beamInfo.m_flAmplitude = random->RandomFloat( 1, 2 );
			beamInfo.m_flBrightness = 255.0;
			beamInfo.m_flSpeed = 0.0;
			beamInfo.m_nStartFrame = 0.0;
			beamInfo.m_flFrameRate = 1.0f;
			beamInfo.m_flRed = 255.0f;;
			beamInfo.m_flGreen = 255.0f;
			beamInfo.m_flBlue = 255.0f;
			beamInfo.m_nSegments = 8;
			beamInfo.m_bRenderable = true;
			beamInfo.m_nFlags = (FBEAM_ONLYNOISEONCE|FBEAM_SHADEOUT);
			
			beams->CreateBeamPoints( beamInfo );*/
		}
	}

	void OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );
		if ( updateType == DATA_UPDATE_CREATED )
		{
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output : RenderGroup_t
	//-----------------------------------------------------------------------------
	RenderGroup_t GetRenderGroup( void )
	{
		return RENDER_GROUP_TRANSLUCENT_ENTITY;
	}

private:
	bool m_bActive;
};

STUB_WEAPON_CLASS_IMPLEMENT( weapon_meme, C_WeaponMeme );

IMPLEMENT_CLIENTCLASS_DT( C_WeaponMeme, DT_WeaponMeme, CWeaponMeme )
	RecvPropInt( RECVINFO(m_bActive) ),
END_RECV_TABLE()

