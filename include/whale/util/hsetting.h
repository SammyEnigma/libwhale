/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
*
* File Name			:	hsetting.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HSIMPLE_INI_H
#define HSIMPLE_INI_H

#include <cstring>
#include <cstdlib>
#include <string>
#include <map>
#include <list>
#include <algorithm>
#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <memory>

#define SIMPLE_INI_SUPPORT_IOSTREAMS
#if defined(SIMPLE_INI_SUPPORT_IOSTREAMS) && !defined(_UNICODE)
# include <fstream>
#endif

#ifdef _MSC_VER
# pragma warning (push)
# pragma warning (disable: 4127 4503 4702 4786)
#endif

#ifdef SIMPLE_INI_SUPPORT_IOSTREAMS
# include <iostream>
#endif // SIMPLE_INI_SUPPORT_IOSTREAMS

enum SimpleIniResult {
	SIMPLE_INI_OK = 0,   //!< No error
	SIMPLE_INI_UPDATED = 1,   //!< An existing value was updated
	SIMPLE_INI_INSERTED = 2,   //!< A new value was inserted
	// note: test for any error with (retval < 0)
	SIMPLE_INI_FAIL = -1,   //!< Generic failure
	SIMPLE_INI_NOMEM = -2,   //!< Out of memory error
	SIMPLE_INI_FILE = -3    //!< File error (see errno for detail error)
};

#define SIMPLE_INI_UTF8_SIGNATURE     "\xEF\xBB\xBF"

#ifdef _WIN32
# define SIMPLE_INI_NEWLINE_A   "\r\n"
# define SIMPLE_INI_NEWLINE_W   L"\r\n"
#else // !_WIN32
# define SIMPLE_INI_NEWLINE_A   "\n"
# define SIMPLE_INI_NEWLINE_W   L"\n"
#endif // _WIN32

#if defined(SIMPLE_INI_CONVERT_ICU)
# include <unicode/ustring.h>
#endif

#if defined(_WIN32)
# define SIMPLE_INI_HAS_WIDE_FILE
# define SIMPLE_INI_WCHAR_T     wchar_t
#elif defined(SIMPLE_INI_CONVERT_ICU)
# define SIMPLE_INI_HAS_WIDE_FILE
# define SIMPLE_INI_WCHAR_T     UChar
#endif

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (unsigned int)0x0000FFFD
#define UNI_MAX_BMP (unsigned int)0x0000FFFF
#define UNI_MAX_UTF16 (unsigned int)0x0010FFFF
#define UNI_MAX_UTF32 (unsigned int)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (unsigned int)0x0010FFFF

typedef enum {
	CONVERT_OK, 		/* conversion successful */
	CONVERT_SOURCE_EXHAUSTED,	/* partial character in source, but hit end */
	CONVERT_TARGET_EXHAUSTED,	/* insuff. room in target for conversion */
	CONVERT_SOURCE_ILLEGAL		/* source sequence is illegal/malformed */
} ConvertResult;

typedef enum {
	CFLAGS_STRICT = 0,
	CFLAGS_LENIENT
} ConvertFlags;

ConvertResult convertUTF8toUTF16(
	const unsigned char** sourceStart, const unsigned char* sourceEnd,
	unsigned short** targetStart, unsigned short* targetEnd, ConvertFlags flags);

ConvertResult convertUTF16toUTF8(
	const unsigned short** sourceStart, const unsigned short* sourceEnd,
	unsigned char** targetStart, unsigned char* targetEnd, ConvertFlags flags);

ConvertResult convertUTF8toUTF32(
	const unsigned char** sourceStart, const unsigned char* sourceEnd,
	unsigned int** targetStart, unsigned int* targetEnd, ConvertFlags flags);

ConvertResult convertUTF32toUTF8(
	const unsigned int** sourceStart, const unsigned int* sourceEnd,
	unsigned char** targetStart, unsigned char* targetEnd, ConvertFlags flags);

ConvertResult convertUTF16toUTF32(
	const unsigned short** sourceStart, const unsigned short* sourceEnd,
	unsigned int** targetStart, unsigned int* targetEnd, ConvertFlags flags);

ConvertResult convertUTF32toUTF16(
	const unsigned int** sourceStart, const unsigned int* sourceEnd,
	unsigned short** targetStart, unsigned short* targetEnd, ConvertFlags flags);

