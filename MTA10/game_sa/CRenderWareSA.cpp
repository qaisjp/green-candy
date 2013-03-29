/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.cpp
*  PURPOSE:     RenderWare mapping to Grand Theft Auto: San Andreas
*               and miscellaneous rendering functions
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               arc_
*               Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*  RenderWare is © Criterion Software
*
*****************************************************************************/

#include "StdInc.h"
#include "gamesa_renderware.h"

/*****************************************************************************/
/** Renderware function mappings                                            **/
/*****************************************************************************/

static void __declspec(naked)    invalid_ptr()
{
    __asm int 3;
}

// US Versions
RwDeviceSystemRequest_t                 RwDeviceSystemRequest                   = (RwDeviceSystemRequest_t)                 invalid_ptr;
RwErrorGet_t                            RwErrorGet                              = (RwErrorGet_t)                            invalid_ptr;
RwAllocAligned_t                        RwAllocAligned                          = (RwAllocAligned_t)                        invalid_ptr;
RwFreeAligned_t                         RwFreeAligned                           = (RwFreeAligned_t)                         invalid_ptr;
RwCreateExtension_t                     RwCreateExtension                       = (RwCreateExtension_t)                     invalid_ptr;
RpPrtStdGlobalDataSetStreamEmbedded_t   RpPrtStdGlobalDataSetStreamEmbedded     = (RpPrtStdGlobalDataSetStreamEmbedded_t)   invalid_ptr;
RwPrefetch_t                            RwPrefetch                              = (RwPrefetch_t)                            invalid_ptr;
RwFlushLoader_t                         RwFlushLoader                           = (RwFlushLoader_t)                         invalid_ptr;
RwPluginRegistryReadDataChunks_t        RwPluginRegistryReadDataChunks          = (RwPluginRegistryReadDataChunks_t)        invalid_ptr;

RwIm3DTransform_t                       RwIm3DTransform                         = (RwIm3DTransform_t)                       invalid_ptr;
RwIm3DRenderIndexedPrimitive_t          RwIm3DRenderIndexedPrimitive            = (RwIm3DRenderIndexedPrimitive_t)          invalid_ptr;
RwIm3DEnd_t                             RwIm3DEnd                               = (RwIm3DEnd_t)                             invalid_ptr;

// Vector functions
RwV3dNormalize_t                        RwV3dNormalize                          = (RwV3dNormalize_t)                        invalid_ptr;
RwV3dTransformVector_t                  RwV3dTransformVector                    = (RwV3dTransformVector_t)                  invalid_ptr;

// Matrix functions
RwMatrixCreate_t                        RwMatrixCreate                          = (RwMatrixCreate_t)                        invalid_ptr;
RwMatrixInvert_t                        RwMatrixInvert                          = (RwMatrixInvert_t)                        invalid_ptr;
RwMatrixTranslate_t                     RwMatrixTranslate                       = (RwMatrixTranslate_t)                     invalid_ptr;
RwMatrixScale_t                         RwMatrixScale                           = (RwMatrixScale_t)                         invalid_ptr;
RwMatrixUnknown_t                       RwMatrixUnknown                         = (RwMatrixUnknown_t)                       invalid_ptr;

// Object functions
RwObjectRegister_t                      RwObjectRegister                        = (RwObjectRegister_t)                      invalid_ptr;

// Stream functions
RwStreamFindChunk_t                     RwStreamFindChunk                       = (RwStreamFindChunk_t)                     invalid_ptr;
RwStreamInitialize_t                    RwStreamInitialize                      = (RwStreamInitialize_t)                    invalid_ptr;
RwStreamOpen_t                          RwStreamOpen                            = (RwStreamOpen_t)                          invalid_ptr;
RwStreamReadChunkHeaderInfo_t           RwStreamReadChunkHeaderInfo             = (RwStreamReadChunkHeaderInfo_t)           invalid_ptr;
RwStreamReadBlocks_t                    RwStreamReadBlocks                      = (RwStreamReadBlocks_t)                    invalid_ptr;
RwStreamClose_t                         RwStreamClose                           = (RwStreamClose_t)                         invalid_ptr;

// Frame functions
RwFrameCreate_t                         RwFrameCreate                           = (RwFrameCreate_t)                         invalid_ptr;
RwFrameCloneRecursive_t                 RwFrameCloneRecursive                   = (RwFrameCloneRecursive_t)                 invalid_ptr;
RwFrameGetLTM_t                         RwFrameGetLTM                           = (RwFrameGetLTM_t)                         invalid_ptr;
RwFrameSetIdentity_t                    RwFrameSetIdentity                      = (RwFrameSetIdentity_t)                    invalid_ptr;
RwFrameTranslate_t                      RwFrameTranslate                        = (RwFrameTranslate_t)                      invalid_ptr;
RwFrameCloneHierarchy_t                 RwFrameCloneHierarchy                   = (RwFrameCloneHierarchy_t)                 invalid_ptr;
RwFrameAddChild_t                       RwFrameAddChild                         = (RwFrameAddChild_t)                       invalid_ptr;
RwFrameRemoveChild_t                    RwFrameRemoveChild                      = (RwFrameRemoveChild_t)                    invalid_ptr;
RwFrameScale_t                          RwFrameScale                            = (RwFrameScale_t)                          invalid_ptr;
RwFrameOrient_t                         RwFrameOrient                           = (RwFrameOrient_t)                         invalid_ptr;
RwFrameDestroy_t                        RwFrameDestroy                          = (RwFrameDestroy_t)                        invalid_ptr;

