#pragma once

#include <chrono>
#include <vector>

#include "core/mesh.h"
#include "core/material.h"
#include "core/transform.h"
#include "math/vec.h"
#include "render/renderer.h"

namespace Byte {

	using TimePoint = std::chrono::steady_clock::time_point;

	struct Particle {
		TimePoint start{ std::chrono::steady_clock::now() };
		float lifeTime = 10.0f;
		Transform transform;
		Vec3 velocity;
	};

	struct ParticleGroup {
		Mesh mesh;
		Material material;
		MeshRenderer renderer;
		std::vector<Particle> particles;
	};

	class ParticleSystem {
	private:
		using GroupMap = std::unordered_map<std::string, ParticleGroup>;
		GroupMap _groups;

	public:
		GroupMap& groups() {
			return _groups;
		}

		void update(float dt, Renderer& renderer) {
			for (auto& [tag, group] : _groups) {
				auto instance{ renderer.context().instances().find(tag)};

				if (instance == renderer.context().instances().end()) {
					renderer.context().createInstance(tag, group.mesh, group.material,group.renderer);
					instance = renderer.context().instances().find(tag);
				}
				instance->second.clearInstances();
			
				group.particles.erase(std::remove_if(group.particles.begin(), group.particles.end(),
					[currentTime = std::chrono::steady_clock::now()](const Particle& p) {
						return std::chrono::duration<float>(currentTime - p.start).count() >= p.lifeTime;
					}),
					group.particles.end());

				for (auto& particle : group.particles) {
					particle.transform.position(particle.transform.position() + particle.velocity * dt);
					renderer.context().submit(tag, particle.transform);
				}
			}
		}
	};

}