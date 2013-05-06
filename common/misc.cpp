#ifdef _WINDOWS
	// VS6 doesn't like the length of STL generated names: disabling
	#pragma warning(disable:4786)
#endif
#include "debug.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <iostream>
#include <zlib.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <time.h>
#include "misc.h"
#include "types.h"
#include <cstdlib>
#include <cstring>

using namespace std;

#define ENC(c) (((c) & 0x3f) + ' ')
#define DEC(c)	(((c) - ' ') & 0x3f)

map<int,string> DBFieldNames;

#ifndef WIN32
#if defined(FREEBSD) || defined(__CYGWIN__)
int print_stacktrace()
{
	printf("Insert stack trace here...\n");
	return(0);
}
#else //!WIN32 && !FREEBSD == linux
#include <execinfo.h>
int print_stacktrace()
{
  void *ba[20];
  int n = backtrace (ba, 20);
  if (n != 0)
    {
      char **names = backtrace_symbols (ba, n);
      if (names != nullptr)
        {
          int i;
          cerr <<  "called from " << (char*)names[0] << endl;
          for (i = 1; i < n; ++i)
            cerr << "            " << (char*)names[i] << endl;
          free (names);
        }
    }
	return(0);
}
#endif //!FREEBSD
#endif //!WIN32

void Unprotect(string &s, char what)
{
	if (s.length()) {
		for(string::size_type i=0;i<s.length()-1;i++) {
			if (s[i]=='\\' && s[i+1]==what)
				s.erase(i,1);
		}
	}
}

void Protect(string &s, char what)
{
	for(string::size_type i=0;i<s.length();i++) {
		if (s[i]==what)
			s.insert(i++,"\\");
	}
}

/*the map argument is:
	item id -> fields_list
		each fields_list is a map of field index -> value
*/
bool ItemParse(const char *data, int length, map<int,map<int,string> > &items, int id_pos, int name_pos, int max_field, int level)
{
int i;
char *end,*ptr;
map<int,string> field;
static char *buffer=nullptr;
static int buffsize=0;
static char *temp=nullptr;
	if (!buffsize || buffsize<(length+1)) {
		buffer=(char *)realloc(buffer,length+1);
		temp=(char *)realloc(temp,length+1);
		buffsize=length+1;
	}
	memcpy(buffer,data,length);
	buffer[length]=0;

	ptr=buffer;

	for(i=0;i<name_pos-1;i++) {
		end=ptr-1;
		while((end=strchr(end+1,'|'))!=nullptr) {
			if (*(end-1)!='\\')
				break;
		}
		if (!end) {
			cerr << "ItemParse: Level " << level << ": (1) Expected '|' not found near '" << ptr << "'" << endl;
			return false;
		} else {
			*end=0;
			field[i]=ptr;
			Unprotect(field[i],'|');
			length-=(end-ptr)+1;
			ptr=end+1;
		}
	}

	if (*ptr!='"') {
		cerr << "ItemParse: Level " << level << ": (2) Expected '\"' not found near '" << ptr << "'" << endl;
		return false;
	}
	ptr++;

	for(i=(name_pos-1);i<(max_field-1);i++) {
		end=ptr-1;
		while((end=strchr(end+1,'|'))!=nullptr) {
			if (*(end-1)!='\\')
				break;
		}
		if (!end) {
			cerr << "ItemParse: Level " << level << ": (1) Expected '|' not found near '" << ptr << "'" << endl;
			return false;
		} else {
			*end=0;
			field[i]=ptr;
			Unprotect(field[i],'|');
			length-=(end-ptr)+1;
			ptr=end+1;
		}
	}

	if (*ptr=='|') {
		field[i]="";
		ptr++;
	} else if (sscanf(ptr,"%[^\"]\"%n",temp,&length)==1) {
		field[i]=temp;
		Unprotect(field[i],'|');
		ptr+=length;
	} else {
		cerr << "ItemParse: Level " << level << ": (4) Expected '\"' not found near '" << ptr << "'" << endl;
		return false;
	}

	if (*ptr!='|') {
		cerr << "ItemParse: Level " << level << ": (5) Expected '|' not found near '" << ptr << "'" << endl;
		return false;
	}
	ptr++;
	uint32 id = atoi(field[id_pos].c_str());
	items[id]=field;

	for(i=0;i<10;i++) {
		if (*ptr=='"') {
			end=ptr;
			while((end=strchr(end+1,'"'))!=nullptr && *(end-1)=='\\');
			if (end) {
				string sub;
				sub.assign(ptr+1,end-ptr-1);
				Unprotect(sub,'"');
				if (!ItemParse(sub.c_str(),sub.length(),items,id_pos,name_pos,max_field,level+1)) {
					return false;
				}
				ptr=end+1;
			} else {
				cerr << "ItemParse: Level " << level << ": (6) Expected closing '\"' not found near '" << (ptr+1) << "'" << endl;
				return false;
			}
		}
		if (*ptr=='|') {
			ptr++;
		} else if (i<9) {
			cerr << "ItemParse: Level " << level << ": (7) Expected '|' (#" << i << ") not found near '" << ptr << "'" << endl;
			return false;
		}

	}
	return true;
}

