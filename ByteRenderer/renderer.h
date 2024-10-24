#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "render_pass.h"
#include "mesh.h"
#include "window.h"
#include "material.h"
#include "transform.h"
#include "camera.h"
#include "light.h"
#include "typedefs.h"
#include "framebuffer.h"

namespace Byte {

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

			data.sphere = Mesh::sphere(1, 10);

			fillMesh(data.sphere);

			compileShaders(config);
		}

		void render(RenderContext& context) {
			for (Mesh* mesh : context.meshes) {
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
			bool isStatic = mesh.meshMode() == MeshMode::STATIC;
			mesh._renderArray = OpenglAPI::RArray::build(
				mesh.position(), 
				mesh.normal(), 
				mesh.uv1(),
				mesh.index(),
				mesh.uv2(),
				isStatic);
		}
	};

}