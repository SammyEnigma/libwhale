#include "whale/basic/hlog.h"
//#include "whale/basic/hdatetime.h"
//#include "whale/data/hmysqldataset.h"
//#include "whale/data/hmysql.h"
//#include "whale/data/horacledataset.h"
//#include "whale/data/horacle.h"
//#include "whale/data/hmssqldataset.h"
//#include "whale/data/mssql.h"
//#include "whale/data/hsqlite.h"

//#include "whale/util/hxml.h"
//#include "system_string_util.h"
//#include "whale/net/hsocket.h"
//#include "whale/net/hnetenv.h"
//#include "whale/util/hjson.h"
//#include "whale/crypto/haes.h"
//#include "whale/crypto/hrsa.h"
//#include "whale/crypto/hmd5.h"
//#include <halys/net/htcpserver.h>

#define TEST_SIZE	1

//#include <halys/basic/threadsafequeue.h>
//#include <halys/basic/hthreadpool.h>
//#include <system_postgresql_dataset.h>
//#include "whale/net/hhttpserver.h"
//#include "whale/net/hdns.h"
//#include "whale/crypto/hmd5.h"
//#include "whale/crypto/hsha.h"
//#include "whale/crypto/haes.h"
//#include "whale/crypto/hrsa.h"
//#include "whale/crypto/hbase64.h"

//#include "whale/util/hjson.h"
//
//#include "whale/basic/hos.h"
//#include "whale/util/hsetting.h"
//#include "whale/util/hoptions.h"
//
//#include "whale/data/hmongodataset.h"
//#include "whale/crypto/hzip.h"
#include "whale/crypto/haes.h"