unsigned char isLegalUTF8Sequence(const unsigned char *source, const unsigned char *sourceEnd);

template<class CharT, class StrlessT, class ConverterT>
class SimpleIniTempl {
public:
	struct Entry {
		const CharT * _item;
		const CharT * _comment;
		int           _order;

		Entry(const CharT * a_pszItem = NULL, int a_nOrder = 0)
			: _item(a_pszItem)
			, _comment(NULL)
			, _order(a_nOrder)
		{ }
		Entry(const CharT * a_pszItem, const CharT * a_pszComment, int a_nOrder)
			: _item(a_pszItem)
			, _comment(a_pszComment)
			, _order(a_nOrder)
		{ }
		Entry(const Entry & rhs) { operator=(rhs); }
		Entry & operator=(const Entry & rhs) {
			_item = rhs._item;
			_comment = rhs._comment;
			_order = rhs._order;
			return *this;
		}

#if defined(_MSC_VER) && _MSC_VER <= 1200
		/** STL of VC6 doesn't allow me to specify my own comparator for list::sort() */
		bool operator<(const Entry & rhs) const { return LoadOrder()(*this, rhs); }
		bool operator>(const Entry & rhs) const { return LoadOrder()(rhs, *this); }
#endif

		/** Strict less ordering by name of key only */
		struct KeyOrder : std::binary_function<Entry, Entry, bool> {
			bool operator()(const Entry & lhs, const Entry & rhs) const {
				const static StrlessT isLess = StrlessT();
				return isLess(lhs._item, rhs._item);
			}
		};

		/** Strict less ordering by order, and then name of key */
		struct LoadOrder : std::binary_function<Entry, Entry, bool> {
			bool operator()(const Entry & lhs, const Entry & rhs) const {
				if (lhs._order != rhs._order) {
					return lhs._order < rhs._order;
				}
				return KeyOrder()(lhs._item, rhs._item);
			}
		};
	};

	/** map keys to values */
	typedef std::multimap<Entry, const CharT *, typename Entry::KeyOrder> TKeyVal;

	/** map sections to key/value map */
	typedef std::map<Entry, TKeyVal, typename Entry::KeyOrder> TSection;

	/** set of dependent string pointers. Note that these pointers are dependent on memory owned by HSimpleIni.	*/
	typedef std::list<Entry> TNamesDepend;

	/** interface definition for the OutputWriter object to pass to Save() in order to output the INI file data. */
	class OutputWriter {
	public:
		OutputWriter() { }
		virtual ~OutputWriter() { }
		virtual void Write(const char * a_pBuf) = 0;
	private:
		OutputWriter(const OutputWriter &);             // disable
		OutputWriter & operator=(const OutputWriter &); // disable
	};

	/** OutputWriter class to write the INI data to a file */
	class FileWriter : public OutputWriter {
		FILE * m_file;
	public:
		FileWriter(FILE * a_file) : m_file(a_file) { }
		void Write(const char * a_pBuf) {
			fputs(a_pBuf, m_file);
		}
	private:
		FileWriter(const FileWriter &);             // disable
		FileWriter & operator=(const FileWriter &); // disable
	};

	/** OutputWriter class to write the INI data to a string */
	class StringWriter : public OutputWriter {
		std::string & m_string;
	public:
		StringWriter(std::string & a_string) : m_string(a_string) { }
		void Write(const char * a_pBuf) {
			m_string.append(a_pBuf);
		}
	private:
		StringWriter(const StringWriter &);             // disable
		StringWriter & operator=(const StringWriter &); // disable
	};

#ifdef SIMPLE_INI_SUPPORT_IOSTREAMS
	/** OutputWriter class to write the INI data to an ostream */
	class StreamWriter : public OutputWriter {
		std::ostream & m_ostream;
	public:
		StreamWriter(std::ostream & outStream) : m_ostream(outStream) { }
		void Write(const char * a_pBuf) {
			m_ostream << a_pBuf;
		}
	private:
		StreamWriter(const StreamWriter &);             // disable
		StreamWriter & operator=(const StreamWriter &); // disable
	};
#endif // SIMPLE_INI_SUPPORT_IOSTREAMS

