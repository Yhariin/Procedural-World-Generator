#pragma once
#include "Bindable.h"

class Renderer;

class Texture : public Bindable
{
public:
	virtual ~Texture() = default;

	virtual bool HasBlending() = 0;

	static const std::string GenerateUID(const std::string& filepath, uint32_t slot = 0);
	static std::shared_ptr<Texture> Resolve(const std::string& filepath, uint32_t slot = 0);

	
};
