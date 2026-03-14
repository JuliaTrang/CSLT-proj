#include "circle.h"
using namespace std;
Circle::Circle() {
	radius = 0;
	center = { -1,-1 };
}
Circle::Circle(Cpoint center, float radius) {
	this->center = center;
	this->radius = radius;
}
void Circle::setCenter(short x, short y) {
	center.set_cpointX(x);
	center.set_cpointY(y);
}
Cpoint Circle::getCenter() const {
	return center;
}
void Circle::setRadius(float& radius) {
	this->radius = radius;
}
float Circle::getRadius() const {
	return radius;
}