/** \file HttpdForm.cpp - read stdin, parse cgi input
 **
 **	Written: 1999-Feb-10 grymse@alhem.net
 **/

/*
Copyright (C) 1999-2005  Anders Hedstrom

This library is made available under the terms of the GNU GPL.

If you would like to use this library in a closed-source application,
a separate license agreement is available. For information about 
the closed-source license agreement for the C++ sockets library,
please visit http://www.alhem.net/Sockets/license.html and/or
email license@alhem.net.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <stdio.h>
#ifdef _WIN32
#pragma warning(disable:4786)
#include <windows.h>
#endif
#include "socket_include.h"
#include "Parse.h"
#include "IFile.h"
#include "HttpdForm.h"
#include <cstdlib>
#include <cstring>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


HttpdForm::HttpdForm(IFile *infil) : raw(false)
{
	CGI *cgi = nullptr;
	char *c_t = getenv("CONTENT_TYPE");
	char *c_l = getenv("CONTENT_LENGTH");
	size_t extra = 2;
	char name[200];

	m_current = m_cgi.end();
	*name = 0;

	if (c_t && !strncmp(c_t, "multipart/form-data",19))
	{
		Parse pa(c_t,";=");
		char *tempcmp = nullptr;
		size_t tc = 0;
		size_t l = 0;
		std::string str = pa.getword();
		m_strBoundary = "";
		while (str.size())
		{
			if (!strcmp(str.c_str(),"boundary"))
			{
				m_strBoundary = pa.getword();
				l = m_strBoundary.size();
				tempcmp = new char[l + extra];
			}
			//
			str = pa.getword();
		}
		if (m_strBoundary.size())
		{
			std::string content_type;
			std::string current_name;
			std::string current_filename;
			char slask[2000];
			infil -> fgets(slask, 200);
			while (!infil -> eof())
			{
				while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
				{
					slask[strlen(slask) - 1] = 0;
				}
				content_type = "";
				current_name = "";
				current_filename = "";
				if ((strstr(slask,m_strBoundary.c_str()) || strstr(m_strBoundary.c_str(),slask)) && strcmp(slask, m_strBoundary.c_str()))
				{
					m_strBoundary = slask;
					l = m_strBoundary.size();
					delete[] tempcmp;
					tempcmp = new char[l + extra];
				}
				if (!strcmp(slask, m_strBoundary.c_str()))
				{
					// Get headers until empty line
					infil -> fgets(slask, 200);
					while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
					{
						slask[strlen(slask) - 1] = 0;
					}
					while (!infil -> eof() && *slask)
					{
						Parse pa(slask,";");
						std::string h = pa.getword();
						if (!strcasecmp(h.c_str(),"Content-type:"))
						{
							content_type = pa.getword();
						}
						else
						if (!strcasecmp(h.c_str(),"Content-Disposition:"))
						{
							h = pa.getword();
							if (!strcmp(h.c_str(),"form-data"))
							{
								pa.EnableQuote(true);
								h = pa.getword();
								while (h.size())
								{
									Parse pa2(slask,"=");
									std::string name = pa2.getword();
									std::string h = pa2.getrest();
									if (!strcmp(name.c_str(),"name"))
									{
										if (h.size() && h[0] == '"')
										{
											current_name = h.substr(1, h.size() - 2);
										}
										else
										{
											current_name = h;
										}
									}
									else
									if (!strcmp(name.c_str(),"filename"))
									{
										if (h.size() && h[0] == '"')
										{
											current_filename = h.substr(1, h.size() - 2);
										}
										else
										{
											current_filename = h;
										}
										size_t x = 0;
										for (size_t i = 0; i < current_filename.size(); i++)
										{
											if (current_filename[i] == '/' || current_filename[i] == '\\')
												x = i + 1;
										}
										if (x)
										{
											current_filename = current_filename.substr(x);
										}
									}
									h = pa.getword();
								}
							}
						}
						// get next header value
						infil -> fgets(slask, 200);
						while (strlen(slask) && (slask[strlen(slask) - 1] == 13 || slask[strlen(slask) - 1] == 10))
						{
							slask[strlen(slask) - 1] = 0;
						}
					}
					// Read content, save...?
					if (!current_filename.size()) // not a file
					{
						std::string val;
						infil -> fgets(slask,1000);
						while (!infil -> eof() && strncmp(slask,m_strBoundary.c_str(),m_strBoundary.size() ))
						{
							val += slask;
							infil -> fgets(slask,1000);
						}
						// remove trailing cr/linefeed
						while (val.size() && (val[val.size() - 1] == 13 || val[val.size() - 1] == 10))
						{
							val = val.substr(0,val.size() - 1);
						}
						cgi = new CGI(current_name, val);
						m_cgi.push_back(cgi);
					}
					else // current_filename.size() > 0
					{
						// read until m_strBoundary...
						FILE *fil;
						int out = 0;
						char c;
						char fn[1000]; // where post'd file will be saved
#ifdef _WIN32
						{
							char tmp_path[1000];
							::GetTempPath(1000, tmp_path);
							if (tmp_path[strlen(tmp_path) - 1] != '\\')
							{
								strcat(tmp_path, "\\");
							}
							sprintf(fn,"%s%s",tmp_path,current_filename.c_str());
						}
#else
						sprintf(fn,"/tmp/%s",current_filename.c_str());
#endif
						if ((fil = fopen(fn, "wb")) != nullptr)
						{
							infil -> fread(&c,1,1);
							while (!infil -> eof())
							{
								if (out)
								{
									fwrite(&tempcmp[tc],1,1,fil);
								}
								tempcmp[tc] = c;
								tc++;
								if (tc >= l + extra)
								{
									tc = 0;
									out = 1;
								}
								if (tc)
								{
									if (!strncmp(tempcmp + tc + extra, m_strBoundary.c_str(), l - tc) &&
											!strncmp(tempcmp, m_strBoundary.c_str() + l - tc, tc))
									{
										break;
									}
								}
								else
								{
									if (!strncmp(tempcmp + extra, m_strBoundary.c_str(), l))
									{
										break;
									}
								}
								infil -> fread(&c,1,1);
							}
							fclose(fil);

							cgi = new CGI(current_name,fn,fn);
							m_cgi.push_back(cgi);
						
							strcpy(slask, m_strBoundary.c_str());
							infil -> fgets(slask + strlen(slask), 200); // next line
						}
						else
						{
							// couldn't open file
							break;
						}
					}
				}
				else
				{
					// Probably '<m_strBoundary>--'
					break;
				}
			} // while (!infil -> eof())
		} // if (m_strBoundary)
		if (tempcmp)
		{
			delete[] tempcmp;
		}
	}
	else
	{
		int i = 0;
		int cl = c_l ? atoi(c_l) : -1;
		char c,chigh,clow;
		char *slask = new char[8888];
		bool got_name = false;
		m_current = m_cgi.end();

		*name = 0;

		infil -> fread(&c,1,1);
		cl--;
		while (cl >= 0)
		{
			switch (c)
			{
				case '=': /* end of name */
					slask[i] = 0;
					i = 0;
					strcpy(name,slask);
					got_name = true;
					break;
				case '&': /* end of value */
					slask[i] = 0;
					i = 0;
					if(got_name) {
						got_name = false;
						cgi = new CGI(name,slask);
						m_cgi.push_back(cgi);
					} else {
						cgi = new CGI(slask,"");
						m_cgi.push_back(cgi);
					}
					break;
				case '+': /* space */
					slask[i++] = ' ';
					break;
				case '%': /* hex value */
					infil -> fread(&chigh,1,1);
					cl--;
					chigh -= 48;
					chigh &= 0xff - 32;
					if (chigh > 9)
						chigh -= 7;
					infil -> fread(&clow,1,1);
					cl--;
					clow -= 48;
					clow &= 0xff - 32;
					if (clow > 9)
						clow -= 7;
					slask[i++] = (char)(chigh * 16 + clow);
					break;
				default: /* just another char */
					slask[i++] = c;
					break;
			}
			if(infil -> eof())
				break;
			//
			if (cl > 0)
			{
				infil -> fread(&c,1,1);
			}
			cl--;
		}
		slask[i] = 0;
		i = 0;
		if(got_name) {
			cgi = new CGI(name,slask);
			m_cgi.push_back(cgi);
		} else {
			cgi = new CGI(slask,"");
			m_cgi.push_back(cgi);
		}
		delete[] slask;
	}
}


