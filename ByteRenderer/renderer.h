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
	};

	class Renderer {
	private:
		using URenderPass = std::unique_ptr<RenderPass>;
		using Pipeline = std::vector<URenderPass>;

		RenderData data;
		Pipeline pipeline;

	public:
		void initialize(Window& window, RenderConfig& config) {
			OpenglAPI::initialize(window);

			data.width = window.width();
			data.height = window.height();
			
			for (auto& pair : config.frameBufferConfigs) {
				data.frameBuffers[pair.first] = OpenglAPI::Framebuffer::build(pair.second);
			}

			data.quad = MeshBuilder::quad();
			fillVertexArray(data.quad);

			data.sphere = MeshBuilder::sphere(1, 10);
			fillVertexArray(data.sphere);

			compileShaders(config);
		}

		void render(RenderContext& context) {
			for (size_t i{}; i < context.targetCount(); ++i) {
				Mesh& mesh{ context.mesh(i) };
				if (mesh.renderArray().data().VAO == 0) {
					fillVertexArray(mesh);
				}
			}

			for (auto& pass : pipeline) {
				pass->render(context, data);
			}
		}

		void update(Window& window) const {
			OpenglAPI::update(window);
		}

		void compileShaders(RenderConfig& config) {
			for (const auto& [shaderTag, shaderPath] : config.shaderPaths) {
				Shader shader(shaderPath.vertex, shaderPath.fragment);
				ShaderCompiler::compile(shader);
				data.shaders[shaderTag] = std::move(shader);
			}
		}

		template<typename... Passes>
		static Renderer build() {
			Renderer renderer;
			(renderer.pipeline.push_back(std::make_unique<Passes>()), ...);

			return renderer;
		}

	private:
		Buffer<VertexAttribute> buildMeshAttributes(Mesh& mesh) const {
			Buffer<VertexAttribute> atts;

			size_t stride{ 0 };

			for (uint8_t i{ 0 }; i < mesh.data().vertexLayout.size(); ++i) {
				uint8_t layout{ mesh.data().vertexLayout[i] };
				uint16_t strideSize{ static_cast<uint16_t>(stride * sizeof(float)) };

				atts.push_back(VertexAttribute{0, sizeof(float), GL_FLOAT,strideSize,layout,i,false });

				stride += layout;
			}

			return atts;
		}

		void fillVertexArray(Mesh& mesh) const {
			bool isStatic{ mesh.mode() == MeshMode::STATIC };

			Buffer<VertexAttribute> atts{ VertexAttribute::build(mesh.data().vertexLayout) };
			 
			mesh.renderArray(OpenglAPI::RArray::build(mesh.vertices(),mesh.indices(),atts,isStatic));
		}

		void fillInstancedVertexArray(RenderInstance& instance) const {
			bool isStatic{ instance.mesh().mode() == MeshMode::STATIC};

			Buffer<VertexAttribute> atts{ VertexAttribute::build(instance.mesh().data().vertexLayout) };

			Buffer<VertexAttribute> iAtts{ VertexAttribute::build({3,3,4},3) };

			auto& vertices{ instance.mesh().vertices() };
			auto& indices{ instance.mesh().indices() };
			instance.mesh().renderArray(OpenglAPI::RArray::build(vertices, indices,atts,iAtts,isStatic));
		}

	};

}