int Tokenize(string s,map<int,string> & tokens, char delim)
{
int i,len;
string::size_type end;
//char temp[1024];
string x;
	tokens.clear();
	i=0;
	while(s.length()) {
		if (s[0]==delim) {
			s.erase(0,1);
			tokens[i++]="";
		} else {
			end=0;
			while((end=s.find(delim,end+1))!=string::npos && s[end-1]=='\\');
			if (end!=string::npos) {
				x=s;
				x.erase(end,string::npos);
				s.erase(0,end+1);
				Unprotect(x,'|');
				tokens[i++]=x;
			} else {
				Unprotect(s,'|');
				tokens[i++]=s;
			break;
			}
		}
		len=0;
	}
	return i;
}

void LoadItemDBFieldNames() {
	DBFieldNames[0]="N/A";			// Charges
	DBFieldNames[1]="unknown002";		// ?
	DBFieldNames[2]="N/A";			// Current Equip Slot
	DBFieldNames[3]="unknown004";
	DBFieldNames[4]="unknown005";		// ?
	DBFieldNames[5]="itemclass";		// "Item Type (0=common, 1=container, 2=book)"
	DBFieldNames[6]="name";			// Name
	DBFieldNames[7]="lore";			// "Lore Name (*=lore, &=summoned, #=artifact)"
	DBFieldNames[8]="idfile";		// IDFile
	DBFieldNames[9]="id";			// ItemNumber
	DBFieldNames[10]="weight";		// Weight
	DBFieldNames[11]="norent";		// "NoRent (0=norent, 255=not norent)"
	DBFieldNames[12]="nodrop";		// "NoDrop (0=nodrop, 255=not nodrop)"
	DBFieldNames[13]="size";		// "Size (0=tiny, 1=small, 2=medium, 3=large, 4=giant)"
	DBFieldNames[14]="slots";		// EquipSlots
	DBFieldNames[15]="cost";		// Cost
	DBFieldNames[16]="icon";		// IconNumber
	DBFieldNames[17]="unknown018";
	DBFieldNames[18]="unknown019";
	DBFieldNames[19]="unknown020";		// ?
	DBFieldNames[20]="tradeskills";		// "Tradeskill Item (1=is a tradeskill item, 0=not)"
	DBFieldNames[21]="cr";			// SvCold
	DBFieldNames[22]="dr";			// SvDisease
	DBFieldNames[23]="pr";			// SvPoison
	DBFieldNames[24]="mr";			// SvMagic
	DBFieldNames[25]="fr";			// SvFire
	DBFieldNames[26]="astr";		// STR
	DBFieldNames[27]="asta";		// STA
	DBFieldNames[28]="aagi";		// AGI
	DBFieldNames[29]="adex";		// DEX
	DBFieldNames[30]="acha";		// CHA
	DBFieldNames[31]="aint";		// INT
	DBFieldNames[32]="awis";		// WIS
	DBFieldNames[33]="hp";			// HP
	DBFieldNames[34]="mana";		// Mana
	DBFieldNames[35]="ac";			// AC
	DBFieldNames[36]="deity";		// Deity
	DBFieldNames[37]="skillmodvalue";	// Skill Mod Value
	DBFieldNames[38]="skillmodtype";	// Skill Mod Type
	DBFieldNames[39]="banedmgrace";		// Bane Dmg Race
	DBFieldNames[40]="banedmgamt";		// Band Dmg
	DBFieldNames[41]="banedmgbody";		// Band Dmg Body
	DBFieldNames[42]="magic";		// "Magic (0=not magic, 1=magic)"
	DBFieldNames[43]="casttime2";		// Casttime appears twice
	DBFieldNames[44]="hasteproclvl";	// "Level (Haste value, rather)"
	DBFieldNames[45]="reqlevel";		// Required Level
	DBFieldNames[46]="bardtype";		// Bard Type
	DBFieldNames[47]="bardvalue";		// Bard Type Amount
	DBFieldNames[48]="light";		// Light
	DBFieldNames[49]="delay";		// Attack Delay
	DBFieldNames[50]="reclevel";		// Recommended Level
	DBFieldNames[51]="recskill";		// Recommended Skill
	DBFieldNames[52]="elemdmgamt";		// "Elemental Dmg Type (1=magic, 2=fire, 3=cold, 4=poison, 5=disease)"
	DBFieldNames[53]="elemdmgtype";		// Elemental Dmg
	DBFieldNames[54]="effecttype";		// "Effect Type (0=combat, 1=clicky, 2=Worn, 3=Expendable charges, 4=Must Equip Clicky, 5=clicky)"
	DBFieldNames[55]="range";		// Range
	DBFieldNames[56]="damage";		// Damage
	DBFieldNames[57]="color";		// Color
	DBFieldNames[58]="classes";		// Classes
	DBFieldNames[59]="races";		// Races
	DBFieldNames[60]="unknown061";
	DBFieldNames[61]="spellid";		// SpellId
	DBFieldNames[62]="maxcharges";		// MaxCharges
	DBFieldNames[63]="itemtype";		// "Skill (ItemType: 1hs, etc)"
	DBFieldNames[64]="material";		// Material
	DBFieldNames[65]="sellrate";		// ** Sell Rate
	DBFieldNames[66]="unknown067";
	DBFieldNames[67]="casttime";		// CastTime (milliseconds)
	DBFieldNames[68]="unknown069";
	DBFieldNames[69]="unknown070";		// ?
	DBFieldNames[70]="focusid";		// Focus Effect Spell Id
	DBFieldNames[71]="combateffects";	// CombatEffects
	DBFieldNames[72]="shielding";		// Shielding
	DBFieldNames[73]="stunresist";		// StunResist
	DBFieldNames[74]="strikethrough";	// StrikeThrough
	DBFieldNames[75]="unknown076";
	DBFieldNames[76]="unknown077";		// ?
	DBFieldNames[77]="spellshield";		// Spell Shield
	DBFieldNames[78]="avoidance";		// Avoidance
	DBFieldNames[79]="accuracy";		// Accuracy
	DBFieldNames[80]="factionmod1";		// Faction Mod Index 1
	DBFieldNames[81]="factionmod2";		// Faction Mod Index 2
	DBFieldNames[82]="factionmod3";		// Faction Mod Index 3
	DBFieldNames[83]="factionmod4";		// Faction Mod Index 4
	DBFieldNames[84]="factionamt1";		// Faction Mod Value 1
	DBFieldNames[85]="factionamt2";		// Faction Mod Value 2
	DBFieldNames[86]="factionamt3";		// Faction Mod Value 3
	DBFieldNames[87]="factionamt4";		// Faction Mod Value 4
	DBFieldNames[88]="unknown089";
	DBFieldNames[89]="charmfile";		// ** Charm File
	DBFieldNames[90]="unknown091";
	DBFieldNames[91]="augslot1type";	// Slot1Type
	DBFieldNames[92]="augslot2type";	// Slot2Type
	DBFieldNames[93]="augslot3type";	// Slot3Type
	DBFieldNames[94]="augslot4type";	// Slot4Type
	DBFieldNames[95]="augslot5type";	// Slot5Type
	DBFieldNames[96]="ldonpointtheme";
	DBFieldNames[97]="ldonpointcost";		// ?
	DBFieldNames[98]="unknown099";
	DBFieldNames[99]="bagtype";		// bag type
	DBFieldNames[100]="bagslots";		// bag slots
	DBFieldNames[101]="bagsize";		// bag size capacity
	DBFieldNames[102]="bagwr";		// bag weight reduction
	DBFieldNames[103]="booktype";		// "book type (0=rolled up note, 1=book)"
	DBFieldNames[104]="unknown105";
	DBFieldNames[105]="filename";		// Book Filename
	DBFieldNames[106]="unknown107";
	DBFieldNames[107]="unknown108";
	DBFieldNames[108]="loreflag";
	DBFieldNames[109]="unknown111";
	DBFieldNames[110]="unknown112";
	DBFieldNames[111]="unknown113";
	DBFieldNames[112]="unknown114";
	DBFieldNames[113]="unknown115";		// ? (end quote)
}

