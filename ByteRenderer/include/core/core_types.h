#pragma once

#include <vector>
#include <filesystem>
#include <string>

namespace Byte {

	using Path = std::filesystem::path;

	template<typename T>
	using Buffer = std::vector<T>;

	using MeshTag = std::string;
}
