#include "whale/util/hfile.h"
#include "whale/util/hdir.h"
#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Util
	{
		namespace HFile
		{
			bool write(const char *filePath, const char* buffer, int size, bool flag)
			{
				assert(filePath);
				assert(buffer);
				assert(size > 0);
				assert(strlen(filePath) < MAX_PATH);

				char directory[MAX_PATH] = { 0 };
				memcpy(directory, filePath, strlen(filePath));

				for (size_t i = 0; i < strlen(directory); i++)
				{
					if (directory[i] == '\\' || directory[i] == '/')
					{
						directory[i] = '/';
					}
				}

				*(strrchr(directory, '/') + 1) = 0;

				if (!Whale::Util::HDir::mkdir(directory))
				{
					return false;
				}

				FILE* fp = fopen(filePath, flag ? "ab+" : "wb+");

				if (fp == NULL)
				{
					return false;
				}

				if (fwrite(buffer, 1, size, fp) == size)
				{
					fclose(fp);
					return true;
				}

				fclose(fp);
				return false;
			}
		
			bool move(const std::string& fp1, const std::string& fp2)
			{
				char directory[MAX_PATH] = { 0 };
				memcpy(directory, fp2.c_str(), fp2.length());

				for (size_t i = 0; i < strlen(directory); i++)
				{
					if (directory[i] == '\\' || directory[i] == '/')
					{
						directory[i] = '/';
					}
				}

				*(strrchr(directory, '/') + 1) = 0;

				if (!Whale::Util::HDir::mkdir(directory))
				{
					return false;
				}

#ifdef OS_WIN32
				if (!MoveFile(fp1.c_str(), fp2.c_str()))
				{
					DeleteFile(fp1.c_str());
				}				
#else
				system(std::string("mv " + fp1 + " " + fp2).c_str());
#endif
				return true;
			}

			bool remove(const std::string& filePath)
			{
#ifdef OS_WIN32
				DeleteFile(filePath.c_str());
#else
				system(std::string("rm -f " + filePath).c_str());
#endif
				return true;
			}
		}
	}
}
