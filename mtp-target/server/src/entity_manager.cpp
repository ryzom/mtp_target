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
#include "time.h"

#include <nel/net/service.h>

#include "bot.h"
#include "main.h"
#include "entity.h"
#include "client.h"
#include "command.h"
#include "network.h"
#include "welcome.h"
#include "variables.h"
#include "lua_engine.h"
#include "level_manager.h"
#include "entity_manager.h"
#include "session_manager.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Types
//


//
// Declarations
//



//
// Variables
//

static uint8 WatchingId = 0;

CVariable<bool> SavePingStat("variables","SavePingStat", "1 if you want to save the ping stat", false, 0, true);


//
// Functions
//

void CEntityManager::init()
{
	IdUpdateList.clear();
}

void CEntityManager::update()
{
	checkForcedClientCount();
	{
		EntityConstIt it;
		for(it = entities().begin(); it != entities().end(); it++)
		{
			(*it)->update();
		}
	}
}

void CEntityManager::add(CEntity *entity)
{
	nlinfo("CEntityManager::add(0x%p,%d:%s)",entity,entity->id(),entity->name().c_str());
	entities().push_back(entity);
}

void CEntityManager::remove(uint8 eid)
{
	nlinfo("CEntityManager::remove(%d)",eid);
	CEntity *c = 0;
	
	{
		EntityIt it;
		for( it = entities().begin(); it != entities().end(); it++)
		{
			if((*it)->id() == eid)
			{
				// only unlink the client from the list
				c = (*it);
				entities().erase(it);
				break;
			}
		}
	}

	if(!c)
	{
		nlwarning("Can't remove client because eid %hu is not found", (uint16)eid);
	}
	else
	{
		nlinfo("Removing client eid %hu name '%s'", (uint16)c->id(), c->name().c_str());
		CLuaEngine::instance().entityLeaveEvent(c);
		CSessionManager::instance().editMode(0);

		if(c->type() == CEntity::Client)
		{
			char d[80];
			time_t ltime;
			time(&ltime);
			struct tm *today = localtime(&ltime);
			strftime(d, 80, "%Y %m %d %H %M %S", today);
			FILE *fp = fopen("connection.stat", "ab");
			if(fp)
			{
				fprintf(fp, "%u %s - '%s' '%s'\n", ltime, d, c->name().c_str(),CLevelManager::instance().haveCurrentLevel()?CLevelManager::instance().currentLevel().name().c_str():"");
				fclose(fp);
			}
			fp = fopen("clients_count.stat", "ab");
			if(fp)
			{
				fprintf(fp, "%u %s c %d\n", ltime, d, humanClientCount());
				fclose(fp);
			}

			clientConnected(dynamic_cast<CClient*>(c)->Cookie, false);
		}

		if(!c->name().empty())
		{
			CNetMessage msgout(CNetMessage::Logout);
			msgout.serial(eid);
			CNetwork::instance().send(msgout);		
			
			if(c->type() == CEntity::Client)
			{
				CNetwork::instance().sendChat(c->name()+" leaves !");
			}
		}

		delete c;
	}
}

void CEntityManager::reset()
{
	if(SavePingStat)
	{
		TTime current = CTime::getLocalTime();
		for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
		{
			if((*it)->type() == CEntity::Client)
			{
				CClient *c = (CClient *)(*it);
				string name = toString("ping_state_%"NL_I64"u_%s_%s.csv", current, CLevelManager::instance().levelFilename().c_str(), c->name().c_str());
				FILE *fp = fopen(name.c_str(), "wb");
				if(fp)
				{
					for(uint i = 0; i < c->StatPing.size(); i++)
						fprintf(fp, "%"NL_I64"u;%d\n", c->StatPing[i].first, c->StatPing[i].second);
				}
				fclose(fp);
			}
		}
	}
	for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
	{
		(*it)->reset();
	}
}

void CEntityManager::release()
{
	CNetwork::instance().release();
}

uint8 CEntityManager::findNewId()
{
	uint8 ni = 0;
	while(true)
	{
		EntityConstIt it;
		for(it = entities().begin(); it != entities().end(); it++)
		{
			if((*it)->id() == ni)
				break;
		}
		if(it == entities().end())
			return ni;
		ni++;
		if(ni == 255)
		{
			nlerror("More than 254 entities, can't add a new one");
		}	
	}
	return 255;
}

void CEntityManager::addBot(const string &name, bool isAutomaticBot)
{
	uint8 eid = findNewId();
	CEntity *e = (CEntity *) new CBot(eid, name, isAutomaticBot);
	nlassert(e);

	nlinfo("Adding bot eid %hu name '%s'", (uint16)e->id(), e->name().c_str());

	uint8 nbc = nbEntities();
	if(nbc >= NbMaxClients)
	{
		nlinfo("cannot add bot , too much client(%d)",nbc);
		return;
	}
		
	
	add(e);

	login(e);
}

