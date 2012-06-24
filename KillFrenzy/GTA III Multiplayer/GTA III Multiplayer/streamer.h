#ifndef _STREAMER_
#define _STREAMER_

#define FUNC_AddEntityToWorld				0x004AEA20
#define FUNC_RemoveEntityFromWorld			0x004AEAC0

// Streamer Functions
void		Streamer_Init();
bool		Streamer_AddEntity(CEntity* pEntity);
void		Streamer_RemoveEntity(CEntity* pEntity);

#endif