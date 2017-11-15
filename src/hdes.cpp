#include "whale/crypto/hdes.h"

int DES_ByteToBit(char ch, char bit[8]);
int DES_BitToByte(char bit[8], char *ch);

int DES_Char8ToBit64(char ch[8], char bit[64]);
int DES_Bit64ToChar8(char bit[64], char ch[8]);

int DES_PC1_Transform(char key[64], char tempbts[56]);
int DES_PC2_Transform(char key[56], char tempbts[48]);

int DES_IP_Transform(char data[64]);
int DES_IP_1_Transform(char data[64]);

int DES_E_Transform(char data[48]);
int DES_P_Transform(char data[32]);

int DES_SBOX(char data[48]);
int DES_XOR(char R[48], char L[48], int count);

int DES_EncryptBlock(char plainBlock[8], char subKeys[16][48], char cipherBlock[8]);
int DES_DecryptBlock(char cipherBlock[8], char subKeys[16][48], char plainBlock[8]);

int DES_ROL(char data[56], int time);
int DES_Swap(char left[32], char right[32]);
int DES_MakeSubKeys(char key[64], char subKeys[16][48]);

// Initial replacement table IP 
static int DES_IP_Table[64] = {
	57, 49, 41, 33, 25, 17, 9, 1,
	59, 51, 43, 35, 27, 19, 11, 3,
	61, 53, 45, 37, 29, 21, 13, 5,
	63, 55, 47, 39, 31, 23, 15, 7,
	56, 48, 40, 32, 24, 16, 8, 0,
	58, 50, 42, 34, 26, 18, 10, 2,
	60, 52, 44, 36, 28, 20, 12, 4,
	62, 54, 46, 38, 30, 22, 14, 6
};

// Inverse initial permutation table IP^-1 
static int DES_IP_1_Table[64] = {
	39, 7, 47, 15, 55, 23, 63, 31,
	38, 6, 46, 14, 54, 22, 62, 30,
	37, 5, 45, 13, 53, 21, 61, 29,
	36, 4, 44, 12, 52, 20, 60, 28,
	35, 3, 43, 11, 51, 19, 59, 27,
	34, 2, 42, 10, 50, 18, 58, 26,
	33, 1, 41, 9, 49, 17, 57, 25,
	32, 0, 40, 8, 48, 16, 56, 24
};

// Augmented permutation table E  
static int DES_E_Table[48] = {
	31, 0, 1, 2, 3, 4,
	3, 4, 5, 6, 7, 8,
	7, 8, 9, 10, 11, 12,
	11, 12, 13, 14, 15, 16,
	15, 16, 17, 18, 19, 20,
	19, 20, 21, 22, 23, 24,
	23, 24, 25, 26, 27, 28,
	27, 28, 29, 30, 31, 0
};

// Permutation function P  
static int DES_P_Table[32] = {
	15, 6, 19, 20, 28, 11, 27, 16,
	0, 14, 22, 25, 4, 17, 30, 9,
	1, 7, 23, 13, 31, 26, 2, 8,
	18, 12, 29, 5, 21, 10, 3, 24
};

// S Box
static int DES_S[8][4][16] = {
	//S1
	{
		{ 14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7 },
		{ 0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8 },
		{ 4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0 },
		{ 15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13 }
	},
	//S2
	{
		{ 15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10 },
		{ 3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5 },
		{ 0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15 },
		{ 13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9 }
	},
	//S3
	{
		{ 10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8 },
		{ 13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1 },
		{ 13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7 },
		{ 1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12 }
	},
	//S4
	{
		{ 7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15 },
		{ 13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9 },
		{ 10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4 },
		{ 3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14 }
	},
	//S5
	{
		{ 2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9 },
		{ 14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6 },
		{ 4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14 },
		{ 11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3 }
	},
	//S6
	{
		{ 12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11 },
		{ 10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8 },
		{ 9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6 },
		{ 4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13 }
	},
	//S7
	{
		{ 4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1 },
		{ 13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6 },
		{ 1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2 },
		{ 6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12 }
	},
	//S8
	{
		{ 13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7 },
		{ 1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2 },
		{ 7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8 },
		{ 2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11 }
	}
};

