#include <patch/patch.h>

namespace EQ
{
	namespace Patches
	{
		class ChatPatch : public BasePatch
		{
		public:
			ChatPatch();
			virtual ~ChatPatch();
			virtual std::string GetName() const { return "Chat"; }
		};
	}
}