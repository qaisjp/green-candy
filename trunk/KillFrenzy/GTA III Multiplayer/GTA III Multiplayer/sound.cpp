#include "StdInc.h"

using namespace FMOD;
System*		m_pSystem=0;
int			m_iChannels;
int			m_iMaxChannels2D;
int			m_iMaxChannels3D;
int			m_iChannels2D;
int			m_iChannels3D;
channelData_t	*m_channels;
Sound*		m_cachedSounds[MAX_CACHED_SOUNDS];
Sound**		m_soundEffects;
unsigned int	m_uiNumSFX;

unsigned int	m_uiSampleOffsets[NUM_SOUNDBUFFER] =
{
	0,
	459
};

#pragma pack(1)

struct soundEntry
{
	DWORD	dwStartPos;
	DWORD	dwSize;
	DWORD	dwSampleRate;
	DWORD	dwLoopStart;
	DWORD	dwLoopEnd;
};

#pragma pack()

static inline void Sound_CraftGenericWafeformHeader( char *buf, size_t sampleSize, unsigned int sampleRate )
{
	*(DWORD*)(buf) = 0x46464952;
	*(DWORD*)(buf+4) = sampleSize + 44-8;
	*(DWORD*)(buf+8) = 0x45564157;
	*(DWORD*)(buf+12) = 0x20746D66;
	*(DWORD*)(buf+16) = 0x10;
	*(WORD*)(buf+20) = 0x01;	//raw format
	*(WORD*)(buf+22) = 0x01;	//one channel
	*(DWORD*)(buf+24) = sampleRate;
	*(DWORD*)(buf+28) = sampleRate * 2;
	*(WORD*)(buf+32) = 0x02;	//block align
	*(WORD*)(buf+34) = 0x10;	//bits per sample
	*(DWORD*)(buf+36) = 0x61746164;
	*(DWORD*)(buf+40) = sampleSize;
}

// Does the sfx file logic and loads all sounds as .wavs
void		Sound_LoadSFX()
{
	CFile *descFile = fileRoot->Open("audio/sfx.sdt", "rb");
	CFile *rawFile = fileRoot->Open("audio/sfx.raw", "rb");
	char pathBuffer[1024];

	// What the fuck, shit happens
	if (!descFile || !rawFile)
	{
		Console_Printf("FMOD: Could not find sound files. Running without sound support.\n", 0xffffffff);
		return;
	}

	// We can actually calculate the array size to save sounds to
	m_uiNumSFX = descFile->GetSize() / sizeof(soundEntry);

	// Allocate the sound array
	m_soundEffects = (Sound**)malloc(sizeof(void*) * m_uiNumSFX);

	m_uiNumSFX = 0;	// We use it as a counter

	while ( !descFile->IsEOF() )
	{
		soundEntry entry;
		char *pData = NULL;
		Sound *pSFX;
		FMOD_CREATESOUNDEXINFO	fmodInfo;

		// Get the sound entry
		descFile->ReadStruct( entry );

		// Check replaceable sounds
		snprintf( pathBuffer, 1023, "sounds/%u.wav", m_uiNumSFX );

		// Fill in fmod struct
		memset(&fmodInfo, 0, sizeof(fmodInfo));
		fmodInfo.cbsize = sizeof(fmodInfo);

		if ( CFile *src = fileRoot->Open( pathBuffer, "rb+" ) )
		{
			// Grab the sound file
			fmodInfo.length = src->GetSize();
			pData = (char*)malloc( fmodInfo.length );

			src->Read( pData, 1, fmodInfo.length );

			delete src;
		}
		else
		{
			// Update it using GTA's data
			fmodInfo.length = entry.dwSize + 44;

			// All right, we actually do it the good way, kk
			rawFile->Seek( entry.dwStartPos, SEEK_SET );
			pData = (char*)malloc( entry.dwSize + 44 );

			// Craft the waveform header
			Sound_CraftGenericWafeformHeader( pData, entry.dwSize, entry.dwSampleRate );

			// Now append the data
			rawFile->Read( pData+44, 1, entry.dwSize );
		}

		// Load the sound in FMOD
		if (m_pSystem->createSound( pData, FMOD_SOFTWARE | FMOD_OPENMEMORY | FMOD_LOOP_NORMAL, &fmodInfo, &pSFX ) != FMOD_OK)
		{
			MessageBox(NULL, "Failed to load FMOD SFX. Please check whether your GTAIII installation is valid!", "FMOD Sound Error", MB_OK);
			TerminateProcess( GetCurrentProcess(), EXIT_FAILURE );
		}

		free(pData);

		// Set loop points
		if (pSFX->setLoopPoints( entry.dwLoopStart, FMOD_TIMEUNIT_PCMBYTES, entry.dwLoopEnd, FMOD_TIMEUNIT_PCMBYTES ) != FMOD_OK)
		{
			MessageBox(NULL, "Failed to set FMOD SFX loop points. Please check whether your GTAIII installation is valid!", "FMOD_Sound Error", MB_OK);
			TerminateProcess( GetCurrentProcess(), EXIT_FAILURE );
		}

		// Put it into our sfx array
		m_soundEffects[m_uiNumSFX] = pSFX;
		m_uiNumSFX++;
	}
	
	delete descFile;
	delete rawFile;

	Console_Printf("Successfully cached all sound effects (%i)\n", 0xffffffff, m_uiNumSFX);
}

