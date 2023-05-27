#include <exception>
#include <chrono>
#include <GlFW/glfw3.h>
#include "VkRHI.h"
#include "IRenderCase.h"
#include "VkModel.h"
#include "VkGLTFModel.h"

int main() {
	std::chrono::steady_clock::time_point tStart, tEnd;
	float tDelta;
	VkRHI rhi;
	//VkModel model;
	VkGLTFModel model;
	model.rhi = &rhi;
	try {
		rhi.initWindow();
		rhi.initVulkan();
		model.setup();
		rhi.createFramebuffers();
		tStart = tEnd = std::chrono::high_resolution_clock::now();
		while (!glfwWindowShouldClose(rhi.window)) {
			glfwPollEvents();
			rhi.beginDrawFrame();
			tStart = std::chrono::high_resolution_clock::now();
			tDelta = std::chrono::duration<float, std::milli>(tStart - tEnd).count();
			model.drawFrame(tDelta);
			rhi.endDrawFrame();
			tEnd = std::chrono::high_resolution_clock::now();
		}
		vkDeviceWaitIdle(rhi.device);
		model.cleanup();
		rhi.cleanup();
		//rhi.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}