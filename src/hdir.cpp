#include "whale/util/hdir.h"
#include "whale/basic/hlog.h"
#include "whale/util/hfile.h"

namespace Whale
{
	namespace Util
	{
		namespace HDir
		{
			bool mkdir(const std::string& dir)
			{
				if (dir.empty() || dir.length() > MAX_PATH)
				{
					return false;
				}

				if (access(dir.c_str(), 0) != -1)
				{
					return true;
				}

				size_t dirlen = dir.length();
				const char* dircur = dir.data();

				int pos = 0;
				char dirtmp[MAX_PATH] = { 0 };

				while (*dircur++ != '\0')
				{
					dirtmp[pos++] = *(dircur - 1);

					if (*dircur == '\\' || *dircur == '/' || *dircur == '\0')
					{
						if (0 != access(dirtmp, 0) && strlen(dirtmp) > 0)
						{
#ifdef OS_WIN32
							if (::_mkdir(dirtmp) != 0)
							{
								return false;
							}
#else
							if (::mkdir(dirtmp,0755) != 0)
							{
								return false;
							}
#endif
						}
					}
				}

				return true;
			}

			bool traversal(const std::string& dir, std::list<std::string>& filelist, int depth)
			{
				std::string dirTmp = dir;

#ifdef OS_WIN32
				_finddata_t FileInfo;
				std::string strfind = dirTmp + "/*";
				long hFile = _findfirst(strfind.c_str(), &FileInfo);

				if (hFile == -1L)
				{
					log_error("Can't Open Folder [%s] !!", dirTmp.c_str());
					return false;
				}

				while (_findnext(hFile, &FileInfo) == 0)
				{
					if (FileInfo.attrib & _A_SUBDIR)
					{
						if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
						{
							if (depth > 0)
							{
								Whale::Util::HDir::traversal(dirTmp + "/" + FileInfo.name, filelist, --depth);
							}
						}
					}
					else
					{
						filelist.push_back(dirTmp + "/" + FileInfo.name);
					}
				}

				_findclose(hFile);
#else
				DIR *dp;
				struct dirent *entry;
				struct stat statbuf;
				if ((dp = opendir(dirTmp.c_str())) == NULL) 
				{
					log_error("Can't Open Folder [%s] !!", dirTmp.c_str());
					return false;
				}

				chdir(dirTmp.c_str());

				while ((entry = readdir(dp)) != NULL)
				{					
					lstat(entry->d_name, &statbuf);

					if (strncmp(entry->d_name, ".", 1) == 0)
					{
						if (S_ISDIR(statbuf.st_mode))
						{
							if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
							{
								continue;
							}

							if (depth > 0)
							{
								Whale::Util::HDir::traversal(entry->d_name, filelist, --depth);
							}
						}
					}					
					else 
					{
						filelist.push_back(dirTmp + "/" + entry->d_name);
					}
				}

				chdir("..");
				closedir(dp);
#endif
				return true;
			}


			bool traversalFolder(const std::string& dir, std::list<std::string>& dirlist, int depth)
			{
				std::string dirTmp = dir;

#ifdef OS_WIN32
				_finddata_t FileInfo;
				std::string strfind = dirTmp + "/*";
				long hFile = _findfirst(strfind.c_str(), &FileInfo);

				if (hFile == -1L)
				{
					log_error("Can't Open Folder [%s] !!", dirTmp.c_str());
					return false;
				}

				while (_findnext(hFile, &FileInfo) == 0)
				{
					if (FileInfo.attrib & _A_SUBDIR)
					{
						if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
						{
							dirlist.push_back(dirTmp + "/" + FileInfo.name);

							if (depth > 0)
							{
								Whale::Util::HDir::traversalFolder(dirTmp + "/" + FileInfo.name, dirlist, --depth);
							}
						}
					}
				}

				_findclose(hFile);
#else
				DIR *dp;
				struct dirent *entry;
				struct stat statbuf;
				if ((dp = opendir(dirTmp.c_str())) == NULL)
				{
					log_error("Can't Open Folder [%s] !!", dirTmp.c_str());
					return false;
				}

				chdir(dirTmp.c_str());

				while ((entry = readdir(dp)) != NULL)
				{
					lstat(entry->d_name, &statbuf);

					if (S_ISDIR(statbuf.st_mode))
					{
						if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
						{
							continue;
						}

						dirlist.push_back(dirTmp + "/" + entry->d_name);

						if (depth > 0)
						{
							Whale::Util::HDir::traversal(entry->d_name, dirlist, --depth);
						}
					}
				}

				chdir("..");
				closedir(dp);
#endif
				return true;
			}

			bool remove(const std::string& dir)
			{
				std::string dirTmp = dir;

#ifdef OS_WIN32
				_finddata_t FileInfo;
				std::string strfind = dirTmp + "/*";
				long hFile = _findfirst(strfind.c_str(), &FileInfo);

				if (hFile == -1L)
				{
					log_error("Can't Open Folder [%s] !!", dirTmp.c_str());
					return false;
				}

				while (_findnext(hFile, &FileInfo) == 0)
				{
					if (FileInfo.attrib & _A_SUBDIR)
					{
						if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
						{
							Whale::Util::HDir::remove(dirTmp + "/" + FileInfo.name);
						}
					}
					else
					{
						DeleteFile((dirTmp + "/" + FileInfo.name).c_str());
						log_notice("Delete File [%s] !!", (dirTmp + "/" + FileInfo.name).c_str());
					}
				}

				_findclose(hFile);
#else
				DIR *dp;
				struct dirent *entry;
				struct stat statbuf;
				if ((dp = opendir(dirTmp.c_str())) == NULL)
				{
					log_error("Can't Open Folder [%s] !!", dirTmp.c_str());
					return false;
				}

				chdir(dirTmp.c_str());

				while ((entry = readdir(dp)) != NULL)
				{
					lstat(entry->d_name, &statbuf);

					if (strncmp(entry->d_name, ".", 1) == 0)
					{
						if (S_ISDIR(statbuf.st_mode))
						{
							if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
							{
								continue;
							}

							Whale::Util::HDir::remove(dirTmp + "/" + entry->d_name);
						}
						else
						{
							Whale::Util::HFile::remove(dirTmp + "/" + entry->d_name);
							log_notice("Delete File [%s] !!", (dirTmp + "/" + entry->d_name).c_str());
						}
					}
				}

				chdir("..");
				closedir(dp);
#endif

				log_notice("Delete Folder [%s] !!", dirTmp.c_str());
				return rmdir(dirTmp.c_str()) == 0;
			}
		}
	}
}