#include <math.h>
#include <stdio.h>
#include "gpoint.h"


GPoint::GPoint() {
	x = 0;
	y = 0;
	z = 0;
}

GPoint::GPoint(VERTEX &v) {
	x = v.x;
	y = v.y;
	z = v.z;
}

GPoint::GPoint(float ix, float iy, float iz) {
	x = ix;
	y = iy;
	z = iz;
}

GPoint::GPoint(const GPoint &them) {
	x = them.x;
	y = them.y;
	z = them.z;
}

//dot of x,y,z
float GPoint::dot3(const GPoint &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z));
}

//cross product
GPoint GPoint::cross(const GPoint &them) const {
	return(GPoint(y * them.z - z * them.y,
					 z * them.x - x * them.z,
					 x * them.y - y * them.x));
}


const GPoint &GPoint::operator+=(const GPoint &them) {
	x += them.x;
	y += them.y;
	z += them.z;
	return(*this);
}
const GPoint &GPoint::operator*=(const float num) {
	x *= num;
	y *= num;
	z *= num;
	return(*this);
}

GPoint operator-(const GPoint &v1, const GPoint &v2) {
	return(GPoint(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z));
}

//ordering on X only
bool operator<(const GPoint &v1, const GPoint &v2) {
	return(v1.x < v2.x);
}

//ordering on X only
bool operator>(const GPoint &v1, const GPoint &v2) {
	return(v1.x > v2.x);
}


GVector::GVector() : GPoint() {
	W = 0;
}

GVector::GVector(const GPoint &them) : GPoint(them) {
	W = 1.0f;
}

GVector::GVector(const GPoint &from, const GPoint &to)
: GPoint(to.x - from.x, to.y - from.y, to.z - from.z)
{
	W = 1.0f;
}

GVector::GVector(float x, float y, float z, float w) : GPoint(x, y, z) {
	W = w;
}

//dot product of x,y,z,w
float GVector::dot4(const GVector &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z) + (W * them.W));
}

//dot product of x,y,z+w
float GVector::dot4(const GPoint &them) const {
	return((x * them.x) + (y * them.y) +
			 (z * them.z) + W);
}

float GVector::length() {
	return(sqrt((x * x) + (y * y) + (z * z)));
}

void GVector::normalize() {
	float len = length();	//stupid square roots take forever
	x /= len;
	y /= len;
	z /= len;
}


