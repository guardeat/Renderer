#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace Byte {

	using Path = std::filesystem::path;

	template<typename T>
	using Buffer = std::vector<T>;

	using ShaderTag = std::string;
	using FramebufferTag = std::string;

	using TextureID = uint32_t;
	using FramebufferID = uint32_t;

	using RArrayID = uint32_t;
	using RBufferID = uint32_t;

	struct TextureData {
		size_t height;
		size_t width;
		size_t channels;

		Path path;

		std::unique_ptr<uint8_t[]> data;
	};

	struct FramebufferConfig {
		struct TextureAttachment {
			std::string tag;
			uint32_t index;
			GLenum internalFormat{ GL_RGBA };
			GLenum format{ GL_RGBA };
			GLenum type{ GL_UNSIGNED_BYTE };
		};

		using AttachmentContainer = std::vector<TextureAttachment>;

		AttachmentContainer attachments;

		size_t width{};
		size_t height{};
	};

	struct FramebufferData {
		FramebufferID id{};

		using TextureMap = std::unordered_map<std::string, TextureID>;

		TextureMap textures;
		Buffer<uint32_t> attachments;
	};

	struct RArrayData {
		RArrayID VAO{ 0 };
		RBufferID PBO{ 0 };
		RBufferID NBO{ 0 };
		RBufferID UVBO{ 0 };
		RBufferID EBO{ 0 };
		RBufferID UV2BO{ 0 };
	};

}