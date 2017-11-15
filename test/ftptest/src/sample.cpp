#include <stdio.h>

#include "FTPClient.h"

int main(int argc, char* argv[]){

	nsFTP::CFTPClient ftpClient;

	nsFTP::CLogonInfo loginInfo;

	loginInfo.SetHost("192.168.7.48", 21, "test", "123456", "test");

	bool reslut = ftpClient.Login(loginInfo);

	nsFTP::TStringVector fvec;
	ftpClient.List("/", fvec);

	ftpClient.Logout();

	getchar();

	return 0;
}