void		Sound_Dump()
{
	CFile *descFile = fileRoot->Open("audio/sfx.sdt", "rb");
	CFile *rawFile = fileRoot->Open("audio/sfx.raw", "rb");

	if ( !descFile || !rawFile )
		return;

	CFile *zipFile = fileRoot->Open( "audio/dump.zip", "wb" );

	if ( !zipFile )
	{
		delete descFile;
		delete rawFile;

		return;
	}

	CArchiveFileTranslator *zip = (CArchiveFileTranslator*)fileSystem->CreateZIPArchive( *zipFile );
	unsigned int n = 0;

	while ( !descFile->IsEOF() )
	{
		soundEntry entry;
		char header[44];
		char pathBuffer[1024];

		descFile->ReadStruct( entry );

		Sound_CraftGenericWafeformHeader( header, entry.dwSize, entry.dwSampleRate );

		snprintf( pathBuffer, 1023, "%u.wav", n );

		CFile *dst = zip->Open( pathBuffer, "wb" );

		dst->WriteStruct( header );

		FileSystem::StreamCopyCount( *rawFile, *dst, entry.dwSize );

		delete dst;

		n++;
	}

	zip->Save();

	delete zip;
	delete zipFile;
	
	delete descFile;
	delete rawFile;
}

FILE*		Hook_SoundSystemLoaded ( char *pFileName, char *pMode )
{
	// Cache stuff
	char *pGameDrive = m_pGame->GetGameDrive();
	char lastDir[MAX_PATH];
	unsigned int n;
	int iNumDrivers;
	char driverName[256];
	FMOD_CAPS iCaps;
	GetCurrentDirectory (MAX_PATH, lastDir);

	// We have to init here
	m_pSystem->getNumDrivers(&iNumDrivers);
	m_pSystem->getDriverInfo(0, driverName, 256, NULL);
	Console_Printf("Selecting '%s' for FMOD output\n", 0xffffffff, driverName);

	m_pSystem->setDriver(0);
	m_pSystem->getDriverCaps(0, &iCaps, NULL, NULL, NULL);

	if (!(iCaps & FMOD_CAPS_HARDWARE))
	{
		//Console_Printf("Your SFX device does not appear to support hardware sound. Performance may drop.\n", 0xffffffff);
		m_pSystem->init(64, FMOD_INIT_DSOUND_HRTFFULL, NULL);
		m_iMaxChannels3D=64;
		m_iMaxChannels2D=64;
		m_iChannels=64;
	}
	else
	{
		m_pSystem->getHardwareChannels(&m_iMaxChannels2D, &m_iMaxChannels3D, &m_iChannels);
		m_pSystem->init(m_iChannels, FMOD_INIT_DSOUND_HRTFFULL, NULL);
	}
	m_channels = (channelData_t*)malloc(sizeof(channelData_t) * m_iChannels);
	memset(m_channels, 0, sizeof(channelData_t) * m_iChannels);

	SetCurrentDirectory(pGameDrive);

	// Scan and precache needed audio streams
	for (n=0; n<196; n++)
	{
		Sound *pSound = Music_LoadFile((char*)MEM_SoundNameBuffer + 25*n);
		m_cachedSounds[n] = pSound;
	}
	SetCurrentDirectory(lastDir);

	// Now we have to load the sfx, which is the most complicated part
	Sound_LoadSFX();

	// Never give valid file
	return NULL;
}

