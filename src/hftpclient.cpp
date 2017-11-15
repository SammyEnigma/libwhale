#include "whale/net/hftpclient.h"
#include "whale/net/hftpparser.h"
#include "whale/net/hftpstatus.h"

#include "whale/basic/herror.h"
#include "whale/basic/hlog.h"
#include "whale/util/hstring.h"
#include "whale/basic/hos.h"

#undef max

namespace Whale
{
	namespace Net
	{
		class HFtpMakeString
		{
		public:
			HFtpMakeString& operator<<(unsigned int dwNum) {
				// unsigned int dwTemp = dwNum;
				// int iCnt = 1; // name lookup of 'iCnt' changed for new ISO 'for' scoping
				// for (; (dwTemp /= 10) != 0; iCnt++)
				// 	;

				// m_str.resize(m_str.size() + iCnt);
				// // string size +1 because tsprintf needs the size including the terminating NULL character;
				// // the size method returns the size without the terminating NULL character
				// snprintf(&(*m_str.begin()), m_str.size() + 1, ("%s%u"), m_str.c_str(), dwNum);

				char buf[12] = {0x00};
				snprintf(buf, sizeof(buf) - 1, "%u", dwNum);
				m_str += buf;
				return *this;
			}

			HFtpMakeString& operator<<(const std::string& strAdd) {
				m_str += strAdd;
				return *this;
			}

			operator std::string() const { return m_str; }

		private:
			std::string m_str;
		};

		class IFtpTransferNotification : public IFtpDataType
		{
		public:
			virtual ~IFtpTransferNotification() {}
			virtual std::string localStreamName() const = 0;
			virtual unsigned int localStreamSize() const = 0;
			virtual void localStreamOffset(unsigned int dwOffsetFromBeginOfStream) = 0;
			virtual void onBytesReceived(const std::vector<char>& /*vBuffer*/, long /*lReceivedBytes*/) {}
			virtual void onPreBytesSend(char* /*pszBuffer*/, size_t /*bufferSize*/, size_t& /*bytesToSend*/) {}
		};

		class HFtpPimpl
		{
			HFtpPimpl& operator=(const HFtpPimpl&); // no implementation for assignment operator
		public:
			HFtpPimpl(const std::string& strEolCharacterSequence, const std::string& strStreamName) :
				_eolCharacterSequence(strEolCharacterSequence),
				_currentPos(_vBuffer.end()),
				_streamName(strStreamName) {
			}

			const std::string _eolCharacterSequence;
			std::string _vBuffer;
			std::string::iterator _currentPos;
			std::string _streamName;
		};

		class HFtpFileTransfer : public IFtpTransferNotification
		{
		private:
			FILE* _pFile;
			std::string _fileName;

		public:
			enum TOriginEnum { orBegin = SEEK_SET, orEnd = SEEK_END, orCurrent = SEEK_CUR };

			HFtpFileTransfer() : _pFile(NULL) {
			}

			virtual ~HFtpFileTransfer()	{
				closeFile();
			}

			bool openFile(const std::string& strFileName, const std::string& strMode) {
				_fileName = strFileName;
#if _MSC_VER >= 1500
				return fopen_s(&_pFile, strFileName.c_str(), strMode.c_str()) == 0;
#else
				_pFile = fopen(strFileName.c_str(), strMode.c_str());
				return _pFile != NULL;
#endif
			}

			bool closeFile() {
				FILE* pFile = _pFile;
				_pFile = NULL;
				return pFile && fclose(pFile) == 0;
			}

			bool seekFile(long lOffset, TOriginEnum enOrigin) {
				return _pFile && fseek(_pFile, lOffset, enOrigin) == 0;
			}

			long tellFile()	{
				return _pFile ? ftell(_pFile) : -1L;
			}

			size_t writeFile(const void* pBuffer, size_t itemSize, size_t itemCount) {
				if (!_pFile)
					return 0;
				return fwrite(pBuffer, itemSize, itemCount, _pFile);
			}

			size_t readFile(void* pBuffer, size_t itemSize, size_t itemCount) {
				return _pFile ? fread(pBuffer, itemSize, itemCount, _pFile) : 0;
			}

			virtual std::string localStreamName() const {
				return _fileName;
			}

			virtual unsigned int localStreamSize() const	{
				if (!_pFile) {
					return 0;
				}

				const long lCurPos = ftell(_pFile);
				fseek(_pFile, 0, SEEK_END);
				const long lEndPos = ftell(_pFile);
				fseek(_pFile, lCurPos, SEEK_SET);

				return lEndPos;
			}

			virtual void localStreamOffset(unsigned int dwOffsetFromBeginOfStream) {
				seekFile(dwOffsetFromBeginOfStream, HFtpFileTransfer::orBegin);
			}

			virtual void onBytesReceived(const std::vector<char>& vBuffer, long lReceivedBytes)	{
				writeFile(&(*vBuffer.begin()), sizeof(std::vector<char>::value_type), lReceivedBytes);
			}

			virtual void onPreBytesSend(char* pszBuffer, size_t bufferSize, size_t& bytesToSend) {
				bytesToSend = readFile(pszBuffer, sizeof(char), bufferSize);
			}
		};

		class HFtpOutputStream : public IFtpTransferNotification
		{
        private:
			std::shared_ptr<HFtpPimpl> m_spPimpl;

		public:
			HFtpOutputStream(
				const std::string& strEolCharacterSequence,
				const std::string& strStreamName) :
				m_spPimpl(new HFtpPimpl(strEolCharacterSequence, strStreamName)) {
			}

			~HFtpOutputStream() {
			}

			void buffer(const std::string& strBuffer) {
				m_spPimpl->_vBuffer = strBuffer;
			}

			const std::string& buffer() const {
				return m_spPimpl->_vBuffer;
			}

			void startPosition()	{
				m_spPimpl->_currentPos = m_spPimpl->_vBuffer.begin();
			}

			bool nextLine(std::string& strLine) {
				std::string::iterator it = std::search(m_spPimpl->_currentPos,
					m_spPimpl->_vBuffer.end(), m_spPimpl->_eolCharacterSequence.begin(),
					m_spPimpl->_eolCharacterSequence.end());
				if (it == m_spPimpl->_vBuffer.end())
					return false;

				strLine.assign(m_spPimpl->_currentPos, it);

				m_spPimpl->_currentPos = it + m_spPimpl->_eolCharacterSequence.size();

				return true;
			}

			std::string localStreamName() const {
				return m_spPimpl->_streamName;
			}

			unsigned int localStreamSize() const {
				return m_spPimpl->_vBuffer.size();
			}

			void localStreamOffset(unsigned int dwOffsetFromBeginOfStream) {
				m_spPimpl->_currentPos = m_spPimpl->_vBuffer.begin() + dwOffsetFromBeginOfStream;
			}

			void onBytesReceived(const std::vector<char>& vBuffer, long lReceivedBytes)	{
				std::copy(vBuffer.begin(), vBuffer.begin() + lReceivedBytes, std::back_inserter(m_spPimpl->_vBuffer));
			}

			void onPreBytesSend(char* pszBuffer, size_t bufferSize, size_t& bytesToSend) {
				for (bytesToSend = 0; m_spPimpl->_currentPos != m_spPimpl->_vBuffer.end() && bytesToSend < bufferSize; ++m_spPimpl->_currentPos, ++bytesToSend)
					pszBuffer[bytesToSend] = *m_spPimpl->_currentPos;
			}
		};

