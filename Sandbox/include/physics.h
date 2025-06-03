#pragma once

#include "math/mat.h"
#include "math/quaternion.h"
#include "math/trigonometry.h"
#include "math/vec.h"

#include "core/transform.h"

#include "render/texture.h"

#include "terrain.h"

namespace Byte {

	enum class ColliderType {
		SPHERE,
		TERRAIN,
	};

	struct Collider {
		ColliderType type;
		Transform* transform;

		virtual ~Collider() = default;
	};

	struct SphereCollider: public Collider {
		float radius{ 1.0f };
		Vec3 velocity{};

		SphereCollider(float radius) 
			:radius{ radius } {

		}
	};

	struct TerrainCollider: public Collider {
		Texture* texture;
		float minX{ -500.0f };
		float minZ{ -500.0f };
		float maxX{ 500.0f };
		float maxZ{ 500.0f };

		TerrainCollider(Texture* texture)
			:texture{texture} {
		}
	};

    struct Physics {

        static void solve(std::vector<SphereCollider*>& spheres) {
            for (size_t i{ 0 }; i < spheres.size(); ++i) {
                SphereCollider* sphereA{ spheres[i] };
                Vec3 posA{ sphereA->transform->position() };
                float radiusA{ sphereA->radius * sphereA->transform->scale().y };

                for (size_t j{ i + 1 }; j < spheres.size(); ++j) {
                    SphereCollider* sphereB{ spheres[j] };
                    Vec3 posB{ sphereB->transform->position() };
                    float radiusB{ sphereB->radius * sphereB->transform->scale().y };

                    Vec3 delta{ posB - posA };
                    float distanceSq{ delta.dot(delta) };
                    float radiusSum{ radiusA + radiusB };

                    if (distanceSq < radiusSum * radiusSum) {
                        float distance{ std::sqrt(distanceSq) };
                        Vec3 normal{ distance > 0.0001f ? delta / distance : Vec3{ 1.0f, 0.0f, 0.0f } };

                        float penetration{ radiusSum - distance };
                        Vec3 correction{ normal * (penetration * 0.5f) };

                        posA -= correction;
                        posB += correction;

                        sphereA->transform->position(posA);
                        sphereB->transform->position(posB);
                    }
                }
            }
        }

        void solve(TerrainCollider& terrain, SphereCollider& sphere) {
            if (!terrain.transform || !sphere.transform || !terrain.texture) {
                return;
            }

            Vec3 spherePos{ sphere.transform->position() };
            float sphereRadius{ sphere.radius * sphere.transform->scale().y };

            float terrainHeight{ getHeight(*terrain.texture, spherePos.x, spherePos.z) };
            float sphereBottomY{ spherePos.y - sphereRadius };

            if (sphereBottomY < terrainHeight) {
                spherePos.y = terrainHeight + sphereRadius;
                sphere.transform->position(spherePos);

                Vec3 normal{ getTerrainNormal(*terrain.texture, spherePos.x, spherePos.z).normalized() };

                Vec3& velocity{ sphere.velocity };

                float penetrationVelocity{ velocity.dot(normal) };
                if (penetrationVelocity < 0.0f) {
                    velocity -= normal * penetrationVelocity;

                    float restitution{ 0.2f };
                    velocity += normal * (-penetrationVelocity * restitution);
                }

                float friction{ 0.6f };
                Vec3 tangent{ velocity - normal * velocity.dot(normal) };
                velocity -= tangent * friction * 0.016f;

                sphere.velocity = velocity;
            }
        }



        static void solve(std::vector<Collider*>& colliders) {
            std::vector<SphereCollider*> spheres{};
            std::vector<TerrainCollider*> terrains{};

            for (Collider* collider : colliders) {
                switch (collider->type) {
                case ColliderType::SPHERE:
                    spheres.push_back(static_cast<SphereCollider*>(collider));
                    break;
                case ColliderType::TERRAIN:
                    terrains.push_back(static_cast<TerrainCollider*>(collider));
                    break;
                }
            }

            solve(spheres);

            Physics physics{};
            for (SphereCollider* sphere : spheres) {
                for (TerrainCollider* terrain : terrains) {
                    physics.solve(*terrain, *sphere);
                }
            }
        }

        static void applyGravity(std::vector<Collider*>& colliders, float deltaTime, float gravity = -9.81f) {
            for (Collider* collider : colliders) {
                if (collider->type != ColliderType::SPHERE) continue;

                SphereCollider* sphere{ static_cast<SphereCollider*>(collider) };

                sphere->velocity.y += gravity * deltaTime;

                Vec3 pos{ sphere->transform->position() };
                pos += sphere->velocity * deltaTime;
                sphere->transform->position(pos);
            }
        }

        static Vec3 getTerrainNormal(Texture& tex, float x, float z, float dx = 1.0f) {
            float hL = getHeight(tex, x - dx, z);    
            float hR = getHeight(tex, x + dx, z);  
            float hD = getHeight(tex, x, z - dx); 
            float hU = getHeight(tex, x, z + dx); 

            Vec3 normal = Vec3{ hL - hR, 2.0f, hD - hU }.normalized(); 
            return normal;
        }
    };


}