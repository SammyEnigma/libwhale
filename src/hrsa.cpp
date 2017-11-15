#include "whale/crypto/hrsa.h"

#include <ctime>
#include "whale/basic/hplatform.h"


namespace Whale
{
	namespace Crypto
	{
		const static int PRIMETABLE[669] = {
			2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31,
			37, 41, 43, 47, 53, 59, 61, 67, 71, 73,
			79, 83, 89, 97, 101, 103, 107, 109, 113, 127,
			131, 137, 139, 149, 151, 157, 163, 167, 173, 179,
			181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
			239, 241, 251, 257, 263, 269, 271, 277, 281, 283,
			293, 307, 311, 313, 317, 331, 337, 347, 349, 353,
			359, 367, 373, 379, 383, 389, 397, 401, 409, 419,
			421, 431, 433, 439, 443, 449, 457, 461, 463, 467,
			479, 487, 491, 499, 503, 509, 521, 523, 541, 547,
			557, 563, 569, 571, 577, 587, 593, 599, 601, 607,
			613, 617, 619, 631, 641, 643, 647, 653, 659, 661,
			673, 677, 683, 691, 701, 709, 719, 727, 733, 739,
			743, 751, 757, 761, 769, 773, 787, 797, 809, 811,
			821, 823, 827, 829, 839, 853, 857, 859, 863, 877,
			881, 883, 887, 907, 911, 919, 929, 937, 941, 947,
			953, 967, 971, 977, 983, 991, 997, 1009, 1013, 1019,
			1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069, 1087,
			1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153,
			1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229,
			1231, 1237, 1249, 1259, 1277, 1279, 1283, 1289, 1291, 1297,
			1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367, 1373, 1381,
			1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453,
			1459, 1471, 1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523,
			1531, 1543, 1549, 1553, 1559, 1567, 1571, 1579, 1583, 1597,
			1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663,
			1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741,
			1747, 1753, 1759, 1777, 1783, 1787, 1789, 1801, 1811, 1823,
			1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879, 1889, 1901,
			1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993,
			1997, 1999, 2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063,
			2069, 2081, 2083, 2087, 2089, 2099, 2111, 2113, 2129, 2131,
			2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221,
			2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293,
			2297, 2309, 2311, 2333, 2339, 2341, 2347, 2351, 2357, 2371,
			2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417, 2423, 2437,
			2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539,
			2543, 2549, 2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621,
			2633, 2647, 2657, 2659, 2663, 2671, 2677, 2683, 2687, 2689,
			2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749,
			2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833,
			2837, 2843, 2851, 2857, 2861, 2879, 2887, 2897, 2903, 2909,
			2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971, 2999, 3001,
			3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083,
			3089, 3109, 3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187,
			3191, 3203, 3209, 3217, 3221, 3229, 3251, 3253, 3257, 3259,
			3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343,
			3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433,
			3449, 3457, 3461, 3463, 3467, 3469, 3491, 3499, 3511, 3517,
			3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559, 3571, 3581,
			3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659,
			3671, 3673, 3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733,
			3739, 3761, 3767, 3769, 3779, 3793, 3797, 3803, 3821, 3823,
			3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911,
			3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001,
			4003, 4007, 4013, 4019, 4021, 4027, 4049, 4051, 4057, 4073,
			4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133, 4139, 4153,
			4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241,
			4243, 4253, 4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327,
			4337, 4339, 4349, 4357, 4363, 4373, 4391, 4397, 4409, 4421,
			4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507,
			4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591,
			4597, 4603, 4621, 4637, 4639, 4643, 4649, 4651, 4657, 4663,
			4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733, 4751, 4759,
			4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861,
			4871, 4877, 4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943,
			4951, 4957, 4967, 4969, 4973, 4987, 4993, 4999,
		};

		enum RADIX_TYPE {
			// 二进制
			RADIX_B = 2,
			// 十进制
			RADIX_D = 10,
			// 十六进制
			RADIX_H = 16
		};

		HBigInt::HBigInt()
		{
			clear();
			sign_ = true;
		}

		HBigInt::HBigInt(const int& input)
		{
			clear();
			data[0] = input;
			sign_ = input >= 0;
		}

