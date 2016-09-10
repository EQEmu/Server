#include <patch/patch.h>

namespace EQ
{
	namespace Patches
	{
		class LoginTitaniumPatch : public BasePatch
		{
		public:
			LoginTitaniumPatch();
			virtual ~LoginTitaniumPatch();
			virtual std::string GetName() const { return "Login Titanium"; }
		};
	}
}