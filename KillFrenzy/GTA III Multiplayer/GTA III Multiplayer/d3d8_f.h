#ifndef __D3D8__
#define __D3D8__

#define D3D_MAX_VERTICES 8
#define D3D_MAX_FONTS 0xFF
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "d3d8.h"

struct d3dproxy8devicevtbl_s
{
	// IUnknown stuff
	HRESULT					(__stdcall *QueryInterface)					( struct d3dproxy8device_s*, REFIID riid, void **ppvObj );
	ULONG					(__stdcall *AddRef)							( struct d3dproxy8device_s* );
	ULONG					(__stdcall *Release)							( struct d3dproxy8device_s* );

	// IDirect3D8Device stuff
	HRESULT					(__stdcall *TestCooperativeLevel)				( struct d3dproxy8device_s* );
	UINT					(__stdcall *GetAvailableTextureMem)			( struct d3dproxy8device_s* );
	HRESULT					(__stdcall *ResourceManagerDiscardBytes)		( struct d3dproxy8device_s*, DWORD Bytes );
//	HRESULT					(*EvictManagedResources)			( struct d3dproxy8device_s* );
	HRESULT					(__stdcall *GetDirect3D)						( struct d3dproxy8device_s*, IDirect3D8** ppD3D );
	HRESULT					(__stdcall *GetDeviceCaps)					( struct d3dproxy8device_s*, D3DCAPS8* pCaps );
	HRESULT					(__stdcall *GetDisplayMode)					( struct d3dproxy8device_s*, D3DDISPLAYMODE* pMode );
	HRESULT					(__stdcall *GetCreationParameters)			( struct d3dproxy8device_s*, D3DDEVICE_CREATION_PARAMETERS *pParameters );
	HRESULT					(__stdcall *SetCursorProperties)				( struct d3dproxy8device_s*, UINT XHotSpot,UINT YHotSpot,IDirect3DSurface8* pCursorBitmap );
	void					(__stdcall *SetCursorPosition)				( struct d3dproxy8device_s*, int X, int Y, DWORD Flags );
	BOOL					(__stdcall *ShowCursor)						( struct d3dproxy8device_s*, BOOL bShow );
	HRESULT					(__stdcall *CreateAdditionalSwapChain)		( struct d3dproxy8device_s*, D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain8** pSwapChain );
//	HRESULT					(*GetSwapChain)						( struct d3dproxy8device_s*, UINT uiSwapChain,IDirect3DSwapChain9** pSwapChain );
//	UINT					(*GetNumberOfSwapChains)			( struct d3dproxy8device_s*, VOID );
	HRESULT					(__stdcall *Reset)							( struct d3dproxy8device_s*, D3DPRESENT_PARAMETERS* pPresentationParameters );
	HRESULT					(__stdcall *Present)							( struct d3dproxy8device_s*, CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion );
	HRESULT					(__stdcall *GetBackBuffer)					( struct d3dproxy8device_s*, UINT uiBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface8** ppBackBuffer );
	HRESULT					(__stdcall *GetRasterStatus)					( struct d3dproxy8device_s*, D3DRASTER_STATUS* pRasterStatus );
//	HRESULT					(*SetDialogBoxMode)					( struct d3dproxy8device_s*, BOOL bEnableDialogs );
	VOID					(__stdcall *SetGammaRamp)						( struct d3dproxy8device_s*, DWORD Flags,CONST D3DGAMMARAMP* pRamp );
	VOID					(__stdcall *GetGammaRamp)						( struct d3dproxy8device_s*, D3DGAMMARAMP* pRamp );
	HRESULT					(__stdcall *CreateTexture)					( struct d3dproxy8device_s*, UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture8** ppTexture );
	HRESULT					(__stdcall *CreateVolumeTexture)				( struct d3dproxy8device_s*, UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture8** ppVolumeTexture );
	HRESULT					(__stdcall *CreateCubeTexture)				( struct d3dproxy8device_s*, UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture8** ppCubeTexture );
	HRESULT					(__stdcall *CreateVertexBuffer)				( struct d3dproxy8device_s*, UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer8** ppVertexBuffer );
	HRESULT					(__stdcall *CreateIndexBuffer)				( struct d3dproxy8device_s*, UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer8** ppIndexBuffer );
	HRESULT					(__stdcall *CreateRenderTarget)				( struct d3dproxy8device_s*, UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,BOOL Lockable,IDirect3DSurface8** ppSurface );
	HRESULT					(__stdcall *CreateDepthStencilSurface)		( struct d3dproxy8device_s*, UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,IDirect3DSurface8** ppSurface );
	HRESULT					(__stdcall *CreateImageSurface)				( struct d3dproxy8device_s*, UINT Width,UINT Height,D3DFORMAT Format,IDirect3DSurface8** ppSurface );
	HRESULT					(__stdcall *CopyRects)						( struct d3dproxy8device_s*, IDirect3DSurface8* pSourceSurface,CONST RECT* pSourceRectsArray,UINT cRects,IDirect3DSurface8* pDestinationSurface,CONST POINT* pDestPointsArray );
//	HRESULT					(*UpdateSurface)					( struct d3dproxy8device_s*, IDirect3DSurface8* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface8* pDestinationSurface,CONST POINT* pDestPoint );
	HRESULT					(__stdcall *UpdateTexture)					( struct d3dproxy8device_s*, IDirect3DBaseTexture8* pSourceTexture,IDirect3DBaseTexture8* pDestinationTexture );
//	HRESULT					(*GetRenderTargetData)				( struct d3dproxy8device_s*, IDirect3DSurface8* pRenderTarget,IDirect3DSurface8* pDestSurface );
	HRESULT					(__stdcall *GetFrontBuffer)					( struct d3dproxy8device_s*, IDirect3DSurface8* pDestSurface );
//	HRESULT					(*GetFrontBufferData)				( struct d3dproxy8device_s*, UINT iSwapChain,IDirect3DSurface8* pDestSurface );
//	HRESULT					(*StretchRect)						( struct d3dproxy8device_s*, IDirect3DSurface8* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface8* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter );
//	HRESULT					(*ColorFill)						( struct d3dproxy8device_s*, IDirect3DSurface8* pSurface,CONST RECT* pRect,D3DCOLOR color );
//	HRESULT					(*CreateOffscreenPlainSurface)		( struct d3dproxy8device_s*, UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface8** ppSurface,HANDLE* pSharedHandle );
	HRESULT					(__stdcall *SetRenderTarget)					( struct d3dproxy8device_s*, IDirect3DSurface8* pRenderTarget,IDirect3DSurface8* pNewZStencil );
	HRESULT					(__stdcall *GetRenderTarget)					( struct d3dproxy8device_s*, IDirect3DSurface8** ppRenderTarget );
//	HRESULT					(*SetDepthStencilSurface)			( struct d3dproxy8device_s*, IDirect3DSurface8* pNewZStencil );
	HRESULT					(__stdcall *GetDepthStencilSurface)			( struct d3dproxy8device_s*, IDirect3DSurface8** ppZStencilSurface );
	HRESULT					(__stdcall *BeginScene)						( struct d3dproxy8device_s* );
	HRESULT					(__stdcall *EndScene)							( struct d3dproxy8device_s* );
	HRESULT					(__stdcall *Clear)							( struct d3dproxy8device_s*, DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil );
	HRESULT					(__stdcall *SetTransform)						( struct d3dproxy8device_s*, D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix );
	HRESULT					(__stdcall *GetTransform)						( struct d3dproxy8device_s*, D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix );
	HRESULT					(__stdcall *MultiplyTransform)				( struct d3dproxy8device_s*, D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix );
	HRESULT					(__stdcall *SetViewport)						( struct d3dproxy8device_s*, CONST D3DVIEWPORT8* pViewport );
	HRESULT					(__stdcall *GetViewport)						( struct d3dproxy8device_s*, D3DVIEWPORT8* pViewport );
	HRESULT					(__stdcall *SetMaterial)						( struct d3dproxy8device_s*, CONST D3DMATERIAL8* pMaterial );
	HRESULT					(__stdcall *GetMaterial)						( struct d3dproxy8device_s*, D3DMATERIAL8* pMaterial );
	HRESULT					(__stdcall *SetLight)							( struct d3dproxy8device_s*, DWORD Index,CONST D3DLIGHT8* pLight );
	HRESULT					(__stdcall *GetLight)							( struct d3dproxy8device_s*, DWORD Index,D3DLIGHT8* pLight );
	HRESULT					(__stdcall *LightEnable)						( struct d3dproxy8device_s*, DWORD Index,BOOL Enable );
	HRESULT					(__stdcall *GetLightEnable)					( struct d3dproxy8device_s*, DWORD Index,BOOL* pEnable );
	HRESULT					(__stdcall *SetClipPlane)						( struct d3dproxy8device_s*, DWORD Index,CONST float* pPlane );
	HRESULT					(__stdcall *GetClipPlane)						( struct d3dproxy8device_s*, DWORD Index,float* pPlane );
	HRESULT					(__stdcall *SetRenderState)					( struct d3dproxy8device_s*, D3DRENDERSTATETYPE State,DWORD Value );
	HRESULT					(__stdcall *GetRenderState)					( struct d3dproxy8device_s*, D3DRENDERSTATETYPE State,DWORD* pValue );
	HRESULT					(__stdcall *BeginStateBlock)					( struct d3dproxy8device_s* );
	HRESULT					(__stdcall *EndStateBlock)					( struct d3dproxy8device_s*, DWORD *pToken );
	HRESULT					(__stdcall *ApplyStateBlock)					( struct d3dproxy8device_s*, DWORD Token );
	HRESULT					(__stdcall *CaptureStateBlock)				( struct d3dproxy8device_s*, DWORD Token );
	HRESULT					(__stdcall *DeleteStateBlock)					( struct d3dproxy8device_s*, DWORD Token );
	HRESULT					(__stdcall *CreateStateBlock)					( struct d3dproxy8device_s*, D3DSTATEBLOCKTYPE Type,DWORD *pToken );
	HRESULT					(__stdcall *SetClipStatus)					( struct d3dproxy8device_s*, CONST D3DCLIPSTATUS8* pClipStatus );
	HRESULT					(__stdcall *GetClipStatus)					( struct d3dproxy8device_s*, D3DCLIPSTATUS8* pClipStatus );
	HRESULT					(__stdcall *GetTexture)						( struct d3dproxy8device_s*, DWORD Stage,IDirect3DBaseTexture8** ppTexture );
	HRESULT					(__stdcall *SetTexture)						( struct d3dproxy8device_s*, DWORD Stage,IDirect3DBaseTexture8* pTexture );
	HRESULT					(__stdcall *GetTextureStageState)				( struct d3dproxy8device_s*, DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue );
	HRESULT					(__stdcall *SetTextureStageState)				( struct d3dproxy8device_s*, DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value );
//	HRESULT					(*GetSamplerState)					( struct d3dproxy8device_s*, DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue );
//	HRESULT					(*SetSamplerState)					( struct d3dproxy8device_s*, DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value );
	HRESULT					(__stdcall *ValidateDevice)					( struct d3dproxy8device_s*, DWORD* pNumPasses );
	HRESULT					(__stdcall *GetInfo)							( struct d3dproxy8device_s*, DWORD DevInfoID, void *pDevInfoStruct, DWORD DevInfoStructSize );
	HRESULT					(__stdcall *SetPaletteEntries)				( struct d3dproxy8device_s*, UINT PaletteNumber,CONST PALETTEENTRY* pEntries );
	HRESULT					(__stdcall *GetPaletteEntries)				( struct d3dproxy8device_s*, UINT PaletteNumber,PALETTEENTRY* pEntries );
	HRESULT					(__stdcall *SetCurrentTexturePalette)			( struct d3dproxy8device_s*, UINT PaletteNumber );
	HRESULT					(__stdcall *GetCurrentTexturePalette)			( struct d3dproxy8device_s*, UINT *pPaletteNumber );
//	HRESULT					(__stdcall *SetScissorRect)					( struct d3dproxy8device_s*, CONST RECT* pRect );
//	HRESULT					(__stdcall *GetScissorRect)					( struct d3dproxy8device_s*, RECT* pRect );
//	HRESULT					(*SetSoftwareVertexProcessing)		( struct d3dproxy8device_s*, BOOL bSoftware );
//	BOOL					(*GetSoftwareVertexProcessing)		( struct d3dproxy8device_s*, VOID );
//	HRESULT					(*SetNPatchMode)					( struct d3dproxy8device_s*, float nSegments );
//	FLOAT					(*GetNPatchMode)					( struct d3dproxy8device_s*, VOID );
	HRESULT					(__stdcall *DrawPrimitive)					( struct d3dproxy8device_s*, D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount );
	HRESULT					(__stdcall *DrawIndexedPrimitive)				( struct d3dproxy8device_s*, D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount );
	HRESULT					(__stdcall *DrawPrimitiveUP)					( struct d3dproxy8device_s*, D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride );
	HRESULT					(__stdcall *DrawIndexedPrimitiveUP)			( struct d3dproxy8device_s*, D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride );
	HRESULT					(__stdcall *ProcessVertices)					( struct d3dproxy8device_s*, UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer8* pDestBuffer,DWORD Flags );
//	HRESULT					(*CreateVertexDeclaration)			( struct d3dproxy8device_s*, CONST D3DVERTEXELEMENT8* pVertexElements,IDirect3DVertexDeclaration8** ppDecl );
//	HRESULT					(*SetVertexDeclaration)				( struct d3dproxy8device_s*, IDirect3DVertexDeclaration8* pDecl );
//	HRESULT					(*GetVertexDeclaration)				( struct d3dproxy8device_s*, IDirect3DVertexDeclaration8** ppDecl );
//	HRESULT					(*SetFVF)							( struct d3dproxy8device_s*, DWORD FVF );
//	HRESULT					(*GetFVF)							( struct d3dproxy8device_s*, DWORD* pFVF );
	HRESULT					(__stdcall *CreateVertexShader)				( struct d3dproxy8device_s*, CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage );
	HRESULT					(__stdcall *SetVertexShader)					( struct d3dproxy8device_s*, DWORD Handle );
	HRESULT					(__stdcall *GetVertexShader)					( struct d3dproxy8device_s*, DWORD* pHandle );
	HRESULT					(__stdcall *DeleteVertexShader)				( struct d3dproxy8device_s*, DWORD Handle );
//	HRESULT					(*SetVertexShaderConstantF)			( struct d3dproxy8device_s*, UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount );
//	HRESULT					(*GetVertexShaderConstantF)			( struct d3dproxy8device_s*, UINT StartRegister,float* pConstantData,UINT Vector4fCount );
//	HRESULT					(*SetVertexShaderConstantI)			( struct d3dproxy8device_s*, UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount );
//	HRESULT					(*GetVertexShaderConstantI)			( struct d3dproxy8device_s*, UINT StartRegister,int* pConstantData,UINT Vector4iCount );
//	HRESULT					(*SetVertexShaderConstantB)			( struct d3dproxy8device_s*, UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount );
//	HRESULT					(*GetVertexShaderConstantB)			( struct d3dproxy8device_s*, UINT StartRegister,BOOL* pConstantData,UINT BoolCount );
	HRESULT					(__stdcall *SetVertexShaderConstant)			( struct d3dproxy8device_s*, DWORD Register,CONST void* pConstantData,DWORD ConstantCount );
	HRESULT					(__stdcall *GetVertexShaderConstant)			( struct d3dproxy8device_s*, UINT Register,void* pConstantData,UINT ConstantCount );
	HRESULT					(__stdcall *GetVertexShaderDeclaration)		( struct d3dproxy8device_s*, DWORD Handle,void* pData,DWORD* pSizeOfData );
	HRESULT					(__stdcall *GetVertexShaderFunction)			( struct d3dproxy8device_s*, DWORD Handle,void* pData,DWORD* pSizeOfData );
	HRESULT					(__stdcall *SetStreamSource)					( struct d3dproxy8device_s*, UINT StreamNumber,IDirect3DVertexBuffer8* pStreamData,UINT Stride );
	HRESULT					(__stdcall *GetStreamSource)					( struct d3dproxy8device_s*, UINT StreamNumber,IDirect3DVertexBuffer8** ppStreamData,UINT* pStride );
//	HRESULT					(*SetStreamSourceFreq)				( struct d3dproxy8device_s*, UINT StreamNumber,UINT Setting );
//	HRESULT					(*GetStreamSourceFreq)				( struct d3dproxy8device_s*, UINT StreamNumber,UINT* pSetting );
	HRESULT					(__stdcall *SetIndices)						( struct d3dproxy8device_s*, IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex );
	HRESULT					(__stdcall *GetIndices)						( struct d3dproxy8device_s*, IDirect3DIndexBuffer8** ppIndexData, UINT *pBaseVertexIndex );
	HRESULT					(__stdcall *CreatePixelShader)				( struct d3dproxy8device_s*, CONST DWORD* pFunction, DWORD *pHandle );
	HRESULT					(__stdcall *SetPixelShader)					( struct d3dproxy8device_s*, DWORD Handle );
	HRESULT					(__stdcall *GetPixelShader)					( struct d3dproxy8device_s*, DWORD* pHandle );
	HRESULT					(__stdcall *DeletePixelShader)				( struct d3dproxy8device_s*, DWORD Handle );
//	HRESULT					(*SetPixelShaderConstantF)			( struct d3dproxy8device_s*, UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount );
//	HRESULT					(*GetPixelShaderConstantF)			( struct d3dproxy8device_s*, UINT StartRegister,float* pConstantData,UINT Vector4fCount );
//	HRESULT					(*SetPixelShaderConstantI)			( struct d3dproxy8device_s*, UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount );
//	HRESULT					(*GetPixelShaderConstantI)			( struct d3dproxy8device_s*, UINT StartRegister,int* pConstantData,UINT Vector4iCount );
//	HRESULT					(*SetPixelShaderConstantB)			( struct d3dproxy8device_s*, UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount );
//	HRESULT					(*GetPixelShaderConstantB)			( struct d3dproxy8device_s*, UINT StartRegister,BOOL* pConstantData,UINT BoolCount );
	HRESULT					(__stdcall *SetPixelShaderConstant)			( struct d3dproxy8device_s*, DWORD Register,CONST void* pConstantData,DWORD ConstantCount );
	HRESULT					(__stdcall *GetPixelShaderConstant)			( struct d3dproxy8device_s*, DWORD Register,void* pConstantData,DWORD ConstantCount );
	HRESULT					(__stdcall *GetPixelShaderFunction)			( struct d3dproxy8device_s*, DWORD Handle,void* pData,DWORD* pSizeOfData );
	HRESULT					(__stdcall *DrawRectPatch)					( struct d3dproxy8device_s*, UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo );
	HRESULT					(__stdcall *DrawTriPatch)						( struct d3dproxy8device_s*, UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo );
	HRESULT					(__stdcall *DeletePatch)						( struct d3dproxy8device_s*, UINT Handle );
//	HRESULT					(*CreateQuery)						( struct d3dproxy8device_s*, D3DQUERYTYPE Type,IDirect3DQuery8** ppQuery );
};

