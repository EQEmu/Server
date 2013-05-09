const char *notin = "~.? ,)=!&|<>\"";

const char * charIn = "`~1234567890-=!@#$%^&*()_+qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP|ASDFGHJKL:ZXCVBNM<>?\"{}";
const char * charIn2 = "`~1234567890-=!@#$%^&*()_+qwertyuiop[]asdfghjkl;'zxcvbnm,./QWERTYUIOP|ASDFGHJKL:ZXCVBNM<>?\" ";

char com_list[512];

/*char * gettok(const char * string, int chara, int pos) {
	static char * pch;
	static char temp[100];
	static char ty[100];
	memset(temp, 0, sizeof(temp));
	memset(ty, 0, sizeof(ty));
	ty[0] = chara;
	strn0cpy(temp, string, sizeof(temp));
	pch = strtok(temp,ty);
	for (int i=0; i < pos; i++) {
		if (pos == i) break;
		pch = strtok(nullptr,ty);
	}
	return pch;
}*/


int calc(char * stuff) {
	int result = 0;
	int i = 0;
	int a = 0;
	static char temp[100];
	int op = 0;
	memset(temp,0x0,100);
	//char heh[100];
	while(*stuff)
	{
		a++;

		if (*stuff >= '0' && *stuff <= '9') {
			if (op) {
				switch ( op ) {
				case 0:
					break;
				case '+':
					result += atoi(stuff);
					memset(temp,0x0,100);
					i = 0;
					break;
				case '-':
					result -= atoi(stuff);
					memset(temp,0x0,100);
					i = 0;
					break;
				case '*':
					result *= atoi(stuff);
					memset(temp,0x0,100);
					i = 0;
					break;
				case '/':
					result /= atoi(stuff);
					memset(temp,0x0,100);
					i = 0;
					break;
				}
			}
			temp[i] = *stuff;
		}
		else if (*stuff == '+' || *stuff == '-' || *stuff == '/' || *stuff == '*') {
			op = *stuff;
			if (!result) result = atoi(temp);
				memset(temp,0x0,100);
			i=0;
		}
		i++;
		stuff++;
	}
	return result;
}

char * postring (char * string, int pos) {
	static char temp[1024];
	memset(temp, 0, sizeof(temp));
	int p=0;
	int tmpStringLen = strlen(string);
	for (int i=pos; i < tmpStringLen; i++) {
		temp[p] = string[i];
		p++;
	}
	return temp;
}

int g_id ( char * string )
{
	char temp[100];
	int i=0;
	while (*string)
	{
		if (*string >= '0' && *string <= '9') temp[i++] = *string++;
		else string++;
	}
	return atoi(temp);
}

int GetArgs(char * string)
{
#if 1
	char temp[255];
	char c_name[255];
	int params = 0;
	//char *buffer = new char[512]; // this is never deleted, causes zone to crash
	char *buffer = com_list;
	memset(temp,0x0,255);
	//memset(buffer,0x0,512);
	//#ifdef WIN32
	//strcpy(buffer,com_list);
	//#else
	//strncpy(buffer,com_list,sizeof(buffer)-1);
	//#endif
	int i=0;
	while (*buffer)
	{
		if (*buffer != '|' && *buffer != ' ') temp[i++] = *buffer++;
		else buffer++;
		if (*buffer == '|') {
			params = atoi(temp);
			if (!strcmp(c_name,string)) {
				return params;
			}
			memset(temp,0x0,255);
			i=0;
		}
		if (*buffer == ' ') {
			memset(c_name,0x0,255);
			strcpy(c_name,temp);
			memset(temp,0x0,255);
			i=0;
		}
	}
	return 0;
#else
	if (strstr(string,"if"))			{ return 0; }
	if (strstr(string,"sfollow"))		{ return 0; }
	if (strstr(string,"save"))			{ return 0; }
	if (strstr(string,"while"))			{ return 0; }
	if (strstr(string,"break"))			{ return 1; }
	if (strstr(string,"follow"))		{ return 1; }
	if (strstr(string,"setallskill"))	{ return 1; }
	if (strstr(string,"me"))			{ return 1; }
	if (strstr(string,"flagcheck"))		{ return 1; }
	if (strstr(string,"echo"))			{ return 1; }
	if (strstr(string,"summonitem"))	{ return 1; }
	if (strstr(string,"say"))			{ return 1; }
	if (strstr(string,"emote"))			{ return 1; }
	if (strstr(string,"shout"))			{ return 1; }
	if (strstr(string,"depop"))			{ return 1; }
	if (strstr(string,"cumflag"))		{ return 1; }
	if (strstr(string,"exp"))			{ return 1; }
	if (strstr(string,"level"))			{ return 1; }
	if (strstr(string,"safemove"))		{ return 1; }
	if (strstr(string,"rain"))			{ return 1; }
	if (strstr(string,"snow"))			{ return 1; }
	if (strstr(string,"pvp"))			{ return 1; }
	if (strstr(string,"doanim"))		{ return 1; }
	if (strstr(string,"dbspawnadd"))	{ return 2; }
	if (strstr(string,"castspell"))		{ return 2; }
	if (strstr(string,"flagcheck"))		{ return 2; }
	if (strstr(string,"addskill"))		{ return 2; }
	if (strstr(string,"write"))			{ return 2; }
	if (strstr(string,"settarget"))		{ return 2; }
	if (strstr(string,"givecash"))		{ return 4; }
	if (strstr(string,"movepc"))		{ return 4; }
	if (strstr(string,"spawn"))			{ return 6; }
	return 0;
#endif // 0
}

Client* gClient = 0;

uint32 Line_Number = 0;