// Core attempts to play sounds, music
bool	__stdcall	Hook_PlayRadioSound(int iSound, int iUnk, int iSoundID)
{
	// Play a sound from the buffer
	unsigned char cSound=iSound&0xFF;
	Sound* pSound;

	if (cSound >= 196)
		return false;

	// Start teh sound
	pSound = m_cachedSounds[cSound];

	if (!Music_Play(pSound, iSoundID, false))
		return false;

	Sound_SetPositionMS(iSoundID, GetTickCount());
	//__asm mov ecx,0x007341E0
	return true;
}

// Core attempts to play a dynamic music track
bool	__stdcall	Hook_PlayDynamicMusic(int iChannelID, char cSound)
{
	if (cSound<196)
	{
		return Music_Play(m_cachedSounds[cSound], iChannelID, false);
	}
	return false;
}

// Core attempts to stop sound
void	__stdcall	Hook_StopRadioSound(int iSoundID)
{
	channelData_t *pData;

	if (iSoundID<0 || iSoundID>m_iChannels)
		return;

	pData = &m_channels[iSoundID];

	if (pData->eSound != SOUND_STREAM)
		return;

	Sound_Stop(iSoundID);
	//__asm mov ecx,0x007341E0
}

// Set volume
void	__stdcall	Hook_SetRadioVolume(int iUnk, unsigned char ucVolume, bool doSpecialCalc, int iSoundID)
{
	unsigned int ucVol=ucVolume;
	//unsigned char ucVolMul;

	if (iSoundID<0 || iSoundID>m_iChannels)
		return;

	if (doSpecialCalc)
	{
		DWORD dwMem = 0x007341E0;

		__asm
		{
			movzx eax,byte ptr [dwMem]
			movzx ebx,iUnk
			movzx edx,byte ptr [dwMem+2]
			imul eax,ebx
			imul eax,edx
			sar eax,0x0E
			mov ucVol,eax
		}
	}

	Sound_SetVolume(iSoundID, ucVol);
	__asm mov ecx,0x007341E0
}

unsigned int	__stdcall	Hook_GetRadioSoundPosition(int iSoundID)
{
	return Sound_GetPositionMS(iSoundID);
}

void	__stdcall		Hook_SetRadioSoundPaused(int iSoundID, bool bPaused)
{
	Sound_SetPaused(iSoundID, bPaused);
}

void	__stdcall		Hook_StartRadioStream(int iSoundID)
{
	Sound_SetPaused(iSoundID, false);
}

bool	__stdcall		Hook_IsStreamRunning(int iSoundID)
{
	if (iSoundID<0 || iSoundID>m_iChannels)
		return false;

	return m_channels[iSoundID].bActive;
}

// Core attempts to set up a channel sound
bool	__stdcall		Hook_SetupSoundEffect( int iChannel, int iSound, char cBufferID )
{
	iChannel += CHANNEL_FREE;

	if ( cBufferID > NUM_SOUNDBUFFER )
		return false;

	// Stop any possible sound on that channel
	Sound_Stop( iChannel );

	// Attach our sound to the channel, paused
	if ( iChannel < 28 + CHANNEL_FREE )
		Sound_PlayInternal3D( iSound, iChannel, 0, 0, 0 );
	else
		Sound_PlayInternal2D( iSound, iChannel );

	Sound_SetPaused( iChannel, true );
	return true;
}

// Set the sound position
bool	__stdcall		Hook_SetSoundPosition3D(int iChannel, float fPosX, float fPosY, float fPosZ)
{
	iChannel+=CHANNEL_FREE;
	if (iChannel<0 || iChannel>m_iChannels)
		return false;

	Sound_SetPosition3D(iChannel, fPosX, fPosY, fPosZ);
	return true;
}