// Proxy device d3d8
typedef struct d3dproxy8device_s
{
	struct d3dproxy8devicevtbl_s* lpVtbl;
} d3dproxy8device_t;

struct d3dproxy8vtbl_s
{
	// Destructor
	//void					(*Destructor)						( struct d3dproxy8_s* );

	// IUnknown stuff
	HRESULT					(__stdcall *QueryInterface)					( struct d3dproxy8_s*, REFIID riid, void **ppvObj );
	ULONG					(__stdcall *AddRef)							( struct d3dproxy8_s* );
	ULONG					(__stdcall *Release)							( struct d3dproxy8_s* );

	// IDirect3D8 stuff
	HRESULT					(__stdcall *RegisterSoftwareDevice)			( struct d3dproxy8_s*, void *pInitFunction );
	UINT					(__stdcall *GetAdapterCount)					( struct d3dproxy8_s* );
	HRESULT					(__stdcall *GetAdapterIdentifier)				( struct d3dproxy8_s*, UINT adapter, DWORD flags, D3DADAPTER_IDENTIFIER8* pIdentifier );
	UINT					(__stdcall *GetAdapterModeCount)				( struct d3dproxy8_s*, UINT adapter );
	HRESULT					(__stdcall *EnumAdapterModes)					( struct d3dproxy8_s*, UINT adapter, UINT mode, D3DDISPLAYMODE *pMode );
	HRESULT					(__stdcall *GetAdapterDisplayMode)			( struct d3dproxy8_s*, UINT adapter, D3DDISPLAYMODE *pMode );
	HRESULT					(__stdcall *CheckDeviceType)					( struct d3dproxy8_s*, UINT adapter, D3DDEVTYPE devType, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat, BOOL bWindowed );
	HRESULT					(__stdcall *CheckDeviceFormat)				( struct d3dproxy8_s*, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat );
	HRESULT					(__stdcall *CheckDeviceMultiSampleType)		( struct d3dproxy8_s*, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType );
	HRESULT					(__stdcall *CheckDepthStencilMatch)			( struct d3dproxy8_s*, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat );
//	HRESULT					(*CheckDeviceFormatConversion)		( struct d3dproxy8_s*, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat );
	HRESULT					(__stdcall *GetDeviceCaps)					( struct d3dproxy8_s*, UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps );
	HMONITOR				(__stdcall *GetAdapterMonitor)				( struct d3dproxy8_s*, UINT Adapter );
	HRESULT					(__stdcall *CreateDevice)						( struct d3dproxy8_s*, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface );
};

