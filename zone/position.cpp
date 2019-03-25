#include "position.h"

#include <algorithm>
#include <string>
#include <cmath>
#include "../common/string_util.h"

static const float position_eps = 0.0001f;

std::string to_string(const glm::vec4 &position) {
	return StringFormat("(%.3f, %.3f, %.3f, %.3f)", position.x,position.y,position.z,position.w);
}

std::string to_string(const glm::vec3 &position){
	return StringFormat("(%.3f, %.3f, %.3f)", position.x,position.y,position.z);
}

std::string to_string(const glm::vec2 &position){
	return StringFormat("(%.3f, %.3f)", position.x,position.y);
}

bool IsOrigin(const glm::vec2 &position) {
	return glm::dot(position, position) == 0;
}

bool IsOrigin(const glm::vec3 &position) {
	return glm::dot(position, position) == 0;
}

bool IsOrigin(const glm::vec4 &position) {
	return IsOrigin(glm::vec3(position));
}

/**
* Produces the non square root'ed distance between the two points within the XY plane.
*/
float DistanceSquared(const glm::vec2& point1, const glm::vec2& point2) {
	auto diff = point1 - point2;
	return glm::dot(diff, diff);
}

/**
* Produces the distance between the two points on the XY plane.
*/
float Distance(const glm::vec2& point1, const glm::vec2& point2) {
	return std::sqrt(DistanceSquared(point1, point2));
}

/**
* Produces the non square root'ed distance between the two points.
*/
float DistanceSquared(const glm::vec3& point1, const glm::vec3& point2) {
	auto diff = point1 - point2;
	return glm::dot(diff, diff);
}

/**
* Produces the non square root'ed distance between the two points.
*/
float DistanceSquared(const glm::vec4& point1, const glm::vec4& point2) {
	return DistanceSquared(static_cast<glm::vec3>(point1), static_cast<glm::vec3>(point2));
}

/**
* Produces the distance between the two points.
*/
float Distance(const glm::vec3& point1, const glm::vec3& point2) {
	return std::sqrt(DistanceSquared(point1, point2));
}

/**
* Produces the distance between the two points.
*/
float Distance(const glm::vec4& point1, const glm::vec4& point2) {
	return Distance(static_cast<glm::vec3>(point1), static_cast<glm::vec3>(point2));
}

/**
* Produces the distance between the two points within the XY plane.
*/
float DistanceNoZ(const glm::vec3& point1, const glm::vec3& point2) {
	return Distance(static_cast<glm::vec2>(point1),static_cast<glm::vec2>(point2));
}

/**
* Produces the distance between the two points within the XY plane.
*/
float DistanceNoZ(const glm::vec4& point1, const glm::vec4& point2) {
	return Distance(static_cast<glm::vec2>(point1),static_cast<glm::vec2>(point2));
}

/**
* Produces the non square root'ed distance between the two points within the XY plane.
*/
float DistanceSquaredNoZ(const glm::vec3& point1, const glm::vec3& point2) {
	return DistanceSquared(static_cast<glm::vec2>(point1),static_cast<glm::vec2>(point2));
}

/**
* Produces the non square root'ed distance between the two points within the XY plane.
*/
float DistanceSquaredNoZ(const glm::vec4& point1, const glm::vec4& point2) {
	return DistanceSquared(static_cast<glm::vec2>(point1),static_cast<glm::vec2>(point2));
}

/**
* Determines if 'position' is within (inclusive) the axis aligned
* box (3 dimensional) formed from the points minimum and maximum.
*/
bool IsWithinAxisAlignedBox(const glm::vec3 &position, const glm::vec3 &minimum, const glm::vec3 &maximum) {
	auto actualMinimum = glm::vec3(std::min(minimum.x, maximum.x), std::min(minimum.y, maximum.y),std::min(minimum.z, maximum.z));
	auto actualMaximum = glm::vec3(std::max(minimum.x, maximum.x), std::max(minimum.y, maximum.y),std::max(minimum.z, maximum.z));

	bool xcheck = position.x >= actualMinimum.x && position.x <= actualMaximum.x;
	bool ycheck = position.y >= actualMinimum.y && position.y <= actualMaximum.y;
	bool zcheck = position.z >= actualMinimum.z && position.z <= actualMaximum.z;

	return xcheck && ycheck && zcheck;
}

/**
* Determines if 'position' is within (inclusive) the axis aligned
* box (2 dimensional) formed from the points minimum and maximum.
*/
bool IsWithinAxisAlignedBox(const glm::vec2 &position, const glm::vec2 &minimum, const glm::vec2 &maximum) {
	auto actualMinimum = glm::vec2(std::min(minimum.x, maximum.x), std::min(minimum.y, maximum.y));
	auto actualMaximum = glm::vec2(std::max(minimum.x, maximum.x), std::max(minimum.y, maximum.y));

	bool xcheck = position.x >= actualMinimum.x && position.x <= actualMaximum.x;
	bool ycheck = position.y >= actualMinimum.y && position.y <= actualMaximum.y;

	return xcheck && ycheck;
}

