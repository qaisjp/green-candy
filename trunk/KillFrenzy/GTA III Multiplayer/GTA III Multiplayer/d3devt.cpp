#include "StdInc.h"

/*=============================
	Events
=============================*/
bool m_bIsFirstFrame=true;

// On Present
void	D3D_OnPresent ( IDirect3DDevice8 *pDevice )
{
	DWORD StateToken=0;

	if (m_bIsFirstFrame)
	{
		Core_FirstFrame ();
		m_bIsFirstFrame=false;
	}
	Core_PreRender ();

	// Begin our new scene
	m_pOrigDDevice->BeginScene ();
	//m_pOrigDDevice->SetRenderTarget ( m_pOrigTarget, NULL );

	// Create a state block
	IDirect3DDevice8_CreateStateBlock ( m_pOrigDDevice, D3DSBT_ALL, &StateToken );

	// Do frame function
	if (Core_Render( m_pOrigDDevice ))
	{
		// Draw console
		Console_Draw ( m_pOrigDDevice );
	}

	// Destroy StateBlock
	if (StateToken)
	{
		IDirect3DDevice8_ApplyStateBlock ( m_pOrigDDevice, StateToken );
		IDirect3DDevice8_DeleteStateBlock ( m_pOrigDDevice, StateToken );
	}

	// End scene
	m_pOrigDDevice->EndScene ();

	Core_PostRender ();
}

// On Restore
void	D3D_OnRestore ( IDirect3DDevice8 *pDevice )
{

}

// On Invalidate
void	D3D_OnInvalidate ( IDirect3DDevice8 *pDevice )
{

}

void	D3D_BeginScene ()
{
	//Core_PreRender();
}