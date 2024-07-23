// Vector3D.cpp: implementation of the Vector3D class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Vector3D.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Vector3D::Vector3D()
{
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

Vector3D::Vector3D(float in_x, float in_y, float in_z)
{
	x = in_x;
	y = in_y;
	z = in_z;
}

Vector3D::~Vector3D()
{

}

float Vector3D::Mag()
{
	float var = powf(x * x + y * y + z * z, 0.5f);

	if(var > -1e-5 && var < 1e-5)
		return 0.0f;
	else
		return var;
}

float Vector3D::operator*(const Vector3D& v2)
{
  return x * v2.x + y * v2.y + z * v2.z;
}

Vector3D Vector3D::operator*(const float& t)
{
  return Vector3D(x * t, y * t, z * t);
}

Vector3D Vector3D::CrossProduct(Vector3D *v_in)
{
	Vector3D v3;

	v3.x = y * v_in->z - z * v_in->y;
	v3.y = z * v_in->x - x * v_in->z;
	v3.z = x * v_in->y - y * v_in->x;

	return v3;
}

Vector3D Vector3D::operator+(const Vector3D& v2)
{
  return Vector3D(x + v2.x, y + v2.y, z + v2.z);
}

Vector3D Vector3D::operator-(const Vector3D& v2)
{
  return Vector3D(x - v2.x, y - v2.y, z - v2.z);
}

Vector3D Vector3D::Normalize()
{
	float mag = Mag();

	if(mag > -1e-5 && mag < 1e-5)
		return Vector3D(0.0f, 0.0f, 0.0f);
	else
		return Vector3D(x / Mag(), y / Mag(), z / Mag());
}

Vector3D Vector3D::TimesScalar(float t)
{
	Vector3D v;

	v.x = x * t;
	v.y = y * t;
	v.z = z * t;

	return v;
}
