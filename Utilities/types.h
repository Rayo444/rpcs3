#pragma once // No BOM and only basic ASCII in this header, or a neko will die

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#include <immintrin.h>
#include <emmintrin.h>

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>
#include <chrono>
#include <limits>
#include <array>

using std::chrono::steady_clock;

using namespace std::literals;

#ifdef _MSC_VER
#if !defined(__cpp_lib_bitops) && _MSC_VER < 1928
#define __cpp_lib_bitops
#endif
#endif
#include <bit>

#ifndef __has_builtin
	#define __has_builtin(x) 0
#endif

#ifdef _MSC_VER
#define SAFE_BUFFERS __declspec(safebuffers)
#define NEVER_INLINE __declspec(noinline)
#define FORCE_INLINE __forceinline
#else // not _MSC_VER
#define SAFE_BUFFERS __attribute__((no_stack_protector))
#define NEVER_INLINE __attribute__((noinline)) inline
#define FORCE_INLINE __attribute__((always_inline)) inline
#endif // _MSC_VER

#define CHECK_SIZE(type, size) static_assert(sizeof(type) == size, "Invalid " #type " type size")
#define CHECK_ALIGN(type, align) static_assert(alignof(type) == align, "Invalid " #type " type alignment")
#define CHECK_MAX_SIZE(type, size) static_assert(sizeof(type) <= size, #type " type size is too big")
#define CHECK_SIZE_ALIGN(type, size, align) CHECK_SIZE(type, size); CHECK_ALIGN(type, align)

#define DECLARE(...) decltype(__VA_ARGS__) __VA_ARGS__

#define STR_CASE(...) case __VA_ARGS__: return #__VA_ARGS__

#if defined(_DEBUG) || defined(_AUDIT)
#define AUDIT(...) (static_cast<void>(ensure(__VA_ARGS__)))
#else
#define AUDIT(...) (static_cast<void>(0))
#endif

#if __cpp_lib_bit_cast >= 201806L
#include <bit>
#else
namespace std
{
	template <class To, class From, typename = std::enable_if_t<sizeof(To) == sizeof(From)>>
	constexpr To bit_cast(const From& from) noexcept
	{
		static_assert(sizeof(To) == sizeof(From), "std::bit_cast<>: incompatible type size");

		if constexpr ((std::is_same_v<std::remove_const_t<To>, std::remove_const_t<From>> && std::is_constructible_v<To, From>) || (std::is_integral_v<From> && std::is_integral_v<To>))
		{
			return static_cast<To>(from);
		}

		To result{};
		std::memcpy(&result, &from, sizeof(From));
		return result;
	}
}
#endif

using schar  = signed char;
using uchar  = unsigned char;
using ushort = unsigned short;
using uint   = unsigned int;
using ulong  = unsigned long;
using ullong = unsigned long long;
using llong  = long long;

#if __APPLE__
using uptr = std::uint64_t;
#else
using uptr = std::uintptr_t;
#endif

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

#if __APPLE__
namespace std
{
	template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
	constexpr int countr_zero(T x) noexcept
	{
		if (x == 0)
			return sizeof(T) * 8;
		if constexpr (sizeof(T) <= sizeof(uint))
			return __builtin_ctz(x);
		else if constexpr (sizeof(T) <= sizeof(ulong))
			return __builtin_ctzl(x);
		else if constexpr (sizeof(T) <= sizeof(ullong))
			return __builtin_ctzll(x);
		else
			static_assert(sizeof(T) <= sizeof(ullong));
	}

	template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
	constexpr int countr_one(T x) noexcept
	{
		return countr_zero<T>(~x);
	}

	template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
	constexpr int countl_zero(T x) noexcept
	{
		if (x == 0)
			return sizeof(T) * 8;
		if constexpr (sizeof(T) <= sizeof(uint))
			return __builtin_clz(x) - (sizeof(uint) - sizeof(T)) * 8;
		else if constexpr (sizeof(T) <= sizeof(ulong))
			return __builtin_clzl(x) - (sizeof(ulong) - sizeof(T)) * 8;
		else if constexpr (sizeof(T) <= sizeof(ullong))
			return __builtin_clzll(x) - (sizeof(ullong) - sizeof(T)) * 8;
		else
			static_assert(sizeof(T) <= sizeof(ullong));
	}