// Material functions
RpMaterialCreate_t                      RpMaterialCreate                        = (RpMaterialCreate_t)                      invalid_ptr;
RpMaterialDestroy_t                     RpMaterialDestroy                       = (RpMaterialDestroy_t)                     invalid_ptr;

// Geometry functions
RpGeometryCreate_t                      RpGeometryCreate                        = (RpGeometryCreate_t)                      invalid_ptr;
RpGeometryAddRef_t                      RpGeometryAddRef                        = (RpGeometryAddRef_t)                      invalid_ptr;
RpGeometryGetSkeleton_t                 RpGeometryGetSkeleton                   = (RpGeometryGetSkeleton_t)                 invalid_ptr;
RpGeometryTriangleSetVertexIndices_t    RpGeometryTriangleSetVertexIndices      = (RpGeometryTriangleSetVertexIndices_t)    invalid_ptr;
RpGeometryTriangleSetMaterial_t         RpGeometryTriangleSetMaterial           = (RpGeometryTriangleSetMaterial_t)         invalid_ptr;
RpGeometryUnlock_t                      RpGeometryUnlock                        = (RpGeometryUnlock_t)                      invalid_ptr;
RpGeometryLock_t                        RpGeometryLock                          = (RpGeometryLock_t)                        invalid_ptr;
RpGeometryTransform_t                   RpGeometryTransform                     = (RpGeometryTransform_t)                   invalid_ptr;
RpGeometryRegisterPlugin_t              RpGeometryRegisterPlugin                = (RpGeometryRegisterPlugin_t)              invalid_ptr;
RpGeometryDestroy_t                     RpGeometryDestroy                       = (RpGeometryDestroy_t)                     invalid_ptr;

// Atomic functions
RpAtomicCreate_t                        RpAtomicCreate                          = (RpAtomicCreate_t)                        invalid_ptr;
RpAtomicClone_t                         RpAtomicClone                           = (RpAtomicClone_t)                         invalid_ptr;
RpAtomicSetFrame_t                      RpAtomicSetFrame                        = (RpAtomicSetFrame_t)                      invalid_ptr;
RpAtomicSetupObjectPipeline_t           RpAtomicSetupObjectPipeline             = (RpAtomicSetupObjectPipeline_t)           invalid_ptr;
RpAtomicSetupVehiclePipeline_t          RpAtomicSetupVehiclePipeline            = (RpAtomicSetupVehiclePipeline_t)          invalid_ptr;
RpAtomicRender_t                        RpAtomicRender                          = (RpAtomicRender_t)                        invalid_ptr;
RpAtomicSetGeometry_t                   RpAtomicSetGeometry                     = (RpAtomicSetGeometry_t)                   invalid_ptr;
RpAtomicDestroy_t                       RpAtomicDestroy                         = (RpAtomicDestroy_t)                       invalid_ptr;

// Light functions
RpLightSetRadius_t                      RpLightSetRadius                        = (RpLightSetRadius_t)                      invalid_ptr;
RpLightGetConeAngle_t                   RpLightGetConeAngle                     = (RpLightGetConeAngle_t)                   invalid_ptr;
RpLightDestroy_t                        RpLightDestroy                          = (RpLightDestroy_t)                        invalid_ptr;

// Camera functions
RwCameraClone_t                         RwCameraClone                           = (RwCameraClone_t)                         invalid_ptr;
RwCameraDestroy_t                       RwCameraDestroy                         = (RwCameraDestroy_t)                       invalid_ptr;

// Clump functions
RpClumpClone_t                          RpClumpClone                            = (RpClumpClone_t)                          invalid_ptr;
RpClumpStreamRead_t                     RpClumpStreamRead                       = (RpClumpStreamRead_t)                     invalid_ptr;
RpClumpAddAtomic_t                      RpClumpAddAtomic                        = (RpClumpAddAtomic_t)                      invalid_ptr;
RpClumpRemoveAtomic_t                   RpClumpRemoveAtomic                     = (RpClumpRemoveAtomic_t)                   invalid_ptr;
RpClumpAddLight_t                       RpClumpAddLight                         = (RpClumpAddLight_t)                       invalid_ptr;
RpClumpGetBoneTransform_t               RpClumpGetBoneTransform                 = (RpClumpGetBoneTransform_t)               invalid_ptr;
RpClumpSetupFrameCallback_t             RpClumpSetupFrameCallback               = (RpClumpSetupFrameCallback_t)             invalid_ptr;
RpClumpDestroy_t                        RpClumpDestroy                          = (RpClumpDestroy_t)                        invalid_ptr;
RpClumpGetNumAtomics_t                  RpClumpGetNumAtomics                    = (RpClumpGetNumAtomics_t)                  invalid_ptr;

