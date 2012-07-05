// Direct3D8 library, hooking and stuff
#include "StdInc.h"
IDirect3D8 *m_pOrigDevice=0;
d3dproxy8_t *m_pDevice=0;
IDirect3DDevice8 *m_pOrigDDevice=0;
d3dproxy8device_t *m_pDDevice=0;
UINT m_uiDeviceAdapter;
D3DDEVTYPE m_DeviceType=(D3DDEVTYPE)0;

unsigned int	m_uiRefCount=0;
unsigned int	m_uiRefCountDevice=0;
HWND hGTAWindow=0;

/*===============================================
	Direct3D Rendering Functions
===============================================*/
LPDIRECT3DTEXTURE8 m_pFontTexture=0;
LPDIRECT3DVERTEXBUFFER8 m_pVertexBuffer=0;
IDirect3DSurface8 *m_pOrigTarget=0;
d3dfont_t *m_pFonts[D3D_MAX_FONTS];
unsigned int m_uiNumFonts=0;
int	m_iScreenWidth=0, m_iScreenHeight=0;
UINT m_uiFontWidth=0;
UINT m_uiFontHeight=0;
DWORD m_cacheStateBlock=0;
BOOL m_bIsDrawing=0;

// Caches the states previously
void	D3D_CaptureState ()
{
	if ( m_cacheStateBlock == 0 )
	{
		m_pOrigDDevice->CreateStateBlock ( D3DSBT_ALL, &m_cacheStateBlock );
	}
}

// Restores the state
void	D3D_RestoreState ()
{
	if ( m_cacheStateBlock )
	{
		m_pOrigDDevice->ApplyStateBlock ( m_cacheStateBlock );
		m_pOrigDDevice->DeleteStateBlock ( m_cacheStateBlock );
		m_cacheStateBlock = 0;
	}
}

