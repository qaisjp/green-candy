#ifndef _ENTITY_
#define _ENTITY_

#define FUNC_CEntity_TeleportInstant		0x004B91F0
#define FUNC_CEntity_Teleport				0x004B9490
#define	MEM_CDamageManager_PedWasted		0x004EADA3

enum ePedTask
{
	PEDTASK_UNK,
	PEDTASK_UNK2,
	PEDTASK_UNK3,
	PEDTASK_UNK4,
	PEDTASK_UNK5,
	PEDTASK_UNK6,
	PEDTASK_UNK7,
	PEDTASK_UNK8,
	PEDTASK_UNK9,
	PEDTASK_UNK10,
	PEDTASK_UNK11,
	PEDTASK_UNK12,
	PEDTASK_UNK13,
	PEDTASK_UNK14,
	PEDTASK_ENTER_PASSENGER,
	PEDTASK_ENTER_DRIVER,
	NUM_PEDTASKS
};

static const NetworkDataType entityDef[] =
{
    { NETWORK_VECTOR3D, "pos" },
    { NETWORK_VECTOR3D, "velocity" }
};

enum eEntityDef
{
    ENTITYDEF_POS,
    ENTITYDEF_VELOCITY
};

class CEntity
{
public:
	virtual					~CEntity() = 0;

	void					SetPosition( float fPosX, float fPosY, float fPosZ );
	void					SetPosition( const vec3_t vecPos );
	void					GetPosition( vec3_t vecPos ) const;
	void					SetRotation( float fRotX, float fRotY, float fRotZ );
	void					SetRotation( const vec3_t vecRot );
	void					GetRotation( vec3_t vecRot ) const;
	void					Teleport( float fPosX, float fPosY, float fPosZ );
	void					RemoveControlEntity();
	void					RemovePassengerEntity( CEntity *pPassenger );

	CMatrix					m_matrix;				// +4
	CMatrix*				m_pPreviousMatrix;		// +68
	void*					m_pRwClump;				// +72
	BYTE					pad4[4];				// +76
	unsigned char			m_unkFlags;				// +80
	BYTE					pad6[11];				// +81
	unsigned short			m_usModelID;			// +92
	BYTE					pad11[26];				// +94
	vec3_t					m_vecVelocity;			// +139
	vec3_t					m_vecTurnVelocity;		// +151	
	vec3_t					m_vecUnknown1;			// +163
	vec3_t					m_vecUnknown2;			// +175
	vec3_t					m_vecUnknown3;			// +187
	vec3_t					m_vecUnknown4;			// +199
	BYTE					pad8[100];				// +163
	unsigned char			m_ucSceneID;			// +292
	unsigned char			pad9;					// +293
	BYTE					pad2[29];				// +294
	unsigned char			m_ucUnknown11;			// +323
	unsigned char			m_ucUnknown4;			// +324
	unsigned char			m_ucUnknown1;			// +325
	unsigned char			m_ucUnknown2;			// +326
	unsigned char			m_ucUnknown3;			// +327
	unsigned char			m_ucUnknown5;			// +328
	unsigned char			m_ucUnknown6;			// +329
	unsigned char			m_ucUnknown7;			// +330
	unsigned char			m_ucUnknown8;			// +331
	float					m_fUnknown9;			// +332
	unsigned char			m_ucUnknown10;			// +336
	BYTE					pad5[4];				// +337
	unsigned char			m_cEntityFlags;			// +341
	unsigned char			m_ucUnknown13;			// +342
	BYTE					pad12[3];				// +343
	unsigned char			m_ucUnknown14;			// +346
	BYTE					pad10[5];				// +347
	unsigned char			m_ucUnknown12;			// +352
	BYTE					pad18[3];				// +353
	unsigned int			m_uiTaskID;				// +356
	unsigned int			m_uiAdvancedTaskID;		// +360
	BYTE					pad19[4];				// +364
	CEntity*				m_pTaskEntity;			// +368
	BYTE					pad13[48];				// +372
	CEntity*				m_ppPassengers[9];		// +420
	unsigned char			m_ucNumPassengers;		// +456
	BYTE					pad16[3];				// +457
	unsigned char			m_ucMaxPassengers;		// +460
	BYTE					pad7[40];				// +461
	unsigned char			m_unkFlags2;			// +501
	BYTE					pad14[46];				// +502
	unsigned int			m_uiUnknownType;		// +548
	BYTE					pad15[92];				// +552
	unsigned int			m_uiPassengerFlag;		// +644
	BYTE					pad3[32];				// +648

    template <class type>
    inline void NetworkWrite( const unsigned char id, const type& val )
    {
        // void for now
    }

    template <>
    inline void NetworkWrite <CVector> ( const unsigned char id, const CVector& val )
    {
        switch( id )
        {
        case ENTITYDEF_POS:
            m_matrix.m_vecPos[0] = val.fX;
            m_matrix.m_vecPos[1] = val.fY;
            m_matrix.m_vecPos[2] = val.fZ;
            break;
        case ENTITYDEF_VELOCITY:
            m_vecVelocity[0] = val.fX;
            m_vecVelocity[1] = val.fY;
            m_vecVelocity[2] = val.fZ;
            break;
        }
    }

    template <class type>
    inline type NetworkRead( const unsigned char id ) const
    {
        return type();
    }

    template <>
    inline CVector NetworkRead( const unsigned char id ) const
    {
        switch( id )
        {
        case ENTITYDEF_POS:         return CVector( m_matrix.m_vecPos[0], m_matrix.m_vecPos[1], m_matrix.m_vecPos[2] );
        case ENTITYDEF_VELOCITY:    return CVector( m_vecVelocity[0], m_vecVelocity[1], m_vecVelocity[2] );
        }

        return CVector();
    }
};	// 680 Bytes size

typedef NetworkSyncStruct <CEntity, ETSIZE(entityDef)> entity_network;

void		Entity_Init();

#endif //_ENTITY_