// Raster functions
RwRasterCreate_t                        RwRasterCreate                          = (RwRasterCreate_t)                        invalid_ptr;
RwRasterUnlock_t                        RwRasterUnlock                          = (RwRasterUnlock_t)                        invalid_ptr;
RwRasterLock_t                          RwRasterLock                            = (RwRasterLock_t)                          invalid_ptr;
RwRasterDestroy_t                       RwRasterDestroy                         = (RwRasterDestroy_t)                       invalid_ptr;

// Texture functions
RwTextureCreate_t                       RwTextureCreate                         = (RwTextureCreate_t)                       invalid_ptr;
RwTextureDestroy_t                      RwTextureDestroy                        = (RwTextureDestroy_t)                      invalid_ptr;

// TexDictionary functions
RwTexDictionaryStreamRead_t             RwTexDictionaryStreamRead               = (RwTexDictionaryStreamRead_t)             invalid_ptr;
RwTexDictionaryGetCurrent_t             RwTexDictionaryGetCurrent               = (RwTexDictionaryGetCurrent_t)             invalid_ptr;
RwTexDictionarySetCurrent_t             RwTexDictionarySetCurrent               = (RwTexDictionarySetCurrent_t)             invalid_ptr;
RwTexDictionaryForAllTextures_t         RwTexDictionaryForAllTextures           = (RwTexDictionaryForAllTextures_t)         invalid_ptr;
RwTexDictionaryFindNamedTexture_t       RwTexDictionaryFindNamedTexture         = (RwTexDictionaryFindNamedTexture_t)       invalid_ptr;
RwTexDictionaryAddTexture_t             RwTexDictionaryAddTexture               = (RwTexDictionaryAddTexture_t)             invalid_ptr;
RwTexDictionaryDestroy_t                RwTexDictionaryDestroy                  = (RwTexDictionaryDestroy_t)                invalid_ptr;

// Scene functions
RwSceneRender_t                         RwSceneRender                           = (RwSceneRender_t)                         invalid_ptr;
RwSceneAddAtomic_t                      RwSceneAddAtomic                        = (RwSceneAddAtomic_t)                      invalid_ptr;
RwSceneAddLight_t                       RwSceneAddLight                         = (RwSceneAddLight_t)                       invalid_ptr;
RwSceneAddClump_t                       RwSceneAddClump                         = (RwSceneAddClump_t)                       invalid_ptr;
RwSceneRemoveLight_t                    RwSceneRemoveLight                      = (RwSceneRemoveLight_t)                    invalid_ptr;
RwSceneDestroy_t                        RwSceneDestroy                          = (RwSceneDestroy_t)                        invalid_ptr;

// Dict functions
RtDictSchemaStreamReadDict_t            RtDictSchemaStreamReadDict              = (RtDictSchemaStreamReadDict_t)            invalid_ptr;
RtDictDestroy_t                         RtDictDestroy                           = (RtDictDestroy_t)                         invalid_ptr;

// Animation functions
RwAnimationInit_t                       RwAnimationInit                         = (RwAnimationInit_t)                       invalid_ptr;
RwSkeletonUpdate_t                      RwSkeletonUpdate                        = (RwSkeletonUpdate_t)                      invalid_ptr;

/*****************************************************************************/
/** GTA:SA function mappings                                                **/
/*****************************************************************************/

LoadCollisionModel_t            LoadCollisionModel              = (LoadCollisionModel_t)            invalid_ptr;
LoadCollisionModelVer2_t        LoadCollisionModelVer2          = (LoadCollisionModelVer2_t)        invalid_ptr;
LoadCollisionModelVer3_t        LoadCollisionModelVer3          = (LoadCollisionModelVer3_t)        invalid_ptr;
LoadCollisionModelVer4_t        LoadCollisionModelVer4          = (LoadCollisionModelVer4_t)        invalid_ptr;
CClothesBuilder_CopyTexture_t   CClothesBuilder_CopyTexture     = (CClothesBuilder_CopyTexture_t)   invalid_ptr;

extern CGameSA *pGame;
extern CBaseModelInfoSAInterface **ppModelInfo;

RwInterface **ppRwInterface = (RwInterface**)0x00C97B24;