// HttpdForm(buffer,l) -- request_method GET

HttpdForm::HttpdForm(const std::string& buffer,size_t l) : raw(false)
{
	CGI *cgi = nullptr;
	char slask[8888];
	char name[200];
	int i = 0;
	char c,chigh,clow;
	bool got_name = false;
	size_t ptr = 0;

	m_current = m_cgi.end();

	*name = 0;
	
	ptr = 0;
	while (ptr < l)
	{
		c = buffer[ptr++];
		switch (c)
		{
			case '=': /* end of name */
				slask[i] = 0;
				i = 0;
				got_name = true;
				strcpy(name,slask);
				break;
			case '&': /* end of value */
				slask[i] = 0;
				i = 0;
				if(got_name) {
					got_name = false;
					cgi = new CGI(name,slask);
					m_cgi.push_back(cgi);
				} else {
					cgi = new CGI(slask, "");
					m_cgi.push_back(cgi);
				}
				break;
			case '+': /* space */
				slask[i++] = ' ';
				break;
			case '%': /* hex value */
				chigh = buffer[ptr++];
				chigh -= 48;
				chigh &= 0xff - 32;
				if (chigh > 9)
					chigh -= 7;
				clow = buffer[ptr++];
				clow -= 48;
				clow &= 0xff - 32;
				if (clow > 9)
					clow -= 7;
				slask[i++] = (char)(chigh * 16 + clow);
				break;
			default: /* just another char */
				slask[i++] = c;
				break;
		}
	}
	slask[i] = 0;
	i = 0;
	if(got_name) {
		cgi = new CGI(name,slask);
		m_cgi.push_back(cgi);
	} else {
		cgi = new CGI(slask, "");
		m_cgi.push_back(cgi);
	}
}