// Attempt to change playback rate
bool	__stdcall		Hook_SetSoundPlaybackRate(int iChannel, int iSampleRate)
{
	iChannel+=CHANNEL_FREE;
	return Sound_SetSampleRate(iChannel, iSampleRate);
}

// Attempt to change volume
bool	__stdcall		Hook_SetSoundVolume(int iChannel, int iVolume)
{
	if (iVolume>0x7F)
		iVolume=0x7F;

	iChannel+=CHANNEL_FREE;
	__asm
	{
		mov ebx,iVolume
		movzx edx,ebx
		mov ecx,0x007341E0
		movzx eax,[ecx]
		movzx eax,[ecx+2]
		imul ebx,edx
		imul ebx,eax
		sar ebx,0x0E
		mov iVolume,ebx
	}
	Sound_SetVolume(iChannel, iVolume);
	return true;
}

// Attempt to change sound effect level
bool	__stdcall		Hook_SetSoundEffectLevel(int iChannel, int iEffectLevel)
{
	iChannel+=CHANNEL_FREE;
	return true; // Fuck that
}

bool	__stdcall		Hook_SetSoundMinMaxDist(int iChannel, float fMinDist, float fMaxDist)
{
	iChannel+=CHANNEL_FREE;
	return Sound_SetMinMaxDistance(iChannel, fMinDist, fMaxDist);
}

bool	__stdcall		Hook_SetSoundLoopBlockBytes(int iChannel, unsigned int uiLoopStart, unsigned int uiLoopEnd)
{
	iChannel+=CHANNEL_FREE;
	return true;
}

bool	__stdcall		Hook_SetSoundLoopCount(int iChannel, int iLoopCount)
{
	iChannel+=CHANNEL_FREE;
	iLoopCount--;
	return Sound_SetLoopCount(iChannel, iLoopCount);
}

bool	__stdcall		Hook_StartSound(int iChannel)
{
	iChannel+=CHANNEL_FREE;
	Sound_SetPaused(iChannel, false);
	return true;
}

bool	__stdcall		Hook_IsSoundRunning(int iChannel)
{
	iChannel+=CHANNEL_FREE;
	if (iChannel<0 || iChannel>m_iChannels)
		return false;
	return m_channels[iChannel].bActive;
}

bool	__stdcall		Hook_UpdateSoundVolume()
{
	//int n;

	/*for (n=0; n<32; n++)
	{
		int iVolume = Sound_GetVolume();

		__asm
		{
			mov ebx,iVolume
			movzx edx,ebx
			mov ecx,0x007341E0
			movzx eax,[ecx]
			movzx eax,[ecx+2]
			imul ebx,edx
			imul ebx,eax
			sar ebx,0x0E
			mov iVolume,ebx
		}
		Sound_SetVolume(n, 128);
	}*/
	return true;
}

bool	__stdcall		Hook_EndSound(int iChannel)
{
	iChannel+=CHANNEL_FREE;
	Sound_Stop(iChannel);
	return false;
}