	template <typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
	constexpr int countl_one(T x) noexcept
	{
		return countl_zero<T>(~x);
	}
}
#endif

// Get integral type from type size
template <std::size_t N>
struct get_int_impl
{
};

template <>
struct get_int_impl<sizeof(u8)>
{
	using utype = u8;
	using stype = s8;
};

template <>
struct get_int_impl<sizeof(u16)>
{
	using utype = u16;
	using stype = s16;
};

template <>
struct get_int_impl<sizeof(u32)>
{
	using utype = u32;
	using stype = s32;
};

template <>
struct get_int_impl<sizeof(u64)>
{
	using utype = u64;
	using stype = s64;
};

template <std::size_t N>
using get_uint_t = typename get_int_impl<N>::utype;

template <std::size_t N>
using get_sint_t = typename get_int_impl<N>::stype;

template <typename T>
std::remove_cvref_t<T> as_rvalue(T&& obj)
{
	return std::forward<T>(obj);
}

// Formatting helper, type-specific preprocessing for improving safety and functionality
template <typename T, typename = void>
struct fmt_unveil;

template <typename Arg>
using fmt_unveil_t = typename fmt_unveil<Arg>::type;

struct fmt_type_info;

namespace fmt
{
	template <typename... Args>
	const fmt_type_info* get_type_info();
}

template <typename T, std::size_t Align>
class atomic_t;

// Extract T::simple_type if available, remove cv qualifiers
template <typename T, typename = void>
struct simple_type_helper
{
	using type = typename std::remove_cv<T>::type;
};

template <typename T>
struct simple_type_helper<T, std::void_t<typename T::simple_type>>
{
	using type = typename T::simple_type;
};

template <typename T>
using simple_t = typename simple_type_helper<T>::type;

// Bool type equivalent
class b8
{
	u8 m_value;

public:
	b8() = default;

	constexpr b8(bool value) noexcept
		: m_value(value)
	{
	}

	constexpr operator bool() const noexcept
	{
		return m_value != 0;
	}

	constexpr bool set(bool value) noexcept
	{
		m_value = value;
		return value;
	}
};

#ifndef _MSC_VER
using u128 = __uint128_t;
using s128 = __int128_t;
#else

// Unsigned 128-bit integer implementation (TODO)
struct alignas(16) u128
{
	u64 lo, hi;

	u128() noexcept = default;

	constexpr u128(u64 l) noexcept
		: lo(l)
		, hi(0)
	{
	}

	constexpr explicit operator bool() const noexcept
	{
		return !!(lo | hi);
	}

	constexpr explicit operator u64() const noexcept
	{
		return lo;
	}

	constexpr friend u128 operator+(const u128& l, const u128& r)
	{
		u128 value = l;
		value += r;
		return value;
	}

	constexpr friend u128 operator-(const u128& l, const u128& r)
	{
		u128 value = l;
		value -= r;
		return value;
	}

	constexpr u128 operator+() const
	{
		return *this;
	}

	constexpr u128 operator-() const
	{
		u128 value{};
		value -= *this;
		return value;
	}

	constexpr u128& operator++()
	{
		*this += 1;
		return *this;
	}

	constexpr u128 operator++(int)
	{
		u128 value = *this;
		*this += 1;
		return value;
	}

	constexpr u128& operator--()
	{
		*this -= 1;
		return *this;
	}

	constexpr u128 operator--(int)
	{
		u128 value = *this;
		*this -= 1;
		return value;
	}

	constexpr u128 operator<<(u128 shift_value) const
	{
		u128 value = *this;
		value <<= shift_value;
		return value;
	}

	constexpr u128 operator>>(u128 shift_value) const
	{
		u128 value = *this;
		value >>= shift_value;
		return value;
	}

	constexpr u128 operator~() const
	{
		u128 value{};
		value.lo = ~lo;
		value.hi = ~hi;
		return value;
	}

	constexpr friend u128 operator&(const u128& l, const u128& r)
	{
		u128 value{};
		value.lo = l.lo & r.lo;
		value.hi = l.hi & r.hi;
		return value;
	}

	constexpr friend u128 operator|(const u128& l, const u128& r)
	{
		u128 value{};
		value.lo = l.lo | r.lo;
		value.hi = l.hi | r.hi;
		return value;
	}

