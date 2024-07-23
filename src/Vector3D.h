// Vector3D.h: interface for the Vector3D class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VECTOR3D_H__65398D65_1BEF_4866_9650_8E38CF8FA767__INCLUDED_)
#define AFX_VECTOR3D_H__65398D65_1BEF_4866_9650_8E38CF8FA767__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Math.h>

class Vector3D  
{
public:
	Vector3D TimesScalar(float t);
	Vector3D Normalize(void);
	float Mag(void);
	Vector3D();
	Vector3D(float in_x, float in_y, float in_z);
	float x, y, z;
	float operator*(const Vector3D& v2);
	Vector3D operator*(const float& t);
	Vector3D CrossProduct(Vector3D *v_in);
	Vector3D operator+(const Vector3D& v2);
	Vector3D operator-(const Vector3D& v2);
	virtual ~Vector3D();
};

#endif // !defined(AFX_VECTOR3D_H__65398D65_1BEF_4866_9650_8E38CF8FA767__INCLUDED_)
