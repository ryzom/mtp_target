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

#include <nel/misc/path.h>

#include "client.h"
#include "main.h"
#include "entity.h"
#include "physics.h"
#include "variables.h"
#include "entity_manager.h"
#include "session_manager.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;


//
// Functions
//

CClient::CClient(uint8 eid, NLNET::CTcpSock *sock) :
	CEntity(eid)
{
	nlassert(sock);
	Sock = sock;
	ReplayFile = 0;
	NetworkReady = false;
}

CClient::~CClient()
{
	if(ReplayFile)
	{
		nlassert(!ReplayFilename.empty());
		fclose(ReplayFile);
		CFile::deleteFile(ReplayFilename);
		ReplayFilename = "";
		ReplayFile = 0;
	}
}

bool CClient::openClose()
{
	if(ReplayFile)
	{
		float rsxTime = (CTime::getLocalTime() - CSessionManager::instance().startTime()) / 1000.0f;
		fprintf(ReplayFile, "%d OC %g\n", (uint16)id(), rsxTime);
	}
	return CEntity::openClose();
}

void CClient::display(CLog &log)
{
	log.displayNL("  C eid %hu name %-10s score %3d/%4d ping %4hu pos (%.2f,%.2f,%.2f) host %s",
				  (uint16)id(), name().c_str(), CurrentScore, Score, Ping.getSmoothValue(), Pos.x, Pos.y, Pos.z, sock()->remoteAddr().asString().c_str());
}

void CClient::setForce(const CVector &clientForce)
{
	if(ReplayFile)
	{
		float rsxTime = (CTime::getLocalTime() - CSessionManager::instance().startTime()) / 1000.0f ;
		//if(replayX!=x || replayY!=y || replayZ!=z)
		{
			fprintf(ReplayFile, "%d PO %.3f %.3f %.3f %.3f %.3f %.3f %.3f\n", id(), rsxTime, clientForce.x, clientForce.y, clientForce.z, Pos.x, Pos.y, Pos.z);
			Replay = clientForce;
		}
	}

	CEntity::setForce(clientForce);
}


//
// Commands
//