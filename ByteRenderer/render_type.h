#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <string>
#include <exception>

namespace Byte {

	using RenderID = uint64_t;

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

	enum class TextureUnit: uint8_t {
		T0, T1, T2, T3,
		T4, T5, T6, T7,
		T8, T9, T10, T11,
		T12, T13, T14, T15,
	};

	enum class TextureType : uint32_t {
		TEXTURE_2D = 0x0DE1,
		TEXTURE_ARRAY = 0x8C1A,
	};

	enum class ShaderType: uint32_t {
		FRAGMENT = 0x8B30,
		VERTEX = 0x8B31,
		GEOMETRY = 0x8DD9,
	};

	enum class DataType : uint8_t {
		BYTE,
		UNSIGNED_BYTE,
		SHORT,
		UNSIGNED_SHORT,
		INT,
		UNSIGNED_INT,
		FLOAT
	};

	enum class ColorFormat : uint32_t {
		DEPTH = 0x1902,
		RED = 0x1903,
		GREEN = 0x1904,
		BLUE = 0x1905,
		ALPHA = 0x1906,
		RGB = 0x1907,
		RGBA = 0x1908,

		RGBA32F = 0x8814,
		RGB32F = 0x8815,
		RGBA16F = 0x881A,
		RGB16F = 0x881B
	};

	enum class AttachmentType: uint8_t {
		COLOR_0,
		COLOR_1,
		COLOR_2,
		COLOR_3,
		DEPTH = 32,
	};

	struct TextureData {
		size_t height;
		size_t width;
		size_t channels;

		Path path;

		std::unique_ptr<uint8_t[]> data;
	};

	struct FramebufferConfig {
		struct TextureAttachmentConfig {
			std::string tag;

			AttachmentType attachment;

			ColorFormat internalFormat{ ColorFormat::RGBA };
			ColorFormat format{ ColorFormat::RGBA };
			DataType dataType{ DataType::BYTE };

			TextureType type{ TextureType::TEXTURE_2D };

			size_t width{};
			size_t height{};
			size_t layerCount{};
		};

		using AttachmentVector = std::vector<TextureAttachmentConfig>;
		AttachmentVector attachments;

		size_t width{};
		size_t height{};
	};

	struct TextureAttachmentData {
		TextureID id{};

		TextureType type{ TextureType::TEXTURE_2D };
		
		size_t width{};
		size_t height{};
		size_t layerCount{};
	};

	struct FramebufferData {
		FramebufferID id{};

		using TextureMap = std::unordered_map<std::string, TextureAttachmentData>;

		TextureMap textures;
		Buffer<AttachmentType> attachments;

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
	};

}