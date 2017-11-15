#include "whale/util/hxml.h"
#include "whale/util/hstring.h"

namespace Whale
{
	namespace Util 
	{
		HXml::HXml(void)
		{
			xmlstr_ = 0;

			document_ = new rapidxml::xml_document<>;
			document_->append_node(document_->allocate_node(rapidxml::node_pi, 
				document_->allocate_string("xml version=\"1.0\" encoding=\"UTF-8\"")));
		}

		HXml::~HXml(void)
		{
			document_->clear();
			delete document_;
		}

		rapidxml::xml_node<>* HXml::search(rapidxml::xml_document<>& xml, const char* key)
		{
			if (key == 0)
			{
				return 0;
			}

			std::vector<std::string> nodes = Whale::Util::HString::split(key, ".");

			return last(xml, nodes, nodes.size());
		}

		rapidxml::xml_node<>* HXml::last(rapidxml::xml_document<>& xml, const std::vector<std::string>& nodes, const size_t index)
		{
			if (nodes.size() <= 0)
			{
				return 0;
			}

			rapidxml::xml_node<>* node = xml.first_node(nodes.at(0).c_str());

			if (nodes.size() == 1 || node == 0)
			{
				return node;
			}

			try
			{
				for (size_t i = 1; i < index; i++)
				{
					std::string temp = nodes.at(i);
					size_t tsize = temp.size();
					size_t tpos = temp.find("-");

					if (tpos != std::string::npos)
					{
						std::string nstr = temp.substr(0, tpos);
						std::string subt = temp.substr(tpos + 1, tsize - tpos + 1);
						int ni = atoi(subt.c_str());
						node = node->first_node(nstr.c_str());
						for (int j = 1; (j <= ni && node != 0); ++j, node = node->next_sibling(nstr.c_str()));
					}
					else
					{
						node = node->first_node(temp.c_str());
					}	
				}
			}
			catch (...)
			{
				return 0;
			}

			return node;
		}

		bool HXml::setAttr(const char* key, const char* attr, const char* value)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || attr == 0 || value == 0)
			{
				return false;
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node != 0)
			{
				node->remove_attribute(node->first_attribute(attr));
				node->append_attribute(document_->allocate_attribute(document_->allocate_string(attr), document_->allocate_string(value)));
				return true;
			}

