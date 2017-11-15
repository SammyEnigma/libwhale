/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hoptions.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HOPTIONS_H
#define HOPTIONS_H

#include "whale/basic/hplatform.h"

namespace Whale 
{
	namespace Util
	{
		struct HOptionsItem
		{
			int _key;
			std::string _longopt;
			std::string _desc;
			bool _hasArg;

			bool operator==(int key) const {
				return _key == key;
			}
		};
		
		class HOptions
		{
		public:
			explicit HOptions(const std::string &version = "1.0.0");
			virtual ~HOptions();

			virtual void help() const;
			virtual void version() const;

			void insert(int key, const std::string &longopt, const std::string& desc, bool hasArg = true);
			bool load(int argc, char* argv[]);

			bool contain(int key) const;
			int readInt32(int key) const;
			std::string readString(int key) const;

			void setVersionString(const std::string &ver);
			void setBriefString(const std::string &brief);
		private:
			std::vector<HOptionsItem> items_;
			std::map<int, std::string> options_;	
			std::string version_;
			std::string brief_;
		};
	}
}
#endif