	/** Characterset conversion utility class to convert strings to the	same format as is used for the storage.	*/
	class SimpleIniConverterImpl : private ConverterT {
	public:
		SimpleIniConverterImpl(bool isu8) : ConverterT(isu8) {
			_scratch.resize(1024);
		}
		SimpleIniConverterImpl(const SimpleIniConverterImpl & rhs) {
			operator=(rhs);
		}
		SimpleIniConverterImpl & operator=(const SimpleIniConverterImpl & rhs) {
			_scratch = rhs._scratch;
			return *this;
		}
		bool ConvertToStore(const CharT * a_pszString) {
			size_t uLen = ConverterT::SizeToStore(a_pszString);
			if (uLen == (size_t)(-1)) {
				return false;
			}
			while (uLen > _scratch.size()) {
				_scratch.resize(_scratch.size() * 2);
			}
			return ConverterT::ConvertToStore(
				a_pszString,
				const_cast<char*>(_scratch.data()),
				_scratch.size());
		}
		const char * data() { return _scratch.data(); }

	private:
		std::string _scratch;
	};

public:
	SimpleIniTempl(bool _utf8 = false, bool _multiKey = false, bool _multiLine = false);

	/** Destructor */
	~SimpleIniTempl();

	/** Deallocate all memory stored by this object */
	void Reset();

	/** Has any data been loaded */
	bool IsEmpty() const { return _section.empty(); }

	void SetUnicode(bool isu8 = true) {
		if (!_dataBuff) _storeIsUtf8 = isu8;
	}

	/** Get the storage format of the INI data. */
	bool IsUnicode() const {
		return _storeIsUtf8;
	}

	void SetMultiKey(bool allowMultiKey = true) {
		_allowMultiKey = allowMultiKey;
	}

	/** Get the storage format of the INI data. */
	bool IsMultiKey() const { return _allowMultiKey; }

	void SetMultiLine(bool allowMultiLine = true) {
		_allowMultiLine = allowMultiLine;
	}

	/** Query the status of multi-line data */
	bool IsMultiLine() const {
		return _allowMultiLine;
	}

	void SetSpaces(bool spaces = true) {
		_shouldSpaces = spaces;
	}

	/** Query the status of spaces output */
	bool UsingSpaces() const {
		return _shouldSpaces;
	}

	SimpleIniResult LoadFile(const char * filepath);

#ifdef SIMPLE_INI_HAS_WIDE_FILE
	SimpleIniResult LoadFile(const SIMPLE_INI_WCHAR_T * filepath);
#endif // SIMPLE_INI_HAS_WIDE_FILE

	SimpleIniResult LoadFile(FILE * fp);

#ifdef SIMPLE_INI_SUPPORT_IOSTREAMS
	/** Load INI file data from an istream.

	@param isStream    Stream to read from

	@return SimpleIniError    See error definitions
	*/
	SimpleIniResult LoadData(std::istream & isStream);
#endif // SIMPLE_INI_SUPPORT_IOSTREAMS

	SimpleIniResult LoadData(const std::string & dataBuff) {
		return LoadData(dataBuff.c_str(), dataBuff.size());
	}

	SimpleIniResult LoadData(const char* inbuff, size_t inbuffsize);

	SimpleIniResult SaveFile(const char * filepath, bool addSignature = true) const;

#ifdef SIMPLE_INI_HAS_WIDE_FILE
	SimpleIniResult SaveFile(const SIMPLE_INI_WCHAR_T *  filepath, bool addSignature = true) const;
#endif // _WIN32

	SimpleIniResult SaveFile(FILE * fp, bool addSignature = false) const;

	SimpleIniResult Save(OutputWriter &  outWriter, bool addSignature = false) const;

#ifdef SIMPLE_INI_SUPPORT_IOSTREAMS
	SimpleIniResult Save(std::ostream &  outStream, bool addSignature = false) const	{
		StreamWriter writer(outStream);
		return Save(writer, addSignature);
	}
#endif // SIMPLE_INI_SUPPORT_IOSTREAMS

	SimpleIniResult Save(std::string & inbuff, bool addSignature = false) const {
		StringWriter writer(inbuff);
		return Save(writer, addSignature);
	}

	void GetAllSections(TNamesDepend & names) const;
	bool GetAllKeys(const CharT * section, TNamesDepend &  names) const;
	bool GetAllValues(const CharT * section, const CharT * key, TNamesDepend &  values) const;

