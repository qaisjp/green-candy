/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.cpp
*  PURPOSE:     RenderWare mapping to Grand Theft Auto: San Andreas
*               and miscellaneous rendering functions
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               arc_
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

extern CGameSA * pGame;
extern CBaseModelInfoSAInterface **ppModelInfo;

RwInterface **ppRwInterface = (RwInterface**)0x00C97B24;

//
// STxdAction stores the current frame txd create and destroy events
//
struct STxdAction
{
    bool bAdd;
    ushort usTxdId;
};

std::vector < STxdAction > ms_txdActionList;

//
// Hooks for creating txd create and destroy events
//
#define HOOKPOS_CTxdStore_SetupTxdParent       0x731D55
DWORD RETURN_CTxdStore_SetupTxdParent =        0x731D5B;
void HOOK_CTxdStore_SetupTxdParent ();

#define HOOKPOS_CTxdStore_RemoveTxd       0x731E90
DWORD RETURN_CTxdStore_RemoveTxd =        0x731E96;
void HOOK_CTxdStore_RemoveTxd ();


// RwFrameForAllObjects struct and callback used to replace dynamic vehicle parts
struct SReplaceParts
{
    const char *szName;                         // name of the part you want to replace (e.g. 'door_lf' or 'door_rf')
    unsigned char ucIndex;                      // index counter for internal usage (0 is the 'ok' part model, 1 is the 'dam' part model)
    RpAtomicContainer *pReplacements;           // replacement atomics
    unsigned int uiReplacements;                // number of replacements
};
static bool ReplacePartsCB ( RwObject * object, SReplaceParts * data )
{
    RpAtomic * Atomic = (RpAtomic*) object;
    char szAtomicName[16] = {0};

    // iterate through our loaded atomics
    for ( unsigned int i = 0; i < data->uiReplacements; i++ )
    {
        // get the correct atomic name based on the object # in our parent frame
        if ( data->ucIndex == 0 )
            snprintf ( &szAtomicName[0], 16, "%s_ok", data->szName );
        else
            snprintf ( &szAtomicName[0], 16, "%s_dam", data->szName );

        // see if we have such an atomic in our replacement atomics array
        if ( strncmp ( &data->pReplacements[i].szName[0], &szAtomicName[0], 16 ) == 0 )
        {
            // if so, override the geometry
            RpAtomicSetGeometry ( Atomic, data->pReplacements[i].atomic->geometry, 0 );

            // and copy the matrices
            RwFrameCopyMatrix ( Atomic->m_parent, data->pReplacements[i].atomic->m_parent );

            object = (RwObject*) data->pReplacements[i].atomic;
            data->ucIndex++;
        }
    }

    return true;
}

// RpClumpForAllAtomicsPointer callback used to add atomics to a vehicle
static bool AddAllAtomicsCB (RpAtomic * atomic, RpClump * data)
{
    RwFrame * pFrame = data->m_parent;

    // add the atomic to the frame
    RpAtomicSetFrame ( atomic, pFrame );
    RpClumpAddAtomic ( data, atomic );
    return true;
}

// RpClumpForAllAtomicsPointer struct and callback used to replace all wheels with a given wheel model
struct SReplaceWheels
{
    const char *szName;                         // name of the new wheel model
    RpClump *pClump;                            // the vehicle's clump
    RpAtomicContainer *pReplacements;           // replacement atomics
    unsigned int uiReplacements;                // number of replacements
};
static bool ReplaceWheelsCB (RpAtomic * atomic, SReplaceWheels * data)
{
    RwFrame * Frame = atomic->m_parent;

    // find our wheel atomics
    if ( strncmp ( &Frame->szName[0], "wheel_lf_dummy", 16 ) == 0 || strncmp ( &Frame->szName[0], "wheel_rf_dummy", 16 ) == 0 || 
         strncmp ( &Frame->szName[0], "wheel_lm_dummy", 16 ) == 0 || strncmp ( &Frame->szName[0], "wheel_rm_dummy", 16 ) == 0 || 
         strncmp ( &Frame->szName[0], "wheel_lb_dummy", 16 ) == 0 || strncmp ( &Frame->szName[0], "wheel_rb_dummy", 16 ) == 0 )
    {
        // find a replacement atomic
        for ( unsigned int i = 0; i < data->uiReplacements; i++ ) {
            // see if it's name is equal to the specified wheel
            if ( strncmp ( &data->pReplacements[i].szName[0], data->szName, 16 ) == 0 ) {
                // clone our wheel atomic
                RpAtomic * WheelAtomic = RpAtomicClone ( data->pReplacements[i].atomic );
                RpAtomicSetFrame ( WheelAtomic, Frame );

                // add it to the clump
                RpClumpAddAtomic ( data->pClump, WheelAtomic );

                // delete the current atomic
                RpClumpRemoveAtomic ( data->pClump, atomic );
            }
        }
    }

    return true;
}

// RpClumpForAllAtomicsPointer struct and callback used to replace all atomics for a vehicle
struct SReplaceAll
{
    RpClump *pClump;                            // the vehicle's clump
    RpAtomicContainer *pReplacements;           // replacement atomics
    unsigned int uiReplacements;                // number of replacements
};
static bool ReplaceAllCB (RpAtomic * atomic, SReplaceAll * data)
{
    RwFrame *frame = atomic->m_parent;

    if ( frame == NULL )
        return true;

    // find a replacement atomic
    for ( unsigned int i = 0; i < data->uiReplacements; i++ ) {
        // see if we can find an atomic with the same name
        if ( strncmp ( &data->pReplacements[i].szName[0], &Frame->szName[0], 16 ) == 0 ) {
            // copy the matrices
            RwFrameCopyMatrix ( frame, RpGetFrame ( data->pReplacements[i].atomic ) );

            // set the new atomic's frame to the current one
            RpAtomicSetFrame ( data->pReplacements[i].atomic, Frame );

            // override all engine and material related callbacks and pointers
            data->pReplacements[i].atomic->renderCallback   = atomic->renderCallback;
            data->pReplacements[i].atomic->frame            = atomic->frame;
            data->pReplacements[i].atomic->render           = atomic->render;
            data->pReplacements[i].atomic->interpolation    = atomic->interpolation;
            data->pReplacements[i].atomic->info             = atomic->info;

            // add the new atomic to the vehicle clump
            RpClumpAddAtomic ( data->pClump, data->pReplacements[i].atomic );

            // remove the current atomic
            RpClumpRemoveAtomic ( data->pClump, atomic );
        }
    }

    return true;
}

// RpClumpForAllAtomicsPointer struct and callback used to load the atomics from a specific clump into a container
struct SLoadAtomics
{
    RpAtomicContainer *pReplacements;           // replacement atomics
    unsigned int uiReplacements;                // number of replacements
};
static bool LoadAtomicsCB (RpAtomic * atomic, SLoadAtomics * data)
{
    RwFrame *Frame = atomic->m_parent;

    // add the atomic to the container
    data->pReplacements [ data->uiReplacements ].atomic = atomic;
    strncpy ( &data->pReplacements [ data->uiReplacements ].szName[0], &Frame->szName[0], 16 );

    // and increment the counter
    data->uiReplacements++;

    return true;
}

