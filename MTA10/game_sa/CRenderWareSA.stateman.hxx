/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        game_sa/CRenderWareSA.stateman.hxx
*  PURPOSE:     RenderWare generic optimized state manager
*  DEVELOPERS:  Martin Turski <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _RENDERWARE_STATE_MANAGER_
#define _RENDERWARE_STATE_MANAGER_

#include "RenderWare/RwInternals.h"
#include "RenderWare/RwRenderTools.hxx"

template <typename dataType>
struct CachedConstructedClassAllocator
{
    struct dataEntry : dataType
    {
        AINLINE dataEntry( CachedConstructedClassAllocator *storage )
        {
            LIST_INSERT( storage->m_freeList.root, this->node );
        }

        RwListEntry <dataEntry> node;
    };

    AINLINE CachedConstructedClassAllocator( unsigned int startCount )
    {
        LIST_CLEAR( m_usedList.root );
        LIST_CLEAR( m_freeList.root );

        void *mem = (void*)new char[ sizeof( dataEntry ) * startCount ];

        for ( unsigned int n = 0; n < startCount; n++ )
        {
            new ( (dataEntry*)mem + n ) dataEntry( this );
        }
    }

    AINLINE dataEntry* AllocateNew( void )
    {
        return new dataEntry( this );
    }

    AINLINE dataType* Allocate( void )
    {
        dataEntry *entry = NULL;

        if ( !LIST_EMPTY( m_freeList.root ) )
        {
            entry = LIST_GETITEM( dataEntry, m_freeList.root.next, node );
        }

        if ( !entry )
        {
            entry = AllocateNew();
        }

        if ( entry )
        {
            LIST_REMOVE( entry->node );
            LIST_INSERT( m_usedList.root, entry->node );
        }

        return entry;
    }

    AINLINE void Free( dataType *data )
    {
        dataEntry *entry = (dataEntry*)data;

        LIST_REMOVE( entry->node );
        LIST_INSERT( m_freeList.root, entry->node );
    }

    RwList <dataEntry> m_usedList;
    RwList <dataEntry> m_freeList;
};

template <typename stateGenericType>
struct RwStateManager
{
    // Required type exports from the generic template.
    typedef typename stateGenericType::valueType stateValueType;
    typedef typename stateGenericType::stateAddress stateAddress;

    struct stateManagedType
    {
        stateValueType value;
        bool isRequired;

        AINLINE stateManagedType( void )
        {
            isRequired = false;
        }

        operator stateValueType& ( void )
        {
            return value;
        }

        operator const stateValueType& ( void ) const
        {
            return value;
        }
    };

    struct stateManagedArrayManager
    {
        AINLINE void InitField( stateManagedType& managed )
        {
            return;
        }
    };
    typedef growableArray <stateManagedType, 8, 0, stateManagedArrayManager, unsigned int> stateManagedArray;

    struct stateAddressArrayManager
    {
        AINLINE void InitField( stateAddress& value )
        {
            return;
        }
    };
    typedef growableArray <stateAddress, 8, 0, stateAddressArrayManager, unsigned int> stateAddressArray;

    struct stateDeviceArrayManager
    {
        AINLINE void InitField( stateValueType& value )
        {
            return;
        }
    };
    typedef growableArray <stateValueType, 8, 0, stateDeviceArrayManager, unsigned int> stateDeviceArray;

    struct __declspec(novtable) deviceStateReferenceDevice abstract
    {
        virtual ~deviceStateReferenceDevice( void )     {}

        virtual bool GetDeviceState( const stateAddress& address, stateValueType& value ) const = 0;
    };

    AINLINE RwStateManager( void ) : allocator( 128 ), changeSetAllocator( 16 )
    {
        LIST_CLEAR( activeChangeSets.root );

        // In the beginning we cannot set device states and will get only invalidated states.
        isValid = false;

        // Allocate the main change set.
        rootChangeSet = AllocateChangeSet();

        changeRefDevice = new changeSetLocalReferenceDevice( *this );

        rootChangeSet->SetReferenceDevice( changeRefDevice );

        bucketChangeRefDevice = new changeSetBucketReferenceDevice( *this );

        bucketChangeSet = NULL;
        isInBucketPass = false;

        InitializeCriticalSection( &localStateLock );
    }

