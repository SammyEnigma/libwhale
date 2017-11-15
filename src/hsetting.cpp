#include "whale/util/hsetting.h"

static const int halfShift = 10; /* used for shifting by 10 bits */
static const unsigned int halfBase = 0x0010000UL;
static const unsigned int halfMask = 0x3FFUL;

#define UNI_SUR_HIGH_START  (unsigned int)0xD800
#define UNI_SUR_HIGH_END    (unsigned int)0xDBFF
#define UNI_SUR_LOW_START   (unsigned int)0xDC00
#define UNI_SUR_LOW_END     (unsigned int)0xDFFF


/* --------------------------------------------------------------------- */

ConvertResult convertUTF32toUTF16(
	const unsigned int** sourceStart, const unsigned int* sourceEnd,
	unsigned short** targetStart, unsigned short* targetEnd, ConvertFlags flags) {
	ConvertResult result = CONVERT_OK;
	const unsigned int* source = *sourceStart;
	unsigned short* target = *targetStart;
	while (source < sourceEnd) {
		unsigned int ch;
		if (target >= targetEnd) {
			result = CONVERT_TARGET_EXHAUSTED; break;
		}
		ch = *source++;
		if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
			/* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
				if (flags == CFLAGS_STRICT) {
					--source; /* return to the illegal value itself */
					result = CONVERT_SOURCE_ILLEGAL;
					break;
				}
				else {
					*target++ = UNI_REPLACEMENT_CHAR;
				}
			}
			else {
				*target++ = (unsigned short)ch; /* normal case */
			}
		}
		else if (ch > UNI_MAX_LEGAL_UTF32) {
			if (flags == CFLAGS_STRICT) {
				result = CONVERT_SOURCE_ILLEGAL;
			}
			else {
				*target++ = UNI_REPLACEMENT_CHAR;
			}
		}
		else {
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			if (target + 1 >= targetEnd) {
				--source; /* Back up source pointer! */
				result = CONVERT_TARGET_EXHAUSTED; break;
			}
			ch -= halfBase;
			*target++ = (unsigned short)((ch >> halfShift) + UNI_SUR_HIGH_START);
			*target++ = (unsigned short)((ch & halfMask) + UNI_SUR_LOW_START);
		}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConvertResult convertUTF16toUTF32(
	const unsigned short** sourceStart, const unsigned short* sourceEnd,
	unsigned int** targetStart, unsigned int* targetEnd, ConvertFlags flags) {
	ConvertResult result = CONVERT_OK;
	const unsigned short* source = *sourceStart;
	unsigned int* target = *targetStart;
	unsigned int ch, ch2;
	while (source < sourceEnd) {
		const unsigned short* oldSource = source; /*  In case we have to back up because of target overflow. */
		ch = *source++;
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
			/* If the 16 bits following the high surrogate are in the source buffer... */
			if (source < sourceEnd) {
				ch2 = *source;
				/* If it's a low surrogate, convert to UTF32. */
				if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
					ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
						+ (ch2 - UNI_SUR_LOW_START) + halfBase;
					++source;
				}
				else if (flags == CFLAGS_STRICT) { /* it's an unpaired high surrogate */
					--source; /* return to the illegal value itself */
					result = CONVERT_SOURCE_ILLEGAL;
					break;
				}
			}
			else { /* We don't have the 16 bits following the high surrogate. */
				--source; /* return to the high surrogate */
				result = CONVERT_SOURCE_EXHAUSTED;
				break;
			}
		}
		else if (flags == CFLAGS_STRICT) {
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
				--source; /* return to the illegal value itself */
				result = CONVERT_SOURCE_ILLEGAL;
				break;
			}
		}
		if (target >= targetEnd) {
			source = oldSource; /* Back up source pointer! */
			result = CONVERT_TARGET_EXHAUSTED; break;
		}
		*target++ = ch;
	}
	*sourceStart = source;
	*targetStart = target;
#ifdef CVTUTF_DEBUG
	if (result == CONVERT_SOURCE_ILLEGAL) {
		fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
		fflush(stderr);
	}
#endif
	return result;
}

/* --------------------------------------------------------------------- */

/*
* Index into the table below with the first byte of a UTF-8 sequence to
* get the number of trailing bytes that are supposed to follow it.
* Note that *legal* UTF-8 values can't have 4 or 5-bytes. The table is
* left as-is for anyone who may want to do such conversion, which was
* allowed in earlier algorithms.
*/
static const char trailingBytesForUTF8[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

/*
* Magic values subtracted from a buffer value during UTF8 conversion.
* This table contains as many values as there might be trailing bytes
* in a UTF-8 sequence.
*/
static const unsigned int offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL,
0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
* Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
* into the first byte, depending on how many bytes follow.  There are
* as many entries in this table as there are UTF-8 sequence types.
* (I.e., one byte sequence, two byte... etc.). Remember that sequencs
* for *legal* UTF-8 will be 4 or fewer bytes total.
*/
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
* Constants have been gathered. Loops & conditionals have been removed as
* much as possible for efficiency, in favor of drop-through switches.
* (See "Note A" at the bottom of the file for equivalent code.)
* If your compiler supports it, the "isLegalUTF8" call can be turned
* into an inline function.
*/

/* --------------------------------------------------------------------- */

ConvertResult convertUTF16toUTF8(
	const unsigned short** sourceStart, const unsigned short* sourceEnd,
	unsigned char** targetStart, unsigned char* targetEnd, ConvertFlags flags) {
	ConvertResult result = CONVERT_OK;
	const unsigned short* source = *sourceStart;
	unsigned char* target = *targetStart;
	while (source < sourceEnd) {
		unsigned int ch;
		unsigned short bytesToWrite = 0;
		const unsigned int byteMask = 0xBF;
		const unsigned int byteMark = 0x80;
		const unsigned short* oldSource = source; /* In case we have to back up because of target overflow. */
		ch = *source++;
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END) {
			/* If the 16 bits following the high surrogate are in the source buffer... */
			if (source < sourceEnd) {
				unsigned int ch2 = *source;
				/* If it's a low surrogate, convert to UTF32. */
				if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
					ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
						+ (ch2 - UNI_SUR_LOW_START) + halfBase;
					++source;
				}
				else if (flags == CFLAGS_STRICT) { /* it's an unpaired high surrogate */
					--source; /* return to the illegal value itself */
					result = CONVERT_SOURCE_ILLEGAL;
					break;
				}
			}
			else { /* We don't have the 16 bits following the high surrogate. */
				--source; /* return to the high surrogate */
				result = CONVERT_SOURCE_EXHAUSTED;
				break;
			}
		}
		else if (flags == CFLAGS_STRICT) {
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END) {
				--source; /* return to the illegal value itself */
				result = CONVERT_SOURCE_ILLEGAL;
				break;
			}
		}
		/* Figure out how many bytes the result will require */
		if (ch < (unsigned int)0x80) {
			bytesToWrite = 1;
		}
		else if (ch < (unsigned int)0x800) {
			bytesToWrite = 2;
		}
		else if (ch < (unsigned int)0x10000) {
			bytesToWrite = 3;
		}
		else if (ch < (unsigned int)0x110000) {
			bytesToWrite = 4;
		}
		else {
			bytesToWrite = 3;
			ch = UNI_REPLACEMENT_CHAR;
		}

		target += bytesToWrite;
		if (target > targetEnd) {
			source = oldSource; /* Back up source pointer! */
			target -= bytesToWrite; result = CONVERT_TARGET_EXHAUSTED; break;
		}
		switch (bytesToWrite) { /* note: everything falls through. */
		case 4: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
		case 3: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
		case 2: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
		case 1: *--target = (unsigned char)(ch | firstByteMark[bytesToWrite]);
		}
		target += bytesToWrite;
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

