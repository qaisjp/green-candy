/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        sdk/game/CStreaming.h
*  PURPOSE:     Game streaming interface
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CStreaming_H
#define __CStreaming_H

class CStreaming
{
public:
    virtual void            RequestModel( unsigned short id, unsigned int flags ) = 0;
    virtual void            FreeModel( unsigned short id ) = 0;
    virtual void            LoadAllRequestedModels( bool onlyPriority = false ) = 0;
    virtual bool            HasModelLoaded( unsigned int id ) = 0;
    virtual bool            IsModelLoading( unsigned int id ) = 0;
    virtual void            WaitForModel( unsigned int id ) = 0;
    virtual void            RequestAnimations( int idx, unsigned int flags ) = 0;
    virtual bool            HaveAnimationsLoaded( int idx ) = 0;
    virtual bool            HasVehicleUpgradeLoaded( int model ) = 0;
    virtual void            RequestSpecialModel( unsigned short model, const char *tex, unsigned int channel ) = 0;
};

#endif
