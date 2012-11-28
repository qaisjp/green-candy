/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.cpp
*  PURPOSE:     RenderWare mapping to Grand Theft Auto: San Andreas
*               and miscellaneous rendering functions
*  DEVELOPERS:  Cecill Etheredge <ijsf@gmx.net>
*               arc_
*               The_GTA <quiret@gmx.de>
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
RwRenderSystemFigureAffairs_t           RwRenderSystemFigureAffairs             = (RwRenderSystemFigureAffairs_t)           invalid_ptr;
RwErrorGet_t                            RwErrorGet                              = (RwErrorGet_t)                            invalid_ptr;
RwAllocAligned_t                        RwAllocAligned                          = (RwAllocAligned_t)                        invalid_ptr;
RwFreeAligned_t                         RwFreeAligned                           = (RwFreeAligned_t)                         invalid_ptr;
RwCreateExtension_t                     RwCreateExtension                       = (RwCreateExtension_t)                     invalid_ptr;
RwObjectRegister_t                      RwObjectRegister                        = (RwObjectRegister_t)                      invalid_ptr;
RwStreamFindChunk_t                     RwStreamFindChunk                       = (RwStreamFindChunk_t)                     invalid_ptr;
RwStreamInitialize_t                    RwStreamInitialize                      = (RwStreamInitialize_t)                    invalid_ptr;
RwStreamOpen_t                          RwStreamOpen                            = (RwStreamOpen_t)                          invalid_ptr;
RwStreamReadChunkHeaderInfo_t           RwStreamReadChunkHeaderInfo             = (RwStreamReadChunkHeaderInfo_t)           invalid_ptr;
RwStreamReadBlocks_t                    RwStreamReadBlocks                      = (RwStreamReadBlocks_t)                    invalid_ptr;
RwStreamReadTexture_t                   RwStreamReadTexture                     = (RwStreamReadTexture_t)                   invalid_ptr;
RwStreamClose_t                         RwStreamClose                           = (RwStreamClose_t)                         invalid_ptr;
RtDictSchemaStreamReadDict_t            RtDictSchemaStreamReadDict              = (RtDictSchemaStreamReadDict_t)            invalid_ptr;
RtDictDestroy_t                         RtDictDestroy                           = (RtDictDestroy_t)                         invalid_ptr;
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
RwCameraClone_t                         RwCameraClone                           = (RwCameraClone_t)                         invalid_ptr;
RwCameraDestroy_t                       RwCameraDestroy                         = (RwCameraDestroy_t)                       invalid_ptr;
RpClumpClone_t                          RpClumpClone                            = (RpClumpClone_t)                          invalid_ptr;
RpClumpStreamRead_t                     RpClumpStreamRead                       = (RpClumpStreamRead_t)                     invalid_ptr;
RpClumpAddAtomic_t                      RpClumpAddAtomic                        = (RpClumpAddAtomic_t)                      invalid_ptr;
RpClumpRemoveAtomic_t                   RpClumpRemoveAtomic                     = (RpClumpRemoveAtomic_t)                   invalid_ptr;
RpClumpAddLight_t                       RpClumpAddLight                         = (RpClumpAddLight_t)                       invalid_ptr;
RpClumpGetBoneTransform_t               RpClumpGetBoneTransform                 = (RpClumpGetBoneTransform_t)               invalid_ptr;
RpClumpSetupFrameCallback_t             RpClumpSetupFrameCallback               = (RpClumpSetupFrameCallback_t)             invalid_ptr;
RpClumpDestroy_t                        RpClumpDestroy                          = (RpClumpDestroy_t)                        invalid_ptr;
RpClumpGetNumAtomics_t                  RpClumpGetNumAtomics                    = (RpClumpGetNumAtomics_t)                  invalid_ptr;
RwAnimationInit_t                       RwAnimationInit                         = (RwAnimationInit_t)                       invalid_ptr;
RwSkeletonUpdate_t                      RwSkeletonUpdate                        = (RwSkeletonUpdate_t)                      invalid_ptr;
RpAtomicCreate_t                        RpAtomicCreate                          = (RpAtomicCreate_t)                        invalid_ptr;
RpAtomicClone_t                         RpAtomicClone                           = (RpAtomicClone_t)                         invalid_ptr;
RpAtomicSetFrame_t                      RpAtomicSetFrame                        = (RpAtomicSetFrame_t)                      invalid_ptr;
RpAtomicSetupObjectPipeline_t           RpAtomicSetupObjectPipeline             = (RpAtomicSetupObjectPipeline_t)           invalid_ptr;
RpAtomicSetupVehiclePipeline_t          RpAtomicSetupVehiclePipeline            = (RpAtomicSetupVehiclePipeline_t)          invalid_ptr;
RpAtomicRender_t                        RpAtomicRender                          = (RpAtomicRender_t)                        invalid_ptr;
RpAtomicSetGeometry_t                   RpAtomicSetGeometry                     = (RpAtomicSetGeometry_t)                   invalid_ptr;
RpAtomicDestroy_t                       RpAtomicDestroy                         = (RpAtomicDestroy_t)                       invalid_ptr;
RwObjectFrameRender_t                   RwObjectFrameRender                     = (RwObjectFrameRender_t)                   invalid_ptr;
RwTexDictionaryCreate_t                 RwTexDictionaryCreate                   = (RwTexDictionaryCreate_t)                 invalid_ptr;
RwTexDictionaryFinalizer_t              RwTexDictionaryFinalizer                = (RwTexDictionaryFinalizer_t)              invalid_ptr;
RwTexDictionaryStreamRead_t             RwTexDictionaryStreamRead               = (RwTexDictionaryStreamRead_t)             invalid_ptr;
RwTexDictionaryGetCurrent_t             RwTexDictionaryGetCurrent               = (RwTexDictionaryGetCurrent_t)             invalid_ptr;
RwTexDictionarySetCurrent_t             RwTexDictionarySetCurrent               = (RwTexDictionarySetCurrent_t)             invalid_ptr;
RwTexDictionaryForAllTextures_t         RwTexDictionaryForAllTextures           = (RwTexDictionaryForAllTextures_t)         invalid_ptr;
RwTexDictionaryFindNamedTexture_t       RwTexDictionaryFindNamedTexture         = (RwTexDictionaryFindNamedTexture_t)       invalid_ptr;
RwTexDictionaryAddTexture_t             RwTexDictionaryAddTexture               = (RwTexDictionaryAddTexture_t)             invalid_ptr;
RwTexDictionaryDestroy_t                RwTexDictionaryDestroy                  = (RwTexDictionaryDestroy_t)                invalid_ptr;
RwTextureCreate_t                       RwTextureCreate                         = (RwTextureCreate_t)                       invalid_ptr;
RwTextureUnlinkFromDictionary_t         RwTextureUnlinkFromDictionary           = (RwTextureUnlinkFromDictionary_t)         invalid_ptr;
RwTextureDestroy_t                      RwTextureDestroy                        = (RwTextureDestroy_t)                      invalid_ptr;
RpGeometryCreate_t                      RpGeometryCreate                        = (RpGeometryCreate_t)                      invalid_ptr;
RpGeometryAddRef_t                      RpGeometryAddRef                        = (RpGeometryAddRef_t)                      invalid_ptr;
RpGeometryGetAnimation_t                RpGeometryGetAnimation                  = (RpGeometryGetAnimation_t)                invalid_ptr;
RpGeometryTriangleSetVertexIndices_t    RpGeometryTriangleSetVertexIndices      = (RpGeometryTriangleSetVertexIndices_t)    invalid_ptr;
RpGeometryTriangleSetMaterial_t         RpGeometryTriangleSetMaterial           = (RpGeometryTriangleSetMaterial_t)         invalid_ptr;
RpGeometryUnlock_t                      RpGeometryUnlock                        = (RpGeometryUnlock_t)                      invalid_ptr;
RpGeometryLock_t                        RpGeometryLock                          = (RpGeometryLock_t)                        invalid_ptr;
RpGeometryTransform_t                   RpGeometryTransform                     = (RpGeometryTransform_t)                   invalid_ptr;
RpGeometryDestroy_t                     RpGeometryDestroy                       = (RpGeometryDestroy_t)                     invalid_ptr;
RwMatrixCreate_t                        RwMatrixCreate                          = (RwMatrixCreate_t)                        invalid_ptr;
RwMatrixInvert_t                        RwMatrixInvert                          = (RwMatrixInvert_t)                        invalid_ptr;
RwMatrixTranslate_t                     RwMatrixTranslate                       = (RwMatrixTranslate_t)                     invalid_ptr;
RwMatrixScale_t                         RwMatrixScale                           = (RwMatrixScale_t)                         invalid_ptr;
RwMatrixUnknown_t                       RwMatrixUnknown                         = (RwMatrixUnknown_t)                       invalid_ptr;
RpMaterialCreate_t                      RpMaterialCreate                        = (RpMaterialCreate_t)                      invalid_ptr;
RpMaterialDestroy_t                     RpMaterialDestroy                       = (RpMaterialDestroy_t)                     invalid_ptr;
RwV3dNormalize_t                        RwV3dNormalize                          = (RwV3dNormalize_t)                        invalid_ptr;
RwV3dTransformVector_t                  RwV3dTransformVector                    = (RwV3dTransformVector_t)                  invalid_ptr;
RwIm3DTransform_t                       RwIm3DTransform                         = (RwIm3DTransform_t)                       invalid_ptr;
RwIm3DRenderIndexedPrimitive_t          RwIm3DRenderIndexedPrimitive            = (RwIm3DRenderIndexedPrimitive_t)          invalid_ptr;
RwIm3DEnd_t                             RwIm3DEnd                               = (RwIm3DEnd_t)                             invalid_ptr;
RpLightSetRadius_t                      RpLightSetRadius                        = (RpLightSetRadius_t)                      invalid_ptr;
RpLightDestroy_t                        RpLightDestroy                          = (RpLightDestroy_t)                        invalid_ptr;
RwRasterCreate_t                        RwRasterCreate                          = (RwRasterCreate_t)                        invalid_ptr;
RwRasterUnlock_t                        RwRasterUnlock                          = (RwRasterUnlock_t)                        invalid_ptr;
RwRasterLock_t                          RwRasterLock                            = (RwRasterLock_t)                          invalid_ptr;
RwRasterDestroy_t                       RwRasterDestroy                         = (RwRasterDestroy_t)                       invalid_ptr;
RwSceneAddAtomic_t                      RwSceneAddAtomic                        = (RwSceneAddAtomic_t)                      invalid_ptr;
RwSceneAddLight_t                       RwSceneAddLight                         = (RwSceneAddLight_t)                       invalid_ptr;
RwSceneAddClump_t                       RwSceneAddClump                         = (RwSceneAddClump_t)                       invalid_ptr;
RwSceneRemoveLight_t                    RwSceneRemoveLight                      = (RwSceneRemoveLight_t)                    invalid_ptr;
RpPrtStdGlobalDataSetStreamEmbedded_t   RpPrtStdGlobalDataSetStreamEmbedded     = (RpPrtStdGlobalDataSetStreamEmbedded_t)   invalid_ptr;
RwPrefetch_t                            RwPrefetch                              = (RwPrefetch_t)                            invalid_ptr;
RwFlushLoader_t                         RwFlushLoader                           = (RwFlushLoader_t)                         invalid_ptr;

