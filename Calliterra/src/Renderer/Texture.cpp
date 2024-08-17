#include "pch.h"
#include "Texture.h"
#include "Renderer.h"

const std::string Texture::GenerateUID(const std::string& filepath, uint32_t slot)
{
	using namespace std::string_literals;
	return typeid(Texture).name() + "#"s + filepath + "#" + std::to_string(slot);
}

std::shared_ptr<Texture> Texture::Resolve(const std::string& filepath, uint32_t slot)
{
	return Renderer::GetResourceLibrary().Resolve<Texture>(filepath, slot);
}
