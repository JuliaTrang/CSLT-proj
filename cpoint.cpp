#include "cpoint.h"
using namespace std;

Cpoint::Cpoint() : x(0), y(0) {}

Cpoint::Cpoint(const short& init_x, const short& init_y) : x(init_x), y(init_y) {};

Cpoint& Cpoint::operator=(const Cpoint& p) {
    if (this != &p) {
        x = p.x;
        y = p.y;
    }
    return *this;
}

void Cpoint::set_cpointX(const short& setX) {
    x = setX;
};
void Cpoint::set_cpointY(const short& setY) {
    y = setY;
};
short Cpoint::get_cpointX() const {
    return x;
};
short Cpoint::get_cpointY() const {
    return y;
};
double Cpoint::calcDistance(const Cpoint& p) const {
    return sqrt((x - p.x) * (x - p.x) + (y - p.y) * (y - p.y));
};