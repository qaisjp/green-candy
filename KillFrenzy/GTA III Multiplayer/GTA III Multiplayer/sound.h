#ifndef _SOUND_
#define _SOUND_

#include "fmod/fmod.hpp"
#define	MAX_CACHED_SOUNDS							0xFF
#define FUNC_CRadioStreamer_PlaySound				0x005680D0
#define FUNC_CRadioStreamer_StopSound				0x00568430
#define FUNC_CRadioStreamer_GetSoundPosition		0x00568480
#define FUNC_CRadioStreamer_UpdateVolume			0x00568520
#define FUNC_CRadioStreamer_SetPaused				0x00568080
#define FUNC_CRadioStreamer_StartStream				0x005680B0
#define FUNC_CRadioStreamer_PlayDynamicStream		0x00567FA0
#define FUNC_CRadioStreamer_IsStreamRunning			0x005685E0
#define MEM_SoundNameBuffer							0x00602DA4
#define MEM_cSoundCheck								0x008F3A1A
#define FUNC_CSoundDriver_SetupSound				0x005679F0
#define FUNC_CSoundDriver_SetSoundPosition3D		0x00567BE0
#define FUNC_CSoundDriver_SetSoundEffectLevel		0x00567980
#define FUNC_CSoundDriver_SetSoundMinMaxDist		0x00567C20
#define FUNC_CSoundDriver_SetSoundSampleRate		0x00567D20
#define FUNC_CSoundDriver_SetSoundVolume			0x00567B70
#define FUNC_CSoundDriver_SetSoundVolume2D			0x00567C50
#define FUNC_CSoundDriver_SetSoundLoopBlockBytes	0x00567D80
#define FUNC_CSoundDriver_SetSoundLoopCount			0x00567DF0
#define FUNC_CSoundDriver_StartSound				0x00567ED0
#define FUNC_CSoundDriver_EndSound					0x00567F30
#define FUNC_CSoundDriver_IsSoundRunning			0x00567E50
#define FUNC_CSoundDriver_UpdateVolume				0x005672A0

class CSound
{
public:
					CSound(char *pFileName);
					~CSound();

	void			Play();
	void			Pause();
	void			SetStreamPosition(unsigned int uiSeek);
};

enum eSoundType
{
	SOUND_STREAM,
	SOUND_SAMPLE2D,
	SOUND_SAMPLE3D
};

enum eSoundBufferType
{
	SOUNDBUFFER_SFX,
	SOUNDBUFFER_VOICE,
	NUM_SOUNDBUFFER
};

typedef struct channelData_s
{
	bool				bActive;
	bool				isLoop;
	eSoundType			eSound;
	FMOD::Channel*		pChannel;
	FMOD::Sound*		pSound;
} channelData_t;

typedef struct coreSound_s
{
	bool bActive;
	unsigned char ucSoundID;
} coreSound_t;

enum eSoundChannel
{
	CHANNEL_RADIO,
	CHANNEL_FREE,
};

void		Sound_Dump();
void		Sound_Init();
void		Sound_Destroy();
FMOD::Sound*	Music_LoadFile(const char *path);
FMOD::Sound*	Sound_LoadFile(const char *path);
bool		Music_Play(FMOD::Sound* pSound, int channel, bool bLoop);
bool		Sound_Play2D(FMOD::Sound* pSound, int channel);
bool		Sound_PlayInternal2D(unsigned int uiID, int channel);
bool		Sound_PlayInternal3D(unsigned int uiID, int channel, float fPosX, float fPosY, float fPosZ);
bool		Sound_SetPaused(int iChannel, bool bPaused);
bool		Sound_Stop(int iChannel);
void		Sound_SetVolume(int iChannel, unsigned char ucVolume);
void		Sound_SetPositionMS(int iChannel, unsigned int ms);
bool		Sound_SetPosition3D(int iChannel, float fPosX, float fPosY, float fPosZ);
bool		Sound_SetSampleRate(int iChannel, unsigned int uiSampleRate);
bool		Sound_SetLoopCount(int iChannel, int iLoopCount);
bool		Sound_SetMinMaxDistance(int iChannel, float fMinDist, float fMaxDist);
unsigned int	Sound_GetPositionMS(int iChannel);
void		Sound_Frame();

#endif