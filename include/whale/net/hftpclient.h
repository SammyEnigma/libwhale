#ifndef __HFTPCLIENT_H
#define __HFTPCLIENT_H

#include "whale/net/hftptypes.h"
#include "whale/net/hftpsocket.h"
#include "whale/net/hftpstatus.h"

namespace Whale
{
	namespace Net
	{
		class IFtpFileListParser;
		class IFtpTransferNotification;
		class HFtpRepresentation;

		class HFtpClient
		{
		public:
			HFtpClient(unsigned int uiTimeout = 10, unsigned int uiBufferSize = 2048,
				unsigned int uiResponseWait = 0, const std::string& strRemoteDirectorySeparator = ("/"));
			virtual ~HFtpClient();

			void fileListParser(std::shared_ptr<IFtpFileListParser> apFileListParser);

			bool connected() const;
			bool transferring() const;
			bool resumeModeEnabled() const;
			void resumeMode(bool fEnable = true);

			bool login(const std::string& host = "127.0.0.1",
				unsigned short port = DEFAULT_FTP_PORT,
				const std::string& user = FTP_ANONYMOUS_USER,
				const std::string& passwd = "anonymous@user.com",
				const std::string& account = "",
				const std::string& firewallHost = "127.0.0.1",
				unsigned short _firewallPort = DEFAULT_FTP_PORT,
				const std::string& firewallUser = FTP_ANONYMOUS_USER,
				const std::string& firewallPasswd = "anonymous@user.com",
				const HFtpFirewallType& firewallType = HFtpFirewallType::none());
			int  logout();

			bool fileList(const std::string& strPath, std::vector<std::string>& vstrFileList, bool fPasv = false) const;
			bool fileNameList(const std::string& strPath, std::vector<std::string>& vstrFileList, bool fPasv = false) const;

			bool fileList(const std::string& strPath, std::vector<std::shared_ptr<HFtpFileStatus>>& vFileList, bool fPasv = false) const;
			bool fileNameList(const std::string& strPath, std::vector<std::shared_ptr<HFtpFileStatus>>& vFileList, bool fPasv = false) const;

			int  deleteFile(const std::string& strFile) const;
			int  renameFile(const std::string& strOldName, const std::string& strNewName) const;
			int  moveFile(const std::string& strFullSourceFilePath, const std::string& strFullTargetFilePath) const;

			bool downloadFile(const std::string& strRemoteFile, IFtpTransferNotification& Observer,
				const HFtpRepresentation& repType = HFtpRepresentation(HFtpDataType::image()), bool fPasv = false) const;
			bool downloadFile(const std::string& strRemoteFile, const std::string& strLocalFile,
				const HFtpRepresentation& repType = HFtpRepresentation(HFtpDataType::image()), bool fPasv = false) const;
			bool downloadFile(const std::string& strSourceFile, const HFtpClient& TargetFtpServer,
				const std::string& strTargetFile, const HFtpRepresentation& repType = HFtpRepresentation(HFtpDataType::image()),
				bool fPasv = true) const;

			bool uploadFile(IFtpTransferNotification& Observer, const std::string& strRemoteFile, bool fStoreUnique = false,
				const HFtpRepresentation& repType = HFtpRepresentation(HFtpDataType::image()), bool fPasv = false) const;
			bool uploadFile(const std::string& strLocalFile, const std::string& strRemoteFile, bool fStoreUnique = false,
				const HFtpRepresentation& repType = HFtpRepresentation(HFtpDataType::image()), bool fPasv = false) const;
			bool uploadFile(const HFtpClient& SourceFtpServer, const std::string& strSourceFile,
				const std::string& strTargetFile, const HFtpRepresentation& repType = HFtpRepresentation(HFtpDataType::image()),
				bool fPasv = true) const;

			static bool transferFile(const HFtpClient& SourceFtpServer, const std::string& strSourceFile,
				const HFtpClient& TargetFtpServer, const std::string& strTargetFile,
				const HFtpRepresentation& repType = HFtpRepresentation(HFtpDataType::image()), bool fSourcePasv = false);

			int removeDirectoryCommand(const std::string& strDirectory) const;
			int makeDirectoryCommand(const std::string& strDirectory) const;
			int workingDirectoryCommand() const;
			int toParentDirectoryCommand() const;
			int changeWorkingDirectoryCommand(const std::string& strDirectory) const;
			int passiveCommand(unsigned int& ulIpAddress, unsigned short& ushPort) const;
			int portCommand(const std::string& strHostIP, unsigned short ushPort) const;
			int abortCommand() const;
			int systemCommand() const;
			int noopCommand() const;

