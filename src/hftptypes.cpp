#include "whale/net/hftptypes.h"

namespace Whale
{
	namespace Net
	{

		std::string HFtpDataStructure::fileStructureEnumString() const
		{
			switch (_enStructure)
			{
			case FTP_FILE:   return ("F");
			case FTP_RECORD: return ("R");
			case FTP_PAGE:   return ("P");
			}
			assert(false);
			return ("");
		}

		std::string HFtpTransferMode::transferModeString() const
		{
			std::string strMode;
			switch (_transferMode)
			{
			case FTP_MODE_STREAM:      return ("S");
			case FTP_MODE_BLOCK:       return ("B");
			case FTP_MODE_COMPRESSED:  return ("C");
			}
			assert(false);
			return ("");
		}

		std::string HFtpDataType::dataTypeEnumString() const
		{
			switch (_dataType)
			{
			case FTP_DTYPE_ASCII:     return ("A");
			case FTP_DTYPE_EBCDIC:    return ("E");
			case FTP_DTYPE_IMAGE:     return ("I");
			case FTP_DTYPE_LOCAL_BYTE: return ("L");
			}
			assert(false);
			return ("");
		}

		std::string HFtpTypeFormat::ftpTypeFormatEnumString() const
		{
			switch (_dataTypeFormat)
			{
			case FTP_TF_NONE_PRINT:        return ("N");
			case FTP_TF_TELNET_FORMAT:    return ("T");
			case FTP_TF_CARRIAGE_CONTROL: return ("C");
			case FTP_TF_INVALID:         break;
			}
			assert(false);
			return ("");
		}

		/// returns the string which is used for display
		std::string HFtpFirewallType::asDisplayString() const
		{
			switch (_firewallType)
			{
			case FTP_FTYPE_NONE:                              return ("no firewall");
			case FTP_FTYPE_SITE_HOSTNAME:                      return ("SITE hostname");
			case FTP_FTYPE_USER_AFTER_LONGON:                    return ("USER after logon");
			case FTP_FTYPE_PROXY_OPEN:                         return ("proxy OPEN");
			case FTP_FTYPE_TRANSPARENT:                       return ("Transparent");
			case FTP_FTYPE_USER_WITH_NOLONGON:                   return ("USER with no logon");
			case FTP_FTYPE_USER_FORE_IDATREMOTEHOST:            return ("USER fireID@remotehost");
			case FTP_FTYPE_USER_REMOTE_IDATREMOTEHOST_FIREID:    return ("USER remoteID@remotehost fireID");
			case FTP_FTYPE_USER_REMOTEIDATFIRE_IDAT_REMOTEHOST:  return ("USER remoteID@fireID@remotehost");
			}
			assert(false);
			return ("");
		}

		/// return the string which is used for storage (e.g. in an XML- or INI-file)
		std::string HFtpFirewallType::asStorageString() const
		{
			switch (_firewallType)
			{
			case FTP_FTYPE_NONE:                              return ("NO_FIREWALL");
			case FTP_FTYPE_SITE_HOSTNAME:                      return ("SITE_HOSTNAME");
			case FTP_FTYPE_USER_AFTER_LONGON:                    return ("USER_AFTER_LOGON");
			case FTP_FTYPE_PROXY_OPEN:                         return ("PROXY_OPEN");
			case FTP_FTYPE_TRANSPARENT:                       return ("TRANSPARENT");
			case FTP_FTYPE_USER_WITH_NOLONGON:                   return ("USER_WITH_NO_LOGON");
			case FTP_FTYPE_USER_FORE_IDATREMOTEHOST:            return ("USER_FIREID@REMOTEHOST");
			case FTP_FTYPE_USER_REMOTE_IDATREMOTEHOST_FIREID:    return ("USER_REMOTEID@REMOTEHOST_FIREID");
			case FTP_FTYPE_USER_REMOTEIDATFIRE_IDAT_REMOTEHOST:  return ("USER_REMOTEID@FIREID@REMOTEHOST");
			}
			assert(false);
			return ("");
		}

		/// returns all available firewall types
		void HFtpFirewallType::getAllTypes(std::vector<HFtpFirewallType>& vTypes)
		{
			vTypes.resize(9);
			vTypes[0] = FTP_FTYPE_NONE;
			vTypes[1] = FTP_FTYPE_SITE_HOSTNAME;
			vTypes[2] = FTP_FTYPE_USER_AFTER_LONGON;
			vTypes[3] = FTP_FTYPE_PROXY_OPEN;
			vTypes[4] = FTP_FTYPE_TRANSPARENT;
			vTypes[5] = FTP_FTYPE_USER_WITH_NOLONGON;
			vTypes[6] = FTP_FTYPE_USER_FORE_IDATREMOTEHOST;
			vTypes[7] = FTP_FTYPE_USER_REMOTE_IDATREMOTEHOST_FIREID;
			vTypes[8] = FTP_FTYPE_USER_REMOTEIDATFIRE_IDAT_REMOTEHOST;
		}

