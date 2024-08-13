#ifndef temp_test
#define temp_test

#include <vector>

std::vector<std::vector<unsigned char>> test_R =
{
	{
		255, 255, 255, // 255,
		 16,  16,  16,  16,
		 64,  64,  64, // 64,
		255, 255
	},
	{
		127, 114, 107, // 100,
		 83,  73,  70,  70,
		 74,  66,  68, // 63,
		 90, 110
	},
	{
		 61,  47,  50, // 41,
		 62,  40,  44,  31,
		 73,  48,  53, // 39,
		 31,  51
	},
	{
		226, 230, 223, // 228,
		191, 198, 183, 192,
		202, 208, 192, // 202,
		218, 238
	},
	{
		 97,  95, 117, // 111,
		157, 155, 165, 162,
		198, 197, 199, // 198,
		101, 121
	},
	{
		152, 154, 133, // 149,
		129, 130, 110, 124,
		 19,  22,  16, // 17,
		139, 159
	}
};

std::vector<std::vector<unsigned char>> test_G =
{
	{
		255, 255, 255,
		 16,  16,  16,
		 64,  64,  64,
		 16,  16
	},
	{
		127, 114, 107,
		 83,  73,  70,
		 74,  66,  68,
		 55,  85
	},
	{
		 61,  47,  50,
		 62,  40,  44,
		 73,  48,  53,
		 16,  46
	},
	{
		226, 230, 223,
		191, 198, 183,
		202, 208, 192,
		177, 207
	},
	{
		 97,  95, 117,
		157, 155, 165,
		198, 197, 199,
		147, 177
	},
	{
		152, 154, 133,
		129, 130, 110,
		 19,  22,  16,
		114, 134
	}
};

std::vector<std::vector<unsigned char>> test_B =
{
	{
		255, 255, 255, 255,
		 16,  16,  16,  16,
		 64,  64,  64,
		 64,  64
	},
	{
		127, 114, 107, 100,
		 83,  73,  70,  70,
		 74,  66,  68,
		 58,  68
	},
	{
		 61,  47,  50,  41,
		 62,  40,  44,  31,
		 73,  48,  53,
		 34,  44
	},
	{
		226, 230, 223, 228,
		191, 198, 183, 192,
		202, 208, 192,
		197, 207
	},
	{
		 97,  95, 117, 111,
		157, 155, 165, 162,
		198, 197, 199,
		193, 203
	},
	{
		152, 154, 133, 149,
		129, 130, 110, 124,
		 19,  22,  16,
		 12,  22
	}
};

#endif