CRenderWareSA::CRenderWareSA( eGameVersion version )
{
    // Version dependant addresses
    switch( version )
    {
    // VERSION 1.0 EU ADDRESSES
    case VERSION_EU_10:
        // Utility functions
        RwDeviceSystemRequest               = (RwDeviceSystemRequest_t)                 0x007F2AF0;
        RwPrefetch                          = (RwPrefetch_t)                            0x0072F480;
        RwFlushLoader                       = (RwFlushLoader_t)                         0x0072E700;
        RwAllocAligned                      = (RwAllocAligned_t)                        0x0072F4C0;
        RwFreeAligned                       = (RwFreeAligned_t)                         0x0072F4F0;
        RwErrorGet                          = (RwErrorGet_t)                            0x008088C0;
        RwPluginRegistryReadDataChunks      = (RwPluginRegistryReadDataChunks_t)        0x008089C0;
        RwCreateExtension                   = (RwCreateExtension_t)                     0x007CCE80;

        RwIm3DTransform                     = (RwIm3DTransform_t)                       0x007EF490; 
        RwIm3DRenderIndexedPrimitive        = (RwIm3DRenderIndexedPrimitive_t)          0x007EF590;
        RwIm3DEnd                           = (RwIm3DEnd_t)                             0x007EF560;

        // Vector functions
        RwV3dNormalize                      = (RwV3dNormalize_t)                        0x007ED9F0;
        RwV3dTransformVector                = (RwV3dTransformVector_t)                  0x007EDE00;

        // Matrix functions
        RwMatrixCreate                      = (RwMatrixCreate_t)                        0x007F2A90;
        RwMatrixInvert                      = (RwMatrixInvert_t)                        0x007F20B0;
        RwMatrixTranslate                   = (RwMatrixTranslate_t)                     0x007F2490;
        RwMatrixScale                       = (RwMatrixScale_t)                         0x007F2300;
        RwMatrixUnknown                     = (RwMatrixUnknown_t)                       0x007326D0;

        // Object functions
        RwObjectRegister                    = (RwObjectRegister_t)                      0x00808720;

        // Stream functions
        RwStreamInitialize                  = (RwStreamInitialize_t)                    0x007EC850;
        RwStreamOpen                        = (RwStreamOpen_t)                          0x007ECF30;
        RwStreamReadChunkHeaderInfo         = (RwStreamReadChunkHeaderInfo_t)           0x007ED5D0;
        RwStreamFindChunk                   = (RwStreamFindChunk_t)                     0x007ED310;
        RwStreamReadBlocks                  = (RwStreamReadBlocks_t)                    0x007ECA10;
        RwStreamClose                       = (RwStreamClose_t)                         0x007ECE60;

        // Frame functions
        RwFrameCreate                       = (RwFrameCreate_t)                         0x007F0450;
        RwFrameCloneRecursive               = (RwFrameCloneRecursive_t)                 0x007F0090;
        RwFrameCloneHierarchy               = (RwFrameCloneHierarchy_t)                 0x007F0290;
        RwFrameGetLTM                       = (RwFrameGetLTM_t)                         0x007F09D0;
        RwFrameSetIdentity                  = (RwFrameSetIdentity_t)                    0x007F10F0;
        RwFrameAddChild                     = (RwFrameAddChild_t)                       0x007F0B40;
        RwFrameRemoveChild                  = (RwFrameRemoveChild_t)                    0x007F0D10;
        RwFrameTranslate                    = (RwFrameTranslate_t)                      0x007F0E70;
        RwFrameOrient                       = (RwFrameOrient_t)                         0x007F2010;
        RwFrameScale                        = (RwFrameScale_t)                          0x007F0F10;
        RwFrameDestroy                      = (RwFrameDestroy_t)                        0x007F04B0;

        // Material functions
        RpMaterialCreate                    = (RpMaterialCreate_t)                      0x0074D9E0;
        RpMaterialDestroy                   = (RpMaterialDestroy_t)                     0x0074DA70;

        // Geometry functions
        RpGeometryCreate                    = (RpGeometryCreate_t)                      0x0074CAE0;
        RpGeometryGetSkeleton               = (RpGeometryGetSkeleton_t)                 0x007C7590;
        RpGeometryTriangleSetVertexIndices  = (RpGeometryTriangleSetVertexIndices_t)    0x0074C6E0;
        RpGeometryUnlock                    = (RpGeometryUnlock_t)                      0x0074C850;
        RpGeometryLock                      = (RpGeometryLock_t)                        0x0074C820;
        RpGeometryTransform                 = (RpGeometryTransform_t)                   0x0074C030;
        RpGeometryRegisterPlugin            = (RpGeometryRegisterPlugin_t)              0x0074CDC0;
        RpGeometryDestroy                   = (RpGeometryDestroy_t)                     0x0074CD10;

        // Atomic functions
        RpAtomicCreate                      = (RpAtomicCreate_t)                        0x00749CA0;
        RpAtomicClone                       = (RpAtomicClone_t)                         0x00749EB0;
        RpAtomicSetFrame                    = (RpAtomicSetFrame_t)                      0x0074BF70;
        RpAtomicSetGeometry                 = (RpAtomicSetGeometry_t)                   0x00749D90;
        RpAtomicRender                      = (RpAtomicRender_t)                        0x00749210;
        RpAtomicSetupObjectPipeline         = (RpAtomicSetupObjectPipeline_t)           0x005D7F00;
        RpAtomicSetupVehiclePipeline        = (RpAtomicSetupVehiclePipeline_t)          0x005D5B20;
        RpAtomicDestroy                     = (RpAtomicDestroy_t)                       0x00749E10;

        // Light functions
        RpLightSetRadius                    = (RpLightSetRadius_t)                      0x00751AC0;
        RpLightGetConeAngle                 = (RpLightGetConeAngle_t)                   0x00751B30;
        RpLightDestroy                      = (RpLightDestroy_t)                        0x00752120;

        // Camera functions
        RwCameraClone                       = (RwCameraClone_t)                         0x007EF3F0;
        RwCameraDestroy                     = (RwCameraDestroy_t)                       0x007EE4F0;

        // Clump functions
        RpClumpClone                        = (RpClumpClone_t)                          0x00749FC0;
        RpClumpAddAtomic                    = (RpClumpAddAtomic_t)                      0x0074A4E0;
        RpClumpRemoveAtomic                 = (RpClumpRemoveAtomic_t)                   0x0074A510;
        RpClumpAddLight                     = (RpClumpAddLight_t)                       0x0074A540;
        RpClumpGetNumAtomics                = (RpClumpGetNumAtomics_t)                  0x00749930;
        RpClumpGetBoneTransform             = (RpClumpGetBoneTransform_t)               0x00735360;
        RpClumpSetupFrameCallback           = (RpClumpSetupFrameCallback_t)             0x00733750;
        RpClumpStreamRead                   = (RpClumpStreamRead_t)                     0x0074B470;
        RpClumpDestroy                      = (RpClumpDestroy_t)                        0x0074A360;

        // Raster functions
        RwRasterCreate                      = (RwRasterCreate_t)                        0x007FB270;
        RwRasterUnlock                      = (RwRasterUnlock_t)                        0x007FAF00;
        RwRasterLock                        = (RwRasterLock_t)                          0x007FB310; 
        RwRasterDestroy                     = (RwRasterDestroy_t)                       0x007FB060;

        // Texture functions
        RwTextureCreate                     = (RwTextureCreate_t)                       0x007F3800;
        RwTextureDestroy                    = (RwTextureDestroy_t)                      0x007F3860;

        // TexDictionary functions
        RwTexDictionaryStreamRead           = (RwTexDictionaryStreamRead_t)             0x00804C70;
        RwTexDictionaryGetCurrent           = (RwTexDictionaryGetCurrent_t)             0x007F3AD0;
        RwTexDictionarySetCurrent           = (RwTexDictionarySetCurrent_t)             0x007F3AB0;
        RwTexDictionaryForAllTextures       = (RwTexDictionaryForAllTextures_t)         0x007F3770;
        RwTexDictionaryFindNamedTexture     = (RwTexDictionaryFindNamedTexture_t)       0x007F3A30;
        RwTexDictionaryAddTexture           = (RwTexDictionaryAddTexture_t)             0x007F39C0;
        RwTexDictionaryDestroy              = (RwTexDictionaryDestroy_t)                0x007F36E0;

        // Scene functions
        RwSceneRender                       = (RwSceneRender_t)                         0x0074F5C0;
        RwSceneAddAtomic                    = (RwSceneAddAtomic_t)                      0x00750FE0;
        RwSceneAddClump                     = (RwSceneAddClump_t)                       0x00751350;
        RwSceneAddLight                     = (RwSceneAddLight_t)                       0x00751960;
        RwSceneRemoveLight                  = (RwSceneRemoveLight_t)                    0x007519B0;
        RwSceneDestroy                      = (RwSceneDestroy_t)                        0x0074F660;

        // Dict functions
        RtDictSchemaStreamReadDict          = (RtDictSchemaStreamReadDict_t)            0x007CF280;
        RtDictDestroy                       = (RtDictDestroy_t)                         0x007CF170;

        // Animation functions
        RwAnimationInit                     = (RwAnimationInit_t)                       0x007CD5E0;
        RwSkeletonUpdate                    = (RwSkeletonUpdate_t)                      0x007C51D0;
        break;

    // VERSION 1.0 US ADDRESSES
    case VERSION_US_10:
        // Utility functions
        RwDeviceSystemRequest               = (RwDeviceSystemRequest_t)                 0x007F2AB0;
        RwAllocAligned                      = (RwAllocAligned_t)                        0x0072F4C0;
        RwFreeAligned                       = (RwFreeAligned_t)                         0x0072F4F0;
        RwCreateExtension                   = (RwCreateExtension_t)                     0x007CCE80;
        RwErrorGet                          = (RwErrorGet_t)                            0x00808880;
        RwPrefetch                          = (RwPrefetch_t)                            0x0072F480;
        RwFlushLoader                       = (RwFlushLoader_t)                         0x0072E700;
        RwPluginRegistryReadDataChunks      = (RwPluginRegistryReadDataChunks_t)        0x00808980;

        RwIm3DTransform                     = (RwIm3DTransform_t)                       0x007EF450;
        RwIm3DRenderIndexedPrimitive        = (RwIm3DRenderIndexedPrimitive_t)          0x007EF550;
        RwIm3DEnd                           = (RwIm3DEnd_t)                             0x007EF520;

        // Vector functions
        RwV3dNormalize                      = (RwV3dNormalize_t)                        0x007ED9B0;
        RwV3dTransformVector                = (RwV3dTransformVector_t)                  0x007EDDC0;

        // Matrix functions
        RwMatrixCreate                      = (RwMatrixCreate_t)                        0x007F2A50;
        RwMatrixInvert                      = (RwMatrixInvert_t)                        0x007F2070;
        RwMatrixTranslate                   = (RwMatrixTranslate_t)                     0x007F2450;
        RwMatrixScale                       = (RwMatrixScale_t)                         0x007F22C0;
        RwMatrixUnknown                     = (RwMatrixUnknown_t)                       0x007326D0;

        // Object functions
        RwObjectRegister                    = (RwObjectRegister_t)                      0x008086E0;

        // Stream functions
        RwStreamInitialize                  = (RwStreamInitialize_t)                    0x007EC810;
        RwStreamOpen                        = (RwStreamOpen_t)                          0x007ECEF0;
        RwStreamReadBlocks                  = (RwStreamReadBlocks_t)                    0x007EC9D0;
        RwStreamFindChunk                   = (RwStreamFindChunk_t)                     0x007ED2D0;
        RwStreamReadChunkHeaderInfo         = (RwStreamReadChunkHeaderInfo_t)           0x007ED590;
        RwStreamClose                       = (RwStreamClose_t)                         0x007ECE20;

        // Frame functions
        RwFrameCreate                       = (RwFrameCreate_t)                         0x007F0410;
        RwFrameCloneRecursive               = (RwFrameCloneRecursive_t)                 0x007F0050;
        RwFrameCloneHierarchy               = (RwFrameCloneHierarchy_t)                 0x007f0250;
        RwFrameGetLTM                       = (RwFrameGetLTM_t)                         0x007F0990;
        RwFrameScale                        = (RwFrameScale_t)                          0x007F0ED0;
        RwFrameTranslate                    = (RwFrameTranslate_t)                      0x007F0E30;
        RwFrameAddChild                     = (RwFrameAddChild_t)                       0x007F0B00;
        RwFrameRemoveChild                  = (RwFrameRemoveChild_t)                    0x007F0CD0;
        RwFrameSetIdentity                  = (RwFrameSetIdentity_t)                    0x007F10B0;
        RwFrameOrient                       = (RwFrameOrient_t)                         0x007F2010;
        RwFrameDestroy                      = (RwFrameDestroy_t)                        0x007F05A0;

        // Material functions
        RpMaterialCreate                    = (RpMaterialCreate_t)                      0x0074D990;
        RpMaterialDestroy                   = (RpMaterialDestroy_t)                     0x0074DA20;

        // Geometry functions
        RpGeometryCreate                    = (RpGeometryCreate_t)                      0x0074CA90;
        RpGeometryTransform                 = (RpGeometryTransform_t)                   0x0074BFE0;
        RpGeometryGetSkeleton               = (RpGeometryGetSkeleton_t)                 0x007C7550;
        RpGeometryTriangleSetVertexIndices  = (RpGeometryTriangleSetVertexIndices_t)    0x0074C690;
        RpGeometryTriangleSetMaterial       = (RpGeometryTriangleSetMaterial_t)         0x0074C6C0;
        RpGeometryUnlock                    = (RpGeometryUnlock_t)                      0x0074C800;
        RpGeometryLock                      = (RpGeometryLock_t)                        0x0074C7D0;
        RpGeometryRegisterPlugin            = (RpGeometryRegisterPlugin_t)              0x0074CD70;
        RpGeometryDestroy                   = (RpGeometryDestroy_t)                     0x0074CCC0;

        // Atomic functions
        RpAtomicCreate                      = (RpAtomicCreate_t)                        0x00749C50;
        RpAtomicClone                       = (RpAtomicClone_t)                         0x00749E60;
        RpAtomicSetGeometry                 = (RpAtomicSetGeometry_t)                   0x00749D40;
        RpAtomicSetFrame                    = (RpAtomicSetFrame_t)                      0x0074BF20;
        RpAtomicSetupObjectPipeline         = (RpAtomicSetupObjectPipeline_t)           0x005D7F00;
        RpAtomicSetupVehiclePipeline        = (RpAtomicSetupVehiclePipeline_t)          0x005D5B20;
        RpAtomicRender                      = (RpAtomicRender_t)                        0x007491C0;
        RpAtomicDestroy                     = (RpAtomicDestroy_t)                       0x00749DC0;

        // Light functions
        RpLightSetRadius                    = (RpLightSetRadius_t)                      0x00751A70;
        RpLightGetConeAngle                 = (RpLightGetConeAngle_t)                   0x00751AE0;
        RpLightDestroy                      = (RpLightDestroy_t)                        0x007520D0;

        // Camera functions
        RwCameraClone                       = (RwCameraClone_t)                         0x007EF3B0;
        RwCameraDestroy                     = (RwCameraDestroy_t)                       0x007EE4B0;

        // Clump functions
        RpClumpClone                        = (RpClumpClone_t)                          0x00749F70;
        RpClumpAddLight                     = (RpClumpAddLight_t)                       0x0074A4F0;
        RpClumpAddAtomic                    = (RpClumpAddAtomic_t)                      0x0074A490;
        RpClumpRemoveAtomic                 = (RpClumpRemoveAtomic_t)                   0x0074A4C0;
        RpClumpGetNumAtomics                = (RpClumpGetNumAtomics_t)                  0x007498E0;
        RpClumpGetBoneTransform             = (RpClumpGetBoneTransform_t)               0x00735360;
        RpClumpSetupFrameCallback           = (RpClumpSetupFrameCallback_t)             0x00733750;
        RpClumpStreamRead                   = (RpClumpStreamRead_t)                     0x0074B420;
        RpClumpDestroy                      = (RpClumpDestroy_t)                        0x0074A310;

        // Raster functions
        RwRasterUnlock                      = (RwRasterUnlock_t)                        0x007FAEC0;
        RwRasterLock                        = (RwRasterLock_t)                          0x007FB2D0;
        RwRasterCreate                      = (RwRasterCreate_t)                        0x007FB230;
        RwRasterDestroy                     = (RwRasterDestroy_t)                       0x007FB020;

        // Texture functions
        RwTextureCreate                     = (RwTextureCreate_t)                       0x007F37C0;
        RwTextureDestroy                    = (RwTextureDestroy_t)                      0x007F3820;

        // TexDictionary functions
        RwTexDictionaryStreamRead           = (RwTexDictionaryStreamRead_t)             0x00804C30;
        RwTexDictionaryGetCurrent           = (RwTexDictionaryGetCurrent_t)             0x007F3A90;
        RwTexDictionarySetCurrent           = (RwTexDictionarySetCurrent_t)             0x007F3A70;
        RwTexDictionaryForAllTextures       = (RwTexDictionaryForAllTextures_t)         0x007F3730;
        RwTexDictionaryFindNamedTexture     = (RwTexDictionaryFindNamedTexture_t)       0x007F39F0;
        RwTexDictionaryAddTexture           = (RwTexDictionaryAddTexture_t)             0x007F3980;
        RwTexDictionaryDestroy              = (RwTexDictionaryDestroy_t)                0x007F36A0;

        // Scene functions
        RwSceneRender                       = (RwSceneRender_t)                         0x0074F570;
        RwSceneAddClump                     = (RwSceneAddClump_t)                       0x00751300;
        RwSceneAddLight                     = (RwSceneAddLight_t)                       0x00751910;
        RwSceneAddAtomic                    = (RwSceneAddAtomic_t)                      0x00750F90;
        RwSceneRemoveLight                  = (RwSceneRemoveLight_t)                    0x00751960;
        RwSceneDestroy                      = (RwSceneDestroy_t)                        0x0074F610;

        // Dict functions
        RtDictSchemaStreamReadDict          = (RtDictSchemaStreamReadDict_t)            0x007CF240;
        RtDictDestroy                       = (RtDictDestroy_t)                         0x007CF130;

        // Animation functions
        RwAnimationInit                     = (RwAnimationInit_t)                       0x007CD5E0;
        RwSkeletonUpdate                    = (RwSkeletonUpdate_t)                      0x007C5210;
        break;
    }

    // Shared addresses
    RpPrtStdGlobalDataSetStreamEmbedded = (RpPrtStdGlobalDataSetStreamEmbedded_t)   0x0041B350;
    LoadCollisionModel                  = (LoadCollisionModel_t)                    0x00537580;
    LoadCollisionModelVer2              = (LoadCollisionModelVer2_t)                0x00537EE0;
    LoadCollisionModelVer3              = (LoadCollisionModelVer3_t)                0x00537CE0;
    LoadCollisionModelVer4              = (LoadCollisionModelVer4_t)                0x00537AE0;
    CClothesBuilder_CopyTexture         = (CClothesBuilder_CopyTexture_t)           0x005A5730;
}

