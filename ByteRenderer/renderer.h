#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "render_pass.h"
#include "window.h"

namespace Byte {

	struct RenderConfig {
		using ShaderPathMap = std::unordered_map<ShaderTag, ShaderPath>;
		ShaderPathMap shaderPaths;

		using FramebufferConfigMap = std::unordered_map<FramebufferTag, FramebufferConfig>;
		FramebufferConfigMap frameBufferConfigs;

		using ParameterMap = RenderData::ParameterMap;
		ParameterMap parameters;

		using MeshMap = RenderData::MeshMap;
		MeshMap meshes;
	};

	class Renderer {
	private:
		using URenderPass = std::unique_ptr<RenderPass>;
		using Pipeline = std::vector<URenderPass>;

		RenderContext _context;
		RenderData _data;
		Pipeline _pipeline;

	public:
		Renderer() = default;

		Renderer(Window& window, RenderConfig& config) {
			initialize(window, config);
		}

		void initialize(Window& window, RenderConfig& config) {
			OpenGLAPI::initialize(window);

			_data.width = window.width();
			_data.height = window.height();
			
			for (auto& pair : config.frameBufferConfigs) {
				_data.frameBuffers[pair.first] = OpenGLAPI::Framebuffer::build(pair.second);
			}

			compileShaders(config);

			_data.parameters = config.parameters;

			_data.meshes = std::move(config.meshes);

			for (auto& pair : _data.meshes) {
				fillVertexArray(pair.second);
			}

			load();
		}

		void load() {
			prepareVertexArrays();
			prepareTextures();
		}

		void render() {
			load();

			for (auto& pass : _pipeline) {
				pass->render(_context, _data);
			}
		}

		void update(Window& window) const {
			OpenGLAPI::update(window);
		}

		RenderContext& context() {
			return _context;
		}

		const RenderContext& context() const {
			return _context;
		}

		template<typename Type>
		Type& parameter(const std::string& parameter) {
			return std::get<Type>(_data.parameters.at(parameter));
		}

		template<typename Type>
		const Type& parameter(const std::string& parameter) const {
			return std::get<Type>(_data.parameters.at(parameter));
		}

		Pipeline& pipeline() {
			return _pipeline;
		}

		const Pipeline& pipeline() const {
			return _pipeline;
		}

		template<typename... Passes>
		static Renderer build() {
			Renderer renderer;
			(renderer._pipeline.push_back(std::make_unique<Passes>()), ...);

			return renderer;
		}

	private:
		void compileShaders(RenderConfig& config) {
			for (const auto& [shaderTag, shaderPath] : config.shaderPaths) {
				Shader shader(shaderPath.vertex, shaderPath.fragment, shaderPath.geometry);
				ShaderCompiler::compile(shader);
				_data.shaders[shaderTag] = std::move(shader);
			}
		}

		void prepareVertexArrays() {
			for (auto& pair : _context.renderEntities()) {
				Mesh& mesh{ *pair.second.mesh };
				if (!mesh.drawable() && !mesh.empty()) {
					fillVertexArray(mesh);
				}
			}

			for (auto& pair : _context.instances()) {
				if (!pair.second.mesh().drawable() && !pair.second.mesh().empty()) {
					fillInstancedVertexArray(pair.second);
					pair.second.resetInstanceBuffer();
				}
				else if (pair.second.changed()) {
					pair.second.resetInstanceBuffer();
				}
			}
		}

		void fillVertexArray(Mesh& mesh) const {
			bool isStatic{ mesh.mode() == MeshMode::STATIC };

			auto atts{ OpenGLAPI::RenderArray::buildAttributes(mesh.data().vertexLayout) };
			 
			mesh.renderArray(OpenGLAPI::RenderArray::build(mesh.vertices(),mesh.indices(),atts,isStatic));
		}

		void fillInstancedVertexArray(RenderInstance& instance) const {
			bool isStatic{ instance.mesh().mode() == MeshMode::STATIC};

			auto atts{ OpenGLAPI::RenderArray::buildAttributes(instance.mesh().data().vertexLayout)};

			auto iAtts{ OpenGLAPI::RenderArray::buildAttributes({3,3,4},3) };

			auto& vertices{ instance.mesh().vertices() };
			auto& indices{ instance.mesh().indices() };
			auto rArrayData{ OpenGLAPI::RenderArray::build(vertices,indices,atts,iAtts,isStatic) };
			instance.mesh().renderArray(std::move(rArrayData));
		}

		void prepareTextures() {
			for (auto& pair : _context.renderEntities()) {
				Material& material{ *pair.second.material };
				if (static_cast<bool>(material.albedoTexture()) && !material.albedoTextureID()) {
					material.albedoTextureID(OpenGLAPI::Texture::build(material.albedoTexture().data()));
				}

				if (static_cast<bool>(material.materialTexture()) && !material.materialTextureID()) {
					material.materialTextureID(OpenGLAPI::Texture::build(material.materialTexture().data()));
				}
			}

			for (auto& pair : _context.instances()) {
				Material& material{ pair.second.material() };
				if (static_cast<bool>(material.albedoTexture()) && !material.albedoTextureID()) {
					material.albedoTextureID(OpenGLAPI::Texture::build(material.albedoTexture().data()));
				}

				if (static_cast<bool>(material.materialTexture()) && !material.materialTextureID()) {
					material.materialTextureID(OpenGLAPI::Texture::build(material.materialTexture().data()));
				}
			}
		}

	};

}