void CEntityManager::addClient(NLNET::TSockId sock)
{
	nlassert(sock);

	uint8 eid = findNewId();
	nlassert(eid != 255);

	// TODO ace: display the good host
	nlinfo("Adding client eid %hu from '%p'", (uint16)eid, sock);

	CEntity *e = (CEntity *) new CClient(eid, sock);
	nlassert(e);

	add(e);
}

void CEntityManager::updateIdUpdateList()
{
	int minId = 0;
	IdUpdateList.clear();
	for(int i=0;i<255;i++)
	{
		for(CEntityManager::EntityConstIt it = entities().begin(); it != entities().end(); it++)
		{
			uint8 eid = (*it)->id();
			if(eid==i)
				IdUpdateList.push_back(eid);
		}			
	}
}

void CEntityManager::login(CEntity *e)
{
	bool self;
	uint8 nid = e->id();
	string n = e->name();
	
	{
		// send info about the new client to everybody but not the new client
		CNetMessage msgout(CNetMessage::Login);
		self = false;
		msgout.serial(self);
		msgout.serial(nid);
		msgout.serial(n);
		msgout.serial(e->Score);
		msgout.serial(e->Color);
		msgout.serial(e->Texture);
		bool s = e->spectator();
		msgout.serial(s);
		bool oc  = e->isOpen();
		msgout.serial(oc);
		msgout.serial(e->Trace);
		msgout.serial(e->MeshName);
		CNetwork::instance().sendAllExcept(nid, msgout);

		if(e->type() == CEntity::Client)
		{
			// only warn players when real players comes
			CNetMessage msgout2(CNetMessage::Chat);
			string str = n + " comes in !";
			msgout2.serial(str);
			CNetwork::instance().sendAllExcept(nid, msgout2);
		}
	}

	if(e->type() == CEntity::Client)
	{
		clientConnected(dynamic_cast<CClient*>(e)->Cookie, true);

		// send the new id for the new client
		CNetMessage msgout(CNetMessage::Login);
		self = true;
		msgout.serial(self);
		msgout.serial(nid);
		msgout.serial(n);
		msgout.serial(e->Score);
		msgout.serial(e->Color);
		msgout.serial(e->Texture);
		bool s = e->spectator();
		msgout.serial(s);

		string CurrentLevel="";
		// version 5 client can't handle startsession on login 
		if(e->networkVersion>=6) //TODO remove this when server version will be >= 6 
		{
			if(CLevelManager::instance().haveCurrentLevel())
				CurrentLevel = CLevelManager::instance().currentLevel().fileName();
		}
		
		msgout.serial(CurrentLevel);
		float timeBeforeTimeout;
		if (CSessionManager::instance().startTime() == 0)
		{
			timeBeforeTimeout = 0;
		}
		else
		{
			TTime currentTime = CTime::getLocalTime();
			timeBeforeTimeout = (float)(CSessionManager::instance().startTime()+(TTime)CLevelManager::instance().timeTimeout()-currentTime);
			//timeBeforeTimeout = (float)((TTime)CLevelManager::instance().timeTimeout() - CTime::getLocalTime()-CSessionManager::instance().startTime())/1000.0f;
		}
		msgout.serial(timeBeforeTimeout);

		bool oc  = e->isOpen();
		msgout.serial(oc);
		msgout.serial(e->Trace);
		msgout.serial(e->MeshName);
		CNetwork::instance().send(nid, msgout);
		
		// send the welcome message to the new client
		CNetMessage msgout2(CNetMessage::Chat);
		CConfigFile::CVar &welcome = IService::getInstance()->ConfigFile.getVar("WelcomeMessage");
		for(sint i = 0; i < welcome.size(); i++)
		{
			string str = welcome.asString(i);
			msgout2.serial(str);
		}
		CNetwork::instance().send(nid, msgout2);
		
		// send all entities informations to the new client
		{
			self = false;
			nlinfo ("There's %d people", entities().size());
			for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
			{
				if((*it) != e)
				{
					msgout.clear();
					msgout.setHeader(CNetMessage::Login);
					msgout.serial(self);
					uint8 eid = (*it)->id();
					msgout.serial(eid);
					string name = (*it)->name();
					msgout.serial(name);
					msgout.serial((*it)->Score);
					msgout.serial((*it)->Color);
					msgout.serial((*it)->Texture);
					bool s = (*it)->spectator();
					msgout.serial(s);
					bool oc  = (*it)->isOpen();
					msgout.serial(oc);
					msgout.serial((*it)->Trace);
					msgout.serial((*it)->MeshName);
					CNetwork::instance().send(nid, msgout);
				}
			}
		}
		
		// now the client can receive position update
		((CClient*)e)->networkReady(true);
	}
	updateIdUpdateList();
}