		HFtpClient::HFtpClient(
			unsigned int uiTimeout/*=10*/,
			unsigned int uiBufferSize/*=2048*/,
			unsigned int uiResponseWait/*=0*/,
			const std::string& strRemoteDirectorySeparator/*=("/")*/) :
			_uiTimeout(uiTimeout),
			_uiResponseWait(uiResponseWait),
			_eolCharacterSequence(("\r\n")),
			_remoteDirectorySeparator(strRemoteDirectorySeparator),//+# documentation missing
			_vBuffer(uiBufferSize),
			_controlConnection(new HFtpSocket()),
			_fileListParser(new HFtpListParser()),
			_transferInProgress(false),
			_abortTransfer(false),
			_resumeEnabled(true)
		{
			assert(_controlConnection.get());
		}

		/// destructor
		HFtpClient::~HFtpClient()
		{
			if (transferring())
				abortCommand();

			if (connected())
				logout();
		}


		/// Sets the file list parser which is used for parsing the results of the LIST command.
		void HFtpClient::fileListParser(std::shared_ptr<IFtpFileListParser> apFileListParser)
		{
			_fileListParser = apFileListParser;
		}


		/// Enables or disables resuming for file transfer operations.
		/// @param[in] fEnable True enables resuming, false disables it.
		void HFtpClient::resumeMode(bool fEnable/*=true*/)
		{
			_resumeEnabled = fEnable;
		}

		/// Indicates if the resume mode is set.
		bool HFtpClient::resumeModeEnabled() const
		{
			return _resumeEnabled;
		}

		/// Opens the control channel to the FTP server.
		/// @param[in] strServerHost IP-address or name of the server
		/// @param[in] iServerPort Port for channel. Usually this is port 21.
		bool HFtpClient::openControlChannel(const std::string& strServerHost, unsigned short ushServerPort/*=DEFAULT_FTP_PORT*/)
		{
			closeControlChannel();

			if (!_controlConnection->create(SOCK_STREAM))
			{
				return false;
			}

			HFtpSockAddr adr = _controlConnection->getHostByName(strServerHost.c_str(), ushServerPort);

			if (!_controlConnection->connect(adr)) {
				_controlConnection->cleanUp();
				return false;
			}

			return true;
		}

		/// Returns the connection state of the client.
		bool HFtpClient::connected() const
		{
			return _controlConnection->operator unsigned int() != (unsigned int)(~0);
		}

		/// Returns true if a download/upload is running, otherwise false.
		bool HFtpClient::transferring() const
		{
			return _transferInProgress;
		}

		/// Closes the control channel to the FTP server.
		void HFtpClient::closeControlChannel()
		{
			_controlConnection->close_();
			_rurrentRepresentation.reset();
		}

		/// Analyse the repy code of a FTP server-response.
		/// @param[in] Reply Reply of a FTP server.
		/// @retval FTP_OK    All runs perfect.
		/// @retval FTP_ERROR Something went wrong. An other response was expected.
		/// @retval NOT_OK    The command was not accepted.
		int HFtpClient::simpleErrorCheck(const HFtpReply& Reply) const
		{
			if (Reply.replyCode().isNegativeReply())
				return FTP_NOTOK;
			else if (Reply.replyCode().isPositiveCompletionReply())
				return FTP_OK;

			assert(Reply.replyCode().isPositiveReply());

			return FTP_ERROR;
		}

		/// Logs on to a FTP server.
		/// @param[in] logonInfo Structure with logon information.
		bool HFtpClient::login(const std::string& host,
			unsigned short port,
			const std::string& user,
			const std::string& passwd,
			const std::string& account,
			const std::string& firewallHost,
			unsigned short _firewallPort,
			const std::string& firewallUser,
			const std::string& firewallPasswd,
			const HFtpFirewallType& firewallType)
		{
			_host = host;
			_port = port;
			_user = user;
			_passwd = passwd;
			_account = account;
			_firewallHost = firewallHost;
			_firewallPort = _firewallPort;
			_firewallUser = firewallUser;
			_firewallPasswd = firewallPasswd;
			_firewallType = firewallType;

			enum {
				LO = -2,      ///< Logged On
				ER = -1,      ///< Error
				NUMLOGIN = 9, ///< currently supports 9 different login sequences
			};

			int iLogonSeq[NUMLOGIN][18] = {
				// this array stores all of the logon sequences for the various firewalls
				// in blocks of 3 nums.
				// 1st num is command to send,
				// 2nd num is next point in logon sequence array if 200 series response
				//         is rec'd from server as the result of the command,
				// 3rd num is next point in logon sequence if 300 series rec'd
				{ 0, LO, 3, 1, LO, 6, 2, LO, ER }, // no firewall
				{ 3, 6, 3, 4, 6, ER, 5, ER, 9, 0, LO, 12, 1, LO, 15, 2, LO, ER }, // SITE hostname
				{ 3, 6, 3, 4, 6, ER, 6, LO, 9, 1, LO, 12, 2, LO, ER }, // USER after logon
				{ 7, 3, 3, 0, LO, 6, 1, LO, 9, 2, LO, ER }, // proxy OPEN
				{ 3, 6, 3, 4, 6, ER, 0, LO, 9, 1, LO, 12, 2, LO, ER }, // Transparent
				{ 6, LO, 3, 1, LO, 6, 2, LO, ER }, // USER with no logon
				{ 8, 6, 3, 4, 6, ER, 0, LO, 9, 1, LO, 12, 2, LO, ER }, // USER fireID@remotehost
				{ 9, ER, 3, 1, LO, 6, 2, LO, ER }, // USER remoteID@remotehost fireID
				{ 10, LO, 3, 11, LO, 6, 2, LO, ER }  // USER remoteID@fireID@remotehost
			};

			// are we connecting directly to the host (logon type 0) or via a firewall? (logon type>0)
			std::string   strTemp;
			unsigned short    ushPort = 0;

			if (_firewallType == HFtpFirewallType::none()) {
				strTemp = _host;
				ushPort = _port;
			}
			else {
				strTemp = _firewallHost;
				ushPort = _firewallPort;
			}

			std::string strHostnamePort(host);
			if (_port != DEFAULT_FTP_PORT) {
				strHostnamePort = HFtpMakeString() << _host << (":") << _port; // add port to hostname (only if port is not 21)
			}

			if (connected()) {
				logout();
			}

			if (!openControlChannel(strTemp, ushPort)) {
				return false;
			}

			// get initial connect msg off server
			HFtpReply Reply;
			if (!getResponse(Reply) || !Reply.replyCode().isPositiveCompletionReply()) {
				return false;
			}

			int logonPoint = 0;

			// go through appropriate logon procedure
			for (;;) {
				// send command, get response
				HFtpReply Reply;
				switch (iLogonSeq[_firewallType.ftpFirewallType()][logonPoint])
				{
					// state                 command           command argument                                                                              success     fail
				case 0:  if (sendCommand(HFtpCommand::USER(), _user, Reply)) break; else return false;
				case 1:  if (sendCommand(HFtpCommand::PASS(), _passwd, Reply)) break; else return false;
				case 2:  if (sendCommand(HFtpCommand::ACCT(), _account, Reply)) break; else return false;
				case 3:  if (sendCommand(HFtpCommand::USER(), _firewallUser, Reply)) break; else return false;
				case 4:  if (sendCommand(HFtpCommand::PASS(), _firewallPasswd, Reply)) break; else return false;
				case 5:  if (sendCommand(HFtpCommand::SITE(), strHostnamePort, Reply)) break; else return false;
				case 6:  if (sendCommand(HFtpCommand::USER(), _user + ("@") + strHostnamePort, Reply)) break; else return false;
				case 7:  if (sendCommand(HFtpCommand::OPEN(), strHostnamePort, Reply)) break; else return false;
				case 8:  if (sendCommand(HFtpCommand::USER(), _firewallUser + ("@") + strHostnamePort, Reply)) break; else return false;
				case 9:  if (sendCommand(HFtpCommand::USER(), _user + ("@") + strHostnamePort + (" ") + _firewallUser, Reply)) break; else return false;
				case 10: if (sendCommand(HFtpCommand::USER(), _user + ("@") + _firewallUser + ("@") + strHostnamePort, Reply)) break; else return false;
				case 11: if (sendCommand(HFtpCommand::PASS(), _passwd + ("@") + _firewallPasswd, Reply)) break; else return false;
				}

				if (!Reply.replyCode().isPositiveCompletionReply() && !Reply.replyCode().isPositiveIntermediateReply()) {
					return false;
				}

				const unsigned int uiFirstDigitOfReplyCode = atol(Reply.replyCode().code()) / 100;
				// get next command from array
				logonPoint = iLogonSeq[_firewallType.ftpFirewallType()][logonPoint + uiFirstDigitOfReplyCode - 1];

				switch (logonPoint)	{
				case ER: // ER means somewhat has gone wrong
					log_error("Logon failed.");
					return false;

				case LO: // LO means we're fully logged on
					return changeWorkingDirectoryCommand(_remoteDirectorySeparator) == FTP_OK;
				}
			}

			return false;
		}

