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

#ifndef _LUA_ENGINE_H_
#define _LUA_ENGINE_H_


//
// Includes
//

#include "../../common/lua_utility.h"
#include "../../common/lunar.h"
#include "entity_lua_proxy.h"
#include "module_lua_proxy.h"
#include "level_lua_proxy.h"


//
// Classes
//

class CLuaEngine : public CSingleton<CLuaEngine>
{
public:
	CLuaEngine();
	void init(const std::string &filename);
	void update();
	void release();


	void entityEntityCollideEvent(CEntity *client1, CEntity *client2);
	void entitySceneCollideEvent(CEntity *client, CModule *module);
	void entityWaterCollideEvent(CEntity *client);

	lua_State *session(){return _luaSession;};

private:
	
	lua_State *_luaSession;
	CLevel *_level;
	
};




#endif