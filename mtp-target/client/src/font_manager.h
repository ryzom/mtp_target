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

#ifndef MTPT_FONT_MANAGER_H
#define MTPT_FONT_MANAGER_H


//
// Includes
//

#include <nel/3d/u_texture.h>
#include <nel/3d/u_material.h>



//
// Classes
//

class CFontManager : public CSingleton<CFontManager>, public ITask
{
public:

	virtual void init();
	virtual void update() { }
	virtual void render() { }
	virtual void release();

	virtual std::string name() { return "CFontManager"; }
	
	NL3D::UMaterial &material() const { nlassert(Material); return *Material; }

	void littlePrintf(float x, float y, const char *format ...);
	void printf(const NLMISC::CRGBA &col, float x, float y, float scale, const char *format, ...);
	void printf3D(const NLMISC::CRGBA &col, const NLMISC::CVector &pos, float scale, const char *format, ...);
	
	uint32 fontWidth() const { return FontWidth; }
	uint32 fontHeight() const { return FontHeight; }
	
private:

	uint32 FontHeight;
	uint32 FontWidth;
	
	NL3D::UTextureFile *Texture;
	NL3D::UMaterial *Material;

};

#endif