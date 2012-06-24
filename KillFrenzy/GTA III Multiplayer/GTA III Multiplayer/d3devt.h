#ifndef __D3DEVT__
#define __D3DEVT__

#include "d3d8.h"

void	D3D_OnPresent ( IDirect3DDevice8 *pDevice );
void	D3D_OnRestore ( IDirect3DDevice8 *pDevice );
void	D3D_OnInvalidate ( IDirect3DDevice8 *pDevice );

#endif //__D3DEVT__