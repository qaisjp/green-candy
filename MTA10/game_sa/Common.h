/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/Common.h
*  PURPOSE:     Common game layer include file
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef __CGAMESA_COMMON
#define __CGAMESA_COMMON

#define CGAME_DLL

#include <game/Common.h>

template < size_t heapSize >
class CAlignedStackSA
{
public:
    CAlignedStackSA()
    {
        m_data = pRwInterface->m_malloc( heapSize );
        m_size = heapSize;
        m_offset = 0;

        memset( m_data, 0, m_size );
    }

    ~CAlignedStackSA()
    {
        pRwInterface->m_free( m_data );
    }

    void*   Allocate( size_t size, size_t align )
    {
        size_t remains = m_offset % elementSize;
        void *alloc;

        // Realign the offset
        if ( remains )
            m_offset += align - remains;

        // Bugfix
        if ( m_offset + size > m_size )
            return NULL;

        alloc = (void*)((unsigned char*)m_data + m_offset);
        m_offset += size;

        return alloc;
    }

    void*   AllocateInt( size_t size )
    {
        return Push( size, sizeof(int) );
    }

    void*                                   m_data;
    size_t                                  m_size;
    size_t                                  m_offset;
};

class CSimpleList
{
public:
    friend class Item;

    class Item
    {
    public:
        Item()
        {
            m_next = NULL;
            m_prev = NULL;
        }

        inline void Append( Item *iter )
        {
            m_next = iter;
            iter->m_prev = this;
        }

        inline void Insert( Item *iter )
        {
            m_prev = iter;
            iter->m_next = this;
        }

        inline void Remove()
        {
            if ( m_next )
                m_next->m_prev = m_prev;

            if ( m_prev )
                m_prev->m_next = m_next;
        }

        Item*       m_next;
        Item*       m_prev;
    };

    CSimpleList()
    {
        m_count = 0;
    }

    ~CSimpleList()
    {
    }

    inline void Add( Item *item )
    {
        item->m_next = NULL;

        if ( m_root.m_next )
            m_root.m_next->Append( item );
        else
        {
            item->m_prev = NULL;

            m_root.m_prev = item;
        }

        m_root.m_next = item;

        m_count++;
    }

    inline void Remove( Item *item )
    {
        item->Remove();

        m_count--;
    }

    inline unsigned int Count()
    {
        return m_count;
    }

    Item            m_root;
    unsigned int    m_count;
};

#undef DEBUG_LOG
#ifdef DEBUG_LOG
    #include <stdio.h>
    #include <time.h>

    static FILE* fDebugFile;

    static void OutputDebugText ( char * szDebugText )
    {
        #ifdef MTA_DEBUG
        char szDebug[500];
        sprintf ( szDebug, "%s\n", szDebugText );
        OutputDebugString ( szDebug );
        #endif
    }

    #ifdef MTA_DEBUG
        #define DEBUG_TRACE(szText) \
            OutputDebugString(szText);
    #endif
#else
    #define DEBUG_TRACE(szText) // we do nothing with release versions
#endif

#endif