CRenderWareSA::CRenderWareSA ( eGameVersion version )
{
    m_pfnWatchCallback = NULL;

    // Version dependant addresses
    switch ( version )
    {
    // VERSION 1.0 EU ADDRESSES
    case VERSION_EU_10:
        RwAllocAligned                      = (RwAllocAligned_t)                        0x0072F4C0;
        RwFreeAligned                       = (RwFreeAligned_t)                         0x0072F4F0;
        RwCreateExtension                   = (RwCreateExtension_t)                     0x007CCE80;
        RwErrorGet                          = (RwErrorGet_t)                            0x008088C0;
        RwStreamOpen                        = (RwStreamOpen_t)                          0x007ECF30;
        RwStreamFindChunk                   = (RwStreamFindChunk_t)                     0x007ED310;
        RwStreamReadBlocks                  = (RwStreamReadBlocks_t)                    0x007ECA10;
        RwStreamReadTexture                 = (RwStreamReadTexture_t)                   0x00730E60;
        RwStreamClose                       = (RwStreamClose_t)                         0x007ECE60;
        RwFrameCreate                       = (RwFrameCreate_t)                         0x007F0450;
        RwFrameGetLTM                       = (RwFrameGetLTM_t)                         0x007F09D0;
        RwFrameSetIdentity                  = (RwFrameSetIdentity_t)                    0x007F10F0;
        RwFrameAddChild                     = (RwFrameAddChild_t)                       0x007F0B40;
        RwFrameRemoveChild                  = (RwFrameRemoveChild_t)                    0x007F0D10;
        RwFrameTranslate                    = (RwFrameTranslate_t)                      0x007F0E70;
        RwFrameCloneHierarchy               = (RwFrameCloneHierarchy_t)                 0x007F05E0;
        RwFrameScale                        = (RwFrameScale_t)                          0x007F0F10;
        RwFrameOrient                       = (RwFrameOrient_t)                         0x007F2010;
        RwCameraClone                       = (RwCameraClone_t)                         0x007EF3F0;
        RpClumpClone                        = (RpClumpClone_t)                          0x00749FC0;
        RpClumpAddAtomic                    = (RpClumpAddAtomic_t)                      0x0074A4E0;
        RpClumpRemoveAtomic                 = (RpClumpRemoveAtomic_t)                   0x0074A510;
        RpClumpAddLight                     = (RpClumpAddLight_t)                       0x0074A540;
        RpClumpGetBoneTransform             = (RpClumpGetBoneTransform_t)               0x00735360;
        RpClumpStreamRead                   = (RpClumpStreamRead_t)                     0x0074B470;
        RpClumpGetLastAtomic                = (RpClumpGetLastAtomic_t)                  0x00734820;
        RpClumpSetupFrameCallback           = (RpClumpSetupFrameCallback_t)             0x00733750;
        RpClumpGetNumAtomics                = (RpClumpGetNumAtomics_t)                  0x00749930;
        RpClumpDestroy                      = (RpClumpDestroy_t)                        0x0074A360;
        RwAnimationInit                     = (RwAnimationInit_t)                       0x007CD5E0;
        RwSkeletonUpdate                    = (RwSkeletonUpdate_t)                      0x007C5210;
        RpAtomicCreate                      = (RpAtomicCreate_t)                        0x00749CA0;
        RpAtomicClone                       = (RpAtomicClone_t)                         0x00749EB0;
        RpAtomicSetFrame                    = (RpAtomicSetFrame_t)                      0x0074BF70;
        RpAtomicSetupObjectPipeline         = (RpAtomicSetupObjectPipeline_t)           0x005D7F00;
        RpAtomicSetupVehiclePipeline        = (RpAtomicSetupVehiclePipeline_t)          0x005D5B20;
        RpAtomicRender                      = (RpAtomicRender_t)                        0x00749210;
        RpAtomicRenderEx                    = (RpAtomicRenderEx_t)                      0x00732480;
        RpAtomicSetGeometry                 = (RpAtomicSetGeometry_t)                   0x00749D90;
        RpAtomicDestroy                     = (RpAtomicDestroy_t)                       0x00749E10;
        RwObjectFrameRender                 = (RwObjectFrameRender_t)                   0x00805750;
        RwTexDictionaryCreate               = (RwTexDictionaryCreate_t)                 0x007F3640;
        RwTexDictionaryStreamRead           = (RwTexDictionaryStreamRead_t)             0x00804C70;
        RwTexDictionaryGetCurrent           = (RwTexDictionaryGetCurrent_t)             0x007F3AD0;
        RwTexDictionarySetCurrent           = (RwTexDictionarySetCurrent_t)             0x007F3AB0;
        RwTexDictionaryForAllTextures       = (RwTexDictionaryForAllTextures_t)         0x007F3770;
        RwTexDictionaryFindNamedTexture     = (RwTexDictionaryFindNamedTexture_t)       0x007F3A30;
        RwTexDictionaryAddTexture           = (RwTexDictionaryAddTexture_t)             0x007F39C0;
        RwTexDictionaryDestroy              = (RwTexDictionaryDestroy_t)                0x007F36E0;
        RwTextureCreate                     = (RwTextureCreate_t)                       0x007F3800;
        RwTextureUnlinkFromDictionary       = (RwTextureUnlinkFromDictionary_t)         0x007F3A00;
        RwTextureDestroy                    = (RwTextureDestroy_t)                      0x007F3860;
        RpGeometryCreate                    = (RpGeometryCreate_t)                      0x0074CAE0;
        RpGeometryGetAnimation              = (RpGeometryGetAnimation_t)                0x007C7590;
        RpGeometryTriangleSetVertexIndices  = (RpGeometryTriangleSetVertexIndices_t)    0x0074C6E0;
        RpGeometryTriangleSetMaterial       = (RpGeometryTriangleSetMaterial_t)         0x0074C710;
        RpGeometryUnlock                    = (RpGeometryUnlock_t)                      0x0074C850;
        RpGeometryLock                      = (RpGeometryLock_t)                        0x0074C820;
        RpGeometryTransform                 = (RpGeometryTransform_t)                   0x0074C030;
        RpGeometryDestroy                   = (RpGeometryDestroy_t)                     0x0074CD10;
        RwMatrixCreate                      = (RwMatrixCreate_t)                        0x007F2A90;
        RwMatrixInvert                      = (RwMatrixInvert_t)                        0x007F20B0;
        RwMatrixTranslate                   = (RwMatrixTranslate_t)                     0x007F2490;
        RwMatrixScale                       = (RwMatrixScale_t)                         0x007F2300;
        RwMatrixUnknown                     = (RwMatrixUnknown_t)                       0x007326D0;
        RpMaterialCreate                    = (RpMaterialCreate_t)                      0x0074D9E0;
        RpMaterialDestroy                   = (RpMaterialDestroy_t)                     0x0074DA70;
        RwV3dNormalize                      = (RwV3dNormalize_t)                        0x007ED9F0;
        RwV3dTransformVector                = (RwV3dTransformVector_t)                  0x007EDE00;
        RwIm3DTransform                     = (RwIm3DTransform_t)                       0x007EF490; 
        RwIm3DRenderIndexedPrimitive        = (RwIm3DRenderIndexedPrimitive_t)          0x007EF590;
        RwIm3DEnd                           = (RwIm3DEnd_t)                             0x007EF560;
        RpLightCreate                       = (RpLightCreate_t)                         0x00752160;
        RpLightSetRadius                    = (RpLightSetRadius_t)                      0x00751AC0;
        RpLightSetColor                     = (RpLightSetColor_t)                       0x00751AE0;
        RwRasterUnlock                      = (RwRasterUnlock_t)                        0x007FAF00;
        RwRasterLock                        = (RwRasterLock_t)                          0x007FB310; 
        RwRasterCreate                      = (RwRasterCreate_t)                        0x007FB270;
        RpWorldAddAtomic                    = (RpWorldAddAtomic_t)                      0x00750FE0;
        RpWorldAddClump                     = (RpWorldAddClump_t)                       0x00751350;
        RpWorldAddLight                     = (RpWorldAddLight_t)                       0x00751960;
        RpPrtStdGlobalDataSetStreamEmbedded = (RpPrtStdGlobalDataSetStreamEmbedded_t)   0x0041B350;
        RwPrefetch                          = (RwPrefetch_t)                            0x0072F480;

        SetTextureDict                      = (SetTextureDict_t)                        0x007319C0;
        LoadClumpFile                       = (LoadClumpFile_t)                         0x005371F0;
        LoadModel                           = (LoadModel_t)                             0x0040C6B0;
        LoadCollisionModel                  = (LoadCollisionModel_t)                    0x00537580;
        LoadCollisionModelVer2              = (LoadCollisionModelVer2_t)                0x00537EE0;
        LoadCollisionModelVer3              = (LoadCollisionModelVer3_t)                0x00537CE0;
        CTxdStore_LoadTxd                   = (CTxdStore_LoadTxd_t)                     0x00731DD0;
        CTxdStore_GetTxd                    = (CTxdStore_GetTxd_t)                      0x00408340;
        CTxdStore_RemoveTxd                 = (CTxdStore_RemoveTxd_t)                   0x00731E90;
        CTxdStore_RemoveRef                 = (CTxdStore_RemoveRef_t)                   0x00731A30;
        CTxdStore_AddRef                    = (CTxdStore_AddRef_t)                      0x00731A00; 
        CClothesBuilder_CopyTexture         = (CClothesBuilder_CopyTexture_t)           0x005A5730;

        // Those hooks are for EU 1.0 really
        HookInstall ( HOOKPOS_CTxdStore_SetupTxdParent, (DWORD)HOOK_CTxdStore_SetupTxdParent, 6 );
        HookInstall ( HOOKPOS_CTxdStore_RemoveTxd, (DWORD)HOOK_CTxdStore_RemoveTxd, 6 );
        break; 

    // VERSION 1.0 US ADDRESSES
    case VERSION_US_10:
        RwStreamFindChunk                   = (RwStreamFindChunk_t)                     0x007ED2D0;
        RpClumpStreamRead                   = (RpClumpStreamRead_t)                     0x0074B420;
        RwErrorGet                          = (RwErrorGet_t)                            0x00808880;
        RwStreamOpen                        = (RwStreamOpen_t)                          0x007ECEF0;
        RwStreamClose                       = (RwStreamClose_t)                         0x007ECE20;
        RpClumpDestroy                      = (RpClumpDestroy_t)                        0x0074A310;
        RpClumpGetNumAtomics                = (RpClumpGetNumAtomics_t)                  0x007498E0;
        RwFrameTranslate                    = (RwFrameTranslate_t)                      0x007F0E30;
        RwFrameAddChild                     = (RwFrameAddChild_t)                       0x007F0B00;
        RpClumpAddAtomic                    = (RpClumpAddAtomic_t)                      0x0074A490;
        RpAtomicSetFrame                    = (RpAtomicSetFrame_t)                      0x0074BF20;
        RwTexDictionaryStreamRead           = (RwTexDictionaryStreamRead_t)             0x00804C30;
        RwTexDictionaryGetCurrent           = (RwTexDictionaryGetCurrent_t)             0x007F3A90;
        RwTexDictionarySetCurrent           = (RwTexDictionarySetCurrent_t)             0x007F3A70;
        RwTexDictionaryForAllTextures       = (RwTexDictionaryForAllTextures_t)         0x007F3730;
        RwTexDictionaryAddTexture           = (RwTexDictionaryAddTexture_t)             0x007F3980;
        RpPrtStdGlobalDataSetStreamEmbedded = (RpPrtStdGlobalDataSetStreamEmbedded_t)   0x0041B350;
        RpClumpRemoveAtomic                 = (RpClumpRemoveAtomic_t)                   0x0074A4C0;
        RpAtomicClone                       = (RpAtomicClone_t)                         0x00749E60;
        RwTexDictionaryFindNamedTexture     = (RwTexDictionaryFindNamedTexture_t)       0x007F39F0;
        RwFrameRemoveChild                  = (RwFrameRemoveChild_t)                    0x007F0CD0;
        RpAtomicDestroy                     = (RpAtomicDestroy_t)                       0x00749DC0;
        RpAtomicSetGeometry                 = (RpAtomicSetGeometry_t)                   0x00749D40;
        RpWorldAddAtomic                    = (RpWorldAddAtomic_t)                      0x00750F90;
        RpGeometryCreate                    = (RpGeometryCreate_t)                      0x0074CA90;
        RpGeometryTriangleSetVertexIndices  = (RpGeometryTriangleSetVertexIndices_t)    0x0074C690;
        RpGeometryUnlock                    = (RpGeometryUnlock_t)                      0x0074C800;
        RpGeometryLock                      = (RpGeometryLock_t)                        0x0074C7D0;
        RpAtomicCreate                      = (RpAtomicCreate_t)                        0x00749C50;
        RwFrameCreate                       = (RwFrameCreate_t)                         0x007F0410;
        RpGeometryTransform                 = (RpGeometryTransform_t)                   0x0074BFE0;
        RwFrameSetIdentity                  = (RwFrameSetIdentity_t)                    0x007F10B0;
        RwMatrixCreate                      = (RwMatrixCreate_t)                        0x007F2A50;
        RwMatrixTranslate                   = (RwMatrixTranslate_t)                     0x007F2450;
        RwMatrixScale                       = (RwMatrixScale_t)                         0x007F22C0;
        RpGeometryTriangleSetMaterial       = (RpGeometryTriangleSetMaterial_t)         0x0074C6C0;
        RpMaterialCreate                    = (RpMaterialCreate_t)                      0x0074D990;
        RpGeometryDestroy                   = (RpGeometryDestroy_t)                     0x0074CCC0;
        RpMaterialDestroy                   = (RpMaterialDestroy_t)                     0x0074DA20;
        RwV3dNormalize                      = (RwV3dNormalize_t)                        0x007ED9B0;
        RwIm3DTransform                     = (RwIm3DTransform_t)                       0x007EF450;
        RwIm3DRenderIndexedPrimitive        = (RwIm3DRenderIndexedPrimitive_t)          0x007EF550;
        RwIm3DEnd                           = (RwIm3DEnd_t)                             0x007EF520;
        RwMatrixInvert                      = (RwMatrixInvert_t)                        0x007F2070;
        RpWorldAddClump                     = (RpWorldAddClump_t)                       0x00751300;
        RwFrameScale                        = (RwFrameScale_t)                          0x007F0ED0;
        RwV3dTransformVector                = (RwV3dTransformVector_t)                  0x007EDDC0;
        RpLightCreate                       = (RpLightCreate_t)                         0x00752110;
        RpClumpAddLight                     = (RpClumpAddLight_t)                       0x0074A4F0;
        RpLightSetRadius                    = (RpLightSetRadius_t)                      0x00751A70;
        RpWorldAddLight                     = (RpWorldAddLight_t)                       0x00751910;
        RpLightSetColor                     = (RpLightSetColor_t)                       0x00751A90;
        RwCameraClone                       = (RwCameraClone_t)                         0x007EF3B0;
        RpClumpClone                        = (RpClumpClone_t)                          0x00749F70;
        RwTexDictionaryDestroy              = (RwTexDictionaryDestroy_t)                0x007F36A0;
        RwTextureDestroy                    = (RwTextureDestroy_t)                      0x007F3820;
        RwRasterUnlock                      = (RwRasterUnlock_t)                        0x007FAEC0;
        RwRasterLock                        = (RwRasterLock_t)                          0x007FB2D0;
        RwRasterCreate                      = (RwRasterCreate_t)                        0x007FB230;
        RwTextureCreate                     = (RwTextureCreate_t)                       0x007F37C0;

        SetTextureDict                      = (SetTextureDict_t)                        0x007319C0;
        LoadClumpFile                       = (LoadClumpFile_t)                         0x005371F0;
        LoadModel                           = (LoadModel_t)                             0x0040C6B0;
        LoadCollisionModel                  = (LoadCollisionModel_t)                    0x00537580;
        LoadCollisionModelVer2              = (LoadCollisionModelVer2_t)                0x00537EE0;
        LoadCollisionModelVer3              = (LoadCollisionModelVer3_t)                0x00537CE0;
        CTxdStore_LoadTxd                   = (CTxdStore_LoadTxd_t)                     0x00731DD0;
        CTxdStore_GetTxd                    = (CTxdStore_GetTxd_t)                      0x00408340;
        CTxdStore_RemoveTxd                 = (CTxdStore_RemoveTxd_t)                   0x00731E90;
        CTxdStore_RemoveRef                 = (CTxdStore_RemoveRef_t)                   0x00731A30;
        CTxdStore_AddRef                    = (CTxdStore_AddRef_t)                      0x00731A00;
        CClothesBuilder_CopyTexture         = (CClothesBuilder_CopyTexture_t)           0x005A5730;
        break;
    }
}

