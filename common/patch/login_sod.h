#include "../patch/patch.h"

namespace EQ
{
	namespace Patches
	{
		class LoginSoDPatch : public BasePatch
		{
		public:
			LoginSoDPatch();
			virtual ~LoginSoDPatch();
			virtual std::string GetName() const { return "Login SoD+"; }
		};
	}
}