HttpdForm::~HttpdForm()
{
	CGI *cgi = nullptr; //,*tmp;

	for (cgi_v::iterator it = m_cgi.begin(); it != m_cgi.end(); it++)
	{
		cgi = *it;
		delete cgi;
	}
}


void HttpdForm::EnableRaw(bool b)
{
	raw = b;
}


void HttpdForm::strcpyval(std::string& v,const char *value) //,size_t len)
{
	v = "";
	for (size_t i = 0; i < strlen(value); i++)
	{
		if (value[i] == '<')
		{
			v += "&lt;";
		}
		else
		if (value[i] == '>')
		{
			v += "&gt;";
		}
		else
		if (value[i] == '&')
		{
			v += "&amp;";
		}
		else
		{
			v += value[i];
		}
	}
}


bool HttpdForm::getfirst(std::string& n) //char *n,size_t len)
{
	m_current = m_cgi.begin();
	return getnext(n);
}


bool HttpdForm::getnext(std::string& n) //char *n,size_t len)
{
	if (m_current != m_cgi.end() )
	{
		CGI *current = *m_current;
		n = current -> name;
		m_current++;
		return true;
	}
	else
	{
		n = "";
	}
	return false;
}


bool HttpdForm::getfirst(std::string& n,std::string& v) //char *n,size_t len,char *v,size_t vlen)
{
	m_current = m_cgi.begin();
	return getnext(n,v);
}


bool HttpdForm::getnext(std::string& n,std::string& v) //char *n,size_t len,char *v,size_t vlen)
{
	if (m_current != m_cgi.end() )
	{
		CGI *current = *m_current;
		n = current -> name;
		if (raw)
		{
			v = current -> value;
		}
		else
		{
			strcpyval(v,current -> value.c_str());
		}
		m_current++;
		return true;
	}
	else
	{
		n = "";
	}
	return false;
}


int HttpdForm::getvalue(const std::string& n,std::string& v) //char *v,size_t len)
{
	CGI *cgi = nullptr;
	int r = 0;

	for (cgi_v::iterator it = m_cgi.begin(); it != m_cgi.end(); it++)
	{
		cgi = *it;
		if (cgi -> name == n)
			break;
		cgi = nullptr;
	}
	if (cgi)
	{
		if (raw)
		{
			v = cgi -> value;
		}
		else
		{
			strcpyval(v,cgi -> value.c_str());
		}
		r++;
	}
	else
	{
		v = "";
	}

	return r;
}


std::string HttpdForm::getvalue(const std::string& n)
{
	for (cgi_v::iterator it = m_cgi.begin(); it != m_cgi.end(); it++)
	{
		CGI *cgi = *it;
		if (cgi -> name == n)
		{
			return cgi -> value;
		}
	}
	return "";
}


size_t HttpdForm::getlength(const std::string& n)
{
	CGI *cgi = nullptr;
	size_t l;

	for (cgi_v::iterator it = m_cgi.begin(); it != m_cgi.end(); it++)
	{
		cgi = *it;
		if (cgi -> name == n)
			break;
		cgi = nullptr;
	}
	l = cgi ? cgi -> value.size() : 0;
	if (cgi && !raw)
	{
		for (size_t i = 0; i < cgi -> value.size(); i++)
		{
			switch (cgi -> value[i])
			{
			case '<': // &lt;
			case '>': // &gt;
				l += 4;
				break;
			case '&': // &amp;
				l += 5;
				break;
			}
		}
	}
	return l;
}


HttpdForm::cgi_v& HttpdForm::getbase()
{
	return m_cgi;
}


const std::string& HttpdForm::GetBoundary()
{
	return m_strBoundary;
}


#ifdef SOCKETS_NAMESPACE
}
#endif