	constexpr friend u128 operator^(const u128& l, const u128& r)
	{
		u128 value{};
		value.lo = l.lo ^ r.lo;
		value.hi = l.hi ^ r.hi;
		return value;
	}

	constexpr u128& operator+=(const u128& r)
	{
		if (std::is_constant_evaluated())
		{
			lo += r.lo;
			hi += r.hi + (lo < r.lo);
		}
		else
		{
			_addcarry_u64(_addcarry_u64(0, r.lo, lo, &lo), r.hi, hi, &hi);
		}

		return *this;
	}

	constexpr u128& operator-=(const u128& r)
	{
		if (std::is_constant_evaluated())
		{
			hi -= r.hi + (lo < r.lo);
			lo -= r.lo;
		}
		else
		{
			_subborrow_u64(_subborrow_u64(0, lo, r.lo, &lo), hi, r.hi, &hi);
		}

		return *this;
	}

	constexpr u128& operator<<=(const u128& r)
	{
		if (std::is_constant_evaluated())
		{
			if (r.hi == 0 && r.lo < 64)
			{
				hi = (hi << r.lo) | (lo >> (64 - r.lo));
				lo = (lo << r.lo);
				return *this;
			}
			else if (r.hi == 0 && r.lo < 128)
			{
				hi = (lo << (r.lo - 64));
				lo = 0;
				return *this;
			}
		}

		const u64 v0 = lo << (r.lo & 63);
		const u64 v1 = __shiftleft128(lo, hi, static_cast<uchar>(r.lo));
		lo = (r.lo & 64) ? 0 : v0;
		hi = (r.lo & 64) ? v0 : v1;
		return *this;
	}

	constexpr u128& operator>>=(const u128& r)
	{
		if (std::is_constant_evaluated())
		{
			if (r.hi == 0 && r.lo < 64)
			{
				lo = (lo >> r.lo) | (hi << (64 - r.lo));
				hi = (hi >> r.lo);
				return *this;
			}
			else if (r.hi == 0 && r.lo < 128)
			{
				lo = (hi >> (r.lo - 64));
				hi = 0;
				return *this;
			}
		}

		const u64 v0 = hi >> (r.lo & 63);
		const u64 v1 = __shiftright128(lo, hi, static_cast<uchar>(r.lo));
		lo = (r.lo & 64) ? v0 : v1;
		hi = (r.lo & 64) ? 0 : v0;
		return *this;
	}

	constexpr u128& operator&=(const u128& r)
	{
		lo &= r.lo;
		hi &= r.hi;
		return *this;
	}

	constexpr u128& operator|=(const u128& r)
	{
		lo |= r.lo;
		hi |= r.hi;
		return *this;
	}

	constexpr u128& operator^=(const u128& r)
	{
		lo ^= r.lo;
		hi ^= r.hi;
		return *this;
	}
};

// Signed 128-bit integer implementation (TODO)
struct alignas(16) s128
{
	u64 lo;
	s64 hi;

	s128() = default;

	constexpr s128(s64 l)
		: hi(l >> 63)
		, lo(l)
	{
	}

	constexpr s128(u64 l)
		: hi(0)
		, lo(l)
	{
	}
};
#endif

CHECK_SIZE_ALIGN(u128, 16, 16);
CHECK_SIZE_ALIGN(s128, 16, 16);

template <>
struct get_int_impl<16>
{
	using utype = u128;
};

