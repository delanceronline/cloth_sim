#include "spring.h"

Spring::Spring(void)
{
	k = 0.0f;
	length = 0.0f;
	extension = 0.0f;
	DampCoef = 0.0f;
	nObj1 = 0;
	nObj2 = 0;
}

Spring::~Spring(void)
{
}