int main(int argc, char* argv[])
{
	Whale::Basic::HByteArray key("78c91ba01a3a5a37");
	Whale::Basic::HByteArray plain;/*
	plain.push_back(0x01);
	plain.push_back(0x20);
	plain.push_back(0x00);
	plain.push_back(0x03);
	plain.push_back(0x01);*/

	//Whale::Basic::HByteArray plain("helloworld666666");

	Whale::Basic::HByteArray cipher = Whale::Crypto::HAes::encode(plain, key);

	std::string sss = cipher.toHex();
	Whale::Basic::HByteArray srcs = Whale::Crypto::HAes::decode(cipher, key);

	std::string sss2 = srcs.toString();

	/*Whale::Basic::HByteArray cipher = Whale::Crypto::HAes::encode1(plain, key);

	std::string sss = cipher.toHex();
	Whale::Basic::HByteArray srcs = Whale::Crypto::HAes::decode1(cipher, key);

	std::string sss2 = srcs.toString();*/


	/*Whale::Basic::HByteArray content("22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222");
	Whale::Basic::HByteArray cipher = Whale::Crypto::HZip::encode(content);
	Whale::Basic::HByteArray plain = Whale::Crypto::HZip::decode(cipher);*/
	getchar();
	/*
	Whale::Util::HOptions hoption("1.2.3");
	hoption.setBriefString("Simple option's test program");

	hoption.insert('c', "config-file", "-c, --config-file=FILE\tUsing FILE as configuration file");

	hoption.load(argc, argv);
	*/
	//std::shared_ptr<Whale::Data::HMongo> mongo(new Whale::Data::HMongo);
	//mongo->init("mongodb://192.168.10.46:27017");

	//Whale::Data::HMongoDataSet d1(mongo, "dmd", "student");	
	//
	//std::vector<Whale::Util::HJson> jsons;
	//int count = 0;
	//while (count <	10) {
	//	Whale::Util::HJson js1;
	//	js1.decode("{\"name\":\"liap\", \"age\":123, \"age1\":123, \"age2\":123, \"age3\":123, \"age4\":123}");
	//	jsons.push_back(js1);
	//	std::cout << count++ << std::endl;
	//}

	//
	//count = 0;
	//while (count < 18000) {
	//	log_info("开始插入");
	//	d1.insertBulk(jsons);
	//	log_info("插入结束");
	//	std::cout << count++ << std::endl;
	//}

	//Whale::Util::HJson js11;
	//d1.select(js11);
	//log_info("记录总数：%d", d1.count());

	//getchar();

	//Whale::Data::HMongoDataSet dt(mongo, "bigdata", "BaseUserInfo");

	//Whale::Util::HJson js;
	////js.decode("{\"name\":\"liap\", \"age\":123, \"age1\":123, \"age2\":123, \"age3\":123, \"age4\":123}");
	////dt.insert(js);

	////js.clearArray(js);
	////js.decode("{\"age1\":123}");
	//dt.select(js);
	//int ss13 = dt.count();

	//int i = 0;

	//while (dt.next(js))	{
	//	std::cout << i++ << std::endl;
	//	//log_info("%s", js.encode(js).c_str());
	//}
	////std::cout << i++ << std::endl;
	//getchar();

	/*
	Whale::Util::HOptions opt;
	if (!opt.load(argc, argv))
	{		
		exit(-1);
	}

	std::string ss1 = Whale::Basic::HDateTime();


	while (true) {
		log_info(NULL);
	}
	getchar();*/

	//getchar();
	return 0;
	
	/*long long memory_using_size = System::General::Param::system_memory_using_size() / 1024 / 1024;
	long long memory_total_size = System::General::Param::system_memory_total_size() / 1024 / 1024;

	long long system_running_seconds = System::General::Param::system_running_seconds();
	long long disk_using_rate = System::General::Param::system_disk_using_rate();*/
	
	/*Whale::Util::HJson json;

	
	json.writeObject(json, "root", "123");

	std::vector<std::string> sss;
	sss.push_back("22");
	sss.push_back("23");
	sss.push_back("24");

	json.writeArray(json["root"], "n1", sss);
	json.pushBackArray(json["root"]["n1"], "222XXXXXX");
	json.writeObject(json["root"], "n2", "123");
	json.writeObject(json["root"]["n2"], "n2", "123");
	json.writeObject(json["root"]["n2"]["n2"], "n2", "123");
	bool ss = json.decode("{\"key\":123}");

	json.writeObject(json, "key", 11);

	std::string prettyJson = json.encodePretty(json);


	getchar();
	//json.insert("HH", std::vector<std::string>{"123", "sacda", "afdsaf"});
	/*json.insert("XX", 123);
	json.insert("XX", std::map<std::string, int>{std::make_pair("A", 0), std::make_pair("B", 1)});*/
	/*std::map<std::string, int> sssssss;
	sssssss["A"] = 32;
	sssssss["B"] = 3;

	std::vector<std::map<std::string, int>> ssss;
	ssss.push_back(sssssss);

	json.insert(0, ssss);
	json.insert(0, ssss);
	json.insert(0, ssss);
	json.insert(0, ssss);

	json.update(0, ssss);*/
	//json.insert("A", std::map<std::string, int>{std::make_pair("A", 0), std::make_pair("B", 1)});


	/*ssss.clear();
	json.value(0, ssss);*/

	

	//json.update("HH", std::vector<int>{4, 5, 6});
	

	//auto mm = json.valueX<const char*>("HH");

	
	/*unsigned char tmp[1024] = { '3', '1', '3', '2' };
	int len = strlen((char*)tmp);
	System::Encoder::Hex::decode(tmp, 1024, len);*/
	//std::string app = System::application();

	/*char msg[4096] = { "12324332222222222222" };
	msg[2] = '\0';
	msg[3] = '3';
	int msglen = 22;

	std::string md5 = System::Encoder::md5(msg, msglen);
	std::string sha256 = System::Encoder::sha256_calcu(msg, msglen);
	int len = sha256.length();
	bool sha256v = System::Encoder::sha256_verify(msg, msglen, sha256.c_str());

	System::Encoder::Base64::encode((unsigned char*)msg, 4096, msglen);
	System::Encoder::Base64::decode((unsigned char*)msg, 4096, msglen);

	System::Encoder::Hex::encode((unsigned char*)msg, 4096, msglen);
	System::Encoder::Hex::decode((unsigned char*)msg, 4096, msglen);

	System::Encoder::Aes aes;
	aes.encode((unsigned char*)msg, 4096, msglen);


	int sss = std::string(msg).length();

	aes.decode((unsigned char*)msg, 4096, msglen);

	std::string ssttsaf = (char*)aes.key();
	int lenss = ssttsaf.length();

	lenss = std::string((char*)aes.ivh()).length();

	System::Encoder::Rsa rsa;
	std::string enc = rsa.encode(msg);


	System::Encoder::Rsa rsa1(rsa.pubkey(), rsa.prikey());*/
	/*std::string dec = rsa1.decode(enc);

	std::string sign = rsa.signature(msg);
	bool v = rsa.verify(msg, sign);


	net_init();*/

	/*CreateW::HttpServer httpserver;
	httpserver.init(9800, "/");
	httpserver.run();
	getchar();
	httpserver.stop();*/
	/*std::vector<std::string> addrs;
	System::Utility::dns_parse_addr("www.sohu.com", addrs);

	std::vector<std::string> hostnames;
	System::Utility::dns_parse_hostname("www.baidu.com", hostnames);*/

	//net_uninit();
	

	/*System::Net::HttpServer httpsrv;
	httpsrv.init(8081, "D:/SVN/libpublic/libcommon/bin");
	httpsrv.run();

	getchar();
	httpsrv.stop();*/

	/*std::shared_ptr<System::PostgreSql> ps(new System::PostgreSql());
	ps->init("127.0.0.1", 5432, "postgres", "postgres", "123456");
	System::PostgreSqlDataSet pd(ps);

	pd.exec_select("select id, name, age, address from company");

	while (pd.next())
	{
		log_error("id : %d name : %s age : %d address : %s", pd.read_i32("id"), pd.read(1).c_str(), pd.read_i32("age"), pd.read(3).c_str());
	}

	getchar();*/

	/*
	net_init();

	System::Net::Socket sock[TEST_SIZE];

	for (int i = 0; i < TEST_SIZE; ++i)
	{
	sock[i].init();
	sock[i].connect("192.168.7.40", 9587);
	}

	//getchar();
	char buff[4096] = { 0 };
	int count = 10000;
	while (true)
	{
	for (int i = 0; i < TEST_SIZE; ++i)
	{
	memset(buff, 0, 4096);
	sprintf_s(buff, 4096, "[%d] : Hello World !!", i);
	sock[i].send(buff, strlen(buff));

	int len = 0;
	memset(buff, 0, 4096);
	sock[i].recv(buff, 4095, len);
	log_info("recv : %s !!", buff);
	}

	//Sleep(50);
	}


	for (int i = 0; i < TEST_SIZE; ++i)
	{
	sock[i].close();
	}

	getchar();

	*/

	/*if (!System::Mysql::instance()->init("127.0.0.1:3306", "cn_cardmanage", "root", "123456"))
	{
	log_error("db init failure.");
	return 0;
	}

	char sql[] = "select * from cn_system_config";
	System::MysqlDataSet dt;
	dt.exec_select(sql);

	while (dt.next())
	{
	log_info("size: %d serial %s type %s", dt.count(), dt.read("id").c_str(), dt.read("value").c_str());
	}

	getchar();
	*/

	/*if (!System::Oracle::instance()->init("192.168.7.111:1521", "orcl1", "comm", "comm"))
	{
	log_error("db init failure.");
	return 0;
	}

	char sql[] = "select * from comm_terminal";
	System::OracleDataSet dt;
	dt.exec_select(sql);

	while (dt.next())
	{
	log_info("size: %d serial %s type %s", dt.count(), dt.read(1).c_str(), dt.read(2).c_str());
	}

	getchar();*/
	/*
	if (!System::Mssql::instance()->init("192.168.10.103", "CardManage", "sa", "gdcn!@#$123"))
	{
	log_error("db init failure.");
	return 0;
	}

	char sql[] = "select * from System_Device";
	System::MssqlDataSet dt;
	dt.exec_select(sql);

	while (dt.next())
	{
	log_info("size: %d serial %s type %s", dt.count(), dt.read(1).c_str(), dt.read(2).c_str());
	}

	getchar();

	*/
	/*
	System::Net::LibeventServer server;
	server.init(9601, 0, 0);
	server.run(true);
	getchar();
	server.stop();
	getchar();
	*/
	/*
	System::Encrypt::Rsa rsa;

	std::string md5_public_key = System::Encrypt::md5((unsigned char*)rsa.public_key().c_str(), rsa.public_key().length());
	cout << "md5 text : " << md5_public_key << endl;

	const char* msg = "asdfas啊的事发生都是短发圣达菲啊252345sdfefasdfasdf";

	unsigned char plain[1024] = { 0 };
	unsigned char cipher[1024] = { 0 };

	int lenxx = 0;
	rsa.encrypt((const unsigned char*)msg, strlen((char*)msg), cipher, 1024, lenxx);
	cout << "cipher text : " << cipher << endl;
	rsa.decrypt(cipher, lenxx, plain, 1024, lenxx);
	cout << "plain text : " << plain << endl;
	getchar();
	*/
	/*System::Encrypt::Aes aes;

	const char* msg = "asdfas啊的事发生都是短发圣达菲啊252345sdfefasdfasdf";

	unsigned char plain[1024] = { 0 };
	unsigned char cipher[1024] = { 0 };

	int lenxx = 0;
	aes.encrypt((const unsigned char*)msg, strlen((char*)msg), cipher, 1024, lenxx);
	aes.decrypt(cipher, lenxx, plain, 1024, lenxx);

	cout << plain << endl;
	getchar();*/

	/*const char* jd = "{\"chinese\":[{\"ID\":1,\"String\":\"五子棋\"},{\"ID\":2,\"String\":\"当前玩家：\"},{\"ID\":3,\"String\":\"黑色方\"},{\"ID\":4,\"String\":\"白色方\"},{\"ID\":5,\"String\":\"选择玩家\"},{\"ID\":6,\"String\":\"失败\"},{\"ID\":7,\"String\":\"胜利\"}]}";
	System::Utility::Json json;
	bool ss = json.parse(jd);*/
	//bool ss = json.load_from_file("1.json");

	//std::string xx;
	//json.clear();
	////json.remove("person");
	////json.value("chinese-2.String", xx);
	//json.save_to_file("1.json");

	//log_info(json.content().c_str());

	//getchar();

	//log_init(9, true, false);

	//for (int i = 0; i < TEST_SIZE; ++i)
	//{
	//	log_debug("current index [%d]", i);
	//}	

	/*
	System::Net::Socket sock[TEST_SIZE];
	for (int i = 0; i < TEST_SIZE; ++i)
	{
	sock[i].init();
	sock[i].connect("127.0.0.1", 9350);
	std::string dt = System::Utility::StringUtil::from_int32(i);
	sock[i].send(dt.c_str(), dt.length());
	}

	getchar();

	for (int i = 0; i < TEST_SIZE; ++i)
	{
	sock[i].close();
	}*/

	/*
	System::Timer timer;

	for (int i = 0; i < 30000; ++i)
	{
	log_error("size: %d", i);
	}

	log_emerg("elapsed: %d", timer.elapsed());
	*/

	/*
	System::Net::Socket sock;
	sock.init(System::Net::SocketType::S_TCP);
	sock.connect("192.168.10.103", 9001);
	sock.send("123456", 6);
	sock.close();
	*/




	/*
	System::Database::Sqlite db;
	if (!db.open("test.db"))
	{
	db.close();
	log_error("sqlite db init failure.");
	return 0;
	}

	char sql[1024] = { 0 };
	sprintf(sql, "%s", ("select * from sqlite_master where type='table' and name='tab_test'"));
	System::Database::SqliteDataSet dataset = db.executeSelect(sql);
	if (!dataset.next())
	{
	memset(sql, 0, sizeof(sql));
	sprintf(sql, "%s", "CREATE TABLE [tab_test] ("
	"[id] INTEGER NOT NULL PRIMARY KEY, "
	"[serial] NVARCHAR(38));"
	"CREATE INDEX [index_id] ON [tab_test] ([id]);");
	if (!db.execute(sql))
	{
	log_error("create tab_record failure [%s]", db.err());
	db.close();
	return false;
	}
	}

	for (int i = 0; i < 10; i++)
	{
	memset(sql, 0, 1024);
	sprintf(sql, "insert into tab_test(id, serial) values(%d, '123456')", i);
	log_error("insert %d result 【%d】", i, db.execute(sql));
	}

	db.begin();

	memset(sql, 0, 1024);
	sprintf(sql, "update tab_test set serial='你好 !!' where id = 0");
	log_error("update table 【%d】", db.execute(sql));

	memset(sql, 0, 1024);
	sprintf(sql, "select * from tab_test");
	System::Database::SqliteDataSet dataset1 = db.executeSelect(sql);
	if (dataset1.next()){
	log_error("id: %d serial: %s", dataset1.fieldValue_i(0), dataset1.fieldValue_s(1));
	}

	db.rollback();

	memset(sql, 0, 1024);
	sprintf(sql, "select * from tab_test");
	System::Database::SqliteDataSet dataset2 = db.executeSelect(sql);
	if (dataset2.next()){
	log_error("id: %d serial: %s", dataset2.fieldValue_i(0), dataset2.fieldValue_s(1));
	}

	*/


	//System::Utility::Xml xml;
	//xml.parse("<?xml version='1.0' encoding='UTF-8' ?><Request cmd = \"UploadACSRecord\"></Request>");

	//xml.load_from_file("1.xml");

	//cout << xml.xml();

	//xml.save_to_file("1.xml");



	//System::Utility::Xml xml1;

	//xml1.add_root("response");
	//xml1.add_child("response", "head");
	//xml1.add_child("response.head", "command");
	//xml1.set_value("response.head.command", 1024);
	//xml1.add_child("response", "content");
	//xml1.add_child("response.content", "record", 10);

	//cout << xml1.xml();

	//xml1.save_to_file("2.xml");

	return 0;
}
