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

#include "physics.h"
#include "bot.h"
#include "main.h"
#include "network.h"
#include "variables.h"
#include "lua_engine.h"
#include "level_manager.h"
#include "entity_manager.h"
#include "session_manager.h"
#include "waiting_ready_session_state.h"
#include "waiting_clients_session_state.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


//
// Functions
//

void CWaitingClientsSessionState::update()
{
	CEntityManager::CEntities::CReadAccessor acces(CEntityManager::instance().entities());
	CEntityManager::EntityConstIt it;

	uint humanClientCount = 0;
	for(it = acces.value().begin(); it != acces.value().end(); it++)
	{
		if((*it)->type() == CEntity::Client)
		{
			humanClientCount++;
		}
		else if ((*it)->type() == CEntity::Bot)
		{
			CBot *b = (CBot *)(*it);
			if(!b->isAutomaticBot())
				humanClientCount++;
		}
	}
	if(humanClientCount >= NbWaitingClients)
	{
		// ok to launch a session
		changeState(CWaitingReadySessionState::instance());

		string string1, string2;
//ace		mtLevelManager::newLevel(CurrentLevel, string1, string2);

		CLevelManager::instance().newLevel();
		string CurrentLevel = CLevelManager::instance().currentLevel().fileName();

//ace		string luaCodeFilename = CPath::lookup(CurrentLevel+".lua", false);
//		CLuaEngine::loadLevelCode(luaCodeFilename);


		// reset everybody in startpos
		static uint st = 0;
		uint i=st++;
		for(it = acces.value().begin(); it != acces.value().end(); it++, i++)
		{
			//dGeomSetPosition((*it)->Geom, StartX+2.5*i, StartY, StartZ);
			//dBodySetPosition((*it)->Body, StartX+2.5*i, StartY, StartZ);
			
			CVector startPos;
			CLevelManager::instance().currentLevel().nextStartingPoint(startPos, (*it)->StartingPointId);

			//i = i % StartPointList.size();
			//(*it)->StartingPointId = i;
			//CVector startPos = StartPointList[i];

			dBodySetPosition((*it)->Body, startPos.x, startPos.y, startPos.z);
			dGeomSetPosition((*it)->Geom, startPos.x, startPos.y, startPos.z);
			
			dBodySetLinearVel((*it)->Body, 0.0f, 0.0f, 0.0f);
			dBodySetAngularVel((*it)->Body, 0.0f, 0.0f, 0.0f);
			(*it)->Force = CVector::Null;
			(*it)->OpenClose = false;
			(*it)->FreezeCommand = false;
			(*it)->NbOpenClose = 0;
			(*it)->InSession = true;
			(*it)->Ready = false;
			(*it)->Time = 0.0f;
			(*it)->StopedScore = 0;
			(*it)->PushScore = 0;
			(*it)->LastTouchedClientId = 255;
			(*it)->FirstStop = true;
			(*it)->OnTheWater = false;
			(*it)->spectator(false);

			for(uint i = 0; i < 10; i++)
				(*it)->LastVel[i] = 0;
			
			(*it)->LastVelPos = 0;

			// bot are always ready
			if((*it)->type() == CEntity::Bot)
			{
				(*it)->Ready = true;
			}
		}
		
		CNetMessage msgout(CNetMessage::StartSession);
		msgout.serial(TimeBeforeStart);
		msgout.serial(TimeTimeout);
		msgout.serial(CurrentLevel, string1, string2);
		//ace		msgout.serial(CurrentLevel, LevelName, string1, string2, Author);
		vector<uint8> ranks;
		vector<uint8> eids;
		for(it = acces.value().begin(); it != acces.value().end(); it++)
		{
			CEntity *e = *it;
			ranks.push_back(e->StartingPointId);
			eids.push_back(e->id());
			nlinfo("startpoint(%d) = %d",e->id(),e->StartingPointId);
		}
		msgout.serialCont(ranks);		
		msgout.serialCont(eids);		
		CNetwork::instance().send(msgout);
		
/*
		for(it = acces.value().begin(); it != acces.value().end(); it++)
		{
			CLuaEngine::clientInit(*it);
		}
*/
		
//		CollisionEntityListIt eit;
//		for(eit = CollisionEntityList.begin(); eit != CollisionEntityList.end(); eit++)
//		{
//			CCollisionEntity *entity = &(*eit);
//			CLuaEngine::collisionEntityInit(entity);
//		}

		dWorldSetGravity(World, 0.0f, 0.0f, 0.0f);
		CSessionManager::instance().forceEnding(false);
	}	
}