			int representationType(const HFtpRepresentation& repType, unsigned int dwSize = 0) const;
			int fileStructure(const HFtpDataStructure& crStructure) const;
			int transferMode(const HFtpTransferMode& crTransferMode) const;
			int allocate(int iReserveBytes, const int* piMaxPageOrRecordSize = NULL) const;
			int structureMount(const std::string& strPath) const;
			int siteParameters(const std::string& strCmd) const;
			int status(const std::string& strPath) const;
			int help(const std::string& strTopic) const;

			int reinitialize() const;
			int restart(unsigned int dwPosition) const;

			int fileSize(const std::string& strPath, long& lSize) const;
			int fileModificationTime(const std::string& strPath, tm& tmModificationTime) const;
			int fileModificationTime(const std::string& strPath, std::string& strModificationTime) const;

		protected:
			bool executeDatachannelCommand(const HFtpCommand& crDatachannelCmd, const std::string& strPath, const HFtpRepresentation& representation,
				bool fPasv, unsigned int dwByteOffset, IFtpTransferNotification& Observer) const;

		private:
			HFtpClient& operator=(const HFtpClient&); // no implementation for assignment operator
			int representationT(const HFtpRepresentation& repType, unsigned int dwSize = 0) const;
			bool transferData(const HFtpCommand& crDatachannelCmd, IFtpTransferNotification& Observer, IFtpSocket& sckDataConnection) const;
			bool openActiveDataConnection(IFtpSocket& sckDataConnection, const HFtpCommand& crDatachannelCmd, const std::string& strPath, unsigned int dwByteOffset) const;
			bool openPassiveDataConnection(IFtpSocket& sckDataConnection, const HFtpCommand& crDatachannelCmd, const std::string& strPath, unsigned int dwByteOffset) const;
			bool sendData(IFtpTransferNotification& Observer, IFtpSocket& sckDataConnection) const;
			bool receiveData(IFtpTransferNotification& Observer, IFtpSocket& sckDataConnection) const;

			int  simpleErrorCheck(const HFtpReply& Reply) const;

			bool sendCommand(const HFtpCommand& Command, const HFtpArg& Arguments) const;
			bool sendCommand(const HFtpCommand& Command, const HFtpArg& Arguments, HFtpReply& Reply) const;
			bool getResponse(HFtpReply& Reply) const;
			bool getSingleResponseLine(std::string& strResponse) const;

			bool openControlChannel(const std::string& strServerHost, unsigned short ushServerPort = DEFAULT_FTP_PORT);
			void closeControlChannel();

			bool getIpAddressFromResponse(const std::string& strResponse, unsigned int& ulIpAddress, unsigned short& ushPort) const;

		private:
			const unsigned int _uiTimeout; ///< timeout for socket-functions
			const unsigned int _uiResponseWait; ///< sleep time between receive calls to socket when getting the response
			const std::string _eolCharacterSequence; ///< end-of-line sequence of current operating system
			const std::string _remoteDirectorySeparator; ///< directory separator character which is used on the FTP server

			mutable std::vector<char> _vBuffer; ///< buffer for sending and receiving
			mutable std::queue<std::string> _responseBuffer; ///< buffer for server-responses
			mutable std::shared_ptr<HFtpRepresentation> _rurrentRepresentation;  ///< representation currently set

			std::shared_ptr<IFtpSocket> _controlConnection; ///< socket for connection to FTP server
			std::shared_ptr<IFtpFileListParser> _fileListParser; ///< object which is used for parsing the result of the LIST command
			mutable bool _transferInProgress; ///< if true, a file transfer is in progress
			mutable bool _abortTransfer; ///< indicates that a running filetransfer should be canceled
			bool _resumeEnabled; ///< try to resume download/upload if possible

			// Login Information
			std::string _host; ///< name or ip-address of the ftp-server
			unsigned short _port; ///< port of the ftp-server
			std::string _user; ///< username for ftp-server
			std::string _passwd; ///< password for ftp-server
			std::string _account; ///< account mostly needed on ftp-servers running on unix/linux
			std::string _firewallHost; ///< name or ip-address of the firewall
			std::string _firewallUser; ///< username for firewall
			std::string _firewallPasswd; ///< password for firewall
			unsigned short _firewallPort; ///< port of the firewall
			HFtpFirewallType _firewallType; ///< type of firewall
		};
	}
}

#endif // __HFTPCLIENT_H