		HBigInt::HBigInt(const HBigInt& input)
		{
			for (int i = 0; i < capacity_; i++) {
				data[i] = input.data[i];
			}

			sign_ = input.sign_;
		}

		HBigInt::HBigInt(std::string str, int radix)
		{
			clear();
			sign_ = true;

			switch (radix) {

			case RADIX_B:
				fromBinaryString(str);
				break;

			case RADIX_D:
				fromString(str);
				break;

			case RADIX_H:
				fromHexString(str);
				break;

			default:
				std::cout << "BigInt radix error !!!" << std::endl;
				exit(1);
				break;
			}
		}

		void HBigInt::fromHexString(std::string str)
		{
			clear();

			int index = 0;
			while (str.length() > 8) {
				std::string seg = str.substr(str.length() - 8, 8);
				str.erase(str.length() - 8, 8);
				unsigned int cur = 0;
				for (int i = 0; i < 8; i++) {
					cur = cur * 16 + hex2Int(seg[i]);
				}
				data[index++] = cur;
			}

			unsigned int cur = 0;
			for (size_t i = 0; i < str.length(); i++) {
				cur = cur * 16 + hex2Int(str[i]);
			}

			data[index] = cur;
		}

		void HBigInt::fromBinaryString(std::string str)
		{
			clear();

			int index = 0;
			while (str.length() > 32) {
				std::string seg = str.substr(str.length() - 32, 32);
				str.erase(str.length() - 32, 32);
				unsigned int cur = 0;
				for (int i = 0; i < 32; i++) {
					cur = cur * 2 + (seg[i] - '0');
				}
				data[index++] = cur;
			}

			unsigned int cur = 0;
			for (size_t i = 0; i < str.length(); i++) {
				cur = cur * 2 + (str[i] - '0');
			}

			data[index] = cur;
		}

		void HBigInt::fromString(const std::string& buf)
		{
			clear();

			size_t len = buf.length();

			int mask[] = { 0, 8, 16, 24 };
			int validmsk = 0xff;
			size_t curmsk = 0, cur = 0;
			for (size_t i = 0; i < buf.size(); ++i) {
				int tmp = (int)buf[i];
				data[cur] |= ((tmp << mask[curmsk]) & (validmsk << mask[curmsk]));
				curmsk++;
				if (curmsk == 4) {
					curmsk = 0;
					cur++;
				}
			}
		}

		std::string HBigInt::toString() const
		{
			std::string res;
			int len = length();
			char ch;
			unsigned int f4 = 0xFF, f3 = 0xFF00, f2 = 0xFF0000, f1 = 0xFF000000;

			for (int i = 0; i < len - 1; i++) {
				ch = (char)(data[i] & f4);
				res += ch;
				ch = (char)((data[i] & f3) >> 8);
				res += ch;
				ch = (char)((data[i] & f2) >> 16);
				res += ch;
				ch = (char)((data[i] & f1) >> 24);
				res += ch;
			}

			unsigned int tmp = data[len - 1];

			while (tmp) {
				ch = (char)(tmp & f4);
				res += ch;
				tmp >>= 8;
			}

			return res;
		}

		HBigInt& HBigInt::operator= (const HBigInt& input)
		{
			for (int i = 0; i < capacity_; i++) {
				data[i] = input.data[i];
			}

			sign_ = input.sign_;
			return *this;
		}

		bool HBigInt::operator< (const HBigInt& b) const
		{
			for (int i = capacity_ - 1; i > 0; i--) {
				if (this->data[i] < b.data[i])
					return true;
				if (this->data[i] > b.data[i])
					return false;
			}
			return this->data[0] < b.data[0];
		}

		bool HBigInt::operator> (const HBigInt& b) const
		{
			for (int i = capacity_ - 1; i >= 0; i--) {
				if (this->data[i] > b.data[i])
					return true;
				if (this->data[i] < b.data[i])
					return false;
			}
			return false;
		}

		bool HBigInt::operator== (const HBigInt& b) const
		{
			for (int i = 0; i < capacity_; i++)
				if (this->data[i] != b.data[i])
					return false;
			return true;
		}

