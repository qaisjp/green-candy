/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/luadefs/luaped.cpp
*  PURPOSE:     Ped Lua instancing
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

static LUA_DECLARE( setRotation )
{
    CVector rot;

    LUA_ARGS_BEGIN;
    argStream.ReadVector( rot );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetRotationDegrees( rot );
    LUA_SUCCESS;
}

static LUA_DECLARE( getRotation )
{
    CVector rot;
    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetRotationDegrees( rot );

    lua_pushnumber( L, rot[0] );
    lua_pushnumber( L, rot[1] );
    lua_pushnumber( L, rot[2] );
    return 3;
}

static LUA_DECLARE( setHealth )
{
    float health;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( health );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetHealth( health );
    LUA_SUCCESS;
}

static LUA_DECLARE( getHealth )
{
    lua_pushnumber( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetHealth() );
    return 1;
}

static LUA_DECLARE( isDead )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsDead() );
    return 1;
}

static LUA_DECLARE( setModel )
{
    unsigned short model;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( model );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetModel( model );
    LUA_SUCCESS;
}

static LUA_DECLARE( getModel )
{
    lua_pushnumber( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetModel() );
    return 1;
}

static LUA_DECLARE( addClothes )
{
    const char *texture, *model;
    unsigned char type;

    LUA_ARGS_BEGIN;
    argStream.ReadString( texture );
    argStream.ReadString( model );
    argStream.ReadNumber( type );
    LUA_ARGS_END;

    CClientPed *ped = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )));
    ped->GetClothes()->AddClothes( texture, model, type, false );
    ped->RebuildModel( true );
    LUA_SUCCESS;
}

static LUA_DECLARE( getClothes )
{
    unsigned char type;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( type );
    LUA_ARGS_END;

    SPlayerClothing *clothes = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetClothes()->GetClothing( type );

    LUA_CHECK( clothes );

    lua_pushstring( L, clothes->szTexture );
    lua_pushstring( L, clothes->szModel );
    return 2;
}

static LUA_DECLARE( removeClothes )
{
    unsigned char type;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( type );
    LUA_ARGS_END;

    CClientPed *ped = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )));
    ped->GetClothes()->RemoveClothes( type, false );
    ped->RebuildModel( true );
    LUA_SUCCESS;
}

static LUA_DECLARE( setControlState )
{
    const char *control;
    bool state;

    LUA_ARGS_BEGIN;
    argStream.ReadString( control );
    argStream.ReadBool( state );
    LUA_ARGS_END;

    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_Pad.SetControlState( control, state ) );
    return 1;
}

static LUA_DECLARE( getControlState )
{
    const char *control;
    bool state;

    LUA_ARGS_BEGIN;
    argStream.ReadString( control );
    LUA_ARGS_END;

    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->m_Pad.GetControlState( control, state ) && state );
    return 1;
}

static LUA_DECLARE( getAmmoInClip )
{
    unsigned char slot;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( slot );
    LUA_ARGS_END;

    CClientPed *ped = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )));

    if ( slot == 0xFF )
        slot = ped->GetCurrentWeaponSlot();

    CWeapon *weapon = ped->GetWeapon( (eWeaponSlot)slot );

    LUA_CHECK( weapon );

    lua_pushnumber( L, weapon->GetAmmoInClip() );
    return 1;
}

static LUA_DECLARE( getTotalAmmo )
{
    unsigned char slot;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( slot );
    LUA_ARGS_END;

    CClientPed *ped = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )));

    if ( slot == 0xFF )
        slot = ped->GetCurrentWeaponSlot();

    CWeapon *weapon = ped->GetWeapon( (eWeaponSlot)slot );

    LUA_CHECK( weapon );

    lua_pushnumber( L, weapon->GetAmmoTotal() );
    return 1;
}

static LUA_DECLARE( setWeaponSlot )
{
    unsigned char slot;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( slot );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetCurrentWeaponSlot( (eWeaponSlot)slot );
    LUA_SUCCESS;
}

static LUA_DECLARE( getWeaponSlot )
{
    lua_pushnumber( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetCurrentWeaponSlot() );
    return 1;
}

static LUA_DECLARE( getWeapon )
{
    unsigned char slot;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( slot );
    LUA_ARGS_END;

    CClientPed *ped = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )));

    if ( slot == 0xFF )
        slot = ped->GetCurrentWeaponSlot();

    CWeapon *weapon = ped->GetWeapon( (eWeaponSlot)slot );

    LUA_CHECK( weapon );

    lua_pushnumber( L, weapon->GetType() );
    return 1;
}

static LUA_DECLARE( setArmor )
{
    float armor;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( armor );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetArmor( armor );
    LUA_SUCCESS;
}

static LUA_DECLARE( getArmor )
{
    lua_pushnumber( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetArmor() );
    return 1;
}

static LUA_DECLARE( getContactElement )
{
    CClientEntity *element = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetContactEntity();

    if ( !element )
        return 0;

    element->PushStack( L );
    return 1;
}

static LUA_DECLARE( getFightingStyle )
{
    lua_pushnumber( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetFightingStyle() );
    return 1;
}

static LUA_DECLARE( getMoveState )
{
    std::string state;

    LUA_CHECK( ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetMovementState( state ) );

    lua_pushlstring( L, state.c_str(), state.size() );
    return 1;
}

static LUA_DECLARE( getVehicle )
{
    CClientVehicle *veh = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetOccupiedVehicle();

    if ( !veh )
        return 0;

    veh->PushStack( L );
    return 1;
}

static LUA_DECLARE( getVehicleSeat )
{
    lua_pushnumber( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetOccupiedVehicleSeat() );
    return 1;
}

static LUA_DECLARE( getTarget )
{
    CClientEntity *target = ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetTargetedEntity();

    if ( !target )
        return 0;

    target->PushStack( L );
    return 1;
}