// Reads and parses a DFF file specified by a path into a CModelInfo identified by the object id
RpClump* CRenderWareSA::ReadDFF( CFile *file, unsigned short id, CColModelSA*& colOut )
{
    if ( id > DATA_TEXTURE_BLOCK-1 )
        return NULL;

    // open the stream
    RwStream *streamModel = RwStreamCreateTranslated( file );

    if ( streamModel == NULL )
        return NULL;

    // DFF header id: 0x10
    // find our dff chunk
    if ( !RwStreamFindChunk( streamModel, 0x10, NULL, NULL ) )
        return NULL;

    CBaseModelInfoSAInterface *model = ppModelInfo[id];
    CTxdInstanceSA *txd;
    CColModelSAInterface *col;
    bool txdReference;

    if ( id != 0 )
    {
        CModelLoadInfoSA *info = (CModelLoadInfoSA*)ARRAY_CModelLoadInfo + id;

        // rockstar's collision hack: set the global particle emitter to the modelinfo pointer of this model
        RpPrtStdGlobalDataSetStreamEmbedded( model );

        // The_GTA: Clumps and atomics load their requirements while being read in this rwStream
        // We therefor have to prepare all resources so it can retrive them; textures and animations!
        if ( model )
        {
            col = model->pColModel;
            model->pColModel = NULL;

            txd = (*ppTxdPool)->Get( model->usTextureDictionary );

            if ( !txd->m_txd )
            {
                CStreamingSA *streamer = pGame->GetStreaming();

                // VERY IMPORTANT: find a way to load the texDictionary and animation ourselves here!
                // We need to isolate this process from CStreaming, because CStreaming includes the model replacement fix
                // If you try to load a dff with a replaced model, the collision might crash you if you delete
                // a model which uses the same collision; the crash does not happen if all DFFs have a own collision
                // A fix would be to clone the collision somehow, but loading resources ourselves is required too!
                // Eventually: custom clump, txd and col async loading functions in Lua

                // Load all requirements
                streamer->RequestModel( id, 0x10 );
                streamer->LoadAllRequestedModels( true );

                // We delete the RenderWare associations in this clump to free resources since GTA:SA loaded the 
                // actual model's dff by now, which we do not need
                // The only thing we need is the reference to the texture container and possibly the collision (?)
                txd->Reference();
                txdReference = true;

                // TODO: make sure that atomic model infos do not delete the associated collision.
                // Otherwise we have to preserve it here! Last time I checked it did not happen.

                // Tell GTA:SA to unload the resources, to cleanup associations
                streamer->FreeModel( id );
            }
            else
                txdReference = false;

            // For atomics we have to set the current texture container so it loads from it properly
            if ( model->GetRwModelType() == RW_ATOMIC )
                txd->SetCurrent();
        }

        RwImportedScan::Apply( model->usTextureDictionary );
    }

    // read the clump with all its extensions
    RpClump *pClump = RpClumpStreamRead( streamModel );

    if ( id != 0 )
    {
        RwImportedScan::Unapply();

        // reset model schemantic loader
        RpPrtStdGlobalDataSetStreamEmbedded( NULL );

        if ( model )
        {
            // We do not have to preserve the texture container, as RenderWare is smart enough to hold references
            // to textures itself
            if ( txdReference )
                txd->Dereference();

            // If there is no collision in our model information by now, the clump did not provide one
            // We should restore to the original collision then
            if ( !model->pColModel )
            {
                colOut = new CColModelSA( col, false );
            }
            else
            {
                colOut = new CColModelSA( model->pColModel, true );
            }

            // Restore the original colmodel as we have not requested the custom model yet
            model->pColModel = col;
        }
        else
            colOut = NULL;
    }
    else
        colOut = NULL;

    // close the stream
    RwStreamClose( streamModel, NULL );
    return pClump;
}

