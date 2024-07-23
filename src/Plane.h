#pragma once

#include "Vector3D.h"

class Plane
{
public:
	Plane(void);
	~Plane(void);

	Vector3D p, n;
};