		/// Rename a file on the FTP server.
		/// @remarks Can be used for moving the file to another directory.
		/// @param[in] strOldName Name of the file to rename.
		/// @param[in] strNewName The new name for the file.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::renameFile(const std::string& strOldName, const std::string& strNewName) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::RNFR(), strOldName, Reply))
				return FTP_ERROR;

			if (Reply.replyCode().isNegativeReply())
				return FTP_NOTOK;
			else if (!Reply.replyCode().isPositiveIntermediateReply())
			{
				assert(Reply.replyCode().isPositiveCompletionReply() || Reply.replyCode().isPositivePreliminaryReply());
				return FTP_ERROR;
			}

			if (!sendCommand(HFtpCommand::RNTO(), strNewName, Reply))
				return FTP_ERROR;

			return simpleErrorCheck(Reply);
		}

		/// Moves a file within the FTP server.
		/// @param[in] strFullSourceFilePath Name of the file which should be moved.
		/// @param[in] strFullTargetFilePath The destination where the file should be moved to (file name must be also given).
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::moveFile(const std::string& strFullSourceFilePath, const std::string& strFullTargetFilePath) const
		{
			return renameFile(strFullSourceFilePath, strFullTargetFilePath);
		}

		/// Gets the directory listing of the FTP server. Sends the LIST command to
		/// the FTP server.
		/// @param[in] strPath Starting path for the list command.
		/// @param[out] vstrFileList Returns a simple list of the files and folders of the specified directory.
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::fileList(const std::string& strPath, std::vector<std::string>& vstrFileList, bool fPasv) const
		{
			HFtpOutputStream outputStream(_eolCharacterSequence, HFtpCommand::LIST().AsString());
			if (!executeDatachannelCommand(HFtpCommand::LIST(), strPath, HFtpRepresentation(HFtpDataType::ascii()), fPasv, 0, outputStream))
				return false;

			vstrFileList.clear();
			std::string strLine;
			outputStream.startPosition();
			while (outputStream.nextLine(strLine))
				vstrFileList.push_back(strPath + strLine.c_str());

			return true;
		}

		/// Gets the directory listing of the FTP server. Sends the NLST command to
		/// the FTP server.
		/// @param[in] strPath Starting path for the list command.
		/// @param[out] vstrFileList Returns a simple list of the files and folders of the specified the directory.
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::fileNameList(const std::string& strPath, std::vector<std::string>& vstrFileList, bool fPasv) const
		{
			HFtpOutputStream outputStream(_eolCharacterSequence, HFtpCommand::NLST().AsString());
			if (!executeDatachannelCommand(HFtpCommand::NLST(), strPath, HFtpRepresentation(HFtpDataType::ascii()), fPasv, 0, outputStream))
				return false;

			vstrFileList.clear();
			std::string strLine;
			outputStream.startPosition();
			while (outputStream.nextLine(strLine))
				vstrFileList.push_back(strPath + strLine.c_str());

			return true;
		}

		/// Gets the directory listing of the FTP server. Sends the LIST command to
		/// the FTP server.
		/// @param[in] strPath Starting path for the list command.
		/// @param[out] vFileList Returns a detailed list of the files and folders of the specified directory.
		///                       vFileList contains CFTPFileStatus-Objects. These Objects provide a lot of
		///                       information about the file/folder.
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::fileList(const std::string& strPath, std::vector<std::shared_ptr<HFtpFileStatus>>& vFileList, bool fPasv) const
		{
			assert(_fileListParser.get());

			HFtpOutputStream outputStream(_eolCharacterSequence, HFtpCommand::LIST().AsString());
			if (!executeDatachannelCommand(HFtpCommand::LIST(), strPath, HFtpRepresentation(HFtpDataType::ascii()), fPasv, 0, outputStream))
				return false;

			vFileList.clear();
			std::string strLine;
			outputStream.startPosition();
			while (outputStream.nextLine(strLine))
			{
				std::shared_ptr<HFtpFileStatus> spFtpFileStatus(new HFtpFileStatus());
				if (_fileListParser->parse(*spFtpFileStatus, strLine))
				{
					spFtpFileStatus->path() = strPath;
					vFileList.push_back(spFtpFileStatus);
				}
			}

			return true;
		}

		/// Gets the directory listing of the FTP server. Sends the NLST command to
		/// the FTP server.
		/// @param[in] strPath Starting path for the list command.
		/// @param[out] vFileList Returns a simple list of the files and folders of the specified directory.
		///                       vFileList contains CFTPFileStatus-Objects. Normally these Objects provide
		///                       a lot of information about the file/folder. But the NLST-command provide
		///                       only a simple list of the directory content (no specific information).
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::fileNameList(const std::string& strPath, std::vector<std::shared_ptr<HFtpFileStatus>>& vFileList, bool fPasv) const
		{
			HFtpOutputStream outputStream(_eolCharacterSequence, HFtpCommand::NLST().AsString());
			if (!executeDatachannelCommand(HFtpCommand::NLST(), strPath, HFtpRepresentation(HFtpDataType::ascii()), fPasv, 0, outputStream))
				return false;

			vFileList.clear();
			std::string strLine;
			outputStream.startPosition();
			while (outputStream.nextLine(strLine))
			{
				std::shared_ptr<HFtpFileStatus> spFtpFileStatus(new HFtpFileStatus());
				spFtpFileStatus->path() = strPath;
				spFtpFileStatus->name() = strLine;
				vFileList.push_back(spFtpFileStatus);
			}

			return true;
		}

		/// Gets a file from the FTP server.
		/// Uses C functions for file access (very fast).
		/// @param[in] strRemoteFile Filename of the sourcefile on the FTP server.
		/// @param[in] strLocalFile Filename of the target file on the local computer.
		/// @param[in] repType Representation Type (see documentation of CRepresentation)
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::downloadFile(const std::string& strRemoteFile, const std::string& strLocalFile, const HFtpRepresentation& repType, bool fPasv) const
		{
			HFtpFileTransfer file;
			if (!file.openFile(strLocalFile, _resumeEnabled ? ("ab") : ("wb")))
			{
				log_error(Whale::Basic::HError::what().c_str());
				return false;
			}
			file.seekFile(0, HFtpFileTransfer::orEnd);

			return downloadFile(strRemoteFile, file, repType, fPasv);
		}