// Reads and parses a DFF file specified by a path (szDFF) into a CModelInfo identified by the object id (usModelID)
// usModelID == 0 means no collisions will be loaded (be careful! seems crashy!)
RpClump* CRenderWareSA::ReadDFF( const char *path, unsigned short id )
{
    // open the stream
    RwStream *streamModel = RwStreamOpen( STREAM_TYPE_FILENAME, STREAM_MODE_READ, path );

    if ( streamModel == NULL )
        return NULL;

    // DFF header id: 0x10
    // find our dff chunk
    if ( !RwStreamFindChunk( streamModel, 0x10, NULL, NULL ) )
        return NULL;

    // rockstar's collision hack: set the global particle emitter to the modelinfo pointer of this model
    //#pragma message(__LOC__ "(IJs) RpPrtStdGlobalDataSetStreamEmbedded R* hack is unstable. Seems to work when forced to packer vehicle id (443).")
    if ( id != 0 )
        RpPrtStdGlobalDataSetStreamEmbedded( ppModelInfo[id] );

    // read the clump with all its extensions
    RpClump *pClump = RpClumpStreamRead( streamModel );

    // reset collision hack
    if ( id != 0 )
        RpPrtStdGlobalDataSetStreamEmbedded( NULL );

    // close the stream
    RwStreamClose( streamModel, NULL );
    return pClump;
}

// Replaces a vehicle model
void CRenderWareSA::ReplaceVehicleModel( RpClump * pNew, unsigned short usModelID )
{
    // get the modelinfo array
    CBaseModelInfoSAInterface *pModel = ppModelInfo[usModelID];

    if ( !pModel )
        return;

    if ( pModel->GetModelType() != MODEL_VEHICLE )
        return;

    CClumpModelInfoSAInterface *info = (CClumpModelInfoSAInterface*)pModel;

    if ( pNew != info->m_rwClump )
    {
        // Delete the old clump
        info->DeleteRwObject();

        // Load a new model association
        info->SetClump( pNew );
    }
}

// Reads and parses a COL3 file
CColModel* CRenderWareSA::ReadCOL( CFile *file )
{
    // Create a new CColModel
    CColModelSA* pColModel = new CColModelSA ();
    ColModelFileHeader header;

    file->Read( &header, sizeof(ColModelFileHeader), 1 );
    
    // Load the col model
    if ( header.version[0] == 'C' && header.version[1] == 'O' && header.version[2] == 'L' )
    {
        unsigned char* pModelData = new unsigned char [ header.size - 0x18 ];
        fread ( pModelData, header.size - 0x18, 1, pFile );

        switch( header.version[3] )
        {
        case 'L':
            LoadCollisionModel ( pModelData, pColModel->GetInterface (), NULL );
            break;
        case '2':
            LoadCollisionModelVer2 ( pModelData, header.size - 0x18, pColModel->GetInterface (), NULL );
            break;
        case '3':
            LoadCollisionModelVer3 ( pModelData, header.size - 0x18, pColModel->GetInterface (), NULL );
            break;
        }

        delete [] pModelData;
    }
    else
    {
        delete pColModel;

        return NULL;
    }

    // Return the collision model
    return pColModel;
}