		class HFtpCommand::CExtendedInfo : public HFtpCommand::IFtpExtendedInfo
		{
			typedef HFtpCommand::TSpecificationEnum TSpecificationEnum;
			typedef HFtpCommand::TTypeEnum TTypeEnum;
		public:
			CExtendedInfo(const std::string& strServerString,
				const std::string& strCompleteServerStringSyntax,
				unsigned int uiNumberOfParameters,
				unsigned int uiNumberOfOptionalParameters,
				TSpecificationEnum enSpecification,
				TTypeEnum enType) :
				m_strServerString(strServerString),
				m_strCompleteServerStringSyntax(strCompleteServerStringSyntax),
				m_uiNumberOfParameters(uiNumberOfParameters),
				m_uiNumberOfOptionalParameters(uiNumberOfOptionalParameters),
				m_enSpecification(enSpecification),
				m_enType(enType) {
			}

			CExtendedInfo(const CExtendedInfo& src) :
				m_strServerString(src.m_strServerString),
				m_strCompleteServerStringSyntax(src.m_strCompleteServerStringSyntax),
				m_uiNumberOfParameters(src.m_uiNumberOfParameters),
				m_uiNumberOfOptionalParameters(src.m_uiNumberOfOptionalParameters),
				m_enSpecification(src.m_enSpecification),
				m_enType(src.m_enType)
			{
			}

			virtual const std::string& GetServerString() const { return m_strServerString; }
			virtual const std::string& GetCompleteServerStringSyntax() const { return m_strCompleteServerStringSyntax; }
			virtual unsigned int GetNumberOfParameters() const { return m_uiNumberOfParameters; }
			virtual unsigned int GetNumberOfOptionalParameters() const { return m_uiNumberOfOptionalParameters; }
			virtual TSpecificationEnum GetSpecification() const  { return m_enSpecification; }
			virtual TTypeEnum GetType() const { return m_enType; }

			const std::string            m_strServerString;
			const std::string            m_strCompleteServerStringSyntax;
			const unsigned int               m_uiNumberOfParameters;
			const unsigned int               m_uiNumberOfOptionalParameters;
			const TSpecificationEnum m_enSpecification;
			const TTypeEnum          m_enType;
		};

		class HFtpCommand::CCmd2Info : private std::map<TCommandEnum, std::shared_ptr<CExtendedInfo>>
		{
			CCmd2Info();
			void Insert(TCommandEnum enCommand, CExtendedInfo* pExtendedInfo) { insert(std::make_pair(enCommand, std::shared_ptr<CExtendedInfo>(pExtendedInfo))); }
			void Insert(TCommandEnum enCommand, const std::string& strServerString, const std::string& strCompleteServerStringSyntax, unsigned int uiNumberOfParameters,
				unsigned int uiNumberOfOptionalParameters, TSpecificationEnum enSpecification, TTypeEnum enType)
			{
				insert(std::make_pair(enCommand, std::shared_ptr<CExtendedInfo>(new CExtendedInfo(strServerString, strCompleteServerStringSyntax, uiNumberOfParameters,
					uiNumberOfOptionalParameters, enSpecification, enType))));
			}

			static CCmd2Info& GetInstance() { static CCmd2Info TheOneAndOnly; return TheOneAndOnly; }
		public:
			static const IFtpExtendedInfo& Get(TCommandEnum enCommand);
		};

