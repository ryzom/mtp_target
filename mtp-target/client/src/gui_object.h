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
// This is the main class that manages all other classes
//

#ifndef MTPT_GUI_OBJECT_H
#define MTPT_GUI_OBJECT_H


//
// Includes
//
#include "gui_mouse_listener.h"
#include "gui_event_behaviour.h"
#include "gui_spg.h"
#include <libxml/parser.h>

NLMISC::CVector CVectorMax(NLMISC::CVector a,NLMISC::CVector b);
class CGuiXml;	

//
// Classes
//
class CGuiObject
{
public:

	enum TGuiAlignment {
			eAlignNone             = 0x0,
			eAlignCenterVertical   = 0x1,
			eAlignCenterHorizontal = 0x2,
			eAlignUp               = 0x4,
			eAlignBottom           = 0x8,
			eAlignLeft             = 0x10,
			eAlignRight            = 0x20,
			eAlignExpandVertical   = 0x40,
			eAlignExpandHorizontal = 0x80,
	};

	CGuiObject();
	virtual ~CGuiObject();
	
	virtual void init();
	virtual void update();
	void render(NLMISC::CVector pos,NLMISC::CVector &maxSize);
	virtual void release();
	
	//virtual bool mouseIsIn(NLMISC::CVector pos,NLMISC::CVector maxSize);
	bool isIn(NLMISC::CVector point,NLMISC::CVector startPos,NLMISC::CVector size);
	static float ToProportionalX(float x);
	static float ToProportionalY(float y);
	virtual float width();
	virtual float height();
	virtual NLMISC::CVector position();
	virtual NLMISC::CVector globalPosition(NLMISC::CVector pos,NLMISC::CVector &maxSize);
	virtual NLMISC::CVector expandSize(NLMISC::CVector &maxSize);
	virtual NLMISC::CVector size();
	virtual void  position(NLMISC::CVector position);
	
	virtual TGuiAlignment alignment();
	virtual void alignment(int alignment);
	NLMISC::CVector offset(NLMISC::CVector maxSize);

	NL3D::UMaterial *CGuiObject::LoadBitmap(std::string filename);
		
	//CVector globalPosition()

	bool focused();
	float minWidth();
	float minHeight();
	void minWidth(float minWidth);
	void minHeight(float minHeight);

	NLMISC::CVector minSize();
	void minSize(NLMISC::CVector minSize);
	
	std::string name();

	static CGuiObject *XmlCreateFromNode(CGuiXml *xml,xmlNodePtr node);
	virtual void init(CGuiXml *xml,xmlNodePtr node);
		
	
friend class CGuiXmlManager;	
friend class CGuiXml;	
protected:
	std::string _name;
	virtual float _width() = 0;
	virtual float _height() = 0;
	virtual void _preRender(NLMISC::CVector pos,NLMISC::CVector &maxSize);
	virtual void _render(NLMISC::CVector pos,NLMISC::CVector &maxSize);
	virtual void _postRender(NLMISC::CVector pos,NLMISC::CVector &maxSize);
	void _checkFocus(NLMISC::CVector pos,NLMISC::CVector maxSize);
private:
	TGuiAlignment _alignment;
	NLMISC::CVector _position;	
	NLMISC::CVector _minSize;	
};

typedef CGuiObject * (*CreateObjectCB) ();
typedef std::map<std::string,CreateObjectCB> string2CreateFunction;


class CGuiObjectManager : public CSingleton<CGuiObjectManager>
{
public:
	virtual void init();
	virtual void update();
	virtual void render();
	virtual void release();

	void registerClass(std::string className,CreateObjectCB cb);
	CGuiObject *create(std::string className);
		
	CGuiMouseListener &mouseListener();
	void focus(CGuiObject *object);
	CGuiObject *focus();

	std::list<CGuiObject *> objects;
protected:
private:
	guiSPG<CGuiMouseListener> _mouseListener;
	CGuiObject *_focus;
	string2CreateFunction _createFunctionMap;
};


#endif