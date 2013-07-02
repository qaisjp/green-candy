/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleSA.render.cpp
*  PURPOSE:     Vehicle interface rendering logic
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CBaseModelInfoSAInterface **ppModelInfo;

/*=========================================================
    CVehicleSAInterface::RenderPassengers

    Purpose:
        Renders all passengers of this vehicle if their
        rendering is not disabled.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x006D3D60
=========================================================*/
void CVehicleSAInterface::RenderPassengers( void )
{
    if ( m_driver && m_driver->IsDrivingVehicle() )
        m_driver->Render();

    for ( unsigned char n = 0; n < MAX_PASSENGERS; n++ )
    {
        CPedSAInterface *pass = m_passengers[n];

        if ( pass && pass->IsDrivingVehicle() )
            pass->Render();
    }
}

/*=========================================================
    CVehicleSAInterface::SetupRender

    Purpose:
        Prepares the rendering of the vehicle atomics by
        temporarily replacing its textures with remap
        ones.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x006D64F0
=========================================================*/
void CVehicleSAInterface::SetupRender( CVehicleSA *mtaVeh )
{
    CVehicleModelInfoSAInterface *info = (CVehicleModelInfoSAInterface*)ppModelInfo[m_model];

    (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 1 );

    if ( !m_unk38 )
        SetPlateTextureForRendering( info );

    // Load another paintjob TexDictionary if requested
    unsigned short remapId = m_queuePaintjob;

    if ( remapId != 0xFFFF )
    {
        // Possible Crashfix: If we attempt to load the same TexDictionary instance, ignore it
        if ( remapId == m_paintjobTxd )
        {
            m_queuePaintjob = 0xFFFF;
        }
        else if ( Streaming::GetModelLoadInfo( remapId ).m_eLoading == MODEL_LOADED )
        {
            if ( m_paintjobTexture )
            {
                (*ppTxdPool)->Get( m_paintjobTxd )->Dereference();

                m_paintjobTexture = NULL;
            }

            // Assign new paintjob
            m_paintjobTxd = remapId;
            m_queuePaintjob = 0xFFFF;

            CTxdInstanceSA *txdInst = (*ppTxdPool)->Get( remapId );

            txdInst->Reference();

            // Apply the vehicle body by the first texture
            m_paintjobTexture = txdInst->m_txd->GetFirstTexture();

#ifdef ROCKSTAR_GAME_LOGIC
            // Reset the first color to white
            if ( !IsColourRemapping() )
                m_color1 = 1;
#endif //ROCKSTAR_GAME_LOGIC
        }
        else
            pGame->GetStreaming()->RequestModel( DATA_TEXTURE_BLOCK + remapId, 8 );  // We need to request that texture container
    }
    else
        SetColourRemapping( false );

    info->SetRenderColor( m_color1, m_color2, m_color3, m_color4 );
    SetVehicleLightsFlags( this );

    // Store the body texture for rendering
    *(RwTexture**)0x00B4E47C = m_paintjobTexture;

    RpClumpSetupVehicleMaterials( (RpClump*)m_rwObject, mtaVeh );
}

/*=========================================================
    CVehicleSAInterface::LeaveRender

    Purpose:
        Unsets changes made in SetupRender to leave the rendering
        stage of this vehicle.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x006D0E20
=========================================================*/
void CVehicleSAInterface::LeaveRender( void )
{
    // Change texture stage
    (*ppRwInterface)->m_deviceCommand( (eRwDeviceCmd)20, 2 );

    // Restore clump data
    RpClumpRestoreVehicleMaterials( (RpClump*)m_rwObject );

    if ( !m_unk38 )
        RestoreLicensePlate( (CVehicleModelInfoSAInterface*)ppModelInfo[m_model] );
}

/*=========================================================
    CVehicleSAInterface::SetPlateTextureForRendering

    Purpose:
        Exchanges the license plate texture for the vehicle
        model for a rendering pass.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x006A2F00
=========================================================*/
static RwTexture *_originalPlateTexture = NULL;

void CVehicleSAInterface::SetPlateTextureForRendering( CVehicleModelInfoSAInterface *info )
{
    RpMaterial *plateMaterial = info->m_plateMaterial;

    if ( !plateMaterial )
        return;

    // Store the original texture in a global temporarily
    RwTexture *original = plateMaterial->m_texture;

    // Crashfix: we do not require an original plate texture
    if ( original )
        original->refs++;

    _originalPlateTexture = original;

    // Change the plate material texture to the new one
    plateMaterial->SetTexture( m_plateTexture );
}

/*=========================================================
    CVehicleSAInterface::RestoreLicensePlate

    Purpose:
        Puts in the original license plate texture into the
        vehicle model to clean up after rendering.
    Binary offsets:
        (1.0 US and 1.0 EU): 0x006A2F30
=========================================================*/
void CVehicleSAInterface::RestoreLicensePlate( CVehicleModelInfoSAInterface *info )
{
    RpMaterial *plateMaterial = info->m_plateMaterial;

    if ( !plateMaterial )
        return;

    // Restore original plate texture
    plateMaterial->SetTexture( _originalPlateTexture );

    // Bugfix: No matter if we had a plate texture or not.
    if ( _originalPlateTexture )
    {
        RwTextureDestroy( _originalPlateTexture );  // we derefence our "keep-reference"

        _originalPlateTexture = NULL;
    }
}