/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2002 EQEMu Development Team (http://eqemu.org)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/
#ifndef POSITION_H
#define POSITION_H

#include <string>

class xy_location {
public:
    float m_X;
    float m_Y;

    xy_location(float x = 0.0f, float y = 0.0f);

    const xy_location operator -(const xy_location& rhs);
};

class xyz_location {
public:
    float m_X;
    float m_Y;
    float m_Z;

    static const xyz_location& Origin() {static xyz_location origin; return origin;}

    xyz_location(float x = 0.0f, float y = 0.0f, float z = 0.0f);
    xyz_location(double x, double y, double z);

    operator xy_location() const;

    const xyz_location operator -(const xyz_location& rhs) const;

    void ABS_XYZ();
    bool isOrigin() const { return m_X == 0 && m_Y == 0 && m_Z == 0;}

};

class xyz_heading {
public:
    float m_X;
    float m_Y;
    float m_Z;

    float m_Heading;

    static const xyz_heading& Origin() {static xyz_heading origin; return origin;}

    xyz_heading(float x = 0.0f, float y = 0.0f, float z = 0.0f, float heading = 0.0f);
    xyz_heading(const xyz_heading& locationDir);
    xyz_heading(const xyz_location& locationDir, float heading = 0.0f);
    explicit xyz_heading(const xy_location& locationDir, float z, float heading);
    explicit xyz_heading(const xy_location locationDir, float z, float heading);

    operator xyz_location() const;
    operator xy_location() const;

    const xyz_heading operator +(const xyz_location& rhs) const;
    const xyz_heading operator +(const xy_location& rhs) const;

    const xyz_heading operator -(const xyz_location& rhs) const;

    void ABS_XYZ();
    bool isOrigin() const { return m_X == 0.0f && m_Y == 0.0f && m_Z == 0.0f;}
};

std::string to_string(const xyz_heading &position);
std::string to_string(const xyz_location &position);
std::string to_string(const xy_location &position);

#endif
