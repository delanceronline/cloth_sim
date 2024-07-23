#include "particle.h"

Particle::Particle(void)
{
	mass = 0.0f;
	invMass = 0.0f;
	AirDragCoef = 0.0f;
	IsFixed = false;
}

Particle::~Particle(void)
{
}