static LUA_DECLARE( giveJetPack )
{
    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetHasJetPack( true );
    return 0;
}

static LUA_DECLARE( hasJetPack )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->HasJetPack() );
    return 1;
}

static LUA_DECLARE( removeJetPack )
{
    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetHasJetPack( false );
    return 0;
}

static LUA_DECLARE( setChoking )
{
    bool state;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( state );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetChoking( state );
    LUA_SUCCESS;
}

static LUA_DECLARE( isChoking )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsChoking() );
    return 1;
}

static LUA_DECLARE( setDoingGangDriveby )
{
    bool state;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( state );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetDoingGangDriveby( state );
    LUA_SUCCESS;
}

static LUA_DECLARE( isDoingGangDriveby )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsDoingGangDriveby() );
    return 1;
}

static LUA_DECLARE( isDucked )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsDucked() );
    return 1;
}

static LUA_DECLARE( isInWater )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsInWater() );
    return 1;
}

static LUA_DECLARE( isOnGround )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsOnGround() );
    return 1;
}

static LUA_DECLARE( isInVehicle )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsInVehicle() );
    return 1;
}

static LUA_DECLARE( setAnimation )
{
    const char *block, *anim;
    int time;
    bool loop, updatePos, interruptable, freezeLastFrame;

    LUA_ARGS_BEGIN;
    argStream.ReadString( block, NULL );
    argStream.ReadString( anim, NULL );
    argStream.ReadNumber( time, -1 );
    argStream.ReadBool( loop, true ); argStream.ReadBool( updatePos, true ); argStream.ReadBool( interruptable, true );
    argStream.ReadBool( freezeLastFrame, true );
    LUA_ARGS_END;

    CAnimBlock *b = g_pGame->GetAnimManager()->GetAnimationBlock( block );

    LUA_CHECK( block );

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->RunNamedAnimation( b, anim, time, loop, updatePos, interruptable, freezeLastFrame );
    LUA_SUCCESS;
}

static LUA_DECLARE( setAnimationProgress )
{
    const char *animName;
    float progress;

    LUA_ARGS_BEGIN;
    argStream.ReadString( animName, NULL );
    argStream.ReadNumber( progress, 0.0f );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetAnimationProgress( animName, progress );
    LUA_SUCCESS;
}

static LUA_DECLARE( setHeadless )
{
    bool state;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( state );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetHeadless( state );
    LUA_SUCCESS;
}

static LUA_DECLARE( isHeadless )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsHeadless() );
    return 1;
}

static LUA_DECLARE( setOnFire )
{
    bool state;

    LUA_ARGS_BEGIN;
    argStream.ReadBool( state );
    LUA_ARGS_END;

    ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->SetOnFire( state );
    LUA_SUCCESS;
}

static LUA_DECLARE( isOnFire )
{
    lua_pushboolean( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->IsOnFire() );
    return 1;
}

static LUA_DECLARE( getStat )
{
    unsigned short stat;

    LUA_ARGS_BEGIN;
    argStream.ReadNumber( stat );
    LUA_ARGS_END;

    lua_pushnumber( L, ((CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 )))->GetStat( stat ) );
    return 1;
}

static const luaL_Reg ped_interface_light[] =
{
    LUA_METHOD( setRotation ),
    LUA_METHOD( getRotation ),
    LUA_METHOD( setHealth ),
    LUA_METHOD( getHealth ),
    LUA_METHOD( isDead ),
    LUA_METHOD( setModel ),
    LUA_METHOD( getModel ),
    LUA_METHOD( addClothes ),
    LUA_METHOD( getClothes ),
    LUA_METHOD( removeClothes ),
    LUA_METHOD( setControlState ),
    LUA_METHOD( getControlState ),
    LUA_METHOD( getAmmoInClip ),
    LUA_METHOD( getTotalAmmo ),
    LUA_METHOD( setWeaponSlot ),
    LUA_METHOD( getWeaponSlot ),
    LUA_METHOD( getWeapon ),
    LUA_METHOD( setArmor ),
    LUA_METHOD( getArmor ),
    LUA_METHOD( getContactElement ),
    LUA_METHOD( getFightingStyle ),
    LUA_METHOD( getMoveState ),
    LUA_METHOD( getVehicle ),
    LUA_METHOD( getVehicleSeat ),
    LUA_METHOD( getTarget ),
    LUA_METHOD( giveJetPack ),
    LUA_METHOD( hasJetPack ),
    LUA_METHOD( removeJetPack ),
    LUA_METHOD( setChoking ),
    LUA_METHOD( isChoking ),
    LUA_METHOD( setDoingGangDriveby ),
    LUA_METHOD( isDoingGangDriveby ),
    LUA_METHOD( isDucked ),
    LUA_METHOD( isInWater ),
    LUA_METHOD( isOnGround ),
    LUA_METHOD( isInVehicle ),
    LUA_METHOD( setAnimation ),
    LUA_METHOD( setAnimationProgress ),
    LUA_METHOD( setHeadless ),
    LUA_METHOD( isHeadless ),
    LUA_METHOD( setOnFire ),
    LUA_METHOD( isOnFire ),
    LUA_METHOD( getStat ),
    { NULL, NULL }
};

LUA_DECLARE( luaconstructor_ped )
{
    CClientPed *ped = (CClientPed*)lua_touserdata( L, lua_upvalueindex( 1 ) );

    ILuaClass& j = *lua_refclass( L, 1 );
    j.SetTransmit( LUACLASS_PED, ped );

    j.RegisterLightInterface( L, ped_interface_light, ped );

    lua_pushlstring( L, "ped", 3 );
    lua_setfield( L, LUA_ENVIRONINDEX, "__type" );
    return 0;
}