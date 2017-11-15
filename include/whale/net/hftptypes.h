#ifndef __HFTPTYPES_H
#define __HFTPTYPES_H

#include "whale/basic/hplatform.h"

const char FTP_ANONYMOUS_USER[] = ("anonymous");

enum T_enConstants {
    ///< The default port that an FTP service listens to on a remote host
    DEFAULT_FTP_PORT = 21,
    FTP_ERROR = -1,
    FTP_OK = 0,
    FTP_NOTOK = 1,
};

namespace Whale
{
	namespace Net
	{
		class IFtpDataType
		{
		protected:
			IFtpDataType() {
			}
			virtual ~IFtpDataType() {
			}

		private:
			IFtpDataType(const IFtpDataType&);
			const IFtpDataType& operator=(const IFtpDataType&);
		};

		class HFtpDataStructure
		{
		public:
			enum FtpFileStructureEnum { FTP_FILE, FTP_RECORD, FTP_PAGE };

			HFtpDataStructure(const HFtpDataStructure& structure) :
				_enStructure(structure.fileStructureEnum()) {}

			bool operator==(const FtpFileStructureEnum& rhs) const {
				return _enStructure == rhs;
			}

			bool operator!=(const FtpFileStructureEnum& rhs) const {
				return !operator==(rhs);
			}

			bool operator==(const HFtpDataStructure& rhs) const {
				return _enStructure == rhs._enStructure;
			}

			bool operator!=(const HFtpDataStructure& rhs) const {
				return !operator==(rhs);
			}

			HFtpDataStructure& operator=(const HFtpDataStructure& rhs) {
				_enStructure = rhs.fileStructureEnum(); return *this;
			}

			FtpFileStructureEnum fileStructureEnum() const {
				return _enStructure;
			}

			std::string fileStructureEnumString() const;

			static const HFtpDataStructure ftpFile()   {
				return FTP_FILE;
			}

			static const HFtpDataStructure ftpRecord() {
				return FTP_RECORD;
			}

			static const HFtpDataStructure ftpPage()   {
				return FTP_PAGE;
			}

		private:
			HFtpDataStructure(FtpFileStructureEnum enStructure) : _enStructure(enStructure) {}
			FtpFileStructureEnum _enStructure;
		};

		class HFtpTransferMode
		{
		public:
			enum HFtpTransferModeEnum { FTP_MODE_STREAM, FTP_MODE_BLOCK, FTP_MODE_COMPRESSED };

			HFtpTransferMode(const HFtpTransferMode& transferMode) :
				_transferMode(transferMode.transferModeEnum()) {}

			bool operator==(const HFtpTransferModeEnum& rhs) const {
				return _transferMode == rhs;
			}
			bool operator!=(const HFtpTransferModeEnum& rhs) const {
				return !operator==(rhs);
			}
			bool operator==(const HFtpTransferMode& rhs) const {
				return _transferMode == rhs._transferMode;
			}
			bool operator!=(const HFtpTransferMode& rhs) const {
				return !operator==(rhs);
			}

			HFtpTransferMode& operator=(const HFtpTransferMode& rhs) {
				_transferMode = rhs.transferModeEnum(); return *this;
			}

			HFtpTransferModeEnum transferModeEnum() const {
				return _transferMode;
			}

			std::string transferModeString() const;

			static const HFtpTransferMode transferModeStream()     { return FTP_MODE_STREAM; }
			static const HFtpTransferMode transferModeBlock()      { return FTP_MODE_BLOCK; }
			static const HFtpTransferMode transferModeCompressed() { return FTP_MODE_COMPRESSED; }

		private:
			HFtpTransferMode(HFtpTransferModeEnum enTransferMode) : _transferMode(enTransferMode) {}
			HFtpTransferModeEnum _transferMode;
		};

		class HFtpFirewallType
		{
		public:
			// don't change order of enumeration
			enum FtpFirewallTypeEnum {
				FTP_FTYPE_NONE, FTP_FTYPE_SITE_HOSTNAME, FTP_FTYPE_USER_AFTER_LONGON,
				FTP_FTYPE_PROXY_OPEN, FTP_FTYPE_TRANSPARENT, FTP_FTYPE_USER_WITH_NOLONGON,
				FTP_FTYPE_USER_FORE_IDATREMOTEHOST, FTP_FTYPE_USER_REMOTE_IDATREMOTEHOST_FIREID,
				FTP_FTYPE_USER_REMOTEIDATFIRE_IDAT_REMOTEHOST
			};

			HFtpFirewallType() : _firewallType(FTP_FTYPE_NONE) {
			}

			HFtpFirewallType(const HFtpFirewallType& firewallType) :
				_firewallType(firewallType.ftpFirewallType()) {}

			bool operator==(const FtpFirewallTypeEnum& rhs) const {
				return _firewallType == rhs;
			}

