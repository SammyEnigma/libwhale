#ifndef __FTPFILESTATUS_H
#define __FTPFILESTATUS_H

#include "whale/basic/hplatform.h"

namespace Whale
{
	namespace Net
	{

		class HFtpFileStatus
		{
		public:
			enum FtpFileEncodeEnum {
				FTP_FILE_ENCODE_UNKNOWN,
				FTP_FILE_ENCODE_BINARY,        ///< size is the number of octets in TYPE I
				FTP_FILE_ENCODE_ASCII,         ///< size is the number of octets in TYPE A
			};

			/// When a time zone is unknown, it is assumed to be GMT. You may want
			/// to use localtime() for LOCAL times, along with an indication that the
			/// time is correct in the local time zone, and gmtime() for REMOTE* times.
			enum FtpFileMTimeTypeEnum {
				FTP_FILE_MTIME_UNKNOWN,
				FTP_FILE_MTIME_LOCAL,        ///< time is correct
				FTP_FILE_MTIME_REMOTE_MINUTE, ///< time zone and secs are unknown
				FTP_FILE_MTIME_REMOTE_DAY,    ///< time zone and time of day are unknown
			};

			enum FtpFileIdTypeEnum {
				FTP_FILE_IDTYPE_UNKNOWN,
				FTP_FILE_IDTYPE_FULL,          ///< unique identifier for files on this FTP server
			};

		public:
			HFtpFileStatus();
			HFtpFileStatus(const HFtpFileStatus& src);
			~HFtpFileStatus();

			HFtpFileStatus& operator=(const HFtpFileStatus& rhs);
			bool operator==(const HFtpFileStatus& rhs) const;
			bool operator!=(const HFtpFileStatus& rhs) const;

		public:
			void reset();

			inline const std::string& name() const {
				return _name;
			};

			inline const std::string& path() const {
				return _path;
			};

			inline bool cmdEnable()  const {
				return _cmdEnable;
			};

			inline bool retryEnable() const {
				return _retryEnable;
			};

			inline FtpFileEncodeEnum   sizeType() const {
				return _sizeType;
			};

			inline long size() const {
				return _size;
			};

			inline FtpFileMTimeTypeEnum  mtimeType() const {
				return _mtimeType;
			};

			inline time_t mtime() const {
				return _mtime;
			};

			inline const std::string& attributes() const {
				return _attributes;
			};

			inline const std::string& uid() const {
				return _uid;
			};

			inline const std::string& gid() const {
				return _gid;
			};

			inline const std::string& link() const {
				return _link;
			};

			inline FtpFileIdTypeEnum idType() const {
				return _idType;
			};

			inline const std::string& id() const {
				return _id;
			};

			inline bool isDot() const {
				return _name == (".") || _name == ("..");
			};

#ifdef _DEBUG
			inline const std::string& mtimeStr() const {
				return _mtimeStr;
			};
#endif

			inline std::string& name() {
				return _name;
			};

			inline std::string& path() {
				return _path;
			};

			inline bool& cmdEnable()  {
				return _cmdEnable;
			};

			inline bool& retryEnable() {
				return _retryEnable;
			};

			inline FtpFileEncodeEnum&  sizeType() {
				return _sizeType;
			};

			inline long& Size() {
				return _size;
			};

			inline FtpFileMTimeTypeEnum& mtimeType() {
				return _mtimeType;
			};

			inline time_t& mtime() {
				return _mtime;
			};

			inline std::string& attributes() {
				return _attributes;
			};

			inline std::string& uid() {
				return _uid;
			};

			inline std::string& gid() {
				return _gid;
			};

			inline std::string& link() {
				return _link;
			};

			inline FtpFileIdTypeEnum& idType() {
				return _idType;
			};

			inline std::string& id() {
				return _id;
			};

#ifdef _DEBUG
			inline std::string& mtimeStr() {
				return _mtimeStr;
			};
#endif

		private:
			std::string _name;
			std::string _path;
			bool _cmdEnable; ///< false if cwd is definitely pointless, true otherwise
			bool _retryEnable; ///< false if RETR is definitely pointless, true otherwise 
			FtpFileEncodeEnum _sizeType;
			long _size; ///< number of octets
			FtpFileMTimeTypeEnum _mtimeType;
			time_t	_mtime; ///< modification time
			std::string	_attributes; ///< file attributes
			std::string	_uid; ///< user identifier (owner of file)
			std::string	_gid; ///< group identifier
			std::string	_link; ///< number of links to file
			FtpFileIdTypeEnum _idType; ///< identifier type
			std::string _id; ///< identifier
#ifdef _DEBUG
			std::string _mtimeStr; ///< for debug only
#endif
		};

		class HFtpFileStatusContainer
		{
			class HFtpSortCriteria
			{
			public:
				virtual ~HFtpSortCriteria() {}
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const = 0;
			};

			const HFtpSortCriteria& _sortBase;  ///< sortkey
			const bool           _ascending; ///< sort order
			const bool           _dirFirst;  ///< if true, then directories are before the files
			///< if false, directories and files are mixed up

			HFtpFileStatusContainer& operator=(const HFtpFileStatusContainer&); // no implementation for assignment operator

		public:
			HFtpFileStatusContainer(const HFtpSortCriteria& rSortBase, bool fAscending, bool fDirFirst) :
				_sortBase(rSortBase),
				_ascending(fAscending),
				_dirFirst(fDirFirst) {
			}

			/// entry function for STL sort algorithm
			bool operator()(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const	{
				if (_dirFirst) {
					if (x->name().size() < 3 || y->name().size() < 3) {
						if (x->name().at(0) == ('.') && y->name().at(0) == ('.'))
							return _ascending && x->name() == (".");
						else if (x->name().at(0) == ('.'))
							return _ascending && true;
						else if (y->name().at(0) == ('.'))
							return _ascending && false;
					}

					if (x->cmdEnable() != y->cmdEnable()) {
						if (x->cmdEnable() && !y->cmdEnable())
							return true;
						return false;
					}
				}

				if (_ascending)
					return !_sortBase.compare(x, y);
				else
					return _sortBase.compare(x, y);
			}

			class HFtpSortCriteriaName : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const {
					return x->name() > y->name();
				}
			};

			/// Order by size.
			class HFtpSortCriteriaSize : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const
				{
					return x->Size() > y->Size();
				}
			};

			/// Order by modification date.
			class HFtpSortCriteriaMTime : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const
				{
					return x->mtime() > y->mtime();
				}
			};

			/// Order by attributes.
			class HFtpSortCriteriaAttributes : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const
				{
					return x->attributes() > y->attributes();
				}
			};

			/// Order by user id.
			class HFtpSortCriteriaUid : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const
				{
					return x->uid() > y->uid();
				}
			};

			/// Order by group id.
			class HFtpSortCriteriaGid : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const
				{
					return x->gid() > y->gid();
				}
			};

			/// Order by link.
			class HFtpSortCriteriaLink : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const
				{
					return x->link() > y->link();
				}
			};

			/// Order by id.
			class HFtpSortCriteriaId : public HFtpSortCriteria
			{
			public:
				virtual bool compare(std::shared_ptr<HFtpFileStatus> x, std::shared_ptr<HFtpFileStatus> y) const
				{
					return x->id() > y->id();
				}
			};
		};
	}
}

#endif // __FTPFILESTATUS_H
