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
// This class manages all client connections and network thread
//

#ifndef MTPT_NETWORK
#define MTPT_NETWORK


//
// Includes
//

#include "nel/misc/thread.h"

#include "../../common/net_message.h"


//
// Classes
//

class CNetwork : public CSingleton<CNetwork>
{
public:

	void init();
	void update();
	void release();

	void reset();

	void sleep(NLMISC::TTime timeout);
		
	// send a message to a specific client
	void send(uint8 eid, CNetMessage &msg, bool checkReady=false);

	// send a message to all connected clients
	void send(CNetMessage &msg);

	// send a message to all connected clients but not to eid
	void sendAllExcept(uint8 eid, CNetMessage &msg);

	// send a message to all connected clients
	void sendChat(const std::string &msg);
	
	// send a message to 1 client
	void sendChat(uint8 eid, const std::string &msg);

	uint32 version() { return Version; }

	const NLNET::CInetAddress& hostAddress( NLNET::TSockId hostid )
	{
		return BufServer->hostAddress(hostid);
	}


private:

	CNetwork();

	friend class CSingleton<CNetwork>;

	NLNET::CBufServer	*BufServer;
	uint32 updateCount;
	float MinDeltaToSendFullUpdate;
	bool  DisableNetworkOptimization;
	uint32 Version;

	NLMISC::TTime NextUpdateTime;
};

#endif