			bool operator!=(const FtpFirewallTypeEnum& rhs) const {
				return !operator==(rhs);
			}

			bool operator==(const HFtpFirewallType& rhs) const {
				return _firewallType == rhs._firewallType;
			}

			bool operator!=(const HFtpFirewallType& rhs) const {
				return !operator==(rhs);
			}

			HFtpFirewallType& operator=(const HFtpFirewallType& rhs) {
				_firewallType = rhs.ftpFirewallType(); return *this;
			}

			FtpFirewallTypeEnum ftpFirewallType() const {
				return _firewallType;
			}

			std::string asDisplayString() const;
			std::string asStorageString() const;
			static void getAllTypes(std::vector<HFtpFirewallType>& vTypes);

			static const HFtpFirewallType none()                             { return FTP_FTYPE_NONE; }
			static const HFtpFirewallType siteHostName()                     { return FTP_FTYPE_SITE_HOSTNAME; }
			static const HFtpFirewallType userAfterLogon()                   { return FTP_FTYPE_USER_AFTER_LONGON; }
			static const HFtpFirewallType proxyOpen()                        { return FTP_FTYPE_PROXY_OPEN; }
			static const HFtpFirewallType transparent()                      { return FTP_FTYPE_TRANSPARENT; }
			static const HFtpFirewallType userWithNoLogon()                  { return FTP_FTYPE_USER_WITH_NOLONGON; }
			static const HFtpFirewallType userFireIDatRemotehost()           { return FTP_FTYPE_USER_FORE_IDATREMOTEHOST; }
			static const HFtpFirewallType userRemoteIDatRemoteHostFireID()   { return FTP_FTYPE_USER_REMOTE_IDATREMOTEHOST_FIREID; }
			static const HFtpFirewallType userRemoteIDatFireIDatRemoteHost() { return FTP_FTYPE_USER_REMOTEIDATFIRE_IDAT_REMOTEHOST; }

		private:
			HFtpFirewallType(FtpFirewallTypeEnum enFirewallType) : _firewallType(enFirewallType) {}
			FtpFirewallTypeEnum _firewallType;
		};

		class HFtpDataType
		{
		public:
			enum FtpDataTypeEnum { FTP_DTYPE_ASCII, FTP_DTYPE_EBCDIC, FTP_DTYPE_IMAGE, FTP_DTYPE_LOCAL_BYTE };

			HFtpDataType(const HFtpDataType& type) :
				_dataType(type.dataTypeEnum()) {}

			bool operator==(const FtpDataTypeEnum& rhs) const {
				return _dataType == rhs;
			}

			bool operator!=(const FtpDataTypeEnum& rhs) const {
				return !operator==(rhs);
			}

			bool operator==(const HFtpDataType& rhs) const {
				return _dataType == rhs._dataType;
			}

			bool operator!=(const HFtpDataType& rhs) const {
				return !operator==(rhs);
			}

			HFtpDataType& operator=(const HFtpDataType& rhs) {
				_dataType = rhs.dataTypeEnum(); return *this;
			}

			FtpDataTypeEnum dataTypeEnum() const {
				return _dataType;
			}

			std::string dataTypeEnumString() const;

			static const HFtpDataType ascii()     { return FTP_DTYPE_ASCII; }
			static const HFtpDataType ebcdic()    { return FTP_DTYPE_EBCDIC; }
			static const HFtpDataType image()     { return FTP_DTYPE_IMAGE; }
			static const HFtpDataType localByte() { return FTP_DTYPE_LOCAL_BYTE; }

		private:
			HFtpDataType(FtpDataTypeEnum enType) : _dataType(enType) {}
			FtpDataTypeEnum _dataType;
		};


		class HFtpTypeFormat
		{
		public:
			enum FtpTypeFormatEnum { FTP_TF_INVALID, FTP_TF_NONE_PRINT, FTP_TF_TELNET_FORMAT, FTP_TF_CARRIAGE_CONTROL };

			HFtpTypeFormat() : _dataTypeFormat(FTP_TF_INVALID) {}
			HFtpTypeFormat(const HFtpTypeFormat& typeFormat) :
				_dataTypeFormat(typeFormat.ftpTypeFormatEnum()) {}

			bool operator==(const FtpTypeFormatEnum& rhs) const {
				return _dataTypeFormat == rhs;
			}
			bool operator!=(const FtpTypeFormatEnum& rhs) const {
				return !operator==(rhs);
			}
			bool operator==(const HFtpTypeFormat& rhs) const {
				return _dataTypeFormat == rhs._dataTypeFormat;
			}
			bool operator!=(const HFtpTypeFormat& rhs) const {
				return !operator==(rhs);
			}

