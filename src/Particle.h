#pragma once

#include "Vector3D.h"

class Particle
{
public:
	Particle(void);
	~Particle(void);

	bool IsFixed;
	float mass, invMass, AirDragCoef;
	Vector3D r, v, a;
};