    AINLINE ~RwStateManager( void )
    {
        assert( isInBucketPass == false );

        while ( !LIST_EMPTY( activeChangeSets.root ) )
        {
            FreeChangeSet( LIST_GETITEM( changeSet, activeChangeSets.root.next, node ) );
        }

        delete bucketChangeRefDevice;

        delete changeRefDevice;

        DeleteCriticalSection( &localStateLock );
    }

    AINLINE const stateValueType& GetDeviceState( const stateAddress& address )
    {
        // If we are not valid, we return invalid device states.
        if ( !isValid )
            return stateGeneric.GetInvalidDeviceState();

        return localStates.ObtainItem( address.GetArrayIndex() );
    }

    AINLINE void LockLocalState( void )
    {
        EnterCriticalSection( &localStateLock );
    }

    AINLINE void UnlockLocalState( void )
    {
        LeaveCriticalSection( &localStateLock );
    }

    AINLINE void SetDeviceState( const stateAddress& address, const stateValueType& value )
    {
        if ( !isValid )
            return;

        // Make sure threads can lock the local state.
        LockLocalState();

        localStates.SetItem( value, address.GetArrayIndex() );

        // Notify the change sets.
        LIST_FOREACH_BEGIN( changeSet, activeChangeSets.root, node )
            item->OnDeviceStateChange( address, value );
        LIST_FOREACH_END

        UnlockLocalState();
    }

    AINLINE void SetDeviceStateChecked( const stateAddress& address, const stateValueType& value )
    {
        // Only try to set device state if it is not equal to local state.
        const stateValueType& currentValue = localStates.ObtainItem( address.GetArrayIndex() );

        if ( !CompareDeviceStates( currentValue, value ) )
        {
            SetDeviceState( address, value );
        }
    }

    AINLINE void ResetDeviceState( const stateAddress& address )
    {
        stateGeneric.ResetDeviceState( address );
    }

    AINLINE bool CompareDeviceStates( const stateValueType& left, const stateValueType& right ) const
    {
        return stateGeneric.CompareDeviceStates( left, right );
    }

    AINLINE bool IsCurrentLocalState( const stateAddress& address, const stateValueType& right )
    {
        return CompareDeviceStates( GetDeviceState( address ), right );
    }

    struct changeSetLocalReferenceDevice : public deviceStateReferenceDevice
    {
        AINLINE changeSetLocalReferenceDevice( RwStateManager& manager ) : manager( manager )
        {
            return;
        }

        AINLINE bool GetDeviceState( const stateAddress& address, stateValueType& value ) const
        {
            value = manager.deviceStates.ObtainItem( address.GetArrayIndex() );
            return true;
        }

        RwStateManager& manager;
    };

    AINLINE void ApplyDeviceStates( void )
    {
        if ( !isValid )
            return;

        // Traverse local states to device states.
        changeSet *changeSet = rootChangeSet;

        for ( changeSet::changeSetIterator iterator = changeSet->GetIterator(); !iterator.IsEnd(); iterator.Increment() )
        {
            const stateAddress& address = iterator.Resolve();

            unsigned int arrayIndex = address.GetArrayIndex();

            // Unset the flag.
            changeSet->isQueried.SetItem( false, arrayIndex );

            // Update now.
            const stateValueType& newState = localStates.Get( arrayIndex );
            stateValueType& currentState = deviceStates.Get( arrayIndex );

            stateGeneric.SetDeviceState( address, newState );

            currentState = newState;
        }

        // Reset the change set.
        changeSet->Reset();
    }

    // Method should be used to ensure device state integrity.
    // MTA could leak some device states, so we need to prevent that.
    AINLINE void ValidateDeviceStates( void )
    {
        if ( !isValid )
            return;

        for ( stateAddress iterator; !iterator.IsEnd(); iterator.Increment() )
        {
            unsigned int arrayIndex = iterator.GetArrayIndex();

            stateValueType& localValue = deviceStates.Get( arrayIndex );
            stateValueType actualValue;

            stateGeneric.GetDeviceState( iterator, actualValue );

            if ( !CompareDeviceStates( localValue, actualValue ) )
            {
#ifdef DEBUG_DEVICE_STATE_INTEGRITY
                __asm int 3
#endif //DEBUG_DEVICE_STATE_INTEGRITY

                localValue = actualValue;
            }
        }
    }

