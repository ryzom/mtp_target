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

#include <deque>
#include <string>

#include <3d/mesh.h>
#include <3d/shape.h>
#include <3d/material.h>
#include <3d/register_3d.h>

#include <nel/misc/quat.h>
#include <nel/misc/common.h>

#include <nel/3d/u_instance_material.h>

#include "3d/water_height_map.h"
#include "3d/water_pool_manager.h"

#include "global.h"
#include "entity.h"
#include "network_task.h"
#include "mtp_target.h"
#include "resource_manager.h"
#include "level_manager.h"

#include "stdpch.h"

#include <deque>

#include <nel/misc/quat.h>
#include <nel/misc/common.h>

#include <nel/3d/u_instance_material.h>

#include "3d/water_height_map.h"
#include "3d/water_pool_manager.h"

#include "editable_element.h"
#include "global.h"
#include "3d_task.h"
#include "time_task.h"
#include "mtp_target.h"
#include "font_manager.h"
#include "config_file_task.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;



CEditableElement::CEditableElement(const string &name,uint8 id, CVector position, CAngleAxis rotation)
{
	Name = name;
	_id = id;
	_changed = false;

	
	mat = C3DTask::instance().driver().createMaterial();

	Position = position;
	Rotation = rotation;

	string res = CResourceManager::instance().get("col_box.shape");
	Mesh = C3DTask::instance().scene().createInstance (res);
	if (!Mesh)
	{
		nlwarning ("Can't load 'col_box.shape'");
	}

	Mesh->setTransformMode(UTransformable::RotQuat);
	Mesh->setRotQuat(CQuat(rotation));
	Mesh->setPos(position);
	
}


bool CEditableElement::intersect(NLMISC::CVector rayStart,NLMISC::CVector rayDir,NLMISC::CVector &rayHit)
{
	return true;
}


CEditableElement::~CEditableElement()
{
	C3DTask::instance().scene().deleteInstance(Mesh);
	
}



//   /E--F
//  A---B|
//  | H |G
//  D---C



void CEditableElement::renderBBox()
{
	mat->setColor(CRGBA(255,255,255,255));
	mat->setZWrite(true);
	mat->setZFunc(UMaterial::always);
	CAABBox bbox;
	Mesh->getShapeAABBox(bbox);
	CAABBox tbbox = CAABBox::transformAABBox(Mesh->getMatrix(),bbox);
	
	CVector boxMin = tbbox.getMin();
	CVector boxMax = tbbox.getMax();

	CQuad quad;
	CVector a = boxMin;
	CVector b = a;
	b.x = boxMax.x;
	CVector c = a;
	c.x = boxMax.x;
	c.y = boxMax.y;
	CVector d = a;
	d.y = boxMax.y;
	
	CVector e = a;
	e.z = boxMax.z;
	CVector f = b;
	f.z = boxMax.z;
	CVector g = c;
	g.z = boxMax.z;
	CVector h = d;
	h.z = boxMax.z;
	
	CLine l;

	l.V0 = a;
	l.V1 = b;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = b;
	l.V1 = c;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = c;
	l.V1 = d;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = d;
	l.V1 = a;
	C3DTask::instance().driver().drawLine(l,*mat);
	
	l.V0 = e;
	l.V1 = f;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = f;
	l.V1 = g;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = g;
	l.V1 = h;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = h;
	l.V1 = e;
	C3DTask::instance().driver().drawLine(l,*mat);
	
	l.V0 = a;
	l.V1 = e;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = b;
	l.V1 = f;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = c;
	l.V1 = g;
	C3DTask::instance().driver().drawLine(l,*mat);
	l.V0 = d;
	l.V1 = h;
	C3DTask::instance().driver().drawLine(l,*mat);
	
	
	/*
	quad.V0 = a;
	quad.V1 = b;
	quad.V2 = c;
	quad.V3 = d;

	C3DTask::instance().driver().drawQuad(quad,*mat);
	*/			
}

void CEditableElement::show()
{
	Mesh->show();
}

void CEditableElement::hide()
{
	Mesh->hide();
}

void CEditableElement::position(CVector pos)
{
	Position = pos;
	Mesh->setPos(pos);
	_changed = true;
	CLevelManager::instance().currentLevel().changed(true);
}

CVector CEditableElement::position()
{
	return Position;
}

UInstance *CEditableElement::mesh()
{
	return Mesh;
}

void CEditableElement::update(NLMISC::CVector pos,NLMISC::CVector rot,uint32 selectedBy)
{
	position(pos);
}