			HFtpTypeFormat& operator=(const HFtpTypeFormat& rhs) {
				_dataTypeFormat = rhs.ftpTypeFormatEnum(); return *this;
			}

			FtpTypeFormatEnum ftpTypeFormatEnum() const { return _dataTypeFormat; }
			std::string ftpTypeFormatEnumString() const;
			bool isValid() const { return _dataTypeFormat != FTP_TF_INVALID; }

			static const HFtpTypeFormat nonPrint()        { return FTP_TF_NONE_PRINT; }
			static const HFtpTypeFormat telnetFormat()    { return FTP_TF_TELNET_FORMAT; }
			static const HFtpTypeFormat carriageControl() { return FTP_TF_CARRIAGE_CONTROL; }

		private:
			HFtpTypeFormat(FtpTypeFormatEnum enTypeFormat) : _dataTypeFormat(enTypeFormat) {}
			FtpTypeFormatEnum _dataTypeFormat;
		};

		class HFtpRepresentation
		{
		public:
			HFtpRepresentation(HFtpDataType Type) : _types(Type) {}
			HFtpRepresentation(HFtpDataType Type, HFtpTypeFormat Format) : _types(Type), _format(Format) {}

			bool operator==(const HFtpRepresentation& rhs) const { return rhs._types == _types && rhs._format == _format; }
			bool operator!=(const HFtpRepresentation& rhs) const { return !operator==(rhs); }
			HFtpRepresentation& operator=(const HFtpRepresentation& rhs)
			{
				_types = rhs._types;
				_format = rhs._format;
				return *this;
			}

			const HFtpDataType&       Type()   const { return _types; }
			const HFtpTypeFormat& Format() const { return _format; }

		private:
			HFtpDataType   _types;
			HFtpTypeFormat _format;
		};

		class HFtpArg : public std::vector<std::string>
		{
		public:
			HFtpArg() {
			}

			HFtpArg(const std::string& strArgument) {
				push_back(strArgument);
			}

			HFtpArg(const std::string& strFirstArgument, const std::string& strSecondArgument) {
				push_back(strFirstArgument); push_back(strSecondArgument);
			}

			HFtpArg(const std::string& strFirstArgument, const std::string& strSecondArgument, const std::string& strThirdArgument) {
				push_back(strFirstArgument); push_back(strSecondArgument); push_back(strThirdArgument);
			}
		};

		class HFtpCommand
		{
		public:
			enum TCommandEnum { cmdABOR, cmdACCT, cmdALLO, cmdAPPE, cmdCDUP, cmdCWD, cmdDELE, cmdHELP, cmdLIST, cmdMDTM, cmdMKD, cmdMODE, cmdNLST, cmdNOOP, cmdOPEN, cmdPASS, cmdPASV, cmdPORT, cmdPWD, cmdQUIT, cmdREIN, cmdREST, cmdRETR, cmdRMD, cmdRNFR, cmdRNTO, cmdSITE, cmdSIZE, cmdSMNT, cmdSTAT, cmdSTOR, cmdSTOU, cmdSTRU, cmdSYST, cmdTYPE, cmdUSER, };
			enum TSpecificationEnum { Unknown, RFC959, RFC3659, };
			enum TTypeEnum { DatachannelRead, DatachannelWrite, NonDatachannel, };

			class IFtpExtendedInfo : public IFtpDataType
			{
			public:
				virtual ~IFtpExtendedInfo() {}
				virtual const std::string& GetServerString() const = 0;
				virtual const std::string& GetCompleteServerStringSyntax() const = 0;
				virtual unsigned int GetNumberOfParameters() const = 0;
				virtual unsigned int GetNumberOfOptionalParameters() const = 0;
				virtual TSpecificationEnum GetSpecification() const = 0;
				virtual TTypeEnum GetType() const = 0;
			};

			HFtpCommand(const HFtpCommand& datachannelCmd) :
				m_enCommand(datachannelCmd.AsEnum()) {}

			bool operator==(TCommandEnum rhs) const { return m_enCommand == rhs; }
			bool operator!=(TCommandEnum rhs) const { return !operator==(rhs); }

			bool operator==(const HFtpCommand& rhs) const { return m_enCommand == rhs.m_enCommand; }
			bool operator!=(const HFtpCommand& rhs) const { return !operator==(rhs); }

			HFtpCommand& operator=(const HFtpCommand& rhs)
			{
				m_enCommand = rhs.AsEnum();
				return *this;
			}

			TCommandEnum AsEnum() const { return m_enCommand; }
			std::string AsString() const;
			std::string AsString(const HFtpArg& Arguments) const;
			const IFtpExtendedInfo& GetExtendedInfo() const;

			bool IsDatachannelReadCommand() const;
			bool IsDatachannelWriteCommand() const;
			bool IsDatachannelCommand() const;

