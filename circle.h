#pragma once
#include "cpoint.h"
class Circle {
private:
	Cpoint center;
	float radius;
public:
	Circle();
	Circle(Cpoint, float);
	Cpoint getCenter() const;
	float getRadius() const;
	void setCenter(short, short);
	void setRadius(float&);
};