class teamNumber
{
public:
	teamNumber(string name) {this->name=name; count=1;};
	teamNumber() {name=""; count=0;};
	string name;
	uint count;
};

uint CEntityManager::getTeam(uint8 eid,uint teamCount)
{
	list<teamNumber> teams;
	list<teamNumber> sortedTeams;
	vector<list<string> > teamPack;
	list<teamNumber>::iterator itTeams;
	
	//list All team
	for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
	{
		CEntity *e = *it;
		string eTeam = e->team();
		if(!eTeam.empty())
		{
			bool teamAlreadyExist = false;
			for(itTeams=teams.begin();itTeams!=teams.end();itTeams++)
			{
				if(itTeams->name==eTeam)
				{
					teamAlreadyExist = true;
					itTeams->count++;
					break;
				}
			}
			if(!teamAlreadyExist)
			{
				//nlinfo(">> team : %s",eTeam.c_str());
				teams.push_back(eTeam);
			}
		}
	}
	while(teams.size())
	{
		teamNumber bigest;
		list<teamNumber>::iterator itErase;
		for(itTeams=teams.begin();itTeams!=teams.end();itTeams++)
		{
			if(itTeams->count>bigest.count || itTeams->count==0)
			{
				bigest = *itTeams;
				itErase = itTeams;
			}
		}
		if(!bigest.name.empty())
		{
			sortedTeams.push_back(bigest);
			teams.erase(itErase);
		}
	}
	uint i = 0;
	teamPack.resize(teamCount);

	//fill team in pack
	for(itTeams=sortedTeams.begin();itTeams!=sortedTeams.end();itTeams++)
	{
		//nlinfo(">> team : %s:%d",itTeams->name.c_str(),itTeams->count);
		if(itTeams->count>1)//at least 2 guys
		{
			for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
			{
				CEntity *e = *it;
				string eTeam = e->team();
				if(eTeam==itTeams->name)
				{
					for(uint j=0;j<teamCount;j++)
					{
						if(teamPack[i].size()<(CLevelManager::instance().currentLevel().getStartPointCount()/teamCount))
						{
							teamPack[i].push_back(e->name());
							break;
						}
						else
							i = (i+1)%teamCount;						
					}
				}
			}
			i = (i+1)%teamCount;
		}
	}
	//fill no team people
	for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
	{
		CEntity *e = *it;
		string eTeam = e->team();
		if(eTeam.empty())
		{
			uint smallestTeamId = 0;
			uint smallestTeamCount = 10000;
			//find smallest pack
			for(uint i=0;i<teamPack.size();i++)
			{
				if(teamPack[i].size()<smallestTeamCount)
				{
					smallestTeamCount = teamPack[i].size();
					smallestTeamId = i;
				}
			}
			teamPack[smallestTeamId].push_back(e->name());
		}
	}
	//fill with one guy teams
	for(itTeams=sortedTeams.begin();itTeams!=sortedTeams.end();itTeams++)
	{
		if(itTeams->count==1)
		{
			for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
			{
				CEntity *e = *it;
				string eTeam = e->team();
				if(eTeam==itTeams->name)
				{
					uint smallestTeamId = 0;
					uint smallestTeamCount = 10000;
					//find smallest pack
					for(uint i=0;i<teamPack.size();i++)
					{
						if(teamPack[i].size()<smallestTeamCount)
						{
							smallestTeamCount = teamPack[i].size();
							smallestTeamId = i;
						}
					}
					teamPack[smallestTeamId].push_back(e->name());
				}
			}
		}
	}
	

	//the entity we search team 
	string eName = getById(eid)->name();

	//display teams
	for(i=0;i<teamPack.size();i++)
	{
		list<string>::iterator it;
		for(it=teamPack[i].begin();it!=teamPack[i].end();it++)
		{
			//nlinfo("team %d : %s",i,(*it).c_str());
			if((*it)==eName)
				return i;
		}
	}
	return 0;
}

CEntity *CEntityManager::getByName(const std::string &name)
{
	CEntity *res = 0;
	for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
	{
		if((*it)->name() == name)
		{
			res = *it;
			break;
		}
	}
	return res;
}