			return false;
		}

		bool HXml::setAttr(const char* key, const char* attr, int value)
		{
			return setAttr(key, attr, Whale::Util::HString::convert<char*>(value));
		}

		bool HXml::setValue(const char* key, const char* value)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || value == 0)
			{
				return false;
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node != 0)
			{
				node->value(document_->allocate_string(value));
				return true;
			}

			return false;
		}

		bool HXml::setValue(const char* key, int value)
		{
			return setValue(key, Whale::Util::HString::convert<char*>(value));
		}

		bool HXml::save(const char* filePath)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (filePath == 0)
			{
				return false;
			}				

			try 
			{
				std::ofstream out(filePath);
				out << *document_;
			}	
			catch (...)	
			{
				return false;
			}

			return true;
		}


		std::string HXml::encode()
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			std::string content = "";

			try 
			{
				rapidxml::print(std::back_inserter(content), *document_, 0);
			}	
			catch (...)	
			{
				// TODO[lianping_master@163.com].....
			}

			return content;
		}

		std::wstring HXml::unicodeEncode(void)
		{
			return Whale::Util::HString::stringToWString(encode());
		}

		bool HXml::load(const char* filePath)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (filePath == 0)
			{
				return false;
			}				

			try 
			{
				rapidxml::file<> fdoc(filePath);
				xmlstr_ = document_->allocate_string(fdoc.data(), fdoc.size() + 1);
				xmlstr_[fdoc.size()] = '\0';
				document_->parse< rapidxml::parse_default | rapidxml::parse_trim_whitespace | rapidxml::parse_no_data_nodes >(xmlstr_);
			}	
			catch (...)	
			{
				return false;
			}

			return true;
		}


		bool HXml::load(const char* xml, int len)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (0 == xml)
			{
				return false;
			}				

			try
			{
				xmlstr_ = document_->allocate_string(xml, len + 1);
				xmlstr_[len] = '\0';
				document_->parse< rapidxml::parse_default | rapidxml::parse_trim_whitespace | rapidxml::parse_no_data_nodes >(xmlstr_);
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		bool HXml::load(const std::string& xml)
		{
			return load(xml.c_str(), xml.length());
		}

		bool HXml::load(const std::wstring& xml)
		{
			std::string value = Whale::Util::HString::wstringToString(xml);
			return load(value.c_str(), value.length());
		}	

		std::string HXml::value(const char* key, int index)
		{
			std::unique_lock<std::mutex> lock{ locker_ };

			if (key == 0 || index < 0)
			{
				return "";
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node != 0) 
			{
				if (index == 0)	
				{
					return node->value();
				}	
				else	
				{
					for (int j = 1; (j <= index && node != 0); ++j, node = node->next_sibling());
					return node != 0 ? node->value() : "";
				}
			}

			return "";
		}

		std::string HXml::attr(const char* key, const char* attr, int index)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || attr == 0 || index < 0)
			{
				return "";
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node == 0)
			{
				return "";
			}				

			if (index == 0)
			{
				return node->first_attribute(attr) == 0 ? "" : node->first_attribute(attr)->value();
			}

			for (int j = 0; j < index; ++j)
			{
				node = node->next_sibling();

				if (node == 0)
				{
					return "";
				}					
			}

			return node->first_attribute(attr) == 0 ? "" : node->first_attribute(attr)->value();
		}	

		void HXml::clear()
		{ 
			std::unique_lock<std::mutex> lock{ locker_ }; 		

			document_->clear();
			delete document_;
			
			document_ = new rapidxml::xml_document<>;
			rapidxml::xml_node<>* head = document_->allocate_node(rapidxml::node_pi, document_->allocate_string("xml version=\"1.0\" encoding=\"utf-8\""));
			document_->append_node(head);
		}

		bool HXml::removeNode(const char* key)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0)
			{
				return false;
			}

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node != 0)
			{
				node->parent()->remove_node(node);
				return true;
			}

			return false;
		}

		bool HXml::removeAttr(const char* key, const char* attr)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || attr == 0)
			{
				return false;
			}

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node != 0)
			{
				node->remove_attribute(node->first_attribute(attr));
				return true;
			}

			return false;
		}

		int HXml::brothers(const char* key)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0)
			{
				return -1;
			}				

			std::vector<std::string> nodes = Whale::Util::HString::split(key, ".");

			try
			{
				rapidxml::xml_node<>* node = last(*document_, nodes, nodes.size());
				std::string nt = nodes.at(nodes.size() - 1);

				if (node == 0)
				{
					return 0;
				}

				int brothers = 1;

				rapidxml::xml_node<>* psibling = node->next_sibling(nt.c_str());

				while (psibling != 0)
				{
					brothers++;
					psibling = psibling->next_sibling(nt.c_str());
				}

				return brothers;
			}
			catch (...)
			{
				return -1;
			}

			return 0;
		}


		int HXml::children(const char* key, const char* name)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0)
			{
				return -1;
			}				

			std::vector<std::string> nodes = Whale::Util::HString::split(key, ".");

			try
			{
				rapidxml::xml_node<>* node = last(*document_, nodes, nodes.size());

				if (node == 0)
				{
					return 0;
				}

				int childs = 0;
				rapidxml::xml_node<>* child = node->first_node();

				while (child != 0)
				{
					if (name == 0)
					{
						childs++;
					}						
					else
					{
						(strcmp(child->name(), name) == 0) && childs++;
					}

					child = child->next_sibling();
				}

				return childs;
			}
			catch (...)
			{
				return -1;
			}

			return 0;
		}

		bool HXml::addChild(const char*  key, const char*  name, int count)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || name == 0)
			{
				return false;
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node == 0)
			{
				return false;
			}
			
			for (int i = 0; i < count; i++)
			{
				node->append_node(document_->allocate_node(rapidxml::node_element, document_->allocate_string(name)));
			}

			return true;
		}

		bool HXml::addBrother(const char* node, const char* name)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (node == 0 || name == 0)
			{
				return false;
			}				

			rapidxml::xml_node<>* pnode = search(*document_, node);

			if (pnode == 0 || pnode->parent() == 0)
			{
				return false;
			}

			pnode->parent()->append_node(document_->allocate_node(rapidxml::node_element, document_->allocate_string(name)));
			
			return true;
		}

		bool HXml::addAttr(const char* key, const char* attr, int value)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || attr == 0)
			{
				return false;
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node == 0)
			{
				return false;
			}

			node->append_attribute(document_->allocate_attribute(document_->allocate_string(attr),
				document_->allocate_string(Whale::Util::HString::convert<char*>(value))));

			return true;
		}


		bool HXml::addAttr(const char* key, const char* attr, const char* value)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || attr == 0 || value == 0)
			{
				return false;
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node == 0)
			{
				return false;
			}

			node->append_attribute(document_->allocate_attribute(document_->allocate_string(attr), document_->allocate_string(value)));
			
			return true;
		}

		bool HXml::addAttr(const char* key, int index, const char* attr, const char* value)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || attr == 0 || value == 0 || index < 0)
			{
				return false;
			}			

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node == 0)
			{
				return false;
			}

			if (index == 0)
			{
				node->append_attribute(document_->allocate_attribute(document_->allocate_string(attr), document_->allocate_string(value)));
				
				return true;
			}
			else
			{
				for (int j = 1; (j <= index && node != 0); ++j, node = node->next_sibling());

				if (node == 0)
				{
					return false;
				}

				node->append_attribute(document_->allocate_attribute(document_->allocate_string(attr), document_->allocate_string(value)));
				
				return true;
			}
		}
		
		bool HXml::setValue(const char* key, int index, const char* value)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || value == 0 || index < 0)
			{
				return false;
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node == 0)
			{
				return false;
			}

			if (index == 0)
			{
				node->value(document_->allocate_string(value));
				
				return true;
			}
			else
			{
				for (int j = 1; (j <= index && node != 0); ++j, node = node->next_sibling());

				if (node == 0)
				{
					return false;
				}

				node->value(document_->allocate_string(value));

				return true;
			}
		}

		std::string HXml::name(const char* key, int index)
		{
			std::unique_lock<std::mutex> lock{ locker_ }; 

			if (key == 0 || index < 0)
			{
				return "";
			}				

			rapidxml::xml_node<>* node = search(*document_, key);

			if (node == 0)
			{
				return "";
			}

			if (index == 0)	
			{
				return std::string(node->name());
			}
			else	
			{
				for (int j = 1; (j <= index && node != 0); ++j, node = node->next_sibling()) {};
				return node != 0 ? node->name() : "";
			}
		}
	}
}
