#include "whale/basic/hplatform.h"
#include "whale/net/hhttpserver.h"
#include "whale/basic/hlog.h"
#include "whale/net/hsocket.h"
#include "whale/basic/hos.h"
#include "whale/util/hstring.h"

#ifdef OS_WIN32
#pragma comment(lib, "z:/libwhale/deps/libevent-2.1.8-stable/lib/libevent.lib")
#pragma comment(lib, "z:/libwhale/deps/libevent-2.1.8-stable/lib/libevent_core.lib")
#pragma comment(lib, "z:/libwhale/deps/libevent-2.1.8-stable/lib/libevent_extras.lib")
#endif

const static char *parseContentType(const char *path);

namespace Whale
{
	namespace Net
	{
		HHttpServer::HHttpServer() : threadPool_(new Whale::Basic::HThreadPool())
		{
#ifdef OS_WIN32
			// Initialize the WinSock dll version 2.0
			WSADATA  wsaData = { 0 };
			if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
				std::cout << "Init Windows Socket Failed !! Err: " << GetLastError() << std::endl;
			}
#endif

			timeoutSeconds_ = 30;
			home_ = "index.html";
		}

		HHttpServer::~HHttpServer()
		{

		}

		bool HHttpServer::create(int port, const std::string& docRoot, const std::string& home, int threads)
		{
			port_ = port;
			docRoot_ = docRoot;
			home_ = home;

			evbase_ = event_base_new();

			if (!evbase_){
				log_error("Function event_base_new Failure !!");
				return false;
			}

			evhttp_ = evhttp_new(evbase_);

			if (!evhttp_){
				log_error("Function evhttp_new Failure !!");
				return false;
			}

			evhttp_set_timeout(evhttp_, timeoutSeconds_);

			//evhttp_set_cb(_evhttp, "/dump", evhttp_dump_cb, this);
			evhttp_set_gencb(evhttp_, generalCallback, this);

			evsock_ = evhttp_bind_socket_with_handle(evhttp_, "0.0.0.0", port_);

			if (!evsock_){
				log_error("Function evhttp_bind_socket_with_handle failure !!");
				return false;
			}

			std::vector<std::string> addrs = Whale::ipAddress();

			if (addrs.empty()) {
				log_notice("Can't catch local address, Please checking !!");
				exit(-1);
			}
			addr_ = addrs[0];

			log_notice("Http server [%s:%d] started !!", addr_.c_str(), port_);
			
			onlyOnceRecyle(true);

			return threadPool_->init(threads);
		}

		void HHttpServer::stop()
		{
			event_base_loopexit(evbase_, NULL);
			evhttp_free(evhttp_);
			event_base_free(evbase_);
			Whale::Basic::HThread::stop();
			log_notice("Exited !!");
		};

		void HHttpServer::action()
		{
#ifdef OS_WIN32
			evthread_use_windows_threads();
#else
			evthread_use_pthreads();
#endif
			evthread_make_base_notifiable(evbase_);

			event_base_dispatch(evbase_);
		};

