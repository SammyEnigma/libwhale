/***************************************************************************************
* Copyright © 2008-2016, CN Technology Corp., Ltd. All Rights Reserved.
* All Rights Reserved.
*
* File Name			:	hrsa.h
* File Mark			:	See configuration management plan
* File Abstract		:	Brief description of the contents of this document
*
* Original Version	:   1.0
* Original Author	:   developer
* Original Date		:   2015-07-01 17:00:00
****************************************************************************************/
#ifndef HRSA_H
#define HRSA_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <vector>

namespace Whale
{
	namespace Crypto
	{
		class HBigInt {
		public:
			HBigInt();
			HBigInt(const int&);
			HBigInt(const HBigInt&);
			HBigInt(std::string str, int radix);

			void fromString(const std::string& buf);
			void fromHexString(std::string str);
			void fromBinaryString(std::string buf);

			std::string toString() const;
			std::string toHexString() const;

			HBigInt& operator= (const HBigInt&);
			HBigInt& operator= (int& a) { clear(); data[0] = a; return *this; }
			HBigInt& operator >> (const int&);
			HBigInt& operator<< (const int&);

			int  length() const;
			int  binaryLen() const;
			bool sign() const { return sign_; }
			void clear();
			void random(int bits);
			void randomSmall(int bits);
			bool odd() const { return (data[0] & 1); }

			HBigInt operator+ (const HBigInt&) const;
			HBigInt operator- (const HBigInt&) const;
			HBigInt operator- (const int&) const;
			HBigInt operator* (const HBigInt&) const;
			HBigInt operator* (const unsigned int&) const;
			HBigInt operator% (const HBigInt&) const;
			int operator% (const int&) const;

			HBigInt operator/ (const HBigInt&) const;
			HBigInt operator& (const HBigInt&) const;
			HBigInt operator^ (const HBigInt&) const;
			HBigInt operator| (const HBigInt&) const;

			bool operator< (const HBigInt&) const;
			bool operator> (const HBigInt&) const;
			bool operator<= (const int&) const;
			bool operator== (const HBigInt&) const;
			bool operator== (const int&) const;

			friend std::ostream& operator<< (std::ostream&, const HBigInt&);

			static HBigInt gcd(const HBigInt& m, const HBigInt& n);
			static HBigInt euclid(HBigInt& E, HBigInt& A);
			static HBigInt extendedGcd(const HBigInt& a, const HBigInt& b, HBigInt& x, HBigInt& y);
			static HBigInt powerMode(const HBigInt& n, const HBigInt& p, const HBigInt& m);

		private:
			static const size_t capacity_ = 128 + 1;

			bool sign_;
			unsigned int data[capacity_];

			int  hex2Int(char c);
			char int2Hex(int c);
			void outstream(std::ostream& out) const;
		};

		class HBigIntArray : public std::vector<HBigInt>
		{
		public:
			HBigIntArray(const std::string& plain, int segmentLen);

			std::string toString();
			std::string toHexString();

			void pushBack(const std::string& plain);
			void pushBack(const HBigInt&);

			HBigIntArray& operator+= (const std::string&);
			HBigIntArray& operator+= (const HBigInt&);
			HBigIntArray& operator+= (const HBigIntArray&);

		private:
			void split(const std::string& plain);

			int segmentLen_;
		};

		class HRsaKey
		{
		public:
			inline HRsaKey() : bits_(0) {}
			inline HRsaKey(int bits) : bits_(bits) { genKey(bits); }
			inline HRsaKey(const HBigInt& n, const HBigInt& e, const HBigInt& d) : N_(n), E_(e), D_(d) {}

			void genKey(int);
			void pubKey(HBigInt& N, HBigInt& E) const;
			void priKey(HBigInt& N, HBigInt& D) const;

		private:
			void	randomPrime(HBigInt& n, int bits);
			bool	rabinMiller(const HBigInt& n, int bits);
			HBigInt generatePrime(int bits);

			int bits_;
			HBigInt N_, E_, D_;
		};

		class HRsa
		{
		public:
			// bits 256 512 768 1024 2048
			inline HRsa(int bits = 512) : rsaKey_(bits / 2) {}
			inline HRsa(const HRsaKey& key) : rsaKey_(key) {}
			inline HRsa(const HBigInt& N, const HBigInt& E, const HBigInt& D) : rsaKey_(N, E, D) {}

			void pubKey(HBigInt& N, HBigInt& E) const;
			void priKey(HBigInt& N, HBigInt& D) const;

			HBigInt encode(const HBigInt& src) const;
			HBigInt decode(const HBigInt& src) const;

			static HBigInt encode(const HBigInt& src, const HBigInt& N, const HBigInt& E);
			static HBigInt decode(const HBigInt& src, const HBigInt& N, const HBigInt& D);

			static void encode(HBigIntArray& st, const HBigInt& N, const HBigInt& E);
			static void decode(HBigIntArray& st, const HBigInt& N, const HBigInt& D);

		private:
			HRsaKey rsaKey_;
		};
	}
}
#endif