void CEntityManager::displayText(float x,float y, float scale, CRGBA col, double duration, const string &text)
{
	string message = text;
	
	CNetMessage msgout(CNetMessage::DisplayText);
	msgout.serial(x);
	msgout.serial(y);
	msgout.serial(scale);
	msgout.serial(message);
	msgout.serial(col);
	msgout.serial(duration);
	CNetwork::instance().send(msgout);
	nlinfo("display message : %s to all",message.c_str());
}


void CEntityManager::displayText(uint8 eid, float x,float y, float scale, CRGBA col, double duration, const string &text)
{
	CEntity *e = getById(eid);
	if(e==0)
		return;

	if(e->type() == CEntity::Client)
	{
		string message = text;
		
		CNetMessage msgout(CNetMessage::DisplayText);
		msgout.serial(x);
		msgout.serial(y);
		msgout.serial(scale);
		msgout.serial(message);
		msgout.serial(col);
		msgout.serial(duration);
		CNetwork::instance().send(e->id(),msgout);
		nlinfo("display message : %s to %s",message.c_str(),e->name().c_str());
	}
}

void CEntityManager::remove(const string &name)
{
	if(name.empty())
	{
		nlwarning("Can't remove a client with empty name");
		return;
	}

	/*
	uint8 eid = 0;
	{
		CEntities::CReadAccessor acces(entities());
		for(EntityConstIt it = acces.value().begin(); it != acces.value().end(); it++)
		{
			if((*it)->name() == name)
			{
				eid =(*it)->id();
				break;
			}
		}
	}
	*/
	CEntity *e = getByName(name);
	if(e)
		remove(e->id());
	else
		nlwarning("Can't remove a client %s : not found",name.c_str());
}

CEntity *CEntityManager::getById(uint8 eid)
{
	if(eid == 255)
	{
		nlwarning("Can't find client because eid 255 is not valid");
		return 0;
	}
	
	CEntity *res = 0;
	
	{
		EntityIt it;
		for( it = entities().begin(); it != entities().end(); it++)
		{
			if((*it)->id() == eid)
			{
				// only unlink the client from the list
				res = (*it);
				return res;
			}
		}
	}
	
	nlwarning("Can't find client eid %hu ", (uint16)eid);
	return 0;
	
}

void CEntityManager::removeBot()
{
	remove("bot");
}

void CEntityManager::checkForcedClientCount()
{
	uint clientCount;
	{
		clientCount = entities().size();
	}

	if(clientCount < ForcedClientCount)
	{
		uint newAutomaticBotCount = ((uint)ForcedClientCount) - clientCount;
		for(uint i=0;i<newAutomaticBotCount;i++)
			addBot("", true);
	}
	else
	{
		uint oldAutomaticBotCount = clientCount - ((uint)ForcedClientCount);
		bool botFound = true;
		while(botFound && oldAutomaticBotCount)
		{
			botFound = false;
			uint8 oldBotId = 0;
			{
				for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
				{
					if((*it)->type() == CEntity::Bot)
					{
						CBot *b = (CBot *) (*it);
						if (b->isAutomaticBot())
						{
							nlinfo("Removing automatic bot : %s", b->name().c_str());
							oldBotId = b->id();
							botFound = true;
							break;
						}
					}
				}
			}
			if(botFound)
			{
				remove(oldBotId);
				oldAutomaticBotCount--;
			}
		}
	}
}

void CEntityManager::openClose(uint8 eid)
{
	bool found = false;
	for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
	{
		if((*it)->id() == eid)
		{
			(*it)->openClose();
			found = true;
			break;
		}
	}

	if(found)
	{
		// send the new state to all entities
		CNetMessage msgout(CNetMessage::OpenClose);
		msgout.serial(eid);
		CNetwork::instance().send(msgout);
	}
}
 
uint8 CEntityManager::nbEntities()
{
	uint8 nb;
	{
		nb = entities().size();
	}
	return nb;
}

CEntity *CEntityManager::getNthEntity(uint8 number)
{
	uint8  num = 0;
	CEntity *res = 0;
	EntityIt it;
	for( it = entities().begin(); it != entities().end(); it++)
	{
		res = (*it);
		if(num==number)
			return res;
		num++;
	}

	return 0;
}



bool CEntityManager::connected(const std::string &name)
{
	for(EntityConstIt it = entities().begin(); it != entities().end(); it++)
	{
		if((*it)->name() == name)
		{
			return true;
		}
	}
	return false;
}