// Substitution selection 1               
static int DES_PC_1[56] = {
	56, 48, 40, 32, 24, 16, 8,
	0, 57, 49, 41, 33, 25, 17,
	9, 1, 58, 50, 42, 34, 26,
	18, 10, 2, 59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14,
	6, 61, 53, 45, 37, 29, 21,
	13, 5, 60, 52, 44, 36, 28,
	20, 12, 4, 27, 19, 11, 3
};

// Substitution selection 2 
static int DES_PC_2[48] = {
	13, 16, 10, 23, 0, 4, 2, 27,
	14, 5, 20, 9, 22, 18, 11, 3,
	25, 7, 15, 6, 26, 19, 12, 1,
	40, 51, 30, 36, 46, 54, 29, 39,
	50, 44, 32, 46, 43, 48, 38, 55,
	33, 52, 45, 41, 49, 35, 28, 31
};

// Left shift times 
static int DES_MOVE_TIMES[16] = {
	1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

namespace Whale
{
	namespace Crypto
	{
		namespace HDes
		{
			Whale::Basic::HByteArray encode(const Whale::Basic::HByteArray& plain, const Whale::Basic::HByteArray& key)
			{
				size_t index = 0;
				size_t count = 0;
				char plainBlock[8];
				char cipherBlock[8];	
				Whale::Basic::HByteArray cipher;

				char keyBlock[8];
				memcpy(keyBlock, key.size() < 8 ? "12345678" : key.data(), 8);

				char bKey[64];
				DES_Char8ToBit64(keyBlock, bKey);

				char subKeys[16][48];
				DES_MakeSubKeys(bKey, subKeys);

				while (index + 8 <= plain.size())
				{
					memcpy(plainBlock, &plain[index], 8);
					DES_EncryptBlock(plainBlock, subKeys, cipherBlock);
					Whale::Basic::HByteArray cb(cipherBlock, 8);
					cipher = cipher + Whale::Basic::HByteArray(cipherBlock, 8);
					index += 8;
				}

				if ((count = plain.size() - index))
				{
					memcpy(plainBlock, &plain[index], count);
					memset(plainBlock + count, '\0', 7 - count);
					plainBlock[7] = static_cast<char>(8 - count);
					DES_EncryptBlock(plainBlock, subKeys, cipherBlock);
					cipher = cipher + Whale::Basic::HByteArray(cipherBlock, 8);
				}

				return cipher;
			}

			Whale::Basic::HByteArray decode(const Whale::Basic::HByteArray& cipher, const Whale::Basic::HByteArray& key)
			{
				size_t index = 0;
				size_t count = 0;
				char plainBlock[8];
				char cipherBlock[8];

				Whale::Basic::HByteArray plain;

				char keyBlock[8];
				memcpy(keyBlock, key.size() < 8 ? "12345678" : key.data(), 8);

				char bKey[64];
				DES_Char8ToBit64(keyBlock, bKey);
				
				char subKeys[16][48];
				DES_MakeSubKeys(bKey, subKeys);

				while (1)
				{
					memcpy(cipherBlock, &cipher[index], 8);
					DES_DecryptBlock(cipherBlock, subKeys, plainBlock);

					if (index + 8 < cipher.size())
					{
						plain = plain + Whale::Basic::HByteArray(plainBlock, 8);
						index += 8;
					}
					else 
					{
						break;
					}
				}

				if (plainBlock[7] < 8)
				{
					for (count = 8 - plainBlock[7]; count < 7; count++)
					{
						if (plainBlock[count] != '\0')
						{
							break;
						}
					}
				}

				return plain + Whale::Basic::HByteArray(plainBlock, count == 7 ? 8 - plainBlock[7] : 8);
			}
			
			bool encodeFile(const std::string& plainFile, const std::string& keyFile, const std::string& cipherFile)
			{
				FILE *fPlain = fopen(plainFile.c_str(), "rb");
				if (fPlain == NULL) {
					return false;
				}

				FILE *fKey = fopen(keyFile.c_str(), "rb");
				if (fKey == NULL) {
					return false;
				}

				FILE *fCipher = fopen(cipherFile.c_str(), "wb");
				if (fCipher == NULL) {
					return false;
				}

				char keyBlock[8];
				if (fread(keyBlock, sizeof(char), 8, fKey) != 8) {
					return false;
				}

				fseek(fPlain, 0, SEEK_END);     
				long plainLen = ftell(fPlain);
				rewind(fPlain);  

				char* plainBlock = (char *)malloc(plainLen * sizeof(char));
				fread(plainBlock, sizeof(char), plainLen, fPlain);

				Whale::Basic::HByteArray cipherBlock;
				cipherBlock = encode(Whale::Basic::HByteArray(plainBlock, plainLen), Whale::Basic::HByteArray(keyBlock));
				fwrite(cipherBlock.data(), sizeof(char), cipherBlock.size(), fCipher);

				fclose(fPlain);
				fclose(fCipher);
				fclose(fKey);
				free(plainBlock);

				return true;
			}

			bool decodeFile(const std::string& cipherFile, const std::string& keyFile, const std::string& plainFile)
			{
				FILE *fPlain = fopen(plainFile.c_str(), "wb");
				if (fPlain == NULL) {
					return false;
				}

				FILE *fKey = fopen(keyFile.c_str(), "rb");
				if (fKey == NULL) {
					return false;
				}

				FILE *fCipher = fopen(cipherFile.c_str(), "rb");
				if (fCipher == NULL) {
					return false;
				}

				char keyBlock[8];
				if (fread(keyBlock, sizeof(char), 8, fKey) != 8) {
					return false;
				}

				fseek(fCipher, 0, SEEK_END);     
				int cipherLen = ftell(fCipher);      
				rewind(fCipher);  

				char* cipherBlock = (char *)malloc(cipherLen * sizeof(char));
				fread(cipherBlock, sizeof(char), cipherLen, fCipher);

				Whale::Basic::HByteArray plainBlock = decode(Whale::Basic::HByteArray(cipherBlock, cipherLen), Whale::Basic::HByteArray(keyBlock));
				fwrite(plainBlock.data(), sizeof(char), plainBlock.size(), fPlain);

				fclose(fPlain);
				fclose(fCipher);
				fclose(fKey);
				free(cipherBlock);

				return true;
			}

			Whale::Basic::HByteArray des3Encode(const Whale::Basic::HByteArray& plain, const Whale::Basic::HByteArray& key)
			{
				size_t index = 0;
				size_t count = 0;
				char plainBlock[8], cipherBlock[8], keyBlock[24];
				char bKey[3][64];
				char subKeys[3][16][48];
				Whale::Basic::HByteArray cipher;

				for (int i = 0; i < 3; i++)
				{
					memcpy(keyBlock + i * 8, key.size() < 3 * 8 ? "12345678" : &key[i * 8], 8);
					DES_Char8ToBit64(keyBlock + i * 8, bKey[i]);
					DES_MakeSubKeys(bKey[i], subKeys[i]);
				}

				while (index + 8 <= plain.size())
				{
					memcpy(plainBlock, &plain[index], 8);
					for (int i = 0; i < 3; i++)
					{
						DES_EncryptBlock(plainBlock, subKeys[i], cipherBlock);
					}
					cipher = cipher + Whale::Basic::HByteArray(cipherBlock, 8);
					index += 8;
				}

				if ((count = plain.size() - index))
				{
					memcpy(plainBlock, &plain[index], count);  
					memset(plainBlock + count, '\0', 7 - count);
					plainBlock[7] = static_cast<char>(8 - count);
					{
						for (int i = 0; i < 3; i++)
						{
							DES_EncryptBlock(plainBlock, subKeys[i], cipherBlock);
						}
					}
					cipher = cipher + Whale::Basic::HByteArray(cipherBlock, 8);
				}

				return cipher;
			}

			Whale::Basic::HByteArray des3Decode(const Whale::Basic::HByteArray& cipher, const Whale::Basic::HByteArray& key)
			{
				size_t index = 0;
				size_t count = 0;
				int i;
				char plainBlock[8], cipherBlock[8], keyBlock[24];
				char bKey[3][64];				
				char subKeys[3][16][48];
				Whale::Basic::HByteArray plain;

				for (int i = 0; i < 3; i++)
				{
					memcpy(keyBlock + i * 8, key.size() < 3 * 8 ? "12345678" : &key[i * 8], 8);
					DES_Char8ToBit64(keyBlock + i * 8, bKey[i]);
					DES_MakeSubKeys(bKey[i], subKeys[i]);
				}

				while (1)
				{
					memcpy(cipherBlock, &cipher[index], 8);
					for (i = 0; i < 3; i++)
					{
						DES_DecryptBlock(cipherBlock, subKeys[i], plainBlock);
					}
					if (index + 8 < cipher.size())
					{
						plain = plain + Whale::Basic::HByteArray(plainBlock, 8);
						index += 8;
					}
					else 
					{
						break;
					}
				}

				if (plainBlock[7] < 8)
				{
					for (count = 8 - plainBlock[7]; count < 7; count++)
					{
						if (plainBlock[count] != '\0')
						{
							break;
						}
					}
				}

				return plain + Whale::Basic::HByteArray(plainBlock, count == 7 ? 8 - plainBlock[7] : 8);
			}

			bool des3EncodeFile(const std::string& plainFile, const std::string& keyFile, const std::string& cipherFile)
			{
				FILE *fPlain = fopen(plainFile.c_str(), "rb");
				if (fPlain == NULL) {
					return false;
				}

				FILE *fKey = fopen(keyFile.c_str(), "rb");
				if (fKey == NULL) {
					return false;
				}

				FILE *fCipher = fopen(cipherFile.c_str(), "wb");
				if (fCipher == NULL) {
					return false;
				}

				char keyBlock[8];
				if (fread(keyBlock, sizeof(char), 8, fKey) != 8) {
					return false;
				}

				fseek(fPlain, 0, SEEK_END);
				long plainLen = ftell(fPlain);
				rewind(fPlain);

				char* plainBlock = (char *)malloc(plainLen * sizeof(char));
				fread(plainBlock, sizeof(char), plainLen, fPlain);

				Whale::Basic::HByteArray cipherBlock;
				cipherBlock = des3Encode(Whale::Basic::HByteArray(plainBlock, plainLen), Whale::Basic::HByteArray(keyBlock));
				fwrite(cipherBlock.data(), sizeof(char), cipherBlock.size(), fCipher);

				fclose(fPlain);
				fclose(fCipher);
				fclose(fKey);
				free(plainBlock);

				return true;
			}

			bool des3DecodeFile(const std::string& cipherFile, const std::string& keyFile, const std::string& plainFile)
			{
				FILE *fPlain = fopen(plainFile.c_str(), "wb");
				if (fPlain == NULL) {
					return false;
				}

				FILE *fKey = fopen(keyFile.c_str(), "rb");
				if (fKey == NULL) {
					return false;
				}

				FILE *fCipher = fopen(cipherFile.c_str(), "rb");
				if (fCipher == NULL) {
					return false;
				}

				char keyBlock[8];
				if (fread(keyBlock, sizeof(char), 8, fKey) != 8) {
					return false;
				}

				fseek(fCipher, 0, SEEK_END);
				int cipherLen = ftell(fCipher);
				rewind(fCipher);

				char* cipherBlock = (char *)malloc(cipherLen * sizeof(char));
				fread(cipherBlock, sizeof(char), cipherLen, fCipher);

				Whale::Basic::HByteArray plainBlock = des3Decode(Whale::Basic::HByteArray(cipherBlock, cipherLen), Whale::Basic::HByteArray(keyBlock));
				fwrite(plainBlock.data(), sizeof(char), plainBlock.size(), fPlain);

				fclose(fPlain);
				fclose(fCipher);
				fclose(fKey);
				free(cipherBlock);

				return true;
			}
		}
	}
}

// Bytes converted to binary   
int DES_ByteToBit(char ch, char bit[8])
{
	int cnt;
	for (cnt = 0; cnt < 8; cnt++)
	{
		*(bit + cnt) = (ch >> cnt) & 1;
	}
	return 0;
}

// Binary to byte
int DES_BitToByte(char bit[8], char *ch)
{
	int cnt;
	for (cnt = 0; cnt < 8; cnt++)
	{
		*ch |= *(bit + cnt) << cnt;
	}
	return 0;
}

// Convert a string of length 8 to a binary bit string   
int DES_Char8ToBit64(char ch[8], char bit[64])
{
	int cnt;
	for (cnt = 0; cnt < 8; cnt++)
	{
		DES_ByteToBit(*(ch + cnt), bit + (cnt << 3));
	}
	return 0;
}

// Converts a binary bit string to a string of length 8    
int DES_Bit64ToChar8(char bit[64], char ch[8])
{
	int cnt;
	memset(ch, 0, 8);
	for (cnt = 0; cnt < 8; cnt++)
	{
		DES_BitToByte(bit + (cnt << 3), ch + cnt);
	}
	return 0;
}

// Generating subkey       
int DES_MakeSubKeys(char key[64], char subKeys[16][48])
{
	char temp[56];
	int cnt;
	DES_PC1_Transform(key, temp);
	for (cnt = 0; cnt < 16; cnt++)
	{
		DES_ROL(temp, DES_MOVE_TIMES[cnt]);
		DES_PC2_Transform(temp, subKeys[cnt]);
	}
	return 0;
}

// Key exchange 1 
int DES_PC1_Transform(char key[64], char tempbts[56])
{
	int cnt;
	for (cnt = 0; cnt < 56; cnt++)
	{
		tempbts[cnt] = key[DES_PC_1[cnt]];
	}
	return 0;
}

// Key exchange 2     
int DES_PC2_Transform(char key[56], char tempbts[48])
{
	int cnt;
	for (cnt = 0; cnt < 48; cnt++)
	{
		tempbts[cnt] = key[DES_PC_2[cnt]];
	}
	return 0;
}

// Loop left shift      
int DES_ROL(char data[56], int time)
{
	char temp[56];
	// Save the bit that will be moved to the right side of the loop   
	memcpy(temp, data, time);
	memcpy(temp + time, data + 28, time);
	// Top 28 move       
	memcpy(data, data + time, 28 - time);
	memcpy(data + 28 - time, temp, time);
	// Posterior 28 position movement    
	memcpy(data + 28, data + 28 + time, 28 - time);
	memcpy(data + 56 - time, temp + time, time);

	return 0;
}

// IP permutation
int DES_IP_Transform(char data[64])
{
	int cnt;
	char temp[64];
	for (cnt = 0; cnt < 64; cnt++)
	{
		temp[cnt] = data[DES_IP_Table[cnt]];
	}
	memcpy(data, temp, 64);
	return 0;
}

// IP inverse permutation     
int DES_IP_1_Transform(char data[64])
{
	int cnt;
	char temp[64];
	for (cnt = 0; cnt < 64; cnt++)
	{
		temp[cnt] = data[DES_IP_1_Table[cnt]];
	}
	memcpy(data, temp, 64);
	return 0;
}

// Extended permutation  
int DES_E_Transform(char data[48])
{
	int cnt;
	char temp[48];
	for (cnt = 0; cnt < 48; cnt++)
	{
		temp[cnt] = data[DES_E_Table[cnt]];
	}
	memcpy(data, temp, 48);
	return 0;
}

// P permutation    
int DES_P_Transform(char data[32])
{
	int cnt;
	char temp[32];
	for (cnt = 0; cnt < 32; cnt++)
	{
		temp[cnt] = data[DES_P_Table[cnt]];
	}
	memcpy(data, temp, 32);
	return 0;
}

// XOR   
int DES_XOR(char R[48], char L[48], int count)
{
	int cnt;
	for (cnt = 0; cnt < count; cnt++)
	{
		R[cnt] ^= L[cnt];
	}
	return 0;
}

// S box replacement     
int DES_SBOX(char data[48])
{
	int cnt;
	int line, row, output;
	int cur1, cur2;
	for (cnt = 0; cnt < 8; cnt++)
	{
		cur1 = cnt * 6;
		cur2 = cnt << 2;
		// Calculates rows and columns in the S box   
		line = (data[cur1] << 1) + data[cur1 + 5];
		row = (data[cur1 + 1] << 3) + (data[cur1 + 2] << 2) + (data[cur1 + 3] << 1) + data[cur1 + 4];
		output = DES_S[cnt][line][row];
		// Into 2 hexadecimal   
		data[cur2] = (output & 0X08) >> 3;
		data[cur2 + 1] = (output & 0X04) >> 2;
		data[cur2 + 2] = (output & 0X02) >> 1;
		data[cur2 + 3] = output & 0x01;
	}
	return 0;
}

// exchange   
int DES_Swap(char left[32], char right[32])
{
	char temp[32];
	memcpy(temp, left, 32);
	memcpy(left, right, 32);
	memcpy(right, temp, 32);
	return 0;
}

// Encrypting a single packet
int DES_EncryptBlock(char plainBlock[8], char subKeys[16][48], char cipherBlock[8])
{
	char plainBits[64];
	char copyRight[48];
	int cnt;

	DES_Char8ToBit64(plainBlock, plainBits);
	DES_IP_Transform(plainBits);

	//  16 wheel iteration  
	for (cnt = 0; cnt < 16; cnt++)
	{
		memcpy(copyRight, plainBits + 32, 32);
		// The right half is expanded and replaced from 32 bit to 48 bit    
		DES_E_Transform(copyRight);
		// XOR operation is performed with the right part of the right part   
		DES_XOR(copyRight, subKeys[cnt], 48);
		// XOR results into the S box and outputs 32 bit results   
		DES_SBOX(copyRight);
		// P permutation 
		DES_P_Transform(copyRight);
		// XOR the left half of the text with the right part    
		DES_XOR(plainBits, copyRight, 32);
		if (cnt != 16 - 1)
		{
			// Eventually complete the exchange of the left and right
			DES_Swap(plainBits, plainBits + 32);
		}
	}
	// Inverse initial permutation (IP^1 permutation)    
	DES_IP_1_Transform(plainBits);
	DES_Bit64ToChar8(plainBits, cipherBlock);
	return 0;
}

// Decrypt individual packets 
int DES_DecryptBlock(char cipherBlock[8], char subKeys[16][48], char plainBlock[8])
{
	char cipherBits[64];
	char copyRight[48];
	int cnt;

	DES_Char8ToBit64(cipherBlock, cipherBits);
	// Initial permutation (IP permutation)   
	DES_IP_Transform(cipherBits);

	// 16 wheel iteration   
	for (cnt = 16 - 1; cnt >= 0; cnt--)
	{
		memcpy(copyRight, cipherBits + 32, 32);
		// The right half is expanded and replaced from 32 bit to 48 bit  
		DES_E_Transform(copyRight);
		// XOR operation is performed with the right part of the right part   
		DES_XOR(copyRight, subKeys[cnt], 48);
		// XOR results into the S box and outputs 32 bit results  
		DES_SBOX(copyRight);
		// P permutation   
		DES_P_Transform(copyRight);
		// XOR the left half of the text with the right part  
		DES_XOR(cipherBits, copyRight, 32);
		if (cnt != 0)
		{
			// Eventually complete the exchange of the left and right  
			DES_Swap(cipherBits, cipherBits + 32);
		}
	}
	// Inverse initial permutation (IP^1 permutation)  
	DES_IP_1_Transform(cipherBits);
	DES_Bit64ToChar8(cipherBits, plainBlock);
	return 0;
}