		HFtpCommand::CCmd2Info::CCmd2Info()
		{
			Insert(cmdABOR, ("ABOR"), ("ABOR <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdACCT, ("ACCT"), ("ACCT <SP> <account-information> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdALLO, ("ALLO"), ("ALLO <SP> <decimal-integer> [<SP> R <SP> <decimal-integer>] <CRLF>"), 3, 2, RFC959, NonDatachannel);
			Insert(cmdAPPE, ("APPE"), ("APPE <SP> <pathname> <CRLF>"), 1, 0, RFC959, DatachannelWrite);
			Insert(cmdCDUP, ("CDUP"), ("CDUP <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdCWD, ("CWD"), ("CWD <SP> <pathname> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdDELE, ("DELE"), ("DELE <SP> <pathname> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdHELP, ("HELP"), ("HELP [<SP> <string>] <CRLF>"), 1, 1, RFC959, NonDatachannel);
			Insert(cmdLIST, ("LIST"), ("LIST [<SP> <pathname>] <CRLF>"), 1, 1, RFC959, DatachannelRead);
			Insert(cmdMDTM, ("MDTM"), ("MDTM <SP> <pathname> <CRLF>"), 1, 0, RFC3659, NonDatachannel);
			Insert(cmdMKD, ("MKD"), ("MKD <SP> <pathname> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdMODE, ("MODE"), ("MODE <SP> <mode-code> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdNLST, ("NLST"), ("NLST [<SP> <pathname>] <CRLF>"), 1, 1, RFC959, DatachannelRead);
			Insert(cmdNOOP, ("NOOP"), ("NOOP <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdOPEN, ("OPEN"), ("OPEN <SP> <string> <CRLF>"), 1, 0, Unknown, NonDatachannel);
			Insert(cmdPASS, ("PASS"), ("PASS <SP> <password> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdPASV, ("PASV"), ("PASV <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdPORT, ("PORT"), ("PORT <SP> <host-port> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdPWD, ("PWD"), ("PWD <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdQUIT, ("QUIT"), ("QUIT <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdREIN, ("REIN"), ("REIN <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdREST, ("REST"), ("REST <SP> <marker> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdRETR, ("RETR"), ("RETR <SP> <pathname> <CRLF>"), 1, 0, RFC959, DatachannelRead);
			Insert(cmdRMD, ("RMD"), ("RMD <SP> <pathname> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdRNFR, ("RNFR"), ("RNFR <SP> <pathname> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdRNTO, ("RNTO"), ("RNTO <SP> <pathname> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdSITE, ("SITE"), ("SITE <SP> <string> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdSIZE, ("SIZE"), ("SIZE <SP> <pathname> <CRLF>"), 1, 0, RFC3659, NonDatachannel);
			Insert(cmdSMNT, ("SMNT"), ("SMNT <SP> <pathname> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdSTAT, ("STAT"), ("STAT [<SP> <pathname>] <CRLF>"), 1, 1, RFC959, NonDatachannel);
			Insert(cmdSTOR, ("STOR"), ("STOR <SP> <pathname> <CRLF>"), 1, 0, RFC959, DatachannelWrite);
			Insert(cmdSTOU, ("STOU"), ("STOU <CRLF>"), 0, 0, RFC959, DatachannelWrite);
			Insert(cmdSTRU, ("STRU"), ("STRU <SP> <structure-code> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdSYST, ("SYST"), ("SYST <CRLF>"), 0, 0, RFC959, NonDatachannel);
			Insert(cmdTYPE, ("TYPE"), ("TYPE <SP> <type-code> <CRLF>"), 1, 0, RFC959, NonDatachannel);
			Insert(cmdUSER, ("USER"), ("USER <SP> <username> <CRLF>"), 1, 0, RFC959, NonDatachannel);
		}

		const HFtpCommand::IFtpExtendedInfo& HFtpCommand::CCmd2Info::Get(TCommandEnum enCommand)
		{
			const_iterator it = GetInstance().find(enCommand);
			assert(it != GetInstance().end());
			return *it->second;
		}

		bool HFtpCommand::IsDatachannelReadCommand() const
		{
			return CCmd2Info::Get(m_enCommand).GetType() == DatachannelRead;
		}

		bool HFtpCommand::IsDatachannelWriteCommand() const
		{
			return CCmd2Info::Get(m_enCommand).GetType() == DatachannelWrite;
		}

		bool HFtpCommand::IsDatachannelCommand() const
		{
			return IsDatachannelReadCommand() || IsDatachannelWriteCommand();
		}

		std::string HFtpCommand::AsString() const
		{
			return CCmd2Info::Get(m_enCommand).GetServerString();
		}

		/// Returns the command string.
		/// @param[in] strArgument Parameter which have to be added to the command.
		std::string HFtpCommand::AsString(const HFtpArg& Arguments) const
		{
			if (Arguments.empty())
				return AsString();

			std::string strArgument;
			for (HFtpArg::const_iterator itArg = Arguments.begin(); itArg != Arguments.end(); ++itArg)
			{
				if (!itArg->empty())
					strArgument += (" ") + *itArg;
			}

			return AsString() + strArgument;
		}

		const HFtpCommand::IFtpExtendedInfo& HFtpCommand::GetExtendedInfo() const
		{
			return CCmd2Info::Get(m_enCommand);
		}
	}
}
