#pragma once
#include "Renderer/RenderQueue/Step.h"

enum class PassName : int
{
	None = -1,
	ClearRenderTarget,
	ClearDepthStencilBuffer,
	Lambertian,
	OutlineMask,
	OutlineDraw,
	ColorInvert,
	NumPasses
};

class RenderPass
{
public:
	virtual ~RenderPass() = default;

	virtual void Execute() const = 0; 
	virtual void Reset() {}

};