			static const HFtpCommand ABOR() { return cmdABOR; }
			static const HFtpCommand ACCT() { return cmdACCT; }
			static const HFtpCommand ALLO() { return cmdALLO; }
			static const HFtpCommand APPE() { return cmdAPPE; }
			static const HFtpCommand CDUP() { return cmdCDUP; }
			static const HFtpCommand CWD()  { return cmdCWD; }
			static const HFtpCommand DELE() { return cmdDELE; }
			static const HFtpCommand HELP() { return cmdHELP; }
			static const HFtpCommand LIST() { return cmdLIST; }
			static const HFtpCommand MDTM() { return cmdMDTM; }
			static const HFtpCommand MKD()  { return cmdMKD; }
			static const HFtpCommand MODE() { return cmdMODE; }
			static const HFtpCommand NLST() { return cmdNLST; }
			static const HFtpCommand NOOP() { return cmdNOOP; }
			static const HFtpCommand OPEN() { return cmdOPEN; }
			static const HFtpCommand PASS() { return cmdPASS; }
			static const HFtpCommand PASV() { return cmdPASV; }
			static const HFtpCommand PORT() { return cmdPORT; }
			static const HFtpCommand PWD()  { return cmdPWD; }
			static const HFtpCommand QUIT() { return cmdQUIT; }
			static const HFtpCommand REIN() { return cmdREIN; }
			static const HFtpCommand REST() { return cmdREST; }
			static const HFtpCommand RETR() { return cmdRETR; }
			static const HFtpCommand RMD()  { return cmdRMD; }
			static const HFtpCommand RNFR() { return cmdRNFR; }
			static const HFtpCommand RNTO() { return cmdRNTO; }
			static const HFtpCommand SITE() { return cmdSITE; }
			static const HFtpCommand SIZE() { return cmdSIZE; }
			static const HFtpCommand SMNT() { return cmdSMNT; }
			static const HFtpCommand STAT() { return cmdSTAT; }
			static const HFtpCommand STOR() { return cmdSTOR; }
			static const HFtpCommand STOU() { return cmdSTOU; }
			static const HFtpCommand STRU() { return cmdSTRU; }
			static const HFtpCommand SYST() { return cmdSYST; }
			static const HFtpCommand TYPE() { return cmdTYPE; }
			static const HFtpCommand USER() { return cmdUSER; }

		private:
			class CExtendedInfo;
			class CCmd2Info;
			HFtpCommand(TCommandEnum enDatachannelCmd) : m_enCommand(enDatachannelCmd) {}
			TCommandEnum m_enCommand;
		};

		class HFtpReply
		{
		private:
			std::string _response;

			class HFtpReplyCode
			{
				char _code[4];
			public:
				HFtpReplyCode()	{
					std::fill_n(_code, sizeof(_code) / sizeof(char), 0);
				}

				const char* code() const {
					return _code;
				}

				bool code(const std::string& strCode) {
					if (strCode.length() != 3 ||
						strCode[0]<('1') || strCode[0]>('5') ||
						strCode[1]<('0') || strCode[1]>('5')) {
						std::fill_n(_code, sizeof(_code) / sizeof(char), 0);
						return false;
					}
					std::copy(strCode.begin(), strCode.end(), _code);
					return true;
				}

				bool isPositiveReply() const {
					return isPositivePreliminaryReply() || isPositiveCompletionReply() || isPositiveIntermediateReply();
				}

				bool isNegativeReply() const {
					return isTransientNegativeCompletionReply() || isPermanentNegativeCompletionReply();
				}

				bool isPositivePreliminaryReply() const         { return _code[0] == ('1'); }
				bool isPositiveCompletionReply() const          { return _code[0] == ('2'); }
				bool isPositiveIntermediateReply() const        { return _code[0] == ('3'); }
				bool isTransientNegativeCompletionReply() const { return _code[0] == ('4'); }
				bool isPermanentNegativeCompletionReply() const { return _code[0] == ('5'); }

				bool isRefferingToSyntax() const                      { return _code[1] == ('0'); }
				bool isRefferingToInformation() const                 { return _code[1] == ('1'); }
				bool isRefferingToConnections() const                 { return _code[1] == ('2'); }
				bool isRefferingToAuthenticationAndAccounting() const { return _code[1] == ('3'); }
				bool isRefferingToUnspecified() const                 { return _code[1] == ('4'); }
				bool isRefferingToFileSystem() const                  { return _code[1] == ('5'); }
			} _replyCode;

		public:
			bool reponse(const std::string& strResponse)
			{
				_response = strResponse;
				if (_response.length() > 2)
					return _replyCode.code(_response.substr(0, 3));
				return false;
			}
			const std::string& reponse() const { return _response; }
			const HFtpReplyCode& replyCode() const { return _replyCode; }
		};


	}
}
#endif // __HFTPTYPES_H
