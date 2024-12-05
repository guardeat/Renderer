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
		void initialize(Window& window, RenderConfig& config) {
			OpenglAPI::initialize(window);

			_data.width = window.width();
			_data.height = window.height();
			
			for (auto& pair : config.frameBufferConfigs) {
				_data.frameBuffers[pair.first] = OpenglAPI::Framebuffer::build(pair.second);
			}

			compileShaders(config);

			_data.parameters = config.parameters;

			_data.meshes = std::move(config.meshes);

			for (auto& pair : _data.meshes) {
				fillVertexArray(pair.second);
			}
		}

		void render() {
			prepareVertexArrays(_context);

			for (auto& pass : _pipeline) {
				pass->render(_context, _data);
			}
		}

		void update(Window& window) const {
			OpenglAPI::update(window);
		}

		void compileShaders(RenderConfig& config) {
			for (const auto& [shaderTag, shaderPath] : config.shaderPaths) {
				Shader shader(shaderPath.vertex, shaderPath.fragment);
				ShaderCompiler::compile(shader);
				_data.shaders[shaderTag] = std::move(shader);
			}
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
		void prepareVertexArrays(RenderContext& context) {
			for (size_t i{}; i < context.entityCount(); ++i) {
				Mesh& mesh{ context.mesh(i) };
				if (!mesh.readyRender() && !mesh.empty()) {
					fillVertexArray(mesh);
				}
			}

			for (auto& pair : context.instances()) {
				if (!pair.second.mesh().readyRender() && !pair.second.mesh().empty()) {
					fillInstancedVertexArray(pair.second);
					pair.second.resetInstanceBuffer();
				}
				else if (pair.second.change()) {
					pair.second.resetInstanceBuffer();
				}
			}
		}

		void fillVertexArray(Mesh& mesh) const {
			bool isStatic{ mesh.mode() == MeshMode::STATIC };

			auto atts{ OpenglAPI::RArray::buildAttributes(mesh.data().vertexLayout) };
			 
			mesh.renderArray(OpenglAPI::RArray::build(mesh.vertices(),mesh.indices(),atts,isStatic));
		}

		void fillInstancedVertexArray(RenderInstance& instance) const {
			bool isStatic{ instance.mesh().mode() == MeshMode::STATIC};

			auto atts{ OpenglAPI::RArray::buildAttributes(instance.mesh().data().vertexLayout)};

			auto iAtts{ OpenglAPI::RArray::buildAttributes({3,3,4},3) };

			auto& vertices{ instance.mesh().vertices() };
			auto& indices{ instance.mesh().indices() };
			auto rArrayData{ OpenglAPI::RArray::build(vertices,indices,atts,iAtts,isStatic) };
			instance.mesh().renderArray(std::move(rArrayData));
		}

	};

}