/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.utils.h
*  PURPOSE:     Data streamer utilities
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _STREAMING_UTILITIES_
#define _STREAMING_UTILITIES_

// Imported textures scanning
namespace RwImportedScan
{
    void Apply( unsigned short model );
    void Unapply();
};

// Remap VEHICLE.TXD export
namespace RwRemapScan
{
    RwTexture* scanMethod( const char *name );
};

// Used by streaming
void __cdecl FreeCOLLibrary( unsigned char collId );
bool __cdecl LoadModel( void *buf, unsigned int id, unsigned int threadId );

#endif //_STREAMING_UTILITIES_