#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "Core/window.h"
#include "render_pass.h"
#include "texture.h"

namespace Byte {

	class Renderer {
	private:
		using URenderPass = std::unique_ptr<RenderPass>;
		using Pipeline = std::vector<URenderPass>;

		RenderContext _context;
		RenderData _data;
		Pipeline _pipeline;

	public:
		Renderer() = default;

		Renderer(Window& window) {
			initialize(window);
		}

		void initialize(Window& window) {
			RenderAPI::initialize(window);

			_data.width = window.width();
			_data.height = window.height();

			for (auto& pair : _data.frameBuffers) {
				pair.second.build();
			}

			compileShaders();

			for (auto& [tag,pair] : _data.meshes) {
				pair.renderer.upload(pair.mesh);
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

		void update(Window& window) {
			RenderAPI::update(window);

			if (_data.width != window.width() || _data.height != window.height()) {
				resize(window.width(), window.height());
			}
		}

		void resize(size_t width, size_t height) {
			for (auto& pair : _data.frameBuffers) {
				pair.second.resize(width, height);
			}

			_data.width = width;
			_data.height = height;
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

		RenderData& data() {
			return _data;
		}

		const RenderData& data() const {
			return _data;
		}

		Pipeline& pipeline() {
			return _pipeline;
		}

		const Pipeline& pipeline() const {
			return _pipeline;
		}

		void compileShaders() {
			for (auto& pair : _data.shaders) {
				if (!pair.second.id()) {
					ShaderCompiler::compile(pair.second);
				}
			}
		}

		template<typename... Passes>
		static Renderer build() {
			Renderer renderer;
			(renderer._pipeline.push_back(std::make_unique<Passes>()), ...);

			return renderer;
		}

	private:
		void prepareVertexArrays() {
			for (auto& pair : _context.renderEntities()) {
				Mesh& mesh{ *pair.second.mesh };
				MeshRenderer& meshRenderer{ *pair.second.meshRenderer };
				if (!meshRenderer.drawable() && !mesh.empty()) {
					meshRenderer.upload(mesh);
				}
			}

			for (auto& [tag, instance] : _context.instances()) {
				if (!instance.meshRenderer().drawable() && !instance.mesh().empty()) {
					instance.meshRenderer().uploadInstanced(instance.mesh(),instance.layout());
					instance.resetInstanceBuffer();
				}
				else if (instance.changed()) {
					instance.resetInstanceBuffer();
				}
			}
		}

		void prepareTextures() {
			for (auto& pair : _context.renderEntities()) {
				Material& material{ *pair.second.material };
				if (material.hasAlbedoTexture() && !material.albedoTexture().id()) {
					RenderAPI::Texture::build(material.albedoTexture().data());
				}

				if (material.hasMaterialTexture() && !material.materialTexture().id()) {
					RenderAPI::Texture::build(material.materialTexture().data());
				}
			}

			for (auto& pair : _context.instances()) {
				Material& material{ pair.second.material() };
				if (material.hasAlbedoTexture() && !material.albedoTexture().id()) {
					RenderAPI::Texture::build(material.albedoTexture().data());
				}

				if (material.hasMaterialTexture() && !material.materialTexture().id()) {
					RenderAPI::Texture::build(material.materialTexture().data());
				}
			}
		}

	};

}