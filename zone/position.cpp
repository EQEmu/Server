#include <string>
#include <cmath>
#include "position.h"
#include "../common/string_util.h"
#include <algorithm>

xy_location::xy_location(float x, float y) :
    m_X(x),
    m_Y(y) {
}

xy_location xy_location::operator -(const xy_location& rhs) const {
    xy_location minus(m_X - rhs.m_X, m_Y - rhs.m_Y);
    return minus;
}

xy_location xy_location::operator +(const xy_location& rhs) const {
    xy_location addition(m_X + rhs.m_X, m_Y + rhs.m_Y);
    return addition;
}

xyz_heading::xyz_heading(float x, float y, float z, float heading) :
    m_X(x),
    m_Y(y),
    m_Z(z),
    m_Heading(heading) {
}

xyz_heading::xyz_heading(const xyz_heading& locationDir) :
    m_X(locationDir.m_X),
    m_Y(locationDir.m_Y),
    m_Z(locationDir.m_Z),
    m_Heading(locationDir.m_Heading) {
}

xyz_heading::xyz_heading(const xyz_location& locationDir, float heading) :
    m_X(locationDir.m_X),
    m_Y(locationDir.m_Y),
    m_Z(locationDir.m_Z),
    m_Heading(heading) {
}

xyz_heading::xyz_heading(const xy_location& locationDir, float z, float heading) :
    m_X(locationDir.m_X),
    m_Y(locationDir.m_Y),
    m_Z(z),
    m_Heading(heading) {
}

xyz_heading::xyz_heading(const xy_location locationDir, float z, float heading) :
    m_X(locationDir.m_X),
    m_Y(locationDir.m_Y),
    m_Z(z),
    m_Heading(heading) {
}

xyz_heading::operator xyz_location() const {
    return xyz_location(m_X,m_Y,m_Z);
}

xyz_heading::operator xy_location() const {
    return xy_location(m_X,m_Y);
}

const xyz_heading xyz_heading::operator +(const xyz_location& rhs) const{
    return xyz_heading(m_X + rhs.m_X, m_Y + rhs.m_Y, m_Z + rhs.m_Z, m_Heading);
}

const xyz_heading xyz_heading::operator +(const xy_location& rhs) const{
    return xyz_heading(m_X + rhs.m_X, m_Y + rhs.m_Y, m_Z, m_Heading);
}

const xyz_heading xyz_heading::operator -(const xyz_location& rhs) const{
    return xyz_heading(m_X - rhs.m_X, m_Y - rhs.m_Y, m_Z - rhs.m_Z, m_Heading);
}

void xyz_heading::ABS_XYZ(void) {
    m_X = abs(m_X);
    m_Y = abs(m_Y);
    m_Z = abs(m_Z);
}

xyz_location::xyz_location(float x, float y, float z) :
    m_X(x),
    m_Y(y),
    m_Z(z) {
}

xyz_location::xyz_location(double x, double y, double z) :
    m_X(static_cast<float>(x)),
    m_Y(static_cast<float>(y)),
    m_Z(static_cast<float>(z)) {
}

xyz_location::operator xy_location() const {
    return xy_location(m_X, m_Y);
}

xyz_location xyz_location::operator -(const xyz_location& rhs) const {
    return xyz_location(m_X - rhs.m_X, m_Y - rhs.m_Y, m_Z - rhs.m_Z);
}

xyz_location xyz_location::operator +(const xyz_location& rhs) const {
    return xyz_location(m_X + rhs.m_X, m_Y + rhs.m_Y, m_Z + rhs.m_Z);
}

void xyz_location::ABS_XYZ(void) {
    m_X = abs(m_X);
    m_Y = abs(m_Y);
    m_Z = abs(m_Z);
}

std::string to_string(const xyz_heading &position) {
    return StringFormat("(%.3f, %.3f, %.3f, %.3f)", position.m_X,position.m_Y,position.m_Z,position.m_Heading);
}

std::string to_string(const xyz_location &position){
    return StringFormat("(%.3f, %.3f, %.3f)", position.m_X,position.m_Y,position.m_Z);
}

std::string to_string(const xy_location &position){
    return StringFormat("(%.3f, %.3f)", position.m_X,position.m_Y);
}

/**
* Produces the non square root'ed distance between the two points within the XY plane.
*/
float ComparativeDistance(const xy_location& point1, const xy_location& point2) {
    auto diff = point1 - point2;
    return diff.m_X * diff.m_X + diff.m_Y * diff.m_Y;
}

/**
* Produces the distance between the two points on the XY plane.
*/
float Distance(const xy_location& point1, const xy_location& point2) {
    return sqrt(ComparativeDistance(point1, point2));
}