string CEntityManager::check(const string &login, const string &password, bool dontCheck, sint32 &score)
{
	nlinfo("Check new client '%s'", login.c_str());

	if(login.empty())
	{
		return "Bad login, it must not be empty";
	}

	if(login.size() > 20)
	{
		return "Bad login, it must have less than 20 characters";
	}

	if(!dontCheck)
	{
		for(uint i = 0; i < login.size(); i++)
		{
			if(!nlisprint(login[i]) || !isalnum(login[i]) && login[i] != '_' && login[i] != '-' && login[i] != '.' && login[i] != '[' && login[i] != ']')
			{
				return toString("Bad login, character '%c' is not allowed", login[i]);
			}
		}

		if(password.empty())
		{
			return "Bad password, it must not be empty";
		}

		if(password.size() > 20)
		{
			return "Bad password, it must have less than 20 characters";
		}

		for(uint i = 0; i < password.size(); i++)
		{
			// switch to lower case
			if(!nlisprint(password[i]) || password[i] == ' ' || password[i] == ',' || password[i] == '{' || password[i] == '}' || password[i] == ';' || password[i] == '/' || password[i] == '\\' || password[i] == '"' || isspace(password[i]))
			{
				return toString("Bad password, character '%c' is not allowed", password[i]);
			}
		}
		
	}

	uint8 nbc = nbEntities();
	
	// check if server not full
	if(nbc > NbMaxClients)
	{
		return "Server full, "+toString(nbc)+" clients connected";
	}
	
	CConfigFile::CVar &accounts = IService::getInstance()->ConfigFile.getVar("Accounts");

	for(uint i = 0; i < (uint)accounts.size(); i += 3)
	{
		if(accounts.asString(i) == login)
		{
			if(dontCheck || accounts.asString(i+1) == password)
			{		
				// check if not already online
				if(connected(login))
				{
					return "Client "+login+" already online";
				}

				// if don't check, it means that we are on internet mode and then
				// we don't want to setup the score
				if(!dontCheck)
					score = accounts.asInt(i+2);

				sint32 maxScore = IService::getInstance()->ConfigFile.getVar("MaxScore").asInt();
				if(!isAdmin(login) && !isModerator(login) && maxScore != -1 && score >= maxScore)
				{
					return toString("Your score (%d) is now too high for this server (limited to %d). Try a more difficult server", score, maxScore);
				}

				return "";
			}
			else
			{
				return "Bad password, may be this login has already been created by someone else";
			}
		}
	}

	// unknown user, add it
	if(accounts.size() == 0)
	{
		accounts.forceAsString(login);
	}
	else
	{
		accounts.setAsString(login, accounts.size());
	}
	accounts.setAsString(password, accounts.size());
	accounts.setAsString("0", accounts.size());
	IService::getInstance()->ConfigFile.save();

	return "";
}

CEntity *getByString(std::string ident)
{
	CEntity *res = NULL;

	uint8 val = atoi(ident.c_str());
	if(val>0)
		res = CEntityManager::instance().getById(val);
	else
		res = CEntityManager::instance().getByName(ident);

	return res;
}


//
// Commands
//

NLMISC_COMMAND(addBot, "add a bot", "[<name>]")
{
	CEntityManager::instance().addBot((args.size() > 0) ? args[0] : "bot", false);
	return true;
}
 
NLMISC_COMMAND(displayEntities, "display info about all entities", "")
{
	if(args.size() != 0) return false;
	
	log.displayNL("Displaying %d entities:", CEntityManager::instance().entities().size());
	for(CEntityManager::EntityConstIt it = CEntityManager::instance().entities().begin(); it != CEntityManager::instance().entities().end(); it++)
	{
		(*it)->display(log);
	}
	return true;
}

NLMISC_COMMAND(kick, "kick a client from the server", "[<eid>|<name>]")
{
	if(args.size() != 1) return false;
	
	nlinfo("kick %s", args[0].c_str());
	
	CEntity *e = getByString(args[0]);

	if(e==NULL)
		return true;
	
	uint8 eid = e->id();
	if(e->isAdmin() || e->isModerator())
		return true;

	if(e->type() == CEntity::Client)
	{
		string reason = toString("You have been kicked !");
		CNetMessage msgout(CNetMessage::Error);
		msgout.serial(reason);
		CNetwork::instance().send(eid, msgout);
	}
	
	CEntityManager::instance().remove(eid);
	
	return true;
}

NLMISC_COMMAND(chat, "switch client chat on/off", "[<eid>|<name>]")
{
	if(args.size() != 1) return false;
	
	nlinfo("ban %s", args[0].c_str());
	
	CEntity *e = getByString(args[0]);

	if(e==NULL)
		return true;

	e->canSpeak(!e->canSpeak());
	CNetwork::instance().sendChat(toString("%s chat is now %s",e->name().c_str(),e->canSpeak()?"on":"off"));
	
	return true;
}

