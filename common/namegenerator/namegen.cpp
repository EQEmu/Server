/**
 *
 * @file A fantasy name generator library.
 * @version 1.0.1
 * @license Public Domain
 * @author German M. Bravo (Kronuz)
 *
 */

#include "namegen.h"

#include <algorithm>  // for move, reverse
#include <chrono>     // for rng seed
#include <cwchar>     // for size_t, mbsrtowcs, wcsrtombs
#include <cwctype>    // for towupper
#include <memory>     // for make_unique
#include <random>     // for mt19937, uniform_real_distribution
#include <stdexcept>  // for invalid_argument, out_of_range


using namespace NameGen;


static std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());


// https://isocpp.org/wiki/faq/ctors#static-init-order
// Avoid the "static initialization order fiasco"
const std::unordered_map<std::string, const std::vector<std::string>>& Generator::SymbolMap()
{
	static auto* const symbols = new std::unordered_map<std::string, const std::vector<std::string>>({
		{
			"s", {
				"ach", "ack", "ad", "age", "ald", "ale", "an", "ang", "ar", "ard",
				"as", "ash", "at", "ath", "augh", "aw", "ban", "bel", "bur", "cer",
				"cha", "che", "dan", "dar", "del", "den", "dra", "dyn", "ech", "eld",
				"elm", "em", "en", "end", "eng", "enth", "er", "ess", "est", "et",
				"gar", "gha", "hat", "hin", "hon", "ia", "ight", "ild", "im", "ina",
				"ine", "ing", "ir", "is", "iss", "it", "kal", "kel", "kim", "kin",
				"ler", "lor", "lye", "mor", "mos", "nal", "ny", "nys", "old", "om",
				"on", "or", "orm", "os", "ough", "per", "pol", "qua", "que", "rad",
				"rak", "ran", "ray", "ril", "ris", "rod", "roth", "ryn", "sam",
				"say", "ser", "shy", "skel", "sul", "tai", "tan", "tas", "ther",
				"tia", "tin", "ton", "tor", "tur", "um", "und", "unt", "urn", "usk",
				"ust", "ver", "ves", "vor", "war", "wor", "yer"
			}
		},
		{
			"v", {
				"a", "e", "i", "o", "u", "y"
			}
		},
		{
			"V", {
				"a", "e", "i", "o", "u", "y", "ae", "ai", "au", "ay", "ea", "ee",
				"ei", "eu", "ey", "ia", "ie", "oe", "oi", "oo", "ou", "ui"
			}
		},
		{
			"c", {
				"b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "q", "r",
				"s", "t", "v", "w", "x", "y", "z"
			}
		},
		{
			"B", {
				"b", "bl", "br", "c", "ch", "chr", "cl", "cr", "d", "dr", "f", "g",
				"h", "j", "k", "l", "ll", "m", "n", "p", "ph", "qu", "r", "rh", "s",
				"sch", "sh", "sl", "sm", "sn", "st", "str", "sw", "t", "th", "thr",
				"tr", "v", "w", "wh", "y", "z", "zh"
			}
		},
		{
			"C", {
				"b", "c", "ch", "ck", "d", "f", "g", "gh", "h", "k", "l", "ld", "ll",
				"lt", "m", "n", "nd", "nn", "nt", "p", "ph", "q", "r", "rd", "rr",
				"rt", "s", "sh", "ss", "st", "t", "th", "v", "w", "y", "z"
			}
		},
		{
			"i", {
				"air", "ankle", "ball", "beef", "bone", "bum", "bumble", "bump",
				"cheese", "clod", "clot", "clown", "corn", "dip", "dolt", "doof",
				"dork", "dumb", "face", "finger", "foot", "fumble", "goof",
				"grumble", "head", "knock", "knocker", "knuckle", "loaf", "lump",
				"lunk", "meat", "muck", "munch", "nit", "numb", "pin", "puff",
				"skull", "snark", "sneeze", "thimble", "twerp", "twit", "wad",
				"wimp", "wipe"
			}
		},
		{
			"m", {
				"baby", "booble", "bunker", "cuddle", "cuddly", "cutie", "doodle",
				"foofie", "gooble", "honey", "kissie", "lover", "lovey", "moofie",
				"mooglie", "moopie", "moopsie", "nookum", "poochie", "poof",
				"poofie", "pookie", "schmoopie", "schnoogle", "schnookie",
				"schnookum", "smooch", "smoochie", "smoosh", "snoogle", "snoogy",
				"snookie", "snookum", "snuggy", "sweetie", "woogle", "woogy",
				"wookie", "wookum", "wuddle", "wuddly", "wuggy", "wunny"
			}
		},
		{
			"M", {
				"boo", "bunch", "bunny", "cake", "cakes", "cute", "darling",
				"dumpling", "dumplings", "face", "foof", "goo", "head", "kin",
				"kins", "lips", "love", "mush", "pie", "poo", "pooh", "pook", "pums"
			}
		},
		{
			"D", {
				"b", "bl", "br", "cl", "d", "f", "fl", "fr", "g", "gh", "gl", "gr",
				"h", "j", "k", "kl", "m", "n", "p", "th", "w"
			}
		},
		{
			"d", {
				"elch", "idiot", "ob", "og", "ok", "olph", "olt", "omph", "ong",
				"onk", "oo", "oob", "oof", "oog", "ook", "ooz", "org", "ork", "orm",
				"oron", "ub", "uck", "ug", "ulf", "ult", "um", "umb", "ump", "umph",
				"un", "unb", "ung", "unk", "unph", "unt", "uzz"
			}
		}
	});

	return *symbols;
}