// Begins our drawing session
HRESULT	D3D_BeginDrawing ()
{
	if ( m_bIsDrawing )
		return D3D_OK;

	m_pOrigDDevice->SetRenderState ( D3DRS_ZENABLE, TRUE );
	m_pOrigDDevice->SetRenderState ( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
	m_pOrigDDevice->SetRenderState ( D3DRS_ALPHABLENDENABLE, TRUE );
	m_pOrigDDevice->SetRenderState ( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pOrigDDevice->SetRenderState ( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pOrigDDevice->SetRenderState ( D3DRS_ALPHATESTENABLE, TRUE );
	m_pOrigDDevice->SetRenderState ( D3DRS_ALPHAREF, 0x08 );
	m_pOrigDDevice->SetRenderState ( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
	m_pOrigDDevice->SetRenderState ( D3DRS_LIGHTING, FALSE );
	m_pOrigDDevice->SetRenderState ( D3DRS_SHADEMODE, D3DSHADE_FLAT );
	m_pOrigDDevice->SetTextureStageState ( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	m_pOrigDDevice->SetTextureStageState ( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	m_pOrigDDevice->SetTextureStageState ( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
	m_pOrigDDevice->SetTextureStageState ( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
	m_pOrigDDevice->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
	m_pOrigDDevice->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	m_bIsDrawing=TRUE;
	return D3D_OK;
}

// Ends the drawing session
HRESULT	D3D_EndDrawing ()
{
	m_pOrigDDevice->SetTexture ( 0, NULL );
	m_bIsDrawing=FALSE;
	return D3D_OK;
}

// Rendering library invalid handle
void	D3D_RendInvalid ( IDirect3DDevice8 *pDevice )
{
	unsigned int n;

	if ( m_pOrigTarget )
		m_pOrigTarget->Release();

	// Release the vertex buffer
	if ( m_pVertexBuffer )
		m_pVertexBuffer->Release();

	// Release all fonts
	for ( n=0; n<m_uiNumFonts; n++ )
	{
		m_pFonts[n]->pFont->Release ( );
		m_pFonts[n]->pFont=NULL;
	}
}

// Rendering library restore handle
void	D3D_RendRestore ( IDirect3DDevice8 *pDevice )
{
	unsigned int n;

	m_pOrigDDevice->GetRenderTarget ( &m_pOrigTarget );

	// Recreate the vertex buffer
	m_pOrigDDevice->CreateVertexBuffer ( D3D_MAX_VERTICES*sizeof(d3dvertex_t), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, D3D_VERTEX, D3DPOOL_DEFAULT, &m_pVertexBuffer );
	assert ( m_pVertexBuffer != NULL );
	m_pVertexBuffer->SetPriority ( D3DPRIO_LOW );

	// Recreate all fonts
	for (n=0; n<m_uiNumFonts; n++)
	{
		HFONT hFont=CreateFont ( 100, 0, 0, 0, 0, 1, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH || FF_DONTCARE, m_pFonts[n]->face_name );
		D3DXCreateFont ( m_pOrigDDevice, hFont, &m_pFonts[n]->pFont );

		assert ( m_pFonts[n]->pFont != NULL );
		m_pFonts[n]->hFont=hFont;
	}
}

// Loads a font texture and sets it as current font to display
BOOL	D3D_LoadFontTextureByFile ( char *file_path )
{
	D3DSURFACE_DESC desc;

	// Free our old texture
	if (m_pFontTexture)
		m_pFontTexture->Release();

	// Try to load it
	if ( !(D3D_OK == D3DXCreateTextureFromFile ( m_pOrigDDevice, file_path, &m_pFontTexture )) )
		return FALSE;

	m_pFontTexture->GetLevelDesc( 0, &desc );
	m_uiFontWidth = desc.Width;
	m_uiFontHeight = desc.Height;

	return TRUE;
}

// Gets a font by filename
ID3DXFont*	D3D_GetFont ( char *file_path )
{
	char *realname=file_path;
	char *lrslt=NULL;
	unsigned int n;
	d3dfont_t *new_font;
	HFONT hFont;

	// Get the filename
	while (*realname)
	{
		lrslt=(realname);
		while (*realname++!='\\' && *realname!='/' && *realname);
	}
	if (!lrslt)
		return NULL;

	// Check our fonts if we have it created already
	for (n=0; n<m_uiNumFonts; n++)
	{
		if (strcmp(m_pFonts[n]->face_name,lrslt)==0)
			return m_pFonts[n]->pFont;
	}

	// Load the font
	if (!AddFontResourceEx(file_path, FR_PRIVATE, 0))
		return NULL;
	if (!(hFont = CreateFont ( D3D_FONTHEIGHT, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH || FF_DONTCARE, lrslt )))
		return NULL;

	// Create it
	new_font=(d3dfont_t*)malloc(sizeof(*new_font));
	if (D3DXCreateFont(m_pOrigDDevice, hFont, &new_font->pFont) != D3D_OK)
	{
		//CloseHandle(hFont);
		free(new_font);
		return NULL;
	}

	strncpy(new_font->face_name, lrslt, 31);
	new_font->hFont=hFont;
	m_pFonts[m_uiNumFonts]=new_font;
	m_uiNumFonts++;
	return new_font->pFont;
}

// Draws a rectangle on screen
void	D3D_DrawRect ( float x, float y, float width, float height, D3DCOLOR Color )
{
	d3dvertex_t *rectVert;

	if (!m_pVertexBuffer)
		return;

	m_pVertexBuffer->Lock ( 0, 0, (BYTE**)&rectVert, 0 );
	rectVert[0].x = x;
	rectVert[0].y = y+height;
	rectVert[0].z = 0;
	rectVert[0].rwh = 1;
	rectVert[0].color = Color;
	rectVert[1].x = x+width;
	rectVert[1].y = y+height;
	rectVert[1].z = 0;
	rectVert[1].rwh = 1;
	rectVert[1].color = Color;
	rectVert[2].x = x+width;
	rectVert[2].y = y;
	rectVert[2].z = 0;
	rectVert[2].rwh = 1;
	rectVert[2].color = Color;
	rectVert[3].x = x;
	rectVert[3].y = y;
	rectVert[3].z = 0;
	rectVert[3].rwh = 1;
	rectVert[3].color = Color;
	/*rectVert[4].x = x+width;
	rectVert[4].y = y;
	rectVert[4].z = 0;
	rectVert[4].rwh = 1;
	rectVert[4].color = Color;
	rectVert[5].x = x;
	rectVert[5].y = y+height;
	rectVert[5].z = 0;
	rectVert[5].rwh = 1;
	rectVert[5].color = Color;*/
	m_pVertexBuffer->Unlock ( );

	m_pOrigDDevice->SetTexture ( 0, NULL );
	m_pOrigDDevice->SetStreamSource ( 0, m_pVertexBuffer, sizeof(d3dvertex_t) );
	m_pOrigDDevice->SetVertexShader ( D3D_VERTEX );
	if (m_pOrigDDevice->DrawPrimitive ( D3DPT_TRIANGLEFAN, 0, 2 ) != D3D_OK)
	{
		assert ( 0 );
	}
}

// Returns the width of a character
int		D3D_GetCharWidth ( ID3DXFont *pFont, char cChar, float fScale )
{
	// Get our font entry
	unsigned int n;
	
	for (n=0; n<m_uiNumFonts; n++)
	{
		if ( m_pFonts[n]->pFont == pFont )
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint ( hGTAWindow, &ps );
			SIZE charSize;
			SelectObject ( hDC, m_pFonts[n]->hFont );
			GetTextExtentPoint32 ( hDC, &cChar, 1, &charSize );
			EndPaint ( hGTAWindow, &ps );
			return charSize.cx;
		}
	}
	return 0;
}

/*===============================================
	Direct3D 8 Device Functions
===============================================*/

HRESULT		__stdcall D3D8_QueryInterface ( d3dproxy8device_t *__this, REFIID refiid, void **ppvObj )
{
	return m_pOrigDDevice->QueryInterface ( refiid, ppvObj );
}

ULONG		__stdcall D3D8_AddRef ( d3dproxy8device_t *_this )
{
	m_uiRefCountDevice++;
	return m_pOrigDDevice->AddRef ( );
}

ULONG		__stdcall D3D8_Release ( d3dproxy8device_t *_this )
{
	m_uiRefCountDevice--;
	
	if ( m_uiRefCountDevice == 0 )
	{
		free(m_pDDevice->lpVtbl);
		free(m_pDDevice);
	}
	return m_pOrigDDevice->Release ( );
}

HRESULT		__stdcall D3D8_TestCooperativeLevel ( d3dproxy8device_t *_this )
{
	return m_pOrigDDevice->TestCooperativeLevel ( );
}

UINT		__stdcall D3D8_GetAvailableTextureMem ( d3dproxy8device_t *_this )
{
	return m_pOrigDDevice->GetAvailableTextureMem ( );
}

HRESULT		__stdcall D3D8_ResourceManagerDiscardBytes ( d3dproxy8device_t *_this, DWORD Bytes )
{
	return m_pOrigDDevice->ResourceManagerDiscardBytes ( Bytes );
}

HRESULT		__stdcall D3D8_EvictManagedResources ( )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_GetDirect3D ( d3dproxy8device_t *_this, IDirect3D8** ppD3D )
{
	*ppD3D = (IDirect3D8*)m_pDevice;
	return D3D_OK;
}

HRESULT		__stdcall D3D8_GetDeviceCaps ( d3dproxy8device_t *_this, D3DCAPS8* pCaps )
{
	return m_pDevice->lpVtbl->GetDeviceCaps ( m_pDevice, m_uiDeviceAdapter, m_DeviceType, pCaps );
}

HRESULT		__stdcall D3D8_GetDisplayMode ( d3dproxy8device_t *_this, D3DDISPLAYMODE* pMode )
{
	return m_pOrigDDevice->GetDisplayMode ( pMode );
}

HRESULT		__stdcall D3D8_GetCreationParameters ( d3dproxy8device_t *_this, D3DDEVICE_CREATION_PARAMETERS *pParameters )
{
	return m_pOrigDDevice->GetCreationParameters ( pParameters );
}

HRESULT		__stdcall D3D8_SetCursorProperties ( d3dproxy8device_t *_this, UINT XHotSpot,UINT YHotSpot,IDirect3DSurface8* pCursorBitmap )
{
	return m_pOrigDDevice->SetCursorProperties ( XHotSpot, YHotSpot, pCursorBitmap );
}

void		__stdcall D3D8_SetCursorPosition ( d3dproxy8device_t *_this, int X, int Y, DWORD Flags )
{
	m_pOrigDDevice->SetCursorPosition ( X, Y, Flags );
}

BOOL		__stdcall D3D8_ShowCursor ( d3dproxy8device_t *_this, BOOL bShow )
{
	if ( m_bConsoleIsTyping )
		return D3D_OK;

	return m_pOrigDDevice->ShowCursor ( bShow );
}

HRESULT		__stdcall D3D8_CreateAdditionalSwapChain ( d3dproxy8device_t *_this, D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain8** pSwapChain )
{
	return m_pOrigDDevice->CreateAdditionalSwapChain ( pPresentationParameters, pSwapChain );
}

HRESULT		__stdcall D3D8_GetSwapChain ( d3dproxy8device_t *_this, UINT uiSwapChain,IDirect3DSwapChain8** pSwapChain )
{
	return FALSE;
}

UINT		__stdcall D3D8_GetNumberOfSwapChains ( d3dproxy8device_t *_this )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_Reset ( d3dproxy8device_t *_this, D3DPRESENT_PARAMETERS* pPresentationParameters )
{
	HRESULT hResult;

	// Invalidate all resources, they no longer are valid
	D3D_RendInvalid ( m_pOrigDDevice );
	D3D_OnInvalidate ( m_pOrigDDevice );

	hResult = m_pOrigDDevice->Reset ( pPresentationParameters );

	// Reset all components
	D3D_RendRestore ( m_pOrigDDevice );
	D3D_OnRestore ( m_pOrigDDevice );

	return hResult;
}

HRESULT		__stdcall D3D8_Present ( d3dproxy8device_t *_this, CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion )
{
	// Do our own business
	D3D_OnPresent ( m_pOrigDDevice );
	return m_pOrigDDevice->Present ( pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion );
}

HRESULT		__stdcall D3D8_GetBackBuffer ( d3dproxy8device_t *_this, UINT uiBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface8** ppBackBuffer )
{
	return m_pOrigDDevice->GetBackBuffer ( uiBackBuffer, Type, ppBackBuffer );
}

HRESULT		__stdcall D3D8_GetRasterStatus ( d3dproxy8device_t *_this, D3DRASTER_STATUS* pRasterStatus )
{
	return m_pOrigDDevice->GetRasterStatus ( pRasterStatus );
}

HRESULT		__stdcall D3D8_SetDialogBoxMode ( d3dproxy8device_t *_this, BOOL bEnableDialogs )
{
	return FALSE;
}

void		__stdcall D3D8_SetGammaRamp ( d3dproxy8device_t *_this, DWORD Flags,CONST D3DGAMMARAMP* pRamp )
{
	m_pOrigDDevice->SetGammaRamp ( Flags, pRamp );
}

void		__stdcall D3D8_GetGammaRamp ( d3dproxy8device_t *_this, D3DGAMMARAMP* pRamp )
{
	m_pOrigDDevice->GetGammaRamp ( pRamp );
}

HRESULT		__stdcall D3D8_CreateTexture ( d3dproxy8device_t *_this, UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture8** ppTexture )
{
	return m_pOrigDDevice->CreateTexture ( Width, Height, Levels, Usage, Format, Pool, ppTexture );
}

HRESULT		__stdcall D3D8_CreateVolumeTexture ( d3dproxy8device_t *_this, UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture8** ppVolumeTexture )
{
	return m_pOrigDDevice->CreateVolumeTexture ( Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture );
}

HRESULT		__stdcall D3D8_CreateCubeTexture ( d3dproxy8device_t *_this, UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture8** ppCubeTexture )
{
	return m_pOrigDDevice->CreateCubeTexture ( EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture );
}

HRESULT		__stdcall D3D8_CreateVertexBuffer ( d3dproxy8device_t *_this, UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer8** ppVertexBuffer )
{
	return m_pOrigDDevice->CreateVertexBuffer ( Length, Usage, FVF, Pool, ppVertexBuffer );
}

HRESULT		__stdcall D3D8_CreateIndexBuffer ( d3dproxy8device_t *_this, UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer8** ppIndexBuffer )
{
	return m_pOrigDDevice->CreateIndexBuffer ( Length, Usage, Format, Pool, ppIndexBuffer );
}

HRESULT		__stdcall D3D8_CreateRenderTarget ( d3dproxy8device_t *_this, UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,BOOL Lockable,IDirect3DSurface8** ppSurface )
{
	return m_pOrigDDevice->CreateRenderTarget ( Width, Height, Format, MultiSample, Lockable, ppSurface );
}

HRESULT		__stdcall D3D8_CreateDepthStencilSurface ( d3dproxy8device_t *_this, UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,IDirect3DSurface8** ppSurface )
{
	return m_pOrigDDevice->CreateDepthStencilSurface ( Width, Height, Format, MultiSample, ppSurface );
}

HRESULT		__stdcall D3D8_CreateImageSurface ( d3dproxy8device_t *_this, UINT Width,UINT Height,D3DFORMAT Format,IDirect3DSurface8** ppSurface )
{
	return m_pOrigDDevice->CreateImageSurface ( Width, Height, Format, ppSurface );
}

HRESULT		__stdcall D3D8_CopyRects ( d3dproxy8device_t *_this, IDirect3DSurface8* pSourceSurface,CONST RECT* pSourceRectsArray,UINT cRects,IDirect3DSurface8* pDestinationSurface,CONST POINT* pDestPointsArray )
{
	return m_pOrigDDevice->CopyRects ( pSourceSurface, pSourceRectsArray, cRects, pDestinationSurface, pDestPointsArray );
}

HRESULT		__stdcall D3D8_UpdateSurface ( d3dproxy8device_t *_this, IDirect3DSurface8* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface8* pDestinationSurface,CONST POINT* pDestPoint )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_UpdateTexture ( d3dproxy8device_t *_this, IDirect3DBaseTexture8* pSourceTexture,IDirect3DBaseTexture8* pDestinationTexture )
{
	return m_pOrigDDevice->UpdateTexture ( pSourceTexture, pDestinationTexture );
}

HRESULT		__stdcall D3D8_GetFrontBuffer ( d3dproxy8device_t *_this, IDirect3DSurface8* pDestSurface )
{
	return m_pOrigDDevice->GetFrontBuffer ( pDestSurface );
}

HRESULT		__stdcall D3D8_GetRenderTargetData ( d3dproxy8device_t *_this, IDirect3DSurface8* pRenderTarget,IDirect3DSurface8* pDestSurface )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_GetFrontBufferData ( d3dproxy8device_t *_this, UINT iSwapChain,IDirect3DSurface8* pDestSurface )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_StretchRect ( d3dproxy8device_t *_this, IDirect3DSurface8* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface8* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_SetRenderTarget ( d3dproxy8device_t *_this, IDirect3DSurface8* pRenderTarget,IDirect3DSurface8* pNewZStencil )
{
	return m_pOrigDDevice->SetRenderTarget ( pRenderTarget, pNewZStencil );
}

HRESULT		__stdcall D3D8_GetRenderTarget ( d3dproxy8device_t *_this, IDirect3DSurface8** ppRenderTarget )
{
	return m_pOrigDDevice->GetRenderTarget ( ppRenderTarget );
}

HRESULT		__stdcall D3D8_SetDepthStencilSurface ( d3dproxy8device_t *_this, IDirect3DSurface8* pNewZStencil )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_GetDepthStencilSurface ( d3dproxy8device_t *_this, IDirect3DSurface8** ppZStencilSurface )
{
	return m_pOrigDDevice->GetDepthStencilSurface ( ppZStencilSurface );
}

HRESULT		__stdcall D3D8_BeginScene ( d3dproxy8device_t *_this )
{
	return m_pOrigDDevice->BeginScene ( );
}

HRESULT		__stdcall D3D8_EndScene ( d3dproxy8device_t *_this )
{
	return m_pOrigDDevice->EndScene ( );
}

HRESULT		__stdcall D3D8_Clear ( d3dproxy8device_t *_this, DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil )
{
	return m_pOrigDDevice->Clear ( Count, pRects, Flags, Color, Z, Stencil );
}

HRESULT		__stdcall D3D8_SetTransform ( d3dproxy8device_t *_this, D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix )
{
	return m_pOrigDDevice->SetTransform ( State, pMatrix );
}

HRESULT		__stdcall D3D8_GetTransform ( d3dproxy8device_t *_this, D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix )
{
	return m_pOrigDDevice->GetTransform ( State, pMatrix );
}

HRESULT		__stdcall D3D8_MultiplyTransform ( d3dproxy8device_t *_this, D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix )
{
	return m_pOrigDDevice->MultiplyTransform ( State, pMatrix );
}

HRESULT		__stdcall D3D8_SetViewport ( d3dproxy8device_t *_this, CONST D3DVIEWPORT8* pViewport )
{
	// Refresh backbuffer status
	m_iScreenWidth = pViewport->Width;
	m_iScreenHeight = pViewport->Height;
	return m_pOrigDDevice->SetViewport ( pViewport );
}

HRESULT		__stdcall D3D8_GetViewport ( d3dproxy8device_t *_this, D3DVIEWPORT8* pViewport )
{
	return m_pOrigDDevice->GetViewport ( pViewport );
}

HRESULT		__stdcall D3D8_SetMaterial ( d3dproxy8device_t *_this, CONST D3DMATERIAL8* pMaterial )
{
	return m_pOrigDDevice->SetMaterial ( pMaterial );
}

HRESULT		__stdcall D3D8_GetMaterial ( d3dproxy8device_t *_this, D3DMATERIAL8* pMaterial )
{
	return m_pOrigDDevice->GetMaterial ( pMaterial );
}

HRESULT		__stdcall D3D8_SetLight ( d3dproxy8device_t *_this, DWORD Index,CONST D3DLIGHT8* pLight )
{
	return m_pOrigDDevice->SetLight ( Index, pLight );
}

HRESULT		__stdcall D3D8_GetLight ( d3dproxy8device_t *_this, DWORD Index,D3DLIGHT8* pLight )
{
	return m_pOrigDDevice->GetLight ( Index, pLight );
}

HRESULT		__stdcall D3D8_LightEnable ( d3dproxy8device_t *_this, DWORD Index,BOOL Enable )
{
	return m_pOrigDDevice->LightEnable ( Index, Enable );
}

HRESULT		__stdcall D3D8_GetLightEnable ( d3dproxy8device_t *_this, DWORD Index,BOOL* pEnable )
{
	return m_pOrigDDevice->GetLightEnable ( Index, pEnable );
}

HRESULT		__stdcall D3D8_SetClipPlane ( d3dproxy8device_t *_this, DWORD Index,CONST float* pPlane )
{
	return m_pOrigDDevice->SetClipPlane ( Index, pPlane );
}

HRESULT		__stdcall D3D8_GetClipPlane ( d3dproxy8device_t *_this, DWORD Index,float* pPlane )
{
	return m_pOrigDDevice->GetClipPlane ( Index, pPlane );
}

HRESULT		__stdcall D3D8_SetRenderState ( d3dproxy8device_t *_this, D3DRENDERSTATETYPE State,DWORD Value )
{
	return m_pOrigDDevice->SetRenderState ( State, Value );
}

HRESULT		__stdcall D3D8_GetRenderState ( d3dproxy8device_t *_this, D3DRENDERSTATETYPE State,DWORD* pValue )
{
	return m_pOrigDDevice->GetRenderState ( State, pValue );
}

HRESULT		__stdcall D3D8_CreateStateBlock ( d3dproxy8device_t *_this, D3DSTATEBLOCKTYPE Type,DWORD *pToken )
{
	return m_pOrigDDevice->CreateStateBlock ( Type, pToken );
}

HRESULT		__stdcall D3D8_BeginStateBlock ( d3dproxy8device_t *_this )
{
	return m_pOrigDDevice->BeginStateBlock ( );
}

HRESULT		__stdcall D3D8_EndStateBlock ( d3dproxy8device_t *_this, DWORD *pToken )
{
	return m_pOrigDDevice->EndStateBlock ( pToken );
}

HRESULT		__stdcall D3D8_ApplyStateBlock ( d3dproxy8device_t *_this, DWORD Token )
{
	return m_pOrigDDevice->ApplyStateBlock ( Token );
}

HRESULT		__stdcall D3D8_CaptureStateBlock ( d3dproxy8device_t *_this, DWORD Token )
{
	return m_pOrigDDevice->CaptureStateBlock ( Token );
}

HRESULT		__stdcall D3D8_DeleteStateBlock ( d3dproxy8device_t *_this, DWORD Token )
{
	return m_pOrigDDevice->DeleteStateBlock ( Token );
}

HRESULT		__stdcall D3D8_SetClipStatus ( d3dproxy8device_t *_this, CONST D3DCLIPSTATUS8* pClipStatus )
{
	return m_pOrigDDevice->SetClipStatus ( pClipStatus );
}

HRESULT		__stdcall D3D8_GetClipStatus ( d3dproxy8device_t *_this, D3DCLIPSTATUS8* pClipStatus )
{
	return m_pOrigDDevice->GetClipStatus ( pClipStatus );
}

HRESULT		__stdcall D3D8_GetTexture ( d3dproxy8device_t *_this, DWORD Stage,IDirect3DBaseTexture8** ppTexture )
{
	return m_pOrigDDevice->GetTexture ( Stage, ppTexture );
}

HRESULT		__stdcall D3D8_SetTexture ( d3dproxy8device_t *_this, DWORD Stage,IDirect3DBaseTexture8* pTexture )
{
	return m_pOrigDDevice->SetTexture ( Stage, pTexture );
}

HRESULT		__stdcall D3D8_GetTextureStageState ( d3dproxy8device_t *_this, DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue )
{
	return m_pOrigDDevice->GetTextureStageState ( Stage, Type, pValue );
}

HRESULT		__stdcall D3D8_SetTextureStageState ( d3dproxy8device_t *_this, DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value )
{
	return m_pOrigDDevice->SetTextureStageState ( Stage, Type, Value );
}

/*HRESULT		D3D8_GetSamplerState ( d3dproxy8device_t *_this, DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue )
{
	return FALSE;
}

HRESULT		D3D8_SetSamplerState ( d3dproxy8device_t *_this, DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value )
{
	return FALSE;
}*/

HRESULT		__stdcall D3D8_ValidateDevice ( d3dproxy8device_t *_this, DWORD* pNumPasses )
{
	return m_pOrigDDevice->ValidateDevice ( pNumPasses );
}

HRESULT		__stdcall D3D8_GetInfo ( d3dproxy8device_t *_this, DWORD DevInfoID, void *pDevInfoStruct, DWORD DevInfoStructSize )
{
	return m_pOrigDDevice->GetInfo ( DevInfoID, pDevInfoStruct, DevInfoStructSize );
}

HRESULT		__stdcall D3D8_SetPaletteEntries ( d3dproxy8device_t *_this, UINT PaletteNumber,CONST PALETTEENTRY* pEntries )
{
	return m_pOrigDDevice->SetPaletteEntries ( PaletteNumber, pEntries );
}

HRESULT		__stdcall D3D8_GetPaletteEntries ( d3dproxy8device_t *_this, UINT PaletteNumber,PALETTEENTRY* pEntries )
{
	return m_pOrigDDevice->GetPaletteEntries ( PaletteNumber, pEntries );
}

HRESULT		__stdcall D3D8_SetCurrentTexturePalette ( d3dproxy8device_t *_this, UINT PaletteNumber )
{
	return m_pOrigDDevice->SetCurrentTexturePalette ( PaletteNumber );
}

HRESULT		__stdcall D3D8_GetCurrentTexturePalette ( d3dproxy8device_t *_this, UINT *pPaletteNumber )
{
	return m_pOrigDDevice->GetCurrentTexturePalette ( pPaletteNumber );
}

HRESULT		__stdcall D3D8_SetScissorRect ( d3dproxy8device_t *_this, CONST RECT* pRect )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_GetScissorRect ( d3dproxy8device_t *_this, RECT* pRect )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_SetSoftwareVertexProcessing ( d3dproxy8device_t *_this, BOOL bSoftware )
{
	return FALSE;
}

BOOL		__stdcall D3D8_GetSoftwareVertexProcessing ( d3dproxy8device_t *_this )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_SetNPatchMode ( d3dproxy8device_t *_this, float nSegments )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_GetNPatchMode ( d3dproxy8device_t *_this )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_DrawPrimitive ( d3dproxy8device_t *_this, D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount )
{
	return m_pOrigDDevice->DrawPrimitive ( PrimitiveType, StartVertex, PrimitiveCount );
}

HRESULT		__stdcall D3D8_DrawIndexedPrimitive ( d3dproxy8device_t *_this, D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount )
{
	return m_pOrigDDevice->DrawIndexedPrimitive ( PrimitiveType, MinVertexIndex, NumVertices, startIndex, primCount );
}

HRESULT		__stdcall D3D8_DrawPrimitiveUP ( d3dproxy8device_t *_this, D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride )
{
	return m_pOrigDDevice->DrawPrimitiveUP ( PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride );
}

HRESULT		__stdcall D3D8_DrawIndexedPrimitiveUP ( d3dproxy8device_t *_this, D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride )
{
	return m_pOrigDDevice->DrawIndexedPrimitiveUP ( PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride );
}

HRESULT		__stdcall D3D8_ProcessVertices ( d3dproxy8device_t *_this, UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer8* pDestBuffer,DWORD Flags )
{
	return m_pOrigDDevice->ProcessVertices ( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, Flags );
}

HRESULT		__stdcall D3D8_SetFVF ( d3dproxy8device_t *_this, DWORD FVF )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_GetFVF ( d3dproxy8device_t *_this, DWORD* pFVF )
{
	return FALSE;
}

HRESULT		__stdcall D3D8_CreateVertexShader ( d3dproxy8device_t *_this, CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage )
{
	return m_pOrigDDevice->CreateVertexShader ( pDeclaration, pFunction, pHandle, Usage );
}

HRESULT		__stdcall D3D8_SetVertexShader ( d3dproxy8device_t *_this, DWORD Handle )
{
	return m_pOrigDDevice->SetVertexShader ( Handle );
}

HRESULT		__stdcall D3D8_GetVertexShader ( d3dproxy8device_t *_this, DWORD* pHandle )
{
	return m_pOrigDDevice->GetVertexShader ( pHandle );
}

HRESULT		__stdcall D3D8_DeleteVertexShader ( d3dproxy8device_t *_this, DWORD Handle )
{
	return m_pOrigDDevice->DeleteVertexShader ( Handle );
}

HRESULT		__stdcall D3D8_SetVertexShaderConstant ( d3dproxy8device_t *_this, DWORD Register,CONST void* pConstantData,DWORD ConstantCount )
{
	return m_pOrigDDevice->SetVertexShaderConstant ( Register, pConstantData, ConstantCount );
}

HRESULT		__stdcall D3D8_GetVertexShaderConstant ( d3dproxy8device_t *_this, UINT Register,void* pConstantData,UINT ConstantCount )
{
	return m_pOrigDDevice->GetVertexShaderConstant ( Register, pConstantData, ConstantCount );
}

HRESULT		__stdcall D3D8_GetVertexShaderDeclaration ( d3dproxy8device_t *_this, DWORD Handle,void* pData,DWORD* pSizeOfData )
{
	return m_pOrigDDevice->GetVertexShaderDeclaration ( Handle, pData, pSizeOfData );
}

HRESULT		__stdcall D3D8_GetVertexShaderFunction ( d3dproxy8device_t *_this, DWORD Handle,void* pData,DWORD* pSizeOfData )
{
	return m_pOrigDDevice->GetVertexShaderFunction ( Handle, pData, pSizeOfData );
}

HRESULT		__stdcall D3D8_SetStreamSource ( d3dproxy8device_t *_this, UINT StreamNumber,IDirect3DVertexBuffer8* pStreamData,UINT Stride )
{
	return m_pOrigDDevice->SetStreamSource ( StreamNumber, pStreamData, Stride );
}

HRESULT		__stdcall D3D8_GetStreamSource ( d3dproxy8device_t *_this, UINT StreamNumber,IDirect3DVertexBuffer8** ppStreamData,UINT* pStride )
{
	return m_pOrigDDevice->GetStreamSource ( StreamNumber, ppStreamData, pStride );
}

HRESULT		__stdcall D3D8_SetIndices ( d3dproxy8device_t *_this, IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex )
{
	return m_pOrigDDevice->SetIndices ( pIndexData, BaseVertexIndex );
}

HRESULT		__stdcall D3D8_GetIndices ( d3dproxy8device_t *_this, IDirect3DIndexBuffer8** ppIndexData, UINT *pBaseVertexIndex )
{
	return m_pOrigDDevice->GetIndices ( ppIndexData, pBaseVertexIndex );
}

HRESULT		__stdcall D3D8_CreatePixelShader ( d3dproxy8device_t *_this, CONST DWORD* pFunction, DWORD* pHandle )
{
	return m_pOrigDDevice->CreatePixelShader ( pFunction, pHandle );
}

HRESULT		__stdcall D3D8_SetPixelShader ( d3dproxy8device_t *_this, DWORD Handle )
{
	return m_pOrigDDevice->SetPixelShader ( Handle );
}

HRESULT		__stdcall D3D8_GetPixelShader ( d3dproxy8device_t *_this, DWORD* pHandle )
{
	return m_pOrigDDevice->GetPixelShader ( pHandle );
}

HRESULT		__stdcall D3D8_DeletePixelShader ( d3dproxy8device_t *_this, DWORD Handle )
{
	return m_pOrigDDevice->DeletePixelShader ( Handle );
}

HRESULT		__stdcall D3D8_SetPixelShaderConstant ( d3dproxy8device_t *_this, DWORD Register, CONST void* pConstantData, DWORD ConstantCount )
{
	return m_pOrigDDevice->SetPixelShaderConstant ( Register, pConstantData, ConstantCount );
}

HRESULT		__stdcall D3D8_GetPixelShaderConstant ( d3dproxy8device_t *_this, DWORD Register, void* pConstantData, DWORD ConstantCount )
{
	return m_pOrigDDevice->GetPixelShaderConstant ( Register, pConstantData, ConstantCount );
}

HRESULT		__stdcall D3D8_GetPixelShaderFunction ( d3dproxy8device_t *_this, DWORD Handle, void* pData, DWORD* pSizeOfData )
{
	return m_pOrigDDevice->GetPixelShaderFunction ( Handle, pData, pSizeOfData );
}

HRESULT		__stdcall D3D8_DrawRectPatch ( d3dproxy8device_t *_this, UINT Handle, const float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo )
{
	return m_pOrigDDevice->DrawRectPatch ( Handle, pNumSegs, pRectPatchInfo );
}

HRESULT		__stdcall D3D8_DrawTriPatch ( d3dproxy8device_t *_this, UINT Handle, const float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo )
{
	return m_pOrigDDevice->DrawTriPatch ( Handle, pNumSegs, pTriPatchInfo );
}

HRESULT		__stdcall D3D8_DeletePatch ( d3dproxy8device_t *_this, UINT Handle )
{
	return m_pOrigDDevice->DeletePatch ( Handle );
}

/*===============================================
	Direct3D 8 Hooking functions
===============================================*/

// Query interface
HRESULT		__stdcall Direct3D8_QueryInterface ( d3dproxy8_t *_this, REFIID refiid, void **ppvObj )
{
	return m_pOrigDevice->QueryInterface ( refiid, ppvObj );
}

// Add reference
ULONG		__stdcall Direct3D8_AddRef ( d3dproxy8_t *_this )
{
	m_uiRefCount++;
	return m_pOrigDevice->AddRef ( );
}

// Release
ULONG		__stdcall Direct3D8_Release ( d3dproxy8_t *_this )
{
	m_uiRefCount--;

	if (m_uiRefCount==0)
	{
		free (m_pDevice->lpVtbl);
		free (m_pDevice);
	}
	return m_pOrigDevice->Release ( );
}

// Register software device
HRESULT		__stdcall Direct3D8_RegisterSoftwareDevice ( d3dproxy8_t *_this, void *pInitFunction )
{
	return m_pOrigDevice->RegisterSoftwareDevice ( pInitFunction );
}

// Get adapter count
UINT		__stdcall Direct3D8_GetAdapterCount ( d3dproxy8_t *_this )
{
	return m_pOrigDevice->GetAdapterCount ( );
}

// Get adapter ID
HRESULT		__stdcall Direct3D8_GetAdapterIdentifier ( d3dproxy8_t *_this, UINT adapter, DWORD flags, D3DADAPTER_IDENTIFIER8* pIdentifier )
{
	return m_pOrigDevice->GetAdapterIdentifier ( adapter, flags, pIdentifier );
}

// Mode count
UINT		__stdcall Direct3D8_GetAdapterModeCount ( d3dproxy8_t *_this, UINT adapter )
{
	return m_pOrigDevice->GetAdapterModeCount ( adapter );
}

// Enumerate all of the modes
HRESULT		__stdcall Direct3D8_EnumAdapterModes ( d3dproxy8_t *_this, UINT adapter, UINT mode, D3DDISPLAYMODE *pMode )
{
	return m_pOrigDevice->EnumAdapterModes ( adapter, mode, pMode );
}

// Adapter display mode
HRESULT		__stdcall Direct3D8_GetAdapterDisplayMode ( d3dproxy8_t *_this, UINT adapter, D3DDISPLAYMODE *pMode )
{
	return m_pOrigDevice->GetAdapterDisplayMode ( adapter, pMode );
}

// Device type
HRESULT		__stdcall Direct3D8_CheckDeviceType ( d3dproxy8_t *_this, UINT adapter, D3DDEVTYPE devType, D3DFORMAT adapterFormat, D3DFORMAT backBufferFormat, BOOL bWindowed )
{
	return m_pOrigDevice->CheckDeviceType ( adapter, devType, adapterFormat, backBufferFormat, bWindowed );
}

// Device format
HRESULT		__stdcall Direct3D8_CheckDeviceFormat ( d3dproxy8_t *_this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat )
{
	return m_pOrigDevice->CheckDeviceFormat ( Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat );
}

// Multi sample type
HRESULT		__stdcall Direct3D8_CheckDeviceMultiSampleType ( d3dproxy8_t *_this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType /*, DWORD* pQualityLevels*/ )
{
	return m_pOrigDevice->CheckDeviceMultiSampleType ( Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType );
}

// Depth stencil match
HRESULT		__stdcall Direct3D8_CheckDepthStencilMatch ( d3dproxy8_t *_this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat )
{
	return m_pOrigDevice->CheckDepthStencilMatch ( Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat );
}

// Format conversion
HRESULT		__stdcall Direct3D8_CheckDeviceFormatConversion ( d3dproxy8_t *_this, UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat )
{
	return FALSE;
}

// Device caps
HRESULT		__stdcall Direct3D8_GetDeviceCaps ( d3dproxy8_t *_this, UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps )
{
	return m_pOrigDevice->GetDeviceCaps ( Adapter, DeviceType, pCaps );
}

// Get Adapter monitor
HMONITOR	__stdcall Direct3D8_GetAdapterMonitor ( d3dproxy8_t *_this, UINT Adapter )
{
	return m_pOrigDevice->GetAdapterMonitor ( Adapter );
}

// Create a device
HRESULT		__stdcall Direct3D8_CreateDevice ( d3dproxy8_t *_this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface )
{
	HRESULT ret;

	hGTAWindow=hFocusWindow;
	Window_ApplyHook ();

	OutputDebugString ( "Created D3D8 device!" );
	m_uiDeviceAdapter = Adapter;
	m_DeviceType = DeviceType;
	pPresentationParameters->EnableAutoDepthStencil = true;
	pPresentationParameters->Windowed = false;

	m_iScreenWidth = pPresentationParameters->BackBufferWidth;
	m_iScreenHeight = pPresentationParameters->BackBufferHeight;

#ifdef _DEBUG
	SetWindowText(hGTAWindow, "KillFrenzy! (Debug Build)");
#else
	SetWindowText(hGTAWindow, "KillFrenzy!");
#endif

	ChangeDisplaySettings(NULL, 0);
    ShowWindow( hFocusWindow, SW_SHOWNORMAL );

	// Create the real device
	ret = m_pOrigDevice->CreateDevice ( Adapter, DeviceType, hGTAWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface );

	if (ret == D3D_OK)
	{
		d3dproxy8device_t *env = (d3dproxy8device_t*)malloc(sizeof(d3dproxy8device_t));
		env->lpVtbl = (struct d3dproxy8devicevtbl_s*)malloc(sizeof(struct d3dproxy8devicevtbl_s));
		env->lpVtbl->QueryInterface = D3D8_QueryInterface;
		env->lpVtbl->AddRef = D3D8_AddRef;
		env->lpVtbl->Release = D3D8_Release;
		env->lpVtbl->TestCooperativeLevel = D3D8_TestCooperativeLevel;
		env->lpVtbl->GetAvailableTextureMem = D3D8_GetAvailableTextureMem;
		env->lpVtbl->ResourceManagerDiscardBytes = D3D8_ResourceManagerDiscardBytes;
		env->lpVtbl->GetDirect3D = D3D8_GetDirect3D;
		env->lpVtbl->GetDeviceCaps = D3D8_GetDeviceCaps;
		env->lpVtbl->GetDisplayMode = D3D8_GetDisplayMode;
		env->lpVtbl->GetCreationParameters = D3D8_GetCreationParameters;
		env->lpVtbl->SetCursorProperties = D3D8_SetCursorProperties;
		env->lpVtbl->SetCursorPosition = D3D8_SetCursorPosition;
		env->lpVtbl->ShowCursor = D3D8_ShowCursor;
		env->lpVtbl->CreateAdditionalSwapChain = D3D8_CreateAdditionalSwapChain;
		env->lpVtbl->Reset = D3D8_Reset;
		env->lpVtbl->Present = D3D8_Present;
		env->lpVtbl->GetBackBuffer = D3D8_GetBackBuffer;
		env->lpVtbl->GetRasterStatus = D3D8_GetRasterStatus;
		env->lpVtbl->SetGammaRamp = D3D8_SetGammaRamp;
		env->lpVtbl->GetGammaRamp = D3D8_GetGammaRamp;
		env->lpVtbl->CreateTexture = D3D8_CreateTexture;
		env->lpVtbl->CreateVolumeTexture = D3D8_CreateVolumeTexture;
		env->lpVtbl->CreateCubeTexture = D3D8_CreateCubeTexture;
		env->lpVtbl->CreateVertexBuffer = D3D8_CreateVertexBuffer;
		env->lpVtbl->CreateIndexBuffer = D3D8_CreateIndexBuffer;
		env->lpVtbl->CreateRenderTarget = D3D8_CreateRenderTarget;
		env->lpVtbl->CreateDepthStencilSurface = D3D8_CreateDepthStencilSurface;
		env->lpVtbl->CreateImageSurface = D3D8_CreateImageSurface;
		env->lpVtbl->CopyRects = D3D8_CopyRects;
		env->lpVtbl->UpdateTexture = D3D8_UpdateTexture;
		env->lpVtbl->GetFrontBuffer = D3D8_GetFrontBuffer;
		env->lpVtbl->SetRenderTarget = D3D8_SetRenderTarget;
		env->lpVtbl->GetRenderTarget = D3D8_GetRenderTarget;
		env->lpVtbl->GetDepthStencilSurface = D3D8_GetDepthStencilSurface;
		env->lpVtbl->BeginScene = D3D8_BeginScene;
		env->lpVtbl->EndScene = D3D8_EndScene;
		env->lpVtbl->Clear = D3D8_Clear;
		env->lpVtbl->SetTransform = D3D8_SetTransform;
		env->lpVtbl->GetTransform = D3D8_GetTransform;
		env->lpVtbl->MultiplyTransform = D3D8_MultiplyTransform;
		env->lpVtbl->SetViewport = D3D8_SetViewport;
		env->lpVtbl->GetViewport = D3D8_GetViewport;
		env->lpVtbl->SetMaterial = D3D8_SetMaterial;
		env->lpVtbl->GetMaterial = D3D8_GetMaterial;
		env->lpVtbl->SetLight = D3D8_SetLight;
		env->lpVtbl->GetLight = D3D8_GetLight;
		env->lpVtbl->LightEnable = D3D8_LightEnable;
		env->lpVtbl->GetLightEnable = D3D8_GetLightEnable;
		env->lpVtbl->SetClipPlane = D3D8_SetClipPlane;
		env->lpVtbl->GetClipPlane = D3D8_GetClipPlane;
		env->lpVtbl->SetRenderState = D3D8_SetRenderState;
		env->lpVtbl->GetRenderState = D3D8_GetRenderState;
		env->lpVtbl->BeginStateBlock = D3D8_BeginStateBlock;
		env->lpVtbl->EndStateBlock = D3D8_EndStateBlock;
		env->lpVtbl->ApplyStateBlock = D3D8_ApplyStateBlock;
		env->lpVtbl->CaptureStateBlock = D3D8_CaptureStateBlock;
		env->lpVtbl->DeleteStateBlock = D3D8_DeleteStateBlock;
		env->lpVtbl->CreateStateBlock = D3D8_CreateStateBlock;
		env->lpVtbl->SetClipStatus = D3D8_SetClipStatus;
		env->lpVtbl->GetClipStatus = D3D8_GetClipStatus;
		env->lpVtbl->GetTexture = D3D8_GetTexture;
		env->lpVtbl->SetTexture = D3D8_SetTexture;
		env->lpVtbl->GetTextureStageState = D3D8_GetTextureStageState;
		env->lpVtbl->SetTextureStageState = D3D8_SetTextureStageState;
		env->lpVtbl->ValidateDevice = D3D8_ValidateDevice;
		env->lpVtbl->GetInfo = D3D8_GetInfo;
		env->lpVtbl->SetPaletteEntries = D3D8_SetPaletteEntries;
		env->lpVtbl->GetPaletteEntries = D3D8_GetPaletteEntries;
		env->lpVtbl->SetCurrentTexturePalette = D3D8_SetCurrentTexturePalette;
		env->lpVtbl->GetCurrentTexturePalette = D3D8_GetCurrentTexturePalette;
		//env->lpVtbl->SetScissorRect = D3D8_SetScissorRect;
		env->lpVtbl->DrawPrimitive = D3D8_DrawPrimitive;
		env->lpVtbl->DrawIndexedPrimitive = D3D8_DrawIndexedPrimitive;
		env->lpVtbl->DrawPrimitiveUP = D3D8_DrawPrimitiveUP;
		env->lpVtbl->DrawIndexedPrimitiveUP = D3D8_DrawIndexedPrimitiveUP;
		env->lpVtbl->ProcessVertices = D3D8_ProcessVertices;
		env->lpVtbl->CreateVertexShader = D3D8_CreateVertexShader;
		env->lpVtbl->SetVertexShader = D3D8_SetVertexShader;
		env->lpVtbl->GetVertexShader = D3D8_GetVertexShader;
		env->lpVtbl->SetVertexShaderConstant = D3D8_SetVertexShaderConstant;
		env->lpVtbl->GetVertexShaderConstant = D3D8_GetVertexShaderConstant;
		env->lpVtbl->GetVertexShaderDeclaration = D3D8_GetVertexShaderDeclaration;
		env->lpVtbl->GetVertexShaderFunction = D3D8_GetVertexShaderFunction;
		env->lpVtbl->SetStreamSource = D3D8_SetStreamSource;
		env->lpVtbl->GetStreamSource = D3D8_GetStreamSource;
		env->lpVtbl->SetIndices = D3D8_SetIndices;
		env->lpVtbl->GetIndices = D3D8_GetIndices;
		env->lpVtbl->CreatePixelShader = D3D8_CreatePixelShader;
		env->lpVtbl->SetPixelShader = D3D8_SetPixelShader;
		env->lpVtbl->GetPixelShader = D3D8_GetPixelShader;
		env->lpVtbl->DeletePixelShader = D3D8_DeletePixelShader;
		env->lpVtbl->SetPixelShaderConstant = D3D8_SetPixelShaderConstant;
		env->lpVtbl->GetPixelShaderConstant = D3D8_GetPixelShaderConstant;
		env->lpVtbl->GetPixelShaderFunction = D3D8_GetPixelShaderFunction;
		env->lpVtbl->DrawRectPatch = D3D8_DrawRectPatch;
		env->lpVtbl->DrawTriPatch = D3D8_DrawTriPatch;
		env->lpVtbl->DeletePatch = D3D8_DeletePatch;
		m_pOrigDDevice = *ppReturnedDeviceInterface;
		m_pDDevice = env;
		*ppReturnedDeviceInterface = (IDirect3DDevice8*)env;

		//Console_Printf ( "OnD3DDeviceCreate();", 0xffffff );

		m_pOrigDDevice->AddRef ( );
		m_uiRefCountDevice = m_pOrigDDevice->Release ( );
	}

	return ret;
}

// Create a direct3D8 environment
d3dproxy8_t*	Direct3D8_Create ( UINT SDK_VERSION )
{
	d3dproxy8_t *env;

	// I suppose thats not gonna happen
	/*if ( SDK_VERSION != D3D_SDK_VERSION )
	{
		OutputDebugString ( "Direct3D versions do not match" );
		TerminateProcess ( GetCurrentProcess(), 1 );
	}*/
	env=(d3dproxy8_t*)malloc(sizeof(d3dproxy8_t));
	env->lpVtbl = (struct d3dproxy8vtbl_s*)malloc(sizeof(struct d3dproxy8vtbl_s));
	env->lpVtbl->QueryInterface = Direct3D8_QueryInterface;
	env->lpVtbl->AddRef = Direct3D8_AddRef;
	env->lpVtbl->Release = Direct3D8_Release;
	env->lpVtbl->RegisterSoftwareDevice = Direct3D8_RegisterSoftwareDevice;
	env->lpVtbl->GetAdapterCount = Direct3D8_GetAdapterCount;
	env->lpVtbl->GetAdapterIdentifier = Direct3D8_GetAdapterIdentifier;
	env->lpVtbl->GetAdapterModeCount = Direct3D8_GetAdapterModeCount;
	env->lpVtbl->EnumAdapterModes = Direct3D8_EnumAdapterModes;
	env->lpVtbl->GetAdapterDisplayMode = Direct3D8_GetAdapterDisplayMode;
	env->lpVtbl->CheckDeviceType = Direct3D8_CheckDeviceType;
	env->lpVtbl->CheckDeviceFormat = Direct3D8_CheckDeviceFormat;
	env->lpVtbl->CheckDeviceMultiSampleType = Direct3D8_CheckDeviceMultiSampleType;
	env->lpVtbl->CheckDepthStencilMatch = Direct3D8_CheckDepthStencilMatch;
	env->lpVtbl->GetDeviceCaps = Direct3D8_GetDeviceCaps;
	env->lpVtbl->GetAdapterMonitor = Direct3D8_GetAdapterMonitor;
	env->lpVtbl->CreateDevice = Direct3D8_CreateDevice;

	// We create our main instances here
	isCoreLoaded=1;
	
	return env;
}

/*===============================================
	Initialize
===============================================*/
typedef void *(__stdcall *pfnDirect3DCreate8)(UINT SDK_VERSION);
pfnDirect3DCreate8 m_pfnDirect3DCreate8;

// Create a direct3D interface
void* __stdcall Hook_Direct3DCreate8 ( UINT SDK_VERSION )
{
	size_t sizeOrig = sizeof( *m_pOrigDDevice );
	size_t sizeProxy = sizeof( *m_pDDevice );

	// We trap in d3d8
	if (SDK_VERSION != D3D_SDK_VERSION)
	{
		return m_pfnDirect3DCreate8 ( SDK_VERSION );
	}

	// Create the direct3D interfaces
	m_pOrigDevice=(IDirect3D8*)m_pfnDirect3DCreate8 ( SDK_VERSION );
	m_pDevice=Direct3D8_Create ( SDK_VERSION );
	m_pOrigDevice->AddRef ( );
	m_uiRefCount=m_pOrigDevice->Release ( );
	return m_pDevice;
}

// Initialize the main routines
void	D3D8_Init()
{
	// Hook us as D3D unit
	m_pfnDirect3DCreate8 = (pfnDirect3DCreate8)DetourFunction( DetourFindFunction("D3D8.DLL", "Direct3DCreate8"), (PBYTE)Hook_Direct3DCreate8);
}