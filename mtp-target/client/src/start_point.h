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
// This class manages an entity (player or bot)
//

#ifndef MTPT_START_POINT
#define MTPT_START_POINT


//
// Includes
//

#include <deque>
#include <string>

#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

#include <nel/3d/u_instance.h>
#include <nel/3d/u_particle_system_instance.h>

#include "global.h"


//
// Includes
//

#include <deque>
#include <string>

#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

#include <nel/3d/u_instance.h>
#include <nel/3d/u_particle_system_instance.h>

#include "sound_manager.h"
#include "interpolator.h"


//
// Namespaces
//

using NLMISC::CVector;
using NLMISC::CMatrix;


//
// Classes
//

class CStartPoint
{
public:
	CStartPoint(const std::string &name,uint8 id, NLMISC::CVector position, NLMISC::CAngleAxis rotation);
	virtual ~CStartPoint();
	
	virtual bool intersect(NLMISC::CVector rayStart,NLMISC::CVector rayDir,NLMISC::CVector &rayHit);

	NL3D::UInstance *mesh();
	void renderBBox();

	void show();
	void hide();
	void update(NLMISC::CVector pos,NLMISC::CVector rot,uint32 selectedBy);
	const std::string	&name() const { return Name; }
	void position(NLMISC::CVector pos);
	CVector position();

	uint8 id() {return _id;}
	bool changed() {return _changed;}
	void changed(bool c) {_changed = c;}
	
	virtual NL3D::UInstance *_loadMesh()=0;
	
protected:
	std::string		Name;
	NL3D::UInstance *Mesh;
	NLMISC::CVector Position;
	NLMISC::CAngleAxis Rotation;
	NL3D::UMaterial *mat;
	uint8 _id;
	bool  _changed;
};


#endif