/*****************************************************************************/
/** GTA:SA function mappings                                                **/
/*****************************************************************************/

LoadCollisionModel_t            LoadCollisionModel              = (LoadCollisionModel_t)            invalid_ptr;
LoadCollisionModelVer2_t        LoadCollisionModelVer2          = (LoadCollisionModelVer2_t)        invalid_ptr;
LoadCollisionModelVer3_t        LoadCollisionModelVer3          = (LoadCollisionModelVer3_t)        invalid_ptr;
CClothesBuilder_CopyTexture_t   CClothesBuilder_CopyTexture     = (CClothesBuilder_CopyTexture_t)   invalid_ptr;

extern CGameSA *pGame;
extern CBaseModelInfoSAInterface **ppModelInfo;

RwInterface **ppRwInterface = (RwInterface**)0x00C97B24;

CRenderWareSA::CRenderWareSA( eGameVersion version )
{
    // Version dependant addresse
    switch( version )
    {
    // VERSION 1.0 EU ADDRESSES
    case VERSION_EU_10:
        RwRenderSystemFigureAffairs         = (RwRenderSystemFigureAffairs_t)           0x007F2AF0;
        RwObjectRegister                    = (RwObjectRegister_t)                      0x00808720;
        RwErrorGet                          = (RwErrorGet_t)                            0x008088C0;
        RwStreamInitialize                  = (RwStreamInitialize_t)                    0x007EC850;
        RwStreamOpen                        = (RwStreamOpen_t)                          0x007ECF30;
        RwStreamReadChunkHeaderInfo         = (RwStreamReadChunkHeaderInfo_t)           0x007ED5D0;
        RwStreamFindChunk                   = (RwStreamFindChunk_t)                     0x007ED310;
        RwStreamReadBlocks                  = (RwStreamReadBlocks_t)                    0x007ECA10;
        RwStreamReadTexture                 = (RwStreamReadTexture_t)                   0x00730EA0;
        RwStreamClose                       = (RwStreamClose_t)                         0x007ECE60;
        RtDictSchemaStreamReadDict          = (RtDictSchemaStreamReadDict_t)            0x007CF280;
        RtDictDestroy                       = (RtDictDestroy_t)                         0x007CF170;
        RwCameraClone                       = (RwCameraClone_t)                         0x007EF3F0;
        RwCameraDestroy                     = (RwCameraDestroy_t)                       0x007EE4F0;
        RwFrameCreate                       = (RwFrameCreate_t)                         0x007F0450;
        RwFrameCloneRecursive               = (RwFrameCloneRecursive_t)                 0x007F0090;
        RwFrameGetLTM                       = (RwFrameGetLTM_t)                         0x007F09D0;
        RwFrameSetIdentity                  = (RwFrameSetIdentity_t)                    0x007F10F0;
        RwFrameAddChild                     = (RwFrameAddChild_t)                       0x007F0B40;
        RwFrameRemoveChild                  = (RwFrameRemoveChild_t)                    0x007F0D10;
        RwFrameTranslate                    = (RwFrameTranslate_t)                      0x007F0E70;
        RwFrameScale                        = (RwFrameScale_t)                          0x007F0F10;
        RwFrameCloneHierarchy               = (RwFrameCloneHierarchy_t)                 0x007F0290;
        RwFrameDestroy                      = (RwFrameDestroy_t)                        0x007F04B0;
        RwCameraClone                       = (RwCameraClone_t)                         0x007EF3F0;
        RwCameraDestroy                     = (RwCameraDestroy_t)                       0x007EE4F0;
        RpClumpClone                        = (RpClumpClone_t)                          0x00749FC0;
        RpClumpAddAtomic                    = (RpClumpAddAtomic_t)                      0x0074A4E0;
        RpClumpRemoveAtomic                 = (RpClumpRemoveAtomic_t)                   0x0074A510;
        RpClumpAddLight                     = (RpClumpAddLight_t)                       0x0074A540;
        RpClumpStreamRead                   = (RpClumpStreamRead_t)                     0x0074B470;
        RpClumpGetNumAtomics                = (RpClumpGetNumAtomics_t)                  0x00749930;
        RpClumpDestroy                      = (RpClumpDestroy_t)                        0x0074A360;
        RpAtomicCreate                      = (RpAtomicCreate_t)                        0x00749CA0;
        RpAtomicClone                       = (RpAtomicClone_t)                         0x00749EB0;
        RpAtomicSetFrame                    = (RpAtomicSetFrame_t)                      0x0074BF70;
        RpAtomicSetGeometry                 = (RpAtomicSetGeometry_t)                   0x00749D90;
        RpAtomicRender                      = (RpAtomicRender_t)                        0x00749210;
        RpAtomicDestroy                     = (RpAtomicDestroy_t)                       0x00749E10;
        RwTexDictionaryCreate               = (RwTexDictionaryCreate_t)                 0x007F3640;
        RwTexDictionaryFinalizer            = (RwTexDictionaryFinalizer_t)              0x007F3980;
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
        RpGeometryTriangleSetVertexIndices  = (RpGeometryTriangleSetVertexIndices_t)    0x0074C6E0;
        RpGeometryUnlock                    = (RpGeometryUnlock_t)                      0x0074C850;
        RpGeometryLock                      = (RpGeometryLock_t)                        0x0074C820;
        RpGeometryTransform                 = (RpGeometryTransform_t)                   0x0074C030;
        RpGeometryDestroy                   = (RpGeometryDestroy_t)                     0x0074CD10;
        RwMatrixCreate                      = (RwMatrixCreate_t)                        0x007F2A90;
        RwMatrixInvert                      = (RwMatrixInvert_t)                        0x007F20B0;
        RwMatrixTranslate                   = (RwMatrixTranslate_t)                     0x007F2490;
        RwMatrixScale                       = (RwMatrixScale_t)                         0x007F2300;
        RpMaterialCreate                    = (RpMaterialCreate_t)                      0x0074D9E0;
        RpMaterialDestroy                   = (RpMaterialDestroy_t)                     0x0074DA70;
        RwV3dNormalize                      = (RwV3dNormalize_t)                        0x007ED9F0;
        RwV3dTransformVector                = (RwV3dTransformVector_t)                  0x007EDE00;
        RwIm3DTransform                     = (RwIm3DTransform_t)                       0x007EF490; 
        RwIm3DRenderIndexedPrimitive        = (RwIm3DRenderIndexedPrimitive_t)          0x007EF590;
        RwIm3DEnd                           = (RwIm3DEnd_t)                             0x007EF560;
        RpLightSetRadius                    = (RpLightSetRadius_t)                      0x00751AC0;
        RpLightDestroy                      = (RpLightDestroy_t)                        0x00752120;
        RwRasterUnlock                      = (RwRasterUnlock_t)                        0x007FAF00;
        RwRasterLock                        = (RwRasterLock_t)                          0x007FB310; 
        RwRasterCreate                      = (RwRasterCreate_t)                        0x007FB270;
        RwRasterDestroy                     = (RwRasterDestroy_t)                       0x007FB060;
        RwSceneAddAtomic                    = (RwSceneAddAtomic_t)                      0x00750FE0;
        RwSceneAddClump                     = (RwSceneAddClump_t)                       0x00751350;
        RwSceneAddLight                     = (RwSceneAddLight_t)                       0x00751960;
        RwSceneRemoveLight                  = (RwSceneRemoveLight_t)                    0x007519A0;
        RpPrtStdGlobalDataSetStreamEmbedded = (RpPrtStdGlobalDataSetStreamEmbedded_t)   0x0041B350;

        LoadCollisionModel                  = (LoadCollisionModel_t)                    0x00537580;
        LoadCollisionModelVer2              = (LoadCollisionModelVer2_t)                0x00537EE0;
        LoadCollisionModelVer3              = (LoadCollisionModelVer3_t)                0x00537CE0;
        CClothesBuilder_CopyTexture         = (CClothesBuilder_CopyTexture_t)           0x005A5730;
        break; 

    // VERSION 1.0 US ADDRESSES
    case VERSION_US_10:
        RwRenderSystemFigureAffairs         = (RwRenderSystemFigureAffairs_t)           0x007F2AB0;
        RwAllocAligned                      = (RwAllocAligned_t)                        0x0072F4C0;
        RwFreeAligned                       = (RwFreeAligned_t)                         0x0072F4F0;
        RwCreateExtension                   = (RwCreateExtension_t)                     0x007CCE80;
        RwObjectRegister                    = (RwObjectRegister_t)                      0x008086E0;
        RwStreamReadBlocks                  = (RwStreamReadBlocks_t)                    0x007EC9D0;
        RwStreamReadTexture                 = (RwStreamReadTexture_t)                   0x00730E60;
        RwStreamFindChunk                   = (RwStreamFindChunk_t)                     0x007ED2D0;
        RwErrorGet                          = (RwErrorGet_t)                            0x00808880;
        RwStreamInitialize                  = (RwStreamInitialize_t)                    0x007EC810;
        RwStreamOpen                        = (RwStreamOpen_t)                          0x007ECEF0;
        RwStreamReadChunkHeaderInfo         = (RwStreamReadChunkHeaderInfo_t)           0x007ED590;
        RwStreamClose                       = (RwStreamClose_t)                         0x007ECE20;
        RtDictSchemaStreamReadDict          = (RtDictSchemaStreamReadDict_t)            0x007CF240;
        RtDictDestroy                       = (RtDictDestroy_t)                         0x007CF130;
        RwCameraClone                       = (RwCameraClone_t)                         0x007EF3B0;
        RwCameraDestroy                     = (RwCameraDestroy_t)                       0x007EE4B0;
        RpClumpClone                        = (RpClumpClone_t)                          0x00749F70;
        RpClumpAddLight                     = (RpClumpAddLight_t)                       0x0074A4F0;
        RpClumpStreamRead                   = (RpClumpStreamRead_t)                     0x0074B420;
        RpClumpAddAtomic                    = (RpClumpAddAtomic_t)                      0x0074A490;
        RpClumpRemoveAtomic                 = (RpClumpRemoveAtomic_t)                   0x0074A4C0;
        RpClumpDestroy                      = (RpClumpDestroy_t)                        0x0074A310;
        RpClumpSetupFrameCallback           = (RpClumpSetupFrameCallback_t)             0x00733750;
        RpClumpGetNumAtomics                = (RpClumpGetNumAtomics_t)                  0x007498E0;
        RpClumpGetBoneTransform             = (RpClumpGetBoneTransform_t)               0x00735360;
        RwAnimationInit                     = (RwAnimationInit_t)                       0x007CD5E0;
        RwSkeletonUpdate                    = (RwSkeletonUpdate_t)                      0x007C5210;
        RwFrameCreate                       = (RwFrameCreate_t)                         0x007F0410;
        RwFrameCloneRecursive               = (RwFrameCloneRecursive_t)                 0x007F0050;
        RwFrameGetLTM                       = (RwFrameGetLTM_t)                         0x007F0990;
        RwFrameScale                        = (RwFrameScale_t)                          0x007F0ED0;
        RwFrameTranslate                    = (RwFrameTranslate_t)                      0x007F0E30;
        RwFrameAddChild                     = (RwFrameAddChild_t)                       0x007F0B00;
        RwFrameRemoveChild                  = (RwFrameRemoveChild_t)                    0x007F0CD0;
        RwFrameSetIdentity                  = (RwFrameSetIdentity_t)                    0x007F10B0;
        RwFrameCloneHierarchy               = (RwFrameCloneHierarchy_t)                 0x007f0250;
        RwFrameOrient                       = (RwFrameOrient_t)                         0x007F2010;
        RwFrameDestroy                      = (RwFrameDestroy_t)                        0x007F05A0;
        RpAtomicDestroy                     = (RpAtomicDestroy_t)                       0x00749DC0;
        RpAtomicSetGeometry                 = (RpAtomicSetGeometry_t)                   0x00749D40;
        RpAtomicSetFrame                    = (RpAtomicSetFrame_t)                      0x0074BF20;
        RpAtomicSetupObjectPipeline         = (RpAtomicSetupObjectPipeline_t)           0x005D7F00;
        RpAtomicSetupVehiclePipeline        = (RpAtomicSetupVehiclePipeline_t)          0x005D5B20;
        RpAtomicClone                       = (RpAtomicClone_t)                         0x00749E60;
        RpAtomicRender                      = (RpAtomicRender_t)                        0x007491C0;
        RpAtomicCreate                      = (RpAtomicCreate_t)                        0x00749C50;
        RwObjectFrameRender                 = (RwObjectFrameRender_t)                   0x00805750;
        RwTexDictionaryCreate               = (RwTexDictionaryCreate_t)                 0x007F3600;
        RwTexDictionaryFinalizer            = (RwTexDictionaryFinalizer_t)              0x008089C0;

        RwTexDictionaryStreamRead           = (RwTexDictionaryStreamRead_t)             0x00804C30;
        RwTexDictionaryGetCurrent           = (RwTexDictionaryGetCurrent_t)             0x007F3A90;
        RwTexDictionarySetCurrent           = (RwTexDictionarySetCurrent_t)             0x007F3A70;
        RwTexDictionaryForAllTextures       = (RwTexDictionaryForAllTextures_t)         0x007F3730;
        RwTexDictionaryAddTexture           = (RwTexDictionaryAddTexture_t)             0x007F3980;
        RwTexDictionaryDestroy              = (RwTexDictionaryDestroy_t)                0x007F36A0;
        RwTextureCreate                     = (RwTextureCreate_t)                       0x007F37C0;
        RwTextureDestroy                    = (RwTextureDestroy_t)                      0x007F3820;
        RwRasterUnlock                      = (RwRasterUnlock_t)                        0x007FAEC0;
        RwRasterLock                        = (RwRasterLock_t)                          0x007FB2D0;
        RwRasterCreate                      = (RwRasterCreate_t)                        0x007FB230;
        RwRasterDestroy                     = (RwRasterDestroy_t)                       0x007FB020;
        RpPrtStdGlobalDataSetStreamEmbedded = (RpPrtStdGlobalDataSetStreamEmbedded_t)   0x0041B350;
        RwTexDictionaryFindNamedTexture     = (RwTexDictionaryFindNamedTexture_t)       0x007F39F0;
        RpGeometryCreate                    = (RpGeometryCreate_t)                      0x0074CA90;
        RpGeometryTransform                 = (RpGeometryTransform_t)                   0x0074BFE0;
        RpGeometryGetAnimation              = (RpGeometryGetAnimation_t)                0x007C7590;
        RpGeometryTriangleSetVertexIndices  = (RpGeometryTriangleSetVertexIndices_t)    0x0074C690;
        RpGeometryTriangleSetMaterial       = (RpGeometryTriangleSetMaterial_t)         0x0074C710;
        RpGeometryUnlock                    = (RpGeometryUnlock_t)                      0x0074C800;
        RpGeometryLock                      = (RpGeometryLock_t)                        0x0074C7D0;
        RpGeometryTriangleSetMaterial       = (RpGeometryTriangleSetMaterial_t)         0x0074C6C0;
        RpGeometryDestroy                   = (RpGeometryDestroy_t)                     0x0074CCC0;
        RwMatrixCreate                      = (RwMatrixCreate_t)                        0x007F2A50;
        RwMatrixInvert                      = (RwMatrixInvert_t)                        0x007F2070;
        RwMatrixTranslate                   = (RwMatrixTranslate_t)                     0x007F2450;
        RwMatrixScale                       = (RwMatrixScale_t)                         0x007F22C0;
        RwMatrixUnknown                     = (RwMatrixUnknown_t)                       0x007326D0;
        RpMaterialCreate                    = (RpMaterialCreate_t)                      0x0074D990;
        RpMaterialDestroy                   = (RpMaterialDestroy_t)                     0x0074DA20;
        RwV3dNormalize                      = (RwV3dNormalize_t)                        0x007ED9B0;
        RwV3dTransformVector                = (RwV3dTransformVector_t)                  0x007EDDC0;
        RwIm3DTransform                     = (RwIm3DTransform_t)                       0x007EF450;
        RwIm3DRenderIndexedPrimitive        = (RwIm3DRenderIndexedPrimitive_t)          0x007EF550;
        RwIm3DEnd                           = (RwIm3DEnd_t)                             0x007EF520;
        RpLightSetRadius                    = (RpLightSetRadius_t)                      0x00751A70;
        RpLightDestroy                      = (RpLightDestroy_t)                        0x007520D0;
        RwSceneAddClump                     = (RwSceneAddClump_t)                       0x00751300;
        RwSceneAddLight                     = (RwSceneAddLight_t)                       0x00751910;
        RwSceneAddAtomic                    = (RwSceneAddAtomic_t)                      0x00750F90;
        RwSceneRemoveLight                  = (RwSceneRemoveLight_t)                    0x00751960;
        RwPrefetch                          = (RwPrefetch_t)                            0x0072F480;
        RwFlushLoader                       = (RwFlushLoader_t)                         0x0072E700;

        LoadCollisionModel                  = (LoadCollisionModel_t)                    0x00537580;
        LoadCollisionModelVer2              = (LoadCollisionModelVer2_t)                0x00537EE0;
        LoadCollisionModelVer3              = (LoadCollisionModelVer3_t)                0x00537CE0;
        CClothesBuilder_CopyTexture         = (CClothesBuilder_CopyTexture_t)           0x005A5730;
        break;
    }
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
            col = model->m_pColModel;
            model->m_pColModel = NULL;

            txd = (*ppTxdPool)->Get( model->m_textureDictionary );

            if ( !txd->m_txd )
            {
                CStreamingSA *streamer = pGame->GetStreaming();

                // VERY IMPORTANT: find a way to load the texDictionary and animation ourselves here!
                // We need to isolate this process from CStreaming, because CStreaming includes the model replacement fix
                // If you try to load a dff with a replaced model, the collision might crash you if you delete
                // a model which uses the same collision; the crash does not happen if all DFFs have a own collision
                // A fix would be to clone the collision somehow, but loading resources ourselves is required too!
                // Eventually: custom clump, txd and col async loading function in Lua

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
    }

    // read the clump with all its extensions
    RpClump *pClump = RpClumpStreamRead( streamModel );

    if ( id != 0 )
    {
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
            if ( !model->m_pColModel )
            {
                colOut = new CColModelSA( col, false );
            }
            else
            {
                colOut = new CColModelSA( model->m_pColModel, true );

                // If we loaded an atomic model and there is a custom collision, which should be very rare,
                // we keep the original collision once we want to restore our model
                // GTA:SA never deletes object models
                if ( model->GetRwModelType() == RW_ATOMIC )
                    colOut->SetOriginal( col );
            }

            // Restore the original colmodel as we have not requested the custom model yet
            model->m_pColModel = col;
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

/*****************************************************************************
*
*   RenderWare Functions
*
*****************************************************************************/

RwAtomicRenderChainInterface *rwRenderChains = (RwAtomicRenderChainInterface*)0x00C88070;
RwScene *const *p_gtaScene = (RwScene**)0x00C17038;


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

void RwObjectFrame::AddToFrame( RwFrame *frame )
{
    RemoveFromFrame();

    m_parent = frame;

    if ( !frame )
        return;

    LIST_INSERT( frame->m_objects.root, m_lFrame );
}

void RwObjectFrame::RemoveFromFrame()
{
    if ( !m_parent )
        return;

    LIST_REMOVE( m_lFrame );

    m_parent = NULL;
}

void RwFrame::SetModelling( const RwMatrix& mat )
{
    m_modelling = mat;

    // Set the frame to dirty
    m_privateFlags |= RW_FRAME_DIRTY;
}

void RwFrame::SetPosition( const CVector& pos )
{
    m_modelling.pos = pos;

    // Set the frame to dirty
    m_privateFlags |= RW_FRAME_DIRTY;
}

const RwMatrix& RwFrame::GetLTM() const
{
    // This function will recalculate the LTM if frame is dirty
    return *RwFrameGetLTM( this );
}

void RwFrame::Link( RwFrame *frame )
{
    // Unlink previous relationship of new child
    frame->Unlink();    // interesting side effect: cached if usage of parent

    // Insert the new child at the beginning
    frame->m_next = m_child;
    m_child = frame;

    frame->m_parent = this;

    frame->SetRootForHierarchy( m_root );
    frame->UnregisterRoot();

    // Mark the main root as independent
    m_root->RegisterRoot();
}

void RwFrame::Unlink()
{
    if ( !m_parent )
        return;

    if ( m_parent->m_child == this )
        m_parent->m_child = m_next;
    else
    {
        RwFrame *prev = m_parent->m_child;

        while ( prev->m_next != this )
            prev = prev->m_next;

        prev->m_next = m_next;
    }

    m_parent = NULL;
    m_next = NULL;

    SetRootForHierarchy( this );

    // Mark as independent
    RegisterRoot();
}

void RwFrame::SetRootForHierarchy( RwFrame *root )
{
    m_root = root;

    RwFrame *child = m_child;

    while ( child )
    {
        child->SetRootForHierarchy( root );

        child = child->m_next;
    }
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

RwFrame* RwFrame::GetFirstChild()
{
    return m_child;
}

struct _rwFrameFindName
{
    const char *name;
    RwFrame *result;
};

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

    if ( ForAllChildren( RwFrameGetFreeByName, &info ) )
        return NULL;

    return info.result;
}

static bool RwFrameGetByName( RwFrame *child, _rwFrameFindName *info )
{
    if ( stricmp( child->m_nodeName, info->name ) != 0 )
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
};

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

RwFrame* RwFrame::CloneRecursive() const
{
    RwFrame *cloned = RwFrameCloneRecursive( this, NULL );

    if ( !cloned )
        return NULL;

    cloned->m_privateFlags &= ~( RW_OBJ_REGISTERED | RW_FRAME_DIRTY );
    cloned->RegisterRoot();
    return cloned;
}

static bool RwFrameGetAnimHierarchy( RwFrame *frame, RpAnimHierarchy **rslt )
{
    if ( frame->m_anim )
    {
        *rslt = frame->m_anim;
        return false;
    }

    return frame->ForAllChildren( RwFrameGetAnimHierarchy, rslt );
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

struct _rwFindObjectType
{
    unsigned char type;
    RwObject *rslt;
};

static bool RwObjectGetByType( RwObject *child, _rwFindObjectType *info )
{
    if ( child->m_type != info->type )
        return true;

    info->rslt = child;
    return false;
}

RwObject* RwFrame::GetFirstObject( unsigned char type )
{
    _rwFindObjectType info;
    info.type = type;

    return ForAllObjects( RwObjectGetByType, &info ) ? NULL : info.rslt;
}

struct _rwObjectByIndex
{
    unsigned char type;
    unsigned int idx;
    unsigned int curIdx;
    RwObject *rslt;
};

static bool RwObjectGetByIndex( RwObject *obj, _rwObjectByIndex *info )
{
    if ( obj->m_type != info->type )
        return true;

    if ( info->idx != info->curIdx )
    {
        info->curIdx++;
        return true;
    }

    info->rslt = obj;
    return false;
}

RwObject* RwFrame::GetObjectByIndex( unsigned char type, unsigned int idx )
{
    _rwObjectByIndex info;
    info.type = type;
    info.idx = idx;
    info.curIdx = 0;
    
    if ( ForAllObjects( RwObjectGetByIndex, &info ) )
        return NULL;

    return info.rslt;
}

struct _rwObjCntByType
{
    unsigned char type;
    unsigned int cnt;
};

static bool RwFrameCountObjectsByType( RwObject *obj, _rwObjCntByType *info )
{
    if ( obj->m_type == info->type )
        info->cnt++;

    return true;
}

unsigned int RwFrame::CountObjectsByType( unsigned char type )
{
    _rwObjCntByType info;
    info.type = type;
    info.cnt = 0;

    ForAllObjects( RwFrameCountObjectsByType, &info );
    return info.cnt;
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
    return obj;
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

static bool RwObjectAtomicSetVisibilityFlags( RpAtomic *atomic, void *ud )
{
    atomic->ApplyVisibilityFlags( (unsigned short)ud );
    return true;
}

void RwFrame::SetAtomicVisibility( unsigned short flags )
{
    ForAllAtomics( RwObjectAtomicSetVisibilityFlags, (void*)flags );
}

static bool RwFrameAtomicBaseRoot( RpAtomic *atomic, RwFrame *root )
{
    RpAtomicSetFrame( atomic, root );
    return true;
}

struct _rwFrameVisibilityAtomics
{
    RpAtomic **primary;
    RpAtomic **secondary;
};

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
    if ( !(m_root->m_privateFlags & ( RW_OBJ_REGISTERED | RW_FRAME_DIRTY ) ) )
    {
        // Add it to the internal list
        LIST_INSERT( pRwInterface->m_nodeRoot.root, m_nodeRoot );

        m_root->m_privateFlags |= RW_OBJ_REGISTERED | RW_FRAME_DIRTY;
    }

    m_privateFlags |= RW_OBJ_REGISTERED | RW_OBJ_HIERARCHY_CACHED;
}

void RwFrame::UnregisterRoot()
{
    if ( !(m_root->m_privateFlags & ( RW_OBJ_REGISTERED | 0x01 ) ) )
        return;

    LIST_REMOVE( m_nodeRoot );

    m_privateFlags &= ~(RW_OBJ_REGISTERED | 1);
}

static bool RwTexDictionaryGetFirstTexture( RwTexture *tex, RwTexture **rslt )
{
    *rslt = tex;
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

static bool RwTexDictionaryFindTexture( RwTexture *tex, _rwTexDictFind *find )
{
    assert( tex->txd );

    if ( stricmp( tex->name, find->name ) != 0 )
        return true;

    find->tex = tex;
    return false;
}

RwTexture* RwTexDictionary::FindNamedTexture( const char *name )
{
    _rwTexDictFind find;
    find.name = name;

    if ( ForAllTextures( RwTexDictionaryFindTexture, &find ) )
        return NULL;

    return find.tex;
}

void RwTexture::AddToDictionary( RwTexDictionary *_txd )
{
    if ( txd )
        LIST_REMOVE( TXDList );

    LIST_INSERT( _txd->textures.root, TXDList );

    txd = _txd;
}

void RwTexture::RemoveFromDictionary()
{
    if ( !txd )
        return;

    LIST_REMOVE( TXDList );

    txd = NULL;
}

RwCamera* RwCameraCreate()
{
    RwCamera *cam = (RwCamera*)pRwInterface->m_allocStruct( pRwInterface->m_cameraInfo, 0x30005 );

    if ( !cam )
        return NULL;

    cam->m_type = RW_CAMERA;
    cam->m_subtype = 0;
    cam->m_flags = 0;
    cam->m_privateFlags = 0;
    cam->m_parent = NULL;

    cam->m_callback = (void*)0x007EE5A0;
    cam->m_preCallback = (RwCameraPreCallback)0x007EF370;
    cam->m_postCallback = (RwCameraPostCallback)0x007EF340;

    cam->m_screen.x = cam->m_screen.y = cam->m_screenInverse.x = cam->m_screenInverse.y = 1;
    cam->m_screenOffset.x = cam->m_screenOffset.y = 0;

    cam->m_nearplane = 0.05f;
    cam->m_farplane = 10;
    cam->m_fog = 5;

    cam->m_rendertarget = NULL;
    cam->m_bufferDepth = NULL;
    cam->m_camType = RW_CAMERA_PERSPECTIVE;

    // Do some plane shifting
    ((void (__cdecl*)( RwCamera* ))0x007EE200)( cam );

    cam->m_matrix.a = 0;

    RwObjectRegister( (void*)0x008E222C, cam );
    return cam;
}

void RwCamera::BeginUpdate()
{
    m_preCallback( this );
}

void RwCamera::EndUpdate()
{
    m_postCallback( this );
}

void RwCamera::AddToClump( RpClump *clump )
{
    // Bugfix: remove from previous clump
    RemoveFromClump();

    LIST_INSERT( clump->m_cameras.root, m_clumpCameras );

    m_clump = clump;
}

void RwCamera::RemoveFromClump()
{
    if ( !m_clump )
        return;

    LIST_REMOVE( m_clumpCameras );

    m_clump = NULL;
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
    return m_link = (RwRenderLink*)RwAllocAligned( (((count * sizeof(RwRenderLink) - 1) >> 6 ) + 1) << 6, 0x40 );
}

bool RpAtomic::IsNight()
{
    if ( m_pipeline == RW_ATOMIC_RENDER_NIGHT )
        return true;

    if ( m_pipeline == RW_ATOMIC_RENDER_REFLECTIVE )
        return false;

    return m_geometry->m_nightColor && m_geometry->m_colors != NULL;
}

void RpAtomic::AddToClump( RpClump *clump )
{
    RemoveFromClump();

    m_clump = clump;

    LIST_INSERT( clump->m_atomics.root, m_atomics );
}

void RpAtomic::RemoveFromClump()
{
    if ( !m_clump )
        return;

    LIST_REMOVE( m_atomics );

    m_clump = NULL;
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

void RpAtomic::FetchMateria( RpMaterials& mats )
{
    unsigned int n;

    if ( !(GetVisibilityFlags() & 0x20) )
        return;

    for (n=0; n<m_geometry->m_linkedMateria->m_count; n++)
        mats.Add( m_geometry->m_linkedMateria->Get(n)->m_material );
}

RpMaterials::RpMaterials( unsigned int max )
{
    m_data = (RpMaterial**)pRwInterface->m_malloc( sizeof(long) * max );

    m_max = max;
    m_entries = 0;
}

RpMaterials::~RpMaterials()
{
    unsigned int n;

    for ( n=0; n<m_entries; n++ )
        RpMaterialDestroy( m_data[n] );

    pRwInterface->m_free( m_data );

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

void RpLight::AddToClump( RpClump *clump )
{
    // Bugfix: remove from previous clump
    RemoveFromClump();

    LIST_INSERT( clump->m_lights.root, m_clumpLights );

    m_clump = clump;
}

void RpLight::RemoveFromClump()
{
    if ( !m_clump )
        return;

    LIST_REMOVE( m_clumpLights );

    m_clump = NULL;
}

void RpLight::AddToScene( RwScene *scene )
{
    RemoveFromScene();

    m_scene = scene;

    if ( scene->m_flags < 0x80 )
        LIST_INSERT( scene->m_lights.root, m_sceneLights );
    else
    {
        if ( scene->m_parent )
            scene->m_parent->RegisterRoot();

        LIST_INSERT( scene->m_activeLights.root, m_sceneLights );
    }
}

void RpLight::RemoveFromScene()
{
    if ( !m_scene )
        return;

    RwSceneRemoveLight( m_scene, this );
}

void RpClump::Render()
{
    LIST_FOREACH_BEGIN( RpAtomic, m_atomics.root, m_atomics )
        if ( item->IsVisible() )
        {
            item->m_parent->GetLTM();   // Possibly update it's world position
            item->m_renderCallback( item );
        }
    LIST_FOREACH_END
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

    link->m_position = frame->GetPosition();

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

    if ( !atomic || !atomic->m_geometry->m_skeleton )
    {
        // Initialize a non animated mesh
        link = geom->AllocateLink( m_parent->CountChildren() );

        // Assign all frames
        m_parent->ForAllChildren( RwAssignRenderLink, &link );

        // Init them
        geom->ForAllLinks( RwRenderLinkInit, (void*)NULL );

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
            info->m_offset = *offset;
            
            info++;
            offset++;
            link++;
            bone++;
        }
    }

    geom->ForAllLinks( RwAnimatedRenderLinkInit, (void*)NULL );

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
    
    return atomic->m_anim;
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

RpAtomic* RpClump::GetFirstAtomic()
{
    if ( LIST_EMPTY( m_atomics.root ) )
        return NULL;

    return LIST_GETITEM( RpAtomic, m_atomics.root.next, m_atomics );
}

RpAtomic* RpClump::GetLastAtomic()
{
    if ( LIST_EMPTY( m_atomics.root ) )
        return NULL;

    return LIST_GETITEM( RpAtomic, m_atomics.root.prev, m_atomics );
}

struct _rwFindAtomicNamed
{
    const char *name;
    RpAtomic *rslt;
};

static bool RpClumpFindNamedAtomic( RpAtomic *atom, _rwFindAtomicNamed *info )
{
    if ( strcmp( atom->m_parent->m_nodeName, info->name ) != 0 )
        return true;

    info->rslt = atom;
    return false;
}

RpAtomic* RpClump::FindNamedAtomic( const char *name )
{
    _rwFindAtomicNamed info;
    info.name = name;

    return ForAllAtomics( RpClumpFindNamedAtomic, &info ) ? NULL : info.rslt;
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

static bool RwAtomicSetupPipeline( RpAtomic *child, int )
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
    return true;
}

void RpClump::RemoveAtomicVisibilityFlags( unsigned short flags )
{
    ForAllAtomics( RwAtomicRemoveVisibilityFlags, flags );
}

static bool RwAtomicFetchMateria( RpAtomic *child, RpMaterials *mats )
{
    child->FetchMateria( *mats );
    return true;
}

void RpClump::FetchMateria( RpMaterials& mats )
{
    ForAllAtomics( RwAtomicFetchMateria, &mats );
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

        pRwInterface->m_matrixTransform3( offset, &mat.up, 1, skel->m_boneMatrices + matId );

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

bool RwMaterialAlphaCheck( RpMaterial *mat, int )
{
    return mat->m_color.a != 0xFF;
}

bool RpGeometry::IsAlpha()
{
    return !ForAllMateria( RwMaterialAlphaCheck, 0 );
}

bool RpMaterialTextureUnlink( RpMaterial *mat, int )
{
    if ( RwTexture *tex = mat->m_texture )
        mat->m_texture = NULL;

    return true;
}

void RpGeometry::UnlinkFX()
{
    // Clean all texture links
    ForAllMateria( RpMaterialTextureUnlink, 0 );

    if ( m_2dfx )
    {
        // Clean the 2dfx structure
        pRwInterface->m_free( m_2dfx );
        m_2dfx = NULL;
    }
}

bool RwAtomicRenderChainInterface::PushRender( RwAtomicZBufferEntry *level )
{
    RwAtomicRenderChain *iter = &m_root;
    RwAtomicRenderChain *progr;

    while ( iter->list.prev != &m_rootLast.list )
    {
        iter = LIST_GETITEM(RwAtomicRenderChain, iter->list.prev, list);

        if ( iter->m_entry.m_distance >= level->m_distance )
            break;
    }

    if ( ( progr = LIST_GETITEM(RwAtomicRenderChain, m_renderStack.list.prev, list) ) == &m_renderLast )
        return false;

    // Update render details
    progr->m_entry = *level;

    LIST_REMOVE( progr->list );
    LIST_INSERT( iter->list, progr->list );
    return true;
}

RwTexture* RwFindTexture( const char *name, const char *secName )
{
    RwTexture *tex = pRwInterface->m_textureManager.m_findInstanceRef( name );

    // The global store will reference textures
    if ( tex )
    {
        tex->refs++;
        return tex;
    }

    if ( !( tex = pRwInterface->m_textureManager.m_findInstance( name, secName ) ) )
    {
        // If we have not found anything, we tell the system about an error
        RwError err;
        err.err1 = 0x01;
        err.err2 = 0x16;

        // Actually, there is a missing texture handler; it is void though
        RwSetError( &err );
        return NULL;
    }

    if ( RwTexDictionary *txd = pRwInterface->m_textureManager.m_current )
    {
        tex->RemoveFromDictionary();
        tex->AddToDictionary( txd );
    }

    return tex;
}

RwError* RwSetError( RwError *info )
{
    if ( pRwInterface->m_errorInfo.err1 )
        return info;

    if ( pRwInterface->m_errorInfo.err2 != 0x80000000 )
        return info;

    if ( info->err1 & 0x80000000 )
        pRwInterface->m_errorInfo.err1 = 0;
    else
        pRwInterface->m_errorInfo.err1 = info->err1;

    pRwInterface->m_errorInfo.err2 = info->err2;
    return info;
}

static void* _lightCallback( void *ptr )
{
    return ptr;
}

RpLight* RpLightCreate( unsigned char type )
{
    RpLight *light = (RpLight*)pRwInterface->m_allocStruct( pRwInterface->m_lightInfo, 0x30012 );

    if ( !light )
        return NULL;

    light->m_type = RW_LIGHT;
    light->m_subtype = type;
    light->m_color.a = 0;
    light->m_color.r = 0;
    light->m_color.g = 0;
    light->m_color.b = 0;

    light->m_callback = (void*)_lightCallback;
    light->m_flags = 0;
    light->m_parent = NULL;

    light->m_radius = 0;
    light->unknown1 = 0;
    light->m_privateFlags = 0;

    // Clear the list awareness
    LIST_CLEAR( light->m_sectors.root );
    LIST_INITNODE( light->m_clumpLights );

    light->m_flags = 3; // why write it again? R* hack?
    light->m_frame = pRwInterface->m_frame;

    RwObjectRegister( (void*)0x008D62F8, light );
    return light;
}

void RpLight::SetColor( const RwColorFloat& color )
{
    m_color = color;
    
    // Check whether we are brightness only
    m_privateFlags = ( m_color.r == m_color.g && m_color.r == m_color.b );
}

static RpClump* _clumpCallback( RpClump *clump, void *data )
{
    return clump;
}

RpClump* RpClumpCreate()
{
    RpClump *clump = (RpClump*)pRwInterface->m_allocStruct( pRwInterface->m_clumpInfo, 0x30010 );

    if ( !clump )
        return NULL;

    clump->m_type = RW_CLUMP;

    LIST_CLEAR( clump->m_atomics.root );
    LIST_CLEAR( clump->m_lights.root );
    LIST_CLEAR( clump->m_cameras.root );

    clump->m_subtype = 0;
    clump->m_flags = 0;
    clump->m_privateFlags = 0;
    clump->m_parent = NULL;

    LIST_INITNODE( clump->m_globalClumps );

    clump->m_callback = _clumpCallback;

    RwObjectRegister( (void*)0x008D6264, clump );
    return clump;
}