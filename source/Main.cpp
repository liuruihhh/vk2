#include <exception>
#include <GlFW/glfw3.h>
#include "VkRHI.h"
#include "VkModel.h"
#include "VkGLTFModel.h"

int main() {
	VkRHI rhi;
	//VkModel model;
	VkGLTFModel model;
	model.rhi = &rhi;
	try {
		rhi.initWindow();
		rhi.initVulkan();
		model.setup();
		rhi.createFramebuffers();
		while (!glfwWindowShouldClose(rhi.window)) {
			glfwPollEvents();
			rhi.beginDrawFrame();
			model.updateUniformBuffer();
			model.recordCommandBuffer();
			rhi.endDrawFrame();
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