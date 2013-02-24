/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CObjectSA.cpp
*  PURPOSE:     Object entity
*  DEVELOPERS:  Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Christian Myhre Lundheim <>
*               The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

dynamicObjectData g_dynObjData[MAX_DYNAMIC_OBJECT_DATA];
extern CBaseModelInfoSAInterface **ppModelInfo;

static char tempBuffer[512];

// Multi-functional configuration line retriver
static const char* GetConfigLine( CFile *file )
{
    if ( !file->GetString( tempBuffer, sizeof( tempBuffer ) ) )
        return NULL;
    
    char *buf = tempBuffer;

    for ( buf = tempBuffer; *buf; buf++ )
    {
        const char c = *buf;

        if ( c < ' ' || c == ',' )
            *buf = ' ';
    }
    
    // Offset until we start at valid offset
    for ( buf = tempBuffer; *buf && *buf <= ' '; buf++ );

    return buf;
}

static void __cdecl _Objects_LoadAttributes( const char *filepath, bool unused )
{
    dynamicObjectData& firstObj = g_dynObjData[0];
    firstObj.mass = 99999;
    firstObj.turnMass = 99999;
    firstObj.airResistance = 0.99000001f;
    firstObj.elasticity = 0.1f;
    firstObj.percSubmerged = 1.6f;
    firstObj.uproot = 0;
    firstObj.CDMult = 1;
    firstObj.CDEff = 0;
    firstObj.SpCDR = 0;
    firstObj.CamAv = 2;

    dynamicObjectData *obj = &g_dynObjData[1];
    *obj = firstObj;
    obj->CamAv = 0;

    obj = &g_dynObjData[2];
    *obj = firstObj;
    obj->SpCDR = 4;

    obj = &g_dynObjData[3];
    *obj = firstObj;
    obj->SpCDR = 4;
    obj->CamAv = 0;

    obj = &g_dynObjData[4];
    *obj = firstObj;
    obj->SpCDR = 5;

    CFile *file = OpenGlobalStream( filepath, "rb" );

    obj++;

    unsigned int num = 5;

    while ( const char *line = GetConfigLine( file ) )
    {
        const char c = *line;

        if ( c == '\0' || c == ';' )
            continue;

        if ( c == '*' )
            break;

        memset( obj, 0, sizeof(*obj) );

        char name[0x100];
        float percSubmerged;
        unsigned int CDEff;
        unsigned int SpCDR;
        unsigned int CamAv;
        unsigned int Expl;
        unsigned int fxType;
        char fxName[0x100];

        // Very big + powerful sscanf :D
        sscanf( line, "%s %f %f %f %f %f %f %f %d %d %d %d %d %f %f %f %s %f %f %f %f %f %d %d",    // fixed gun mode and spark parameters (R* slip-up)
            name, &obj->mass, &obj->turnMass, &obj->airResistance, &obj->elasticity,
            &percSubmerged, &obj->uproot, &obj->CDMult, &CDEff, &SpCDR, &CamAv, &Expl, &fxType,
            &obj->fxOffset[0], &obj->fxOffset[1], &obj->fxOffset[2], fxName,
            &obj->smashMult, &obj->smashVelocity[0], &obj->smashVelocity[1], &obj->smashVelocity[2], &obj->smashRand,
            &obj->b_gun, &obj->b_spk );

        obj->effDef = NULL;
        obj->fxType = fxType;

        if ( obj->fxType )
            obj->effDef = pParticleSystem->GetBlueprintByName( fxName );

        obj->SpCDR = SpCDR;
        obj->CDEff = CDEff;
        obj->CamAv = CamAv;
        obj->explosion = Expl;

        obj->percSubmerged = 100.0f / percSubmerged * obj->mass * 0.008f;

        CBaseModelInfoSAInterface *model = CStreaming__GetModelByHash( pGame->GetKeyGen()->GetUppercaseKey( name ), NULL );

        if ( !model )
            continue;

        if ( obj->mass == firstObj.mass && obj->CDMult == firstObj.CDMult && obj->CDEff == firstObj.CDEff )
        {
            // Some optimizations, apparrently?
            if ( obj->SpCDR == firstObj.SpCDR )
            {
                model->m_dynamicIndex = obj->CamAv == 0 ? 1 : 0;
                continue;
            }
            else if ( obj->SpCDR == g_dynObjData[1].SpCDR )
            {
                model->m_dynamicIndex = obj->CamAv == 0 ? 3 : 2;
                continue;
            }
        }

        for ( unsigned int n = 0; n < num; n++ )
        {
            dynamicObjectData& simDef = g_dynObjData[n];

            if ( obj->mass == simDef.mass && obj->turnMass == simDef.turnMass && obj->airResistance == simDef.airResistance &&
                 obj->elasticity == simDef.elasticity && obj->percSubmerged == simDef.percSubmerged && obj->uproot == simDef.uproot &&
                 obj->CDMult == simDef.CDMult && obj->CDEff == simDef.CDEff && obj->SpCDR == simDef.SpCDR && obj->CamAv == simDef.CamAv &&
                 obj->explosion == simDef.explosion && obj->fxType == simDef.fxType && obj->fxOffset == simDef.fxOffset && 
                 obj->effDef == simDef.effDef && obj->smashMult == simDef.smashMult && obj->smashVelocity == simDef.smashVelocity &&
                 obj->smashRand == simDef.smashRand && obj->b_gun == simDef.b_gun && obj->b_spk == simDef.b_spk )
            {
                model->m_dynamicIndex = (short)n;
                goto reloop;
            } 
        }

        if ( num != MAX_DYNAMIC_OBJECT_DATA )
        {
            model->m_dynamicIndex = (short)num++;
            obj++;
        }

reloop:
        continue;
    }

    delete file;
}

