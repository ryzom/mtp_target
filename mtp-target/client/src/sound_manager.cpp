/* Copyright, 2003 Melting Pot
 *
 * This file is part of MTP Target.
 * MTP Target is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * MTP Target is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with MTP Target; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


//
// Includes
//

#include "stdpch.h"

#include "sound_manager.h"
#include "config_file_task.h"
#include "resource_manager.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;


//
// Variables
//

//
// Functions
//

#ifdef USE_FMOD
static FSOUND_SAMPLE *loadSoundSample(const std::string &filename)
{
	FSOUND_SAMPLE *sample = 0;
	string longName = CResourceManager::instance().get(filename);
	if(!longName.empty())
	{
		sample = FSOUND_Sample_Load(FSOUND_FREE, longName.c_str(), 0, 0);
		if (!sample)
		{
			nlwarning("CSoundManager: Can't load '%s' (%s)", longName.c_str(), FMOD_ErrorString(FSOUND_GetError()));
		}
	}
	return sample;
}
#endif

void CSoundManager::init()
{
#ifdef USE_FMOD
	streamStream = NULL;
	// check version
	if (FSOUND_GetVersion() < FMOD_VERSION)
	{
		nlwarning("CSoundManager: You are using the wrong fmod library version (at least version %g needed)", FMOD_VERSION);
		return;
	}
	// init fmod
	int frequency = CConfigFileTask::instance().configFile().getVar("SoundFrequency").asInt();
	if (! FSOUND_Init(frequency, 32, 0))
	{
		nlwarning("CSoundManager: Error while initializing FMOD (%s)", FMOD_ErrorString(FSOUND_GetError()));
		return;
	}
	
	memset(entitySoundSamples, 0, CSoundManager::CEntitySoundsDescriptor::SoundCount * sizeof(FSOUND_SAMPLE *));
	// load all samples
	entitySoundSamples[CSoundManager::CEntitySoundsDescriptor::BallOpen]  = loadSoundSample(CPath::lookup(CConfigFileTask::instance().configFile().getVar("SoundBallOpen").asString(),false));
	entitySoundSamples[CSoundManager::CEntitySoundsDescriptor::BallClose] = loadSoundSample(CPath::lookup(CConfigFileTask::instance().configFile().getVar("SoundBallClose").asString(),false));
	entitySoundSamples[CSoundManager::CEntitySoundsDescriptor::Splash]    = loadSoundSample(CPath::lookup(CConfigFileTask::instance().configFile().getVar("SoundSplash").asString(), false));
	
	// load all gui samples
	for (uint i = 0; i < CSoundManager::GuiSoundCount; i++)
		guiChannels[i] = -1;
	memset(guiSoundSamples, 0, CSoundManager::GuiSoundCount * sizeof(FSOUND_SAMPLE *));
	guiSoundSamples[CSoundManager::GuiReady] = loadSoundSample(CConfigFileTask::instance().configFile().getVar("SoundGuiReady").asString());
#endif

	if (CConfigFileTask::instance().configFile().getVar("Music").asInt() == 1)
	{
		string res = CResourceManager::instance().get("zik.mp3");
		if(res.empty())
		{
			nlwarning("Couldn't found zip.mp3");
		}
		else
		{
			playStream(res);
		}
	}
}

void CSoundManager::update()
{
#ifdef USE_FMOD
	// first update listener
	FSOUND_3D_Listener_SetAttributes(ListenerPosition, ListenerVelocity, ListenerAtVector.x, ListenerAtVector.y, ListenerAtVector.z, ListenerUpVector.x, ListenerUpVector.y, ListenerUpVector.z);
	// then sources
	TEntitySoundsDescriptorList::iterator first(entitySoundsDescriptorList.begin()), last(entitySoundsDescriptorList.end());
	for ( ; first != last; ++first)
	{
		CEntitySoundsDescriptor *esd = *first;
		for (uint i = 0; i < CEntitySoundsDescriptor::SoundCount; i++)
		{
			CEntitySoundsDescriptor::TSound &s = esd->Sounds[i];
			
			if (s.Command & CEntitySoundsDescriptor::CommandStop)
			{
				if (s.Channel != -1)
				{
					FSOUND_StopSound(s.Channel);
					s.Channel = -1;
				}
			}
			else if (s.Command & CEntitySoundsDescriptor::CommandPlay)
			{
				bool start = true;
				if (s.Channel != -1)
				{
					if (FSOUND_IsPlaying(s.Channel))
					{
						// already playing
						FSOUND_3D_SetAttributes(s.Channel, esd->Position, esd->Velocity);
						start = false;
					}
				}
				if (start)
				{
					// must play it
					if (s.Channel == -1 && entitySoundSamples[i] != 0)
					{
						// play it paused
						s.Channel = FSOUND_PlaySoundEx(FSOUND_FREE, entitySoundSamples[i], 0, true);
					}
					if (s.Channel != -1)
					{
						// playde ? ok, set attributes and unpause it
						FSOUND_3D_SetAttributes(s.Channel, esd->Position, esd->Velocity);
						FSOUND_SetPaused(s.Channel, false);
					}
				}
			}
			else if (s.Channel != -1)
			{
				if (! FSOUND_IsPlaying(s.Channel))
				{
					FSOUND_StopSound(s.Channel);
					s.Channel = -1;
				}
				else
				{
					FSOUND_3D_SetAttributes(s.Channel, esd->Position, esd->Velocity);
				}
			}
			s.Command = CEntitySoundsDescriptor::CommandNone;
		}
	}
	// and fmod
	FSOUND_Update();
#endif
}

void CSoundManager::updateListener(const NLMISC::CVector &position, const NLMISC::CVector &velocity, const NLMISC::CVector &atVector, const NLMISC::CVector &upVector)
{
#ifdef USE_FMOD
	// fmod is left handed
	ListenerPosition[0] = position.x;
	ListenerPosition[1] = position.y;
	ListenerPosition[2] = - position.z;
	
	ListenerVelocity[0] = velocity.x;
	ListenerVelocity[1] = velocity.y;
	ListenerVelocity[2] = - velocity.z;
	
	ListenerAtVector = atVector;
	ListenerAtVector.z = - ListenerAtVector.z;
	
	ListenerUpVector = upVector;
	ListenerUpVector.z = - ListenerUpVector.z;
#endif
}

void CSoundManager::render()
{
}

void CSoundManager::release()
{
#ifdef USE_FMOD
	// check if all entities have been unregistered
	nlassertex(entitySoundsDescriptorList.empty(), ("CSoundManager: %u entities already registered", entitySoundsDescriptorList.size()));
	TEntitySoundsDescriptorList::iterator first(entitySoundsDescriptorList.begin()), last(entitySoundsDescriptorList.end());
	for ( ; first != last; ++first)
	{
		(*first)->reset();
	}
	entitySoundsDescriptorList.clear();
	
	for (uint i = 0; i < CSoundManager::CEntitySoundsDescriptor::SoundCount; i++)
	{
		if (entitySoundSamples[i] != 0)
		{
			FSOUND_Sample_Free(entitySoundSamples[i]);
			entitySoundSamples[i] = 0;
		}
	}
	FSOUND_Close();
#endif
}

void CSoundManager::registerEntity(CEntitySoundsDescriptor &esd)
{
#ifdef USE_FMOD
	nlassert(std::find(entitySoundsDescriptorList.begin(), entitySoundsDescriptorList.end(), &esd) == entitySoundsDescriptorList.end());
	entitySoundsDescriptorList.push_back(&esd);
	esd.reset();
#endif
}

// -----------------------------------------------------------------------------
void CSoundManager::unregisterEntity(CEntitySoundsDescriptor &esd)
{
#ifdef USE_FMOD
	TEntitySoundsDescriptorList::iterator it = std::find(entitySoundsDescriptorList.begin(), entitySoundsDescriptorList.end(), &esd);
	if(it == entitySoundsDescriptorList.end())
		return;
	entitySoundsDescriptorList.erase(it);
	esd.reset();
#endif
}

// -----------------------------------------------------------------------------
CSoundManager::CEntitySoundsDescriptor::CEntitySoundsDescriptor()
{
#ifdef USE_FMOD
	for (uint i = 0; i < SoundCount; i++)
	{
		Sounds[i].Channel = -1;
		Sounds[i].Command = CommandNone;
	}
#endif
}

// -----------------------------------------------------------------------------
CSoundManager::CEntitySoundsDescriptor::~CEntitySoundsDescriptor()
{
#ifdef USE_FMOD
	reset();
#endif
}

// -----------------------------------------------------------------------------
void CSoundManager::CEntitySoundsDescriptor::play(TSoundId sound, bool loop)
{
#ifdef USE_FMOD
	Sounds[sound].Command |= loop ? CommandPlayLoop : CommandPlay;
#endif
}

// -----------------------------------------------------------------------------
void CSoundManager::CEntitySoundsDescriptor::stop(TSoundId sound)
{
#ifdef USE_FMOD
	Sounds[sound].Command |= CommandStop;
#endif
}

// -----------------------------------------------------------------------------
void CSoundManager::CEntitySoundsDescriptor::update3d(const NLMISC::CVector &pos, const NLMISC::CVector &vel)
{
#ifdef USE_FMOD
	// fmod is left handed
	Position[0] = pos.x;
	Position[1] = pos.y;
	Position[2] = - pos.z;
	Velocity[0] = vel.x;
	Velocity[1] = vel.y;
	Velocity[2] = - vel.z;	
#endif
}

// -----------------------------------------------------------------------------
void CSoundManager::CEntitySoundsDescriptor::reset()
{
#ifdef USE_FMOD
	for (uint i = 0; i < SoundCount; i++)
	{
		if (Sounds[i].Channel != -1)
		{
			FSOUND_StopSound(Sounds[i].Channel);
			Sounds[i].Channel = -1;
		}
		Sounds[i].Command = CommandNone;
	}
#endif
}

// -----------------------------------------------------------------------------
// gui methods
// -----------------------------------------------------------------------------
void CSoundManager::playGuiSound(TGuiSound sound, bool loop)
{
#ifdef USE_FMOD
	if (guiSoundSamples[sound] != 0)
	{
		if (guiChannels[sound] != -1)
		{
			if (FSOUND_IsPlaying(guiChannels[sound]))
			{
				//  already playing
			}
			else
			{
				if (! FSOUND_StopSound(guiChannels[sound]))
				{
					nlwarning("CSoundManager: can't stop a sound (%s)", FMOD_ErrorString(FSOUND_GetError()));
				}
				guiChannels[sound] = -1;
			}
		}
		if (guiChannels[sound] == -1)
		{
			guiChannels[sound] = FSOUND_PlaySoundEx(FSOUND_FREE, guiSoundSamples[sound], 0, loop);
			if (guiChannels[sound] == -1)
			{
				nlwarning("mSound: can't play a sound (%s)", FMOD_ErrorString(FSOUND_GetError()));
			}
			else
			{
				if (loop)
				{
					if (! FSOUND_SetLoopMode(guiChannels[sound], FSOUND_LOOP_NORMAL))
					{
						nlwarning("CSoundManager: can't set loop mode (%s)", FMOD_ErrorString(FSOUND_GetError()));
					}
					if (! FSOUND_SetPaused(guiChannels[sound], false))
					{
						nlwarning("CSoundManager: can't unpause a sound (%s)", FMOD_ErrorString(FSOUND_GetError()));
					}
				}
			}
		}
	}
#endif
}

// -----------------------------------------------------------------------------
void CSoundManager::stopGuiSound(TGuiSound sound)
{
#ifdef USE_FMOD
	if (guiChannels[sound] != -1)
	{
		if (! FSOUND_StopSound(guiChannels[sound]))
		{
			nlwarning("CSoundManager: can't stop a sound (%s)", FMOD_ErrorString(FSOUND_GetError()));
		}
		guiChannels[sound] = -1;
	}
#endif
}

// -----------------------------------------------------------------------------
// streams methods
// -----------------------------------------------------------------------------

#ifdef USE_FMOD
static signed char CSoundTaskStreamEndCallback(FSOUND_STREAM *stream, void *buff, int len, int param)
{
	if (buff == 0)
	{
		// if we are here, the stream is in loop mode : restart it
		if (! FSOUND_Stream_Stop(stream))
		{
			nlwarning("CSoundManager: Can't stop stream (%s)", FMOD_ErrorString(FSOUND_GetError()));
		}
		if (! FSOUND_Stream_Play(FSOUND_FREE, stream))
		{
			nlwarning("CSoundManager: Can't play stream (%s)", FMOD_ErrorString(FSOUND_GetError()));
		}
	}
	return true;
}
#endif

// -----------------------------------------------------------------------------
void CSoundManager::playStream(const std::string &filename, bool loop)
{
#ifdef USE_FMOD
	stopStream();
	std::string longName = NLMISC::CPath::lookup(filename, false);
	if (! longName.empty())
	{
		streamStream = FSOUND_Stream_OpenFile(longName.c_str(), FSOUND_NORMAL | FSOUND_LOOP_OFF, 0);
		if (streamStream == 0)
		{
			nlwarning("CSoundManager: Can't load stream '%s' (%s)", longName.c_str(), FMOD_ErrorString(FSOUND_GetError()));
		}
		else
		{
			/*
			if (loop)
			{
				if (! FSOUND_Stream_SetEndCallback(streamStream, CSoundTaskStreamEndCallback, 0))
				{
					nlwarning("CSoundManager: Can't set end stream callback (%s)", FMOD_ErrorString(FSOUND_GetError()));
				}
			}
			*/
			if (FSOUND_Stream_Play(FSOUND_FREE, streamStream) == -1)
			{
				nlwarning("CSoundManager: Can't play stream (%s)", FMOD_ErrorString(FSOUND_GetError()));
			}
		}
	}
#endif
}

// -----------------------------------------------------------------------------
void CSoundManager::stopStream()
{
#ifdef USE_FMOD
	if(streamStream != 0)
	{
		if (! FSOUND_Stream_Stop(streamStream))
		{
			nlwarning("CSoundManager: Can't stop stream (%s)", FMOD_ErrorString(FSOUND_GetError()));
		}
		if (! FSOUND_Stream_Close(streamStream))
		{
			nlwarning("CSoundManager: Can't close stream (%s)", FMOD_ErrorString(FSOUND_GetError()));
		}
		streamStream = 0;
	}
#endif
}