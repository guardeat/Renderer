#pragma once

namespace Byte {

	template<typename Type = float>
	inline constexpr Type pi() {
		return  static_cast<Type>(3.1415926535897931);
	}

	template<>
	inline constexpr float pi<float>() {
		return  3.14159265f;
	}

	template<>
	inline constexpr double pi<double>() {
		return  3.1415926535897931;
	}

	template<typename Type>
	Type radians(Type degree) {
		return static_cast<Type>((degree * pi()) / 180);
	}

	template<>
	inline float radians<float>(float degree) {
		return (degree * pi()) / 180;
	}

	template<>
	inline double radians<double>(double degree) {
		return (degree * pi<double>()) / 180;
	}
}