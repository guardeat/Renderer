#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "render_pass.h"
#include "window.h"

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

			auto atts{ RenderAPI::RenderArray::buildAttributes(mesh.data().vertexLayout) };
			 
			mesh.renderArray(RenderAPI::RenderArray::build(mesh.vertices(),mesh.indices(),atts,isStatic));
		}

		void fillInstancedVertexArray(RenderInstance& instance) const {
			bool isStatic{ instance.mesh().mode() == MeshMode::STATIC};

			auto atts{ RenderAPI::RenderArray::buildAttributes(instance.mesh().data().vertexLayout)};

			auto iAtts{ RenderAPI::RenderArray::buildAttributes(instance.layout(),3)};

			auto& vertices{ instance.mesh().vertices() };
			auto& indices{ instance.mesh().indices() };
			auto rArrayData{ RenderAPI::RenderArray::build(vertices,indices,atts,iAtts,isStatic) };
			instance.mesh().renderArray(std::move(rArrayData));
		}

		void prepareTextures() {
			for (auto& pair : _context.renderEntities()) {
				Material& material{ *pair.second.material };
				if (static_cast<bool>(material.albedoTexture()) && !material.albedoTextureID()) {
					material.albedoTextureID(RenderAPI::Texture::build(material.albedoTexture().data()));
				}

				if (static_cast<bool>(material.materialTexture()) && !material.materialTextureID()) {
					material.materialTextureID(RenderAPI::Texture::build(material.materialTexture().data()));
				}
			}

			for (auto& pair : _context.instances()) {
				Material& material{ pair.second.material() };
				if (static_cast<bool>(material.albedoTexture()) && !material.albedoTextureID()) {
					material.albedoTextureID(RenderAPI::Texture::build(material.albedoTexture().data()));
				}

				if (static_cast<bool>(material.materialTexture()) && !material.materialTextureID()) {
					material.materialTextureID(RenderAPI::Texture::build(material.materialTexture().data()));
				}
			}
		}

	};

}