/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientWater.h
*  PURPOSE:     Water polygon class
*  DEVELOPERS:  arc_
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#ifndef __CCLIENTWATER_H
#define __CCLIENTWATER_H

#define LUACLASS_WATER  57

class CClientWater : public CClientEntity
{
public:
                                CClientWater( CClientManager* pManager, ElementID ID, LuaClass& root, bool system, CVector& vecBL, CVector& vecBR, CVector& vecTL, CVector& vecTR, bool bShallow = false );
                                CClientWater( CClientManager* pManager, ElementID ID, LuaClass& root, bool system, CVector& vecL, CVector& vecR, CVector& vecTB, bool bShallow = false );
                                ~CClientWater();

    bool                        Valid()                                          { return m_pPoly != NULL; }

    eClientEntityType           GetType() const                                  { return CCLIENTWATER; }
    int                         GetNumVertices() const;
    void                        GetPosition( CVector& vecPosition ) const;
    bool                        GetVertexPosition( int iVertexIndex, CVector& vecPosition );
    void                        SetPosition( const CVector& vecPosition );
    bool                        SetVertexPosition( int iVertexIndex, CVector& vecPosition, void* pChangeSource = NULL );
    void                        Unlink();

private:
    void                        InstanceLua( bool system );

    CWaterPoly*                 m_pPoly;
    CClientWaterManager*        m_pWaterManager;

    friend class CClientWaterManager;
};

#endif