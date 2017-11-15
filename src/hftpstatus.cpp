#include "whale/net/hftpstatus.h"

namespace Whale
{
	namespace Net
	{

		HFtpFileStatus::HFtpFileStatus() :
			_cmdEnable(false),
			_retryEnable(false),
			_sizeType(FTP_FILE_ENCODE_UNKNOWN),
			_size(-1),
			_mtimeType(FTP_FILE_MTIME_UNKNOWN),
			_mtime(0),
			_idType(FTP_FILE_IDTYPE_UNKNOWN)
		{
		}

		HFtpFileStatus::HFtpFileStatus(const HFtpFileStatus& src) :
			_name(src._name),
			_path(src._path),
			_cmdEnable(src._cmdEnable),
			_retryEnable(src._retryEnable),
			_sizeType(src._sizeType),
			_size(src._size),
			_mtimeType(src._mtimeType),
			_mtime(src._mtime),
			_attributes(src._attributes),
			_uid(src._uid),
			_gid(src._gid),
			_link(src._link),
			_idType(src._idType),
			_id(src._id)
#ifdef _DEBUG
			, _mtimeStr(src._mtimeStr)
#endif
		{
		}

		HFtpFileStatus::~HFtpFileStatus()
		{
		}

		HFtpFileStatus& HFtpFileStatus::operator=(const HFtpFileStatus& rhs)
		{
			if (&rhs == this)
				return *this;

			_name = rhs._name;
			_path = rhs._path;
			_cmdEnable = rhs._cmdEnable;
			_retryEnable = rhs._retryEnable;
			_sizeType = rhs._sizeType;
			_size = rhs._size;
			_mtimeType = rhs._mtimeType;
			_mtime = rhs._mtime;
			_attributes = rhs._attributes;
			_uid = rhs._uid;
			_gid = rhs._gid;
			_link = rhs._link;
			_idType = rhs._idType;
			_id = rhs._id;
#ifdef _DEBUG
			_mtimeStr = rhs._mtimeStr;
#endif

			return *this;
		}

		bool HFtpFileStatus::operator==(const HFtpFileStatus& rhs) const
		{
			return _name == rhs._name                &&
				_path == rhs._path                &&
				_cmdEnable == rhs._cmdEnable                &&
				_retryEnable == rhs._retryEnable               &&
				_sizeType == rhs._sizeType             &&
				_size == rhs._size                  &&
				_mtimeType == rhs._mtimeType &&
				_mtime == rhs._mtime                  &&
				_attributes == rhs._attributes          &&
				_uid == rhs._uid                 &&
				_gid == rhs._gid                 &&
				_link == rhs._link                &&
				_idType == rhs._idType               &&
#ifdef _DEBUG
				_mtimeStr == rhs._mtimeStr               &&
#endif
				_id == rhs._id;
		}

		bool HFtpFileStatus::operator!=(const HFtpFileStatus& rhs) const
		{
			return !operator==(rhs);
		}

		void HFtpFileStatus::reset()
		{
			_name.erase();
			_path.erase();
			_cmdEnable = false;
			_retryEnable = false;
			_sizeType = FTP_FILE_ENCODE_UNKNOWN;
			_size = -1;
			_mtimeType = FTP_FILE_MTIME_UNKNOWN;
			_mtime = 0;
			_attributes.erase();
			_uid.erase();
			_gid.erase();
			_link.erase();
			_idType = FTP_FILE_IDTYPE_UNKNOWN;
			_id.erase();
#ifdef _DEBUG
			_mtimeStr.erase();
#endif
		}

	}
}