		bool HBigInt::operator== (const int& b) const
		{
			for (int i = 1; i < this->length(); i++) {
				if (this->data[i] != 0)
					return false;
			}
			return this->data[0] == b;
		}

		HBigInt HBigInt::operator+ (const HBigInt& b) const
		{
			HBigInt result;
			// 64位数据,存放每两位数相加的临时和
			unsigned long long sum;
			// carry为进位标志,sub为当两数符号相异时,存放每两位数相减的临时差
			unsigned int carry = 0, sub;
			// 取a,b中长度较长的长度
			int length = (this->length() >= b.length() ? this->length() : b.length());

			if (this->sign_ == b.sign_) { // 当两数符号相同时,进行加法运算		
				for (int i = 0; i < length; i++) { // 每一位进行竖式相加
					sum = (unsigned long long)this->data[i] + b.data[i] + carry;
					result.data[i] = (unsigned int)sum;
					//sum的高位为进位
					carry = (sum >> 32);
				}

				result.sign_ = this->sign_;
				return result;
			}
			else { // 两数符号不同时,进行减法运算
				HBigInt tempa, tempb;

				if (*this < b) { // 取出a,b中绝对值较大的作为被减数
					tempa = b;
					tempb = *this;
				}
				else {
					tempa = *this;
					tempb = b;
				}

				for (int i = 0; i < length; i++) { // 每一位进行竖式减
					sub = tempb.data[i] + carry;
					if (tempa.data[i] >= sub) {
						result.data[i] = tempa.data[i] - sub;
						carry = 0;
					}
					else {	// 借位减
						result.data[i] = static_cast<uint32_t>((unsigned long long)tempa.data[i] + ((unsigned long long)1 << 32) - sub);
						carry = 1;
					}
				}
				result.sign_ = tempa.sign_;
				return result;
			}
		}

		HBigInt HBigInt::operator- (const HBigInt& b) const
		{
			HBigInt result;
			// 64位数据,存放每两位数相加的临时和
			unsigned long long sum;
			// carry为进位标志,sub为当两数符号相异时,存放每两位数相减的临时差
			unsigned int carry = 0, sub;

			if (this->sign_ == b.sign_) { // 符号相同时,进行减法运算
				HBigInt tempa, tempb;

				if (*this < b) { // 取出a,b中绝对值较大的作为被减数
					tempa = b;
					tempb = *this;
					tempa.sign_ = !tempa.sign_;
				}
				else {
					tempa = *this;
					tempb = b;
				}

				for (int i = 0; i < capacity_; i++) { // 每一位进行竖式减
					sub = tempb.data[i] + carry;
					if (tempa.data[i] >= sub) {
						result.data[i] = tempa.data[i] - sub;
						carry = 0;
					}
					else { // 借位减
						result.data[i] = static_cast<uint32_t>((unsigned long long)tempa.data[i] + ((unsigned long long)1 << 32) - sub);
						carry = 1;
					}
				}
				result.sign_ = tempa.sign_;
				return result;
			}
			else { //两数符号不同时,进行加法运算		
				for (int i = 0; i < capacity_; i++) { // 每一位进行竖式相加
					sum = (unsigned long long)this->data[i] + b.data[i] + carry;
					result.data[i] = (unsigned int)sum;
					//sum的高位为进位
					carry = (sum >> 32);
				}
				result.sign_ = this->sign_;
				return result;
			}
		}

		HBigInt HBigInt::operator- (const int& b) const
		{
			HBigInt temp(b);
			HBigInt result = *this - temp;
			return result;
		}

		HBigInt HBigInt::operator* (const unsigned int& b) const
		{
			HBigInt result;
			//存放B乘以A的每一位的临时积
			unsigned long long sum;
			//存放进位
			unsigned int carry = 0;

			for (int i = 0; i < capacity_; i++) {
				sum = ((unsigned long long)this->data[i]) * b + carry;
				result.data[i] = (unsigned int)sum;
				//进位在SUM的高位中
				carry = (sum >> 32);
			}
			result.sign_ = this->sign_;
			return result;
		}


