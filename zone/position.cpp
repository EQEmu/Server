#include "position.h"
#include <string>
#include "../common/string_util.h"

xy_location::xy_location(float x, float y) :
    m_X(x),
    m_Y(y) {
}

const xy_location xy_location::operator -(const xy_location& rhs) {
    xy_location minus(m_X - rhs.m_X, m_Y - rhs.m_Y);
    return minus;
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


xyz_heading::xyz_heading(const xyz_location locationDir, float heading) :
    m_X(locationDir.m_X),
    m_Y(locationDir.m_Y),
    m_Z(locationDir.m_Z),
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

const xyz_heading xyz_heading::operator +(const xyz_location& rhs) {
    return xyz_heading(m_X + rhs.m_X, m_Y + rhs.m_Y, m_Z + rhs.m_Z, m_Heading);
}

const xyz_heading xyz_heading::operator +(const xy_location& rhs) {
    return xyz_heading(m_X + rhs.m_X, m_Y + rhs.m_Y, m_Z, m_Heading);
}

const xyz_heading xyz_heading::operator -(const xyz_location& rhs) {
    return xyz_heading(m_X - rhs.m_X, m_Y - rhs.m_Y, m_Z - rhs.m_Z, m_Heading);
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

const xyz_location xyz_location::operator -(const xyz_location& rhs) {
    return xyz_location(m_X - rhs.m_X, m_Y - rhs.m_Y, m_Z - rhs.m_Z);
}

void xyz_location::ABS_XYZ(void) {
    if (m_X < 0)
        m_X = -m_X;

    if (m_Y < 0)
        m_Y = -m_Y;

    if (m_Z < 0)
        m_Z = -m_Z;
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