static void __cdecl _Object_PrepareDynamicPhysics( unsigned short model, CObjectSAInterface *obj )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[model];

    if ( info->m_dynamicIndex == -1 )
    {
        obj->m_mass = 99999;
        obj->m_turnMass = 99999;
        obj->m_nodeFlags &= ~0x02;
        obj->m_nodeFlags |= 0x80000C;
        obj->m_dynData = &g_dynObjData[0];
        return;
    }

    dynamicObjectData *data = &g_dynObjData[info->m_dynamicIndex];

    obj->m_dynData = data;
    obj->m_mass = data->mass;
    obj->m_turnMass = data->turnMass;
    obj->m_airResistance = data->airResistance;
    obj->m_elasticity = data->elasticity;
    obj->m_buoyancyConstant = data->percSubmerged;
    obj->m_collDamEffect = data->CDEff;

    if ( data->mass >= 99998 )
    {
        obj->m_nodeFlags &= ~0x02;
        obj->m_nodeFlags |= 0x0C;

        if ( data->CDEff == 0 )
            obj->m_nodeFlags |= 0x800000;
    }

    if ( data->SpCDR == 6 )
    {
        obj->m_nodeFlags |= 0x20;
        obj->m_objFlags &= ~0xC000;
    }
    else if ( data->SpCDR == 7 )
    {
        obj->m_objFlags &= ~0x8000;
        obj->m_objFlags |= 0x4000;
        obj->m_nodeFlags |= 0x2C;
    }
    else if ( data->SpCDR == 8 )
    {
        obj->m_nodeFlags |= 0x40;
        obj->m_centerOfMass = CVector( 0, 0, info->m_pColModel->m_bounds.vecBoundMin[2] );
    }
    else if ( data->SpCDR == 9 )
    {
        obj->m_nodeFlags &= ~0x02;
        obj->m_nodeFlags |= 0x80;
    }
}

void Objects_Init()
{
    // Load the attributes ourselves.
    // We need a seperate structure to save original attributes to, restoring them at mod unload.
    // Introducing a new callback framework to game_sa: mod init + mod unload
    HookInstall( 0x005B5360, (DWORD)_Objects_LoadAttributes, 5 );
    HookInstall( 0x005A2D00, (DWORD)_Object_PrepareDynamicPhysics, 5 );

    *(dynamicObjectData**)0x0059F85D = g_dynObjData;
}

void Objects_Shutdown()
{
    
}

CObjectSA::CObjectSA( CObjectSAInterface *obj )
{
    DEBUG_TRACE("CObjectSA::CObjectSA( CObjectSAInterface *obj )");

    m_pInterface = obj;

    // Setup some flags
    m_doNotRemoveFromGame = false;
    obj->m_unk40 = 6;
    BOOL_FLAG( obj->m_entityFlags, ENTITY_DISABLESTREAMING, true );

    m_poolIndex = (*ppObjectPool)->GetIndex( obj );
    mtaObjects[m_poolIndex] = this;

    m_ucAlpha = 255;

    CheckForGangTag ();
}

CObjectSA::~CObjectSA()
{
    DEBUG_TRACE("CObjectSA::~CObjectSA()");

    mtaObjects[m_poolIndex] = NULL;
}

void* CObjectSA::operator new ( size_t )
{
    return mtaObjectPool->Allocate();
}

void CObjectSA::operator delete ( void *ptr )
{
    return mtaObjectPool->Free( (CObjectSA*)ptr );
}

void CObjectSA::Explode()
{
    DWORD dwFunc = FUNC_CObject_Explode;
    DWORD dwThis = (DWORD)GetInterface();

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CObjectSA::Break()
{
    // Works only if health is 0
    DWORD dwFunc = 0x5A0D90;
    DWORD dwThis = (DWORD)GetInterface ();

    _asm
    {
        mov     ecx, dwThis
        call    dwFunc
    }
}

void CObjectSA::SetModelIndex( unsigned short ulModel )
{
    m_pInterface->SetModelIndex( ulModel );

    CheckForGangTag();
}

CRpAtomicSA* CObjectSA::CloneAtomic() const
{
    return new CRpAtomicSA( RpAtomicClone( (RpAtomic*)GetInterface()->m_rwObject ) );
}

void CObjectSA::CheckForGangTag()
{
    switch( GetModelIndex() )
    {
    case 1524: case 1525: case 1526: case 1527:
    case 1528: case 1529: case 1530: case 1531:
        m_bIsAGangTag = true;
        break;
    default:
        m_bIsAGangTag = false; 
        break;
    }
}

void* CObjectSAInterface::operator new( size_t )
{
    return (*ppObjectPool)->Allocate();
}

void CObjectSAInterface::operator delete( void *ptr )
{
    (*ppObjectPool)->Free( (CObjectSAInterface*)ptr );
}