		HBigInt HBigInt::operator* (const HBigInt& b) const
		{
			//last存放竖式上一行的积,temp存放当前行的积
			HBigInt result, last, temp;
			//sum存放当前行带进位的积
			unsigned long long sum;
			//存放进位
			unsigned int carry;

			for (int i = 0; i < b.length(); i++) { // 进行竖式乘
				carry = 0;
				for (int j = 0; j < this->length() + 1; j++) { // B的每一位与A相乘
					sum = ((unsigned long long)this->data[j]) * b.data[i] + carry;
					if ((i + j) < capacity_)
						temp.data[i + j] = (unsigned int)sum;
					carry = (sum >> 32);
				}
				result = (temp + last);
				last = result;
				temp.clear();
			}

			if (this->sign_ == b.sign_) { //判断积的符号
				result.sign_ = true;
			}
			else {
				result.sign_ = false;
			}

			return result;
		}


		HBigInt HBigInt::operator/ (const HBigInt& b) const
		{
			// mul为当前试商,low,high为二分查找试商时所用的标志
			unsigned int mul, low, high;
			// sub为除数与当前试商的积,subsequent为除数与下一试商的积
			// dividend存放临时被除数
			HBigInt dividend, quotient, sub, subsequent;
			int lengtha = this->length(), lengthb = b.length();

			if (*this < b) { // 如果被除数小于除数,直接返回0
				if (this->sign_ == b.sign_)
					quotient.sign_ = true;
				else
					quotient.sign_ = false;
				return quotient;
			}

			for (int i = 0; i < lengthb; i++) { // 把被除数按除数的长度从高位截位
				dividend.data[i] = this->data[lengtha - lengthb + i];
			}

			for (int i = lengtha - lengthb; i >= 0; i--) {
				if (dividend < b) { // 如果被除数小于除数,再往后补位
					for (int j = lengthb; j > 0; j--)
						dividend.data[j] = dividend.data[j - 1];
					dividend.data[0] = this->data[i - 1];
					continue;
				}

				low = 0;
				high = 0xffffffff;

				while (low < high) { // 二分查找法查找试商
					mul = (((unsigned long long)high) + low) / 2;
					sub = (b * mul);
					subsequent = (b * (mul + 1));

					if (((sub < dividend) && (subsequent > dividend)) || (sub == dividend))
						break;
					if (subsequent == dividend) {
						mul++;
						sub = subsequent;
						break;
					}

					if ((sub < dividend) && (subsequent < dividend)) {
						low = mul;
						continue;
					}

					if ((sub > dividend) && (subsequent > dividend)) {
						high = mul;
						continue;
					}
				}

				// 试商结果保存到商中去
				quotient.data[i] = mul;
				// 临时被除数变为被除数与试商积的差
				dividend = dividend - sub;

				if ((i - 1) >= 0) { // 临时被除数往后补位
					for (int j = lengthb; j > 0; j--)
						dividend.data[j] = dividend.data[j - 1];
					dividend.data[0] = this->data[i - 1];
				}
			}

			// 判断商的符号
			quotient.sign_ = (this->sign_ == b.sign_);

			return quotient;
		}

		HBigInt HBigInt::operator% (const HBigInt& b) const
		{
			unsigned int mul, low, high;
			HBigInt dividend, quotient, sub, subsequent;
			int lengtha = this->length(), lengthb = b.length();

			if (*this < b) { // 如果被除数小于除数,返回被除数为模
				dividend = *this;
				//余数的商永远与被除数相同
				dividend.sign_ = this->sign_;
				return dividend;
			}

			for (int i = 0; i < lengthb; i++) { //进行除法运算
				dividend.data[i] = this->data[lengtha - lengthb + i];
			}

			for (int i = lengtha - lengthb; i >= 0; i--) {
				if (dividend < b) {
					for (int j = lengthb; j > 0; j--)
						dividend.data[j] = dividend.data[j - 1];
					dividend.data[0] = this->data[i - 1];
					continue;
				}

				low = 0;
				high = 0xffffffff;

				while (low <= high) {
					mul = (((unsigned long long)high) + low) / 2;
					sub = (b * mul);
					subsequent = (b * (mul + 1));

					if (((sub < dividend) && (subsequent > dividend)) || (sub == dividend))
						break;

					if (subsequent == dividend) {
						mul++;
						sub = subsequent;
						break;
					}

					if ((sub < dividend) && (subsequent < dividend)) {
						low = mul;
						continue;
					}

					if ((sub > dividend) && (subsequent > dividend)) {
						high = mul;
						continue;
					}
				}

				quotient.data[i] = mul;
				dividend = dividend - sub;
				if ((i - 1) >= 0) {
					for (int j = lengthb; j > 0; j--)
						dividend.data[j] = dividend.data[j - 1];
					dividend.data[0] = this->data[i - 1];
				}
			}

			//临时被除数即为所求模
			dividend.sign_ = this->sign_;
			return dividend;
		}