// Our proxy instance
typedef struct d3dproxy8_s
{
	struct d3dproxy8vtbl_s* lpVtbl;
} d3dproxy8_t;

// Font information structure
typedef struct d3dfont_s
{
	char face_name[32];
	ID3DXFont *pFont;
	HFONT hFont;
} d3dfont_t;

#define D3D_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

// Vertex declaration after given type
typedef struct d3dvertex_s
{
	float x,y,z;
	float rwh;
	D3DCOLOR color;
} d3dvertex_t;

// D3D defintions
#define D3DPRIO_LOW		0x10000000
#define D3DPRIO_MED		0x10000001
#define D3DPRIO_HIGH	0x10000002
#define D3D_GetAbsFromRel2D(fRX,fRY) (float)((m_iScreenWidth)*(fRX)),(float)((m_iScreenHeight)*(fRY))
#define D3D_FONTHEIGHT	15

// Direct3D Rendering Functions
void	D3D_CaptureState ();
void	D3D_RestoreState ();
HRESULT	D3D_BeginDrawing ();
HRESULT	D3D_EndDrawing ();
ID3DXFont*	D3D_GetFont ( char *file_path );
void	D3D_DrawRect ( float x, float y, float width, float height, D3DCOLOR Color );
int		D3D_GetCharWidth ( ID3DXFont *pFont, char cChar, float fScale );
// Init
void	D3D8_Init();

// Exports
extern HWND hGTAWindow;
extern UINT m_uiDeviceAdapter;
extern IDirect3D8 *m_pOrigDevice;
extern IDirect3DDevice8 *m_pOrigDDevice;
extern IDirect3DSurface8 *m_pOrigTarget;
extern int m_iScreenWidth, m_iScreenHeight;

#endif //__D3D8__