void encode_length(unsigned long length, char *out)
{
char buf[4];
	memcpy(buf,&length,sizeof(unsigned long));
	encode_chunk(buf,3,out);
}

unsigned long encode(char *in, unsigned long length, char *out)
{
unsigned long used=0,len=0;
	while(used<length) {
		encode_chunk(in+used,length-used,out+len);
		used+=3;
		len+=4;
	}
	*(out+len)=0;

	return len;
}

unsigned long decode_length(char *in)
{
int length;
char buf[4];
	decode_chunk(in,&buf[0]);
	buf[3]=0;
	memcpy(&length,buf,sizeof(unsigned long));

	return length;
}

void decode(char *in, char *out)
{
char *ptr=in;
char *outptr=out;
	while(*ptr) {
		decode_chunk(ptr,outptr);
		ptr+=4;
		outptr+=3;
	}
	*outptr=0;
}

void encode_chunk(char *in, int len, char *out)
{
	*out=ENC(in[0] >> 2);
	*(out+1)=ENC((in[0] << 4)|(((len<2 ? 0 : in[1]) >> 4) & 0xF));
	*(out+2)=ENC(((len<2 ? 0 : in[1]) << 2)|(((len<3 ? 0 : in[2]) >> 6) & 0x3));
	*(out+3)=ENC((len<3 ? 0 : in[2]));
}