		void HHttpServer::generalCallback(struct evhttp_request *request, void *pvoid)
		{
			assert(pvoid);
			auto httpServer = static_cast<Whale::Net::HHttpServer*>(pvoid);

//			httpServer->threadPool()->assign(
//				std::bind([=](
//					Whale::Net::HHttpServer* httpServer, 
//					struct evhttp_request *request) -> void {
//				auto session = std::make_shared<Whale::Net::HHttpSession>(httpServer);
//
//				session->request().insertObject(session->request(), "HEAD");
//				session->request().insertObject(session->request(), "FORM");
//				session->request().insertObject(session->request(), "REFS");
//				session->request().insertObject(session->request(), "LOCATION");
//				session->request().insertObject(session->request(), "REQUEST");
//				session->request().insertObject(session->request(), "REPONSE");
//
//				std::string cmdtype = "UNKNOWN";
//				switch (evhttp_request_get_command(request)) {
//				case EVHTTP_REQ_GET:
//					cmdtype = "GET";
//					break;
//				case EVHTTP_REQ_POST:
//					cmdtype = "POST";
//					break;
//				case EVHTTP_REQ_HEAD:
//					cmdtype = "HEAD";
//					break;
//				case EVHTTP_REQ_PUT:
//					cmdtype = "PUT";
//					break;
//				case EVHTTP_REQ_DELETE:
//					cmdtype = "DELETE";
//					break;
//				case EVHTTP_REQ_OPTIONS:
//					cmdtype = "OPTIONS";
//					break;
//				case EVHTTP_REQ_TRACE:
//					cmdtype = "TRACE";
//					break;
//				case EVHTTP_REQ_CONNECT:
//					break;
//				case EVHTTP_REQ_PATCH:
//					cmdtype = "PATCH";
//					break;
//				default:
//					cmdtype = "UNKNOWN";
//					break;
//				}
//
//				session->request().writeObject(session->request()["HEAD"], "Type", cmdtype);
//
//				const char* uri = evhttp_request_get_uri(request);
//				struct evhttp_uri *httpUri = evhttp_uri_parse(uri);
//
//				if (!httpUri) {
//					log_error("Decode %s failure !!", uri);
//					return;
//				}
//
//				const char *path = evhttp_uri_get_path(httpUri);
//
//				if (!path) {
//					path = "/";
//					log_notice("Can't get url %s Path, Use default /.", uri);
//				}
//
//				/* We need to decode it, to see what path the user really wanted. */
//				char* uridecode = evhttp_uridecode(path, 1, NULL);
//				std::string location = uridecode;
//
//				if (uridecode) {
//					free(uridecode);
//				}
//
//				if (location.empty()) {
//					log_error("Function evhttp_uridecode %s failure!!", path);
//					return;
//				}
//
//				log_notice("HTTP Url: %s", location.c_str());
//
//				session->request().writeObject(session->request(), "LOCATION", location);
//
//				struct evkeyvalq *inputHeaders = evhttp_request_get_input_headers(request);
//
//				for (auto header = inputHeaders->tqh_first; header; header = header->next.tqe_next) {
//					log_notice("HTTP %s: %s", header->key, header->value);
//					session->request().writeObject(session->request()["HEAD"], std::string(header->key), std::string(header->value));
//				}
//
//				struct evkeyvalq hrefs;
//				evhttp_parse_query(evhttp_request_get_uri(request), &hrefs);
//
//				for (auto href = hrefs.tqh_first; href; href = href->next.tqe_next) {
//					log_notice("HTTP Param: [%s] - [%s]", href->key, href->value);
//					session->request().writeObject(session->request()["REFS"], std::string(href->key), std::string(href->value));
//				}
//
//				size_t size = evbuffer_get_length(request->input_buffer);
//
//				char* buffer = new char[size + 1];
//				buffer[size] = '\0';
//				size = evbuffer_copyout(request->input_buffer, buffer, size);
//				char* requestPtr = evhttp_uridecode(buffer, 1, &size);
//				session->request().writeObject(session->request(), "REQUEST",
//#ifdef OS_WIN32
//					Whale::Util::HString::utf8ToString(std::string(requestPtr, size))
//#else
//					std::string(requestPtr, size)
//#endif				
//				);
//				delete[] buffer; buffer = 0;
//
//				if (requestPtr) {
//					free(requestPtr);
//				}
//
//				std::string ext = ".do";
//				if (location.length() >= ext.length() &&
//					location.substr(location.length() - ext.length(), ext.length()) == ext) {
//					std::vector<std::string> kvList =
//						Whale::Util::HString::split(session->request().readString(session->request(), "REQUEST"), "&");
//
//					for (auto it : kvList) {
//						std::vector<std::string> kv = Whale::Util::HString::split(it, "=");
//
//						if (kv.size() == 2) {
//							session->request().writeObject(session->request()["FORM"], kv[0], kv[1]);
//						}
//					}
//
//					httpServer->signalRequest(session);
//					location = session->request().readString(session->request(), "LOCATION");
//				}
//
//				if (location.compare("/") == 0) {
//					location += httpServer->home_;
//					log_notice("Website Root Unauthorized, Jump to Default Url: %s !!", location.c_str());
//				}
//
//				struct evbuffer *responsePtr = evbuffer_new();
//
//				if (session->request().readString(session->request(), "RESPONSE").empty()) {
//					if (strstr(location.c_str(), "..")) {
//						log_error("Forbiden, Url %s Contain .. !!", location.c_str());
//						evhttp_send_error(request, HTTP_BADREQUEST, 0);
//						goto REQUEST_AND_RESPONSE_FINISHED;
//					}
//
//					std::string filePath = httpServer->docRoot_ + location;
//
//					if (filePath[filePath.length() - 1] == '/') {
//						filePath.pop_back();
//					}
//
//					struct stat fileStat;
//
//					if (stat(filePath.c_str(), &fileStat) < 0) {
//						log_error("File %s Isn't Existed !!", filePath.c_str());
//						evhttp_send_error(request, HTTP_NOTFOUND, 0);
//						goto REQUEST_AND_RESPONSE_FINISHED;
//					}
//
//					if (S_ISDIR(fileStat.st_mode)) {
//#ifdef WIN32
//						HANDLE hd;
//						WIN32_FIND_DATA finddt;
//#else
//						DIR *hd;
//						struct dirent *finddt;
//#endif
//						const char *trailing_slash = "";
//
//						if (!strlen(path) || path[strlen(path) - 1] != '/') {
//							trailing_slash = "/";
//						}
//
//#ifdef WIN32
//						hd = ::FindFirstFile((filePath + "/*.*").c_str(), &finddt);
//
//						if (hd == INVALID_HANDLE_VALUE) {
//							log_error("Request %s Isn't Existed !!", filePath.c_str());
//							evhttp_send_error(request, HTTP_NOTFOUND, 0);
//							goto REQUEST_AND_RESPONSE_FINISHED;
//						}
//#else
//						if (!(hd = opendir(filePath.c_str()))) {
//							log_error("Request Url %s Isn't Exist!!", filePath.c_str());
//							evhttp_send_error(request, HTTP_NOTFOUND, 0);
//							goto REQUEST_AND_RESPONSE_FINISHED;
//						}
//#endif
//
//						evbuffer_add_printf(responsePtr,
//							"<html>\n<head>\n<title>%s</title>\n<base href='%s%s'>\n</head>\n<body>\n<h1>%s</h1>\n<ul>\n",
//							location.c_str(), path, trailing_slash, location.c_str());
//#ifdef WIN32
//						do {
//							const char *fileName = finddt.cFileName;
//#else
//						while ((finddt = readdir(hd))) {
//							const char *fileName = finddt->d_name;
//#endif
//							if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0)
//							{
//								continue;
//							}
//
//							evbuffer_add_printf(responsePtr, "<li><a href=\"%s\">%s</a>\n", fileName, fileName);
//#ifdef WIN32
//						} while (FindNextFileA(hd, &finddt));
//#else
//						}
//#endif
//						evbuffer_add_printf(responsePtr, "</ul></body></html>\n");
//#ifdef WIN32
//						FindClose(hd);
//#else
//						closedir(hd);
//#endif
//						evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type", "text/html");
//						evhttp_send_reply(request, HTTP_OK, "OK", responsePtr);
//					}
//					else {
//						int fd = -1;
//#ifdef OS_WIN32
//						fd = open(filePath.c_str(), O_RDONLY | O_BINARY);
//#else
//						fd = open(filePath.c_str(), O_RDONLY);
//#endif
//						if (fd < 0 || fstat(fd, &fileStat) < 0) {
//							log_error("Request %s Isn't Existed!!", filePath.c_str());
//							if (fd > 0) close(fd);
//							evhttp_send_error(request, HTTP_NOTFOUND, "Document Can't Open !!");
//							goto REQUEST_AND_RESPONSE_FINISHED;
//						}
//
//						evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type", parseContentType(location.c_str()));
//						evbuffer_add_file(responsePtr, fd, 0, fileStat.st_size);
//					}
//
//					evhttp_send_reply(request, HTTP_OK, "OK", responsePtr);
//				}
//				else {
//					evhttp_add_header(request->output_headers, "Server", "myhttpd v 0.0.1");
//					evhttp_add_header(request->output_headers, "Content-Type", parseContentType(location.c_str()));
//					evhttp_add_header(request->output_headers, "Connection", "close");
//
//					evbuffer_add_printf(responsePtr,
//#ifdef OS_WIN32
//						Whale::Util::HString::stringToUTF8(session->request().readString(session->request(), "RESPONSE")).c_str()
//#else
//						session->request().readString(session->request(), "RESPONSE").c_str()
//#endif
//
//					);
//					evhttp_send_reply(request, HTTP_OK, "OK", responsePtr);
//				}
//
//			REQUEST_AND_RESPONSE_FINISHED:
//
//				evhttp_clear_headers(&hrefs);
//
//				if (responsePtr) {
//					evbuffer_free(responsePtr);
//				}
//
//				if (httpUri) {
//					evhttp_uri_free(httpUri);
//				}
//			}, httpServer, request));

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
			
			auto session = std::make_shared<Whale::Net::HHttpSession>(httpServer);
			
			session->request().insertObject(session->request(), "HEAD");
			session->request().insertObject(session->request(), "FORM");
			session->request().insertObject(session->request(), "REFS");
			session->request().insertObject(session->request(), "LOCATION");
			session->request().insertObject(session->request(), "REQUEST");
			session->request().insertObject(session->request(), "REPONSE");

			std::string cmdtype = "UNKNOWN";
			switch (evhttp_request_get_command(request)) {
			case EVHTTP_REQ_GET:
				cmdtype = "GET";
				break;
			case EVHTTP_REQ_POST:
				cmdtype = "POST";
				break;
			case EVHTTP_REQ_HEAD:
				cmdtype = "HEAD";
				break;
			case EVHTTP_REQ_PUT:
				cmdtype = "PUT";
				break;
			case EVHTTP_REQ_DELETE:
				cmdtype = "DELETE";
				break;
			case EVHTTP_REQ_OPTIONS:
				cmdtype = "OPTIONS";
				break;
			case EVHTTP_REQ_TRACE:
				cmdtype = "TRACE";
				break;
			case EVHTTP_REQ_CONNECT:
				break;
			case EVHTTP_REQ_PATCH:
				cmdtype = "PATCH";
				break;
			default:
				cmdtype = "UNKNOWN";
				break;
			}

			session->request().writeObject(session->request()["HEAD"], "Type", cmdtype);

			const char* uri = evhttp_request_get_uri(request);
			struct evhttp_uri *httpUri = evhttp_uri_parse(uri);

			if (!httpUri)	{
				log_error("Decode %s failure !!", uri);
				return;
			}

			const char *path = evhttp_uri_get_path(httpUri);

			if (!path) {
				path = "/";
				log_notice("Can't get url %s Path, Use default /.", uri);				
			}

			/* We need to decode it, to see what path the user really wanted. */
			char* uridecode = evhttp_uridecode(path, 1, NULL);
			std::string location = uridecode;

			if (uridecode) { 
				free(uridecode); 
			}

			if (location.empty()) {
				log_error("Function evhttp_uridecode %s failure!!", path);
				return;
			}

			log_notice("HTTP Url: %s", location.c_str());		

			session->request().writeObject(session->request(), "LOCATION", location);

			struct evkeyvalq *inputHeaders = evhttp_request_get_input_headers(request);

			for (auto header = inputHeaders->tqh_first; header; header = header->next.tqe_next)	{
				log_notice("HTTP %s: %s", header->key, header->value);
				session->request().writeObject(session->request()["HEAD"], std::string(header->key), std::string(header->value));
			}

			struct evkeyvalq hrefs;
			evhttp_parse_query(evhttp_request_get_uri(request), &hrefs);

			for (auto href = hrefs.tqh_first; href; href = href->next.tqe_next)	{
				log_notice("HTTP Param: [%s] - [%s]", href->key, href->value);
				session->request().writeObject(session->request()["REFS"], std::string(href->key), std::string(href->value));
			}

			size_t size = evbuffer_get_length(request->input_buffer);

			char* buffer = new char[size + 1];
			buffer[size] = '\0';
			size = evbuffer_copyout(request->input_buffer, buffer, size);
			char* requestPtr = evhttp_uridecode(buffer, 1, &size);
			session->request().writeObject(session->request(), "REQUEST", 
#ifdef OS_WIN32
				Whale::Util::HString::utf8ToString(std::string(requestPtr, size))
#else
				std::string(requestPtr, size)
#endif				
				);
			delete[] buffer; buffer = 0;

			if (requestPtr) {
				free(requestPtr);
			}

			std::string ext = ".do";
			if (location.length() >= ext.length() &&
				location.substr(location.length() - ext.length(), ext.length()) == ext)	{
				std::vector<std::string> kvList = 
					Whale::Util::HString::split(session->request().readString(session->request(), "REQUEST"), "&");

				for (auto it : kvList) {
					std::vector<std::string> kv = Whale::Util::HString::split(it, "=");

					if (kv.size() == 2)	{ 
						session->request().writeObject(session->request()["FORM"], kv[0], kv[1]);
					}
				}

				httpServer->signalRequest(session);
				location = session->request().readString(session->request(), "LOCATION");
			}			

			if (location.compare("/") == 0) {
				location += httpServer->home_;
				log_notice("Website Root Unauthorized, Jump to Default Url: %s !!", location.c_str());
			}

			struct evbuffer *responsePtr = evbuffer_new();

			if (session->request().readString(session->request(), "RESPONSE").empty())	{
				if (strstr(location.c_str(), ".."))	{
					log_error("Forbiden, Url %s Contain .. !!", location.c_str());
					evhttp_send_error(request, HTTP_BADREQUEST, 0);
					goto REQUEST_AND_RESPONSE_FINISHED;
				}

				std::string filePath = httpServer->docRoot_ + location;

				if (filePath[filePath.length() - 1] == '/')	{
					filePath.pop_back();
				}

				struct stat fileStat;

				if (stat(filePath.c_str(), &fileStat) < 0)	{
					log_error("File %s Isn't Existed !!", filePath.c_str());
					evhttp_send_error(request, HTTP_NOTFOUND, 0);
					goto REQUEST_AND_RESPONSE_FINISHED;
				}

				if (S_ISDIR(fileStat.st_mode)) {
#ifdef WIN32
					HANDLE hd;
					WIN32_FIND_DATA finddt;
#else
					DIR *hd;
					struct dirent *finddt;
#endif
					const char *trailing_slash = "";

					if (!strlen(path) || path[strlen(path) - 1] != '/')	{
						trailing_slash = "/";
					}

#ifdef WIN32
					hd = ::FindFirstFile((filePath + "/*.*").c_str(), &finddt);

					if (hd == INVALID_HANDLE_VALUE)	{
						log_error("Request %s Isn't Existed !!", filePath.c_str());
						evhttp_send_error(request, HTTP_NOTFOUND, 0);
						goto REQUEST_AND_RESPONSE_FINISHED;
					}
#else
					if (!(hd = opendir(filePath.c_str())))	{
						log_error("Request Url %s Isn't Exist!!", filePath.c_str());
						evhttp_send_error(request, HTTP_NOTFOUND, 0);
						goto REQUEST_AND_RESPONSE_FINISHED;
					}
#endif

					evbuffer_add_printf(responsePtr,
						"<html>\n<head>\n<title>%s</title>\n<base href='%s%s'>\n</head>\n<body>\n<h1>%s</h1>\n<ul>\n",
						location.c_str(), path, trailing_slash, location.c_str());
#ifdef WIN32
					do {
						const char *fileName = finddt.cFileName;
#else
					while ((finddt = readdir(hd))) {
						const char *fileName = finddt->d_name;
#endif
						if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0)
						{
							continue;
						}

						evbuffer_add_printf(responsePtr, "<li><a href=\"%s\">%s</a>\n", fileName, fileName);
#ifdef WIN32
					} while (FindNextFileA(hd, &finddt));
#else
					}
#endif
					evbuffer_add_printf(responsePtr, "</ul></body></html>\n");
#ifdef WIN32
					FindClose(hd);
#else
					closedir(hd);
#endif
					evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type", "text/html");
					evhttp_send_reply(request, HTTP_OK, "OK", responsePtr);
				}
				else{
					int fd = -1;
#ifdef OS_WIN32
					fd = open(filePath.c_str(), O_RDONLY | O_BINARY);
#else
					fd = open(filePath.c_str(), O_RDONLY);
#endif
					if (fd < 0 || fstat(fd, &fileStat) < 0)	{
						log_error("Request %s Isn't Existed!!", filePath.c_str());
						if (fd > 0) close(fd);
						evhttp_send_error(request, HTTP_NOTFOUND, "Document Can't Open !!");
						goto REQUEST_AND_RESPONSE_FINISHED;
					}

					evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type", parseContentType(location.c_str()));
					evbuffer_add_file(responsePtr, fd, 0, fileStat.st_size);
				}

				evhttp_send_reply(request, HTTP_OK, "OK", responsePtr);
			}
			else {
				evhttp_add_header(request->output_headers, "Server", "myhttpd v 0.0.1");
				evhttp_add_header(request->output_headers, "Content-Type", parseContentType(location.c_str()));
				evhttp_add_header(request->output_headers, "Connection", "close");

				evbuffer_add_printf(responsePtr, 
#ifdef OS_WIN32
					Whale::Util::HString::stringToUTF8(session->request().readString(session->request(), "RESPONSE")).c_str()
#else
					session->request().readString(session->request(), "RESPONSE").c_str()
#endif

					);
				evhttp_send_reply(request, HTTP_OK, "OK", responsePtr);
			}