/*
* Utility routine to tell whether a sequence of bytes is legal UTF-8.
* This must be called with the length pre-determined by the first byte.
* If not calling this from ConvertUTF8to*, then the length can be set by:
*  length = trailingBytesForUTF8[*source]+1;
* and the sequence is illegal right away if there aren't that many bytes
* available.
* If presented with a length > 4, this returns false.  The Unicode
* definition of UTF-8 goes up to 4-byte sequences.
*/

static unsigned char isLegalUTF8(const unsigned char *source, int length) {
	unsigned char a;
	const unsigned char *srcptr = source + length;
	switch (length) {
	default: return false;
		/* Everything else falls through when "true"... */
	case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 2: if ((a = (*--srcptr)) > 0xBF) return false;

		switch (*source) {
			/* no fall-through in this inner switch */
		case 0xE0: if (a < 0xA0) return false; break;
		case 0xED: if (a > 0x9F) return false; break;
		case 0xF0: if (a < 0x90) return false; break;
		case 0xF4: if (a > 0x8F) return false; break;
		default:   if (a < 0x80) return false;
		}

	case 1: if (*source >= 0x80 && *source < 0xC2) return false;
	}
	if (*source > 0xF4) return false;
	return true;
}

/* --------------------------------------------------------------------- */

/*
* Exported function to return whether a UTF-8 sequence is legal or not.
* This is not used here; it's just exported.
*/
unsigned char isLegalUTF8Sequence(const unsigned char *source, const unsigned char *sourceEnd) {
	int length = trailingBytesForUTF8[*source] + 1;
	if (source + length > sourceEnd) {
		return false;
	}
	return isLegalUTF8(source, length);
}

/* --------------------------------------------------------------------- */

