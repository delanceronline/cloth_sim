#pragma once

class Spring
{
public:
	Spring(void);
	~Spring(void);

	float k, length, extension, DampCoef;
	int nObj1, nObj2;
};
