/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        Shared/logic/networking/NetworkStruct.h
*  PURPOSE:     Network traversal structure management
*  DEVELOPERS:  The_GTA <quiret@gmx.de>
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _NETWORK_STRUCT_
#define _NETWORK_STRUCT_

#ifdef _MSC_VER
#define NETWORK_NOEXCEPT    __declspec(nothrow)
#else
#define NETWORK_NOEXCEPT    noexcept
#endif

// http://groups.google.ie/group/comp.lang.c/msg/706324f25e4a60b0?hl=en&
#define LOG2_8BIT(v)  (8 - 90/(((v)/4+14)|1) - 2/((v)/2+1))
#define LOG2_16BIT(v) (8*((v)>255) + LOG2_8BIT((v) >>8*((v)>255)))
#define LOG2_32BIT(v) \
    (16*((v)>65535L) + LOG2_16BIT((v)*1L >>16*((v)>65535L)))
#define LOG2_64BIT(v)\
    (32*((v)/2L>>31 > 0) \
     + LOG2_32BIT((v)*1L >>16*((v)/2L>>31 > 0) \
                         >>16*((v)/2L>>31 > 0)))

#define INT2BITS(x)     ( (unsigned char)LOG2_32BIT(x) + 1 )
#define BITSIZE(x)      ( INT2BITS(sizeof(x)) )
#define ETSIZE(x)       ( sizeof(x) / sizeof(Networking::NetworkDataType) )

namespace Networking
{
    // exceptions
    class bad_streamer_seek
    { };

    enum eNetworkDataType
    {
        NETWORK_NONE,
        NETWORK_BYTE,
        NETWORK_WORD,
        NETWORK_DWORD,
        NETWORK_FLOAT,
        NETWORK_DOUBLE,
        NETWORK_VECTOR3D,
        NETWORK_BOOL
    };

    struct NetworkDataType
    {
        const eNetworkDataType  type;
        const std::string       name;
    };

    template <size_t defStructSize>
    class DefaultDataStreamer
    {
        inline void NETWORK_NOEXCEPT ForwardID()
        {
            if ( INT2BITS(defStructSize) <= 8 )
                Forward <char> ();
            else if ( INT2BITS(defStructSize) <= 16 )
                Forward <short> ();
            else
                Forward <int> ();
        }

    public:
        template <class type>
        inline NETWORK_NOEXCEPT void Forward( const unsigned int count = 1 )
        {
            m_seek = (void*)( (type*)m_seek + count );
        }

        NETWORK_NOEXCEPT DefaultDataStreamer( size_t size )
        {
            m_seek = m_instance = new char [ size ];
        }

        NETWORK_NOEXCEPT DefaultDataStreamer( const NetworkDataType *def )
        {
            size_t n = defStructSize;

            m_seek = 0;

            // First count for changes
            ForwardID();

            while ( n-- )
            {
                switch( def->type )
                {
                case NETWORK_BOOL:      Forward <bool> (); break;
                case NETWORK_BYTE:      Forward <char> (); break;
                case NETWORK_WORD:      Forward <short> (); break;
                case NETWORK_DWORD:     Forward <int> (); break;
                case NETWORK_FLOAT:     Forward <float> (); break;
                case NETWORK_DOUBLE:    Forward <double> (); break;
                case NETWORK_VECTOR3D:  Forward <CVector> (); break;
                default:                goto end;
                }

                // We add size for an identifier
                ForwardID();

                def++;
            }

end:
            m_seek = m_instance = new char [ (size_t)(const char*)m_seek ];
        }

        ~DefaultDataStreamer()
        {
            delete m_instance;
        }

        inline NETWORK_NOEXCEPT void Reset()
        {
            m_seek = m_instance;
        }

        template <class type>
        inline NETWORK_NOEXCEPT void Write( const type& val )
        {
            *(type*)m_seek = val;
            Forward <type> ();
        }

        template <class type>
        inline NETWORK_NOEXCEPT const type& Get()
        {
            return *(type*)m_seek;
        }

        template <class type>
        inline NETWORK_NOEXCEPT const type& Read()
        {
            const type& val = Get <type> ();
            Forward <type> ();
            return val;
        }
        
    protected:
        void*   m_instance;
        void*   m_seek;
    };

    class NullSyncStruct
    {
    public:
        template <class type>
        inline NETWORK_NOEXCEPT const type& Acknowledge( const unsigned int, const type&, const type& next )
        {
            return next;
        }
    };

