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

			data.quad = OpenglAPI::RArray::buildQuad();

			data.sphere = MeshBuilder::sphere(1, 10);

			fillMesh(data.sphere);

			compileShaders(config);
		}

		void render(RenderContext& context) {
			for (Mesh* mesh : context.meshes()) {
				if (mesh->renderArray().data().VAO == 0) {
					fillMesh(*mesh);
				}
			}

			for (auto& pass : pipeline) {
				pass->render(context, data);
			}
		}

		void update(Window& window) {
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
		void fillMesh(Mesh& mesh) {
			bool isStatic{ mesh.meshMode() == MeshMode::STATIC };

			VertexAttribute pos{ 0, sizeof(float), GL_FLOAT,0,3,0,false };
			VertexAttribute normal{ 0, sizeof(float), GL_FLOAT,3 * sizeof(float),3, 1,false };
			VertexAttribute uv{ 0, sizeof(float), GL_FLOAT,6 * sizeof(float),2,2,false };

			Buffer<VertexAttribute> attributes{ pos,normal,uv };
			 
			mesh.renderArray(OpenglAPI::RArray::build(mesh.vertices(),mesh.indices(),attributes,isStatic));
		}
	};

}