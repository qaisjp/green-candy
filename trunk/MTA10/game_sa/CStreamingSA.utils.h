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
    void Unapply( void );
};

// Remap VEHICLE.TXD export
namespace RwRemapScan
{
    void Apply( void );
    void Unapply( void );
};

// This is an optimization because the computer binary system works that way.
// Removes the need for additional clock cycles by avoiding conversion to integer.
#define usOffset( num, off )                ((unsigned short)( (unsigned short)(num) - (unsigned short)(off) ))
#define usRangeCheck( num, off, range )     (usOffset((num),(off)) < (unsigned short)(range))
#define usRangeCheckEx( num, range )        ((unsigned short)(num) < (unsigned short)(range))

// Used by streaming
void __cdecl FreeCOLLibrary( unsigned char collId );
bool __cdecl CheckTXDDependency( int id );
bool __cdecl CheckAnimDependency( int id );
bool __cdecl LoadModel( void *buf, unsigned int id, unsigned int threadId );

unsigned int __cdecl ProcessLoadQueue( unsigned int offset, bool favorPriority );
bool __cdecl ProcessStreamingRequest( unsigned int id );
void __cdecl PulseStreamingRequests( void );
void __cdecl PulseStreamingRequest( unsigned int reqId );

#endif //_STREAMING_UTILITIES_