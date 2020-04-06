/**
 *
 * @file A fantasy name generator library.
 * @version 1.0.1
 * @license Public Domain
 * @author German M. Bravo (Kronuz)
 *
 * This library is designed after the RinkWorks Fantasy Name Generator.
 * @see http://www.rinkworks.com/namegen/
 *
 * @example
 * NameGen::Generator generator("sV'i");
 * generator.toString();  // Returns a new name each call with produce()
 * // => "entheu'loaf"
 *
 * ## Pattern Syntax
 *
 *   The compile() function creates a name generator based on an input
 * pattern. The letters s, v, V, c, B, C, i, m, M, D, and d represent
 * different types of random replacements. Everything else is produced
 * literally.
 *
 *   s - generic syllable
 *   v - vowel
 *   V - vowel or vowel combination
 *   c - consonant
 *   B - consonant or consonant combination suitable for beginning a word
 *   C - consonant or consonant combination suitable anywhere in a word
 *   i - insult
 *   m - mushy name
 *   M - mushy name ending
 *   D - consonant suited for a stupid person's name
 *   d - syllable suited for a stupid person's name (begins with a vowel)
 *
 *   All characters between parenthesis () are produced literally. For
 * example, the pattern "s(dim)", produces a random generic syllable
 * followed by "dim".
 *
 *   Characters between angle brackets <> produce patterns from the table
 * above. Imagine the entire pattern is wrapped in one of these.
 *
 *   In both types of groupings, a vertical bar | denotes a random
 * choice. Empty groups are allowed. For example, "(foo|bar)" produces
 * either "foo" or "bar". The pattern "<c|v|>" produces a constant,
 * vowel, or nothing at all.
 *
 *   An exclamation point ! means to capitalize the component that
 * follows it. For example, "!(foo)" will produce "Foo" and "v!s" will
 * produce a lowercase vowel followed by a capitalized syllable, like
 * "eRod".
 *
 *   A tilde ~ means to reverse the letters of the component that
 * follows it. For example, "~(foo)" will produce "oof". To reverse an
 * entire template, wrap it in brackets. For example, to reverse
 * "sV'i" as a whole use "~<sV'i>". The template "~sV'i" will only
 * reverse the initial syllable.
 *
 * ## Internals
 *
 *   A name generator is anything with a toString() method, including,
 * importantly, strings themselves. The generator constructors
 * (Random, Sequence) perform additional optimizations when *not* used
 * with the `new` keyword: they may pass through a provided generator,
 * combine provided generators, or even return a simple string.
 *
 *   New pattern symbols added to Generator.symbols will automatically
 * be used by the compiler.
 */

#pragma once

#include <stddef.h>       // for size_t
#include <iosfwd>         // for wstring
#include <memory>         // for unique_ptr
#include <stack>          // for stack
#include <string>         // for string
#include <unordered_map>  // for unordered_map
#include <vector>         // for vector


