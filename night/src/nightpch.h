#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>
#include <list>
#include <set>
#include <sstream>

#ifdef NIGHT_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace light
{
	typedef double real; // TODO: add macro
	typedef float r32;
	typedef double r64;

	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long u64;
	typedef unsigned long long u128;

	typedef char s8;
	typedef short s16;
	typedef int s32;
	typedef long s64;
	typedef long long s128;

	//typedef glm::vec1 vec1;
	//typedef glm::vec2 vec2;
	//typedef glm::vec3 vec3;
	//typedef glm::vec4 vec4;
	//
	//typedef glm::ivec1 ivec1;
	//typedef glm::ivec2 ivec2;
	//typedef glm::ivec2 ivec3;
	//typedef glm::ivec2 ivec4;
	//
	//typedef glm::uvec1 uvec1;
	//typedef glm::uvec2 uvec2;
	//typedef glm::uvec2 uvec3;
	//typedef glm::uvec2 uvec4;
	//
	//typedef glm::dvec1 dvec1;
	//typedef glm::dvec2 dvec2;
	//typedef glm::dvec2 dvec3;
	//typedef glm::dvec2 dvec4;
	//
	//typedef glm::mat2 mat2;
	//typedef glm::mat2x3 mat2x3;
	//typedef glm::mat2x4 mat2x4;
	//typedef glm::mat3x2 mat3x2;
	//typedef glm::mat3 mat3;
	//typedef glm::mat3x4 mat3x4;
	//typedef glm::mat4x2 mat4x2;
	//typedef glm::mat4x3 mat4x3;
	//typedef glm::mat4 mat4;

	template<typename T>
	using ref = std::shared_ptr<T>;

	template<typename T>
	using wref = std::weak_ptr<T>;

	template<typename T1, typename T2>
	using pair = std::pair<T1, T2>;

	template<typename... Args>
	using tuple = std::tuple<Args...>;

	template<typename T>
	using queue = std::queue<T>;

	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using deque = std::deque<T>;

	template<typename T>
	using list = std::list<T>;

	template<typename K, typename T>
	using map = std::map<K, T>;

	template<typename K, typename T>
	using multimap = std::multimap<K, T>;

	template<typename K, typename T>
	using umap = std::unordered_map<K, T>;

	template<typename K, typename T>
	using umultimap = std::unordered_map<K, T>;

	template<typename T>
	using set = std::set<T>;

	template<typename T>
	using uset = std::unordered_set<T>;

	template<typename T>
	using function = std::function<T>;

	using string = std::string;

	template<typename... Args>
	inline auto bind(Args&&... args)
	{
		return std::bind(std::forward<Args&&>(args)...);
	}

	constexpr auto placeholder_1 = std::placeholders::_1;
	constexpr auto placeholder_2 = std::placeholders::_2;
	constexpr auto placeholder_3 = std::placeholders::_3;
	constexpr auto placeholder_4 = std::placeholders::_4;
	constexpr auto placeholder_5 = std::placeholders::_5;
	// TODO: implement rest.

	using sstream = std::stringstream;

	using ostream = std::ostream;

	template<typename T>
	inline string to_string(const T& t)
	{
		return std::to_string(t);
	}

}