	int GetSectionSize(const CharT * section) const;
	const TKeyVal * GetSection(const CharT * section) const;
	const CharT * GetValue(const CharT * section, const CharT * key, const CharT * def = NULL, bool *multiple = NULL) const;
	long GetLongValue(const CharT * section, const CharT * key, long def = 0, bool * multiple = NULL) const;
	double GetDoubleValue(const CharT * section, const CharT * key, double def = 0, bool * multiple = NULL) const;
	bool GetBoolValue(const CharT * section, const CharT * key, bool def = false, bool * multiple = NULL) const;

	SimpleIniResult SetValue(const CharT * section, const CharT * key, const CharT * value, const CharT * comment = NULL, bool replace = false) {
		return AddEntry(section, key, value, comment, replace, true);
	}
	SimpleIniResult SetLongValue(const CharT * section, const CharT * key, long value, const CharT * comment = NULL, bool a_bUseHex = false, bool replace = false);
	SimpleIniResult SetDoubleValue(const CharT * section, const CharT * key, double value, const CharT * comment = NULL, bool replace = false);
	SimpleIniResult SetBoolValue(const CharT * section, const CharT * key, bool value, const CharT * comment = NULL, bool replace = false);

	bool Delete(const CharT * section, const CharT * key, bool removeEmpty = false);
	bool DeleteValue(const CharT * section, const CharT * key, const CharT * value, bool removeEmpty = false);

	SimpleIniConverterImpl GetConverter() const {
		return SimpleIniConverterImpl(_storeIsUtf8);
	}

private:
	SimpleIniTempl(const SimpleIniTempl &); // disabled
	SimpleIniTempl & operator=(const SimpleIniTempl &); // disabled

	SimpleIniResult FindFileComment(CharT *& inbuff, bool copyStrings);
	bool FindEntry(CharT *&  inbuff, const CharT *&  section, const CharT *&  key, const CharT *&  value, const CharT *&  comment) const;
	SimpleIniResult AddEntry(const CharT * section, const CharT * key, const CharT * value, const CharT * comment, bool replace, bool copyStrings);

	/** Is the supplied character a whitespace character? */
	inline bool IsSpace(CharT ch) const {
		return (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
	}

	/** Does the supplied character start a comment line? */
	inline bool IsComment(CharT ch) const {
		return (ch == ';' || ch == '#');
	}

	/** Skip over a newline character (or characters) for either DOS or UNIX */
	inline void SkipNewLine(CharT *& inbuff) const {
		inbuff += (*inbuff == '\r' && *(inbuff + 1) == '\n') ? 2 : 1;
	}

	/** Make a copy of the supplied string, replacing the original pointer */
	SimpleIniResult CopyString(const CharT *& text);

	/** Delete a string from the copied strings buffer if necessary */
	void DeleteString(const CharT * text);

	/** Internal use of our string comparison function */
	bool IsLess(const CharT * left, const CharT * right) const {
		const static StrlessT isLess = StrlessT();
		return isLess(left, right);
	}

	bool IsMultiLineTag(const CharT * inbuff) const;
	bool IsMultiLineData(const CharT * inbuff) const;
	bool LoadMultiLineText(CharT *& inbuff, const CharT *& value, const CharT * name, bool allowBlankComment = false) const;
	bool IsNewLineChar(CharT ch) const;

	bool OutputMultiLineText(OutputWriter &  outWriter, SimpleIniConverterImpl & converter, const CharT * text) const;

private:
	size_t _dataLen;
	CharT * _dataBuff;

	TSection _section;
	TNamesDepend _nameDepend;
	const CharT * _fileComment;

	/** Is the format of our datafile UTF-8 or MBCS? */
	bool _storeIsUtf8;

	/** Are multiple values permitted for the same key? */
	bool _allowMultiKey;

	/** Are data values permitted to span multiple lines? */
	bool _allowMultiLine;

	/** Should spaces be written out surrounding the equals sign? */
	bool _shouldSpaces;

	/** Next order value, used to ensure sections and keys are output in the same order that they are loaded/added. */
	int _orderNum;
};



// ---------------------------------------------------------------------------
//                              CONVERSION FUNCTIONS
// ---------------------------------------------------------------------------

// Defines the conversion classes for different libraries. Before including
// SimpleIni.h, set the converter that you wish you use by defining one of the
// following symbols.
//
//  SIMPLE_INI_CONVERT_GENERIC      Use the Unicode reference conversion library in
//                          the accompanying files ConvertUTF.h/c
//  SIMPLE_INI_CONVERT_ICU          Use the IBM ICU conversion library. Requires
//                          ICU headers on include path and icuuc.lib
//  SIMPLE_INI_CONVERT_WIN32        Use the Win32 API functions for conversion.

#if !defined(SIMPLE_INI_CONVERT_GENERIC) && !defined(SIMPLE_INI_CONVERT_WIN32) && !defined(SIMPLE_INI_CONVERT_ICU)
# ifdef _WIN32
#  define SIMPLE_INI_CONVERT_WIN32
# else
#  define SIMPLE_INI_CONVERT_GENERIC
# endif
#endif

/**
* Generic case-sensitive less than comparison. This class returns numerically
* ordered ASCII case-sensitive text for all possible sizes and types of
* SIMPLE_INI_CHAR.
*/
template<class CharT>
struct SimpleIniGenericCase {
	bool operator()(const CharT * pLeft, const CharT * pRight) const {
		long cmp;
		for (; *pLeft && *pRight; ++pLeft, ++pRight) {
			cmp = (long)*pLeft - (long)*pRight;
			if (cmp != 0) {
				return cmp < 0;
			}
		}
		return *pRight != 0;
	}
};

/**
* Generic ASCII case-insensitive less than comparison. This class returns
* numerically ordered ASCII case-insensitive text for all possible sizes
* and types of SIMPLE_INI_CHAR. It is not safe for MBCS text comparison where
* ASCII A-Z characters are used in the encoding of multi-byte characters.
*/
template<class CharT>
struct SimpleIniGenericNoCase {
	inline CharT locase(CharT ch) const {
		return (ch < 'A' || ch > 'Z') ? ch : (ch - 'A' + 'a');
	}
	bool operator()(const CharT * pLeft, const CharT * pRight) const {
		long cmp;
		for (; *pLeft && *pRight; ++pLeft, ++pRight) {
			cmp = (long)locase(*pLeft) - (long)locase(*pRight);
			if (cmp != 0) {
				return cmp < 0;
			}
		}
		return *pRight != 0;
	}
};

template<class CharT>
class SimpleIniConvertA {
	bool _storeIsUtf8;
protected:
	SimpleIniConvertA() { }
public:
	SimpleIniConvertA(bool isu8) : _storeIsUtf8(isu8) { }

