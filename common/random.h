/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2014 EQEMu Development Team (http://eqemulator.net)

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

#ifndef __random_h__
#define __random_h__

#include <random>
#include <utility>

/* This uses mt19937 seeded with the std::random_device
 * The idea is to have this be included as a member of another class
 * so mocking out for testing is easier
 * If you need to reseed random.Reseed()
 * Eventually this should be derived from an abstract base class
 */

namespace EQEmu {
	class Random {
	public:
		// AKA old MakeRandomInt
		const int Int(int low, int high)
		{
			if (low > high)
				std::swap(low, high);
			return std::uniform_int_distribution<int>(low, high)(m_gen); // [low, high]
		}

		// AKA old MakeRandomFloat
		const double Real(double low, double high)
		{
			if (low > high)
				std::swap(low, high);
			return std::uniform_real_distribution<double>(low, high)(m_gen); // [low, high)
		}

		// example Roll(50) would have a 50% success rate
		// Roll(100) 100%, etc
		// valid values 0-100 (well, higher works too but ...)
		const bool Roll(const int required)
		{
			return Int(0, 99) < required;
		}

		// valid values 0.0 - 1.0
		const bool Roll(const double required)
		{
			return Real(0.0, 1.0) <= required;
		}

		void Reseed()
		{
			// We could do the seed_seq thing here too if we need better seeding
			// but that is mostly overkill for us, so just seed once
			std::random_device rd;
			m_gen.seed(rd());
		}

		Random()
		{
			Reseed();
		}

	private:
		std::mt19937 m_gen;
	};
}

#endif /* !__random_h__ */

