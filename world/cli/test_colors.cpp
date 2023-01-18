#include "../../common/zone_store.h"
#include "../../common/termcolor/rang.hpp"

void WorldserverCLI::TestColors(int argc, char **argv, argh::parser &cmd, std::string &description)
{
	description = "Test command";

	if (cmd[{"-h", "--help"}]) {
		return;
	}

	using namespace std;
	using namespace rang;

	// Because rang use static values it means that all redirections should be done
	// before calling rang functions.

	// Visual test for background colors
	cout << bg::green
		 << "This text has green background." << bg::reset << endl
		 << bg::red << "This text has red background." << bg::reset << endl
		 << bg::black << "This text has black background." << bg::reset << endl
		 << bg::yellow << "This text has yellow background." << bg::reset
		 << endl
		 << bg::blue << "This text has blue background." << bg::reset << endl
		 << bg::magenta << "This text has magenta background." << bg::reset
		 << endl
		 << bg::cyan << "This text has cyan background." << bg::reset << endl
		 << bg::gray << fg::black << "This text has gray background."
		 << bg::reset << style::reset << endl

		 << endl

		 // Visual test for foreground colors
		 << fg::green << "This text has green color." << fg::reset << endl
		 << fg::red << "This text has red color." << fg::reset << endl
		 << fg::black << bg::gray << "This text has black color." << fg::reset
		 << bg::reset << endl
		 << fg::yellow << "This text has yellow color." << fg::reset << endl
		 << fg::blue << "This text has blue color." << fg::reset << endl
		 << fg::magenta << "This text has magenta color." << fg::reset << endl
		 << fg::cyan << "This text has cyan color." << fg::reset << endl
		 << fg::gray << "This text has gray color." << style::reset << endl

		 << endl

		 // Visual test for bright background colors
		 << bgB::green << fg::black << "This text has bright green background."
		 << style::reset << endl
		 << bgB::red << "This text has bright red background." << style::reset
		 << endl
		 << bgB::black << "This text has bright black background."
		 << style::reset << endl
		 << bgB::yellow << fg::black
		 << "This text has bright yellow background." << style::reset << endl
		 << bgB::blue << "This text has bright blue background." << style::reset
		 << endl
		 << bgB::magenta << "This text has bright magenta background."
		 << style::reset << endl
		 << bgB::cyan << "This text has bright cyan background." << style::reset
		 << endl
		 << bgB::gray << fg::black << "This text has bright gray background."
		 << style::reset << style::reset << endl

		 << endl

		 // Visual test for bright foreground colors
		 << fgB::green << "This text has bright green color." << endl
		 << fgB::red << "This text has bright red color." << endl
		 << fgB::black << "This text has bright black color." << endl
		 << fgB::yellow << "This text has bright yellow color." << endl
		 << fgB::blue << "This text has bright blue color." << endl
		 << fgB::magenta << "This text has bright magenta color." << endl
		 << fgB::cyan << "This text has bright cyan color." << endl
		 << fgB::gray << "This text has bright gray color." << style::reset
		 << endl

		 << endl

		 // Visual test for text effects
		 << style::bold << "This text is bold." << style::reset << endl
		 << style::dim << "This text is dim." << style::reset << endl
		 << style::italic << "This text is italic." << style::reset << endl
		 << style::underline << "This text is underlined." << style::reset
		 << endl
		 << style::blink << "This text text has blink effect." << style::reset
		 << endl
		 << style::rblink << "This text text has rapid blink effect."
		 << style::reset << endl
		 << style::reversed << "This text is reversed." << style::reset << endl
		 << style::conceal << "This text is concealed." << style::reset << endl
		 << style::crossed << "This text is crossed."
		 << style::reset << endl;
}