	/* copy and assignment */
	SimpleIniConvertA(const SimpleIniConvertA & rhs) { operator=(rhs); }
	SimpleIniConvertA & operator=(const SimpleIniConvertA & rhs) {
		_storeIsUtf8 = rhs._storeIsUtf8;
		return *this;
	}

	size_t SizeFromStore(const char * inbuff,size_t inbuffsize) {
		(void)inbuff;
		assert(inbuffsize != (size_t)-1);

		// ASCII/MBCS/UTF-8 needs no conversion
		return inbuffsize;
	}

	bool ConvertFromStore(const char * inbuff, size_t inbuffsize, CharT * outBuff, size_t outBuffSize)	{
		// ASCII/MBCS/UTF-8 needs no conversion
		if (inbuffsize > outBuffSize) {
			return false;
		}
		memcpy(outBuff, inbuff, inbuffsize);
		return true;
	}

	size_t SizeToStore(const CharT * inbuff) {
		// ASCII/MBCS/UTF-8 needs no conversion
		return strlen((const char *)inbuff) + 1;
	}

	bool ConvertToStore(const CharT * inbuff, char * outBuff, size_t outBuffSize){
		size_t uInputLen = strlen((const char *)inbuff) + 1;
		if (uInputLen > outBuffSize) {
			return false;
		}

		memcpy(outBuff, inbuff, uInputLen);
		return true;
	}
};


#ifdef SIMPLE_INI_CONVERT_GENERIC

#define SimpleIniCase     SimpleIniGenericCase
#define SimpleIniNoCase   SimpleIniGenericNoCase

#include <wchar.h>

template<class CharT>
class SimpleIniConvertW
{
	bool _storeIsUtf8;

protected:
	SimpleIniConvertW() { 
	}

public:
	SimpleIniConvertW(bool isu8) : _storeIsUtf8(isu8) {
	}

	SimpleIniConvertW(const SimpleIniConvertW & rhs) { operator=(rhs); }
	SimpleIniConvertW & operator=(const SimpleIniConvertW & rhs) {
		_storeIsUtf8 = rhs._storeIsUtf8;
		return *this;
	}