void decode_chunk(char *in, char *out)
{
        *out = DEC(*in) << 2 | DEC(in[1]) >> 4;
        *(out+1) = DEC(in[1]) << 4 | DEC(in[2]) >> 2;
        *(out+2) = DEC(in[2]) << 6 | DEC(in[3]);
}

void dump_message_column(unsigned char *buffer, unsigned long length, string leader, FILE *to)
{
unsigned long i,j;
unsigned long rows,offset=0;
	rows=(length/16)+1;
	for(i=0;i<rows;i++) {
		fprintf(to, "%s%05ld: ",leader.c_str(),i*16);
		for(j=0;j<16;j++) {
			if(j == 8)
				fprintf(to, "- ");
			if (offset+j<length)
				fprintf(to, "%02x ",*(buffer+offset+j));
			else
				fprintf(to, "   ");
		}
		fprintf(to, "| ");
		for(j=0;j<16;j++,offset++) {
			if (offset<length) {
				char c=*(buffer+offset);
				fprintf(to, "%c",isprint(c) ? c : '.');
			}
		}
		fprintf(to, "\n");
	}
}

string long2ip(unsigned long ip)
{
char temp[16];
union { unsigned long ip; struct { unsigned char a,b,c,d; } octet;} ipoctet;

	ipoctet.ip=ip;
	sprintf(temp,"%d.%d.%d.%d",ipoctet.octet.a,ipoctet.octet.b,ipoctet.octet.c,ipoctet.octet.d);

	return string(temp);
}

