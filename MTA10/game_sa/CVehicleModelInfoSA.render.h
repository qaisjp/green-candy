/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CVehicleModelInfoSA.render.h
*  PURPOSE:     Vehicle model info rendering module
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _CVehicleModelInfoSA_render_H
#define _CVehicleModelInfoSA_render_H

void __cdecl CacheVehicleRenderCameraSettings( RwObject *obj );
void __cdecl ClearVehicleRenderChains( void );
void __cdecl ExecuteVehicleRenderChains( void );

void __cdecl SetVehicleColorFlags( class CVehicleSAInterface *vehicle );

void __cdecl RpClumpSetupVehicleMaterials( RpClump *clump, class CVehicleSA *gameVehicle );
void __cdecl RpClumpRestoreVehicleMaterials( RpClump *clump );

void VehicleModelInfoRender_Init( void );
void VehicleModelInfoRender_Shutdown( void );

#endif //_CVehicleModelInfoSA_render_H