MTPT_COMMAND(ban, "ban a client from the server", "[<eid>|<name>] [duration]")
{
	if(args.size() != 2 && args.size() != 1) return false;
	
	CEntity *e = getByString(args[0]);

	if(e==NULL)
		return true;

	CClient *c = NULL;
	if(e->type()!=e->Client)
		return true;

	c = (CClient *)e;

	CMessage msgout("BC");

	const CInetAddress inetAddr = CNetwork::instance().hostAddress(c->sock());
	string ip = inetAddr.ipAddress();
	string userName = c->name();
	string kickerName = entity->name();
	uint32 duration = 60;
	msgout.serial(ip, userName, kickerName, duration);
	
	nlinfo("%s bans %s (%s) %d",entity->name().c_str(), userName.c_str(),ip.c_str(),duration);
	//CUnifiedNetwork::getInstance()->send("LS", msgout);
	
	//TODO
	
	return true;
}


NLMISC_CATEGORISED_COMMAND(commands,bof, "watch a client", "[<eid>]")
{

	return true;
}

NLMISC_COMMAND(watch, "watch a client", "[<eid>]")
{
	if(args.size() > 1) return false;
	WatchingId = (args.size() == 0) ? 0 : atoi(args[0].c_str());
	return true;
}

NLMISC_DYNVARIABLE(uint32, NbEntities, "Number of entities (player+bot)")
{
	if(!get) return;
	
	*pointer = CEntityManager::instance().entities().size();
}

NLMISC_DYNVARIABLE(uint32, NbClients, "Number of players")
{
	if(!get) return;

	uint32 nb = 0;
	for(CEntityManager::EntityConstIt it = CEntityManager::instance().entities().begin(); it != CEntityManager::instance().entities().end(); it++)
	{
		if((*it)->type() == CEntity::Client)
		{
			nb++;
		}
	}
	*pointer = nb;
}

NLMISC_DYNVARIABLE(string, Watch1, "")
{
	if(!get) return;
	if(!WatchingId)
	{
		*pointer = "NoWatch                                                                       ";
		return;
	}

	{
		for(CEntityManager::EntityConstIt it = CEntityManager::instance().entities().begin(); it != CEntityManager::instance().entities().end(); it++)
		{
			if((*it)->id() == WatchingId)
			{
				*pointer  = "Id:"+NLMISC::toString((*it)->id());
				*pointer += " Name:"+(*it)->name();
				if((*it)->type() == CEntity::Bot) *pointer += " Bot";
				else if((*it)->type() == CEntity::Client) *pointer += " Client";
				else *pointer += " ???";
				if ((*it)->type() == CEntity::Client)
				{
					*pointer += " TCPFrom:";
					// TODO ace: display the good host
					*pointer += NLMISC::toString("%p", ((CClient*)(*it))->sock());
				}
				*pointer += " Ping:"+NLMISC::toString((*it)->Ping.getSmoothValue());
				return;
			}
		}
		*pointer = "Entity "+NLMISC::toString(WatchingId)+" not found                                       ";
	}
}

NLMISC_DYNVARIABLE(string, Watch2, "")
{
	if(!get) return;
	if(!WatchingId)
	{
		*pointer = "NoWatch                                                                       ";
		return;
	}
	
	{
		for(CEntityManager::EntityConstIt it = CEntityManager::instance().entities().begin(); it != CEntityManager::instance().entities().end(); it++)
		{
			if((*it)->id() == WatchingId)
			{
				*pointer  = "CurScore:"+NLMISC::toString((*it)->CurrentScore);
				*pointer += " TotScore:"+NLMISC::toString((*it)->Score);
				if((*it)->FreezeCommand) *pointer += " FreezeCommand";
				if((*it)->OnTheWater) *pointer += " OnTheWater";
				if((*it)->InGame) *pointer += " InGame";
				else *pointer += " OutSession";
				if((*it)->OpenClose) *pointer += " Open";
				else *pointer += " Close";
				if((*it)->Ready) *pointer += " IsReady";
				else *pointer += " IsNotReady";
				*pointer += " NbOC:"+NLMISC::toString((*it)->NbOpenClose);
				*pointer += " Frict:"+NLMISC::toString((*it)->Friction);
				*pointer += " Accel:"+NLMISC::toString((*it)->Accel);
				
				return;
			}
		}
		*pointer = "Entity "+NLMISC::toString(WatchingId)+" not found                                       ";
	}
}