		int HBigInt::operator% (const int& b) const
		{
			int len = this->length();
			if (len == 1)
				return this->data[0] % b;

			unsigned long long cur = 0;

			for (int i = len - 1; i >= 0; i--) {
				cur = (cur << 32) + this->data[i];
				cur = cur % b;
			}
			return (int)cur;
		}

		HBigInt HBigInt::operator& (const HBigInt& b) const
		{
			int len = MAX(this->length(), b.length());
			HBigInt res;
			for (int i = 0; i < len; i++)
				res.data[i] = (this->data[i] & b.data[i]);
			res.sign_ = (this->sign_ & b.sign_);
			return res;
		}

		HBigInt HBigInt::operator^ (const HBigInt& b) const
		{
			int len = MAX(this->length(), b.length());
			HBigInt res;
			for (int i = 0; i < len; i++)
				res.data[i] = (this->data[i] ^ b.data[i]);
			res.sign_ = (this->sign_ ^ b.sign_);
			return res;
		}

		HBigInt HBigInt::operator| (const HBigInt& b) const
		{
			int len = MAX(this->length(), b.length());
			HBigInt res;
			for (int i = 0; i < len; i++)
				res.data[i] = (this->data[i] ^ b.data[i]);
			res.sign_ = (this->sign_ ^ b.sign_);
			return res;
		}

		void HBigInt::outstream(std::ostream& out) const
		{
			unsigned int temp, result;
			unsigned int filter = 0xf0000000;
			std::string resStr;
			for (int i = length() - 1; i >= 0; i--) {
				temp = data[i];

				for (int j = 0; j < 8; j++) { // 大数的每一位数字转换成16进制输出
					result = temp & filter;
					result = (result >> 28);
					temp = (temp << 4);
					if (result >= 0 && result <= 9) {
						resStr += (result + '0');
					}
					else {
						switch (result) {
						case 10:
							resStr += 'A';
							break;
						case 11:
							resStr += 'B';
							break;
						case 12:
							resStr += 'C';
							break;
						case 13:
							resStr += 'D';
							break;
						case 14:
							resStr += 'E';
							break;
						case 15:
							resStr += 'F';
							break;
						}
					}
				}
			}

			while (resStr[0] == '0') {
				resStr.erase(0, 1);
			}

			out << resStr;
		}


		std::ostream& operator<< (std::ostream& out, const HBigInt& x)
		{
			x.outstream(out);
			return out;
		}

		std::string HBigInt::toHexString() const
		{
			std::string str;
			unsigned int temp, result;
			unsigned int filter = 0xf0000000;

			for (int i = length() - 1; i >= 0; i--) {
				temp = data[i];

				for (int j = 0; j < 8; j++) { //大数的每一位数字转换成16进制输出
					result = temp & filter;
					result = (result >> 28);
					temp = (temp << 4);
					if (result >= 0 && result <= 9) {
						str += (result + '0');
					}
					else {
						switch (result) {
						case 10:
							str += 'A';
							break;
						case 11:
							str += 'B';
							break;
						case 12:
							str += 'C';
							break;
						case 13:
							str += 'D';
							break;
						case 14:
							str += 'E';
							break;
						case 15:
							str += 'F';
							break;
						}
					}
				}
			}

			while (str[0] == '0') {
				str.erase(0, 1);
			}
			return str;
		}

		void HBigInt::clear()
		{
			for (int i = 0; i < capacity_; i++)
				data[i] = 0;
		}

		int HBigInt::length() const
		{
			int length = capacity_;
			for (int i = capacity_ - 1; i >= 0; i--) {
				//第一位不为0即为LENGTH
				if (data[i] == 0)
					length--;
				else
					break;
			}
			if (length == 0)
				length = 1;
			return length;
		}