// Initialize sound hooks and dependencies
void		Sound_Init()
{
	unsigned int fmodVersion;

	if (System_Create(&m_pSystem) != FMOD_OK)
	{
		MessageBox(NULL, "Could not initialize FMOD system", "FMOD Error", MB_OK);
		TerminateProcess( GetCurrentProcess(), EXIT_FAILURE );
	}

	m_pSystem->getVersion(&fmodVersion);
	if (fmodVersion != FMOD_VERSION)
	{
		MessageBox(NULL, "Invalid FMOD version!", "FMOD Error", MB_OK);
		TerminateProcess( GetCurrentProcess(), EXIT_FAILURE );
	}	
	m_iChannels2D=0;
	m_iChannels3D=0;

	// Install music and sound hooks
	*(DWORD*)(0x00566CF8) =  (int)Hook_SoundSystemLoaded - 0x00566CFC;
	DetourFunction((PBYTE)FUNC_CRadioStreamer_PlaySound, (PBYTE)Hook_PlayRadioSound);
	DetourFunction((PBYTE)FUNC_CRadioStreamer_StopSound, (PBYTE)Hook_StopRadioSound);
	DetourFunction((PBYTE)FUNC_CRadioStreamer_UpdateVolume, (PBYTE)Hook_SetRadioVolume);
	DetourFunction((PBYTE)FUNC_CRadioStreamer_GetSoundPosition, (PBYTE)Hook_GetRadioSoundPosition);
	DetourFunction((PBYTE)FUNC_CRadioStreamer_SetPaused, (PBYTE)Hook_SetRadioSoundPaused);
	DetourFunction((PBYTE)FUNC_CRadioStreamer_PlayDynamicStream, (PBYTE)Hook_PlayDynamicMusic);
	DetourFunction((PBYTE)FUNC_CRadioStreamer_StartStream, (PBYTE)Hook_StartRadioStream);
	DetourFunction((PBYTE)FUNC_CRadioStreamer_IsStreamRunning, (PBYTE)Hook_IsStreamRunning);

	// Sound stuff
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetupSound, (PBYTE)Hook_SetupSoundEffect);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundPosition3D, (PBYTE)Hook_SetSoundPosition3D);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundVolume, (PBYTE)Hook_SetSoundVolume);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundVolume2D, (PBYTE)Hook_SetSoundVolume);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundSampleRate, (PBYTE)Hook_SetSoundPlaybackRate);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundMinMaxDist, (PBYTE)Hook_SetSoundMinMaxDist);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundEffectLevel, (PBYTE)Hook_SetSoundEffectLevel);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundLoopBlockBytes, (PBYTE)Hook_SetSoundLoopBlockBytes);
	DetourFunction((PBYTE)FUNC_CSoundDriver_SetSoundLoopCount, (PBYTE)Hook_SetSoundLoopCount);
	DetourFunction((PBYTE)FUNC_CSoundDriver_StartSound, (PBYTE)Hook_StartSound);
	DetourFunction((PBYTE)FUNC_CSoundDriver_IsSoundRunning, (PBYTE)Hook_IsSoundRunning);
	DetourFunction((PBYTE)FUNC_CSoundDriver_UpdateVolume, (PBYTE)Hook_UpdateSoundVolume);
	DetourFunction((PBYTE)FUNC_CSoundDriver_EndSound, (PBYTE)Hook_EndSound);
}

// Returns a free sound channel
channelData_t*	Sound_GetFreeChannel ()
{
	int n;

	for (n=CHANNEL_FREE; n<m_iChannels; n++)
	{
		channelData_t *pData = &m_channels[n];

		if (!pData->bActive)
			return pData;
	}
	return NULL;
}

// Load music stream
Sound*		Music_LoadFile(const char *pFileName)
{
	Sound *pSound;

	if (m_pSystem->createStream(pFileName, FMOD_HARDWARE, NULL, &pSound) != FMOD_OK)
		return NULL;

	return pSound;
}

// Load sound sample
Sound*		Sound_LoadFile(const char *pFileName)
{
	Sound *pSound;

	if (m_pSystem->createSound(pFileName, FMOD_SOFTWARE, NULL, &pSound) != FMOD_OK)
		return NULL;

	return pSound;
}

// Play music
bool		Music_Play(Sound* pSound, int channel, bool bLoop)
{
	Channel *pChannel;
	channelData_t *pData=&m_channels[channel];
	FMOD_MODE fmodMode;

	if (pData->bActive)
	{
		// If its the same music, we dont do anything
		//if (pData->pSound == pSound)
		//	return true;
		// We have to clean running sounds or music
		Sound_Stop(channel);
	}
	else
	{
		if (m_iChannels2D == m_iMaxChannels2D)
			return false;
	}

	if (m_pSystem->playSound((FMOD_CHANNELINDEX)channel, pSound, false, &pChannel) != FMOD_OK)
		return false;

	pChannel->getMode(&fmodMode);
	fmodMode |= FMOD_2D;
	pChannel->setMode(fmodMode);
	pData->isLoop=bLoop;
	pData->pChannel=pChannel;
	pData->pSound=pSound;
	pData->eSound=SOUND_STREAM;
	pData->bActive=true;
	m_iChannels2D++;
	return true;
}