// Return magic value for any unsigned type
constexpr inline struct umax_helper
{
	constexpr umax_helper() noexcept = default;

	template <typename T, typename S = simple_t<T>, typename = std::enable_if_t<std::is_unsigned_v<S>>>
	explicit constexpr operator T() const
	{
		return std::numeric_limits<S>::max();
	}

	template <typename T, typename S = simple_t<T>, typename = std::enable_if_t<std::is_unsigned_v<S>>>
	constexpr bool operator==(const T& rhs) const
	{
		return rhs == std::numeric_limits<S>::max();
	}

#if __cpp_impl_three_way_comparison >= 201711 && !__INTELLISENSE__
#else
	template <typename T>
	friend constexpr std::enable_if_t<std::is_unsigned_v<simple_t<T>>, bool> operator==(const T& lhs, const umax_helper& rhs)
	{
		return lhs == std::numeric_limits<simple_t<T>>::max();
	}
#endif

#if __cpp_impl_three_way_comparison >= 201711
#else
	template <typename T, typename S = simple_t<T>, typename = std::enable_if_t<std::is_unsigned_v<S>>>
	constexpr bool operator!=(const T& rhs) const
	{
		return rhs != std::numeric_limits<S>::max();
	}

	template <typename T>
	friend constexpr std::enable_if_t<std::is_unsigned_v<simple_t<T>>, bool> operator!=(const T& lhs, const umax_helper& rhs)
	{
		return lhs != std::numeric_limits<simple_t<T>>::max();
	}
#endif
} umax;

using f32 = float;
using f64 = double;

union alignas(2) f16
{
	u16 _u16;
	u8 _u8[2];

	explicit f16(u16 raw)
	{
		_u16 = raw;
	}

	explicit operator f32() const
	{
		// See http://stackoverflow.com/a/26779139
		// The conversion doesn't handle NaN/Inf
		u32 raw = ((_u16 & 0x8000) << 16) |             // Sign (just moved)
				  (((_u16 & 0x7c00) + 0x1C000) << 13) | // Exponent ( exp - 15 + 127)
				  ((_u16 & 0x03FF) << 13);              // Mantissa

		return std::bit_cast<f32>(raw);
	}
};

CHECK_SIZE_ALIGN(f16, 2, 2);

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
constexpr T align(T value, ullong align)
{
	return static_cast<T>((value + (align - 1)) & (0 - align));
}

// General purpose aligned division, the result is rounded up not truncated
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
constexpr T aligned_div(T value, ullong align)
{
	return static_cast<T>((value + align - 1) / align);
}

// General purpose aligned division, the result is rounded to nearest
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr T rounded_div(T value, std::conditional_t<std::is_signed<T>::value, llong, ullong> align)
{
	if constexpr (std::is_unsigned<T>::value)
	{
		return static_cast<T>((value + (align / 2)) / align);
	}

	return static_cast<T>((value + (value < 0 ? 0 - align : align) / 2) / align);
}

template <typename T, typename T2>
inline u32 offset32(T T2::*const mptr)
{
#ifdef _MSC_VER
	return std::bit_cast<u32>(mptr);
#elif __GNUG__
	return std::bit_cast<std::size_t>(mptr);
#else
	static_assert(sizeof(mptr) == 0, "Unsupported pointer-to-member size");
#endif
}

template <typename T>
struct offset32_array
{
	static_assert(std::is_array<T>::value, "Invalid pointer-to-member type (array expected)");

	template <typename Arg>
	static inline u32 index32(const Arg& arg)
	{
		return u32{sizeof(std::remove_extent_t<T>)} * static_cast<u32>(arg);
	}
};

template <typename T, std::size_t N>
struct offset32_array<std::array<T, N>>
{
	template <typename Arg>
	static inline u32 index32(const Arg& arg)
	{
		return u32{sizeof(T)} * static_cast<u32>(arg);
	}
};

template <typename Arg>
struct offset32_detail;

template <typename T, typename T2, typename Arg, typename... Args>
inline u32 offset32(T T2::*const mptr, const Arg& arg, const Args&... args)
{
	return offset32_detail<Arg>::offset32(mptr, arg, args...);
}

template <typename Arg>
struct offset32_detail
{
	template <typename T, typename T2, typename... Args>
	static inline u32 offset32(T T2::*const mptr, const Arg& arg, const Args&... args)
	{
		return ::offset32(mptr, args...) + offset32_array<T>::index32(arg);
	}
};

template <typename T3, typename T4>
struct offset32_detail<T3 T4::*>
{
	template <typename T, typename T2, typename... Args>
	static inline u32 offset32(T T2::*const mptr, T3 T4::*const mptr2, const Args&... args)
	{
		return ::offset32(mptr) + ::offset32(mptr2, args...);
	}
};

// Helper function, used by ""_u16, ""_u32, ""_u64
constexpr u32 to_u8(char c)
{
	return static_cast<u8>(c);
}

