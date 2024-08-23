#pragma once

#include "buffer.h"
#include "mesh.h"
#include "material.h"
#include "transform.h"
#include "camera.h"
#include "light.h"

namespace Byte {

	struct RenderContext {
		Buffer<Mesh*> meshes;
		Buffer<Material*> materials;
		Buffer<Transform*> transforms;

		Camera* mainCamera;
		Transform* mainCameraTransform;

		DirectionalLight* directionalLight;
		Transform* directionalLightTransform;
	};

}