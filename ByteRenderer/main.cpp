#include <chrono>

#include "test.h"
#include "render.h"

using namespace Byte;

struct SphereCollider {
	float radius{ 1.0f };
};

struct RigidBody {
	float speed{ 0.0 };
	float mass{ 1.0f };
};

struct Entity {
	Mesh mesh;
	Material material;
	Transform transform;
	RigidBody rigidBody;
	SphereCollider collider;
};

inline bool collided(Entity& left, Entity& right) {
	Vec3 delta = left.transform.position() - right.transform.position();
	float distance = delta.length();
	float radiusSum = left.collider.radius + right.collider.radius;

	return distance < radiusSum;
}

inline void solveCollision(Entity& left, Entity& right) {
	Vec3 delta = left.transform.position() - right.transform.position();
	float distance = delta.length();
	float radiusSum = left.collider.radius + right.collider.radius;

	if (distance < radiusSum) {
		Vec3 collisionNormal = delta.normalized();
		float penetrationDepth = radiusSum - distance;

		Vec3 correction = collisionNormal * (penetrationDepth / 2.0f);
		left.transform.position(left.transform.position() + correction);
		right.transform.position(right.transform.position() - correction);
	}
}

inline void handleCollisions(Buffer<Entity*>& entities) {
	for (size_t i = 0; i < entities.size(); ++i) {
		for (size_t j = i + 1; j < entities.size(); ++j) {
			if (collided(*entities.at(i), *entities.at(j))) {
				solveCollision(*entities.at(i), *entities.at(j));
			}
		}
	}
}

inline void handleGravity(Buffer<Entity*>& entities, float dt) {
	
	const float gravity = -9.81f;

	for (Entity* entity : entities) {
		entity->rigidBody.speed += gravity * dt;

		Vec3 currentPosition = entity->transform.position();
		Vec3 newPosition = currentPosition;
		newPosition.y += entity->rigidBody.speed * dt;

		if (newPosition.y <= entity->collider.radius) {
			newPosition.y = entity->collider.radius;
			entity->rigidBody.speed = 0.0f;
		}

		entity->transform.position(newPosition);
	}
}

int main() {
	glfwInit();

	Window window{ 1336,768,"Test" };

	Renderer renderer{ RendererGenerator::deferred(window) };

	Camera camera;
	Transform transform;
	transform.position(Vec3{ -10.0f,10.0f,5.0f });
	FPSCamera fpsCamera;

	DirectionalLight dLight;
	Transform dLightTransform;
	dLightTransform.rotate(Vec3{ -45.0f,20.0f,0.0f });
	dLightTransform.position(Vec3(50.0f,80.0f,80.0f));

	renderer.context().submit(camera, transform);
	renderer.context().submit(dLight,dLightTransform);

	Mesh plane(MeshBuilder::plane(100,100,1));
	Material planeMaterial;
	planeMaterial.albedo(Vec4(0.2f, 0.7f, 0.2f, 1.0f));
	Transform planeTransform;
	planeTransform.rotate(Vec3(270.0f, 0.0f, 0.0f));
	
	renderer.context().submit(plane,planeMaterial,planeTransform);

	auto lastTime{ std::chrono::high_resolution_clock::now() };
	int frameCount{ 0 };
	float fpsTimer{ 0.0f };

	float sphereRadius{ 1.0f };

	Buffer<Entity*> entities{ 
		new Entity{MeshBuilder::sphere(1,10),Material{},Transform{transform}, RigidBody{}, SphereCollider{5.0f}} };

	while (!glfwWindowShouldClose(window.glfwWindow)) {
		renderer.render();
		renderer.update(window);
		fpsCamera.update(window, transform);
		glfwPollEvents();

		entities.at(0)->transform = transform;

		if (glfwGetKey(window.glfwWindow, GLFW_KEY_UP) == GLFW_PRESS) {
			sphereRadius += 0.01f; 
		}

		if (glfwGetKey(window.glfwWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
			sphereRadius -= 0.01f; 
			if (sphereRadius < 0.1f) sphereRadius = 0.1f;
		}

		if (glfwGetMouseButton(window.glfwWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			entities.push_back(new Entity{
				MeshBuilder::sphere(sphereRadius, 20),
				Material{},
				Transform{transform},
				RigidBody{},
				SphereCollider{sphereRadius}
				});
			renderer.context().submit(entities.back()->mesh, entities.back()->material, entities.back()->transform);
			entities.back()->material.albedo(Vec4(1.0f, 0.0f, 1.0f, 1.0f));
		}

		auto currentTime{ std::chrono::high_resolution_clock::now() };
		float deltaTime{ std::chrono::duration<float>(currentTime - lastTime).count() };
		lastTime = currentTime;

		handleGravity(entities, deltaTime);
		handleCollisions(entities);

		frameCount++;
		fpsTimer += deltaTime;

		if (fpsTimer >= 1.0f) {
			std::cout << "FPS: " << frameCount << std::endl;
			std::cout << "ENTITY COUNT: " << entities.size() << std::endl;
			GLenum error{ glGetError() };
			if (error) {
				std::cout << "GRAPHIC ERROR: " << error << std::endl;
			}
			frameCount = 0;
			fpsTimer = 0.0f;
		}
	}

	return 0;
}