    template <class Owner, const size_t defStructSize, class DataStreamer = DefaultDataStreamer <defStructSize>, class Syncer = NullSyncStruct>
    class NetworkStruct
    {
    public:
        NetworkStruct( const NetworkDataType *def ) : m_streamer( def )
        {
            m_def = def;
        }

        NetworkStruct( const NetworkDataType *def, DataStreamer streamer ) : m_streamer( streamer )
        {
            m_def = def;
        }

        ~NetworkStruct()
        {
        }

        template <class SyncerEx>
        inline NETWORK_NOEXCEPT void Write( const Owner& instance, SyncerEx& syncer = SyncerEx() )
        {
            const NetworkDataType *seek = m_def;

            m_streamer.Reset();

            for ( unsigned int n = 0; n < defStructSize; n++ )
            {
                switch( seek->type )
                {
                case NETWORK_BOOL:      m_streamer.Write( syncer.Acknowledge <bool> ( n, m_streamer.Get <bool> (), instance.NetworkRead <bool> ( n ) ) ); break;
                case NETWORK_BYTE:      m_streamer.Write( syncer.Acknowledge <char> ( n, m_streamer.Get <char> (), instance.NetworkRead <char> ( n ) ) ); break;
                case NETWORK_WORD:      m_streamer.Write( syncer.Acknowledge <short> ( n, m_streamer.Get <short> (), instance.NetworkRead <short> ( n ) ) ); break;
                case NETWORK_DWORD:     m_streamer.Write( syncer.Acknowledge <int> ( n, m_streamer.Get <int> (), instance.NetworkRead <int> ( n ) ) ); break;
                case NETWORK_FLOAT:     m_streamer.Write( syncer.Acknowledge <float> ( n, m_streamer.Get <float> (), instance.NetworkRead <float> ( n ) ) ); break;
                case NETWORK_DOUBLE:    m_streamer.Write( syncer.Acknowledge <double> ( n, m_streamer.Get <double> (), instance.NetworkRead <double> ( n ) ) ); break;
                case NETWORK_VECTOR3D:  m_streamer.Write( syncer.Acknowledge <CVector> ( n, m_streamer.Get <CVector> (), instance.NetworkRead <CVector> ( n ) ) ); break;
                default:                return;
                }

                seek++;
            }
        }

        inline NETWORK_NOEXCEPT void Read( Owner& instance )
        {
            const NetworkDataType *seek = m_def;

            m_streamer.Reset();

            for ( unsigned int n = 0; n < defStructSize; n++ )
            {
                switch( seek->type )
                {
                case NETWORK_BOOL:      instance.NetworkWrite( n, m_streamer.Read <bool> () ); break;
                case NETWORK_BYTE:      instance.NetworkWrite( n, m_streamer.Read <char> () ); break;
                case NETWORK_WORD:      instance.NetworkWrite( n, m_streamer.Read <short> () ); break;
                case NETWORK_DWORD:     instance.NetworkWrite( n, m_streamer.Read <int> () ); break;
                case NETWORK_FLOAT:     instance.NetworkWrite( n, m_streamer.Read <float> () ); break;
                case NETWORK_DOUBLE:    instance.NetworkWrite( n, m_streamer.Read <double> () ); break;
                case NETWORK_VECTOR3D:  instance.NetworkWrite( n, m_streamer.Read <CVector> () ); break;
                default:                return;
                }

                seek++;
            }
        }

    protected:
        DataStreamer    m_streamer;
        const NetworkDataType*  m_def;
    };

    template <size_t defStructSize>
    class DefaultBitStreamer : public DefaultDataStreamer <defStructSize>
    {
    public:
        DefaultBitStreamer( const NetworkDataType *def ) : DefaultDataStreamer( def )
        {

        }

        inline NETWORK_NOEXCEPT void Wake() {}
        inline NETWORK_NOEXCEPT void Finalize() {}

        inline NETWORK_NOEXCEPT void WriteBits( const unsigned int val, const size_t bits )
        {
            if ( bits <= 8 )
                Write <char> ( (char)val );
            else if ( bits <= 16 )
                Write <short> ( (short)val );
            else
                Write <int> ( (int)val );
        }