// Reads and parses a COLL(2, 3, 4) file
CColModel* CRenderWareSA::ReadCOL( CFile *file )
{
    // Create a new CColModel
    CColModelSA* pColModel = new CColModelSA ();
    ColModelFileHeader header;

    file->Read( &header, sizeof(ColModelFileHeader), 1 );
    
    // Load the col model
    if ( header.version[0] == 'C' && header.version[1] == 'O' && header.version[2] == 'L' )
    {
        char* pModelData = new char [ header.size - 0x18 ];
        file->Read( pModelData, header.size - 0x18, 1 );

        switch( header.version[3] )
        {
        case 'L':
            LoadCollisionModel( pModelData, pColModel->GetInterface(), NULL );
            break;
        case '2':
            LoadCollisionModelVer2( pModelData, header.size - 0x18, pColModel->GetInterface(), NULL );
            break;
        case '3':
            LoadCollisionModelVer3( pModelData, header.size - 0x18, pColModel->GetInterface(), NULL );
            break;
        case '4':   // Is this a hidden present by R*?
            LoadCollisionModelVer4( pModelData, header.size - 0x18, pColModel->GetInterface(), NULL );
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
    CVehicleComponentInfoSAInterface *seats = ((CVehicleModelInfoSAInterface*)ppModelInfo[usModelID])->m_componentInfo;

    // read out the 'ped_frontseat' frame
    RwFrame *pPedFrontSeat = pClump->m_parent->FindChildByName( "ped_frontseat" );

    if ( pPedFrontSeat == NULL )
        return false;

    // in the vehicle specific dummy data, +30h contains the front seat vector
    seats->m_seatOffset[4] = pPedFrontSeat->m_modelling.pos;
    return true;
}

/*==========================================================

    RenderWare native exports

==========================================================*/

RwMatrix* CRenderWareSA::AllocateMatrix()
{
    return new ( pRwInterface->m_allocStruct( pRwInterface->m_matrixInfo, 0x3000D ) ) RwMatrix();
}

CRpLight* CRenderWareSA::CreateLight( RpLightType type )
{
    return new CRpLightSA( RpLightCreate( type ) );
}

CRwFrame* CRenderWareSA::CreateFrame()
{
    RwFrame *frame = RwFrameCreate();
    frame->RegisterRoot();  // We belong to RenderWare directly

    return new CRwFrameSA( frame );
}

CRwCamera* CRenderWareSA::CreateCamera( int width, int height )
{
    if ( width == 0 || height == 0 )
        return NULL;

    CRwCameraSA *cam = new CRwCameraSA( RwCameraCreate() );
    cam->SetRenderSize( width, height );
    return cam;
}

bool CRenderWareSA::IsRendering() const
{
    return pRwInterface->m_renderCam != NULL;
}