// Convert 1-2-byte string to u16 value like reinterpret_cast does
constexpr u16 operator""_u16(const char* s, std::size_t /*length*/)
{
	if constexpr (std::endian::little == std::endian::native)
	{
		return static_cast<u16>(to_u8(s[1]) << 8 | to_u8(s[0]));
	}
	else
	{
		return static_cast<u16>(to_u8(s[0]) << 8 | to_u8(s[1]));
	}
}

// Convert 3-4-byte string to u32 value like reinterpret_cast does
constexpr u32 operator""_u32(const char* s, std::size_t /*length*/)
{
	if constexpr (std::endian::little == std::endian::native)
	{
		return to_u8(s[3]) << 24 | to_u8(s[2]) << 16 | to_u8(s[1]) << 8 | to_u8(s[0]);
	}
	else
	{
		return to_u8(s[0]) << 24 | to_u8(s[1]) << 16 | to_u8(s[2]) << 8 | to_u8(s[3]);
	}
}

// Convert 5-6-byte string to u64 value like reinterpret_cast does
constexpr u64 operator""_u48(const char* s, std::size_t /*length*/)
{
	if constexpr (std::endian::little == std::endian::native)
	{
		return static_cast<u64>(to_u8(s[5]) << 8 | to_u8(s[4])) << 32 | to_u8(s[3]) << 24 | to_u8(s[2]) << 16 | to_u8(s[1]) << 8 | to_u8(s[0]);
	}
	else
	{
		return static_cast<u64>(to_u8(s[0]) << 8 | to_u8(s[1])) << 32 | to_u8(s[2]) << 24 | to_u8(s[3]) << 16 | to_u8(s[4]) << 8 | to_u8(s[5]);
	}
}

// Convert 7-8-byte string to u64 value like reinterpret_cast does
constexpr u64 operator""_u64(const char* s, std::size_t /*length*/)
{
	if constexpr (std::endian::little == std::endian::native)
	{
		return static_cast<u64>(to_u8(s[7]) << 24 | to_u8(s[6]) << 16 | to_u8(s[5]) << 8 | to_u8(s[4])) << 32 | to_u8(s[3]) << 24 | to_u8(s[2]) << 16 | to_u8(s[1]) << 8 | to_u8(s[0]);
	}
	else
	{
		return static_cast<u64>(to_u8(s[0]) << 24 | to_u8(s[1]) << 16 | to_u8(s[2]) << 8 | to_u8(s[3])) << 32 | to_u8(s[4]) << 24 | to_u8(s[5]) << 16 | to_u8(s[6]) << 8 | to_u8(s[7]);
	}
}

#if !defined(__INTELLISENSE__) && !__has_builtin(__builtin_COLUMN) && !defined(_MSC_VER)
constexpr unsigned __builtin_COLUMN()
{
	return -1;
}
#endif

struct src_loc
{
	u32 line;
	u32 col;
	const char* file;
	const char* func;
};

namespace fmt
{
	[[noreturn]] void raw_verify_error(const src_loc& loc);
	[[noreturn]] void raw_narrow_error(const src_loc& loc, const fmt_type_info* sup, u64 arg);
}

template <typename T>
constexpr decltype(auto) ensure(T&& arg,
	u32 line = __builtin_LINE(),
	u32 col = __builtin_COLUMN(),
	const char* file = __builtin_FILE(),
	const char* func = __builtin_FUNCTION()) noexcept
{
	if (std::forward<T>(arg)) [[likely]]
	{
		return std::forward<T>(arg);
	}

	fmt::raw_verify_error({line, col, file, func});
}

// narrow() function details
template <typename From, typename To = void, typename = void>
struct narrow_impl
{
	// Temporarily (diagnostic)
	static_assert(std::is_void<To>::value, "narrow_impl<> specialization not found");

	// Returns true if value cannot be represented in type To
	static constexpr bool test(const From& value)
	{
		// Unspecialized cases (including cast to void) always considered narrowing
		return true;
	}
};

// Unsigned to unsigned narrowing
template <typename From, typename To>
struct narrow_impl<From, To, std::enable_if_t<std::is_unsigned<From>::value && std::is_unsigned<To>::value>>
{
	static constexpr bool test(const From& value)
	{
		return sizeof(To) < sizeof(From) && static_cast<To>(value) != value;
	}
};