		/// Gets a file from the FTP server.
		/// Uses C functions for file access (very fast).
		/// @param[in] strRemoteFile Filename of the sourcefile on the FTP server.
		/// @param[in] Observer Object which receives the transfer notifications.
		/// @param[in] repType Representation Type (see documentation of CRepresentation)
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::downloadFile(const std::string& strRemoteFile, IFtpTransferNotification& Observer, const HFtpRepresentation& repType, bool fPasv) const
		{
			long lRemoteFileSize = 0;
			fileSize(strRemoteFile, lRemoteFileSize);

			const bool fRet = executeDatachannelCommand(HFtpCommand::RETR(), strRemoteFile, repType, fPasv,
				_resumeEnabled ? Observer.localStreamSize() : 0, Observer);

			return fRet;
		}

		/// Gets a file from the FTP server.
		/// The target file is on an other FTP server (FXP).
		/// NOTICE: The file is directly transferred from one server to the other server.
		/// @param[in] strSourceFile File which is on the source FTP server.
		/// @param[in] TargetFtpServer The FTP server where the downloaded file will be stored.
		/// @param[in] strTargetFile Filename of the target file on the target FTP server.
		/// @param[in] repType Representation Type (see documentation of CRepresentation)
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::downloadFile(const std::string& strSourceFile, const HFtpClient& TargetFtpServer,
			const std::string& strTargetFile, const HFtpRepresentation& repType/*=CRepresentation(CType::Image())*/,
			bool fPasv/*=true*/) const
		{
			return transferFile(*this, strSourceFile, TargetFtpServer, strTargetFile, repType, fPasv);
		}

		/// Puts a file on the FTP server.
		/// Uses C functions for file access (very fast).
		/// @param[in] strLocalFile Filename of the the local sourcefile which to put on the FTP server.
		/// @param[in] strRemoteFile Filename of the target file on the FTP server.
		/// @param[in] fStoreUnique if true, the FTP command STOU is used for saving
		///                         else the FTP command STOR is used.
		/// @param[in] repType Representation Type (see documentation of CRepresentation)
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::uploadFile(const std::string& strLocalFile, const std::string& strRemoteFile, bool fStoreUnique, const HFtpRepresentation& repType, bool fPasv) const
		{
			HFtpFileTransfer file;
			if (!file.openFile(strLocalFile, ("rb")))
			{
				log_error(Whale::Basic::HError::what().c_str());
				return false;
			}

			return uploadFile(file, strRemoteFile, fStoreUnique, repType, fPasv);
		}

		/// Puts a file on the FTP server.
		/// Uses C functions for file access (very fast).
		/// @param[in] Observer Object which receives the transfer notifications for upload requests.
		/// @param[in] strRemoteFile Filename of the target file on the FTP server.
		/// @param[in] fStoreUnique if true, the FTP command STOU is used for saving
		///                         else the FTP command STOR is used.
		/// @param[in] repType Representation Type (see documentation of CRepresentation)
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::uploadFile(IFtpTransferNotification& Observer, const std::string& strRemoteFile, bool fStoreUnique, const HFtpRepresentation& repType, bool fPasv) const
		{
			long lRemoteFileSize = 0;
			if (_resumeEnabled)
				fileSize(strRemoteFile, lRemoteFileSize);

			HFtpCommand cmd(HFtpCommand::STOR());
			if (lRemoteFileSize > 0)
				cmd = HFtpCommand::APPE();
			else if (fStoreUnique)
				cmd = HFtpCommand::STOU();

			const long lLocalFileSize = Observer.localStreamSize();
			Observer.localStreamOffset(lRemoteFileSize);

			const bool fRet = executeDatachannelCommand(cmd, strRemoteFile, repType, fPasv, 0, Observer);
			return fRet;
		}

		/// Puts a file on the FTP server.
		/// The source file is on an other FTP server (FXP).
		/// NOTICE: The file is directly transferred from one server to the other server.
		/// @param[in] SourceFtpServer A FTP server from which the file is taken for upload action.
		/// @param[in] strSourceFile File which is on the source FTP server.
		/// @param[in] strTargetFile Filename of the target file on the FTP server.
		/// @param[in] repType Representation Type (see documentation of CRepresentation)
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		bool HFtpClient::uploadFile(const HFtpClient& SourceFtpServer, const std::string& strSourceFile,
			const std::string& strTargetFile, const HFtpRepresentation& repType/*=CRepresentation(CType::Image())*/,
			bool fPasv/*=true*/) const
		{
			return transferFile(SourceFtpServer, strSourceFile, *this, strTargetFile, repType, !fPasv);
		}

		/// Transfers a file from a FTP server to another FTP server.
		/// The source file is on an other FTP server (FXP).
		/// NOTICE: The file is directly transferred from one server to the other server.
		/// @param[in] SourceFtpServer A FTP server from which the file which is copied.
		/// @param[in] strSourceFile Name of the file which is on the source FTP server.
		/// @param[in] TargetFtpServer A FTP server to which the file is copied.
		/// @param[in] strTargetFile Name of the file on the target FTP server.
		/// @param[in] repType Representation Type (see documentation of CRepresentation)
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		/*static*/ bool HFtpClient::transferFile(const HFtpClient& SourceFtpServer, const std::string& strSourceFile,
			const HFtpClient& TargetFtpServer, const std::string& strTargetFile,
			const HFtpRepresentation& repType/*=CRepresentation(CType::Image())*/,
			bool fSourcePasv/*=false*/)
		{
			// transmit representation to server
			if (SourceFtpServer.representationType(repType) != FTP_OK)
				return false;

			if (TargetFtpServer.representationType(repType) != FTP_OK)
				return false;

			const HFtpClient& PassiveServer = fSourcePasv ? SourceFtpServer : TargetFtpServer;
			const HFtpClient& ActiveServer = fSourcePasv ? TargetFtpServer : SourceFtpServer;

			// set one FTP server in passive mode
			// the FTP server opens a port and tell us the socket (ip address + port)
			// this socket is used for opening the data connection
			unsigned int  ulIP = 0;
			unsigned short ushSock = 0;
			if (PassiveServer.passiveCommand(ulIP, ushSock) != FTP_OK)
				return false;

			HFtpSockAddr csaPassiveServer(ulIP, ushSock);

			// transmit the socket (ip address + port) of the first FTP server to the
			// second server
			// the second FTP server establishes then the data connection to the first
			if (ActiveServer.portCommand(csaPassiveServer.dottedDecimal(), ushSock) != FTP_OK)
				return false;

			if (!SourceFtpServer.sendCommand(HFtpCommand::RETR(), strSourceFile))
				return false;

			HFtpReply ReplyTarget;
			if (!TargetFtpServer.sendCommand(HFtpCommand::STOR(), strTargetFile, ReplyTarget) ||
				!ReplyTarget.replyCode().isPositivePreliminaryReply())
				return false;

			HFtpReply ReplySource;
			if (!SourceFtpServer.getResponse(ReplySource) || !ReplySource.replyCode().isPositivePreliminaryReply())
				return false;

			// get response from FTP servers
			if (!SourceFtpServer.getResponse(ReplySource) || !ReplySource.replyCode().isPositiveCompletionReply() ||
				!TargetFtpServer.getResponse(ReplyTarget) || !ReplyTarget.replyCode().isPositiveCompletionReply())
				return false;

			return true;
		}

