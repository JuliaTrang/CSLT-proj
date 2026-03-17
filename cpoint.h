#pragma once
#include "Library.h"
class Cpoint {
private:
	short x, y;

public:
	Cpoint();
	Cpoint(const short&, const short&);

	Cpoint& operator=(const Cpoint&);


	void set_cpointX(const short&);
	void set_cpointY(const short&);

	short get_cpointX() const;
	short get_cpointY() const;
	double calcDistance(const Cpoint&) const;

};