	size_t SizeFromStore(
		const char *    inbuff,
		size_t          inbuffsize) {
		assert(inbuffsize != (size_t)-1);

		if (_storeIsUtf8) {
			return inbuffsize;
		}

#if defined(SIMPLE_INI_NO_MBSTOWCS_NULL) || (!defined(_MSC_VER) && !defined(_linux))
		(void)inbuff;
		return inbuffsize;
#else
		return mbstowcs(NULL, inbuff, inbuffsize);
#endif
	}

	bool ConvertFromStore(const char * inbuff, size_t inbuffsize,CharT * outBuff, size_t outBuffSize) {
		if (_storeIsUtf8) {
			ConvertResult retval;
			const unsigned char * pUtf8 = (const unsigned char *)inbuff;
			if (sizeof(wchar_t) == sizeof(unsigned int)) {
				unsigned int * pUtf32 = (unsigned int *)outBuff;
				retval = convertUTF8toUTF32(
					&pUtf8, pUtf8 + inbuffsize,
					&pUtf32, pUtf32 + outBuffSize,
					CFLAGS_LENIENT);
			}
			else if (sizeof(wchar_t) == sizeof(unsigned short)) {
				unsigned short * pUtf16 = (unsigned short *)outBuff;
				retval = convertUTF8toUTF16(
					&pUtf8, pUtf8 + inbuffsize,
					&pUtf16, pUtf16 + outBuffSize,
					CFLAGS_LENIENT);
			}
			return retval == CONVERT_OK;
		}

		// convert to wchar_t
		size_t retval = mbstowcs(outBuff,
			inbuff, outBuffSize);
		return retval != (size_t)(-1);
	}

	size_t SizeToStore(const CharT * inbuff) {
		if (_storeIsUtf8) {
			// worst case scenario for wchar_t to UTF-8 is 1 wchar_t -> 6 char
			size_t uLen = 0;
			while (inbuff[uLen]) {
				++uLen;
			}
			return (6 * uLen) + 1;
		}
		else {
			size_t uLen = wcstombs(NULL, inbuff, 0);
			if (uLen == (size_t)(-1)) {
				return uLen;
			}
			return uLen + 1; // include NULL terminator
		}
	}

	bool ConvertToStore(const CharT * inbuff, char * outBuff, size_t outBuffSize) {
		if (_storeIsUtf8) {
			// calc input string length (SIMPLE_INI_CHAR type and size independent)
			size_t uInputLen = 0;
			while (inbuff[uInputLen]) {
				++uInputLen;
			}
			++uInputLen; // include the NULL char

			ConvertResult retval;
			unsigned char * pUtf8 = (unsigned char *)outBuff;
			if (sizeof(wchar_t) == sizeof(unsigned int)) {
				const unsigned int * pUtf32 = (const unsigned int *)inbuff;
				retval = convertUTF32toUTF8(&pUtf32, pUtf32 + uInputLen,&pUtf8, pUtf8 + outBuffSize,CFLAGS_LENIENT);
			}
			else if (sizeof(wchar_t) == sizeof(unsigned short)) {
				const unsigned short * pUtf16 = (const unsigned short *)inbuff;
				retval = convertUTF16toUTF8(&pUtf16, pUtf16 + uInputLen,&pUtf8, pUtf8 + outBuffSize,CFLAGS_LENIENT);
			}
			return retval == CONVERT_OK;
		}
		else {
			size_t retval = wcstombs(outBuff, inbuff, outBuffSize);
			return retval != (size_t)-1;
		}
	}
};

#endif // SIMPLE_INI_CONVERT_GENERIC

#ifdef SIMPLE_INI_CONVERT_ICU

#define SimpleIniCase     SimpleIniGenericCase
#define SimpleIniNoCase   SimpleIniGenericNoCase

#include <unicode/ucnv.h>

template<class CharT>
class SimpleIniConvertW 
{
	const char * _encoding;
	UConverter * _converter;

protected:
	SimpleIniConvertW() : _encoding(NULL), _converter(NULL) { 
	}

public:
	SimpleIniConvertW(bool isu8) : _converter(NULL) {
		_encoding = isu8 ? "UTF-8" : NULL;
	}

	SimpleIniConvertW(const SimpleIniConvertW & rhs) { 
		operator=(rhs); 
	}

	SimpleIniConvertW & operator=(const SimpleIniConvertW & rhs) {
		_encoding = rhs._encoding;
		_converter = NULL;
		return *this;
	}