		/// Executes a commando that result in a communication over the data port.
		/// @param[in] crDatachannelCmd Command to be executeted.
		/// @param[in] strPath Parameter for the command usually a path.
		/// @param[in] representation see documentation of CFTPClient::CRepresentation
		/// @param[in] fPasv see documentation of CFTPClient::Passive
		/// @param[in] dwByteOffset Server marker at which file transfer is to be restarted.
		/// @param[in] Observer Object for observing the execution of the command.
		bool HFtpClient::executeDatachannelCommand(const HFtpCommand& crDatachannelCmd, const std::string& strPath, const HFtpRepresentation& representation,
			bool fPasv, unsigned int dwByteOffset, IFtpTransferNotification& Observer) const
		{
			if (!crDatachannelCmd.IsDatachannelCommand())
			{
				assert(false);
				return false;
			}

			if (_transferInProgress)
				return false;

			if (!connected())
				return false;

			// transmit representation to server
			if (representationType(representation) != FTP_OK)
				return false;

			std::shared_ptr<IFtpSocket> apSckDataConnection(_controlConnection->instance());

			if (fPasv)
			{
				if (!openPassiveDataConnection(*apSckDataConnection, crDatachannelCmd, strPath, dwByteOffset)) {
					return false;
				}
			}
			else
			{
				if (!openActiveDataConnection(*apSckDataConnection, crDatachannelCmd, strPath, dwByteOffset)) {
					return false;
				}
			}

			const bool fTransferOK = transferData(crDatachannelCmd, Observer, *apSckDataConnection);

			apSckDataConnection->close_();

			// get response from FTP server
			HFtpReply Reply;
			if (!fTransferOK || !getResponse(Reply) || !Reply.replyCode().isPositiveCompletionReply())
				return false;

			return true;
		}

		/// Executes a commando that result in a communication over the data port.
		/// @param[in] crDatachannelCmd Command to be executeted.
		/// @param[in] Observer Object for observing the execution of the command.
		/// @param[in] sckDataConnection Socket which is used for sending/receiving data.
		bool HFtpClient::transferData(const HFtpCommand& crDatachannelCmd, IFtpTransferNotification& Observer, IFtpSocket& sckDataConnection) const
		{
			if (crDatachannelCmd.IsDatachannelWriteCommand())
			{
				if (!sendData(Observer, sckDataConnection))
					return false;
			}
			else if (crDatachannelCmd.IsDatachannelReadCommand())
			{
				if (!receiveData(Observer, sckDataConnection))
					return false;
			}
			else
			{
				assert(false);
				return false;
			}
			return true;
		}

		/// Opens an active data connection.
		/// @param[out] sckDataConnection
		/// @param[in] crDatachannelCmd Command to be executeted.
		/// @param[in] strPath Parameter for the command usually a path.
		/// @param[in] dwByteOffset Server marker at which file transfer is to be restarted.
		bool HFtpClient::openActiveDataConnection(IFtpSocket& sckDataConnection, const HFtpCommand& crDatachannelCmd, const std::string& strPath, unsigned int dwByteOffset) const
		{
			if (!crDatachannelCmd.IsDatachannelCommand())
			{
				assert(false);
				return false;
			}

			std::shared_ptr<IFtpSocket> apSckServer(_controlConnection->instance());

			unsigned short ushLocalSock = 0;
			HFtpSockAddr csaAddressTemp(INADDR_ANY, 0);
			apSckServer->create(SOCK_STREAM);
			apSckServer->bind(csaAddressTemp);
			apSckServer->sockAddr(csaAddressTemp);
			ushLocalSock = csaAddressTemp.port();

			if (!apSckServer->listen())	{
				apSckServer->cleanUp();
				return false;
			}

			// get own ip address
			HFtpSockAddr csaLocalAddress;
			_controlConnection->sockAddr(csaLocalAddress);

			// transmit the socket (ip address + port) to the server
			// the FTP server establishes then the data connection
			if (portCommand(csaLocalAddress.dottedDecimal(), ushLocalSock) != FTP_OK)
				return false;

			// if resuming is activated then set offset
			if (_resumeEnabled &&
				(crDatachannelCmd == HFtpCommand::STOR() || crDatachannelCmd == HFtpCommand::RETR() || crDatachannelCmd == HFtpCommand::APPE()) &&
				(dwByteOffset != 0 && restart(dwByteOffset) != FTP_OK))
				return false;

			// send FTP command RETR/STOR/NLST/LIST to the server
			HFtpReply Reply;
			if (!sendCommand(crDatachannelCmd, strPath, Reply) ||
				!Reply.replyCode().isPositivePreliminaryReply()) {
				log_error("Reply: %s", Reply.replyCode().code());
				return false;
			}

			// accept the data connection
			HFtpSockAddr sockAddrTemp;
			if (!apSckServer->accept(sckDataConnection, sockAddrTemp))
				return false;

			return true;
		}

		/// Opens a passive data connection.
		/// @param[out] sckDataConnection
		/// @param[in] crDatachannelCmd Command to be executeted.
		/// @param[in] strPath Parameter for the command usually a path.
		/// @param[in] dwByteOffset Server marker at which file transfer is to be restarted.
		bool HFtpClient::openPassiveDataConnection(IFtpSocket& sckDataConnection, const HFtpCommand& crDatachannelCmd, const std::string& strPath, unsigned int dwByteOffset) const
		{
			if (!crDatachannelCmd.IsDatachannelCommand())
			{
				assert(false);
				return false;
			}

			unsigned int   ulRemoteHostIP = 0;
			unsigned short  ushServerSock = 0;

			// set passive mode
			// the FTP server opens a port and tell us the socket (ip address + port)
			// this socket is used for opening the data connection
			if (passiveCommand(ulRemoteHostIP, ushServerSock) != FTP_OK)
				return false;

			// establish connection
			HFtpSockAddr sockAddrTemp;
			sckDataConnection.create(SOCK_STREAM);
			HFtpSockAddr csaAddress(ulRemoteHostIP, ushServerSock);
			if (!sckDataConnection.connect(csaAddress))
			{
				sckDataConnection.cleanUp();
				return false;
			}

			// if resuming is activated then set offset
			if (_resumeEnabled &&
				(crDatachannelCmd == HFtpCommand::STOR() || crDatachannelCmd == HFtpCommand::RETR() || crDatachannelCmd == HFtpCommand::APPE()) &&
				(dwByteOffset != 0 && restart(dwByteOffset) != FTP_OK))
				return false;

			// send FTP command RETR/STOR/NLST/LIST to the server
			HFtpReply Reply;
			if (!sendCommand(crDatachannelCmd, strPath, Reply) ||
				!Reply.replyCode().isPositivePreliminaryReply())
				return false;

			return true;
		}

		/// Sends data over a socket to the server.
		/// @param[in] Observer Object for observing the execution of the command.
		/// @param[in] sckDataConnection Socket which is used for the send action.
		bool HFtpClient::sendData(IFtpTransferNotification& Observer, IFtpSocket& sckDataConnection) const
		{
			_transferInProgress = true;

			int iNumWrite = 0;
			size_t bytesRead = 0;

			Observer.onPreBytesSend(&_vBuffer.front(), _vBuffer.size(), bytesRead);

			while (!_abortTransfer && bytesRead != 0)
			{
				iNumWrite = sckDataConnection.write(&(*_vBuffer.begin()), static_cast<int>(bytesRead), _uiTimeout);

				if (iNumWrite != static_cast<int>(bytesRead))
				{
					_transferInProgress = false;
					sckDataConnection.cleanUp();
					return false;
				}

				Observer.onPreBytesSend(&_vBuffer.front(), _vBuffer.size(), bytesRead);
			}

			_transferInProgress = false;

			if (_abortTransfer)
			{
				abortCommand();
				return false;
			}

			return true;
		}

