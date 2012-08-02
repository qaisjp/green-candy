/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientGroups.cpp
*  PURPOSE:     Dummy grouping class
*  DEVELOPERS:  Chris McArthur <>
*               Jax <>
*               The_GTA <quiret@gmx.de>
*
*****************************************************************************/

#include <StdInc.h>

using std::list;

void CClientGroups::DeleteAll ( void )
{
    // TODO
    return; // We cannot afford to do this

    // Delete all the elements
    m_bDontRemoveFromList = true;

    for ( list < CClientDummy* >::iterator iter = m_List.begin () ; iter != m_List.end () ; ++iter )
        (*iter)->Delete();

    m_List.clear ();

    m_bDontRemoveFromList = false;
}


void CClientGroups::RemoveFromList ( CClientDummy* pDummy )
{
    if ( !m_bDontRemoveFromList )
    {
        if ( !m_List.empty () )
        {
            for ( list < CClientDummy* >::iterator iter = m_List.begin () ; iter != m_List.end () ; )
            {
                CClientDummy* pCurrentDummy = *iter;
                if ( pCurrentDummy == pDummy )
                {
                    iter = m_List.erase ( iter );
                }
                else
                {
                    ++iter;
                }
            }
        }
    }
}