// Positions the front seat by reading out the vector from the 'ped_frontseat' atomic in the clump (RpClump*)
// and changing the vector in the CModelInfo class identified by the model id (usModelID)
bool CRenderWareSA::PositionFrontSeat( RpClump *pClump, unsigned short usModelID )
{
    // get the modelinfo array (+5Ch contains a pointer to vehicle specific dummy data)
    CVehicleSeatPlacementSAInterface *seats = ((CVehicleModelInfoSAInterface*)ppModelInfo[usModelID])->m_seatPlacement;

    // read out the 'ped_frontseat' frame
    RwFrame *pPedFrontSeat = pClump->m_parent->FindChildByName( "ped_frontseat" );

    if ( pPedFrontSeat == NULL )
        return false;

    // in the vehicle specific dummy data, +30h contains the front seat vector
    seats->m_seatOffset[4] = pPedFrontSeat->m_modelling.pos;
    return true;
}

// Loads all atomics from a clump into a container struct and returns the number of atomics it loaded
unsigned int CRenderWareSA::LoadAtomics ( RpClump * pClump, RpAtomicContainer * pAtomics )
{
    // iterate through all atomics in the clump
    SLoadAtomics data = {0};
    data.pReplacements = pAtomics;
    pClump->ForAllAtomics( LoadAtomicsCB, &data );
    
    // return the number of atomics that were added to the container
    return data.uiReplacements;
}

// Replaces all atomics for a specific model
typedef struct
{
    unsigned short usTxdID;
    unsigned short modelID;
} SAtomicsReplacer;

// Based on FUNC_AtomicsReplacer
static void RwAtomicInsertClump( unsigned short modelID, RpAtomic *atomic )
{
    CBaseModelInfoSAInterface *info = ppModelInfo[modelID];
    CAtomicModelInfoSA *ainfo = info->GetAtomicModelInfo();
    bool unk;
    void *unk2;

    // MTA: we create a copy of the atomic
    RpAtomic *newatom = RpAtomicClone( atomic );

    // This possibly adds the reference to the texture, we should reven this
    ((void (*)(const char*, void*&, bool&))0x005370A0)( atomic->m_parent->m_nodeName, unk2, unk );

    newatom->SetRenderCallback( NULL );

    if ( !unk )
        ainfo->GetDamageAtomicModelInfo()->SetupPipeline( newatom );
    else
        ainfo->SetAtomic( newatom );

#if 0
    RpClumpRemoveAtomic( clump, atomic );
#endif

    RpAtomicSetFrame( newatom, RwFrameCreate() );
    
    newatom->SetExtendedRenderFlags( modelID );
}

// TODO: identify this runtime; it might be better to assign single atomics!
static bool AtomicsReplacer( RpAtomic* pAtomic, SAtomicsReplacer* pData )
{
    RwAtomicInsertClump( pData->modelID, pAtomic );

    // The above function adds a reference to the model's TXD. Remove it again.
    (*ppTxdEntry)->Get( pData->usTxdID )->Dereference();
    return true;
}

void CRenderWareSA::ReplaceAllAtomicsInModel( RpClump *src, unsigned short id )
{
    // Replace the atomics
    SAtomicsReplacer data;
    data.usTxdID = ppModelInfo[id]->m_textureDictionary;
    data.modelID = id;

    src->ForAllAtomics( AtomicsReplacer, &data );
}

// Replaces all atomics in a vehicle
void CRenderWareSA::ReplaceAllAtomicsInClump ( RpClump * pDst, RpAtomicContainer * pAtomics, unsigned int uiAtomics )
{
    SReplaceAll data;
    data.pClump = pDst;
    data.pReplacements = pAtomics;
    data.uiReplacements = uiAtomics;
    pDst->ForAllAtomics( ReplaceAllCB, &data );
}

// Replaces the wheels in a vehicle
void CRenderWareSA::ReplaceWheels ( RpClump * pClump, RpAtomicContainer * pAtomics, unsigned int uiAtomics, const char * szWheel )
{
    // get the clump's frame
    RwFrame *pFrame = pClump->m_parent;

    SReplaceWheels data;
    data.pClump = pClump;
    data.pReplacements = pAtomics;
    data.uiReplacements = uiAtomics;
    data.szName = szWheel;
    pClump->ForAllAtomics( ReplaceWheelsCB, &data );
}

// Repositions an atomic
void CRenderWareSA::RepositionAtomic ( RpClump * pDst, RpClump * pSrc, const char * szName )
{
    RwFrameCopyMatrix( pDst->m_parent->FindChildByName( szName ), pSrc->m_parent->FindChildByName( szName ) );
}

// Adds the atomics from a source clump (pSrc) to a destination clump (pDst)
void CRenderWareSA::AddAllAtomics ( RpClump * pDst, RpClump * pSrc )
{
    pSrc->ForAllAtomics( AddAllAtomicsCB, pDst );
}

// Replaces dynamic parts of the vehicle (models that have two different versions: 'ok' and 'dam'), such as doors
// szName should be without the part suffix (e.g. 'door_lf' or 'door_rf', and not 'door_lf_dummy')
bool CRenderWareSA::ReplacePartModels ( RpClump * pClump, RpAtomicContainer * pAtomics, unsigned int uiAtomics, const char * szName )
{
    // get the part's dummy name
    char szDummyName[16] = {0};
    snprintf ( &szDummyName[0], 16, "%s_dummy", szName );

    // get the part's dummy frame
    RwFrame * pPart = pClump->m_parent->FindChildByName( szDummyName );

    if ( pPart == NULL )
        return false;

    // now replace all the objects in the frame
    SReplaceParts data = {0};
    data.pReplacements = pAtomics;
    data.uiReplacements = uiAtomics;
    data.szName = szName;
    pPart->ForAllObjects( ReplacePartsCB, &data );

    return true;
}

// Replaces a CColModel for a specific object identified by the object id (usModelID)
void CRenderWareSA::ReplaceCollisions ( CColModel* pCol, unsigned short usModelID )
{
    DWORD *pPool = (DWORD *) ARRAY_ModelInfo;
    CColModelSA * pColModel = (CColModelSA*) pCol;
    CModelInfoSA * pModelInfoSA = (CModelInfoSA*)(pGame->GetModelInfo ( usModelID ));

    // Apply some low-level hacks (copies the old col area and sets a flag)
    DWORD pColModelInterface = (DWORD)pColModel->GetInterface ();
    DWORD pOldColModelInterface = *((DWORD *) pPool [ usModelID ] + 20);
    MemOrFast < BYTE > ( pPool [usModelID ] + 0x13, 8 );
    MemPutFast < BYTE > ( pColModelInterface + 40, *((BYTE *)( pOldColModelInterface + 40 )) );

    // TODO: It seems that on entering the game, when this function is executed, the modelinfo array for this
    // model is still zero, leading to a crash!
    pModelInfoSA->IsLoaded ();
}