		/// Receives data over a socket from the server.
		/// @param[in] Observer Object for observing the execution of the command.
		/// @param[in] sckDataConnection Socket which is used for receiving the data.
		bool HFtpClient::receiveData(IFtpTransferNotification& Observer, IFtpSocket& sckDataConnection) const
		{
			_transferInProgress = true;

			int iNumRead = sckDataConnection.receive(&(*_vBuffer.begin()), static_cast<int>(_vBuffer.size()), _uiTimeout);
			long lTotalBytes = iNumRead;
			while (!_abortTransfer && iNumRead != 0)
			{
				Observer.onBytesReceived(_vBuffer, iNumRead);

				iNumRead = sckDataConnection.receive(&(*_vBuffer.begin()), static_cast<int>(_vBuffer.size()), _uiTimeout);

				if (iNumRead < 0) {
					_transferInProgress = false;
					sckDataConnection.cleanUp();
					return false;
				}

				lTotalBytes += iNumRead;
			}

			_transferInProgress = false;

			if (_abortTransfer)
			{
				abortCommand();
				return false;
			}

			return true;
		}

		/// Sends a command to the server.
		/// @param[in] Command Command to send.
		bool HFtpClient::sendCommand(const HFtpCommand& Command, const HFtpArg& Arguments) const
		{
			if (!connected())
				return false;
			
			const std::string strCommand = Command.AsString(Arguments) + "\r\n";

			if (_controlConnection->write(strCommand.c_str(), static_cast<int>(strCommand.length()), _uiTimeout) <= 0)
			{
				const_cast<HFtpClient*>(this)->_controlConnection->cleanUp();
				return false;
			}
			return true;
		}

		/// Sends a command to the server.
		/// @param[in]  Command Command to send.
		/// @param[out] Reply The Reply of the server to the sent command.
		bool HFtpClient::sendCommand(const HFtpCommand& Command, const HFtpArg& Arguments, HFtpReply& Reply) const
		{
			if (!sendCommand(Command, Arguments) || !getResponse(Reply))
				return false;
			return true;
		}

		/// This function gets the server response.
		/// A server response can exists of more than one line. This function
		/// returns the full response (multiline).
		/// @param[out] Reply Reply of the server to a command.
		bool HFtpClient::getResponse(HFtpReply& Reply) const
		{
			std::string strResponse;
			if (!getSingleResponseLine(strResponse))
				return false;

			if (strResponse.length() > 3 && strResponse.at(3) == ('-'))
			{
				std::string strSingleLine(strResponse);
				const int iRetCode = atol(strResponse.c_str());
				// handle multi-line server responses
				while (!(strSingleLine.length() > 3 &&
					strSingleLine.at(3) == (' ') &&
					atol(strSingleLine.c_str()) == iRetCode))
				{
					if (!getSingleResponseLine(strSingleLine))
						return false;
					strResponse += _eolCharacterSequence + strSingleLine;
				}
			}

			bool fRet = Reply.reponse(strResponse);

			return fRet;
		}

		/// Reads a single response line from the server control channel.
		/// @param[out] strResponse Response of the server as string.
		bool HFtpClient::getSingleResponseLine(std::string& strResponse) const
		{
			if (!connected())
				return false;

			if (_responseBuffer.empty())
			{
				// internal buffer is empty ==> get response from FTP server
				int iNum = 0;
				std::string strTemp;

				do
				{
					iNum = _controlConnection->receive(&(*_vBuffer.begin()), static_cast<int>(_vBuffer.size()) - 1, _uiTimeout);

					if (iNum <= 0)
					{
						const_cast<HFtpClient*>(this)->_controlConnection->cleanUp();
						return false;
					}

					if (_uiResponseWait != 0)
						Whale::sleep(_uiResponseWait);
					_vBuffer[iNum] = '\0';
					strTemp += &(*_vBuffer.begin());
				} while (iNum == static_cast<int>(_vBuffer.size()) - 1 && _controlConnection->readability());

				// each line in response is a separate entry in the internal buffer
				while (strTemp.length())
				{
					size_t iCRLF = strTemp.find('\n');
					if (iCRLF != std::string::npos)
					{
						_responseBuffer.push(strTemp.substr(0, iCRLF + 1));
						strTemp.erase(0, iCRLF + 1);
					}
					else
					{
						// this is not rfc standard; normally each command must end with CRLF
						// in this case it doesn't
						_responseBuffer.push(strTemp);
						strTemp.clear();
					}
				}

				if (_responseBuffer.empty())
					return false;
			}

			// get first response-line from buffer
			strResponse = _responseBuffer.front();
			_responseBuffer.pop();

			// remove CrLf if exists (don't use mc_strEolCharacterSequence here)
			if (strResponse.length() > 1 && strResponse.substr(strResponse.length() - 2) == ("\r\n"))
				strResponse.erase(strResponse.length() - 2, 2);

			return true;
		}

