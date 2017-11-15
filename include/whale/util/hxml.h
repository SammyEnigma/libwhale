/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hxml.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HXML_H
#define HXML_H

#include "whale/basic/hplatform.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"

namespace Whale
{
	namespace Util 
	{
		class HXml
		{
		public:
			explicit HXml(void);
			virtual ~HXml(void);

			std::string	encode();
			std::wstring unicodeEncode(void);

			bool save(const char* filePath);
			bool load(const char* filePath);
			bool load(const char* xml, int len);
			bool load(const std::string& xml);
			bool load(const std::wstring& xml);			

			std::string name(const char* key, int index = 0);
			std::string value(const char* key, int index = 0);
			std::string attr(const char* key, const char* attr, int index = 0);

			int children(const char* key, const char* name = 0);
			int brothers(const char* key);

			void clear();
			bool removeNode(const char* key);
			bool removeAttr(const char* key, const char* attr);

			bool addChild(const char*  key, const char*  name, int count = 1);
			bool addBrother(const char* node, const char* name);

			bool addAttr(const char* key, const char* attr, int value);
			bool addAttr(const char* key, const char* attr, const char* value);
			bool addAttr(const char* key, int index, const char* attr, const char* value);

			bool setAttr(const char* key, const char* attr, const char* value);
			bool setAttr(const char* key, const char* attr, int value);

			bool setValue(const char* key, const char* value);
			bool setValue(const char* key, int value);
			bool setValue(const char* key, int index, const char* value);

		private:
			rapidxml::xml_node<>* search(rapidxml::xml_document<>& xml, const char* key);
			rapidxml::xml_node<>* last(rapidxml::xml_document<>& xml, const std::vector<std::string>& nodes, const size_t index);
           
			std::mutex locker_;
			rapidxml::xml_node<> *rootNode;
			rapidxml::xml_document<> *document_;
			char* xmlstr_;
		};
	}
}

#endif