/**
* Produces the non square root'ed distance between the two points.
*/
float ComparativeDistance(const xyz_location& point1, const xyz_location& point2) {
    auto diff = point1 - point2;
    return diff.m_X * diff.m_X + diff.m_Y * diff.m_Y + diff.m_Z * diff.m_Z;
}

/**
* Produces the non square root'ed distance between the two points.
*/
float ComparativeDistance(const xyz_heading& point1, const xyz_heading& point2) {
    return ComparativeDistance(static_cast<xyz_location>(point1), static_cast<xyz_location>(point2));
}

/**
* Produces the distance between the two points.
*/
float Distance(const xyz_location& point1, const xyz_location& point2) {
    return sqrt(ComparativeDistance(point1, point2));
}

/**
* Produces the distance between the two points.
*/
float Distance(const xyz_heading& point1, const xyz_heading& point2) {
    return Distance(static_cast<xyz_location>(point1), static_cast<xyz_location>(point2));
}

/**
* Produces the distance between the two points within the XY plane.
*/
float DistanceNoZ(const xyz_location& point1, const xyz_location& point2) {
    return Distance(static_cast<xy_location>(point1),static_cast<xy_location>(point2));
}

/**
* Produces the distance between the two points within the XY plane.
*/
float DistanceNoZ(const xyz_heading& point1, const xyz_heading& point2) {
    return Distance(static_cast<xy_location>(point1),static_cast<xy_location>(point2));
}

/**
* Produces the non square root'ed distance between the two points within the XY plane.
*/
float ComparativeDistanceNoZ(const xyz_location& point1, const xyz_location& point2) {
    return ComparativeDistance(static_cast<xy_location>(point1),static_cast<xy_location>(point2));
}

/**
* Produces the non square root'ed distance between the two points within the XY plane.
*/
float ComparativeDistanceNoZ(const xyz_heading& point1, const xyz_heading& point2) {
    return ComparativeDistance(static_cast<xy_location>(point1),static_cast<xy_location>(point2));
}

/**
* Determines if 'position' is within (inclusive) the axis aligned
* box (3 dimensional) formed from the points minimum and maximum.
*/
bool IsWithinAxisAlignedBox(const xyz_location &position, const xyz_location &minimum, const xyz_location &maximum) {
    auto actualMinimum = xyz_location(std::min(minimum.m_X, maximum.m_X), std::min(minimum.m_Y, maximum.m_Y),std::min(minimum.m_Z, maximum.m_Z));
    auto actualMaximum = xyz_location(std::max(minimum.m_X, maximum.m_X), std::max(minimum.m_Y, maximum.m_Y),std::max(minimum.m_Z, maximum.m_Z));

    bool xcheck = position.m_X >= actualMinimum.m_X && position.m_X <= actualMaximum.m_X;
    bool ycheck = position.m_Y >= actualMinimum.m_Y && position.m_Y <= actualMaximum.m_Y;
    bool zcheck = position.m_Z >= actualMinimum.m_Z && position.m_Z <= actualMaximum.m_Z;

    return xcheck && ycheck && zcheck;
}

/**
* Determines if 'position' is within (inclusive) the axis aligned
* box (2 dimensional) formed from the points minimum and maximum.
*/
bool IsWithinAxisAlignedBox(const xy_location &position, const xy_location &minimum, const xy_location &maximum) {
    auto actualMinimum = xy_location(std::min(minimum.m_X, maximum.m_X), std::min(minimum.m_Y, maximum.m_Y));
    auto actualMaximum = xy_location(std::max(minimum.m_X, maximum.m_X), std::max(minimum.m_Y, maximum.m_Y));

    bool xcheck = position.m_X >= actualMinimum.m_X && position.m_X <= actualMaximum.m_X;
    bool ycheck = position.m_Y >= actualMinimum.m_Y && position.m_Y <= actualMaximum.m_Y;

    return xcheck && ycheck;
}

/**
* Gives the heading directly 180 degrees from the
* current heading.
* Takes the EQfloat from the xyz_heading and returns
* an EQFloat.
*/
float GetReciprocalHeading(const xyz_heading& point1) {
    return GetReciprocalHeading(point1.m_Heading);
}

/**
* Gives the heading directly 180 degrees from the
* current heading.
* Takes an EQfloat and returns an EQFloat.
*/
float GetReciprocalHeading(const float heading) {
    float result = 0;

    // Convert to radians
    float h = (heading / 256.0f) * 6.283184f;

    // Calculate the reciprocal heading in radians
    result = h + 3.141592f;

    // Convert back to eq heading from radians
    result = (result / 6.283184f) * 256.0f;

    return result;
}