namespace NameGen {

// Middle Earth
#define MIDDLE_EARTH "(bil|bal|ban|hil|ham|hal|hol|hob|wil|me|or|ol|od|gor|for|fos|tol|ar|fin|ere|leo|vi|bi|bren|thor)(|go|orbis|apol|adur|mos|ri|i|na|ole|n)(|tur|axia|and|bo|gil|bin|bras|las|mac|grim|wise|l|lo|fo|co|ra|via|da|ne|ta|y|wen|thiel|phin|dir|dor|tor|rod|on|rdo|dis)"

// Japanese Names (Constrained)
#define JAPANESE_NAMES_CONSTRAINED "(aka|aki|bashi|gawa|kawa|furu|fuku|fuji|hana|hara|haru|hashi|hira|hon|hoshi|ichi|iwa|kami|kawa|ki|kita|kuchi|kuro|marui|matsu|miya|mori|moto|mura|nabe|naka|nishi|no|da|ta|o|oo|oka|saka|saki|sawa|shita|shima|i|suzu|taka|take|to|toku|toyo|ue|wa|wara|wata|yama|yoshi|kei|ko|zawa|zen|sen|ao|gin|kin|ken|shiro|zaki|yuki|asa)(||||||||||bashi|gawa|kawa|furu|fuku|fuji|hana|hara|haru|hashi|hira|hon|hoshi|chi|wa|ka|kami|kawa|ki|kita|kuchi|kuro|marui|matsu|miya|mori|moto|mura|nabe|naka|nishi|no|da|ta|o|oo|oka|saka|saki|sawa|shita|shima|suzu|taka|take|to|toku|toyo|ue|wa|wara|wata|yama|yoshi|kei|ko|zawa|zen|sen|ao|gin|kin|ken|shiro|zaki|yuki|sa)"

// Japanese Names (Diverse)
#define JAPANESE_NAMES_DIVERSE "(a|i|u|e|o|||||)(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|ro|ro|wa|wa|wa|wa|wo|wo)(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|ro|ro|wa|wa|wa|wa|wo|wo)(|(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|ro|ro|wa|wa|wa|wa|wo|wo)|(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|ro|ro|wa|wa|wa|wa|wo|wo)(|(ka|ki|ki|ku|ku|ke|ke|ko|ko|sa|sa|sa|shi|shi|shi|su|su|se|so|ta|ta|chi|chi|tsu|te|to|na|ni|ni|nu|nu|ne|no|no|ha|hi|fu|fu|he|ho|ma|ma|ma|mi|mi|mi|mu|mu|mu|mu|me|mo|mo|mo|ya|yu|yu|yu|yo|ra|ra|ra|ri|ru|ru|ru|re|ro|ro|ro|wa|wa|wa|wa|wo|wo)))(|||n)"

// Chinese Names
#define CHINESE_NAMES "(zh|x|q|sh|h)(ao|ian|uo|ou|ia)(|(l|w|c|p|b|m)(ao|ian|uo|ou|ia)(|n)|-(l|w|c|p|b|m)(ao|ian|uo|ou|ia)(|(d|j|q|l)(a|ai|iu|ao|i)))"

// Greek Names
#define GREEK_NAMES "<s<v|V>(tia)|s<v|V>(os)|B<v|V>c(ios)|B<v|V><c|C>v(ios|os)>"

// Hawaiian Names (1)
#define HAWAIIAN_NAMES_1 "((h|k|l|m|n|p|w|')|)(a|e|i|o|u)((h|k|l|m|n|p|w|')|)(a|e|i|o|u)(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)(((h|k|l|m|n|p|w|')|)(a|e|i|o|u)|)"

// Hawaiian Names (2)
#define HAWAIIAN_NAMES_2 "((h|k|l|m|n|p|w|)(a|e|i|o|u|a'|e'|i'|o'|u'|ae|ai|ao|au|oi|ou|eu|ei)(k|l|m|n|p|)|)(h|k|l|m|n|p|w|)(a|e|i|o|u|a'|e'|i'|o'|u'|ae|ai|ao|au|oi|ou|eu|ei)(k|l|m|n|p|)"

// Old Latin Place Names
#define OLD_LATIN_PLACE_NAMES "sv(nia|lia|cia|sia)"

// Dragons (Pern)
#define DRAGONS_PERN "<<s|ss>|<VC|vC|B|BVs|Vs>><v|V|v|<v(l|n|r)|vc>>(th)"

// Dragon Riders
#define DRAGON_RIDERS "c'<s|cvc>"

// Pokemon
#define POKEMON "<i|s>v(mon|chu|zard|rtle)"

// Fantasy (Vowels, R, etc.)
#define FANTASY_VOWELS_R "(|(<B>|s|h|ty|ph|r))(i|ae|ya|ae|eu|ia|i|eo|ai|a)(lo|la|sri|da|dai|the|sty|lae|due|li|lly|ri|na|ral|sur|rith)(|(su|nu|sti|llo|ria|))(|(n|ra|p|m|lis|cal|deu|dil|suir|phos|ru|dru|rin|raap|rgue))"

// Fantasy (S, A, etc.)
#define FANTASY_S_A "(cham|chan|jisk|lis|frich|isk|lass|mind|sond|sund|ass|chad|lirt|und|mar|lis|il|<BVC>)(jask|ast|ista|adar|irra|im|ossa|assa|osia|ilsa|<vCv>)(|(an|ya|la|sta|sda|sya|st|nya))"

// Fantasy (H, L, etc.)
#define FANTASY_H_L "(ch|ch't|sh|cal|val|ell|har|shar|shal|rel|laen|ral|jh't|alr|ch|ch't|av)(|(is|al|ow|ish|ul|el|ar|iel))(aren|aeish|aith|even|adur|ulash|alith|atar|aia|erin|aera|ael|ira|iel|ahur|ishul)"

// Fantasy (N, L, etc.)
#define FANTASY_N_L "(ethr|qil|mal|er|eal|far|fil|fir|ing|ind|il|lam|quel|quar|quan|qar|pal|mal|yar|um|ard|enn|ey)(|(<vc>|on|us|un|ar|as|en|ir|ur|at|ol|al|an))(uard|wen|arn|on|il|ie|on|iel|rion|rian|an|ista|rion|rian|cil|mol|yon)"

// Fantasy (K, N, etc.)
#define FANTASY_K_N "(taith|kach|chak|kank|kjar|rak|kan|kaj|tach|rskal|kjol|jok|jor|jad|kot|kon|knir|kror|kol|tul|rhaok|rhak|krol|jan|kag|ryr)(<vc>|in|or|an|ar|och|un|mar|yk|ja|arn|ir|ros|ror)(|(mund|ard|arn|karr|chim|kos|rir|arl|kni|var|an|in|ir|a|i|as))"

// Fantasy (J, G, Z, etc.)
#define FANTASY_J_G_Z "(aj|ch|etz|etzl|tz|kal|gahn|kab|aj|izl|ts|jaj|lan|kach|chaj|qaq|jol|ix|az|biq|nam)(|(<vc>|aw|al|yes|il|ay|en|tom||oj|im|ol|aj|an|as))(aj|am|al|aqa|ende|elja|ich|ak|ix|in|ak|al|il|ek|ij|os|al|im)"

// Fantasy (K, J, Y, etc.)
#define FANTASY_K_J_Y "(yi|shu|a|be|na|chi|cha|cho|ksa|yi|shu)(th|dd|jj|sh|rr|mk|n|rk|y|jj|th)(us|ash|eni|akra|nai|ral|ect|are|el|urru|aja|al|uz|ict|arja|ichi|ural|iru|aki|esh)"

// Fantasy (S, E, etc.)
#define FANTASY_S_E "(syth|sith|srr|sen|yth|ssen|then|fen|ssth|kel|syn|est|bess|inth|nen|tin|cor|sv|iss|ith|sen|slar|ssil|sthen|svis|s|ss|s|ss)(|(tys|eus|yn|of|es|en|ath|elth|al|ell|ka|ith|yrrl|is|isl|yr|ast|iy))(us|yn|en|ens|ra|rg|le|en|ith|ast|zon|in|yn|ys)"