string string_from_time(string pattern, time_t now)
{
struct tm *now_tm;
char time_string[51];

	if (!now)
		time(&now);
	now_tm=localtime(&now);

	strftime(time_string,51,pattern.c_str(),now_tm);

	return string(time_string);
}

string timestamp(time_t now)
{
	return string_from_time("[%Y%m%d.%H%M%S] ",now);
}


string pop_arg(string &s, string seps, bool obey_quotes)
{
string ret;
unsigned long i;
bool in_quote=false;

	unsigned long length=s.length();
	for(i=0;i<length;i++) {
		char c=s[i];
		if (c=='"' && obey_quotes) {
			in_quote=!in_quote;
		}
		if (in_quote)
			continue;
		if (seps.find(c)!=string::npos) {
			break;
		}
	}

	if (i==length) {
		ret=s;
		s="";
	} else {
		ret=s.substr(0,i);
		s.erase(0,i+1);
	}


	return ret;
}

int EQsprintf(char *buffer, const char *pattern, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const char *arg6, const char *arg7, const char *arg8, const char *arg9)
{
const char *args[9],*ptr;
char *bptr;
	args[0]=arg1;
	args[1]=arg2;
	args[2]=arg3;
	args[3]=arg4;
	args[4]=arg5;
	args[5]=arg6;
	args[6]=arg7;
	args[7]=arg8;
	args[8]=arg9;
	for(ptr=pattern,bptr=buffer;*ptr;) {
		switch (*ptr) {
			case '%':
				ptr++;
				switch (*ptr) {
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						strcpy(bptr,args[*ptr-'0'-1]);
						bptr+=strlen(args[*ptr-'0'-1]);
						break;
				}
				break;
			default:
				*bptr=*ptr;
				bptr++;
		}
		ptr++;
	}

	*bptr=0;
	return (bptr-buffer);
}

string generate_key(int length)
{
string key;
//TODO: write this for win32...
#ifndef WIN32
int i;
timeval now;
	static const char *chars="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	for(i=0;i<length;i++) {
		gettimeofday(&now,nullptr);
		srand(now.tv_sec^now.tv_usec);
		key+=(char)chars[(int) (36.0*rand()/(RAND_MAX+1.0))];
	}
#endif
	return key;
}

void print_hex(const char *data, unsigned long length) {
	char buffer[80];
	uint32 offset;
	for(offset=0;offset<length;offset+=16) {
		build_hex_line((const char *)data,length,offset,buffer,5);
		printf("%s\n", buffer);	//%s is to prevent % escapes in the ascii
	}
}

void build_hex_line(const char *buffer, unsigned long length, unsigned long offset, char *out_buffer, unsigned char padding)
{
char *ptr=out_buffer;
int i;
char printable[17];
	ptr+=sprintf(ptr,"%0*lu:",padding,offset);
	for(i=0;i<16; i++) {
		if (i==8) {
			strcpy(ptr," -");
			ptr+=2;
		}
		if (i+offset < length) {
			unsigned char c=*(const unsigned char *)(buffer+offset+i);
			ptr+=sprintf(ptr," %02x",c);
			printable[i]=isprint(c) ? c : '.';
		} else {
			ptr+=sprintf(ptr,"   ");
			printable[i]=0;
		}
	}
	sprintf(ptr,"  | %.16s",printable);
}

