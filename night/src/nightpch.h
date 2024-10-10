#pragma once

#include <memory>
#include <array>
#include <vector>
#include <deque>
#include <stack>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <queue>
#include <list>
#include <set>
#include <sstream>
#include "core.h"

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#ifdef NIGHT_PLATFORM_WINDOWS
#include <windows.h>
#endif

#define S32_MAX INT_MAX
#define S32_MIN INT_MIN
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(x, lower_bound, upper_bound) MIN(MAX(x, lower_bound), upper_bound)
#define NORMALIZE(x, lower_bound, upper_bound) (abs(x - lower_bound) / (upper_bound - lower_bound))
#define SWAP(x, y) std::swap(x, y)
#define BIT(x) (1 << x)

//#undef NIGHT_USE_DOUBLE_PRECISION

namespace night
{

#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef double real;
#else
	typedef float real;
#endif
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

#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef glm::dvec1  vec1;
	typedef glm::dvec2  vec2;
	typedef glm::dvec3  vec3;
	typedef glm::dvec4  vec4;
#else
	typedef glm::vec1  vec1;
	typedef glm::vec2  vec2;
	typedef glm::vec3  vec3;
	typedef glm::vec4  vec4;
#endif

	typedef glm::ivec1  ivec1;
	typedef glm::ivec2  ivec2;
	typedef glm::ivec2  ivec3;
	typedef glm::ivec2  ivec4;

	typedef glm::uvec1  uvec1;
	typedef glm::uvec2  uvec2;
	typedef glm::uvec2  uvec3;
	typedef glm::uvec2  uvec4;

	typedef glm::fvec1  fvec1;
	typedef glm::fvec2  fvec2;
	typedef glm::fvec2  fvec3;
	typedef glm::fvec2  fvec4;

	typedef glm::dvec1  dvec1;
	typedef glm::dvec2  dvec2;
	typedef glm::dvec2  dvec3;
	typedef glm::dvec2  dvec4;

#ifdef NIGHT_USE_DOUBLE_PRECISION
	typedef glm::dmat2		 mat2;
	typedef glm::dmat2x3	 mat2x3;
	typedef glm::dmat2x4	 mat2x4;
	typedef glm::dmat3x2	 mat3x2;
	typedef glm::dmat3		 mat3;
	typedef glm::dmat3x4	 mat3x4;
	typedef glm::dmat4x2	 mat4x2;
	typedef glm::dmat4x3	 mat4x3;
	typedef glm::dmat4		 mat4;
#else
	typedef glm::mat2		 mat2;
	typedef glm::mat2x3		 mat2x3;
	typedef glm::mat2x4		 mat2x4;
	typedef glm::mat3x2		 mat3x2;
	typedef glm::mat3		 mat3;
	typedef glm::mat3x4		 mat3x4;
	typedef glm::mat4x2		 mat4x2;
	typedef glm::mat4x3		 mat4x3;
	typedef glm::mat4		 mat4;
#endif

	typedef glm::fmat2		 fmat2;
	typedef glm::fmat2x3	 fmat2x3;
	typedef glm::fmat2x4	 fmat2x4;
	typedef glm::fmat3x2	 fmat3x2;
	typedef glm::fmat3		 fmat3;
	typedef glm::fmat3x4	 fmat3x4;
	typedef glm::fmat4x2	 fmat4x2;
	typedef glm::fmat4x3	 fmat4x3;
	typedef glm::fmat4		 fmat4;

	typedef glm::dmat2		 dmat2;
	typedef glm::dmat2x3	 dmat2x3;
	typedef glm::dmat2x4	 dmat2x4;
	typedef glm::dmat3x2	 dmat3x2;
	typedef glm::dmat3		 dmat3;
	typedef glm::dmat3x4	 dmat3x4;
	typedef glm::dmat4x2	 dmat4x2;
	typedef glm::dmat4x3	 dmat4x3;
	typedef glm::dmat4		 dmat4;

	//template<typename T>
	//using ref = std::shared_ptr<T>;

	template<typename T>
	using wref = std::weak_ptr<T>;

	//template<typename T>
	//using uref = std::unique_ptr<T>;

	template<typename T>
	using sref = std::shared_ptr<T>;

	template<typename T>
	struct ref
	{
		ref() = default;

		ref(const ref<T>& other) = default;

		template<typename O>
		ref(const ref<O>& other)
		{
			_ptr = std::static_pointer_cast<T>(other.ptr().lock());
		}

		ref(sref<T> t)
		{
			_ptr = t;
		}

		ref(const nullptr_t& null)
		{
			_ptr = {};
		}

		T* operator->() const
		{
			return _ptr.lock().get();
		}

		T* operator->()
		{
			return _ptr.lock().get();
		}

		u8 operator==(const std::nullptr_t& null) const
		{
			return _ptr.expired();
		}

		u8 operator!=(const std::nullptr_t& null) const
		{
			return !_ptr.expired();
		}

		u8 operator!=(const ref<T>& other) const
		{
			return _ptr.lock() != other._ptr.lock();
		}

		template<typename O>
		u8 operator!=(const ref<O>& other) const
		{
			return _ptr.lock() != other.ptr().lock();
		}

		void operator=(const std::nullptr_t& null)
		{
			_ptr = wref<T>();
		}

		template<typename O>
		void operator=(ref<O> other)
		{
			_ptr = std::static_pointer_cast<T>(other.ptr().lock());
		}

		void operator=(wref<T> other)
		{
			_ptr = other;
		}

		T& operator*()
		{
			return *_ptr.lock();
		}

		inline operator u8 () { return !_ptr.expired(); }

		const wref<T>& ptr() const { return _ptr; }

	private:

		wref<T> _ptr;
	};

	template<typename T1, typename T2>
	using pair = std::pair<T1, T2>;

	template<typename... Args>
	using tuple = std::tuple<Args...>;

	template<typename T>
	using queue = std::queue<T>;

	template<typename T, s32 S>
	using array = std::array<T, S>;

	template<typename T>
	using vector = std::vector<T>;

	template<typename T>
	using deque = std::deque<T>;

	template<typename T>
	using stack = std::stack<T>;

	template<typename T>
	using list = std::list<T>;

	template<typename K, typename T>
	using map = std::map<K, T>;

	template<typename K, typename T>
	using multimap = std::multimap<K, T>;

	template<typename K, typename T>
	using umap = std::unordered_map<K, T>;

	template<typename K, typename T>
	using umultimap = std::unordered_multimap<K, T>;

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

	inline void random_set_seed(u32 seed)
	{
		srand(seed);
	}

	inline s32 random()
	{
		return rand();
	}

	inline s32 random(s32 range)
	{
		return rand() % range;
	}

//#define RANDOM_REAL_MOD 100000
	inline real random(real range)
	{
		return ((real)(rand()) / (real)(RAND_MAX)) * range;

	}

}