	class Generator
	{
		typedef enum wrappers {
			capitalizer,
			reverser
		} wrappers_t;

		typedef enum group_types {
			symbol,
			literal
		} group_types_t;


		class Group {
			std::stack<wrappers_t> wrappers;
			std::vector<std::unique_ptr<Generator>> set;

		public:
			group_types_t type;

			Group(group_types_t type_);
			virtual ~Group() { }

			std::unique_ptr<Generator> produce();
			void split();
			void wrap(wrappers_t type);
			void add(std::unique_ptr<Generator>&& g);

			virtual void add(char c);
		};


		class GroupSymbol : public Group {
		public:
			GroupSymbol();
			void add(char c);
		};


		class GroupLiteral : public Group {
		public:
			GroupLiteral();
		};

	protected:
		std::vector<std::unique_ptr<Generator>> generators;

	public:
		static const std::unordered_map<std::string, const std::vector<std::string>>& SymbolMap();

		Generator();
		Generator(const std::string& pattern, bool collapse_triples=true);
		Generator(std::vector<std::unique_ptr<Generator>>&& generators_);

		virtual ~Generator() = default;

		virtual size_t combinations();
		virtual size_t min();
		virtual size_t max();
		virtual std::string toString();

		void add(std::unique_ptr<Generator>&& g);
	};


	class Random : public Generator
	{
	public:
		Random();
		Random(std::vector<std::unique_ptr<Generator>>&& generators_);

		size_t combinations();
		size_t min();
		size_t max();
		std::string toString();
	};


	class Sequence : public Generator
	{
	public:
		Sequence();
		Sequence(std::vector<std::unique_ptr<Generator>>&& generators_);
	};


	class Literal : public Generator
	{
		std::string value;

	public:
		Literal(const std::string& value_);

		size_t combinations();
		size_t min();
		size_t max();
		std::string toString();
	};


	class Reverser : public Generator {
	public:
		Reverser(std::unique_ptr<Generator>&& g);

		std::string toString();
	};


	class Capitalizer : public Generator
	{
	public:
		Capitalizer(std::unique_ptr<Generator>&& g);

		std::string toString();
	};


	class Collapser : public Generator
	{
	public:
		Collapser(std::unique_ptr<Generator>&& g);

		std::string toString();
	};

}

std::wstring towstring(const std::string& s);
std::string tostring(const std::wstring& s);