		int HBigInt::binaryLen() const {
			int len = length();
			int res = (len - 1) * 32;
			unsigned int tmp = data[len - 1];
			while (tmp > 0) {
				res++;
				tmp = (tmp >> 1);
			}
			return res;
		}

		// 重载移位操作符,向右移a(a < 32)位
		HBigInt& HBigInt::operator >> (const int& a)
		{
			unsigned int bit, filter;
			filter = (1 << a) - 1;
			data[0] = (data[0] >> a);
			for (int i = 1; i < length(); i++) {
				//先将每一位的低a位移到BIT中
				bit = data[i] & filter;
				//再把BIT移到上一位的高位中
				bit = bit << (32 - a);
				data[i - 1] = data[i - 1] | bit;
				data[i] = (data[i] >> a);
			}
			return *this;
		}

		//重载移位操作符,向左移a位
		HBigInt& HBigInt::operator<< (const int& a)
		{
			unsigned int bit, filter;
			filter = (1 << a) - 1;
			filter = (filter << (32 - a));
			int len = length();
			for (int i = len - 1; i >= 0; i--) {
				bit = data[i] & filter;
				bit = bit >> (32 - a);
				data[i + 1] = data[i + 1] | bit;
				data[i] = (data[i] << a);
			}
			return *this;
		}

		//判断大数和一个INT的大小
		bool HBigInt::operator<= (const int& b) const
		{
			for (int i = 1; i < this->length(); i++) {
				if (this->data[i] != 0)
					return false;
			}
			if (this->data[0] <= static_cast<uint32_t>(b))
				return true;
			else
				return false;
		}

		HBigInt HBigInt::powerMode(const HBigInt& n, const HBigInt& p, const HBigInt& m)
		{
			HBigInt temp = p;
			HBigInt r = n % m;
			HBigInt k(1);

			while (!(temp <= 1)) {
				if (temp.odd()) {
					k = (k * r) % m;
				}
				r = (r * r) % m;
				temp >> 1;
			}
			return (r * k) % m;
		}

		char HBigInt::int2Hex(int n)
		{
			char c;
			if (n >= 0 && n <= 9) {
				c = '0' + n;
			}
			else {
				switch (n)
				{
				case 10:
					c = 'A';
					break;
				case 11:
					c = 'B';
					break;
				case 12:
					c = 'C';
					break;
				case 13:
					c = 'D';
					break;
				case 14:
					c = 'E';
					break;
				case 15:
					c = 'F';
					break;
				default:
					break;
				}
			}

			return c;
		}

		int HBigInt::hex2Int(char c)
		{
			if (c >= '0' && c <= '9') {
				return c - '0';
			}
			else {
				switch (c)
				{
				case 'a':
				case 'A':
					return 10;
				case 'b':
				case 'B':
					return 11;
				case 'c':
				case 'C':
					return 12;
				case 'd':
				case 'D':
					return 13;
				case 'e':
				case 'E':
					return 14;
				case 'f':
				case 'F':
					return 15;
				default:
					break;
				}
			}

			std::cout << "BigInt: hexCharToInt error" << std::endl;
			exit(0);
		}

		void HBigInt::random(int bits)
		{
			if (bits < 32) {
				data[0] = (rand() << 17) + (rand() << 2) + rand() % 4;
				unsigned int filter = 1;
				filter = (1 << bits) - 1;
				data[0] = (data[0] & filter);
				return;
			}
			for (int i = 0; i < bits / 32; i++) {
				// 由于RAND()最大只能产生0X7FFF的数,为了能产生32位的随机数
				// 需要3次RAND()操作
				data[i] = (rand() << 17) + (rand() << 2) + rand() % 4;
			}
			data[bits / 32 - 1] = data[bits / 32 - 1] | 0x80000000;
		}


