/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CStreamingSA.utils.hxx
*  PURPOSE:     Internal data streamer utilities (not in global headers!)
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

// Should be inluded once per streaming .cpp file

// ModelCheckDispatch and its API is established so it is easy to extend the resource
// loading algorithms. Any new handlers can be added with their id regions.
// Once we handle all resource loading routines of the engine, this will the common handler
// (we are still missing ASM code which does this dispatching, right?)

// Default skeleton for model id type dispatching.
// A dispatch method returns either true or false.
//  true - the event was successfully handled
//  false - it could not be handled; might trigger another (default) handler
// (at least this is what it could mean :P)
template <bool defaultReturn>
struct ModelCheckDispatch abstract
{
    __forceinline bool DoBaseModel( unsigned short id )
    {
        return defaultReturn;
    }

    __forceinline bool DoTexDictionary( unsigned short id )
    {
        return defaultReturn;
    }

    __forceinline bool DoCollision( unsigned short id )
    {
        return defaultReturn;
    }

    __forceinline bool DoIPL( unsigned short id )
    {
        return defaultReturn;
    }

    __forceinline bool DoPathFind( unsigned short id )
    {
        return defaultReturn;
    }

    __forceinline bool DoAnimation( unsigned short id )
    {
        return defaultReturn;
    }

    __forceinline bool DoRecording( unsigned short id )
    {
        return defaultReturn;
    }

    __forceinline bool DoScript( unsigned short id )
    {
        return defaultReturn;
    }
};

// Quick macro so we do not have to repeat logic!
#define usRangeCheckExec( id, off, range, func ) \
{ \
    unsigned short offId = usOffset( (id), (off) ); \
    if ( usRangeCheckEx( offId, (range) ) ) \
        return func( offId ); \
}

#define executeDispatch( id, dispatch ) \
{ \
    usRangeCheckExec( (id), 0, MAX_MODELS, (dispatch).DoBaseModel ); \
    usRangeCheckExec( (id), DATA_TEXTURE_BLOCK, 5000, (dispatch).DoTexDictionary ); \
    usRangeCheckExec( (id), DATA_COLL_BLOCK, 256, (dispatch).DoCollision ); \
    usRangeCheckExec( (id), DATA_IPL_BLOCK, 256, (dispatch).DoIPL ); \
    usRangeCheckExec( (id), DATA_PATHFIND_BLOCK, 64, (dispatch).DoPathFind ); \
    usRangeCheckExec( (id), DATA_ANIM_BLOCK, 480, (dispatch).DoAnimation ); \
    usRangeCheckExec( (id), DATA_RECORD_BLOCK, 75, (dispatch).DoRecording ); \
}

template <class type>
bool __forceinline ExecuteDispatch( unsigned short id, type dispatch )
{
    executeDispatch( id, dispatch );
    return false;
}

template <class numType, class dispatchType>
bool __forceinline ExecuteDispatchEasy( numType id, dispatchType dispatch )
{
    if ( (unsigned)id < MAX_MODELS )                                                            return dispatch.DoBaseModel( id );
    if ( (unsigned)id >= DATA_TEXTURE_BLOCK && (unsigned)id < DATA_TEXTURE_BLOCK + MAX_TXD )    return dispatch.DoTexDictionary( id - DATA_TEXTURE_BLOCK );
    if ( (unsigned)id >= DATA_COLL_BLOCK && (unsigned)id < DATA_COLL_BLOCK + 256 )              return dispatch.DoCollision( id - DATA_COLL_BLOCK );
    if ( (unsigned)id >= DATA_IPL_BLOCK && (unsigned)id < DATA_IPL_BLOCK + 256 )                return dispatch.DoIPL( id - DATA_IPL_BLOCK );
    if ( (unsigned)id >= DATA_PATHFIND_BLOCK && (unsigned)id < DATA_PATHFIND_BLOCK )            return dispatch.DoPathFind( id - DATA_PATHFIND_BLOCK );
    if ( (unsigned)id >= DATA_ANIM_BLOCK && (unsigned)id < DATA_ANIM_BLOCK + 480 )              return dispatch.DoAnimation( id - DATA_ANIM_BLOCK );
    if ( (unsigned)id >= DATA_RECORD_BLOCK && (unsigned)id < DATA_RECORD_BLOCK + 75 )           return dispatch.DoRecording( id - DATA_RECORD_BLOCK );

    return false;
}

// Use only if model id type information is chained (0-19999 base model, 20000-24999 txds, ...)
// If the compiler is smart, then it would detect ExecuteDispatchEasy as this
// We cannot be certain for now, as we are not the GTA:SA engine ourselves.
// That makes it hard to establish a perfect ID chain.
template <class numType, class dispatchType>
bool __forceinline ExecuteDispatchChain( numType id, dispatchType dispatch )
{
    if ( (unsigned)id < DATA_TEXTURE_BLOCK )                return dispatch.DoBaseModel( id );
    if ( (unsigned)id < DATA_COLL_BLOCK )                   return dispatch.DoTexDictionary( usOffset( id, DATA_TEXTURE_BLOCK ) );
    if ( (unsigned)id < DATA_IPL_BLOCK )                    return dispatch.DoCollision( usOffset( id, DATA_COLL_BLOCK ) );
    if ( (unsigned)id < DATA_PATHFIND_BLOCK )               return dispatch.DoIPL( usOffset( id, DATA_IPL_BLOCK ) );
    if ( (unsigned)id < DATA_ANIM_BLOCK )                   return dispatch.DoPathFind( usOffset( id, DATA_PATHFIND_BLOCK ) );
    if ( (unsigned)id < DATA_RECORD_BLOCK )                 return dispatch.DoAnimation( usOffset( id, DATA_ANIM_BLOCK ) );
    if ( (unsigned)id < DATA_RECORD_BLOCK + 75 )            return dispatch.DoRecording( usOffset( id, DATA_RECORD_BLOCK ) );

    return false;
}

// Define this to use your favorite dispatcher method.
// Might improve performance if the compiler optimizes a certain type of branching better.
// You could try experimenting with this.
// This method should be used if you are uncertain which method grants you best performance.
// Otherwise choose any dispatching method provided above.
#define DefaultDispatchExecute( id, dispatch )  ( ExecuteDispatchEasy( (id), (dispatch) ) )