#include "whale/util/hrandom.h"

#include <random>

namespace Whale
{
	namespace Util
	{
		namespace HRandom
		{
			uint32_t gen()
			{
				std::random_device rd;
				std::mt19937 mt(rd());
				return mt();
			}
		}
	}
}