        inline NETWORK_NOEXCEPT unsigned int ReadBits( const size_t bits )
        {
            if ( bits <= 8 )
                return Read <char> ();
            else if ( bits <= 16 )
                return Read <short> ();
            else
                return Read <int> ();
        }

        inline NETWORK_NOEXCEPT bool IsWritten()
        {
            return m_seek != m_instance;
        }
    };

    template <const size_t defEntryCount, class BitStreamer = DefaultBitStreamer <defEntryCount>>
    class BitwiseSyncStruct
    {
    private:
        BitwiseSyncStruct( const BitwiseSyncStruct& other )
        { }

    public:
        typedef BitStreamer streamType;

        BitwiseSyncStruct( streamType& stream ) : m_stream( stream )
        {
        }

        ~BitwiseSyncStruct()
        {
            if ( m_instructs.empty() )
                return;

            m_stream.Wake();
            m_stream.WriteBits( m_instructs.size(), INT2BITS(defEntryCount) );

            for ( instList_t::const_iterator iter = m_instructs.begin(); iter != m_instructs.end(); iter++ )
            {
                (*iter)->Write( m_stream );
                delete *iter;
            }

            m_stream.Finalize();
        }

        class virtual_instruct
        {
        public:
            virtual_instruct( const unsigned int id ) : m_id( id )  {}
            virtual ~virtual_instruct() {}

            virtual NETWORK_NOEXCEPT void Write( streamType& out ) const = 0;

        protected:
            const unsigned int m_id;
        };

        template <class type>
        class instruct : public virtual_instruct
        {
        public:
            NETWORK_NOEXCEPT instruct( const unsigned int id, const type& t ) : virtual_instruct( id ), m_val( t )
            { }

            NETWORK_NOEXCEPT void Write( streamType& out ) const
            {
                out.WriteBits( m_id, INT2BITS( defEntryCount ) );
                out.Write <type> ( m_val );
            }

        private:
            type    m_val;
        };

        template <class type>
        inline NETWORK_NOEXCEPT const type& Acknowledge( const unsigned int id, const type& previous, const type& next )
        {
            if ( previous != next )
                m_instructs.push_back( new instruct <type> ( id, next ) );

            return next;
        }

    private:
        typedef std::vector <virtual_instruct*> instList_t;

        instList_t      m_instructs;
        streamType&     m_stream;
    };

    template <class Owner, size_t defStructSize, class BitStreamer = DefaultBitStreamer <defStructSize>, class DataStreamer = DefaultDataStreamer <defStructSize>, class Syncer = BitwiseSyncStruct <defStructSize, BitStreamer>>
    class NetworkSyncStruct : public NetworkStruct <Owner, defStructSize, DataStreamer, Syncer>
    {
    public:
        typedef BitStreamer streamType;
        typedef Syncer syncType;

        NetworkSyncStruct( const NetworkDataType *def ) : NetworkStruct( def )
        {
        }

        inline NETWORK_NOEXCEPT void Set( const Owner& instance )
        {
            NetworkStruct::Write( instance, NullSyncStruct() );
        }

        inline NETWORK_NOEXCEPT void Write( const Owner& instance, streamType& stream )
        {
            NetworkStruct::Write( instance, syncType( stream ) );
        }

        inline NETWORK_NOEXCEPT void Establish( Owner& instance, streamType& stream )
        {
            size_t chgcnt = stream.ReadBits( INT2BITS(defStructSize) );
            
            while ( chgcnt-- )
            {
                unsigned int id = stream.ReadBits( INT2BITS(defStructSize) );
                const NetworkDataType& type = m_def[id];

                switch( type.type )
                {
                case NETWORK_BOOL:      instance.NetworkWrite( id, stream.Read <bool> () ); break;
                case NETWORK_BYTE:      instance.NetworkWrite( id, stream.Read <char> () ); break;
                case NETWORK_WORD:      instance.NetworkWrite( id, stream.Read <short> () ); break;
                case NETWORK_DWORD:     instance.NetworkWrite( id, stream.Read <int> () ); break;
                case NETWORK_FLOAT:     instance.NetworkWrite( id, stream.Read <float> () ); break;
                case NETWORK_DOUBLE:    instance.NetworkWrite( id, stream.Read <double> () ); break;
                case NETWORK_VECTOR3D:  instance.NetworkWrite( id, stream.Read <CVector> () ); break;
                }
            }
        }
    };
}

#endif //_NETWORK_STRUCT_