bool CRenderWareSA::ListContainsNamedTexture ( std::list < RwTexture* >& list, const char* szTexName )
{
    std::list < RwTexture* >::iterator it;
    for ( it = list.begin (); it != list.end (); it++ )
    {
        if ( !stricmp ( szTexName, (*it)->name ) )
            return true;
    }
    return false;
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::GetTXDIDForModelID
//
// Get a TXD ID associated with the model ID
//
////////////////////////////////////////////////////////////////
ushort CRenderWareSA::GetTXDIDForModelID ( ushort usModelID )
{
    if ( usModelID >= 20000 && usModelID < 25000 )
    {
        // Get global TXD ID instead
        return usModelID - 20000;
    }
    else
    {
        // Get the CModelInfo's TXD ID

        // Ensure valid
        if ( usModelID >= 20000 || !((CBaseModelInfoSAInterface**)ARRAY_ModelInfo)[usModelID] )
            return 0;

        return ppModelInfo[usModelID]->m_textureDictionary;
    }
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::InitWorldTextureWatch
//
// Setup texture watch callback
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::InitWorldTextureWatch ( PFN_WATCH_CALLBACK pfnWatchCallback )
{
    m_pfnWatchCallback = pfnWatchCallback;
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::AddWorldTextureWatch
//
// Begin watching for changes to the d3d resource associated with this texture name 
//
// Returns false if shader/match already exists
//
////////////////////////////////////////////////////////////////
bool CRenderWareSA::AddWorldTextureWatch ( CSHADERDUMMY* pShaderData, const char* szMatch, float fOrderPriority )
{
    SShadInfo* pNewShadInfo = CreateShadInfo ( pShaderData, szMatch, fOrderPriority );
    if ( !pNewShadInfo )
        return false;

    // Step through texinfo list looking for all matches
    for ( std::list < STexInfo > ::iterator iter = m_TexInfoList.begin () ; iter != m_TexInfoList.end () ; ++iter )
    {
        STexInfo* pTexInfo = &*iter;
        if ( WildcardMatch ( pNewShadInfo->strMatch, pTexInfo->strTextureName ) )
        {
            // Check previous association
            if ( SShadInfo* pPrevShadInfo = pTexInfo->pAssociatedShadInfo )
            {
				// Check priority
                if ( pPrevShadInfo->fOrderPriority > pNewShadInfo->fOrderPriority )
                    continue;

                // Remove previous association
                BreakAssociation ( pPrevShadInfo, pTexInfo );
            }

            // Add new association
            MakeAssociation ( pNewShadInfo, pTexInfo );
        }
    }
    return true;
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::MakeAssociation
//
// Make the texture use the shader
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::MakeAssociation ( SShadInfo* pShadInfo, STexInfo* pTexInfo )
{
    assert ( !pTexInfo->pAssociatedShadInfo );
    assert ( !MapContains ( pShadInfo->associatedTexInfoMap, pTexInfo ) );
    pTexInfo->pAssociatedShadInfo = pShadInfo;
    pShadInfo->associatedTexInfoMap.insert ( pTexInfo );

    if ( m_pfnWatchCallback )
        m_pfnWatchCallback ( pShadInfo->pShaderData, pTexInfo->pD3DData, NULL );
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::BreakAssociation
//
// Stop the texture using the shader
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::BreakAssociation ( SShadInfo* pShadInfo, STexInfo* pTexInfo )
{
    assert ( pTexInfo->pAssociatedShadInfo == pShadInfo );
    assert ( MapContains ( pShadInfo->associatedTexInfoMap, pTexInfo ) );
    pTexInfo->pAssociatedShadInfo = NULL;
    MapRemove ( pShadInfo->associatedTexInfoMap, pTexInfo );
    if ( m_pfnWatchCallback )
        m_pfnWatchCallback ( pShadInfo->pShaderData, NULL, pTexInfo->pD3DData );
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::RemoveWorldTextureWatch
//
// End watching for changes to the d3d resource associated with this texture name
//
// When shadinfo removed:
//      unassociate with matches, let each texture find a new latest match
//
// If szMatch is NULL, remove all usage of the shader
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::RemoveWorldTextureWatch ( CSHADERDUMMY* pShaderData, const char* szMatch )
{
    SString strMatch = SStringX ( szMatch ).ToLower ();
    // Find shadInfo
    for ( std::list < SShadInfo > ::iterator iter = m_ShadInfoList.begin () ; iter != m_ShadInfoList.end () ; )
    {
        SShadInfo* pShadInfo = &*iter;
        if ( pShadInfo->pShaderData == pShaderData && ( !szMatch || pShadInfo->strMatch == strMatch ) )
        {
            std::vector < STexInfo* > reassociateList;

            // Unassociate with all matches
            while ( !pShadInfo->associatedTexInfoMap.empty () )
            {
                STexInfo* pTexInfo = *pShadInfo->associatedTexInfoMap.begin ();
                BreakAssociation ( pShadInfo, pTexInfo );

                // reassociateList for earlier shaders
                reassociateList.push_back ( pTexInfo );
            }

            OnDestroyShadInfo ( pShadInfo );
            iter = m_ShadInfoList.erase ( iter );

            // Allow unassociated texinfos to find new associations
            for ( uint i = 0 ; i < reassociateList.size () ; i++ )
            {
                STexInfo* pTexInfo = reassociateList [ i ];
                FindNewAssociationForTexInfo ( pTexInfo );
            }

			// If a match string was supplied, there should be only one in the list, so we can stop here
            if ( szMatch )
                break;
        }
        else
            ++iter;
    }
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::RemoveWorldTextureWatchByContext
//
// End watching for changes to the d3d resource associated with this context 
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::RemoveWorldTextureWatchByContext ( CSHADERDUMMY* pShaderData )
{
    RemoveWorldTextureWatch ( pShaderData, NULL );
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::PulseWorldTextureWatch
//
// Update watched textures status
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::PulseWorldTextureWatch ( void )
{
    // Got through ms_txdActionList
    for ( std::vector < STxdAction >::const_iterator iter = ms_txdActionList.begin () ; iter != ms_txdActionList.end () ; ++iter )
    {
        const STxdAction& action = *iter;
        if ( action.bAdd )
        {
            //
            // New txd has been loaded
            //

            // Get list of texture names and data to add

            // Note: If txd has been unloaded since, textureList will be empty
            std::vector < RwTexture* > textureList;
            GetTxdTextures ( textureList, action.usTxdId );

            for ( std::vector < RwTexture* > ::iterator iter = textureList.begin () ; iter != textureList.end () ; iter++ )
            {
                RwTexture* texture = *iter;
                const char* szTextureName = texture->name;
                CD3DDUMMY* pD3DData = texture->raster ? (CD3DDUMMY*)texture->raster->renderResource : NULL;
                AddActiveTexture ( action.usTxdId, szTextureName, pD3DData );
            }
        }
        else
        {
            //
            // Txd has been unloaded
            //

            RemoveTxdActiveTextures ( action.usTxdId );
        }
    }

    ms_txdActionList.clear ();
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::AddActiveTexture
//
// Create a texinfo for the texture and find a best match shader for it
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::AddActiveTexture ( ushort usTxdId, const SString& strTextureName, CD3DDUMMY* pD3DData )
{
    STexInfo* pTexInfo = CreateTexInfo ( usTxdId, strTextureName, pD3DData );
    FindNewAssociationForTexInfo ( pTexInfo );
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::CreateTexInfo
//
//
//
////////////////////////////////////////////////////////////////
STexInfo* CRenderWareSA::CreateTexInfo ( ushort usTxdId, const SString& strTextureName, CD3DDUMMY* pD3DData )
{
    // Create texinfo
    m_TexInfoList.push_back ( STexInfo ( usTxdId, strTextureName, pD3DData ) );
    STexInfo* pTexInfo = &m_TexInfoList.back ();

    // Add to lookup maps
    SString strUniqueKey ( "%d_%s", pTexInfo->usTxdId, *pTexInfo->strTextureName );
    assert ( !MapContains ( m_UniqueTexInfoMap, strUniqueKey ) );
    MapSet ( m_UniqueTexInfoMap, strUniqueKey, pTexInfo );

	// This assert fails when using engine txd replace functions - TODO find out why
    //assert ( !MapContains ( m_D3DDataTexInfoMap, pTexInfo->pD3DData ) );
    MapSet ( m_D3DDataTexInfoMap, pTexInfo->pD3DData, pTexInfo );
    return pTexInfo;
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::OnDestroyTexInfo
//
//
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::OnDestroyTexInfo ( STexInfo* pTexInfo )
{
    // Remove from lookup maps
    SString strUniqueKey ( "%d_%s", pTexInfo->usTxdId, *pTexInfo->strTextureName );
    assert ( MapContains ( m_UniqueTexInfoMap, strUniqueKey ) );
    MapRemove ( m_UniqueTexInfoMap, strUniqueKey );

	// This assert fails when using engine txd replace functions - TODO find out why
    //assert ( MapContains ( m_D3DDataTexInfoMap, pTexInfo->pD3DData ) );
    MapRemove ( m_D3DDataTexInfoMap, pTexInfo->pD3DData );
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::CreateShadInfo
//
//
//
////////////////////////////////////////////////////////////////
SShadInfo* CRenderWareSA::CreateShadInfo (  CSHADERDUMMY* pShaderData, const SString& strMatch, float fOrderPriority )
{
    // Create shadinfo
    m_ShadInfoList.push_back ( SShadInfo ( strMatch, pShaderData, fOrderPriority ) );
    SShadInfo* pShadInfo = &m_ShadInfoList.back ();

    // Check for uniqueness
    SString strUniqueKey ( "%08x_%s", pShadInfo->pShaderData, *pShadInfo->strMatch );
    if ( MapContains ( m_UniqueShadInfoMap, strUniqueKey ) )
    {
        // Remove again if not unique
        m_ShadInfoList.pop_back ();
        return NULL;
    }

    // Set unique map
    MapSet ( m_UniqueShadInfoMap, strUniqueKey, pShadInfo );

    // Add to order map
    MapInsert ( m_orderMap, pShadInfo->fOrderPriority, pShadInfo );

    return pShadInfo;
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::OnDestroyShadInfo
//
//
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::OnDestroyShadInfo ( SShadInfo* pShadInfo )
{
    // Remove from lookup maps
    SString strUniqueKey ( "%08x_%s", pShadInfo->pShaderData, *pShadInfo->strMatch );
    assert ( MapContains ( m_UniqueShadInfoMap, strUniqueKey ) );
    MapRemove ( m_UniqueShadInfoMap, strUniqueKey );

    //  Remove from order map
    MapRemovePair ( m_orderMap, pShadInfo->fOrderPriority, pShadInfo );
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::FindNewAssociationForTexInfo
//
// Find best match for this texinfo
// Called when a texture is loaded or a shader is removed from a texture
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::FindNewAssociationForTexInfo ( STexInfo* pTexInfo )
{
    // Stepping backwards will bias high priority, and if the priorty is the same, bias the latest additions
    for ( std::multimap < float, SShadInfo* > ::reverse_iterator iter = m_orderMap.rbegin () ; iter != m_orderMap.rend () ; ++iter )
    {
        SShadInfo* pShadInfo = iter->second;
        if ( WildcardMatch ( pShadInfo->strMatch, pTexInfo->strTextureName ) )
        {
            // Found one
            MakeAssociation ( pShadInfo, pTexInfo );
            break;
        }
    }
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::RemoveTxdActiveTextures
//
// Called when a TXD is being unloaded.
// Delete texinfos which came from that TXD
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::RemoveTxdActiveTextures ( ushort usTxdId )
{
    // Find all TexInfo's for this txd
    for ( std::list < STexInfo > ::iterator iter = m_TexInfoList.begin () ; iter != m_TexInfoList.end () ; )
    {
        STexInfo* pTexInfo = &*iter;
        if ( pTexInfo->usTxdId == usTxdId )
        {
            // If texinfo has a shadinfo, unassociate
            if ( SShadInfo* pShadInfo = pTexInfo->pAssociatedShadInfo )
            {
                BreakAssociation ( pShadInfo, pTexInfo );
            }

            OnDestroyTexInfo ( pTexInfo );
            iter = m_TexInfoList.erase ( iter );
        }
        else
            ++iter;
    }
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::GetModelTextureNames
//
// Get list of texture names associated with the model
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::GetModelTextureNames ( std::vector < SString >& outNameList, ushort usModelID )
{
    outNameList.empty ();

    ushort usTxdId = GetTXDIDForModelID ( usModelID );

    if ( usTxdId == 0 )
        return;

    std::vector < RwTexture* > textureList;
    GetTxdTextures ( textureList, usTxdId );

    for ( std::vector < RwTexture* > ::iterator iter = textureList.begin () ; iter != textureList.end () ; iter++ )
    {
        outNameList.push_back ( (*iter)->name );
    }
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::GetTxdTextures
//
//
//
////////////////////////////////////////////////////////////////
void CRenderWareSA::GetTxdTextures ( std::vector < RwTexture* >& outTextureList, ushort usTxdId )
{
    if ( usTxdId == 0 )
        return;

    // Get the TXD corresponding to this ID
    SetTextureDict ( usTxdId );

    RwTexDictionary* pTXD = CTxdStore_GetTxd ( usTxdId );

    if ( pTXD )
    {
        RwTexDictionaryForAllTextures ( pTXD, StaticGetTextureCB, &outTextureList );
    }
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::StaticGetTextureCB
//
// Callback used in GetTxdTextures
//
////////////////////////////////////////////////////////////////
bool CRenderWareSA::StaticGetTextureCB ( RwTexture* texture, std::vector < RwTexture* >* pTextureList )
{
    pTextureList->push_back ( texture );
    return true;
}


////////////////////////////////////////////////////////////////
//
// CRenderWareSA::GetTextureName
//
//
//
////////////////////////////////////////////////////////////////
const SString& CRenderWareSA::GetTextureName ( CD3DDUMMY* pD3DData )
{
    STexInfo** ppTexInfo = MapFind ( m_D3DDataTexInfoMap, pD3DData );
    if ( ppTexInfo )
        return (*ppTexInfo)->strTextureName;
    static SString strDummy;
    return strDummy;
}


////////////////////////////////////////////////////////////////
//
//
//
// Hooks
//
//
//
////////////////////////////////////////////////////////////////

//
// Txd created
//

void _cdecl OnAddTxd ( DWORD dwTxdId )
{
    STxdAction action;
    action.bAdd = true;
    action.usTxdId = (ushort)dwTxdId;
    ms_txdActionList.push_back ( action );
}

// called from streaming on TXD create
void _declspec(naked) HOOK_CTxdStore_SetupTxdParent ()
{
    _asm
    {
        // Hooked from 731D55  6 bytes

        // eax - txd id
        pushad
        push eax
        call OnAddTxd
        add esp, 4
        popad

        // orig
        mov     esi, ds:00C8800Ch 
        jmp     RETURN_CTxdStore_SetupTxdParent  // 731D5B
    }
}


//
// Txd remove
//

void _cdecl OnRemoveTxd ( DWORD dwTxdId )
{
    STxdAction action;
    action.bAdd = false;
    action.usTxdId = (ushort)dwTxdId - 20000;
    ms_txdActionList.push_back ( action );
}

// called from streaming on TXD destroy
void _declspec(naked) HOOK_CTxdStore_RemoveTxd ()
{
    _asm
    {
        // Hooked from 731E90  6 bytes

        // esi - txd id + 20000
        pushad
        push esi
        call OnRemoveTxd
        add esp, 4
        popad

        // orig
        mov     ecx, ds:00C8800Ch
        jmp     RETURN_CTxdStore_RemoveTxd      // 731E96
    }
}

/////////////////////////////////////////

/*****************************************************************************
*
*   RenderWare Functions
*
*****************************************************************************/

RwAtomicRenderChainInterface *rwRenderChains = (RwAtomicRenderChainInterface*)0x00C88070;


RwMatrix* CRenderWareSA::AllocateMatrix()
{
    return new ( pRwInterface->m_allocStruct( pRwInterface->m_matrixInfo, 0x3000D ) ) RwMatrix();
}

static bool RwFrameGetChildCount( RwFrame *child, unsigned int *count )
{
    child->ForAllChildren( RwFrameGetChildCount, count );

    (*count)++;
    return true;
}

unsigned int RwFrame::CountChildren()
{
    unsigned int count = 0;

    ForAllChildren( RwFrameGetChildCount, &count );
    return count;
}

bool RwFrame::ForAllChildren( bool (*callback)( RwFrame* child, void *data ), void *data )
{
    RwFrame *child;

    for ( child = m_child; child; child = child->m_next )
    {
        if ( !callback( child, data ) )
            return false;
    }

    return true;
}

static bool RwFrameGetChild( RwFrame *child, RwFrame **dst )
{
    *dst = child;
    return false;
}

RwFrame* RwFrame::GetFirstChild()
{
    RwFrame *child;

    if ( ForAllChildren( RwFrameGetChild, &child ) )
        return NULL;

    return child;
}

struct _rwFrameFindName
{
    const char *name;
    RwFrame *result;
}

static bool RwFrameGetFreeByName( RwFrame *child, _rwFrameFindName *info )
{
    if ( child->m_hierarchyId || strcmp(child->m_nodeName, info->name) != 0 )
        return child->ForAllChildren( RwFrameGetFreeByName, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindFreeChildByName( const char *name )
{
    _rwFrameFindName info;

    info.name = name;

    if ( ForAllChildren( RwFrameGetFreeByName, &_rwFrameFindName ) )
        return NULL;

    return info.result;
}

static bool RwFrameGetByName( RwFrame *child, _rwFrameFindName *info )
{
    if ( strcmp(child->m_nodeName, info->name) != 0 )
        return child->ForAllChildren( RwFrameGetByName, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindChildByName( const char *name )
{
    _rwFrameFindName info;

    info.name = name;

    if ( ForAllChildren( RwFrameGetByName, &info ) )
        return NULL;

    return info.result;
}

struct _rwFrameFindHierarchy
{
    unsigned int    hierarchy;
    RwFrame*        result;
}

static bool RwFrameGetByHierarchy( RwFrame *child, _rwFrameFindHierarchy *info )
{
    if ( child->m_hierarchyId != info->hierarchy )
        return child->ForAllChildren( RwFrameGetByHierarchy, info );

    info->result = child;
    return false;
}

RwFrame* RwFrame::FindChildByHierarchy( unsigned int id )
{
    _rwFrameFindHierarchy info;

    info.hierarchy = id;

    if ( ForAllChildren( RwFrameGetByHierarchy, &info ) )
        return NULL;

    return info.result;
}

static bool RwFrameGetAnimHierarchy( RwFrame *frame, RpAnimHierarchy **anim )
{
    if ( frame->m_anim )
    {
        *anim = frame->m_anim;
        return false;
    }

    return frame->ForAllChildren( RwFrameGetAnimHierarchy, anim );
}

bool RwFrame::ForAllObjects( bool (*callback)( RwObject *object, void *data ), void *data )
{
    RwListEntry <RwObjectFrame> *child;

    for ( child = m_objects.root.next; child != &m_objects.root; child = child->next )
    {
        if ( !callback( (RwObject*)( (unsigned int)child - offsetof(RwObjectFrame, m_lFrame) ), data ) )
            return false;
    }

    return true;
}

static bool RwObjectGet( RwObject *child, RwObject **dst )
{
    *dst = child;
    return false;
}

RwObject* RwFrame::GetFirstObject()
{
    RwObject *obj;

    if ( ForAllObjects( RwObjectGet, &obj ) )
        return NULL;

    return obj;
}

static bool RwFrameObjectGetLast( RwObject *obj, RwObject **dst )
{
    *dst = obj;
    return true;
}

RwObject* RwFrame::GetLastObject()
{
    RwObject *obj = NULL;

    ForAllObjects( RwFrameObjectGetLast, &obj );
    return obj;
}

static bool RwFrameObjectGetVisibleLast( RwObject *obj, RwObject **dst )
{
    if ( obj->m_flags & RW_OBJ_VISIBLE )
        *dst = obj;

    return true;
}

RwObject* RwFrame::GetLastVisibleObject()
{
    RwObject *obj = NULL;

    ForAllObjects( RwFrameObjectGetVisibleLast, &obj );
    return atomic;
}

struct _rwObjectGetAtomic
{
    bool                (*routine)( RpAtomic *atomic, void *data );
    void*               data;
}

static bool RwObjectDoAtomic( RwObjectFrame *child, _rwObjectGetAtomic *info )
{
    // Check whether the object is a atomic
    if ( child->m_type != 0x14 )
        return true;

    return info->routine( (RpAtomic*)child, info->data );
}

bool RwFrame::ForAllAtomics( bool (*callback)( RpAtomic *atomic, void *data ), void *data )
{
    _rwObjectGetAtomic info;

    info.routine = callback;
    info.data = data;

    return ForAllObjects( RwObjectDoAtomic, &info );
}

static bool RwObjectGetAtomic( RpAtomic *atomic, RpAtomic **dst )
{
    *dst = atomic;
    return false;
}

RpAtomic* RwFrame::GetFirstAtomic()
{
    RpAtomic *atomic;

    if ( ForAllAtomics( RwObjectGetAtomic, &atomic ) )
        return NULL;

    return atomic;
}

static bool RwObjectAtomicSetVisibilityFlags( RpAtomic *atomic, unsigned short flags )
{
    atomic->ApplyVisibilityFlags( unsigned short flags );
    return true;
}

bool RwFrame::SetAtomicVisibility( unsigned short flags )
{
    ForAllAtomics( RwObjectAtomicSetVisibilityFlags, (void*)flags );
    return true;
}

static bool RwFrameAtomicBaseRoot( RpAtomic *atomic, RwFrame *root )
{
    RpAtomicSetFrame( atomic, root );
    return true;
}

static bool RwFrameBaseAtomicHierarchy( RwFrame *child, RwFrame *root )
{
    child->ForAllChildren( RwFrameBaseAtomicHierarchy, root );

    // Set all atomics to root
    child->ForAllAtomics( RwFrameAtomicBaseRoot, root );

    RwFrameCloneHierarchy( child );
    return true;
}

void RwFrame::BaseAtomicHierarchy()
{
    ForAllChildren( RwFrameBaseAtomicHierarchy, this );
}

struct _rwFrameVisibilityAtomics
{
    RpAtomic **primary;
    RpAtomic **secondary;
}

static bool RwFrameAtomicFindVisibility( RpAtomic *atomic, _rwFrameVisibilityAtomics *info )
{
    if ( atomic->GetVisibilityFlags() & 0x01 )
    {
        *info->primary = atomic;
        return true;
    }

    if ( atomic->GetVisibilityFlags() & 0x02 )
        *info->secondary = atomic;

    return true;
}

void RwFrame::FindVisibilityAtomics( RpAtomic **primary, RpAtomic **secondary )
{
    _rwFrameVisibilityAtomics info;

    info.primary = primary;
    info.secondary = secondary;

    ForAllAtomics( RwFrameAtomicFindVisibility, &info );
}

RpAnimHierarchy* RwFrame::GetAnimHierarchy()
{
    RpAnimHierarchy *anim;

    if ( m_anim )
        return m_anim;

    // We want false, since it has to interrupt == found
    if ( ForAllChildren( RwFrameGetAnimHierarchy, &anim ) )
        return NULL;

    return anim;
}

void RwFrame::RegisterRoot()
{
    if ( !(m_root->m_privateFlags & ( RW_OBJ_REGISTERED | 0x01 ) ) )
    {
        // Add it to the internal list
        LIST_INSERT( pRwInterface->m_nodeRoot.root, m_nodeRoot );

        m_root->m_privateFlags |= RW_OBJ_REGISTERED | 0x01;
    }

    m_privateFlags |= RW_OBJ_REGISTERED | RW_OBJ_HIERARCHY_CACHED;
}

bool RwTexDictionary::ForAllTextures( bool (*callback)( RwTexture *tex, void *ud ), void *ud )
{
    RwListEntry <RwTexture> *child;

    for ( child = textures.root.next; child != &textures.root; child = child->next )
    {
        if ( !callback( (RwTexture*)( (unsigned int)child - offsetof(RwTexture, TXDList) ), ud ) )
            return false;
    }

    return true;
}

static bool RwTexDictionaryGetFirstTexture( RwTexture *tex, void *ud )
{
    *(RwTexture**)ud = tex;
    return false;
}

RwTexture* RwTexDictionary::GetFirstTexture()
{
    RwTexture *tex;

    if ( ForAllTextures( RwTexDictionaryGetFirstTexture, &tex ) )
        return NULL;

    return tex;
}

struct _rwTexDictFind
{
    const char *name;
    RwTexture *tex;
};

static bool RwTexDictionaryFindTexture( RwTexture *tex, void *ud )
{
    _rwTexDictFind& find = *(_rwTexDictFind*)ud;

    if ( stricmp( tex->name, find.name ) != 0 )
        return true;

    find.tex = tex;
    return false;
}

RwTexture* RwTexDictionary::FindNamedTexture( const char *name )
{
    _rwTexDictFind find;
    find.name = name;

    if ( ForAllTextures( RwTexDictionaryFindTexture, name ) )
        return NULL;

    return find.tex;
}

void RwTexture::AddToDictionary( RwTexDictionary *_txd )
{
    if ( txd )
        LIST_REMOVE( TXDList );

    LIST_APPEND( _txd->textures.root, TXDList );

    txd = _txd;
}

void RwTexture::RemoveFromDictionary()
{
    if ( !txd )
        return;

    LIST_REMOVE( TXDList.root );

    txd = NULL;
}

RwStaticGeometry::RwStaticGeometry()
{
    m_count = 0;
    m_link = NULL;
}

RwRenderLink* RwStaticGeometry::AllocateLink( unsigned int count )
{
    if ( m_link )
        RwFreeAligned( m_link );

    m_count = count;
    return m_link = RwAllocAligned( ((count * sizeof(RwRenderLink) - 1) >> 6 + 1) << 6, 0x40 );
}

void RwStaticGeometry::ForAllLinks( void (*callback)( RwRenderLink *link, void *data ), void *data )
{
    RwRenderLink *link = m_link;
    unsigned int n;

    for ( n=0; n<m_count; link++ )
        callback( link, data );
}

bool RpAtomic::IsNight()
{
    if ( m_pipeline == RW_ATOMIC_RENDER_NIGHT )
        return true;

    if ( m_pipeline == RW_ATOMIC_RENDER_REFLECTIVE )
        return false;

    return m_geometry->m_nightColor && m_geometry->m_color;
}

void RpAtomic::SetRenderCallback( RpAtomicCallback callback )
{
    if ( !callback )
    {
        m_renderCallback = (RpAtomicCallback)RpAtomicRender;
        return;
    }

    m_renderCallback = callback;
}

void RpAtomic::ApplyVisibilityFlags( unsigned short flags )
{
    *(unsigned short*)&m_matrixFlags |= flags;
}

void RpAtomic::RemoveVisibilityFlags( unsigned short flags )
{
    *(unsigned short*)&m_matrixFlags &= ~flags;
}

unsigned short RpAtomic::GetVisibilityFlags()
{
    return *(unsigned short*)&m_matrixFlags;
}

void RpAtomic::SetExtendedRenderFlags( unsigned short flags )
{
    *(unsigned short*)&m_renderFlags = flags;
}

void RpAtomic::FetchMateria( RpMaterials *mats )
{
    unsigned int n;

    if ( !(GetVisibilityFlags() & 0x20) )
        return;

    for (n=0; n<m_geometry->m_linkedMateria->m_count; n++)
        mats->Add( m_geometry->m_linkedMateria->Get(n)->m_material );
}

RpMaterials::RpMaterials( unsigned int max )
{
    m_data = pRwInterface->m_malloc( sizeof(long) * max );

    m_max = max;
    m_entries = 0;
}

RpMaterials::~RpMaterials()
{
    unsigned int n;

    for (n=0; n<m_entries; n++)
        RpMaterialDestroy( m_data[n] );

    pRwInterface->m_free( m_entries );

    m_entries = 0;
    m_max = 0;
}

bool RpMaterials::Add( RpMaterial *mat )
{
    if ( m_entries == m_max )
        return false;   // We do not need RW error handlers anymore?

    // Reference it
    mat->m_refs++;

    m_data[ m_entries++ ] = mat;
    return true;
}

RwLinkedMaterial* RwLinkedMateria::Get( unsigned int index )
{
    if ( index >= m_count )
        return NULL;

    return (RwLinkedMaterial*)(this + 1) + index;
}

static bool RwAssignRenderLink( RwFrame *child, RwRenderLink **link )
{
    (*link)->m_context = child;
    (*link)++;

    child->ForAllChildren( RwAssignRenderLink, link );
    return true;
}

static void RwRenderLinkInit( RwRenderLink *link, void *data )
{
    RwFrame *frame = (RwFrame*)link->m_context;

    link->m_flags = 0;

    new (link->m_position) CVector( frame->m_modelling.pos[0], frame->m_modelling.pos[1], frame->m_modelling.pos[2] );

    link->m_id = -1;
}

static void RwAnimatedRenderLinkInit( RwRenderLink *link, void *data )
{
    link->m_flags = 0;
}

void RpClump::InitStaticSkeleton()
{
    RpAtomic *atomic = GetFirstAtomic();
    RwStaticGeometry *geom = CreateStaticGeometry();
    RwRenderLink *link;
    CVector boneOffsets[MAX_BONES];
    unsigned int boneCount;

    if ( !atomic || !atomic->m_skeleton )
    {
        // Initialize a non animated mesh
        link = geom->AllocateLink( m_parent->CountChildren() );

        // Assign all frames
        m_parent->ForAllChildren( RwAssignRenderLink, &link );

        // Init them
        geom->ForAllLinks( RwRenderLinkInit, 0 );

        geom->m_link->m_flags |= BONE_ROOT;
        return;
    }

    // Grab the number of bones
    boneCount = atomic->m_geometry->m_skeleton->m_boneCount;

    link = geom->AllocateLink( boneCount );

    if ( boneCount != 0 )
    {
        CVector *offset = boneOffsets;
        RpAnimHierarchy *anim = atomic->m_anim;  
        RwBoneInfo *bone = anim->m_boneInfo;
        RwAnimInfo *info = anim->m_anim->m_info;
        unsigned int n;

        // Get the real bone positions
        GetBoneTransform( boneOffsets );

        // I guess its always one bone ahead...?
        link++;

        for (n=0; n<boneCount; n++)
        {
            link->m_context = info;
            link->m_id = bone->m_index;

            // Update the bone offset in the animation
            info->m_offset = offset;
            
            info++;
            offset++;
            link++;
            bone++;
        }
    }

    geom->ForAllLinks( RwAnimatedRenderLinkInit, 0 );

    // Flag the first render link, root bone?
    geom->m_link->m_flags |= BONE_ROOT;
}

RwStaticGeometry* RpClump::CreateStaticGeometry()
{
    return m_static = new RwStaticGeometry();
}

RpAnimHierarchy* RpClump::GetAtomicAnimHierarchy()
{
    RpAtomic *atomic = GetFirstAtomic();

    if (!atomic)
        return NULL;
    
    return atomic->m_animHierarchy;
}

RpAnimHierarchy* RpClump::GetAnimHierarchy()
{
    return m_parent->GetAnimHierarchy();
}

struct _rwFrameScanHierarchy
{
    RwFrame **output;
    size_t max;
};

static bool RwFrameGetAssignedHierarchy( RwFrame *child, _rwFrameScanHierarchy *info )
{
    if ( child->m_hierarchyId && child->m_hierarchyId < info->max )
        info->output[child->m_hierarchyId] = child;

    return child->ForAllChildren( RwFrameGetAssignedHierarchy, info );
}

void RpClump::ScanAtomicHierarchy( RwFrame **atomics, size_t max )
{
    _rwFrameScanHierarchy info;

    info.output = atomics;
    info.max = max;

    m_parent->ForAllChildren( RwFrameGetAssignedHierarchy, &info );
}

static bool RwGetAtomic( RpAtomic *child, RpAtomic **atomic )
[
    *atomic = child;
    return false;
}

RpAtomic* RpClump::GetFirstAtomic()
{
    RpAtomic *atomic = NULL;

    ForAllAtomics( RwGetAtomic, &atomic );
    return atomic;
}

static bool RwAtomicGet2dfx( RpAtomic *child, RpAtomic **atomic )
{
    // Crashfix, invalid geometry
    if ( !child->m_geometry )
        return true;

    if ( !child->m_geometry->m_2dfx || child->m_geometry->m_2dfx->m_count == 0 )
        return true;

    *atomic = child;
    return false;
}

RpAtomic* RpClump::Find2dfx()
{
    RpAtomic *atomic;

    if ( ForAllAtomics( RwAtomicGet2dfx, &atomic ) )
        return NULL;

    return atomic;
}

static bool RwAtomicSetupPipeline( RpAtomic *child, void *data )
{
    if ( child->IsNight() )
        RpAtomicSetupObjectPipeline( child );
    else if ( child->m_pipeline == RW_ATOMIC_RENDER_VEHICLE )
        RpAtomicSetupVehiclePipeline( child );

    return true;
}

void RpClump::SetupAtomicRender()
{
    ForAllAtomics( RwAtomicSetupPipeline, 0 );
}

static bool RwAtomicRemoveVisibilityFlags( RpAtomic *child, unsigned short flags )
{
    child->RemoveVisibilityFlags( flags );
}

void RpClump::RemoveAtomicVisibilityFlags( unsigned short flags )
{
    ForAllAtomics( RwAtomicRemoveVisibilityFlags, (void*)flags );
}

static bool RwAtomicFetchMateria( RpAtomic *child, RpMaterials *mats )
{
    child->FetchMateria( mats );
    return true;
}

void RpClump::FetchMateria( RpMaterials *mats )
{
    ForAllAtomics( RwAtomicFetchMateria, mats );
}

RpClump* RpClump::ForAllAtomics( bool (*callback)( RpAtomic *child, void *data ), void *data )
{
    RpAtomic *child = m_atomics.root.next;

    while ( &child->m_atomics != &m_atomics.root )
    {
        if ( !callback( child, data ) )
            return NULL;

        child = (RpAtomic*)( (unsigned int)child->m_atomics.next - offsetof(RpAtomic, m_atomics));
    }

    return this;
}

void RpClump::GetBoneTransform( CVector *offset )
{
    RpAtomic *atomic;
    RpSkeleton *skel;
    RpAnimHierarchy* anim;
    RwBoneInfo *bone;
    RwMatrix *skelMat;
    unsigned int n;
    unsigned int matId = 0;
    unsigned int boneIndexes[20];
    unsigned int *idxPtr = boneIndexes;

    if ( !offset )
        return;

    atomic = GetFirstAtomic();
    skel = atomic->m_geometry->m_skeleton;

    anim = atomic->m_anim;

    // Reset the matrix
    offset->fX = 0;
    offset->fY = 0;
    offset->fZ = 0;

    offset++;

    // Do nothing if the bone count is smaller than 2
    if ( skel->m_boneCount < 2 )
        return;

    skelMat = skel->m_boneMatrices + 1;
    bone = anim->m_boneInfo + 1;

    // We apparrently have the first one initialized already?
    for (n=1; n<skel->m_boneCount; n++)
    {
        RwMatrix mat;

        RwMatrixInvert( &mat, skelMat );

        pRwInterface->m_matrixTransform3( offset, mat.up, 1, skel->m_boneMatrices + matId );

        // Some sort of stacking mechanism, maximum 20
        if ( bone->m_flags & 0x02 )
            *(++idxPtr) = matId;

        if ( bone->m_flags & 0x01 )
            matId = *(idxPtr--);
        else
            matId = n;

        skelMat++;
        offset++;
        bone++;
    }
}

bool RpGeometry::ForAllMateria( bool (*callback)( RpMaterial *mat, void *data ), void *data )
{
    unsigned int n;

    for (n=0; n<m_materials.m_entries; n++)
    {
        if ( !callback( m_materials.m_data[n], data ) )
            return false;
    }

    return true;
}

bool RwMaterialAlphaCheck( RpMaterial *mat, void *data )
{
    return mat->m_color.a != 0xFF;
}

bool RpGeometry::IsAlpha()
{
    return !ForAllMateria( RwMaterialAlphaCheck, 0 );
}

bool RwAtomicRenderChainInterface::PushRender( RwAtomicZBufferEntry *level )
{
    RwAtomicRenderChain *iter = &m_root;
    RwAtomicRenderChain *progr;

    while ( ( (iter = iter->m_list.prev) != &m_rootLast ) && m_lod < level->m_lod );

    if ( ( progr = m_renderStack.list.prev ) == &m_renderLast )
        return false;

    // Update render details
    progr->m_entry = *level;

    LIST_REMOVE( progr->list );
    LIST_INSERT( iter->list, progr->list );
    return true;
}