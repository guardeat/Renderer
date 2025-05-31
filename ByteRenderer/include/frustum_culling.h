#pragma once

#include "camera.h"
#include "render_pass.h"
#include "context.h"
#include "renderer.h"

namespace Byte {

	class FrustumCullingPass : RenderPass {
	public:
		void render(RenderContext& context, RenderData& data) override {

		}
	};

}
