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

#include <nel/3d/u_material.h>

#include "3d_task.h"
#include "time_task.h"
#include "gui_object.h"
#include "font_manager.h"
#include "resource_manager.h"
#include "gui_multiline_text.h"


//
// Namespaces
//

using namespace std;
using namespace NL3D;
using namespace NLMISC;

#define MultilineStringYSpace 3
//
// Variables
//


//
// Functions
//
	
template<class OutIt> void Split( const std::string& s, char sep, OutIt dest ) 
{
	std::string::size_type left = 0;
	std::string::size_type right = left;
	while( left <= s.size())
	{
		right=left;
		for(;right != s.size() && s[right]!=sep;right++);
		*dest = s.substr( left, right-left );
		++dest;
		if(right==s.size()) break;
		left = right+1;
	}
}




CGuiMultilineText::CGuiMultilineText()
{
}

CGuiMultilineText::~CGuiMultilineText()
{
}


void CGuiMultilineText::Printf(float x, float y, int cursorIndex, CVector &cursorPos, const char *format ...)
{
	string str;
	NLMISC_CONVERT_VARGS (str, format, 256);

	Print(x,y,cursorIndex,cursorPos,str);
}

void CGuiMultilineText::Print(float x, float y, int cursorIndex, CVector &cursorPos, const string &str)
{
	if(str.size()==0)
	{
		cursorPos.x = x;
		cursorPos.y = y;
		return;
	}

	UTextContext::CStringInfo stringInfo = CFontManager::instance().guiTextContext().getStringInfo(ucstring(str));

	UTextContext::CStringInfo defaultStringInfo = CFontManager::instance().guiTextContext().getStringInfo(ucstring("Yg"));
	float StringHeight = defaultStringInfo.StringHeight; //to find max height
	float StringLine = defaultStringInfo.StringLine;

	std::vector<std::string> vstr;
	Split(str, '\n', std::back_inserter(vstr));
	int subStringStart = 0;
	for( std::vector<std::string>::size_type i = 0; i < vstr.size(); ++i ) 
	{
		int subStringLen = vstr[i].size();
		CFontManager::instance().guiTextContext().printAt (CGuiObject::ToProportionalX(x), 1.0f - CGuiObject::ToProportionalY(y - StringLine + StringHeight), ucstring(ucstring(vstr[i])));

		if(subStringStart <= cursorIndex && cursorIndex <= subStringStart+subStringLen)
		{
			string subSubString = vstr[i].substr(0,cursorIndex-subStringStart);
			UTextContext::CStringInfo subStringInfo = CFontManager::instance().guiTextContext().getStringInfo(ucstring(subSubString));
			cursorPos.y = y - /*TODO ace*/12 + StringHeight - StringLine + 1;
			cursorPos.x = x + subStringInfo.StringWidth;
		}

		y += StringHeight + MultilineStringYSpace;
		subStringStart += subStringLen+1;
	}
}
	

CVector CGuiMultilineText::Size(bool shaded,int size,const std::string &str)
{
	CFontManager::instance().guiTextContext().setFontSize (size);
	CFontManager::instance().guiTextContext().setShaded(shaded);

	UTextContext::CStringInfo defaultStringInfo = CFontManager::instance().guiTextContext().getStringInfo(ucstring("Yg"));
	float StringHeight = defaultStringInfo.StringHeight; //to find max height
	float StringLine = defaultStringInfo.StringLine;
	
	UTextContext::CStringInfo stringInfo = CFontManager::instance().guiTextContext().getStringInfo(ucstring(str));
	CVector res;
	res.y = StringHeight;
	
	std::vector<std::string> vstr;
	Split(str, '\n', std::back_inserter(vstr));
	res.y *= vstr.size();

	if(vstr.size()>1)
		res.y += MultilineStringYSpace * (vstr.size()-1);
	
	res.x = 0;

	for( std::vector<std::string>::size_type i = 0; i < vstr.size(); ++i ) 
	{
		UTextContext::CStringInfo subStringInfo = CFontManager::instance().guiTextContext().getStringInfo(ucstring(vstr[i]));
		if(subStringInfo.StringWidth>res.x)
			res.x = subStringInfo.StringWidth;
	}
	
	return res;
}



