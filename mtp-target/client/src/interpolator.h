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


#ifndef MTPT_INTERPOLATOR
#define MTPT_INTERPOLATOR


//
// Includes
//

#include <deque>
#include <string>

#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

#include <nel/3d/u_instance.h>
#include <nel/3d/u_particle_system_instance.h>


//
// Namespaces
//

using NLMISC::CVector;
using NLMISC::CMatrix;


//
// Classes
//


class CEntityState
{
public:
	CEntityState();
	CEntityState(CVector position,bool onWater);
	CVector position;
	bool    onWater;
	CEntityState operator+( const CEntityState &other ) const;
	friend CEntityState operator*( double coef, CEntityState &value );
	friend CEntityState operator*( CEntityState &value, double coef );
protected:
private:
};

template <class T> class CInterpolatorKey
{
public:
	CInterpolatorKey(T value,double time)
	{
		_value = value;
		_serverTime = time;
	}
	~CInterpolatorKey(){};

	T      value() 
	{
		return _value;
	}
	double  serverTime()
	{
		return _serverTime;
	}
	
	
	static T Lerp(T a,T b,double pos)
	{
		return a * (1.0 - pos) + b * pos;
	}
friend class CInterpolator;
protected:
	double		_serverTime;
	T			_value;	
};

typedef CInterpolatorKey<CEntityState> CEntityInterpolatorKey;


class CInterpolator
{
public:
	CInterpolator();
	virtual ~CInterpolator();
	virtual void update();

	CVector	position() const;
	CVector	speed() const;
	CVector direction() const;
	CVector smoothDirection() const;
	bool    onWater() const;
	double  lct() const;

	void    lct(double lct) const;

	bool available() const { return _available; };
	void addKey(CEntityInterpolatorKey key);
	bool outOfKey();
	double localTime() const;
	
	virtual void reset();
	
protected:
	virtual CVector _position(double time);
	virtual CVector _speed(double time);
	virtual CVector _direction(double time);
	virtual void _autoAdjustLct();
	
	std::deque<CEntityInterpolatorKey> _keys;
	double  _startTime;
	double  _serverTime;
	double  _localTime;
	double  _deltaTime;
	double  _lastUpdateTime;
	double  _addKeyTime;
	double  _maxKeyDiffTime;
	double  _meanKeyDiffTime;
	bool    _available;
	bool    _outOfKey;

	CVector _currentPosition;
	CVector _currentSpeed;
	CVector _currentDirection;
	CVector _currentSmoothDirection;
	bool    _currentOnWater;
	
	double  _lct;
	static const double _minLct;
	static const double _maxLct;
private:
};


class CLinearInterpolator : public CInterpolator
{
public:
	virtual ~CLinearInterpolator();
protected:
	virtual CVector _position(double time);
	virtual CVector _speed(double time);
	virtual CVector _direction(double time);
	virtual void _autoAdjustLct();
private:
};

class CEntity;

class CExtendedInterpolator : public CLinearInterpolator
{
public:
	CExtendedInterpolator();
	virtual ~CExtendedInterpolator();
	void update();

	CVector	rotation() const;
	CVector right() const;
	double  facing() const;
	CMatrix getMatrix() const;
	
	void rotation(CVector rotation);
	void entity(CEntity *entity);
	
	void reset();
	

	static double RotLerp(double rsrc,double rdst,double pos);
	
private:
	
	CVector	_rotation(double deltaTime);

	CVector _currentRotation;
	CMatrix _currentMatrix;
	double  _currentFacing;
	double  _lastRotationTime;

	CEntity *_entity;
};

//#include "extrapolator.tpl"


#endif