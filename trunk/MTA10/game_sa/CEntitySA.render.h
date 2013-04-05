/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CEntitySA.render.h
*  PURPOSE:     Header file for entity render utilities
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _ENTITY_RENDER_
#define _ENTITY_RENDER_

// Common utility for grouped z-layered rendering
template <typename entryType>
class CRenderChainInterface
{
public:
    typedef entryType depthLevel;

    class renderChain
    {
    public:
        depthLevel              m_entry;
        renderChain*            next;
        renderChain*            prev;
    };

    /*=========================================================
        CRenderChainInterface::constructor

        Arguments:
            max - maximum number of rendering entries available
        Purpose:
            Constructs this rendering chain by allocating the maximum
            number of entries which this chain can hold.
        Note:
            Multiple constructors have been inlined into
            HOOK_InitRenderChains.
    =========================================================*/
    CRenderChainInterface( unsigned int max )
    {
        // Allocate all entries
        m_stack = new renderChain[max];

        // Initialize the list
        m_root.prev = &m_rootLast;
        m_rootLast.next = &m_root;
        m_renderStack.prev = &m_renderLast;
        m_renderLast.next = &m_renderStack;

        // Setup height properties
        m_rootLast.m_entry.InitLast();
        m_root.m_entry.InitFirst();

        while ( max-- )
        {
            renderChain& iter = m_stack[max];

            iter.prev = m_renderStack.prev;
            m_renderStack.prev->next = &iter;
            iter.next = &m_renderStack;
            m_renderStack.prev = &iter;
        }
    }

    /*=========================================================
        CRenderChainInterface::destructor

        Purpose:
            Deallocates the memory used for this rendering list.
    =========================================================*/
    ~CRenderChainInterface( void )
    {
        delete [] m_stack;
    }

    /*=========================================================
        CRenderChainInterface::PushRender

        Arguments:
            level - the description of the new entry
        Purpose:
            Queries to render the specified userdata using a given
            callback.
        Binary offsets:
            (1.0 US and 1.0 EU): 0x00733910 (atomicRenderChain)
        Note:
            Apparently the render chains have a pre-allocated amount of
            possible render instances. We should investigate, how this limit
            affects the performance and quality of gameplay.
    =========================================================*/
    bool PushRender( depthLevel *level )
    {
        renderChain *iter = m_root.prev;
        renderChain *progr;

        // Scan until we find appropriate slot
        for ( ; iter != &m_rootLast && iter->m_entry < *level; 
            iter = iter->prev );

        if ( ( progr = m_renderStack.prev ) == &m_renderLast )
            return false;

        // Update render details
        progr->m_entry = *level;

        LIST_REMOVE( *progr );
        
        iter = iter->next;
        progr->prev = iter->prev;
        iter->prev->next = progr;
        progr->next = iter;
        iter->prev = progr;
        return true;
    }

    /*=========================================================
        CRenderChainInterface::Execute

        Purpose:
            Executes all items in their order.
        Binary offsets:
            (1.0 US and 1.0 EU): 0x00733E90 (vehicleRenderChain)
    =========================================================*/
    void Execute( void )
    {
        for ( renderChain *iter = m_rootLast.next; iter != &m_root; iter = iter->next )
        {
            iter->m_entry.Execute();
        }
    }

    /*=========================================================
        CRenderChainInterface::Clear

        Purpose:
            Resets all ordered items
        Binary offsets:
            (1.0 US and 1.0 EU): 0x00733F40 (vehicleRenderChain)
    =========================================================*/
    void Clear( void )
    {
        while ( m_rootLast.next != &m_root )
        {
            renderChain *iter = m_rootLast.next;

            LIST_REMOVE( *iter );
            LIST_APPEND( m_renderStack, *iter );
        }
    }

    renderChain                 m_root;             // 0
    renderChain                 m_rootLast;         // 20
    renderChain                 m_renderStack;      // 40
    renderChain                 m_renderLast;       // 60
    renderChain*                m_stack;            // 80
};

// Used by vehicles and such.
struct atomicRenderInfo
{
    void InitFirst( void )
    {
        distance = 0;
    }

    void InitLast( void )
    {
        distance = 100000000;
    }

    bool operator <( const atomicRenderInfo& right )
    {
        return distance < right.distance;
    }

    RpAtomic    *atomic;
    void        (__cdecl*callback)( RpAtomic *atom );
    float       distance;

    inline void Execute( void )
    {
        callback( atomic );
    }
};
typedef CRenderChainInterface <atomicRenderInfo> atomicRenderChain_t;

// Entity chain information
struct entityRenderInfo
{
    void InitFirst( void )
    {
        distance = 0;
    }

    void InitLast( void )
    {
        distance = 100000000;
    }

    bool operator <( const entityRenderInfo& right )
    {
        return distance < right.distance;
    }

    CEntitySAInterface  *entity;
    void                (__cdecl*callback)( CEntitySAInterface *intf, float dist );
    float               distance;

    inline void Execute( void )
    {
        callback( entity, distance );
    }
};
typedef CRenderChainInterface <entityRenderInfo> entityRenderChain_t;

// Ped information
struct pedRenderInfo
{
    void InitFirst( void )
    {
    }

    void InitLast( void )
    {
    }

    bool operator <( const pedRenderInfo& right )
    {
        return false;
    }

    class CPedSAInterface   *ped;
    
    inline void Execute();
};
typedef CRenderChainInterface <pedRenderInfo> pedRenderChain_t;

void EntityRender_Init( void );
void EntityRender_Shutdown( void );

#endif //_ENTITY_RENDER_