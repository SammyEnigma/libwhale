#include "whale/util/hoptions.h"
#include "whale/basic/hos.h"
#include "whale/util/hstring.h"

#ifdef __GNUC__
#include <getopt.h>
#else
// Standard GNU options
#define	no_argument			0	/*Argument Switch Only*/
#define required_argument	1	/*Argument Required*/
#define optional_argument	2	/*Argument Optional*/	
enum ENUM_ORDERING { REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER };

int optind = 1;
int opterr = 1;
int optopt = '?';

// Ansi
struct option
{
	const char* name;
	int has_arg;
	int *flag;
	int val;
};

//
//
//		Ansi structures and functions follow
// 
//

static struct _getopt_data
{
	int optind;
	int opterr;
	int optopt;
	char *optarg;
	int __initialized;
	char *__nextchar;
	enum ENUM_ORDERING __ordering;
	int __posixly_correct;
	int __first_nonopt;
	int __last_nonopt;
} getopt_data;

char *optarg;

static void exchange(char **argv, struct _getopt_data *d)
{
	int bottom = d->__first_nonopt;
	int middle = d->__last_nonopt;
	int top = d->optind;
	char *tem;
	while (top > middle && middle > bottom)
	{
		if (top - middle > middle - bottom)
		{
			int len = middle - bottom;
			register int i;
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[top - (middle - bottom) + i];
				argv[top - (middle - bottom) + i] = tem;
			}
			top -= len;
		}
		else
		{
			int len = top - middle;
			register int i;
			for (i = 0; i < len; i++)
			{
				tem = argv[bottom + i];
				argv[bottom + i] = argv[middle + i];
				argv[middle + i] = tem;
			}
			bottom += len;
		}
	}
	d->__first_nonopt += (d->optind - d->__last_nonopt);
	d->__last_nonopt = d->optind;
}
static const char *_getopt_initialize (const char *optstring, struct _getopt_data *d, int posixly_correct)
{
	d->__first_nonopt = d->__last_nonopt = d->optind;
	d->__nextchar = NULL;
	d->__posixly_correct = posixly_correct | !!getenv("POSIXLY_CORRECT");
	if (optstring[0] == '-')
	{
		d->__ordering = RETURN_IN_ORDER;
		++optstring;
	}
	else if (optstring[0] == '+')
	{
		d->__ordering = REQUIRE_ORDER;
		++optstring;
	}
	else if (d->__posixly_correct)
		d->__ordering = REQUIRE_ORDER;
	else
		d->__ordering = PERMUTE;
	return optstring;
}
int _getopt_internal_r (int argc, char *const *argv, const char *optstring, const struct option *longopts, int *longind, int long_only, struct _getopt_data *d, int posixly_correct)
{
	int print_errors = d->opterr;
	if (argc < 1)
		return -1;
	d->optarg = NULL;
	if (d->optind == 0 || !d->__initialized)
	{
		if (d->optind == 0)
			d->optind = 1;
		optstring = _getopt_initialize (optstring, d, posixly_correct);
		d->__initialized = 1;
	}
	else if (optstring[0] == '-' || optstring[0] == '+')
		optstring++;
	if (optstring[0] == ':')
		print_errors = 0;
	if (d->__nextchar == NULL || *d->__nextchar == '\0')
	{
		if (d->__last_nonopt > d->optind)
			d->__last_nonopt = d->optind;
		if (d->__first_nonopt > d->optind)
			d->__first_nonopt = d->optind;
		if (d->__ordering == PERMUTE)
		{
			if (d->__first_nonopt != d->__last_nonopt && d->__last_nonopt != d->optind)
				exchange ((char **) argv, d);
			else if (d->__last_nonopt != d->optind)
				d->__first_nonopt = d->optind;
			while (d->optind < argc && (argv[d->optind][0] != '-' || argv[d->optind][1] == '\0'))
				d->optind++;
			d->__last_nonopt = d->optind;
		}
		if (d->optind != argc && !strcmp(argv[d->optind], "--"))
		{
			d->optind++;
			if (d->__first_nonopt != d->__last_nonopt && d->__last_nonopt != d->optind)
				exchange((char **) argv, d);
			else if (d->__first_nonopt == d->__last_nonopt)
				d->__first_nonopt = d->optind;
			d->__last_nonopt = argc;
			d->optind = argc;
		}
		if (d->optind == argc)
		{
			if (d->__first_nonopt != d->__last_nonopt)
				d->optind = d->__first_nonopt;
			return -1;
		}
		if ((argv[d->optind][0] != '-' || argv[d->optind][1] == '\0'))
		{
			if (d->__ordering == REQUIRE_ORDER)
				return -1;
			d->optarg = argv[d->optind++];
			return 1;
		}
		d->__nextchar = (argv[d->optind] + 1 + (longopts != NULL && argv[d->optind][1] == '-'));
	}
	if (longopts != NULL && (argv[d->optind][1] == '-' || (long_only && (argv[d->optind][2] || !strchr(optstring, argv[d->optind][1])))))
	{
		char *nameend;
		unsigned int namelen;
		const struct option *p;
		const struct option *pfound = NULL;
		struct option_list
		{
			const struct option *p;
			struct option_list *next;
		} *ambig_list = NULL;
		int exact = 0;
		int indfound = -1;
		int option_index;
		for (nameend = d->__nextchar; *nameend && *nameend != '='; nameend++);
		namelen = (unsigned int)(nameend - d->__nextchar);
		for (p = longopts, option_index = 0; p->name; p++, option_index++)
			if (!strncmp(p->name, d->__nextchar, namelen))
			{
				if (namelen == (unsigned int)strlen(p->name))
				{
					pfound = p;
					indfound = option_index;
					exact = 1;
					break;
				}
				else if (pfound == NULL)
				{
					pfound = p;
					indfound = option_index;
				}
				else if (long_only || pfound->has_arg != p->has_arg || pfound->flag != p->flag || pfound->val != p->val)
				{
					struct option_list *newp = (struct option_list*)alloca(sizeof(*newp));
					newp->p = p;
					newp->next = ambig_list;
					ambig_list = newp;
				}
			}
			if (ambig_list != NULL && !exact)
			{
				if (print_errors)
				{
					struct option_list first;
					first.p = pfound;
					first.next = ambig_list;
					ambig_list = &first;
					fprintf (stderr, "%s: option '%s' is ambiguous; possibilities:", argv[0], argv[d->optind]);
					do
					{
						fprintf (stderr, " '--%s'", ambig_list->p->name);
						ambig_list = ambig_list->next;
					}
					while (ambig_list != NULL);
					fputc ('\n', stderr);
				}
				d->__nextchar += strlen(d->__nextchar);
				d->optind++;
				d->optopt = 0;
				return '?';
			}
			if (pfound != NULL)
			{
				option_index = indfound;
				d->optind++;
				if (*nameend)
				{
					if (pfound->has_arg)
						d->optarg = nameend + 1;
					else
					{
						if (print_errors)
						{
							if (argv[d->optind - 1][1] == '-')
							{
								fprintf(stderr, "%s: option '--%s' doesn't allow an argument\n",argv[0], pfound->name);
							}
							else
							{
								fprintf(stderr, "%s: option '%c%s' doesn't allow an argument\n",argv[0], argv[d->optind - 1][0],pfound->name);
							}
						}
						d->__nextchar += strlen(d->__nextchar);
						d->optopt = pfound->val;
						return '?';
					}
				}
				else if (pfound->has_arg == 1)
				{
					if (d->optind < argc)
						d->optarg = argv[d->optind++];
					else
					{
						if (print_errors)
						{
							fprintf(stderr,"%s: option '--%s' requires an argument\n",argv[0], pfound->name);
						}
						d->__nextchar += strlen(d->__nextchar);
						d->optopt = pfound->val;
						return optstring[0] == ':' ? ':' : '?';
					}
				}
				d->__nextchar += strlen(d->__nextchar);
				if (longind != NULL)
					*longind = option_index;
				if (pfound->flag)
				{
					*(pfound->flag) = pfound->val;
					return 0;
				}
				return pfound->val;
			}
			if (!long_only || argv[d->optind][1] == '-' || strchr(optstring, *d->__nextchar) == NULL)
			{
				if (print_errors)
				{
					if (argv[d->optind][1] == '-')
					{
						fprintf(stderr, "%s: unrecognized option '--%s'\n",argv[0], d->__nextchar);
					}
					else
					{
						fprintf(stderr, "%s: unrecognized option '%c%s'\n",argv[0], argv[d->optind][0], d->__nextchar);
					}
				}
				d->__nextchar = (char *)"";
				d->optind++;
				d->optopt = 0;
				return '?';
			}
	}
	{
		char c = *d->__nextchar++;
		char *temp = (char*)strchr(optstring, c);
		if (*d->__nextchar == '\0')
			++d->optind;
		if (temp == NULL || c == ':' || c == ';')
		{
			if (print_errors)
			{
				fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
			}
			d->optopt = c;
			return '?';
		}
		if (temp[0] == 'W' && temp[1] == ';')
		{
			char *nameend;
			const struct option *p;
			const struct option *pfound = NULL;
			int exact = 0;
			int ambig = 0;
			int indfound = 0;
			int option_index;
			if (longopts == NULL)
				goto no_longs;
			if (*d->__nextchar != '\0')
			{
				d->optarg = d->__nextchar;
				d->optind++;
			}
			else if (d->optind == argc)
			{
				if (print_errors)
				{
					fprintf(stderr,"%s: option requires an argument -- '%c'\n",argv[0], c);
				}
				d->optopt = c;
				if (optstring[0] == ':')
					c = ':';
				else
					c = '?';
				return c;
			}
			else
				d->optarg = argv[d->optind++];
			for (d->__nextchar = nameend = d->optarg; *nameend && *nameend != '='; nameend++);
			for (p = longopts, option_index = 0; p->name; p++, option_index++)
				if (!strncmp(p->name, d->__nextchar, nameend - d->__nextchar))
				{
					if ((unsigned int) (nameend - d->__nextchar) == strlen(p->name))
					{
						pfound = p;
						indfound = option_index;
						exact = 1;
						break;
					}
					else if (pfound == NULL)
					{
						pfound = p;
						indfound = option_index;
					}
					else if (long_only || pfound->has_arg != p->has_arg || pfound->flag != p->flag || pfound->val != p->val)
						ambig = 1;
				}
				if (ambig && !exact)
				{
					if (print_errors)
					{
						fprintf(stderr, "%s: option '-W %s' is ambiguous\n",argv[0], d->optarg);
					}
					d->__nextchar += strlen(d->__nextchar);
					d->optind++;
					return '?';
				}
				if (pfound != NULL)
				{
					option_index = indfound;
					if (*nameend)
					{
						if (pfound->has_arg)
							d->optarg = nameend + 1;
						else
						{
							if (print_errors)
							{
								fprintf(stderr, "%s: option '-W %s' doesn't allow an argument\n",argv[0], pfound->name);
							}
							d->__nextchar += strlen(d->__nextchar);
							return '?';
						}
					}
					else if (pfound->has_arg == 1)
					{
						if (d->optind < argc)
							d->optarg = argv[d->optind++];
						else
						{
							if (print_errors)
							{
								fprintf(stderr, "%s: option '-W %s' requires an argument\n",argv[0], pfound->name);
							}
							d->__nextchar += strlen(d->__nextchar);
							return optstring[0] == ':' ? ':' : '?';
						}
					}
					else
						d->optarg = NULL;
					d->__nextchar += strlen(d->__nextchar);
					if (longind != NULL)
						*longind = option_index;
					if (pfound->flag)
					{
						*(pfound->flag) = pfound->val;
						return 0;
					}
					return pfound->val;
				}
no_longs:
				d->__nextchar = NULL;
				return 'W';
		}
		if (temp[1] == ':')
		{
			if (temp[2] == ':')
			{
				if (*d->__nextchar != '\0')
				{
					d->optarg = d->__nextchar;
					d->optind++;
				}
				else
					d->optarg = NULL;
				d->__nextchar = NULL;
			}
			else
			{
				if (*d->__nextchar != '\0')
				{
					d->optarg = d->__nextchar;
					d->optind++;
				}
				else if (d->optind == argc)
				{
					if (print_errors)
					{
						fprintf(stderr,"%s: option requires an argument -- '%c'\n",argv[0], c);
					}
					d->optopt = c;
					if (optstring[0] == ':')
						c = ':';
					else
						c = '?';
				}
				else
					d->optarg = argv[d->optind++];
				d->__nextchar = NULL;
			}
		}
		return c;
	}
}
int _getopt_internal (int argc, char *const *argv, const char *optstring, const struct option *longopts, int *longind, int long_only, int posixly_correct)
{
	int result;
	getopt_data.optind = optind;
	getopt_data.opterr = opterr;
	result = _getopt_internal_r (argc, argv, optstring, longopts,longind, long_only, &getopt_data,posixly_correct);
	optind = getopt_data.optind;
	optarg = getopt_data.optarg;
	optopt = getopt_data.optopt;
	return result;
}
int getopt (int argc, char *const *argv, const char *optstring) 
{
	return _getopt_internal (argc, argv, optstring, (const struct option *) 0, (int *) 0, 0, 0);
}
int getopt_long (int argc, char *const *argv, const char *options, const struct option *long_options, int *opt_index) 
{
	return _getopt_internal (argc, argv, options, long_options, opt_index, 0, 0);
}
int getopt_long_only (int argc, char *const *argv, const char *options, const struct option *long_options, int *opt_index) 
{
	return _getopt_internal (argc, argv, options, long_options, opt_index, 1, 0);
}
int _getopt_long_r (int argc, char *const *argv, const char *options, const struct option *long_options, int *opt_index, struct _getopt_data *d)
{
	return _getopt_internal_r (argc, argv, options, long_options, opt_index,0, d, 0);
}
int _getopt_long_only_r (int argc, char *const *argv, const char *options, const struct option *long_options, int *opt_index, struct _getopt_data *d)
{
	return _getopt_internal_r (argc, argv, options, long_options, opt_index, 1, d, 0);
}