// Play sound 2D
bool		Sound_Play2D(Sound* pSound, int channel)
{
	Channel *pChannel;
	channelData_t *pData=&m_channels[channel];
	FMOD_MODE fmodMode;

	if (pData->bActive)
	{
		// If its the same music, we dont do anything
		//if (pData->pSound == pSound)
		//	return true;
		// We have to clean running sounds or music
		Sound_Stop(channel);
	}
	else
	{
		if (m_iChannels2D == m_iMaxChannels2D)
			return false;
	}

	if (m_pSystem->playSound((FMOD_CHANNELINDEX)channel, pSound, false, &pChannel) != FMOD_OK)
		return false;

	pChannel->getMode(&fmodMode);
	fmodMode |= FMOD_2D;
	pChannel->setMode(fmodMode);
	pData->pChannel=pChannel;
	pData->pSound=pSound;
	pData->eSound=SOUND_SAMPLE2D;
	pData->bActive=true;
	m_iChannels2D++;
	return true;
}

// Play a sound 2D, internal one
bool		Sound_PlayInternal2D(unsigned int uiID, int channel)
{
	Channel *pChannel;
	channelData_t *pData=&m_channels[channel];
	FMOD_MODE fmodMode;
	Sound *pSound;

	if (uiID >= m_uiNumSFX)
		return false;
	pSound = m_soundEffects[uiID];

	if (pData->bActive)
	{
		// If its the same music, we dont do anything
		//if (pData->pSound == pSound)
		//	return true;
		// We have to clean running sounds or music
		Sound_Stop(channel);
	}
	else
	{
		if (m_iChannels2D == m_iMaxChannels2D)
			return false;
	}

	if (m_pSystem->playSound((FMOD_CHANNELINDEX)channel, pSound, false, &pChannel) != FMOD_OK)
		return false;

	pChannel->getMode(&fmodMode);
	fmodMode |= FMOD_2D;
	pChannel->setMode(fmodMode);
	pData->pChannel=pChannel;
	pData->pSound=pSound;
	pData->eSound=SOUND_SAMPLE2D;
	pData->bActive=true;
	m_iChannels2D++;
	return true;
}

// Play a sound 3D, internal one
bool		Sound_PlayInternal3D(unsigned int uiID, int channel, float fPosX, float fPosY, float fPosZ)
{
	Channel *pChannel;
	channelData_t *pData=&m_channels[channel];
	FMOD_MODE fmodMode;
	Sound *pSound;

	if (uiID >= m_uiNumSFX)
		return false;
	pSound = m_soundEffects[uiID];

	if (pData->bActive)
	{
		// If its the same music, we dont do anything
		//if (pData->pSound == pSound)
		//	return true;
		// We have to clean running sounds or music
		Sound_Stop(channel);
	}
	else
	{
		if (m_iChannels3D == m_iMaxChannels3D)
			return false;
	}

	if (m_pSystem->playSound((FMOD_CHANNELINDEX)channel, pSound, false, &pChannel) != FMOD_OK)
		return false;

	pChannel->getMode(&fmodMode);
	fmodMode |= FMOD_3D;
	fmodMode |= FMOD_LOOP_NORMAL;
	pChannel->setMode(fmodMode);
	pData->pChannel=pChannel;
	pData->pSound=pSound;
	pData->eSound=SOUND_SAMPLE3D;
	pData->bActive=true;
	m_iChannels3D++;

	Sound_SetPosition3D(channel, fPosX, fPosY, fPosZ);
	return true;
}

// UnPause a channel
bool		Sound_SetPaused(int iChannel, bool bPaused)
{
	channelData_t *pData;

	if (iChannel<0 || iChannel>m_iChannels)
		return false;
	pData=&m_channels[iChannel];

	if (!pData->bActive)
		return false;

	pData->pChannel->setPaused(bPaused);
	return true;
}

// Stop a channel
bool		Sound_Stop(int iChannel)
{
	channelData_t *pData;

	if (iChannel<0 || iChannel>m_iChannels)
		return false;
	pData = &m_channels[iChannel];

	if (!pData->bActive)
		return false;

	/*pData->pChannel->isPlaying(&bPlaying);
	if (bPlaying)
		return false;*/

	if (pData->pChannel->stop() != FMOD_OK)
		return false;

	switch(pData->eSound)
	{
	case SOUND_STREAM:
	case SOUND_SAMPLE2D:
		m_iChannels2D--;
		break;
	case SOUND_SAMPLE3D:
		m_iChannels3D--;
		break;
	}

	pData->bActive=false;
	return true;
}

