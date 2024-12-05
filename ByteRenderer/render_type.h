#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>
#include <exception>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace Byte {

	using Path = std::filesystem::path;

	template<typename T>
	using Buffer = std::vector<T>;

	using ShaderTag = std::string;
	using FramebufferTag = std::string;
	using MeshTag = std::string;

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

		bool depthMap{ false };
	};

	struct FramebufferData {
		FramebufferID id{};

		using TextureMap = std::unordered_map<std::string, TextureID>;

		TextureMap textures;
		Buffer<uint32_t> attachments;

		size_t width{};
		size_t height{};
	};

	struct VertexAttribute {
		RBufferID bufferID;

		uint32_t size;
		uint32_t type;
		uint16_t offset;
		uint16_t stride;

		uint8_t index;

		bool normalized{ false };
	};

	struct RBufferData {
		RBufferID id{};
		Buffer<VertexAttribute> attributes;
	};

	struct RArrayData {
		RArrayID VAO{ 0 };
		
		Buffer<RBufferData> VBuffers;

		RBufferID EBO{ 0 };
		size_t elementCount{ 0 };

		bool isStatic{ true };
	};

}