    struct changeSet
    {
        AINLINE changeSet( void )
        {
            referenceDevice = NULL;
            manager = NULL;
        }

        AINLINE void SetManager( RwStateManager *manager )
        {
            this->manager = manager;
        }

        AINLINE void SetReferenceDevice( deviceStateReferenceDevice *device )
        {
            referenceDevice = device;
        }

        AINLINE void Initialize( void )
        {
            for ( unsigned int n = 0; n < isQueried.GetSizeCount(); n++ )
            {
                isQueried.SetItem( false, n );
            }
            changedStates.Clear();
        }

        AINLINE void SetChanged( const stateAddress& address, bool isChanged )
        {
            bool isInsideQuery = IsChanged( address );

            isQueried.SetItem( isChanged, address.GetArrayIndex() );
            
            if ( isChanged )
            {
                if ( !isInsideQuery )
                {
                    changedStates.AddItem( address );
                }
            }
            else
            {
                if ( isInsideQuery )
                {
                    changedStates.RemoveItem( address );
                }
            }
        }

        AINLINE bool IsChanged( const stateAddress& address ) const
        {
            unsigned int arrayIndex = address.GetArrayIndex();

            if ( arrayIndex < isQueried.GetSizeCount() )
            {
                return isQueried.Get( arrayIndex );
            }

            return false;
        }

        AINLINE void OnDeviceStateChange( const stateAddress& address, const stateValueType& value )
        {
            stateValueType refVal;

            bool isRequired = GetOriginalDeviceState( address, refVal );

            if ( isRequired )
            {
                SetChanged( address, !manager->CompareDeviceStates( refVal, value ) );
            }
        }
        
        AINLINE bool HasChanges( void ) const
        {
            return ( changedStates.GetCount() != 0 );
        }

        AINLINE void Reset( void )
        {
            changedStates.Clear();
        }

        AINLINE bool GetOriginalDeviceState( const stateAddress& address, stateValueType& refVal )
        {
            return referenceDevice->GetDeviceState( address, refVal );
        }

        struct changeSetIterator
        {
            AINLINE changeSetIterator( changeSet& manager ) : manager( manager )
            {
                iter = 0;
            }

            AINLINE void Increment( void )
            {
                iter++;
            }

            AINLINE bool IsEnd( void ) const
            {
                return ( iter == manager.changedStates.GetCount() );
            }

            AINLINE const stateAddress& Resolve( void )
            {
                return manager.changedStates.Get( iter );
            }

            changeSet& manager;
            unsigned int iter;
        };

        AINLINE changeSetIterator GetIterator( void )
        {
            return changeSetIterator( *this );
        }

        struct updateFlagArrayManager
        {
            AINLINE void InitField( bool& isUpdated )
            {
                isUpdated = false;
            }
        };
        typedef growableArray <bool, 32, 0, updateFlagArrayManager, unsigned int> updateFlagArray;

        stateAddressArray changedStates;
        updateFlagArray isQueried;

        RwListEntry <changeSet> node;

        deviceStateReferenceDevice *referenceDevice;

        RwStateManager *manager;
    };

    typedef CachedConstructedClassAllocator <changeSet> changeSetAllocator_t;

    changeSetAllocator_t changeSetAllocator;

    AINLINE changeSet* AllocateChangeSet( void )
    {
        changeSet *set = changeSetAllocator.Allocate();

        set->Initialize();
        set->SetManager( this );

        LIST_INSERT( activeChangeSets.root, set->node );
        return set;
    }

    AINLINE void FreeChangeSet( changeSet *set )
    {
        LIST_REMOVE( set->node );

        changeSetAllocator.Free( set );
    }

    struct capturedState
    {
        struct capturedStateValueType
        {
            AINLINE capturedStateValueType( void )
            {
                isSet = false;
            }

            stateManagedType managedType;
            bool isSet;
        };

        struct capturedStateValueTypeArrayManager
        {
            AINLINE void InitField( capturedStateValueType& value )
            {
                return;
            }
        };
        typedef growableArray <capturedStateValueType, 8, 0, capturedStateValueTypeArrayManager, unsigned int> capturedStateValueArray;

        AINLINE capturedState( void )
        {
            manager = NULL;
            stateChangeSet = NULL;
            refDevice = new changeSetCapturedReferenceDevice( *this );
        }

