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
// This class manage the physics thread and every linked to ODE
//

#ifndef MTPT_PHYSICS
#define MTPT_PHYSICS


//
// Includes
//

#include <ode/ode.h>

#include <nel/misc/thread.h>
#include <nel/misc/vector.h>
#include <nel/misc/command.h>

#include <nel/net/service.h>


//
// Constants
//

#define MT_DEFAULT_SPECTRUM 0xffffffff
#define MT_SCENE_SPECTRUM 0x0001
#define MT_CLIENT_SPECTRUM 0x0002


//
// Functions
//

void pausePhysics();
void resumePhysics();
void initPhysics();
void releasePhysics();


extern dWorldID			World;

extern NLMISC::CSynchronized<dSpaceID>		Space;

extern dJointGroupID	ContactGroup;
extern dGeomID			Water;


//
// Classes
//

/*class CCollisionEntity
{
public:
	CCollisionEntity() : Score(0), Accel(0.0f), Friction(0.0f), Bounce(false), Geom(0), LuaUserData(0), LuaUserDataRef(0)
	{
	}
	
	~CCollisionEntity()
	{
		if(Geom)
		{
			dGeomDestroy(Geom);
			Geom = 0;
		}
	}
	
	void setTriColl(const std::string &filename)
	{
		nlassert(!Geom);
		dGeomSetData(Geom, (void*)this);
	}
	
	void setBox(const NLMISC::CVector &size)
	{
		nlassert(!Geom);
		Geom = dCreateBox(Space, size.x, size.y, size.z);
		dGeomSetData(Geom, (void*)this);
	}
	
	void setRotation(const NLMISC::CAngleAxis &aa)
	{
		nlassert(Geom);
		dMatrix3 R;
		dRFromAxisAndAngle(R, aa.Axis.x, aa.Axis.y, aa.Axis.z, aa.Angle);
		dGeomSetRotation(Geom, R);
	}
	
	void setPosition(const NLMISC::CVector &pos)
	{
		nlassert(Geom);
		dGeomSetPosition(Geom, pos.x, pos.y, pos.z);
	}

	void getPosition(NLMISC::CVector &pos) const
	{
		nlassert(Geom);
		const dReal *p = dGeomGetPosition(Geom);
		if(p == 0)
			pos = NLMISC::CVector::Null;
		else
		{
			pos.x = p[0];
			pos.y = p[1];
			pos.z = p[2];
		}
	}
	
	uint32		Score;
	float		Accel;
	float		Friction;
	bool		Bounce;
	std::string	Name;

	dGeomID				Geom;		// TODO SKEET_WARNING put in private
	std::vector<dReal>	Vertices;	// 3 entries for one vertex (x,y,z)
	std::vector<int>	Indices;
	
	void		*LuaUserData;
	int			 LuaUserDataRef;	
	std::string	 LuaFunctionName;
};*/

//
// Typedefs
//

typedef NLMISC::CSynchronized<NLMISC::TTime> SyncPhyTime;

//typedef std::list<CCollisionEntity>::iterator CollisionEntityListIt;
//extern std::list<CCollisionEntity> CollisionEntityList;

//
// Variables
//

extern NLMISC::TTime syncStartPhyTime;
extern SyncPhyTime syncPhyTime;

//extern std::vector<NLMISC::CVector> StartPointList;

#endif