Generator::Generator()
{
}


Generator::Generator(std::vector<std::unique_ptr<Generator>>&& generators_) :
	generators(std::move(generators_))
{
}


size_t Generator::combinations()
{
	size_t total = 1;
	for (auto& g : generators) {
		total *= g->combinations();
	}
	return total;
}


size_t Generator::min()
{
	size_t final = 0;
	for (auto& g : generators) {
		final += g->min();
	}
	return final;
}


size_t Generator::max()
{
	size_t final = 0;
	for (auto& g : generators) {
		final += g->max();
	}
	return final;
}


std::string Generator::toString() {
	std::string str;
	for (auto& g : generators) {
		str.append(g->toString());
	}
	return str;
}


void Generator::add(std::unique_ptr<Generator>&& g)
{
	generators.push_back(std::move(g));
}


Random::Random()
{
}

Random::Random(std::vector<std::unique_ptr<Generator>>&& generators_) :
	Generator(std::move(generators_))
{
}

size_t Random::combinations()
{
	size_t total = 0;
	for (auto& g : generators) {
		total += g->combinations();
	}
	return total ? total : 1;
}

size_t Random::min()
{
	size_t final = -1;
	for (auto& g : generators) {
		size_t current = g->min();
		if (current < final) {
			final = current;
		}
	}
	return final;
}

size_t Random::max()
{
	size_t final = 0;
	for (auto& g : generators) {
		size_t current = g->max();
		if (current > final) {
			final = current;
		}
	}
	return final;
}


std::string Random::toString()
{
	if (!generators.size()) {
		return "";
	}
	std::uniform_real_distribution<double> distribution(0, generators.size() - 1);
	int rnd = distribution(rng) + 0.5;
	return generators[rnd]->toString();
}


Sequence::Sequence()
{
}

Sequence::Sequence(std::vector<std::unique_ptr<Generator>>&& generators_) :
	Generator(std::move(generators_))
{
}

Literal::Literal(const std::string &value_) :
	value(value_)
{
}

size_t Literal::combinations()
{
	return 1;
}

size_t Literal::min()
{
	return value.size();
}

size_t Literal::max()
{
	return value.size();
}

std::string Literal::toString()
{
	return value;
}

Reverser::Reverser(std::unique_ptr<Generator>&& g)
{
	add(std::move(g));
}


std::string Reverser::toString()
{
	std::wstring str = towstring(Generator::toString());
	std::reverse(str.begin(), str.end());
	return tostring(str);
}

Capitalizer::Capitalizer(std::unique_ptr<Generator>&& g)
{
	add(std::move(g));
}

std::string Capitalizer::toString()
{
	std::wstring str = towstring(Generator::toString());
	str[0] = std::towupper(str[0]);
	return tostring(str);
}


Collapser::Collapser(std::unique_ptr<Generator>&& g)
{
	add(std::move(g));
}

std::string Collapser::toString()
{
	std::wstring str = towstring(Generator::toString());
	std::wstring out;
	int cnt = 0;
	wchar_t pch = L'\0';
	for (auto ch : str) {
		if (ch == pch) {
			cnt++;
		} else {
			cnt = 0;
		}
		int mch = 2;
		switch(ch) {
			case 'a':
			case 'h':
			case 'i':
			case 'j':
			case 'q':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
				mch = 1;
		}
		if (cnt < mch) {
			out.push_back(ch);
		}
		pch = ch;
	}
	return tostring(out);
}