/**
* Gives the heading directly 180 degrees from the
* current heading.
* Takes the EQfloat from the glm::vec4 and returns
* an EQFloat.
*/
float GetReciprocalHeading(const glm::vec4& point1) {
	return GetReciprocalHeading(point1.w);
}

/**
* Gives the heading directly 180 degrees from the
* current heading.
* Takes an EQfloat and returns an EQFloat.
*/
float GetReciprocalHeading(const float heading)
{
	float result = 0;

	// Convert to radians
	float h = (heading / 512.0f) * 6.283184f;

	// Calculate the reciprocal heading in radians
	result = h + 3.141592f;

	// Convert back to eq heading from radians
	result = (result / 6.283184f) * 512.0f;

	return result;
}

bool IsHeadingEqual(const float h1, const float h2)
{
	return std::abs(h2 - h1) < 0.01f;
}

bool IsPositionEqual(const glm::vec2 &p1, const glm::vec2 &p2)
{
	return std::abs(p1.x - p2.x) < position_eps && std::abs(p1.y - p2.y) < position_eps;
}

bool IsPositionEqual(const glm::vec3 &p1, const glm::vec3 &p2)
{
	return std::abs(p1.x - p2.x) < position_eps && std::abs(p1.y - p2.y) < position_eps && std::abs(p1.z - p2.z) < position_eps;
}

bool IsPositionEqual(const glm::vec4 &p1, const glm::vec4 &p2)
{
	return std::abs(p1.x - p2.x) < position_eps && std::abs(p1.y - p2.y) < position_eps && std::abs(p1.z - p2.z) < position_eps;
}

bool IsPositionEqualWithinCertainZ(const glm::vec3 &p1, const glm::vec3 &p2, float z_eps) {
	return std::abs(p1.x - p2.x) < position_eps && std::abs(p1.y - p2.y) < position_eps && std::abs(p1.z - p2.z) < z_eps;
}

bool IsPositionEqualWithinCertainZ(const glm::vec4 &p1, const glm::vec4 &p2, float z_eps) {
	return std::abs(p1.x - p2.x) < position_eps && std::abs(p1.y - p2.y) < position_eps && std::abs(p1.z - p2.z) < z_eps;
}

bool IsPositionWithinSimpleCylinder(const glm::vec3 &p1, const glm::vec3 &cylinder_center, float cylinder_radius, float cylinder_height)
{
	//If we're outside the height of cylinder then we're not in it (duh)
	auto d = std::abs(p1.z - cylinder_center.z);
	if (d > cylinder_height / 2.0) {
		return false;
	}

	glm::vec2 p1d(p1.x, p1.y);
	glm::vec2 ccd(cylinder_center.x, cylinder_center.y);

	//If we're outside the radius of the cylinder then we're not in it (also duh)
	d = Distance(p1d, ccd);
	if (d > cylinder_radius) {
		return false;
	}

	return true;
}

bool IsPositionWithinSimpleCylinder(const glm::vec4 &p1, const glm::vec4 &cylinder_center, float cylinder_radius, float cylinder_height)
{
	//If we're outside the height of cylinder then we're not in it (duh)
	auto d = std::abs(p1.z - cylinder_center.z);
	if (d > cylinder_height / 2.0) {
		return false;
	}

	glm::vec2 p1d(p1.x, p1.y);
	glm::vec2 ccd(cylinder_center.x, cylinder_center.y);

	//If we're outside the radius of the cylinder then we're not in it (also duh)
	d = Distance(p1d, ccd);
	if (d > cylinder_radius) {
		return false;
	}

	return true;
}

float CalculateHeadingAngleBetweenPositions(float x1, float y1, float x2, float y2)
{
	float y_diff = std::abs(y1 - y2);
	float x_diff = std::abs(x1 - x2);
	if (y_diff < 0.0000009999999974752427)
		y_diff = 0.0000009999999974752427;

	float angle = atan2(x_diff, y_diff) * 180.0f * 0.3183099014828645f; // angle, nice "pi"

	// return the right thing based on relative quadrant
	// I'm sure this could be improved for readability, but whatever
	if (y1 >= y2) {
		if (x2 >= x1)
			return (90.0f - angle + 90.0f) * 511.5f * 0.0027777778f;
		if (x2 <= x1)
			return (angle + 180.0f) * 511.5f * 0.0027777778f;
	}
	if (y1 > y2 || x2 > x1)
		return angle * 511.5f * 0.0027777778f;
	else
		return (90.0f - angle + 270.0f) * 511.5f * 0.0027777778f;
}
