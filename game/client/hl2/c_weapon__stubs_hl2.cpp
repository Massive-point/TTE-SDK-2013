//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon );

STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon );
#ifdef TACTICALTHOTS
STUB_WEAPON_CLASS( weapon_bbar, WeaponBootyBlasterAR, C_HLSelectFireMachineGun );
#else
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun );
#endif
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage );

#ifndef HL2MP
STUB_WEAPON_CLASS( weapon_ar2, WeaponAR2, C_HLMachineGun );
STUB_WEAPON_CLASS( weapon_frag, WeaponFrag, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon );
#ifdef TACTICALTHOTS
STUB_WEAPON_CLASS( weapon_usp, WeaponUSP, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_m4a1, WeaponM4A1, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_dualpistol, WeaponDualPistol, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS( weapon_kar98, WeaponKar98, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_dmr, WeaponDMR, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_deagle, WeaponDeagle, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_striker, WeaponStriker, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_stg44, WeaponSTG44, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_tommy, WeaponTommy, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_lit, LitGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_lmg, WeaponLMG, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_grenadelauncher, WeaponGrenadeLauncher, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_flamethrower, WeaponFlamethower, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS( weapon_sunnyd, WeaponSunnyd, C_BaseHLCombatWeapon);
//STUB_WEAPON_CLASS( weapon_meme, WeaponMeme, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_flash, WeaponFlash, C_BaseHLCombatWeapon );
#endif
STUB_WEAPON_CLASS( weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_357, Weapon357, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon );
#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon );
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
#endif
#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS( weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage );
#endif
#endif


