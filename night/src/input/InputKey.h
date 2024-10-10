#pragma once

#include "core.h"
#include "EInput.h"

namespace night
{

struct /*NIGHT_API*/ InputKey
{
	union
	{
		struct
		{
			EKey key;
			EMouse mouse;
			EButton button;
			EInputType type;
		};

		u64 hash;
	};


	u8 operator==(const InputKey& rhs) const
	{
		return key == rhs.key && mouse == rhs.mouse && button == rhs.button && type == rhs.type;
	}
};

}

namespace std
{
	template<>
	struct hash<night::InputKey>
	{
		uint64_t operator()(const night::InputKey& key) const
		{
			return hash<uint64_t>()((uint64_t)key.hash);
		}
	};
};