NLMISC_DYNVARIABLE(string, Watch3, "")
{
	if(!get) return;
	if(!WatchingId)
	{
		*pointer = "NoWatch                                                                       ";
		return;
	}
	
	{
		for(CEntityManager::EntityConstIt it = CEntityManager::instance().entities().begin(); it != CEntityManager::instance().entities().end(); it++)
		{
			if((*it)->id() == WatchingId)
			{
				*pointer += "Pos:("+NLMISC::toString("%.2f",(*it)->Pos.x)+","+NLMISC::toString("%.2f",(*it)->Pos.y)+","+NLMISC::toString("%.2f",(*it)->Pos.z)+")";
				*pointer += " Forc:("+NLMISC::toString("%.2f",(*it)->Force.x)+","+NLMISC::toString("%.2f",(*it)->Force.y)+","+NLMISC::toString("%.2f",(*it)->Force.z)+")";
				*pointer += " "+(*it)->ColState;
				return;
			}
		}
		*pointer = "Entity "+NLMISC::toString(WatchingId)+" not found                                       ";
	}
}


NLMISC_DYNVARIABLE(string, Watch4, "")
{
	if(!get) return;
	if(!WatchingId)
	{
		*pointer = "NoWatch                                                                       ";
		return;
	}
	
	{
		for(CEntityManager::EntityConstIt it = CEntityManager::instance().entities().begin(); it != CEntityManager::instance().entities().end(); it++)
		{
			if((*it)->id() == WatchingId)
			{
				const dReal *vec = dBodyGetPosition((*it)->Body);
				*pointer += "PPos:("+NLMISC::toString("%.2f",vec[0])+","+NLMISC::toString("%.2f",vec[1])+","+NLMISC::toString("%.2f",vec[2])+")";
				vec = dBodyGetLinearVel((*it)->Body);
				*pointer += " PLinVel:("+NLMISC::toString("%.2f",vec[0])+","+NLMISC::toString("%.2f",vec[1])+","+NLMISC::toString("%.2f",vec[2])+")";
				*pointer += " PForce:("+NLMISC::toString("%.2f",vec[0])+","+NLMISC::toString("%.2f",vec[1])+","+NLMISC::toString("%.2f",vec[2])+")";
				vec = dBodyGetTorque((*it)->Body);
				return;
			}
		}
		*pointer = "Entity "+NLMISC::toString(WatchingId)+" not found                                       ";
	}
}

NLMISC_DYNVARIABLE(string, Watch5, "")
{
	if(!get) return;
	if(!WatchingId)
	{
		*pointer = "NoWatch                                                                       ";
		return;
	}
	
	{
		for(CEntityManager::EntityConstIt it = CEntityManager::instance().entities().begin(); it != CEntityManager::instance().entities().end(); it++)
		{
			if((*it)->id() == WatchingId)
			{
				const dReal *vec = dBodyGetRotation((*it)->Body);
				*pointer += "PRot:("+NLMISC::toString("%.2f",vec[0])+","+NLMISC::toString("%.2f",vec[1])+","+NLMISC::toString("%.2f",vec[2])+")";
				vec = dBodyGetAngularVel((*it)->Body);
				*pointer += " PAngVel:("+NLMISC::toString("%.2f",vec[0])+","+NLMISC::toString("%.2f",vec[1])+","+NLMISC::toString("%.2f",vec[2])+")";
				vec = dBodyGetForce((*it)->Body);
				*pointer += " PTorque:("+NLMISC::toString("%.2f",vec[0])+","+NLMISC::toString("%.2f",vec[1])+","+NLMISC::toString("%.2f",vec[2])+")";
				
				return;
			}
		}
		*pointer = "Entity "+NLMISC::toString(WatchingId)+" not found                                       ";
	}
}


uint CEntityManager::humanClientCount()
{
	CEntityManager::EntityConstIt it;
	
	uint res = 0;
	for(it = entities().begin(); it != entities().end(); it++)
	{
		if((*it)->type() == CEntity::Client)
		{
			res++;
		}
		else if ((*it)->type() == CEntity::Bot)
		{
			CBot *b = (CBot *)(*it);
			if(!b->isAutomaticBot())
				res++;
		}
	}
	return res;
}



bool CEntityManager::nameExist(std::string name)
{
	CEntityManager::EntityConstIt it;

	for(CEntityManager::EntityConstIt it = entities().begin(); it != entities().end(); it++)
	{
		if((*it)->name() == name)
		{
			return true;
		}
	}
	
	return false;
}

void CEntityManager::saveAllValidReplay()
{
	nlinfo("CEntityManager::saveAllValidReplay()");
	CEntityManager::EntityConstIt it;
	// close all replay file
	for(it = entities().begin(); it != entities().end(); it++)
	{
		if((*it)->type() == CEntity::Client)
		{
			CClient *c = (CClient *)(*it);
			if(c->ReplayFile)
			{
				fprintf(c->ReplayFile, "%d AU %.1f %s %d\n", c->id(), 0.0f, c->name().c_str(), c->CurrentScore);
				fclose(c->ReplayFile);
				c->ReplayFile = 0;
				if(c->CurrentScore <IService::getInstance()->ConfigFile.getVar("SavedReplayMinimumScore").asInt())
					CFile::deleteFile(c->ReplayFilename.c_str());
			}
		}
		//		else
		//			nlinfo("bot do score = %d ",c->CurrentScore);
	}
}