// Signed to signed narrowing
template <typename From, typename To>
struct narrow_impl<From, To, std::enable_if_t<std::is_signed<From>::value && std::is_signed<To>::value>>
{
	static constexpr bool test(const From& value)
	{
		return sizeof(To) < sizeof(From) && static_cast<To>(value) != value;
	}
};

// Unsigned to signed narrowing
template <typename From, typename To>
struct narrow_impl<From, To, std::enable_if_t<std::is_unsigned<From>::value && std::is_signed<To>::value>>
{
	static constexpr bool test(const From& value)
	{
		return sizeof(To) <= sizeof(From) && value > (static_cast<std::make_unsigned_t<To>>(-1) >> 1);
	}
};

// Signed to unsigned narrowing (I)
template <typename From, typename To>
struct narrow_impl<From, To, std::enable_if_t<std::is_signed<From>::value && std::is_unsigned<To>::value && sizeof(To) >= sizeof(From)>>
{
	static constexpr bool test(const From& value)
	{
		return value < static_cast<From>(0);
	}
};

// Signed to unsigned narrowing (II)
template <typename From, typename To>
struct narrow_impl<From, To, std::enable_if_t<std::is_signed<From>::value && std::is_unsigned<To>::value && sizeof(To) < sizeof(From)>>
{
	static constexpr bool test(const From& value)
	{
		return static_cast<std::make_unsigned_t<From>>(value) > static_cast<To>(-1);
	}
};

// Simple type enabled (TODO: allow for To as well)
template <typename From, typename To>
struct narrow_impl<From, To, std::void_t<typename From::simple_type>>
	: narrow_impl<simple_t<From>, To>
{
};

template <typename To = void, typename From, typename = decltype(static_cast<To>(std::declval<From>()))>
[[nodiscard]] constexpr To narrow(const From& value,
	u32 line = __builtin_LINE(),
	u32 col = __builtin_COLUMN(),
	const char* file = __builtin_FILE(),
	const char* func = __builtin_FUNCTION())
{
	// Narrow check
	if (narrow_impl<From, To>::test(value)) [[unlikely]]
	{
		// Pack value as formatting argument
		fmt::raw_narrow_error({line, col, file, func}, fmt::get_type_info<fmt_unveil_t<From>>(), fmt_unveil<From>::get(value));
	}

	return static_cast<To>(value);
}

// Returns u32 size() for container
template <typename CT, typename = decltype(static_cast<u32>(std::declval<CT>().size()))>
[[nodiscard]] constexpr u32 size32(const CT& container,
	u32 line = __builtin_LINE(),
	u32 col = __builtin_COLUMN(),
	const char* file = __builtin_FILE(),
	const char* func = __builtin_FUNCTION())
{
	return narrow<u32>(container.size(), line, col, file, func);
}

// Returns u32 size for an array
template <typename T, std::size_t Size>
[[nodiscard]] constexpr u32 size32(const T (&)[Size])
{
	static_assert(Size < UINT32_MAX, "Array is too big for 32-bit");
	return static_cast<u32>(Size);
}

// Simplified hash algorithm for pointers. May be used in std::unordered_(map|set).
template <typename T, std::size_t Align = alignof(T)>
struct pointer_hash
{
	std::size_t operator()(T* ptr) const
	{
		return reinterpret_cast<std::uintptr_t>(ptr) / Align;
	}
};

template <typename T, std::size_t Shift = 0>
struct value_hash
{
	std::size_t operator()(T value) const
	{
		return static_cast<std::size_t>(value) >> Shift;
	}
};

// Contains value of any POD type with fixed size and alignment. TT<> is the type converter applied.
// For example, `simple_t` may be used to remove endianness.
template <template <typename> class TT, std::size_t S, std::size_t A = S>
struct alignas(A) any_pod
{
	alignas(A) std::byte data[S];

	any_pod() = default;

	template <typename T, typename T2 = TT<T>, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && sizeof(T2) == S && alignof(T2) <= A>>
	any_pod(const T& value)
	{
		*this = std::bit_cast<any_pod>(value);
	}

	template <typename T, typename T2 = TT<T>, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && sizeof(T2) == S && alignof(T2) <= A>>
	T2& as()
	{
		return reinterpret_cast<T2&>(data);
	}

