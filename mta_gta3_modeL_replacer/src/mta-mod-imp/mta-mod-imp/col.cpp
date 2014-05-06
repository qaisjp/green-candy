#include "StdInc.h"

Collision::Collision( void )
{
}

Collision::~Collision( void )
{
    delete m_data;
}

Collision* Collision::ReadFromFile( CFile *stream )
{
    ColModelFileHeader header;

    if ( !stream->ReadStruct( header ) )
        return NULL;

    if ( header.size <= 0x18u )
        return NULL;

    size_t dataSize = header.size - 0x18;

    // Validate if we can even read that much.
    size_t bytesAhead = stream->GetSize() - stream->Tell();

    if ( bytesAhead < dataSize )
        return NULL;

    char *data = new char[dataSize];

    bool success = ( stream->Read( data, 1, dataSize ) == dataSize );

    if ( success )
    {
        Collision *col = new Collision;
        
        col->m_header = header;
        col->m_data = data;

        return col;
    }

    delete data;

    return NULL;
}

void Collision::WriteToFile( CFile *stream ) const
{
    stream->WriteStruct( m_header );
    stream->Write( m_data, 1, m_header.size - 0x18 );
}

CollisionContainer::CollisionContainer( void )
{
}

CollisionContainer::~CollisionContainer( void )
{
    for ( collisions_t::const_iterator iter = m_collisions.begin(); iter != m_collisions.end(); iter++ )
    {
        Collision *colEntry = *iter;

        delete colEntry;
    }

    free( (void*)m_name );
}

CollisionContainer* CollisionContainer::ReadFromFile( const char *containerName, CFile *stream )
{
    CollisionContainer *container = NULL;

    Collision *colEntry = Collision::ReadFromFile( stream );

    if ( colEntry )
    {
        container = new CollisionContainer;

        container->m_name = strdup( containerName );

        while ( colEntry )
        {
            container->m_collisions.push_back( colEntry );

            colEntry = Collision::ReadFromFile( stream );
        }
    }

    return container;
}

const Collision* CollisionContainer::GetCollisionByName( const char *name ) const
{
    for ( collisions_t::const_iterator iter = m_collisions.begin(); iter != m_collisions.end(); iter++ )
    {
        const Collision *col = *iter;

        if ( CompareNames( col->m_header.name, name ) )
            return col;
    }

    return NULL;
}

CCollisionRegistry::CCollisionRegistry( void )
{
}

CCollisionRegistry::~CCollisionRegistry( void )
{
    for ( colContainers_t::const_iterator iter = m_colContainers.begin(); iter != m_colContainers.end(); iter++ )
    {
        CollisionContainer *colContainer = *iter;

        delete colContainer;
    }
}

bool CCollisionRegistry::AddContainer( CFileTranslator *sourceTranslator, const char *fileName )
{
    std::string containerName = FileSystem::GetFileNameItem( fileName );

    CFile *stream = sourceTranslator->Open( fileName, "rb" );

    if ( !stream )
        return false;

    CollisionContainer *colContainer = CollisionContainer::ReadFromFile( containerName.c_str(), stream );

    delete stream;

    m_colContainers.push_back( colContainer );

    return true;
}

const CollisionContainer* CCollisionRegistry::GetContainerByName( const char *name ) const
{
    for ( colContainers_t::const_iterator iter = m_colContainers.begin(); iter != m_colContainers.end(); iter++ )
    {
        const CollisionContainer *colContainer = *iter;

        if ( CompareNames( colContainer->m_name, name ) == 0 )
            return colContainer;
    }

    return NULL;
}

const Collision* CCollisionRegistry::FindCollisionByModel( const char *name ) const
{
    for ( colContainers_t::const_iterator iter = m_colContainers.begin(); iter != m_colContainers.end(); iter++ )
    {
        const CollisionContainer *colContainer = *iter;

        const Collision *colEntry = colContainer->GetCollisionByName( name );

        if ( colEntry )
            return colEntry;
    }

    return NULL;
}