void CEntityManager::checkAfkClient()
{
	nlinfo("CEntityManager::checkAfkClient()");
	if(IService::getInstance()->ConfigFile.getVar("CheckAfk").asInt()==0)
		return;
	
	vector<uint8> IdToRemove;
	vector<string> msgs;
	{
		CEntityManager::EntityConstIt it;
		
		// send the message to all entities
		for(it = entities().begin(); it != entities().end(); it++)
		{
			if((*it)->type()==CEntity::Client && !(*it)->forceReceived())
			{
				(*it)->AfkCount++;
				(*it)->CurrentScore = 0;
				if((*it)->AfkCount>=(uint)IService::getInstance()->ConfigFile.getVar("MaxAfkSessionCount").asInt())
				{
					IdToRemove.push_back((*it)->id());
					string timeoutMsg = toString("kick %s : away from keyboard",(*it)->name().c_str());
					msgs.push_back(timeoutMsg);
				}
			}
			else
			{
				(*it)->AfkCount = 0;
			}
		}	
	}
	for(uint i = 0; i < IdToRemove.size(); i++)
	{
		string reason = "Away from keyboard";
		CNetMessage msgout(CNetMessage::Error);
		msgout.serial(reason);
		CNetwork::instance().send(IdToRemove[i], msgout);
		
		CEntityManager::instance().remove(IdToRemove[i]);
	}
	IdToRemove.clear();
	for(uint i = 0; i < msgs.size(); i++)
	{
		CNetwork::instance().sendChat(msgs[i]);
	}
	msgs.clear();
	
}

bool CEntityManager::everyBodyReady()
{
	bool res = true;
	vector<uint8> IdToRemove;
	vector<string> msgs;
	{
		float waitingReadyTimeout = IService::getInstance()->ConfigFile.getVar("WaitingReadyTimeout").asFloat(); 
		CEntityManager::EntityConstIt it;
		
		bool allReady = true;
		// send the message to all entities
		for(it = entities().begin(); it != entities().end(); it++)
		{
			if((*it)->type()==CEntity::Client && (*it)->WaitingReady && !(*it)->Ready )
			{
				if( (float)(CTime::getLocalTime() - (*it)->WaitingReadyTimeoutStart)/1000.0f > waitingReadyTimeout) //timeout
				{
					IdToRemove.push_back((*it)->id());
					string timeoutMsg = toString("kick %s : wait for ready timeout",(*it)->name().c_str());
					msgs.push_back(timeoutMsg);
					nlinfo("%s",timeoutMsg.c_str());
				}
				else
					res = false;
				break;
			}
		}	
	}
	for(uint i = 0; i < IdToRemove.size(); i++)
	{
		string reason = "waiting for ready has timed out";
		CNetMessage msgout(CNetMessage::Error);
		msgout.serial(reason);
		CNetwork::instance().send(IdToRemove[i], msgout);

		CEntityManager::instance().remove(IdToRemove[i]);
	}
	IdToRemove.clear();
	for(uint i = 0; i < msgs.size(); i++)
	{
		CNetwork::instance().sendChat(msgs[i]);
	}
	msgs.clear();
	
	return res;
}


float CEntityManager::slowerVelocity()
{

	float res = 1000;
	CEntityManager::EntityConstIt it;
	
	for(it = entities().begin(); it != entities().end(); it++)
	{
		CEntity *c = *it;

		float val = c->meanVelocity();
		{
			if(val<res)
				res = val;
		}
	}
	

	return res;
}

void CEntityManager::initBeforeStartLevel()
{
	CEntityManager::EntityConstIt it;
	for(it = entities().begin(); it != entities().end(); it++)
	{
		CEntity *c = *it;
		c->initBeforeStartLevel();		
	}
}

bool CEntityManager::isAdmin(const string &name) const
{
	CConfigFile::CVar &admin = IService::getInstance()->ConfigFile.getVar("Admin");
	for(uint i = 0; i < (uint)admin.size(); i++)
	{
		if(admin.asString(i) == name)
		{
			return true;
		}
	}
	return false;
}

bool CEntityManager::isModerator(const string &name) const
{
	CConfigFile::CVar &mod = IService::getInstance()->ConfigFile.getVar("Moderator");
	for(uint i = 0; i < (uint)mod.size(); i++)
	{
		if(mod.asString(i) == name)
		{
			return true;
		}
	}
	return false;
}