		REQUEST_AND_RESPONSE_FINISHED:

			evhttp_clear_headers(&hrefs);

			if (responsePtr) {
				evbuffer_free(responsePtr);
			}

			if (httpUri) {
				evhttp_uri_free(httpUri);
			}
		}
	}
}

struct HttpEntry
{
	const char *_extension;
	const char *_contentType;
};

const static HttpEntry g_httpEntryTable[] =
{
	{ "...*", "application/octet-stream; charset=UTF-8" },
	{ ".do", "application/json; charset=UTF-8" },
	{ ".txt", "text/plain; charset=UTF-8" },
	{ ".c", "text/plain; charset=UTF-8" },
	{ ".h", "text/plain; charset=UTF-8" },
	{ ".cpp", "text/plain; charset=UTF-8" },
	{ ".hpp", "text/plain; charset=UTF-8" },
	{ ".tif", "image/tiff; charset=UTF-8" },
	{ ".001", "application/x-001; charset=UTF-8" },
	{ ".301", "application/x-301; charset=UTF-8" },
	{ ".323", "text/h323; charset=UTF-8" },
	{ ".906", "application/x-906; charset=UTF-8" },
	{ ".907", "drawing/907; charset=UTF-8" },
	{ ".a11", "application/x-a11; charset=UTF-8" },
	{ ".acp", "audio/x-mei-aac; charset=UTF-8" },
	{ ".ai", "application/postscript; charset=UTF-8" },
	{ ".aif", "audio/aiff; charset=UTF-8" },
	{ ".aifc", "audio/aiff; charset=UTF-8" },
	{ ".aiff", "audio/aiff; charset=UTF-8" },
	{ ".anv", "application/x-anv; charset=UTF-8" },
	{ ".asa", "text/asa; charset=UTF-8" },
	{ ".asf", "video/x-ms-asf; charset=UTF-8" },
	{ ".asp", "text/asp; charset=UTF-8" },
	{ ".asx", "video/x-ms-asf; charset=UTF-8" },
	{ ".au", "audio/basic; charset=UTF-8" },
	{ ".avi", "video/avi; charset=UTF-8" },
	{ ".awf", "application/vnd; charset=UTF-8" },
	{ ".adobe", "workflow; charset=UTF-8" },
	{ ".biz", "text/xml; charset=UTF-8" },
	{ ".bmp", "application/x-bmp; charset=UTF-8" },
	{ ".bot", "application/x-bot; charset=UTF-8" },
	{ ".c4t", "application/x-c4t; charset=UTF-8" },
	{ ".c90", "application/x-c90; charset=UTF-8" },
	{ ".cal", "application/x-cals; charset=UTF-8" },
	{ ".cat", "application/vnd.ms-pki.seccat; charset=UTF-8" },
	{ ".cdf", "application/x-netcdf; charset=UTF-8" },
	{ ".cdr", "application/x-cdr; charset=UTF-8" },
	{ ".cel", "application/x-cel; charset=UTF-8" },
	{ ".cer", "application/x-x509-ca-cert; charset=UTF-8" },
	{ ".cg4", "application/x-g4; charset=UTF-8" },
	{ ".cgm", "application/x-cgm; charset=UTF-8" },
	{ ".cit", "application/x-cit; charset=UTF-8" },
	{ ".class", "java/*; charset=UTF-8" },
	{ ".cml", "text/xml; charset=UTF-8" },
	{ ".cmp", "application/x-cmp; charset=UTF-8" },
	{ ".cmx", "application/x-cmx; charset=UTF-8" },
	{ ".cot", "application/x-cot; charset=UTF-8" },
	{ ".crl", "application/pkix-crl; charset=UTF-8" },
	{ ".crt", "application/x-x509-ca-cert; charset=UTF-8" },
	{ ".csi", "application/x-csi; charset=UTF-8" },
	{ ".css", "text/css; charset=UTF-8" },
	{ ".cut", "application/x-cut; charset=UTF-8" },
	{ ".dbf", "application/x-dbf; charset=UTF-8" },
	{ ".dbm", "application/x-dbm; charset=UTF-8" },
	{ ".dbx", "application/x-dbx; charset=UTF-8" },
	{ ".dcd", "text/xml; charset=UTF-8" },
	{ ".dcx", "application/x-dcx; charset=UTF-8" },
	{ ".der", "application/x-x509-ca-cert; charset=UTF-8" },
	{ ".dgn", "application/x-dgn; charset=UTF-8" },
	{ ".dib", "application/x-dib; charset=UTF-8" },
	{ ".dll", "application/x-msdownload; charset=UTF-8" },
	{ ".doc", "application/msword; charset=UTF-8" },
	{ ".dot", "application/msword; charset=UTF-8" },
	{ ".drw", "application/x-drw; charset=UTF-8" },
	{ ".dtd", "text/xml; charset=UTF-8" },
	{ ".dwf", "Model/vnd.dwf; charset=UTF-8" },
	{ ".dwf", "application/x-dwf; charset=UTF-8" },
	{ ".dwg", "application/x-dwg; charset=UTF-8" },
	{ ".dxb", "application/x-dxb; charset=UTF-8" },
	{ ".dxf", "application/x-dxf; charset=UTF-8" },
	{ ".edn", "application/vnd.adobe.edn; charset=UTF-8" },
	{ ".emf", "application/x-emf; charset=UTF-8" },
	{ ".eml", "message/rfc822; charset=UTF-8" },
	{ ".ent", "text/xml; charset=UTF-8" },
	{ ".epi", "application/x-epi; charset=UTF-8" },
	{ ".eps", "application/x-ps; charset=UTF-8" },
	{ ".eps", "application/postscript; charset=UTF-8" },
	{ ".etd", "application/x-ebx; charset=UTF-8" },
	{ ".exe", "application/x-msdownload; charset=UTF-8" },
	{ ".fax", "image/fax; charset=UTF-8" },
	{ ".fdf", "application/vnd.fdf; charset=UTF-8" },
	{ ".fif", "application/fractals; charset=UTF-8" },
	{ ".fo", "text/xml; charset=UTF-8" },
	{ ".frm", "application/x-frm; charset=UTF-8" },
	{ ".g4", "application/x-g4; charset=UTF-8" },
	{ ".gbr", "application/x-gbr; charset=UTF-8" },
	{ ".", "application/x-; charset=UTF-8" },
	{ ".gif", "image/gif; charset=UTF-8" },
	{ ".gl2", "application/x-gl2; charset=UTF-8" },
	{ ".gp4", "application/x-gp4; charset=UTF-8" },
	{ ".hgl", "application/x-hgl; charset=UTF-8" },
	{ ".hmr", "application/x-hmr; charset=UTF-8" },
	{ ".hpg", "application/x-hpgl; charset=UTF-8" },
	{ ".hpl", "application/x-hpl; charset=UTF-8" },
	{ ".hqx", "application/mac-binhex40; charset=UTF-8" },
	{ ".hrf", "application/x-hrf; charset=UTF-8" },
	{ ".hta", "application/hta; charset=UTF-8" },
	{ ".htc", "text/x-component; charset=UTF-8" },
	{ ".htm", "text/html; charset=UTF-8" },
	{ ".html", "text/html; charset=UTF-8" },
	{ ".htt", "text/webviewhtml; charset=UTF-8" },
	{ ".htx", "text/html; charset=UTF-8" },
	{ ".icb", "application/x-icb; charset=UTF-8" },
	{ ".ico", "image/x-icon; charset=UTF-8" },
	{ ".ico", "application/x-ico; charset=UTF-8" },
	{ ".iff", "application/x-iff; charset=UTF-8" },
	{ ".ig4", "application/x-g4; charset=UTF-8" },
	{ ".igs", "application/x-igs; charset=UTF-8" },
	{ ".iii", "application/x-iphone; charset=UTF-8" },
	{ ".img", "application/x-img; charset=UTF-8" },
	{ ".ins", "application/x-internet-signup; charset=UTF-8" },
	{ ".isp", "application/x-internet-signup; charset=UTF-8" },
	{ ".IVF", "video/x-ivf; charset=UTF-8" },
	{ ".java", "java/*; charset=UTF-8" },
	{ ".jfif", "image/jpeg; charset=UTF-8" },
	{ ".jpe", "image/jpeg; charset=UTF-8" },
	{ ".jpe", "application/x-jpe; charset=UTF-8" },
	{ ".jpeg", "image/jpeg; charset=UTF-8" },
	{ ".jpg", "image/jpeg; charset=UTF-8" },
	{ ".jpg", "application/x-jpg; charset=UTF-8" },
	{ ".js", "application/x-javascript; charset=UTF-8" },
	{ ".jsp", "text/html; charset=UTF-8" },
	{ ".la1", "audio/x-liquid-file; charset=UTF-8" },
	{ ".lar", "application/x-laplayer-reg; charset=UTF-8" },
	{ ".latex", "application/x-latex; charset=UTF-8" },
	{ ".lavs", "audio/x-liquid-secure; charset=UTF-8" },
	{ ".lbm", "application/x-lbm; charset=UTF-8" },
	{ ".lmsff", "audio/x-la-lms; charset=UTF-8" },
	{ ".ls", "application/x-javascript; charset=UTF-8" },
	{ ".ltr", "application/x-ltr; charset=UTF-8" },
	{ ".m1v", "video/x-mpeg; charset=UTF-8" },
	{ ".m2v", "video/x-mpeg; charset=UTF-8" },
	{ ".m3u", "audio/mpegurl; charset=UTF-8" },
	{ ".m4e", "video/mpeg4; charset=UTF-8" },
	{ ".mac", "application/x-mac; charset=UTF-8" },
	{ ".man", "application/x-troff-man; charset=UTF-8" },
	{ ".math", "text/xml; charset=UTF-8" },
	{ ".mdb", "application/msaccess; charset=UTF-8" },
	{ ".mdb", "application/x-mdb; charset=UTF-8" },
	{ ".mfp", "application/x-shockwave-flash; charset=UTF-8" },
	{ ".mht", "message/rfc822; charset=UTF-8" },
	{ ".mhtml", "message/rfc822; charset=UTF-8" },
	{ ".mi", "application/x-mi; charset=UTF-8" },
	{ ".mid", "audio/mid; charset=UTF-8" },
	{ ".midi", "audio/mid; charset=UTF-8" },
	{ ".mil", "application/x-mil; charset=UTF-8" },
	{ ".mml", "text/xml; charset=UTF-8" },
	{ ".mnd", "audio/x-musicnet-download; charset=UTF-8" },
	{ ".mns", "audio/x-musicnet-stream; charset=UTF-8" },
	{ ".mocha", "application/x-javascript; charset=UTF-8" },
	{ ".movie", "video/x-sgi-movie; charset=UTF-8" },
	{ ".mp1", "audio/mp; charset=UTF-8" },
	{ ".mp2", "audio/mp2; charset=UTF-8" },
	{ ".mp2v", "video/mpeg; charset=UTF-8" },
	{ ".mp3", "audio/mp3; charset=UTF-8" },
	{ ".mp4", "video/mpeg4; charset=UTF-8" },
	{ ".mpa", "video/x-mpg; charset=UTF-8" },
	{ ".mpd", "application/vnd.ms-project; charset=UTF-8" },
	{ ".mpe", "video/x-mpeg; charset=UTF-8" },
	{ ".mpeg", "video/mpg; charset=UTF-8" },
	{ ".mpg", "video/mpg; charset=UTF-8" },
	{ ".mpga", "audio/rn-mpeg; charset=UTF-8" },
	{ ".mpp", "application/vnd.ms-project; charset=UTF-8" },
	{ ".mps", "video/x-mpeg; charset=UTF-8" },
	{ ".mpt", "application/vnd.ms-project; charset=UTF-8" },
	{ ".mpv", "video/mpg; charset=UTF-8" },
	{ ".mpv2", "video/mpeg; charset=UTF-8" },
	{ ".mpw", "application/vnd.ms-project; charset=UTF-8" },
	{ ".mpx", "application/vnd.ms-project; charset=UTF-8" },
	{ ".mtx", "text/xml; charset=UTF-8" },
	{ ".mxp", "application/x-mmxp; charset=UTF-8" },
	{ ".net", "image/pnetvue; charset=UTF-8" },
	{ ".nrf", "application/x-nrf; charset=UTF-8" },
	{ ".nws", "message/rfc822; charset=UTF-8" },
	{ ".odc", "text/x-ms-odc; charset=UTF-8" },
	{ ".out", "application/x-out; charset=UTF-8" },
	{ ".p10", "application/pkcs10; charset=UTF-8" },
	{ ".p12", "application/x-pkcs12; charset=UTF-8" },
	{ ".p7b", "application/x-pkcs7-certificates; charset=UTF-8" },
	{ ".p7c", "application/pkcs7-mime; charset=UTF-8" },
	{ ".p7m", "application/pkcs7-mime; charset=UTF-8" },
	{ ".p7r", "application/x-pkcs7-certreqresp; charset=UTF-8" },
	{ ".p7s", "application/pkcs7-signature; charset=UTF-8" },
	{ ".pc5", "application/x-pc5; charset=UTF-8" },
	{ ".pci", "application/x-pci; charset=UTF-8" },
	{ ".pcl", "application/x-pcl; charset=UTF-8" },
	{ ".pcx", "application/x-pcx; charset=UTF-8" },
	{ ".pdf", "application/pdf; charset=UTF-8" },
	{ ".pdf", "application/pdf; charset=UTF-8" },
	{ ".pdx", "application/vnd.adobe.pdx; charset=UTF-8" },
	{ ".pfx", "application/x-pkcs12; charset=UTF-8" },
	{ ".pgl", "application/x-pgl; charset=UTF-8" },
	{ ".pic", "application/x-pic; charset=UTF-8" },
	{ ".pko", "application/vnd.ms-pki.pko; charset=UTF-8" },
	{ ".pl", "application/x-perl; charset=UTF-8" },
	{ ".plg", "text/html; charset=UTF-8" },
	{ ".pls", "audio/scpls; charset=UTF-8" },
	{ ".plt", "application/x-plt; charset=UTF-8" },
	{ ".png", "image/png; charset=UTF-8" },
	{ ".png", "application/x-png; charset=UTF-8" },
	{ ".pot", "application/vnd.ms-powerpoint; charset=UTF-8" },
	{ ".ppa", "application/vnd.ms-powerpoint; charset=UTF-8" },
	{ ".ppm", "application/x-ppm; charset=UTF-8" },
	{ ".pps", "application/vnd.ms-powerpoint; charset=UTF-8" },
	{ ".ppt", "application/vnd.ms-powerpoint; charset=UTF-8" },
	{ ".ppt", "application/x-ppt; charset=UTF-8" },
	{ ".pr", "application/x-pr; charset=UTF-8" },
	{ ".prf", "application/pics-rules; charset=UTF-8" },
	{ ".prn", "application/x-prn; charset=UTF-8" },
	{ ".prt", "application/x-prt; charset=UTF-8" },
	{ ".ps", "application/x-ps; charset=UTF-8" },
	{ ".ps", "application/postscript; charset=UTF-8" },
	{ ".ptn", "application/x-ptn; charset=UTF-8" },
	{ ".pwz", "application/vnd.ms-powerpoint; charset=UTF-8" },
	{ ".r3t", "text/vnd.rn-realtext3d; charset=UTF-8" },
	{ ".ra", "audio/vnd.rn-realaudio; charset=UTF-8" },
	{ ".ram", "audio/x-pn-realaudio; charset=UTF-8" },
	{ ".ras", "application/x-ras; charset=UTF-8" },
	{ ".rat", "application/rat-file; charset=UTF-8" },
	{ ".rdf", "text/xml; charset=UTF-8" },
	{ ".rec", "application/vnd.rn-recording; charset=UTF-8" },
	{ ".red", "application/x-red; charset=UTF-8" },
	{ ".rgb", "application/x-rgb; charset=UTF-8" },
	{ ".rjs", "application/vnd.rn-realsystem-rjs; charset=UTF-8" },
	{ ".rjt", "application/vnd.rn-realsystem-rjt; charset=UTF-8" },
	{ ".rlc", "application/x-rlc; charset=UTF-8" },
	{ ".rle", "application/x-rle; charset=UTF-8" },
	{ ".rm", "application/vnd.rn-realmedia; charset=UTF-8" },
	{ ".rmf", "application/vnd.adobe.rmf; charset=UTF-8" },
	{ ".rmi", "audio/mid; charset=UTF-8" },
	{ ".rmj", "application/vnd.rn-realsystem-rmj; charset=UTF-8" },
	{ ".rmm", "audio/x-pn-realaudio; charset=UTF-8" },
	{ ".rmp", "application/vnd.rn-rn_music_package; charset=UTF-8" },
	{ ".rms", "application/vnd.rn-realmedia-secure; charset=UTF-8" },
	{ ".rmvb", "application/vnd.rn-realmedia-vbr; charset=UTF-8" },
	{ ".rmx", "application/vnd.rn-realsystem-rmx; charset=UTF-8" },
	{ ".rnx", "application/vnd.rn-realplayer; charset=UTF-8" },
	{ ".rp", "image/vnd.rn-realpix; charset=UTF-8" },
	{ ".rpm", "audio/x-pn-realaudio-; charset=UTF-8" },
	{ ".rsml", "application/vnd.rn-rsml; charset=UTF-8" },
	{ ".rt", "text/vnd.rn-realtext; charset=UTF-8" },
	{ ".rtf", "application/msword; charset=UTF-8" },
	{ ".rtf", "application/x-rtf; charset=UTF-8" },
	{ ".rv", "video/vnd.rn-realvideo; charset=UTF-8" },
	{ ".sam", "application/x-sam; charset=UTF-8" },
	{ ".sat", "application/x-sat; charset=UTF-8" },
	{ ".sdp", "application/sdp; charset=UTF-8" },
	{ ".sdw", "application/x-sdw; charset=UTF-8" },
	{ ".sit", "application/x-stuffit; charset=UTF-8" },
	{ ".slb", "application/x-slb; charset=UTF-8" },
	{ ".sld", "application/x-sld; charset=UTF-8" },
	{ ".slk", "drawing/x-slk; charset=UTF-8" },
	{ ".smi", "application/smil; charset=UTF-8" },
	{ ".smil", "application/smil; charset=UTF-8" },
	{ ".smk", "application/x-smk; charset=UTF-8" },
	{ ".snd", "audio/basic; charset=UTF-8" },
	{ ".sol", "text/plain; charset=UTF-8" },
	{ ".sor", "text/plain; charset=UTF-8" },
	{ ".spc", "application/x-pkcs7-certificates; charset=UTF-8" },
	{ ".spl", "application/futuresplash; charset=UTF-8" },
	{ ".spp", "text/xml; charset=UTF-8" },
	{ ".ssm", "application/streamingmedia; charset=UTF-8" },
	{ ".sst", "application/vnd.ms-pki.certstore; charset=UTF-8" },
	{ ".stl", "application/vnd.ms-pki.stl; charset=UTF-8" },
	{ ".stm", "text/html; charset=UTF-8" },
	{ ".sty", "application/x-sty; charset=UTF-8" },
	{ ".svg", "text/xml; charset=UTF-8" },
	{ ".swf", "application/x-shockwave-flash; charset=UTF-8" },
	{ ".tdf", "application/x-tdf; charset=UTF-8" },
	{ ".tg4", "application/x-tg4; charset=UTF-8" },
	{ ".tga", "application/x-tga; charset=UTF-8" },
	{ ".tif", "image/tiff; charset=UTF-8" },
	{ ".tif", "application/x-tif; charset=UTF-8" },
	{ ".tiff", "image/tiff; charset=UTF-8" },
	{ ".tld", "text/xml; charset=UTF-8" },
	{ ".top", "drawing/x-top; charset=UTF-8" },
	{ ".torrent", "application/x-bittorrent; charset=UTF-8" },
	{ ".tsd", "text/xml; charset=UTF-8" },
	{ ".txt", "text/plain; charset=UTF-8" },
	{ ".uin", "application/x-icq; charset=UTF-8" },
	{ ".uls", "text/iuls; charset=UTF-8" },
	{ ".vcf", "text/x-vcard; charset=UTF-8" },
	{ ".vda", "application/x-vda; charset=UTF-8" },
	{ ".vdx", "application/vnd.visio; charset=UTF-8" },
	{ ".vml", "text/xml; charset=UTF-8" },
	{ ".vpg", "application/x-vpeg005; charset=UTF-8" },
	{ ".vsd", "application/vnd.visio; charset=UTF-8" },
	{ ".vsd", "application/x-vsd; charset=UTF-8" },
	{ ".vss", "application/vnd.visio; charset=UTF-8" },
	{ ".vst", "application/vnd.visio; charset=UTF-8" },
	{ ".vst", "application/x-vst; charset=UTF-8" },
	{ ".vsw", "application/vnd.visio; charset=UTF-8" },
	{ ".vsx", "application/vnd.visio; charset=UTF-8" },
	{ ".vtx", "application/vnd.visio; charset=UTF-8" },
	{ ".vxml", "text/xml; charset=UTF-8" },
	{ ".wav", "audio/wav; charset=UTF-8" },
	{ ".wax", "audio/x-ms-wax; charset=UTF-8" },
	{ ".wb1", "application/x-wb1; charset=UTF-8" },
	{ ".wb2", "application/x-wb2; charset=UTF-8" },
	{ ".wb3", "application/x-wb3; charset=UTF-8" },
	{ ".wbmp", "image/vnd.wap.wbmp; charset=UTF-8" },
	{ ".wiz", "application/msword; charset=UTF-8" },
	{ ".wk3", "application/x-; charset=UTF-8" },
	{ ".wk4", "application/x-wk4; charset=UTF-8" },
	{ ".wkq", "application/x-wkq; charset=UTF-8" },
	{ ".wks", "application/x-wks; charset=UTF-8" },
	{ ".wm", "video/x-ms-wm; charset=UTF-8" },
	{ ".wma", "audio/x-ms-wma; charset=UTF-8" },
	{ ".wmd", "application/x-ms-wmd; charset=UTF-8" },
	{ ".wmf", "application/x-wmf; charset=UTF-8" },
	{ ".wml", "text/vnd.wap.wml; charset=UTF-8" },
	{ ".wmv", "video/x-ms-wmv; charset=UTF-8" },
	{ ".wmx", "video/x-ms-wmx; charset=UTF-8" },
	{ ".wmz", "application/x-ms-wmz; charset=UTF-8" },
	{ ".wp6", "application/x-wp6; charset=UTF-8" },
	{ ".wpd", "application/x-wpd; charset=UTF-8" },
	{ ".wpg", "application/x-wpg; charset=UTF-8" },
	{ ".wpl", "application/vnd.ms-wpl; charset=UTF-8" },
	{ ".wq1", "application/x-wq1; charset=UTF-8" },
	{ ".wr1", "application/x-wr1; charset=UTF-8" },
	{ ".wri", "application/x-wri; charset=UTF-8" },
	{ ".wrk", "application/x-wrk; charset=UTF-8" },
	{ ".ws", "application/x-ws; charset=UTF-8" },
	{ ".ws2", "application/x-ws; charset=UTF-8" },
	{ ".wsc", "text/scriptlet; charset=UTF-8" },
	{ ".wsdl", "text/xml; charset=UTF-8" },
	{ ".wvx", "video/x-ms-wvx; charset=UTF-8" },
	{ ".xdp", "application/vnd.adobe.xdp; charset=UTF-8" },
	{ ".xdr", "text/xml; charset=UTF-8" },
	{ ".xfd", "application/vnd.adobe.xfd; charset=UTF-8" },
	{ ".xfdf", "application/vnd.adobe.xfdf; charset=UTF-8" },
	{ ".xhtml", "text/html; charset=UTF-8" },
	{ ".xls", "application/vnd.ms-excel; charset=UTF-8" },
	{ ".xls", "application/x-xls; charset=UTF-8" },
	{ ".xlw", "application/x-xlw; charset=UTF-8" },
	{ ".xml", "text/xml; charset=UTF-8" },
	{ ".xpl", "audio/scpls; charset=UTF-8" },
	{ ".xq", "text/xml; charset=UTF-8" },
	{ ".xql", "text/xml; charset=UTF-8" },
	{ ".xquery", "text/xml; charset=UTF-8" },
	{ ".xsd", "text/xml; charset=UTF-8" },
	{ ".xsl", "text/xml; charset=UTF-8" },
	{ ".xslt", "text/xml; charset=UTF-8" },
	{ ".xwd", "application/x-xwd; charset=UTF-8" },
	{ ".x_b", "application/x-x_b; charset=UTF-8" },
	{ ".sis", "application/vnd.symbian.install; charset=UTF-8" },
	{ ".sisx", "application/vnd.symbian.install; charset=UTF-8" },
	{ ".x_t", "application/x-x_t; charset=UTF-8" },
	{ ".ipa", "application/vnd.iphone; charset=UTF-8" },
	{ ".apk", "application/vnd.android.package-archive; charset=UTF-8" },
	{ ".xap", "application/x-silverlight-app; charset=UTF-8" },
	{ NULL, NULL }
};

const static char *parseContentType(const char *path)
{
	const char *extension;
	const struct HttpEntry *entry;
	extension = strrchr(path, '.');

	if (!extension || strchr(extension, '/'))
	{
		goto DEFAULT_ENTRY;
	}

	for (entry = &g_httpEntryTable[0]; entry->_extension; ++entry)
	{
		if (!evutil_ascii_strcasecmp(entry->_extension, extension))
		{
			return entry->_contentType;
		}
	}

DEFAULT_ENTRY:

	return "text/html; charset=UTF-8";
}