        AINLINE ~capturedState( void )
        {
            Terminate();

            delete refDevice;
        }

        struct changeSetCapturedReferenceDevice : public deviceStateReferenceDevice
        {
            AINLINE changeSetCapturedReferenceDevice( capturedState& state ) : state( state )
            {
                return;
            }

            AINLINE bool GetDeviceState( const stateAddress& address, stateValueType& value ) const
            {
                unsigned int arrayIndex = address.GetArrayIndex();

                const capturedStateValueType& stateValue = state.deviceStates.ObtainItem( arrayIndex );

                if ( !stateValue.managedType.isRequired )
                    return false;

                if ( !stateValue.isSet )
                {
                    value = state.manager->bucketStates.ObtainItem( arrayIndex );
                }
                else
                {
                    value = stateValue.managedType;
                }

                return true;
            }

            capturedState& state;
        };

        AINLINE void SetManager( RwStateManager *manager )
        {
            this->manager = manager;

            if ( !stateChangeSet )
            {
                stateChangeSet = manager->AllocateChangeSet();

                stateChangeSet->SetReferenceDevice( refDevice );
            }

            // Reset our device states.
            for ( unsigned int n = 0; n < deviceStates.GetSizeCount(); n++ )
            {
                capturedStateValueType& value = deviceStates.Get( n );

                value.isSet = false;
            }
        }

        AINLINE void Terminate( void )
        {
            if ( stateChangeSet )
            {
                manager->FreeChangeSet( stateChangeSet );

                stateChangeSet = NULL;
            }
        }

        AINLINE void Capture( void )
        {
            for ( stateChangeIterator iterator( *manager ); !iterator.IsEnd(); iterator.Increment() )
            {
                const stateAddress& address = iterator.Resolve();

                unsigned int arrayIndex = address.GetArrayIndex();

                capturedStateValueType value;

                bool isRequired = manager->stateGeneric.IsDeviceStateRequired( address, *manager->changeRefDevice );

                value.managedType.isRequired = isRequired;
                value.managedType.value = manager->GetDeviceState( address );
                value.isSet = true;

                deviceStates.SetItem( value, arrayIndex );
            }
        }

        AINLINE bool IsCurrent( void ) const
        {
            for ( changeSet::changeSetIterator iterator( *stateChangeSet ); !iterator.IsEnd(); iterator.Increment() )
            {
                const stateAddress& address = iterator.Resolve();

                // Check that the state is even required for us.
                if ( deviceStates.Get( address.GetArrayIndex() ).managedType.isRequired )
                {
                    // De-facto changed.
                    return false;
                }
            }

            return true;
        }

        AINLINE changeSet* GetChangeSet( void )
        {
            return stateChangeSet;
        }

        AINLINE void SetDeviceTo( void )
        {
            manager->RestoreToChangeSetState( stateChangeSet );
        }

        RwStateManager *manager;
        capturedStateValueArray deviceStates;

        changeSet *stateChangeSet;
        changeSetCapturedReferenceDevice *refDevice;
    };

    typedef CachedConstructedClassAllocator <capturedState> stateAllocator;

    stateAllocator allocator;

    AINLINE capturedState* CaptureState( void )
    {
        capturedState *state = allocator.Allocate();

        state->SetManager( this );
        state->Capture();
        return state;
    }

    AINLINE void FreeState( capturedState *state )
    {
        state->Terminate();

        allocator.Free( state );
    }

    AINLINE void Initialize( void )
    {
        // Grab the current states.
        // Also set up the local states.
        for ( stateAddress iterator; !iterator.IsEnd(); iterator.Increment() )
        {
            unsigned int arrayIndex = iterator.GetArrayIndex();

            stateValueType& deviceValue = deviceStates.ObtainItem( arrayIndex );

            stateGeneric.GetDeviceState( iterator, deviceValue );

            // Set up local states array.
            localStates.SetItem( deviceValue, arrayIndex );
        }

        // We are now valid.
        isValid = true;

        // Reset change sets.
        LIST_FOREACH_BEGIN( changeSet, activeChangeSets.root, node )
            item->Initialize();
        LIST_FOREACH_END
    }

    AINLINE void Invalidate( void )
    {
        // Invalidated.
        isValid = false;
    }

