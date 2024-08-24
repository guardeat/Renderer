#pragma once

#include <vector>
#include <memory>

#include "render_context.h"
#include "render_pass.h"
#include "mesh.h"
#include "window.h"
#include "material.h"
#include "transform.h"
#include "camera.h"
#include "render_data.h"
#include "render_context.h"

namespace Byte {

	struct RenderConfig {
		using ShaderPathMap = std::unordered_map<ShaderTag, ShaderPath>;
		ShaderPathMap shaderPathMap;
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

			data.gBuffer = GBuffer{ OpenglAPI::buildGbuffer(data.width, data.height) };

			data.quad.renderArray = OpenglAPI::buildQuad(
				data.quad.positions,
				data.quad.indices);

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
			for (const auto& [shaderTag, shaderPath] : config.shaderPathMap) {
				Shader shader(shaderPath.vertex, shaderPath.fragment);
				ShaderCompiler::compile(shader);
				data.shaderMap[shaderTag] = std::move(shader);
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
			mesh.data.renderArray = OpenglAPI::buildRenderArray(mesh.data.geometry, isStatic);
		}
	};

}