		//产生一个较小的随机大数,其二进制长度为bits的1/4;
		void HBigInt::randomSmall(int bits)
		{
			if (bits < 128) {
				data[0] = (rand() << 17) + (rand() << 2) + rand() % 4;
				unsigned int filter = 1;
				int tmp = bits / 4;
				filter = (1 << tmp) - 1;
				data[0] = (data[0] & filter);
				return;
			}
			for (int i = 0; i < bits / 128; i++) {
				//由于RAND()最大只能产生0X7FFF的数,为了能产生32位的随机数,需要
				//3次RAND()操作
				data[i] = (rand() << 17) + (rand() << 2) + rand() % 4;
			}
			data[bits / 128 - 1] = data[bits / 32 - 1] | 0x80000000;
		}

		HBigInt HBigInt::gcd(const HBigInt& m, const HBigInt& n)
		{
			if (n == 0)
				return m;
			else
				return gcd(n, m%n);
		}

		HBigInt HBigInt::extendedGcd(const HBigInt& a, const HBigInt& b, HBigInt& x, HBigInt& y)
		{
			HBigInt x0 = 1, y0 = 0, x1 = 0, y1 = 1, c = a, d = b;
			HBigInt k, r, t;
			while (!(d == 0)) {
				k = c / d;
				r = c % d;
				t = c;
				c = d;
				d = r;
				t = x0;
				x0 = x1;
				x1 = t - k * x1;
				t = y0;
				y0 = y1;
				y1 = t - k * y1;
			}
			x = x0, y = y0;
			return c;
		}

		HBigInt HBigInt::euclid(HBigInt& E, HBigInt& A)
		{
			HBigInt M, X, Y, I, J;
			int x, y;
			M = A;
			X = 0;
			Y = 1;
			x = y = 1;
			while ((E.length() != 1) || !(E == 0)) {
				I = M / E;
				J = M % E;
				M = E;
				E = J;
				J = Y;
				Y = Y * I;
				if (x == y) {
					if (X > Y)
						Y = X - Y;
					else {
						Y = Y - X;
						y = 0;
					}
				}
				else {
					Y = X + Y;
					x = 1 - x;
					y = 1 - y;
				}
				X = J;
			}
			if (x == 0)
				X = A - X;
			return X;
		}

		HBigIntArray::HBigIntArray(const std::string& plain, int segmentLen)
		{
			segmentLen_ = segmentLen;
			split(plain);
		}

		void HBigIntArray::split(const std::string& plain)
		{
			size_t n = 0, i = 0, j = 0;
			int strLen = segmentLen_ / 8;

			clear();
			const char* p = plain.c_str();

			do {
				char segment[255] = { 0 };

				for (i = n * strLen, j = 0; i < plain.length() && i < (n + 1) * strLen; i++, j++) {
					segment[j] = p[i];
				}

				vector<HBigInt>::push_back(HBigInt(segment, 10));
				n++;

			} while (n * strLen < plain.length());
		}

		std::string HBigIntArray::toHexString()
		{
			std::string str = "";
			std::string tmp = "";

			for (size_t i = 0; i < size(); i++) {
				tmp = (*this)[i].toHexString();
				str = str + tmp;
			}

			return str;
		}

		std::string HBigIntArray::toString()
		{
			std::string str = "";
			std::string tmp = "";

			for (size_t i = 0; i < size(); i++) {
				tmp = (*this)[i].toString();
				str = str + tmp;
			}

			return str;
		}

		void HBigIntArray::pushBack(const std::string& str)
		{
			pushBack(HBigInt(str, RADIX_D));
		}

		void HBigIntArray::pushBack(const HBigInt& bigInt) {
			push_back(bigInt);
		}

		HBigIntArray& HBigIntArray::operator+= (const std::string& str)
		{
			pushBack(str);
			return *this;
		}

		HBigIntArray& HBigIntArray::operator+= (const HBigInt& bigInt)
		{
			pushBack(bigInt);
			return *this;
		}

		HBigIntArray& HBigIntArray::operator+= (const HBigIntArray& st)
		{
			for (HBigIntArray::const_iterator itr = st.begin(); itr != st.end(); itr++)
				pushBack(*itr);
			return *this;
		}