	~SimpleIniConvertW() { 
		if (_converter) ucnv_close(_converter); 
	}

	size_t SizeFromStore(const char * inbuff, size_t inbuffsize) {
		assert(inbuffsize != (size_t)-1);

		UErrorCode nError;

		if (!_converter) {
			nError = U_ZERO_ERROR;
			_converter = ucnv_open(_encoding, &nError);
			if (U_FAILURE(nError)) {
				return (size_t)-1;
			}
		}

		nError = U_ZERO_ERROR;
		int32_t nLen = ucnv_toUChars(_converter, NULL, 0, inbuff, (int32_t)inbuffsize, &nError);
		if (U_FAILURE(nError) && nError != U_BUFFER_OVERFLOW_ERROR) {
			return (size_t)-1;
		}

		return (size_t)nLen;
	}

	bool ConvertFromStore(const char * inbuff,size_t inbuffsize,UChar * outBuff,size_t outBuffSize) {
		UErrorCode nError;

		if (!_converter) {
			nError = U_ZERO_ERROR;
			_converter = ucnv_open(_encoding, &nError);
			if (U_FAILURE(nError)) {
				return false;
			}
		}

		nError = U_ZERO_ERROR;
		ucnv_toUChars(_converter,outBuff, (int32_t)outBuffSize,	inbuff, (int32_t)inbuffsize, &nError);
		if (U_FAILURE(nError)) {
			return false;
		}

		return true;
	}

	size_t SizeToStore(const UChar * inbuff) {
		UErrorCode nError;

		if (!_converter) {
			nError = U_ZERO_ERROR;
			_converter = ucnv_open(_encoding, &nError);
			if (U_FAILURE(nError)) {
				return (size_t)-1;
			}
		}

		nError = U_ZERO_ERROR;
		int32_t nLen = ucnv_fromUChars(_converter, NULL, 0,	inbuff, -1, &nError);
		if (U_FAILURE(nError) && nError != U_BUFFER_OVERFLOW_ERROR) {
			return (size_t)-1;
		}

		return (size_t)nLen + 1;
	}

	bool ConvertToStore(const UChar * inbuff,	char * outBuff, size_t outBuffSize) {
		UErrorCode nError;

		if (!_converter) {
			nError = U_ZERO_ERROR;
			_converter = ucnv_open(_encoding, &nError);
			if (U_FAILURE(nError)) {
				return false;
			}
		}

		nError = U_ZERO_ERROR;
		ucnv_fromUChars(_converter,	outBuff, (int32_t)outBuffSize,	inbuff, -1, &nError);
		return !U_FAILURE(nError);
	}
};

#endif // SIMPLE_INI_CONVERT_ICU

#ifdef SIMPLE_INI_CONVERT_WIN32

#define SimpleIniCase     SimpleIniGenericCase

// Windows CE doesn't have errno or MBCS libraries
#ifdef _WIN32_WCE
# ifndef SIMPLE_INI_NO_MBCS
#  define SIMPLE_INI_NO_MBCS
# endif
#endif

#include <windows.h>
#ifdef SIMPLE_INI_NO_MBCS
# define SimpleIniNoCase   SimpleIniGenericNoCase
#else // !SIMPLE_INI_NO_MBCS
/**
* Case-insensitive comparison class using Win32 MBCS functions. This class
* returns a case-insensitive semi-collation order for MBCS text. It may not
* be safe for UTF-8 text returned in char format as we don't know what
* characters will be folded by the function! Therefore, if you are using
* SIMPLE_INI_CHAR == char and SetUnicode(true), then you need to use the generic
* SimpleIniNoCase class instead.
*/
#include <mbstring.h>
template<class CharT>
struct SimpleIniNoCase {
	bool operator()(const CharT * pLeft, const CharT * pRight) const {
		if (sizeof(CharT) == sizeof(char)) {
			return _mbsicmp((const unsigned char *)pLeft, (const unsigned char *)pRight) < 0;
		}

		if (sizeof(CharT) == sizeof(wchar_t)) {
			return _wcsicmp((const wchar_t *)pLeft,	(const wchar_t *)pRight) < 0;
		}

		return SimpleIniGenericNoCase<CharT>()(pLeft, pRight);
	}
};
#endif // SIMPLE_INI_NO_MBCS

/**
* Converts MBCS and UTF-8 to a wchar_t (or equivalent) on Windows. This uses
* only the Win32 functions and doesn't require the external Unicode UTF-8
* conversion library. It will not work on Windows 95 without using Microsoft
* Layer for Unicode in your application.
*/
template<class CharT>
class SimpleIniConvertW {
private:
	UINT _codePage;

protected:
	SimpleIniConvertW() {
	}

public:
	SimpleIniConvertW(bool isu8) {
		_codePage = isu8 ? CP_UTF8 : CP_ACP;
	}