		/// Executes the FTP command CDUP (change to parent directory).
		/// This command is a special case of CFTPClient::ChangeWorkingDirectory
		/// (CWD), and is  included to simplify the implementation of programs for
		/// transferring directory trees between operating systems having different
		/// syntaxes for naming the parent directory.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::toParentDirectoryCommand() const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::CDUP(), HFtpArg(), Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command QUIT.
		/// This command terminates a USER and if file transfer is not in progress,
		/// the server closes the control connection. If file transfer is in progress,
		/// the connection will remain open for result response and the server will
		/// then close it.
		/// If the user-process is transferring files for several USERs but does not
		/// wish to close and then reopen connections for each, then the REIN command
		/// should be used instead of QUIT.
		/// An unexpected close on the control connection will cause the server to take
		/// the effective action of an abort (ABOR) and a logout.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::logout()
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::QUIT(), HFtpArg(), Reply))
				return FTP_ERROR;

			closeControlChannel();

			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command PASV. Set the passive mode.
		/// This command requests the server-DTP (data transfer process) on a data to
		/// "listen"  port (which is not its default data port) and to wait for a
		/// connection rather than initiate one upon receipt of a transfer command.
		/// The response to this command includes the host and port address this
		/// server is listening on.
		/// @param[out] ulIpAddress IP address the server is listening on.
		/// @param[out] ushPort Port the server is listening on.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::passiveCommand(unsigned int& ulIpAddress, unsigned short& ushPort) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::PASV(), HFtpArg(), Reply))
				return FTP_ERROR;

			if (Reply.replyCode().isPositiveCompletionReply())
			{
				if (!getIpAddressFromResponse(Reply.reponse(), ulIpAddress, ushPort))
					return FTP_ERROR;
			}

			return simpleErrorCheck(Reply);
		}

		/// Parses a response string and extracts the ip address and port information.
		/// @param[in]  strResponse The response string of a FTP server which holds
		///                         the ip address and port information.
		/// @param[out] ulIpAddress Buffer for the ip address.
		/// @param[out] ushPort     Buffer for the port information.
		/// @retval true  Everything went ok.
		/// @retval false An error occurred (invalid format).
		bool HFtpClient::getIpAddressFromResponse(const std::string& strResponse, unsigned int& ulIpAddress, unsigned short& ushPort) const
		{
			// parsing of ip-address and port implemented with a finite state machine
			// ...(192,168,1,1,3,44)...
			enum T_enState { state0, state1, state2, state3, state4 } enState = state0;

			std::string strIpAddress, strPort;
			unsigned short ushTempPort = 0;
			unsigned int  ulTempIpAddress = 0;
			int iCommaCnt = 4;
			for (std::string::const_iterator it = strResponse.begin(); it != strResponse.end(); ++it)
			{
				switch (enState)
				{
				case state0:
					if (*it == ('('))
						enState = state1;
					break;
				case state1:
					if (*it == (','))
					{
						if (--iCommaCnt == 0)
						{
							enState = state2;
							ulTempIpAddress += atol(strIpAddress.c_str());
						}
						else
						{
							ulTempIpAddress += atol(strIpAddress.c_str()) << 8 * iCommaCnt;
							strIpAddress.clear();
						}
					}
					else
					{
						if (!isdigit(*it))
							return false;
						strIpAddress += *it;
					}
					break;
				case state2:
					if (*it == (','))
					{
						ushTempPort = static_cast<unsigned short>(atol(strPort.c_str()) << 8);
						strPort.clear();
						enState = state3;
					}
					else
					{
						if (!isdigit(*it))
							return false;
						strPort += *it;
					}
					break;
				case state3:
					if (*it == (')'))
					{
						// compiler warning if using +=operator
						ushTempPort = ushTempPort + static_cast<unsigned short>(atol(strPort.c_str()));
						enState = state4;
					}
					else
					{
						if (!isdigit(*it))
							return false;
						strPort += *it;
					}
					break;
				case state4:
					break; // some compilers complain if not all enumeration values are listet
				}
			}

			if (enState == state4)
			{
				ulIpAddress = ulTempIpAddress;
				ushPort = ushTempPort;
			}

			return enState == state4;
		}

		/// Executes the FTP command ABOR.
		/// This command tells the server to abort the previous FTP service command
		/// and any associated transfer of data.  The abort command may require
		/// "special action", as discussed in the Section on FTP Commands, to force
		/// recognition by the server. No action is to be taken if the previous
		/// command has been completed (including data transfer). The control
		/// connection is not to be closed by the server, but the data connection
		/// must be closed.
		/// There are two cases for the server upon receipt of this command:<BR>
		/// (1) the FTP service command was already completed, or <BR>
		/// (2) the FTP service command is still in progress.<BR>
		/// In the first case, the server closes the data connection (if it is open)
		/// and responds with a 226 reply, indicating that the abort command was
		/// successfully processed.
		/// In the second case, the server aborts the FTP service in progress and
		/// closes the data connection, returning a 426 reply to indicate that the
		/// service request terminated abnormally. The server then sends a 226 reply,
		/// indicating that the abort command was successfully processed.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::abortCommand() const
		{
			if (_transferInProgress)
			{
				_abortTransfer = true;
				return FTP_OK;
			}

			_abortTransfer = false;
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::ABOR(), HFtpArg(), Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command PWD (PRINT WORKING DIRECTORY)
		/// This command causes the name of the current working directory
		/// to be returned in the reply.
		int HFtpClient::workingDirectoryCommand() const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::PWD(), HFtpArg(), Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command SYST (SYSTEM)
		/// This command is used to find out the type of operating system at the server.
		/// The reply shall have as its first word one of the system names listed in the
		/// current version of the Assigned Numbers document [Reynolds, Joyce, and
		/// Jon Postel, "Assigned Numbers", RFC 943, ISI, April 1985.].
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::systemCommand() const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::SYST(), HFtpArg(), Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command NOOP
		/// This command does not affect any parameters or previously entered commands.
		/// It specifies no action other than that the server send an FTP_OK reply.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::noopCommand() const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::NOOP(), HFtpArg(), Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command PORT (DATA PORT)
		/// The argument is a HOST-PORT specification for the data port to be used in data
		/// connection. There are defaults for both the user and server data ports, and
		/// under normal circumstances this command and its reply are not needed.  If
		/// this command is used, the argument is the concatenation of a 32-bit internet
		/// host address and a 16-bit TCP port address.
		/// @param[in] strHostIP IP-address like xxx.xxx.xxx.xxx
		/// @param[in] uiPort 16-bit TCP port address.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::portCommand(const std::string& strHostIP, unsigned short ushPort) const
		{
			std::string strPortArguments;
			// convert the port number to 2 bytes + add to the local IP
			strPortArguments = HFtpMakeString() << strHostIP << (",") << (ushPort >> 8) << (",") << (ushPort & 0xFF);
			strPortArguments = Whale::Util::HString::replace(strPortArguments, ".", ",");


			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::PORT(), strPortArguments, Reply)) {
				return FTP_ERROR;
			}
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command TYPE (REPRESENTATION TYPE)
		/// Caches the representation state if successful.
		/// see Documentation of CRepresentation
		/// @param[in] representation see Documentation of CRepresentation
		/// @param[in] iSize Indicates Bytesize for type LocalByte.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::representationType(const HFtpRepresentation& representation, unsigned int dwSize) const
		{
			// check representation
			if (_rurrentRepresentation.get() != NULL && representation == *_rurrentRepresentation)
				return FTP_OK;

			const int iRet = representationT(representation, dwSize);

			if (iRet == FTP_OK)
			{
				if (_rurrentRepresentation.get() == NULL)
					_rurrentRepresentation.reset(new HFtpRepresentation(representation));
				else
					*_rurrentRepresentation = representation;
			}
			else
				_rurrentRepresentation.reset();

			return iRet;
		}

		/// Executes the FTP command TYPE (REPRESENTATION TYPE)
		/// see Documentation of CRepresentation
		/// @param[in] representation see Documentation of CRepresentation
		/// @param[in] dwSize Indicates Bytesize for type LocalByte.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::representationT(const HFtpRepresentation& representation, unsigned int dwSize) const
		{
			HFtpArg Arguments(representation.Type().dataTypeEnumString());

			switch (representation.Type().dataTypeEnum())
			{
			case HFtpDataType::FTP_DTYPE_LOCAL_BYTE:
				Arguments.push_back(HFtpMakeString() << dwSize);
				break;
			case HFtpDataType::FTP_DTYPE_ASCII:
			case HFtpDataType::FTP_DTYPE_EBCDIC:
			case HFtpDataType::FTP_DTYPE_IMAGE:
				if (representation.Format().isValid())
					Arguments.push_back(representation.Format().ftpTypeFormatEnumString());
			}

			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::TYPE(), Arguments, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command CWD (CHANGE WORKING DIRECTORY)
		/// This command allows the user to work with a different directory or dataset
		/// for file storage or retrieval without altering his login or accounting
		/// information. Transfer parameters are similarly unchanged.
		/// @param[in] strDirectory Pathname specifying a directory or other system
		///                         dependent file group designator.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::changeWorkingDirectoryCommand(const std::string& strDirectory) const
		{
			assert(!strDirectory.empty());
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::CWD(), strDirectory, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command MKD (MAKE DIRECTORY)
		/// This command causes the directory specified in the pathname to be created
		/// as a directory (if the pathname is absolute) or as a subdirectory of the
		/// current working directory (if the pathname is relative).
		/// @pararm[in] strDirectory Pathname specifying a directory to be created.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::makeDirectoryCommand(const std::string& strDirectory) const
		{
			assert(!strDirectory.empty());
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::MKD(), strDirectory, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command SITE (SITE PARAMETERS)
		/// This command is used by the server to provide services specific to his
		/// system that are essential to file transfer but not sufficiently universal
		/// to be included as commands in the protocol.  The nature of these services
		/// and the specification of their syntax can be stated in a reply to the HELP
		/// SITE command.
		/// @param[in] strCmd Command to be executed.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::siteParameters(const std::string& strCmd) const
		{
			assert(!strCmd.empty());
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::SITE(), strCmd, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command HELP
		/// This command shall cause the server to send helpful information regarding
		/// its implementation status over the control connection to the user.
		/// The command may take an argument (e.g., any command name) and return more
		/// specific information as a response.  The reply is type 211 or 214.
		/// It is suggested that HELP be allowed before entering a USER command. The
		/// server may use this reply to specify site-dependent parameters, e.g., in
		/// response to HELP SITE.
		/// @param[in] strTopic Topic of the requested help.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::help(const std::string& strTopic) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::HELP(), strTopic, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command DELE (DELETE)
		/// This command causes the file specified in the pathname to be deleted at the
		/// server site.  If an extra level of protection is desired (such as the query,
		/// "Do you really wish to delete?"), it should be provided by the user-FTP process.
		/// @param[in] strFile Pathname of the file to delete.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::deleteFile(const std::string& strFile) const
		{
			assert(!strFile.empty());
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::DELE(), strFile, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command RMD (REMOVE DIRECTORY)
		/// This command causes the directory specified in the pathname to be removed
		/// as a directory (if the pathname is absolute) or as a subdirectory of the
		/// current working directory (if the pathname is relative).
		/// @param[in] strDirectory Pathname of the directory to delete.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::removeDirectoryCommand(const std::string& strDirectory) const
		{
			assert(!strDirectory.empty());
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::RMD(), strDirectory, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command STRU (FILE STRUCTURE)
		/// see documentation of CStructure
		/// The default structure is File.
		/// @param[in] crStructure see Documentation of CStructure
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::fileStructure(const HFtpDataStructure& crStructure) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::STRU(), crStructure.fileStructureEnumString(), Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command MODE (TRANSFER MODE)
		/// see documentation of CTransferMode
		/// The default transfer mode is Stream.
		/// @param[in] crTransferMode see Documentation of CTransferMode
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::transferMode(const HFtpTransferMode& crTransferMode) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::MODE(), crTransferMode.transferModeString(), Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command STAT (STATUS)
		/// This command shall cause a status response to be sent over the control
		/// connection in the form of a reply. The command may be sent during a file
		/// transfer (along with the Telnet IP and Synch signals--see the Section on
		/// FTP Commands) in which case the server will respond with the status of the
		/// operation in progress, or it may be sent between file transfers. In the
		/// latter case, the command may have an argument field.
		/// @param[in] strPath If the argument is a pathname, the command is analogous
		///                    to the "list" command except that data shall be transferred
		///                    over the control connection. If a partial pathname is
		///                    given, the server may respond with a list of file names or
		///                    attributes associated with that specification. If no argument
		///                    is given, the server should return general status information
		///                    about the server FTP process. This should include current
		///                    values of all transfer parameters and the status of connections.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::status(const std::string& strPath) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::STAT(), strPath, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command ALLO (ALLOCATE)
		/// This command may be required by some servers to reserve sufficient storage
		/// to accommodate the new file to be transferred.
		/// @param[in] iReserveBytes The argument shall be a decimal integer representing
		///                          the number of bytes (using the logical byte size) of
		///                          storage to be reserved for the file. For files sent
		///                          with record or page structure a maximum record or page
		///                          size (in logical bytes) might also be necessary; this
		///                          is indicated by a decimal integer in a second argument
		///                          field of the command.
		/// @pararm[in] piMaxPageOrRecordSize This second argument is optional. This command
		///                          shall be followed by a STORe or APPEnd command.
		///                          The ALLO command should be treated as a NOOP (no operation)
		///                          by those servers which do not require that the maximum
		///                          size of the file be declared beforehand, and those servers
		///                          interested in only the maximum record or page size should
		///                          accept a dummy value in the first argument and ignore it.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::allocate(int iReserveBytes, const int* piMaxPageOrRecordSize/*=NULL*/) const
		{
			HFtpArg Arguments(HFtpMakeString() << iReserveBytes);
			if (piMaxPageOrRecordSize != NULL)
			{
				Arguments.push_back(("R"));
				Arguments.push_back(HFtpMakeString() << *piMaxPageOrRecordSize);
			}

			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::ALLO(), Arguments, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command SMNT ()
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::structureMount(const std::string& strPath) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::SMNT(), strPath, Reply))
				return FTP_ERROR;
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command (STRUCTURE MOUNT)
		/// This command allows the user to mount a different file system data structure
		/// without altering his login or accounting information. Transfer parameters
		/// are similarly unchanged.  The argument is a pathname specifying a directory
		/// or other system dependent file group designator.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::reinitialize() const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::REIN(), HFtpArg(), Reply))
				return FTP_ERROR;

			if (Reply.replyCode().isPositiveCompletionReply())
				return FTP_OK;
			else if (Reply.replyCode().isPositivePreliminaryReply())
			{
				if (!getResponse(Reply) || !Reply.replyCode().isPositiveCompletionReply())
					return FTP_ERROR;
			}
			else if (Reply.replyCode().isNegativeReply())
				return FTP_NOTOK;

			assert(Reply.replyCode().isPositiveIntermediateReply());
			return FTP_ERROR;
		}

		/// Executes the FTP command REST (RESTART)
		/// This command does not cause file transfer but skips over the file to the
		/// specified data checkpoint. This command shall be immediately followed
		/// by the appropriate FTP service command which shall cause file transfer
		/// to resume.
		/// @param[in] dwPosition Represents the server marker at which file transfer
		///                       is to be restarted.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::restart(unsigned int dwPosition) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::REST(), HFtpArg(HFtpMakeString() << dwPosition), Reply))
				return FTP_ERROR;

			if (Reply.replyCode().isPositiveIntermediateReply())
				return FTP_OK;
			else if (Reply.replyCode().isNegativeReply())
				return FTP_NOTOK;

			assert(Reply.replyCode().isPositiveReply());

			return FTP_ERROR;
		}

		/// Executes the FTP command SIZE
		/// Return size of file.
		/// SIZE is not specified in RFC 959.
		/// @param[in] Pathname of a file.
		/// @param[out] Size of the file specified in pathname.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::fileSize(const std::string& strPath, long& lSize) const
		{
			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::SIZE(), strPath, Reply))
				return FTP_ERROR;
			lSize = atol(Reply.reponse().substr(4).c_str());
			return simpleErrorCheck(Reply);
		}

		/// Executes the FTP command MDTM
		/// Show last modification time of file.
		/// MDTM is not specified in RFC 959.
		/// @param[in] strPath Pathname of a file.
		/// @param[out] strModificationTime Modification time of the file specified in pathname.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::fileModificationTime(const std::string& strPath, std::string& strModificationTime) const
		{
			strModificationTime.erase();

			HFtpReply Reply;
			if (!sendCommand(HFtpCommand::MDTM(), strPath, Reply))
				return FTP_ERROR;

			if (Reply.reponse().length() >= 18)
			{
				std::string strTemp(Reply.reponse().substr(4));
				size_t iPos = strTemp.find(('.'));
				if (iPos != std::string::npos)
					strTemp = strTemp.substr(0, iPos);
				if (strTemp.length() == 14)
					strModificationTime = strTemp;
			}

			if (strModificationTime.empty())
				return FTP_ERROR;

			return simpleErrorCheck(Reply);
		}

		/// Show last modification time of file.
		/// @param[in] strPath Pathname of a file.
		/// @param[out] tmModificationTime Modification time of the file specified in pathname.
		/// @return see return values of CFTPClient::SimpleErrorCheck
		int HFtpClient::fileModificationTime(const std::string& strPath, tm& tmModificationTime) const
		{
			std::string strTemp;
			const int iRet = fileModificationTime(strPath, strTemp);

			memset(&tmModificationTime, 0, sizeof(tmModificationTime));
			if (iRet == FTP_OK)
			{
				tmModificationTime.tm_year = atol(strTemp.substr(0, 4).c_str());
				tmModificationTime.tm_mon = atol(strTemp.substr(4, 2).c_str());
				tmModificationTime.tm_mday = atol(strTemp.substr(6, 2).c_str());
				tmModificationTime.tm_hour = atol(strTemp.substr(8, 2).c_str());
				tmModificationTime.tm_min = atol(strTemp.substr(10, 2).c_str());
				tmModificationTime.tm_sec = atol(strTemp.substr(12).c_str());
			}
			return iRet;
		}
	}
}