		void HRsaKey::genKey(int bits)
		{
			this->bits_ = bits;
			srand((unsigned)time(NULL));

			// 根据种子生成大素数P/Q
			HBigInt p(generatePrime(bits));
			HBigInt q(generatePrime(bits));

			std::cout << "generate key ......" << std::endl;

			HBigInt t = (p - 1) * (q - 1);
			HBigInt d, y, temp;

			while (true) {
				E_.random(bits);

				// 产生与T互质的E
				while (!(HBigInt::gcd(E_, t) == 1)) {
					E_.random(bits);
				}

				temp = HBigInt::extendedGcd(E_, t, d, y);
				temp = (E_ * d) % t;

				if (temp == 1) {
					break;
				}
			}

			std::cout << "E: " << E_ << std::endl;

			N_ = p * q;
			std::cout << "N: " << N_ << std::endl;

			D_ = d;
			std::cout << "D: " << D_ << std::endl;
		}

		void HRsaKey::pubKey(HBigInt& N, HBigInt& E) const
		{
			N = N_;
			E = E_;
		}

		void HRsaKey::priKey(HBigInt& N, HBigInt& D) const
		{
			N = N_;
			D = D_;
		}

		void HRsaKey::randomPrime(HBigInt& n, int bits)
		{
			int i = 0;
			HBigInt divisor;
			const int length = sizeof(PRIMETABLE) / sizeof(int);

			while (i != length) {

				n.random(bits);

				while (!n.odd()) {
					n.random(bits);
				}

				i = 0;
				for (; i < length; i++) {
					divisor = PRIMETABLE[i];
					if ((n % divisor) == 0)
						break;
				}
			}
		}

		bool HRsaKey::rabinMiller(const HBigInt& n, int bits)
		{
			HBigInt r, a, y;
			unsigned int s, j;
			r = n - 1;
			s = 0;

			while (!r.odd()) {
				s++;
				r >> 1;
			}

			a.randomSmall(bits);

			y = HBigInt::powerMode(a, r, n);

			if ((!(y == 1)) && (!(y == (n - 1)))) {
				j = 1;
				while ((j <= s - 1) && (!(y == (n - 1)))) {
					y = (y * y) % n;
					if (y == 1)
						return false;
					j++;
				}
				if (!(y == (n - 1)))
					return false;
			}
			return true;
		}

		HBigInt HRsaKey::generatePrime(int bits)
		{
			int count = 0;
			HBigInt prime;

			std::cout << "generate prime ...";

			while (count < 5) {
				randomPrime(prime, bits);

				count = 0;
				for (; count < 5; count++) {
					if (!rabinMiller(prime, bits)) {
						std::cout << ".";
						break;
					}
				}
			}
			std::cout << std::endl << prime << std::endl;
			return prime;
		}

		void HRsa::pubKey(HBigInt& N, HBigInt& E) const
		{
			rsaKey_.pubKey(N, E);
		}

		void HRsa::priKey(HBigInt& N, HBigInt& D) const
		{
			rsaKey_.priKey(N, D);
		}

		HBigInt HRsa::encode(const HBigInt& src) const
		{
			HBigInt N, E;
			rsaKey_.pubKey(N, E);
			return HRsa::encode(src, N, E);
		}

		HBigInt HRsa::decode(const HBigInt& src) const
		{
			HBigInt N, D;
			rsaKey_.priKey(N, D);
			return HRsa::decode(src, N, D);
		}

		HBigInt HRsa::encode(const HBigInt& src, const HBigInt& N, const HBigInt& E)
		{
			if (!(src < N)) {
				fprintf(stderr, "RSA Error: Encrypt source is bigger than N\n");
				exit(EXIT_FAILURE);
			}

			return HBigInt::powerMode(src, E, N);
		}

		HBigInt HRsa::decode(const HBigInt& src, const HBigInt& N, const HBigInt& D)
		{
			if (!(src < N)) {
				fprintf(stderr, "RSA Error: Encrypt source is bigger than N\n");
				exit(EXIT_FAILURE);
			}

			return HBigInt::powerMode(src, D, N);
		}

		void HRsa::encode(HBigIntArray& st, const HBigInt& N, const HBigInt& E)
		{
			for (auto& itr : st) {
				itr = HRsa::encode(itr, N, E);
			}
		}

		void HRsa::decode(HBigIntArray& st, const HBigInt& N, const HBigInt& D) {
			for (auto& itr : st) {
				itr = HRsa::decode(itr, N, D);
			}
		}
	}
}
