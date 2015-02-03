/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2006 EQEMu Development Team (http://eqemulator.net)

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
#include "global_define.h"
#include "xml_parser.h"

XMLParser::XMLParser() {
	ParseOkay = false;
}

bool XMLParser::ParseFile(const char *file, const char *root_ele) {
	std::map<std::string,ElementHandler>::iterator handler;
	TiXmlDocument doc( file );
	if(!doc.LoadFile()) {
		printf("Unable to load '%s': %s\n", file, doc.ErrorDesc());
		return(false);
	}

	TiXmlElement *root = doc.FirstChildElement( root_ele );
	if(root == nullptr) {
		printf("Unable to find root '%s' in %s\n",root_ele, file);
		return(false);
	}

	ParseOkay=true;

	TiXmlNode *main_element = nullptr;
	while( (main_element = root->IterateChildren( main_element )) ) {
		if(main_element->Type() != TiXmlNode::ELEMENT)
			continue;	//skip crap we dont care about
		TiXmlElement *ele = (TiXmlElement *) main_element;

		handler=Handlers.find(ele->Value());
		if (handler!=Handlers.end() && handler->second) {
			ElementHandler h=handler->second;

			/*
			*
			* This is kinda a sketchy operation here, since all of these
			* element handler methods will be functions in child classes.
			* This essentially causes us to do an un-checkable (and hence
			* un-handle-properly-able) cast down to the child class. This
			* WILL BREAK if any children classes do multiple inheritance.
			*
			*
			*/

			(this->*h)(ele);
		} else {
			//unhandled element.... do nothing for now
		}

	}

	return(ParseOkay);
}

const char *XMLParser::ParseTextBlock(TiXmlNode *within, const char *name, bool optional) {
	TiXmlElement * txt = within->FirstChildElement(name);
	if(txt == nullptr) {
		if(!optional) {
			printf("Unable to find a '%s' element on %s element at line %d\n", name, within->Value(), within->Row());
			ParseOkay=false;
		}
		return(nullptr);
	}
	TiXmlNode *contents = txt->FirstChild();
	if(contents == nullptr || contents->Type() != TiXmlNode::TEXT) {
		if(!optional)
			printf("Node '%s' was expected to be a text element in %s element at line %d\n", name, txt->Value(), txt->Row());
		return(nullptr);
	}
	return(contents->Value());
}

const char *XMLParser::GetText(TiXmlNode *within, bool optional) {
	TiXmlNode *contents = within->FirstChild();
	if(contents == nullptr || contents->Type() != TiXmlNode::TEXT) {
		if(!optional) {
			printf("Node was expected to be a text element in %s element at line %d\n", within->Value(), within->Row());
			ParseOkay=false;
		}
		return(nullptr);
	}
	return(contents->Value());
}