#endif

#ifdef OS_WIN32
#define COL(x)  ""
#define GRAY    "" 
#endif // OS_WIN32

#ifdef OS_LINUX
#define COL(x)  "\033[;" #x "m"
#define GRAY    "\033[0m" 
#endif // OS_LINUX

#define RED     COL(31)  
#define GREEN   COL(32)  
#define YELLOW  COL(33)  
#define BLUE    COL(34)  
#define MAGENTA COL(35)  
#define CYAN    COL(36)  
#define WHITE   COL(0)  


namespace Whale 
{
	namespace Util
	{

		static void fatal(const char *format, ...)
		{
			va_list ap;

			va_start(ap, format);
			fprintf(stderr, "%s: ", Whale::appName().c_str());
			vfprintf(stderr, format, ap);
			va_end(ap);
			fprintf(stderr, "\n");
			exit(EXIT_FAILURE);
		}

		HOptions::HOptions(const std::string &version)
		{
			version_ = version;
		}

		HOptions::~HOptions()
		{
		}

		void HOptions::insert(int key, const std::string &longopt, const std::string& desc, bool hasArg)
		{
			struct HOptionsItem item = {key, longopt, desc, hasArg};
			items_.push_back(item);
		}

		void HOptions::help() const
		{
			printf("Usage: %s [OPTION] ...\n", Whale::appName().c_str());
			printf("%s\n", brief_.empty() ? "Program description" : brief_.c_str());
			printf("\n");
			printf("  -h, --help\t\t\tDisplay this help and exit\n");
			printf("  -V, --version\t\t\tOutput version information and exit\n");
			printf("  -d, --daemon\t\t\tRun as daemon\n");
			printf("  -c, --config=FILE\t\tUsing FILE as configuration file\n");
			printf("  -v, --verbose\t\t\tVerbose mode, explain what is being done\n");
			printf("  -l, --log-level=LEVEL\t\tUsing log LEVEL [1-9]\n");
			printf("  -s, --log-sys\t\t\tLog to syslog\n");
			printf("  -f, --log-file=LOGFILE\tOutput log string to LOGFILE\n");
			for (auto it : items_)
			{
				printf("  %s\n", it._desc.c_str());
			}
		}

