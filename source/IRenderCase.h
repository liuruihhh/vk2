#pragma once
#include "VkRHI.h"

class VkRHI;

class IRenderCase {
public:
	VkRHI* rhi;
	virtual ~IRenderCase() = 0;
	virtual void setup() = 0;
	virtual void cleanup() = 0;
	virtual void drawFrame(float delta, float time) = 0;
};