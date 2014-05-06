#ifndef _COLLISION_REGISTRY_
#define _COLLISION_REGISTRY_

typedef struct
{
    union
    {
        char version[4];
        unsigned int checksum;
    };

    DWORD size;
    char name[22];
    unsigned short modelId;
} ColModelFileHeader;

struct Collision
{
                        Collision( void );
                        ~Collision( void );

    static Collision*   ReadFromFile( CFile *stream );

    void                WriteToFile( CFile *stream ) const;

    ColModelFileHeader  m_header;
    char*               m_data;
};

struct CollisionContainer
{
                        CollisionContainer( void );
                        ~CollisionContainer( void );

    static CollisionContainer*  ReadFromFile( const char *containerName, CFile *stream );

    const Collision*    GetCollisionByName( const char *name ) const;

    const char*         GetName( void ) const;

    const char*         m_name;

    typedef std::list <Collision*> collisions_t;
    collisions_t        m_collisions;
};

class CCollisionRegistry
{
public:
                                CCollisionRegistry( void );
                                ~CCollisionRegistry( void );

    bool                        AddContainer( CFileTranslator *sourceTranslator, const char *fileName );

    const CollisionContainer*   GetContainerByName( const char *name ) const;
    const Collision*            FindCollisionByModel( const char *model ) const;

protected:
    typedef std::list <CollisionContainer*> colContainers_t;
    colContainers_t             m_colContainers;
};

#endif //_COLLISION_REGISTRY_