		void HOptions::version() const
		{
			printf("%s\n", version_.c_str());
		}

		bool HOptions::load(int argc, char* argv[])
		{
			std::vector<struct option> longOptions;
			std::string shortopts = "hVdc:vl:sf:";
			
			longOptions.push_back({"help",      no_argument,       NULL, 'h'});
			longOptions.push_back({"version",   no_argument,       NULL, 'V'});
			longOptions.push_back({"daemon",    no_argument,       NULL, 'd'});
			longOptions.push_back({"config",    required_argument, NULL, 'c'});
			longOptions.push_back({"verbose",   no_argument,       NULL, 'v'});
			longOptions.push_back({"log-level", required_argument, NULL, 'l'});
			longOptions.push_back({"log-sys",   no_argument,       NULL, 's'});
			longOptions.push_back({"log-file",  required_argument, NULL, 'f'});
			
			for (auto it : items_) {
				struct option optItem = {
					it._longopt.c_str(),
					(int)it._hasArg,
					NULL,
					it._key
				};
				longOptions.push_back(optItem);

				shortopts += it._hasArg ? Whale::Util::HString::format("%c:", it._key) : Whale::Util::HString::format("%c", it._key);
			}

			int opt = -1;
			int longind = 0;
			while ((opt = getopt_long(argc, argv, shortopts.c_str(),
									  longOptions.data(), &longind)) >= 0) {
				switch (opt) {
				case 'h':
					help();
					exit(EXIT_SUCCESS);
				case 'V':
					version();
					exit(EXIT_SUCCESS);
				case 'd':
				case 'v':
				case 's':
					options_[opt] = "";
					continue;
				case 'c':
				case 'l':
				case 'f':
					options_[opt] = optarg;
					continue;
				default:
					break;
				}

				auto it = std::find(items_.begin(), items_.end(), opt);

				if (it != items_.end()) {
					if (it->_hasArg && optarg) {
						options_[opt] = optarg;
					} else {
						options_[opt] = "";
					}
				} else {
					fatal("More info with: \"%s --help\"", Whale::appName().c_str());
				}
			}
			return true;			
		}

		bool HOptions::contain(int key) const
		{
			return options_.find(key) != options_.end();
		}

		std::string HOptions::readString(int key) const
		{
			auto it = options_.find(key);
			assert(it != options_.end());
			return it->second;
		}

		int HOptions::readInt32(int key) const
		{
			auto it = options_.find(key);
			assert(it != options_.end());
			return Whale::Util::HString::convert<int>(it->second);
		}

		void HOptions::setVersionString(const std::string &ver)
		{
			version_ = ver;
		}
		
		void HOptions::setBriefString(const std::string &brief)
		{
			brief_ = brief;
		}
	}
}