    AINLINE void ClearDevice( void )
    {
        // For all possible states, clear them.
        for ( stateAddress iterator; !iterator.IsEnd(); iterator.Increment() )
        {
            stateGeneric.ResetDeviceState( iterator );
        }
    }

    struct changeSetBucketReferenceDevice : public deviceStateReferenceDevice
    {
        AINLINE changeSetBucketReferenceDevice( RwStateManager& manager ) : manager( manager )
        {
            return;
        }

        AINLINE bool GetDeviceState( const stateAddress& address, stateValueType& value ) const
        {
            value = manager.bucketStates.ObtainItem( address.GetArrayIndex() );
            return true;
        }

        RwStateManager& manager;
    };

    AINLINE void BeginBucketPass( void )
    {
        assert( isInBucketPass == false );

        bucketChangeSet = AllocateChangeSet();

        // Clone the current local states into the bucket reference states.
        bucketStates = localStates;

        bucketChangeSet->SetReferenceDevice( bucketChangeRefDevice );

        isInBucketPass = true;
    }

    AINLINE changeSet* GetBucketChangeSet( void )
    {
        return bucketChangeSet;
    }

    stateAddressArray changedStateAddresses;

    AINLINE void RestoreToChangeSetState( changeSet *set )
    {
        // Set device states to the state when the change set started from.
        for ( changeSet::changeSetIterator iterator = set->GetIterator(); !iterator.IsEnd(); iterator.Increment() )
        {
            changedStateAddresses.AddItem( iterator.Resolve() );
        }

        for ( unsigned int n = 0; n < changedStateAddresses.GetCount(); n++ )
        {
            const stateAddress& changedAddress = changedStateAddresses.Get( n );

            stateValueType origValue;
            
            if ( set->GetOriginalDeviceState( changedAddress, origValue ) )
            {
                SetDeviceState( changedAddress, origValue );
            }
        }
        changedStateAddresses.Clear();
    }

    AINLINE void SetBucketStates( void )
    {
        RestoreToChangeSetState( GetBucketChangeSet() );
    }

    AINLINE void EndBucketPass( void )
    {
        assert( isInBucketPass == true );

        FreeChangeSet( bucketChangeSet );

        isInBucketPass = false;
    }

    struct stateChangeIterator
    {
        AINLINE stateChangeIterator( RwStateManager& manager )
        {
            changeSet *set = manager.GetBucketChangeSet();

            if ( set )
            {
                isChangeSetIterator = true;

                changeSetIterator = new (staticAlloc.Allocate()) changeSet::changeSetIterator( set->GetIterator() );
            }
            else
            {
                isChangeSetIterator = false;
            }
        }

        AINLINE ~stateChangeIterator( void )
        {
            if ( isChangeSetIterator )
            {
                changeSetIterator->~changeSetIterator();
            }
        }

        AINLINE void Increment( void )
        {
            if ( isChangeSetIterator )
            {
                changeSetIterator->Increment();
            }
            else
            {
                stateIterator.Increment();
            }
        }

        AINLINE bool IsEnd( void ) const
        {
            if ( isChangeSetIterator )
            {
                return changeSetIterator->IsEnd();
            }
            else
            {
                return stateIterator.IsEnd();
            }
        }

        AINLINE const stateAddress& Resolve( void )
        {
            if ( isChangeSetIterator )
            {
                return changeSetIterator->Resolve();
            }
            else
            {
                return stateIterator;
            }
        }

        StaticAllocator <typename changeSet::changeSetIterator, 1> staticAlloc;

        bool isChangeSetIterator;
        typename changeSet::changeSetIterator *changeSetIterator;
        stateAddress stateIterator;
    };

    // Interface to access the device states.
    stateGenericType stateGeneric;

    // Root change set used for updating to the device.
    changeSetLocalReferenceDevice *changeRefDevice;
    changeSet* rootChangeSet;

    // Variables for bucket management.
    changeSet* bucketChangeSet;
    changeSetBucketReferenceDevice *bucketChangeRefDevice;
    bool isInBucketPass;

    RwList <changeSet> activeChangeSets;

    stateDeviceArray localStates;
    stateDeviceArray deviceStates;

    // Boolean whether the state manager is valid.
    // It it valid if the device is valid.
    bool isValid;

    stateDeviceArray bucketStates;

    // Lock object used to state management.
    CRITICAL_SECTION localStateLock;
};

#endif //_RENDERWARE_STATE_MANAGER_