// Repositioning
bool		Sound_SetPosition3D(int iChannel, float fPosX, float fPosY, float fPosZ)
{
	channelData_t *pData;
	FMOD_VECTOR fmodVec;

	if (iChannel<0 || iChannel>m_iChannels)
		return false;
	pData=&m_channels[iChannel];
	
	fmodVec.x=fPosX;
	fmodVec.y=fPosY;
	fmodVec.z=fPosZ;
	return pData->pChannel->set3DAttributes(&fmodVec, NULL) == FMOD_OK;
}

bool		Sound_SetSampleRate(int iChannel, unsigned int uiSampleRate)
{
	channelData_t *pData;
	
	if (iChannel<0 || iChannel>m_iChannels)
		return false;
	pData=&m_channels[iChannel];

	return pData->pChannel->setFrequency((float)uiSampleRate) == FMOD_OK;
}

bool		Sound_SetLoopCount(int iChannel, int iLoopCount)
{
	if (iChannel<0 || iChannel>m_iChannels)
		return false;
	if (m_channels[iChannel].pChannel->setLoopCount(iLoopCount) != FMOD_OK)
		return false;
	return true;
}

bool		Sound_SetMinMaxDistance(int iChannel, float fMinDist, float fMaxDist)
{
	channelData_t *pData;

	if (iChannel<0 || iChannel>m_iChannels)
		return false;
	pData=&m_channels[iChannel];

	if (pData->pChannel->set3DMinMaxDistance(fMinDist, fMaxDist) != FMOD_OK)
		return false;
	return true;
}

// Set the volume
void		Sound_SetVolume(int iChannel, unsigned char ucVolume)
{
	channelData_t *pData;

	if (iChannel<0 || iChannel>m_iChannels)
		return;
	pData = &m_channels[iChannel];

	if (!pData->bActive)
		return;

	pData->pChannel->setVolume((float)ucVolume/255);
}

void		Sound_SetPositionMS(int iChannel, unsigned int ms)
{
	channelData_t *pData;
	unsigned int uiLength;

	if (iChannel<0 || iChannel>m_iChannels)
		return;
	pData = &m_channels[iChannel];

	if (!pData->bActive)
		return;

	pData->pSound->getLength(&uiLength, FMOD_TIMEUNIT_MS);
	pData->pChannel->setPosition(ms % uiLength, FMOD_TIMEUNIT_MS);
}

unsigned int	Sound_GetPositionMS(int iChannel)
{
	channelData_t *pData;
	unsigned int uiLength;

	if (iChannel<0 || iChannel>m_iChannels)
		return 0;
	pData = &m_channels[iChannel];

	if (!pData->bActive)
		return 0;

	pData->pSound->getLength(&uiLength, FMOD_TIMEUNIT_MS);
	return uiLength;
}

// Updates sounds
void		Sound_Frame()
{
	int n;

	// First, update FMOD
	m_pSystem->update();

	// Now check sounds
	for (n=0; n<m_iChannels; n++)
	{
		channelData_t* pChannelData = &m_channels[n];
		Channel *pChannel=pChannelData->pChannel;
		bool bPlaying;

		if (!pChannelData->bActive)
			continue;

		// Ended playing?
		pChannel->isPlaying(&bPlaying);
		if (!bPlaying)
		{
			switch(pChannelData->eSound)
			{
			case SOUND_STREAM:
				// Hardware accelerated sounds need custom loophandling
				if (!pChannelData->isLoop)
				{
					m_channels[n].bActive=false;
				}
				else
				{
					// Recreate, todo: multi sound type support
					Music_Play(pChannelData->pSound, n, true);
				}
				m_iChannels2D--;
				break;
			case SOUND_SAMPLE2D:
				// Never loop sounds, its stupid shit
				m_channels[n].bActive=false;
				m_iChannels2D--;
				break;
			case SOUND_SAMPLE3D:
				m_channels[n].bActive=false;
				m_iChannels3D--;
				break;
			}

			continue;
		}

		//todo: Sound handling
	}
}

// Destroy stuff
void		Sound_Destroy()
{
	m_pSystem->close();
	m_pSystem->release();
}