	SimpleIniConvertW(const SimpleIniConvertW & rhs) { 
		operator=(rhs);
	}

	SimpleIniConvertW & operator=(const SimpleIniConvertW & rhs) {
		_codePage = rhs._codePage;
		return *this;
	}

	size_t SizeFromStore(const char * inbuff, size_t inbuffsize){
		assert(inbuffsize != (size_t)-1);
		int retval = MultiByteToWideChar(_codePage, 0,inbuff, (int)inbuffsize,0, 0);
		return (size_t)(retval > 0 ? retval : -1);
	}

	bool ConvertFromStore(const char * inbuff,size_t inbuffsize,CharT * outBuff,size_t outBuffSize) {
		int nSize = MultiByteToWideChar(_codePage, 0,inbuff, (int)inbuffsize,(wchar_t *)outBuff, (int)outBuffSize);
		return (nSize > 0);
	}

	size_t SizeToStore(const CharT * inbuff) {
		int retval = WideCharToMultiByte(_codePage, 0,(const wchar_t *)inbuff, -1,0, 0, 0, 0);
		return (size_t)(retval > 0 ? retval : -1);
	}

	bool ConvertToStore(const CharT * inbuff, char * outBuff, size_t outBuffSize) {
		int retval = WideCharToMultiByte(_codePage, 0,(const wchar_t *)inbuff, -1, outBuff, (int)outBuffSize, 0, 0);
		return retval > 0;
	}
};

#endif // SIMPLE_INI_CONVERT_WIN32

typedef SimpleIniTempl<char, SimpleIniNoCase<char>, SimpleIniConvertA<char> >                 HSimpleIniA;
typedef SimpleIniTempl<char, SimpleIniCase<char>, SimpleIniConvertA<char> >                   HSimpleIniCaseA;

#if defined(SIMPLE_INI_CONVERT_ICU)
typedef SimpleIniTempl<UChar, SimpleIniNoCase<UChar>, SimpleIniConvertW<UChar> >              HSimpleIniW;
typedef SimpleIniTempl<UChar, SimpleIniCase<UChar>, SimpleIniConvertW<UChar> >                HSimpleIniCaseW;
#else
typedef SimpleIniTempl<wchar_t,	SimpleIniNoCase<wchar_t>, SimpleIniConvertW<wchar_t> >        HSimpleIniW;
typedef SimpleIniTempl<wchar_t,	SimpleIniCase<wchar_t>, SimpleIniConvertW<wchar_t> >          HSimpleIniCaseW;
#endif

#ifdef _UNICODE
# define HSimpleIni      HSimpleIniW
# define HSimpleIniCase  HSimpleIniCaseW
# define SIMPLE_INI_NEWLINE      SIMPLE_INI_NEWLINE_W
#else // !_UNICODE
# define HSimpleIni      HSimpleIniA
# define HSimpleIniCase  HSimpleIniCaseA
# define SIMPLE_INI_NEWLINE      SIMPLE_INI_NEWLINE_A
#endif // _UNICODE

#ifdef _MSC_VER
# pragma warning (pop)
#endif

#ifdef SIMPLE_INI_CONVERT_ICU
// if converting using ICU then we need the ICU library
# pragma comment(lib, "icuuc.lib")
#endif

namespace Whale
{
	namespace Util
	{
		class HSetting
		{
		public:
			explicit HSetting(void);
			virtual ~HSetting(void);

			inline std::string filepath() const {
				return filepath_;
			}

			bool load(const std::string& filepath);

			bool write(const std::string& section, const std::string& key, uint32_t value);
			bool write(const std::string& section, const std::string& key, const std::string& value);

			uint32_t read(const std::string& section, const std::string& key, uint32_t def) const;
			std::string read(const std::string& section, const std::string& key, const std::string& def) const;

		private:
			std::shared_ptr<HSimpleIni> simpleIni_;
			std::string filepath_;
		};
	}
}
#endif