ConvertResult convertUTF8toUTF16(
	const unsigned char** sourceStart, const unsigned char* sourceEnd,
	unsigned short** targetStart, unsigned short* targetEnd, ConvertFlags flags) {
	ConvertResult result = CONVERT_OK;
	const unsigned char* source = *sourceStart;
	unsigned short* target = *targetStart;
	while (source < sourceEnd) {
		unsigned int ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd) {
			result = CONVERT_SOURCE_EXHAUSTED; break;
		}
		/* Do this check whether lenient or strict */
		if (!isLegalUTF8(source, extraBytesToRead + 1)) {
			result = CONVERT_SOURCE_ILLEGAL;
			break;
		}
		/*
		* The cases all fall through. See "Note A" below.
		*/
		switch (extraBytesToRead) {
		case 5: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
		case 4: ch += *source++; ch <<= 6; /* remember, illegal UTF-8 */
		case 3: ch += *source++; ch <<= 6;
		case 2: ch += *source++; ch <<= 6;
		case 1: ch += *source++; ch <<= 6;
		case 0: ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (target >= targetEnd) {
			source -= (extraBytesToRead + 1); /* Back up source pointer! */
			result = CONVERT_TARGET_EXHAUSTED; break;
		}
		if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
				if (flags == CFLAGS_STRICT) {
					source -= (extraBytesToRead + 1); /* return to the illegal value itself */
					result = CONVERT_SOURCE_ILLEGAL;
					break;
				}
				else {
					*target++ = UNI_REPLACEMENT_CHAR;
				}
			}
			else {
				*target++ = (unsigned short)ch; /* normal case */
			}
		}
		else if (ch > UNI_MAX_UTF16) {
			if (flags == CFLAGS_STRICT) {
				result = CONVERT_SOURCE_ILLEGAL;
				source -= (extraBytesToRead + 1); /* return to the start */
				break; /* Bail out; shouldn't continue */
			}
			else {
				*target++ = UNI_REPLACEMENT_CHAR;
			}
		}
		else {
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			if (target + 1 >= targetEnd) {
				source -= (extraBytesToRead + 1); /* Back up source pointer! */
				result = CONVERT_TARGET_EXHAUSTED; break;
			}
			ch -= halfBase;
			*target++ = (unsigned short)((ch >> halfShift) + UNI_SUR_HIGH_START);
			*target++ = (unsigned short)((ch & halfMask) + UNI_SUR_LOW_START);
		}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConvertResult convertUTF32toUTF8(
	const unsigned int** sourceStart, const unsigned int* sourceEnd,
	unsigned char** targetStart, unsigned char* targetEnd, ConvertFlags flags) {
	ConvertResult result = CONVERT_OK;
	const unsigned int* source = *sourceStart;
	unsigned char* target = *targetStart;
	while (source < sourceEnd) {
		unsigned int ch;
		unsigned short bytesToWrite = 0;
		const unsigned int byteMask = 0xBF;
		const unsigned int byteMark = 0x80;
		ch = *source++;
		if (flags == CFLAGS_STRICT) {
			/* UTF-16 surrogate values are illegal in UTF-32 */
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
				--source; /* return to the illegal value itself */
				result = CONVERT_SOURCE_ILLEGAL;
				break;
			}
		}
		/*
		* Figure out how many bytes the result will require. Turn any
		* illegally large UTF32 things (> Plane 17) into replacement chars.
		*/
		if (ch < (unsigned int)0x80) {
			bytesToWrite = 1;
		}
		else if (ch < (unsigned int)0x800) {
			bytesToWrite = 2;
		}
		else if (ch < (unsigned int)0x10000) {
			bytesToWrite = 3;
		}
		else if (ch <= UNI_MAX_LEGAL_UTF32) {
			bytesToWrite = 4;
		}
		else {
			bytesToWrite = 3;
			ch = UNI_REPLACEMENT_CHAR;
			result = CONVERT_SOURCE_ILLEGAL;
		}

		target += bytesToWrite;
		if (target > targetEnd) {
			--source; /* Back up source pointer! */
			target -= bytesToWrite; result = CONVERT_TARGET_EXHAUSTED; break;
		}
		switch (bytesToWrite) { /* note: everything falls through. */
		case 4: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
		case 3: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
		case 2: *--target = (unsigned char)((ch | byteMark) & byteMask); ch >>= 6;
		case 1: *--target = (unsigned char)(ch | firstByteMark[bytesToWrite]);
		}
		target += bytesToWrite;
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConvertResult convertUTF8toUTF32(
	const unsigned char** sourceStart, const unsigned char* sourceEnd,
	unsigned int** targetStart, unsigned int* targetEnd, ConvertFlags flags) {
	ConvertResult result = CONVERT_OK;
	const unsigned char* source = *sourceStart;
	unsigned int* target = *targetStart;
	while (source < sourceEnd) {
		unsigned int ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd) {
			result = CONVERT_SOURCE_EXHAUSTED; break;
		}
		/* Do this check whether lenient or strict */
		if (!isLegalUTF8(source, extraBytesToRead + 1)) {
			result = CONVERT_SOURCE_ILLEGAL;
			break;
		}
		/*
		* The cases all fall through. See "Note A" below.
		*/
		switch (extraBytesToRead) {
		case 5: ch += *source++; ch <<= 6;
		case 4: ch += *source++; ch <<= 6;
		case 3: ch += *source++; ch <<= 6;
		case 2: ch += *source++; ch <<= 6;
		case 1: ch += *source++; ch <<= 6;
		case 0: ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (target >= targetEnd) {
			source -= (extraBytesToRead + 1); /* Back up the source pointer! */
			result = CONVERT_TARGET_EXHAUSTED; break;
		}
		if (ch <= UNI_MAX_LEGAL_UTF32) {
			/*
			* UTF-16 surrogate values are illegal in UTF-32, and anything
			* over Plane 17 (> 0x10FFFF) is illegal.
			*/
			if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END) {
				if (flags == CFLAGS_STRICT) {
					source -= (extraBytesToRead + 1); /* return to the illegal value itself */
					result = CONVERT_SOURCE_ILLEGAL;
					break;
				}
				else {
					*target++ = UNI_REPLACEMENT_CHAR;
				}
			}
			else {
				*target++ = ch;
			}
		}
		else { /* i.e., ch > UNI_MAX_LEGAL_UTF32 */
			result = CONVERT_SOURCE_ILLEGAL;
			*target++ = UNI_REPLACEMENT_CHAR;
		}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniTempl<CharT, StrlessT, ConverterT>::SimpleIniTempl(
	bool isu8,
	bool allowMultiKey,
	bool allowMultiLine
	)
	: _dataBuff(0)
	, _dataLen(0)
	, _fileComment(NULL)
	, _storeIsUtf8(isu8)
	, _allowMultiKey(allowMultiKey)
	, _allowMultiLine(allowMultiLine)
	, _shouldSpaces(true)
	, _orderNum(0) {
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniTempl<CharT, StrlessT, ConverterT>::~SimpleIniTempl() {
	Reset();
}

template<class CharT, class StrlessT, class ConverterT>
void SimpleIniTempl<CharT, StrlessT, ConverterT>::Reset() {
	delete[] _dataBuff;
	_dataBuff = NULL;
	_dataLen = 0;
	_fileComment = NULL;

	if (!_section.empty()) {
		_section.erase(_section.begin(), _section.end());
	}

	if (!_nameDepend.empty()) {
		typename TNamesDepend::iterator i = _nameDepend.begin();
		for (; i != _nameDepend.end(); ++i) {
			delete[] const_cast<CharT*>(i->_item);
		}
		_nameDepend.erase(_nameDepend.begin(), _nameDepend.end());
	}
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult SimpleIniTempl<CharT, StrlessT, ConverterT>::LoadFile(const char * filepath) {
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	fopen_s(&fp, filepath, "rb");
#else // !__STDC_WANT_SECURE_LIB__
	fp = fopen(filepath, "rb");
#endif // __STDC_WANT_SECURE_LIB__
	if (!fp) {
		return SIMPLE_INI_FILE;
	}
	SimpleIniResult rc = LoadFile(fp);
	fclose(fp);
	return rc;
}

#ifdef SIMPLE_INI_HAS_WIDE_FILE
template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult SimpleIniTempl<CharT, StrlessT, ConverterT>::LoadFile(const SIMPLE_INI_WCHAR_T * filepath) {
#ifdef _WIN32
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	_wfopen_s(&fp, filepath, L"rb");
#else // !__STDC_WANT_SECURE_LIB__
	fp = _wfopen(filepath, L"rb");
#endif // __STDC_WANT_SECURE_LIB__
	if (!fp) return SIMPLE_INI_FILE;
	SimpleIniResult rc = LoadFile(fp);
	fclose(fp);
	return rc;
#else // !_WIN32 (therefore SIMPLE_INI_CONVERT_ICU)
	char szFile[256];
	u_austrncpy(szFile, filepath, sizeof(szFile));
	return LoadFile(szFile);
#endif // _WIN32
}
#endif // SIMPLE_INI_HAS_WIDE_FILE

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult SimpleIniTempl<CharT, StrlessT, ConverterT>::LoadFile(FILE * fp) {
	int retval = fseek(fp, 0, SEEK_END);
	if (retval != 0) {
		return SIMPLE_INI_FILE;
	}
	long lSize = ftell(fp);
	if (lSize < 0) {
		return SIMPLE_INI_FILE;
	}
	if (lSize == 0) {
		return SIMPLE_INI_OK;
	}

	// allocate and ensure NULL terminated
	char * pData = new(std::nothrow) char[lSize + 1];
	if (!pData) {
		return SIMPLE_INI_NOMEM;
	}
	pData[lSize] = 0;

	// load data into buffer
	fseek(fp, 0, SEEK_SET);
	size_t uRead = fread(pData, sizeof(char), lSize, fp);
	if (uRead != (size_t)lSize) {
		delete[] pData;
		return SIMPLE_INI_FILE;
	}

	// convert the raw data to unicode
	SimpleIniResult rc = LoadData(pData, uRead);
	delete[] pData;
	return rc;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult SimpleIniTempl<CharT, StrlessT, ConverterT>::LoadData(const char * inbuff, size_t inbuffsize) {
	ConverterT converter(_storeIsUtf8);

	// consume the UTF-8 BOM if it exists
	if (_storeIsUtf8 && inbuffsize >= 3) {
		if (memcmp(inbuff, SIMPLE_INI_UTF8_SIGNATURE, 3) == 0) {
			inbuff += 3;
			inbuffsize -= 3;
		}
	}

	if (inbuffsize == 0) {
		return SIMPLE_INI_OK;
	}

	// determine the length of the converted data
	size_t uLen = converter.SizeFromStore(inbuff, inbuffsize);
	if (uLen == (size_t)(-1)) {
		return SIMPLE_INI_FAIL;
	}

	// allocate memory for the data, ensure that there is a NULL
	// terminator wherever the converted data ends
	CharT * pData = new(std::nothrow) CharT[uLen + 1];
	if (!pData) {
		return SIMPLE_INI_NOMEM;
	}
	memset(pData, 0, sizeof(CharT)*(uLen + 1));

	// convert the data
	if (!converter.ConvertFromStore(inbuff, inbuffsize, pData, uLen)) {
		delete[] pData;
		return SIMPLE_INI_FAIL;
	}

	// parse it
	const static CharT empty = 0;
	CharT * pWork = pData;
	const CharT * pSection = &empty;
	const CharT * pItem = NULL;
	const CharT * pVal = NULL;
	const CharT * pComment = NULL;

	// We copy the strings if we are loading data into this class when we
	// already have stored some.
	bool bCopyStrings = (_dataBuff != NULL);

	// find a file comment if it exists, this is a comment that starts at the
	// beginning of the file and continues until the first blank line.
	SimpleIniResult rc = FindFileComment(pWork, bCopyStrings);
	if (rc < 0) return rc;

	// add every entry in the file to the data table
	while (FindEntry(pWork, pSection, pItem, pVal, pComment)) {
		rc = AddEntry(pSection, pItem, pVal, pComment, false, bCopyStrings);
		if (rc < 0) return rc;
	}

	// store these strings if we didn't copy them
	if (bCopyStrings) {
		delete[] pData;
	}
	else {
		_dataBuff = pData;
		_dataLen = uLen + 1;
	}

	return SIMPLE_INI_OK;
}

#ifdef SIMPLE_INI_SUPPORT_IOSTREAMS
template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult SimpleIniTempl<CharT, StrlessT, ConverterT>::LoadData(std::istream & isStream) {
	std::string strData;
	char szBuf[512];
	do {
		isStream.get(szBuf, sizeof(szBuf), '\0');
		strData.append(szBuf);
	} while (isStream.good());
	return LoadData(strData);
}
#endif // SIMPLE_INI_SUPPORT_IOSTREAMS

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult SimpleIniTempl<CharT, StrlessT, ConverterT>::FindFileComment(CharT *& inbuff, bool copyStrings) {
	// there can only be a single file comment
	if (_fileComment) {
		return SIMPLE_INI_OK;
	}

	// Load the file comment as multi-line text, this will modify all of
	// the newline characters to be single \n chars
	if (!LoadMultiLineText(inbuff, _fileComment, NULL, false)) {
		return SIMPLE_INI_OK;
	}

	// copy the string if necessary
	if (copyStrings) {
		SimpleIniResult rc = CopyString(_fileComment);
		if (rc < 0) return rc;
	}

	return SIMPLE_INI_OK;
}

template<class CharT, class StrlessT, class ConverterT>
bool SimpleIniTempl<CharT, StrlessT, ConverterT>::FindEntry(
CharT *& inbuff,
const CharT *&  section,
const CharT *&  key,
const CharT *&  value,
const CharT *&  comment
) const {
	comment = NULL;

	CharT * pTrail = NULL;
	while (*inbuff) {
		// skip spaces and empty lines
		while (*inbuff && IsSpace(*inbuff)) {
			++inbuff;
		}
		if (!*inbuff) {
			break;
		}

		// skip processing of comment lines but keep a pointer to
		// the start of the comment.
		if (IsComment(*inbuff)) {
			LoadMultiLineText(inbuff, comment, NULL, true);
			continue;
		}

		// process section names
		if (*inbuff == '[') {
			// skip leading spaces
			++inbuff;
			while (*inbuff && IsSpace(*inbuff)) {
				++inbuff;
			}

			// find the end of the section name (it may contain spaces)
			// and convert it to lowercase as necessary
			section = inbuff;
			while (*inbuff && *inbuff != ']' && !IsNewLineChar(*inbuff)) {
				++inbuff;
			}

			// if it's an invalid line, just skip it
			if (*inbuff != ']') {
				continue;
			}

			// remove trailing spaces from the section
			pTrail = inbuff - 1;
			while (pTrail >= section && IsSpace(*pTrail)) {
				--pTrail;
			}
			++pTrail;
			*pTrail = 0;

			// skip to the end of the line
			++inbuff;  // safe as checked that it == ']' above
			while (*inbuff && !IsNewLineChar(*inbuff)) {
				++inbuff;
			}

			key = NULL;
			value = NULL;
			return true;
		}

		// find the end of the key name (it may contain spaces)
		// and convert it to lowercase as necessary
		key = inbuff;
		while (*inbuff && *inbuff != '=' && !IsNewLineChar(*inbuff)) {
			++inbuff;
		}

		// if it's an invalid line, just skip it
		if (*inbuff != '=') {
			continue;
		}

		// empty keys are invalid
		if (key == inbuff) {
			while (*inbuff && !IsNewLineChar(*inbuff)) {
				++inbuff;
			}
			continue;
		}

		// remove trailing spaces from the key
		pTrail = inbuff - 1;
		while (pTrail >= key && IsSpace(*pTrail)) {
			--pTrail;
		}
		++pTrail;
		*pTrail = 0;

		// skip leading whitespace on the value
		++inbuff;  // safe as checked that it == '=' above
		while (*inbuff && !IsNewLineChar(*inbuff) && IsSpace(*inbuff)) {
			++inbuff;
		}

		// find the end of the value which is the end of this line
		value = inbuff;
		while (*inbuff && !IsNewLineChar(*inbuff)) {
			++inbuff;
		}

		// remove trailing spaces from the value
		pTrail = inbuff - 1;
		if (*inbuff) { // prepare for the next round
			SkipNewLine(inbuff);
		}
		while (pTrail >= value && IsSpace(*pTrail)) {
			--pTrail;
		}
		++pTrail;
		*pTrail = 0;

		// check for multi-line entries
		if (_allowMultiLine && IsMultiLineTag(value)) {
			// skip the "<<<" to get the tag that will end the multiline
			const CharT * pTagName = value + 3;
			return LoadMultiLineText(inbuff, value, pTagName);
		}

		// return the standard entry
		return true;
	}

	return false;
}

template<class CharT, class StrlessT, class ConverterT>
bool SimpleIniTempl<CharT, StrlessT, ConverterT>::IsMultiLineTag(
const CharT * value
) const {
	// check for the "<<<" prefix for a multi-line entry
	if (*value++ != '<') return false;
	if (*value++ != '<') return false;
	if (*value++ != '<') return false;
	return true;
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::IsMultiLineData(
const CharT * inbuff) const {
	// empty string
	if (!*inbuff) {
		return false;
	}

	// check for prefix
	if (IsSpace(*inbuff)) {
		return true;
	}

	// embedded newlines
	while (*inbuff) {
		if (IsNewLineChar(*inbuff)) {
			return true;
		}
		++inbuff;
	}

	// check for suffix
	if (IsSpace(*--inbuff)) {
		return true;
	}

	return false;
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::IsNewLineChar(
CharT ch
) const {
	return (ch == '\n' || ch == '\r');
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::LoadMultiLineText(
CharT *&          inbuff,
const CharT *&    value,
const CharT *     name,
bool                allowBlankComment
) const {
	// we modify this data to strip all newlines down to a single '\n'
	// character. This means that on Windows we need to strip out some
	// characters which will make the data shorter.
	// i.e.  LINE1-LINE1\r\nLINE2-LINE2\0 will become
	//       LINE1-LINE1\nLINE2-LINE2\0
	// The pDataLine entry is the pointer to the location in memory that
	// the current line needs to start to run following the existing one.
	// This may be the same as pCurrLine in which case no move is needed.
	CharT * pDataLine = inbuff;
	CharT * pCurrLine;

	// value starts at the current line
	value = inbuff;

	// find the end tag. This tag must start in column 1 and be
	// followed by a newline. No whitespace removal is done while
	// searching for this tag.
	CharT cEndOfLineChar = *inbuff;
	for (;;) {
		// if we are loading comments then we need a comment character as
		// the first character on every line
		if (!name && !IsComment(*inbuff)) {
			// if we aren't allowing blank lines then we're done
			if (!allowBlankComment) {
				break;
			}

			// if we are allowing blank lines then we only include them
			// in this comment if another comment follows, so read ahead
			// to find out.
			CharT * pCurr = inbuff;
			int nNewLines = 0;
			while (IsSpace(*pCurr)) {
				if (IsNewLineChar(*pCurr)) {
					++nNewLines;
					SkipNewLine(pCurr);
				}
				else {
					++pCurr;
				}
			}

			// we have a comment, add the blank lines to the output
			// and continue processing from here
			if (IsComment(*pCurr)) {
				for (; nNewLines > 0; --nNewLines) *pDataLine++ = '\n';
				inbuff = pCurr;
				continue;
			}

			// the comment ends here
			break;
		}

		// find the end of this line
		pCurrLine = inbuff;
		while (*inbuff && !IsNewLineChar(*inbuff)) ++inbuff;

		// move this line down to the location that it should be if necessary
		if (pDataLine < pCurrLine) {
			size_t nLen = (size_t)(inbuff - pCurrLine);
			memmove(pDataLine, pCurrLine, nLen * sizeof(CharT));
			pDataLine[nLen] = '\0';
		}

		// end the line with a NULL
		cEndOfLineChar = *inbuff;
		*inbuff = 0;

		// if are looking for a tag then do the check now. This is done before
		// checking for end of the data, so that if we have the tag at the end
		// of the data then the tag is removed correctly.
		if (name &&
			(!IsLess(pDataLine, name) && !IsLess(name, pDataLine)))
		{
			break;
		}

		// if we are at the end of the data then we just automatically end
		// this entry and return the current data.
		if (!cEndOfLineChar) {
			return true;
		}

		// otherwise we need to process this newline to ensure that it consists
		// of just a single \n character.
		pDataLine += (inbuff - pCurrLine);
		*inbuff = cEndOfLineChar;
		SkipNewLine(inbuff);
		*pDataLine++ = '\n';
	}

	// if we didn't find a comment at all then return false
	if (value == inbuff) {
		value = NULL;
		return false;
	}

	// the data (which ends at the end of the last line) needs to be
	// null-terminated BEFORE before the newline character(s). If the
	// user wants a new line in the multi-line data then they need to
	// add an empty line before the tag.
	*--pDataLine = '\0';

	// if looking for a tag and if we aren't at the end of the data,
	// then move inbuff to the start of the next line.
	if (name && cEndOfLineChar) {
		assert(IsNewLineChar(cEndOfLineChar));
		*inbuff = cEndOfLineChar;
		SkipNewLine(inbuff);
	}

	return true;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::CopyString(
const CharT *& text
)
{
	size_t uLen = 0;
	if (sizeof(CharT) == sizeof(char)) {
		uLen = strlen((const char *)text);
	}
	else if (sizeof(CharT) == sizeof(wchar_t)) {
		uLen = wcslen((const wchar_t *)text);
	}
	else {
		for (; text[uLen]; ++uLen) /*loop*/;
	}
	++uLen; // NULL character
	CharT * pCopy = new(std::nothrow) CharT[uLen];
	if (!pCopy) {
		return SIMPLE_INI_NOMEM;
	}
	memcpy(pCopy, text, sizeof(CharT)*uLen);
	_nameDepend.push_back(pCopy);
	text = pCopy;
	return SIMPLE_INI_OK;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::AddEntry(
const CharT * section,
const CharT * key,
const CharT * value,
const CharT * comment,
bool            replace,
bool            copyStrings
)
{
	SimpleIniResult rc;
	bool bInserted = false;

	assert(!comment || IsComment(*comment));

	// if we are copying strings then make a copy of the comment now
	// because we will need it when we add the entry.
	if (copyStrings && comment) {
		rc = CopyString(comment);
		if (rc < 0) return rc;
	}

	// create the section entry if necessary
	typename TSection::iterator iSection = _section.find(section);
	if (iSection == _section.end()) {
		// if the section doesn't exist then we need a copy as the
		// string needs to last beyond the end of this function
		if (copyStrings) {
			rc = CopyString(section);
			if (rc < 0) return rc;
		}

		// only set the comment if this is a section only entry
		Entry oSection(section, ++_orderNum);
		if (comment && (!key || !value)) {
			oSection._comment = comment;
		}

		typename TSection::value_type oEntry(oSection, TKeyVal());
		typedef typename TSection::iterator SectionIterator;
		std::pair<SectionIterator, bool> i = _section.insert(oEntry);
		iSection = i.first;
		bInserted = true;
	}
	if (!key || !value) {
		// section only entries are specified with pItem and pVal as NULL
		return bInserted ? SIMPLE_INI_INSERTED : SIMPLE_INI_UPDATED;
	}

	// check for existence of the key
	TKeyVal & keyval = iSection->second;
	typename TKeyVal::iterator iKey = keyval.find(key);

	// remove all existing entries but save the load order and
	// comment of the first entry
	int nLoadOrder = ++_orderNum;
	if (iKey != keyval.end() && _allowMultiKey && replace) {
		const CharT * pComment = NULL;
		while (iKey != keyval.end() && !IsLess(key, iKey->first._item)) {
			if (iKey->first._order < nLoadOrder) {
				nLoadOrder = iKey->first._order;
				pComment = iKey->first._comment;
			}
			++iKey;
		}
		if (pComment) {
			DeleteString(comment);
			comment = pComment;
			CopyString(comment);
		}
		Delete(section, key);
		iKey = keyval.end();
	}

	// make string copies if necessary
	bool bForceCreateNewKey = _allowMultiKey && !replace;
	if (copyStrings) {
		if (bForceCreateNewKey || iKey == keyval.end()) {
			// if the key doesn't exist then we need a copy as the
			// string needs to last beyond the end of this function
			// because we will be inserting the key next
			rc = CopyString(key);
			if (rc < 0) return rc;
		}

		// we always need a copy of the value
		rc = CopyString(value);
		if (rc < 0) return rc;
	}

	// create the key entry
	if (iKey == keyval.end() || bForceCreateNewKey) {
		Entry oKey(key, nLoadOrder);
		if (comment) {
			oKey._comment = comment;
		}
		typename TKeyVal::value_type oEntry(oKey, static_cast<const CharT *>(NULL));
		iKey = keyval.insert(oEntry);
		bInserted = true;
	}
	iKey->second = value;
	return bInserted ? SIMPLE_INI_INSERTED : SIMPLE_INI_UPDATED;
}

template<class CharT, class StrlessT, class ConverterT>
const CharT *
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetValue(
const CharT * section,
const CharT * key,
const CharT * def,
bool *          multiple
) const
{
	if (multiple) {
		*multiple = false;
	}
	if (!section || !key) {
		return def;
	}
	typename TSection::const_iterator iSection = _section.find(section);
	if (iSection == _section.end()) {
		return def;
	}
	typename TKeyVal::const_iterator iKeyVal = iSection->second.find(key);
	if (iKeyVal == iSection->second.end()) {
		return def;
	}

	// check for multiple entries with the same key
	if (_allowMultiKey && multiple) {
		typename TKeyVal::const_iterator iTemp = iKeyVal;
		if (++iTemp != iSection->second.end()) {
			if (!IsLess(key, iTemp->first._item)) {
				*multiple = true;
			}
		}
	}

	return iKeyVal->second;
}

template<class CharT, class StrlessT, class ConverterT>
long
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetLongValue(
const CharT * section,
const CharT * key,
long            def,
bool *          multiple
) const
{
	// return the default if we don't have a value
	const CharT * pszValue = GetValue(section, key, NULL, multiple);
	if (!pszValue || !*pszValue) return def;

	// convert to UTF-8/MBCS which for a numeric value will be the same as ASCII
	char szValue[64] = { 0 };
	ConverterT c(_storeIsUtf8);
	if (!c.ConvertToStore(pszValue, szValue, sizeof(szValue))) {
		return def;
	}

	// handle the value as hex if prefaced with "0x"
	long nValue = def;
	char * pszSuffix = szValue;
	if (szValue[0] == '0' && (szValue[1] == 'x' || szValue[1] == 'X')) {
		if (!szValue[2]) return def;
		nValue = strtol(&szValue[2], &pszSuffix, 16);
	}
	else {
		nValue = strtol(szValue, &pszSuffix, 10);
	}

	// any invalid strings will return the default value
	if (*pszSuffix) {
		return def;
	}

	return nValue;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::SetLongValue(
const CharT * section,
const CharT * key,
long            value,
const CharT * comment,
bool            a_bUseHex,
bool            replace
)
{
	// use SetValue to create sections
	if (!section || !key) return SIMPLE_INI_FAIL;

	// convert to an ASCII string
	char szInput[64];
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	sprintf_s(szInput, a_bUseHex ? "0x%lx" : "%ld", value);
#else // !__STDC_WANT_SECURE_LIB__
	sprintf(szInput, a_bUseHex ? "0x%lx" : "%ld", value);
#endif // __STDC_WANT_SECURE_LIB__

	// convert to output text
	CharT szOutput[64];
	ConverterT c(_storeIsUtf8);
	c.ConvertFromStore(szInput, strlen(szInput) + 1,
		szOutput, sizeof(szOutput) / sizeof(CharT));

	// actually add it
	return AddEntry(section, key, szOutput, comment, replace, true);
}

template<class CharT, class StrlessT, class ConverterT>
double
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetDoubleValue(
const CharT * section,
const CharT * key,
double          def,
bool *          multiple
) const
{
	// return the default if we don't have a value
	const CharT * pszValue = GetValue(section, key, NULL, multiple);
	if (!pszValue || !*pszValue) return def;

	// convert to UTF-8/MBCS which for a numeric value will be the same as ASCII
	char szValue[64] = { 0 };
	ConverterT c(_storeIsUtf8);
	if (!c.ConvertToStore(pszValue, szValue, sizeof(szValue))) {
		return def;
	}

	char * pszSuffix = NULL;
	double nValue = strtod(szValue, &pszSuffix);

	// any invalid strings will return the default value
	if (!pszSuffix || *pszSuffix) {
		return def;
	}

	return nValue;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::SetDoubleValue(
const CharT * section,
const CharT * key,
double          value,
const CharT * comment,
bool            replace
)
{
	// use SetValue to create sections
	if (!section || !key) return SIMPLE_INI_FAIL;

	// convert to an ASCII string
	char szInput[64];
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	sprintf_s(szInput, "%f", value);
#else // !__STDC_WANT_SECURE_LIB__
	sprintf(szInput, "%f", value);
#endif // __STDC_WANT_SECURE_LIB__

	// convert to output text
	CharT szOutput[64];
	ConverterT c(_storeIsUtf8);
	c.ConvertFromStore(szInput, strlen(szInput) + 1,
		szOutput, sizeof(szOutput) / sizeof(CharT));

	// actually add it
	return AddEntry(section, key, szOutput, comment, replace, true);
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetBoolValue(
const CharT * section,
const CharT * key,
bool            def,
bool *          multiple
) const
{
	// return the default if we don't have a value
	const CharT * pszValue = GetValue(section, key, NULL, multiple);
	if (!pszValue || !*pszValue) return def;

	// we only look at the minimum number of characters
	switch (pszValue[0]) {
	case 't': case 'T': // true
	case 'y': case 'Y': // yes
	case '1':           // 1 (one)
		return true;

	case 'f': case 'F': // false
	case 'n': case 'N': // no
	case '0':           // 0 (zero)
		return false;

	case 'o': case 'O':
		if (pszValue[1] == 'n' || pszValue[1] == 'N') return true;  // on
		if (pszValue[1] == 'f' || pszValue[1] == 'F') return false; // off
		break;
	}

	// no recognized value, return the default
	return def;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::SetBoolValue(
const CharT * section,
const CharT * key,
bool            value,
const CharT * comment,
bool            replace
)
{
	// use SetValue to create sections
	if (!section || !key) return SIMPLE_INI_FAIL;

	// convert to an ASCII string
	const char * pszInput = value ? "true" : "false";

	// convert to output text
	CharT szOutput[64];
	ConverterT c(_storeIsUtf8);
	c.ConvertFromStore(pszInput, strlen(pszInput) + 1,
		szOutput, sizeof(szOutput) / sizeof(CharT));

	// actually add it
	return AddEntry(section, key, szOutput, comment, replace, true);
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetAllValues(
const CharT * section,
const CharT * key,
TNamesDepend &  values
) const
{
	values.clear();

	if (!section || !key) {
		return false;
	}
	typename TSection::const_iterator iSection = _section.find(section);
	if (iSection == _section.end()) {
		return false;
	}
	typename TKeyVal::const_iterator iKeyVal = iSection->second.find(key);
	if (iKeyVal == iSection->second.end()) {
		return false;
	}

	// insert all values for this key
	values.push_back(Entry(iKeyVal->second, iKeyVal->first._comment, iKeyVal->first._order));
	if (_allowMultiKey) {
		++iKeyVal;
		while (iKeyVal != iSection->second.end() && !IsLess(key, iKeyVal->first._item)) {
			values.push_back(Entry(iKeyVal->second, iKeyVal->first._comment, iKeyVal->first._order));
			++iKeyVal;
		}
	}

	return true;
}

template<class CharT, class StrlessT, class ConverterT>
int
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetSectionSize(
const CharT * section
) const
{
	if (!section) {
		return -1;
	}

	typename TSection::const_iterator iSection = _section.find(section);
	if (iSection == _section.end()) {
		return -1;
	}
	const TKeyVal & kv = iSection->second;

	// if multi-key isn't permitted then the section size is
	// the number of keys that we have.
	if (!_allowMultiKey || kv.empty()) {
		return (int)kv.size();
	}

	// otherwise we need to count them
	int nCount = 0;
	const CharT * pLastKey = NULL;
	typename TKeyVal::const_iterator iKeyVal = kv.begin();
	for (int n = 0; iKeyVal != kv.end(); ++iKeyVal, ++n) {
		if (!pLastKey || IsLess(pLastKey, iKeyVal->first._item)) {
			++nCount;
			pLastKey = iKeyVal->first._item;
		}
	}
	return nCount;
}

template<class CharT, class StrlessT, class ConverterT>
const typename SimpleIniTempl<CharT, StrlessT, ConverterT>::TKeyVal *
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetSection(
const CharT * section
) const
{
	if (section) {
		typename TSection::const_iterator i = _section.find(section);
		if (i != _section.end()) {
			return &(i->second);
		}
	}
	return 0;
}

template<class CharT, class StrlessT, class ConverterT>
void
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetAllSections(
TNamesDepend & names
) const
{
	names.clear();
	typename TSection::const_iterator i = _section.begin();
	for (int n = 0; i != _section.end(); ++i, ++n) {
		names.push_back(i->first);
	}
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::GetAllKeys(
const CharT * section,
TNamesDepend &  names
) const
{
	names.clear();

	if (!section) {
		return false;
	}

	typename TSection::const_iterator iSection = _section.find(section);
	if (iSection == _section.end()) {
		return false;
	}

	const TKeyVal & kv = iSection->second;
	const CharT * pLastKey = NULL;
	typename TKeyVal::const_iterator iKeyVal = kv.begin();
	for (int n = 0; iKeyVal != kv.end(); ++iKeyVal, ++n) {
		if (!pLastKey || IsLess(pLastKey, iKeyVal->first._item)) {
			names.push_back(iKeyVal->first);
			pLastKey = iKeyVal->first._item;
		}
	}

	return true;
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::SaveFile(
const char *    filepath,
bool            addSignature
) const
{
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	fopen_s(&fp, filepath, "wb");
#else // !__STDC_WANT_SECURE_LIB__
	fp = fopen(filepath, "wb");
#endif // __STDC_WANT_SECURE_LIB__
	if (!fp) return SIMPLE_INI_FILE;
	SimpleIniResult rc = SaveFile(fp, addSignature);
	fclose(fp);
	return rc;
}

#ifdef SIMPLE_INI_HAS_WIDE_FILE
template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::SaveFile(
const SIMPLE_INI_WCHAR_T *  filepath,
bool                addSignature
) const
{
#ifdef _WIN32
	FILE * fp = NULL;
#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	_wfopen_s(&fp, filepath, L"wb");
#else // !__STDC_WANT_SECURE_LIB__
	fp = _wfopen(filepath, L"wb");
#endif // __STDC_WANT_SECURE_LIB__
	if (!fp) return SIMPLE_INI_FILE;
	SimpleIniResult rc = SaveFile(fp, addSignature);
	fclose(fp);
	return rc;
#else // !_WIN32 (therefore SIMPLE_INI_CONVERT_ICU)
	char szFile[256];
	u_austrncpy(szFile, filepath, sizeof(szFile));
	return SaveFile(szFile, addSignature);
#endif // _WIN32
}
#endif // SIMPLE_INI_HAS_WIDE_FILE

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::SaveFile(
FILE *  fp,
bool    addSignature
) const
{
	FileWriter writer(fp);
	return Save(writer, addSignature);
}

template<class CharT, class StrlessT, class ConverterT>
SimpleIniResult
SimpleIniTempl<CharT, StrlessT, ConverterT>::Save(
OutputWriter &  outWriter,
bool            addSignature
) const
{
	SimpleIniConverterImpl convert(_storeIsUtf8);

	// add the UTF-8 signature if it is desired
	if (_storeIsUtf8 && addSignature) {
		outWriter.Write(SIMPLE_INI_UTF8_SIGNATURE);
	}

	// get all of the sections sorted in load order
	TNamesDepend oSections;
	GetAllSections(oSections);
#if defined(_MSC_VER) && _MSC_VER <= 1200
	oSections.sort();
#elif defined(__BORLANDC__)
	oSections.sort(Entry::LoadOrder());
#else
	oSections.sort(typename Entry::LoadOrder());
#endif

	// write the file comment if we have one
	bool bNeedNewLine = false;
	if (_fileComment) {
		if (!OutputMultiLineText(outWriter, convert, _fileComment)) {
			return SIMPLE_INI_FAIL;
		}
		bNeedNewLine = true;
	}

	// iterate through our sections and output the data
	typename TNamesDepend::const_iterator iSection = oSections.begin();
	for (; iSection != oSections.end(); ++iSection) {
		// write out the comment if there is one
		if (iSection->_comment) {
			if (bNeedNewLine) {
				outWriter.Write(SIMPLE_INI_NEWLINE_A);
				outWriter.Write(SIMPLE_INI_NEWLINE_A);
			}
			if (!OutputMultiLineText(outWriter, convert, iSection->_comment)) {
				return SIMPLE_INI_FAIL;
			}
			bNeedNewLine = false;
		}

		if (bNeedNewLine) {
			outWriter.Write(SIMPLE_INI_NEWLINE_A);
			outWriter.Write(SIMPLE_INI_NEWLINE_A);
			bNeedNewLine = false;
		}

		// write the section (unless there is no section name)
		if (*iSection->_item) {
			if (!convert.ConvertToStore(iSection->_item)) {
				return SIMPLE_INI_FAIL;
			}
			outWriter.Write("[");
			outWriter.Write(convert.data());
			outWriter.Write("]");
			outWriter.Write(SIMPLE_INI_NEWLINE_A);
		}

		// get all of the keys sorted in load order
		TNamesDepend oKeys;
		GetAllKeys(iSection->_item, oKeys);
#if defined(_MSC_VER) && _MSC_VER <= 1200
		oKeys.sort();
#elif defined(__BORLANDC__)
		oKeys.sort(Entry::LoadOrder());
#else
		oKeys.sort(typename Entry::LoadOrder());
#endif

		// write all keys and values
		typename TNamesDepend::const_iterator iKey = oKeys.begin();
		for (; iKey != oKeys.end(); ++iKey) {
			// get all values for this key
			TNamesDepend oValues;
			GetAllValues(iSection->_item, iKey->_item, oValues);

			typename TNamesDepend::const_iterator iValue = oValues.begin();
			for (; iValue != oValues.end(); ++iValue) {
				// write out the comment if there is one
				if (iValue->_comment) {
					outWriter.Write(SIMPLE_INI_NEWLINE_A);
					if (!OutputMultiLineText(outWriter, convert, iValue->_comment)) {
						return SIMPLE_INI_FAIL;
					}
				}

				// write the key
				if (!convert.ConvertToStore(iKey->_item)) {
					return SIMPLE_INI_FAIL;
				}
				outWriter.Write(convert.data());

				// write the value
				if (!convert.ConvertToStore(iValue->_item)) {
					return SIMPLE_INI_FAIL;
				}
				outWriter.Write(_shouldSpaces ? " = " : "=");
				if (_allowMultiLine && IsMultiLineData(iValue->_item)) {
					// multi-line data needs to be processed specially to ensure
					// that we use the correct newline format for the current system
					outWriter.Write("<<<END_OF_TEXT" SIMPLE_INI_NEWLINE_A);
					if (!OutputMultiLineText(outWriter, convert, iValue->_item)) {
						return SIMPLE_INI_FAIL;
					}
					outWriter.Write("END_OF_TEXT");
				}
				else {
					outWriter.Write(convert.data());
				}
				outWriter.Write(SIMPLE_INI_NEWLINE_A);
			}
		}

		bNeedNewLine = true;
	}

	return SIMPLE_INI_OK;
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::OutputMultiLineText(
OutputWriter &  outWriter,
SimpleIniConverterImpl &     converter,
const CharT * text
) const
{
	const CharT * pEndOfLine;
	CharT cEndOfLineChar = *text;
	while (cEndOfLineChar) {
		// find the end of this line
		pEndOfLine = text;
		for (; *pEndOfLine && *pEndOfLine != '\n'; ++pEndOfLine) /*loop*/;
		cEndOfLineChar = *pEndOfLine;

		// temporarily null terminate, convert and output the line
		*const_cast<CharT*>(pEndOfLine) = 0;
		if (!converter.ConvertToStore(text)) {
			return false;
		}
		*const_cast<CharT*>(pEndOfLine) = cEndOfLineChar;
		text += (pEndOfLine - text) + 1;
		outWriter.Write(converter.data());
		outWriter.Write(SIMPLE_INI_NEWLINE_A);
	}
	return true;
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::Delete(
const CharT * section,
const CharT * key,
bool            removeEmpty
)
{
	return DeleteValue(section, key, NULL, removeEmpty);
}

template<class CharT, class StrlessT, class ConverterT>
bool
SimpleIniTempl<CharT, StrlessT, ConverterT>::DeleteValue(
const CharT * section,
const CharT * key,
const CharT * value,
bool            removeEmpty
)
{
	if (!section) {
		return false;
	}

	typename TSection::iterator iSection = _section.find(section);
	if (iSection == _section.end()) {
		return false;
	}

	// remove a single key if we have a keyname
	if (key) {
		typename TKeyVal::iterator iKeyVal = iSection->second.find(key);
		if (iKeyVal == iSection->second.end()) {
			return false;
		}

		const static StrlessT isLess = StrlessT();

		// remove any copied strings and then the key
		typename TKeyVal::iterator iDelete;
		bool bDeleted = false;
		do {
			iDelete = iKeyVal++;

			if (value == NULL ||
				(isLess(value, iDelete->second) == false &&
				isLess(iDelete->second, value) == false)) {
				DeleteString(iDelete->first._item);
				DeleteString(iDelete->second);
				iSection->second.erase(iDelete);
				bDeleted = true;
			}
		} while (iKeyVal != iSection->second.end()
			&& !IsLess(key, iKeyVal->first._item));

		if (!bDeleted) {
			return false;
		}

		// done now if the section is not empty or we are not pruning away
		// the empty sections. Otherwise let it fall through into the section
		// deletion code
		if (!removeEmpty || !iSection->second.empty()) {
			return true;
		}
	}
	else {
		// delete all copied strings from this section. The actual
		// entries will be removed when the section is removed.
		typename TKeyVal::iterator iKeyVal = iSection->second.begin();
		for (; iKeyVal != iSection->second.end(); ++iKeyVal) {
			DeleteString(iKeyVal->first._item);
			DeleteString(iKeyVal->second);
		}
	}

	// delete the section itself
	DeleteString(iSection->first._item);
	_section.erase(iSection);

	return true;
}

template<class CharT, class StrlessT, class ConverterT>
void
SimpleIniTempl<CharT, StrlessT, ConverterT>::DeleteString(
const CharT * text
)
{
	// strings may exist either inside the data block, or they will be
	// individually allocated and stored in m_strings. We only physically
	// delete those stored in m_strings.
	if (text < _dataBuff || text >= _dataBuff + _dataLen) {
		typename TNamesDepend::iterator i = _nameDepend.begin();
		for (; i != _nameDepend.end(); ++i) {
			if (text == i->_item) {
				delete[] const_cast<CharT*>(i->_item);
				_nameDepend.erase(i);
				break;
			}
		}
	}
}

namespace Whale
{
	namespace Util
	{		
		HSetting::HSetting() : simpleIni_(new HSimpleIni())
		{
			setlocale(LC_ALL, "");

			// _simpleIni->SetUnicode(true);
			// _simpleIni->SetMultiKey(false);
			// _simpleIni->SetMultiLine(false);
		}

		HSetting::~HSetting(void) 
		{

		}

		bool HSetting::load(const std::string& filepath)
		{
			filepath_ = filepath;			

			try {
				std::ifstream instream;
				instream.open(filepath.c_str(), std::ifstream::in | std::ifstream::binary);

				if (simpleIni_->LoadData(instream) < 0) {
					return false;
				}

				instream.close();
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
				return false;
			}
			catch (...) {
				std::cout << "Unknown Err." << std::endl;
				return false;
			}

			return true;
		}

		bool HSetting::write(const std::string& section, const std::string& key, uint32_t value)
		{
			try {
				if (simpleIni_->SetLongValue(section.c_str(), key.c_str(), value) < 0)
					return false;
				if (simpleIni_->SaveFile(filepath_.c_str()) < 0)
					return false;
				return true;
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
			}
			catch (...) {
				std::cout << "Unknown Err." << std::endl;
			}

			return false;
		}

		bool HSetting::write(const std::string& section, const std::string& key, const std::string& value)
		{
			try {
				if (simpleIni_->SetValue(section.c_str(), key.c_str(), value.c_str()) < 0)
					return false;
				if (simpleIni_->SaveFile(filepath_.c_str()) < 0)
					return false;
				return true;
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
			}
			catch (...) {
				std::cout << "Unknown Err." << std::endl;
			}

			return false;
		}


		uint32_t HSetting::read(const std::string& section, const std::string& key, uint32_t def) const
		{
			try {
				return simpleIni_->GetLongValue(section.c_str(), key.c_str(), def);
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
			}
			catch (...) {
				std::cout << "Unknown Err." << std::endl;
			}

			return def;
		}

		std::string HSetting::read(const std::string& section, const std::string& key, const std::string& def) const
		{
			try {
				return simpleIni_->GetValue(section.c_str(), key.c_str(), def.c_str());
			}
			catch (const std::exception& e) {
				std::cout << e.what() << std::endl;
			}
			catch (...) {
				std::cout << "Unknown Err." << std::endl;
			}

			return def;
		}
	}
}