Generator::Generator(const std::string &pattern, bool collapse_triples) {
	std::unique_ptr<Generator> last;

	std::stack<std::unique_ptr<Group>> stack;
	std::unique_ptr<Group> top = std::unique_ptr<GroupSymbol>();

	for (auto c : pattern) {
		switch (c) {
			case '<':
				stack.push(std::move(top));
				top = std::unique_ptr<GroupSymbol>();
				break;
			case '(':
				stack.push(std::move(top));
				top = std::unique_ptr<GroupLiteral>();
				break;
			case '>':
			case ')':
				if (stack.size() == 0) {
					throw std::invalid_argument("Unbalanced brackets");
				} else if (c == '>' && top->type != group_types::symbol) {
					throw std::invalid_argument("Unexpected '>' in pattern");
				} else if (c == ')' && top->type != group_types::literal) {
					throw std::invalid_argument("Unexpected ')' in pattern");
				}
                                last = top->produce();
				top = std::move(stack.top());
				stack.pop();
				top->add(std::move(last));
				break;
			case '|':
				top->split();
				break;
			case '!':
				if (top->type == group_types::symbol) {
					top->wrap(wrappers::capitalizer);
				} else {
					top->add(c);
				}
				break;
			case '~':
				if (top->type == group_types::symbol) {
					top->wrap(wrappers::reverser);
				} else {
					top->add(c);
				}
				break;
			default:
				top->add(c);
				break;
		}
	}

	if (stack.size() != 0) {
		throw std::invalid_argument("Missing closing bracket");
	}

        std::unique_ptr<Generator> g = top->produce();
	if (collapse_triples) {
		g = std::unique_ptr<Collapser>(new Collapser(std::move(g)));
	}
	add(std::move(g));
}


Generator::Group::Group(group_types_t type_) :
	type(type_)
{
}

void Generator::Group::add(std::unique_ptr<Generator>&& g)
{
	while (!wrappers.empty()) {
		switch (wrappers.top()) {
			case reverser:
				g = std::unique_ptr<Reverser>(new Reverser(std::move(g)));
				break;
			case capitalizer:
				g = std::unique_ptr<Capitalizer>(new Capitalizer(std::move(g)));
				break;
		}
		wrappers.pop();
	}
	if (set.size() == 0) {
		set.push_back(std::unique_ptr<Sequence>());
	}
	set.back()->add(std::move(g));
}

void Generator::Group::add(char c)
{
	std::string value(1, c);
	std::unique_ptr<Generator> g = std::unique_ptr<Random>();
	g->add(std::unique_ptr<Literal>(new Literal(value)));
	Group::add(std::move(g));
}

std::unique_ptr<Generator> Generator::Group::produce()
{
	switch (set.size()) {
		case 0:
			return std::unique_ptr<Literal>(new Literal(""));
		case 1:
			return std::move(*set.begin());
		default:
			return std::unique_ptr<Random>(new Random(std::move(set)));
	}
}

void Generator::Group::split()
{
	if (set.size() == 0) {
		set.push_back(std::unique_ptr<Sequence>());
	}
	set.push_back(std::unique_ptr<Sequence>());
}

void Generator::Group::wrap(wrappers_t type)
{
	wrappers.push(type);
}

Generator::GroupSymbol::GroupSymbol() :
	Group(group_types::symbol)
{
}

void Generator::GroupSymbol::add(char c)
{
	std::string value(1, c);
	std::unique_ptr<Generator> g = std::unique_ptr<Random>();
	try {
		static const auto& symbols = SymbolMap();
		for (const auto& s : symbols.at(value)) {
			g->add(std::unique_ptr<Literal>(new Literal(s)));
		}
	} catch (const std::out_of_range&) {
		g->add(std::unique_ptr<Literal>(new Literal(value)));
	}
	Group::add(std::move(g));
}

Generator::GroupLiteral::GroupLiteral() :
	Group(group_types::literal)
{
}

std::wstring towstring(const std::string & s)
{
	const char *cs = s.c_str();
	const size_t wn = std::mbsrtowcs(nullptr, &cs, 0, nullptr);

	if (wn == static_cast<size_t>(-1)) {
		return L"";
	}

	std::vector<wchar_t> buf(wn);
	cs = s.c_str();
	const size_t wn_again = std::mbsrtowcs(buf.data(), &cs, wn, nullptr);

	if (wn_again == static_cast<size_t>(-1)) {
		return L"";
	}

	return std::wstring(buf.data(), wn);
}

std::string tostring(const std::wstring & s)
{
	const wchar_t *cs = s.c_str();
	const size_t wn = std::wcsrtombs(nullptr, &cs, 0, nullptr);

	if (wn == static_cast<size_t>(-1)) {
		return "";
	}

	std::vector<char> buf(wn);
	const size_t wn_again = std::wcsrtombs(buf.data(), &cs, wn, nullptr);

	if (wn_again == static_cast<size_t>(-1)) {
		return "";
	}

	return std::string(buf.data(), wn);
}