	template <typename T, typename T2 = TT<T>, typename = std::enable_if_t<std::is_trivially_copyable_v<T> && sizeof(T2) == S && alignof(T2) <= A>>
	const T2& as() const
	{
		return reinterpret_cast<const T2&>(data);
	}
};

using any16 = any_pod<simple_t, sizeof(u16)>;
using any32 = any_pod<simple_t, sizeof(u32)>;
using any64 = any_pod<simple_t, sizeof(u64)>;

struct cmd64 : any64
{
	struct pair_t
	{
		any32 arg1;
		any32 arg2;
	};

	cmd64() = default;

	template <typename T>
	cmd64(const T& value)
		: any64(value)
	{
	}

	template <typename T1, typename T2>
	cmd64(const T1& arg1, const T2& arg2)
		: any64(pair_t{arg1, arg2})
	{
	}

	explicit operator bool() const
	{
		return as<u64>() != 0;
	}

	// TODO: compatibility with std::pair/std::tuple?

	template <typename T>
	decltype(auto) arg1()
	{
		return as<pair_t>().arg1.as<T>();
	}

	template <typename T>
	decltype(auto) arg1() const
	{
		return as<const pair_t>().arg1.as<const T>();
	}

	template <typename T>
	decltype(auto) arg2()
	{
		return as<pair_t>().arg2.as<T>();
	}

	template <typename T>
	decltype(auto) arg2() const
	{
		return as<const pair_t>().arg2.as<const T>();
	}
};

static_assert(sizeof(cmd64) == 8 && std::is_trivially_copyable_v<cmd64>, "Incorrect cmd64 type");

// Error code type (return type), implements error reporting. Could be a template.
class error_code
{
	// Use fixed s32 type for now
	s32 value;

public:
	error_code() = default;

	// Implementation must be provided specially
	static s32 error_report(const fmt_type_info* sup, u64 arg, const fmt_type_info* sup2, u64 arg2);

	// Helper type
	enum class not_an_error : s32
	{
		__not_an_error // SFINAE marker
	};

	// __not_an_error tester
	template<typename ET, typename = void>
	struct is_error : std::integral_constant<bool, std::is_enum<ET>::value || std::is_integral<ET>::value>
	{
	};

	template<typename ET>
	struct is_error<ET, std::enable_if_t<sizeof(ET::__not_an_error) != 0>> : std::false_type
	{
	};

	// Common constructor
	template<typename ET>
	error_code(const ET& value)
		: value(static_cast<s32>(value))
	{
		if constexpr(is_error<ET>::value)
		{
			this->value = error_report(fmt::get_type_info<fmt_unveil_t<ET>>(), fmt_unveil<ET>::get(value), nullptr, 0);
		}
	}

	// Error constructor (2 args)
	template<typename ET, typename T2>
	error_code(const ET& value, const T2& value2)
		: value(error_report(fmt::get_type_info<fmt_unveil_t<ET>>(), fmt_unveil<ET>::get(value), fmt::get_type_info<fmt_unveil_t<T2>>(), fmt_unveil<T2>::get(value2)))
	{
	}

	operator s32() const
	{
		return value;
	}
};

// Helper function for error_code
template <typename T>
constexpr FORCE_INLINE error_code::not_an_error not_an_error(const T& value)
{
	return static_cast<error_code::not_an_error>(static_cast<s32>(value));
}

// Synchronization helper (cache-friendly busy waiting)
inline void busy_wait(std::size_t cycles = 3000)
{
	const u64 s = __rdtsc();
	do _mm_pause(); while (__rdtsc() - s < cycles);
}

// TODO: Remove when moving to c++20
template <typename T>
inline constexpr uintmax_t floor2(T value)
{
	value >>= 1;

	for (uintmax_t i = 0;; i++, value >>= 1)
	{
		if (value == 0)
		{
			return i;
		}
	}
}

template <typename T>
inline constexpr uintmax_t ceil2(T value)
{
	const uintmax_t ispow2 = value & (value - 1); // if power of 2 the result is 0

	value >>= 1;

	for (uintmax_t i = 0;; i++, value >>= 1)
	{
		if (value == 0)
		{
			return i + std::min<uintmax_t>(ispow2, 1);
		}
	}
}
