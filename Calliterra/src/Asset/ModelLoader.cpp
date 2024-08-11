#include "pch.h"
#include "ModelLoader.h"
#include "fastgltf/tools.hpp"
#include "fastgltf/math.hpp"

std::shared_ptr<rapidobj::Result> ModelLoader::LoadModelObj(const std::filesystem::path& filepath)
{
	std::shared_ptr<rapidobj::Result> model = std::make_shared<rapidobj::Result>(rapidobj::ParseFile(filepath, rapidobj::MaterialLibrary::Ignore()));
	ASSERT(!model->error, model->error.code.message());
	ASSERT_VERIFY(Triangulate(*model.get()), model->error.code.message());

	return model;
}

std::shared_ptr<fastgltf::Asset> ModelLoader::LoadModelGltf(const std::filesystem::path& filepath)
{
	auto gltfFile = fastgltf::MappedGltfFile::FromPath(filepath);
	ASSERT(bool(gltfFile), fastgltf::getErrorMessage(gltfFile.error()));

	auto asset = (m_GltfParser.loadGltf(gltfFile.get(), filepath.parent_path(), m_GltfOptions));
	ASSERT(asset.error() == fastgltf::Error::None, fastgltf::getErrorMessage(asset.error()));

	return std::make_shared<fastgltf::Asset>(std::move(asset.get()));
}

std::shared_ptr<UfbxScene> ModelLoader::LoadModelFbx(const std::filesystem::path& filepath)
{
	ufbx_load_opts opts = { }; // Optional, pass NULL for defaults
	ufbx_error error; // Optional, pass NULL if you don't care about errors

	std::shared_ptr<UfbxScene> scene = std::make_shared<UfbxScene>(ufbx_load_file(ufbx_string_view(filepath.generic_string().c_str(), filepath.generic_string().length()), &opts, &error));
	ASSERT(scene.get(), error.description.data);

	return scene;
}

std::vector<std::unique_ptr<Mesh>> ModelLoader::GetModelMeshes(const rapidobj::Result& model, const DX::XMMATRIX& transform, DX::XMFLOAT3 color)
{
	std::vector<std::unique_ptr<Mesh>> meshes;

	for (int i = 0; i < model.shapes.size(); i++)
	{
		meshes.emplace_back(std::make_unique<Mesh>(i, model, transform, color));
	}

	return meshes;
}

std::vector<std::unique_ptr<Mesh>> ModelLoader::GetModelMeshes(const fastgltf::Asset& model, const DX::XMMATRIX& transform, DX::XMFLOAT3 color)
{
	std::vector<std::unique_ptr<Mesh>> meshes;

	for (int i = 0; i < model.meshes.size(); i++)
	{
		meshes.emplace_back(std::make_unique<Mesh>(i, model, transform, color));
	}

	return meshes;
}

std::vector<std::unique_ptr<Mesh>> ModelLoader::GetModelMeshes(const UfbxScene& model, const DX::XMMATRIX& transform, DX::XMFLOAT3 color)
{
	std::vector<std::unique_ptr<Mesh>> meshes;
	return meshes;
}

std::vector<ModelVertex> ModelLoader::GetMeshVertexVector(const rapidobj::Result& objModel, int meshIndex)
{
	const rapidobj::Mesh& mesh = objModel.shapes[meshIndex].mesh;
	std::vector<ModelVertex> vertices;

	for (int i = 0; i < objModel.shapes[meshIndex].mesh.num_face_vertices.size(); i++)
	{
		ASSERT(objModel.shapes[meshIndex].mesh.num_face_vertices[i] == 3);

		for (int j = 0; j < 3; j++)
		{
			int posIndex = mesh.indices[i * 3 + j].position_index;
			int normIndex = mesh.indices[i * 3 + j].normal_index;

			DX::XMFLOAT3 pos;
			DX::XMFLOAT3 norm;

			pos.x = objModel.attributes.positions[posIndex * 3];
			pos.y = objModel.attributes.positions[posIndex * 3 + 1];
			pos.z = objModel.attributes.positions[posIndex * 3 + 2];

			norm.x = objModel.attributes.normals[normIndex * 3];
			norm.y = objModel.attributes.normals[normIndex * 3 + 1];
			norm.z = objModel.attributes.normals[normIndex * 3 + 2];

			vertices.push_back({ pos, norm });
		}

	}

	ASSERT(vertices.size() == mesh.num_face_vertices.size() * 3);

	return vertices;
}

std::vector<uint32_t> ModelLoader::GetMeshIndexVector(const rapidobj::Result& objModel, int meshIndex)
{
	std::vector<uint32_t> indices(objModel.shapes[meshIndex].mesh.num_face_vertices.size() * 3);

	std::generate(indices.begin(), indices.end(), [index = 0]() mutable { return index++; });

	return indices;
}

std::vector<ModelVertex> ModelLoader::GetMeshVertexVector(const fastgltf::Asset& objModel, int meshIndex)
{
	std::vector<ModelVertex> vertices;

	for (auto it = objModel.meshes[meshIndex].primitives.begin(); it < objModel.meshes[meshIndex].primitives.end(); it++)
	{
		DX::XMFLOAT3 position;
		DX::XMFLOAT3 normal;

		auto* positionIt = it->findAttribute("POSITION");
		ASSERT(positionIt != it->attributes.end()); // A mesh primitive is required to hold the POSITION attribute
		ASSERT(it->indicesAccessor.has_value()); // We specify GenerateMeshIndices, so we should always have indices

		auto& positionAccessor = objModel.accessors[positionIt->accessorIndex];
		if (!positionAccessor.bufferViewIndex.has_value())
		{
			continue;
		}

		fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(objModel, positionAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) {
			position.x = pos.x();
			position.y = pos.y();
			position.z = pos.z();
			
			vertices.push_back({ position, {} });
		});

		auto* normalIt = it->findAttribute("NORMAL");
		ASSERT(normalIt != it->attributes.end()); // A mesh primitive is required to hold the POSITION attribute
		ASSERT(it->indicesAccessor.has_value()); // We specify GenerateMeshIndices, so we should always have indices

		auto& normalAccessor = objModel.accessors[normalIt->accessorIndex];
		if (!normalAccessor.bufferViewIndex.has_value())
		{
			continue;
		}

		fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(objModel, normalAccessor, [&](fastgltf::math::fvec3 norm, std::size_t idx) {
			normal.x = norm.x();
			normal.y = norm.y();
			normal.z = norm.z();
			
			vertices[idx].Normal = normal;
		});

		vertices.push_back({ position, normal });
	}
	
	return vertices;
}

std::vector<uint32_t> ModelLoader::GetMeshIndexVector(const fastgltf::Asset& objModel, int meshIndex)
{
	std::vector<uint32_t> indices;
	for (auto it = objModel.meshes[meshIndex].primitives.begin(); it < objModel.meshes[meshIndex].primitives.end(); it++)
	{
		auto& indexAccessor = objModel.accessors[it->indicesAccessor.value()];
		ASSERT(indexAccessor.bufferViewIndex.has_value());

		indices.resize(indexAccessor.count);

		fastgltf::copyFromAccessor<std::uint32_t>(objModel, indexAccessor, indices.data());
	}

	return indices;
}

std::vector<ModelVertex> ModelLoader::GetModelVertexVector(const UfbxScene& objModel)
{
	std::vector<ModelVertex> vertices;
	auto& mesh = objModel.Meshes()[0];

	std::vector<uint32_t> tri_indices;
	tri_indices.resize(mesh->max_face_triangles * 3);

	for (ufbx_face face : mesh->faces)
	{

		// Triangulate the face into tri_indices[]
		uint32_t num_tris = ufbx_triangulate_face(tri_indices.data(), tri_indices.size(), mesh, face);

		for (uint32_t i = 0; i < num_tris * 3; i++)
		{
			uint32_t index = tri_indices[i];

            ufbx_vec3 position = mesh->vertex_position[index];
            ufbx_vec3 normal = mesh->vertex_normal[index];

			vertices.push_back({ {position.x, position.y, position.z}, {normal.x, normal.y, normal.z} });
		}

    }
	ASSERT(vertices.size() == mesh->num_triangles * 3);

	return vertices;
}

std::vector<uint32_t> ModelLoader::GetModelIndexVector(const UfbxScene& objModel, std::vector<ModelVertex>& vertices)
{
	std::vector<uint32_t> indices;
	auto& mesh = objModel.Meshes()[0];
	indices.resize(mesh->num_triangles * 3);

	ufbx_vertex_stream stream[1] = { {vertices.data(), vertices.size(), sizeof(ModelVertex)} };

	uint32_t num_vertices = static_cast<uint32_t>(ufbx_generate_indices(stream, 1, indices.data(), indices.size(), nullptr, nullptr));

	vertices.resize(num_vertices);
	return indices;

}

#pragma warning(disable:4715) // Disable warning about no return for all control paths since we are always guarenteed to return
const fastgltf::Node& ModelLoader::GetRootNode(const fastgltf::Asset& model)
{
	std::map<size_t, size_t> nodeMap;
	for (size_t i = 0; i < model.nodes.size(); i++)
	{
		nodeMap[i] = -1;
		for (int j = 0; j < model.nodes[i].children.size(); j++)
		{
			nodeMap[model.nodes[i].children[j]] = i;
		}
	}

	for (auto it = nodeMap.begin(); it != nodeMap.end(); it++)
	{
		if (it->second == -1) return model.nodes[it->first];
	}

	ASSERT(false, "No Root node found!");
	#pragma warning(default:4715)
}

DX::XMMATRIX ModelLoader::GetMeshTransform(const fastgltf::Asset& model, const fastgltf::Node& node)
{
	DX::XMMATRIX transform;
	if (auto value = std::get_if<fastgltf::math::fmat4x4>(&node.transform))
	{
		transform = DX::XMMatrixTranspose(DX::XMLoadFloat4x4(reinterpret_cast<const DX::XMFLOAT4X4*>(&node.transform)));
	}
	else
	{
		auto trs = std::get<fastgltf::TRS>(node.transform);
		transform =
			DX::XMMatrixScaling(trs.scale.x(), trs.scale.y(), trs.scale.z()) *
			DX::XMMatrixRotationRollPitchYaw(trs.rotation.x(), trs.rotation.y(), trs.rotation.z()) *
			DX::XMMatrixTranslation(trs.translation.x(), trs.translation.y(), trs.translation.z());
	}

	return transform;
}

std::unique_ptr<Node> ModelLoader::ParseNode(const fastgltf::Asset& model, const fastgltf::Node& node, const std::vector<std::unique_ptr<Mesh>>& modelMeshes)
{
	DX::XMMATRIX transform = GetMeshTransform(model, node);

	std::vector<Mesh*> currentMesh;
	currentMesh.push_back(modelMeshes.at(node.meshIndex.value()).get());

	std::unique_ptr<Node> nextNode = std::make_unique<Node>(static_cast<int>(node.meshIndex.value()), node.name.c_str(), std::move(currentMesh), transform);
	for (int i = 0; i < node.children.size(); i++)
	{
		nextNode->AddChild(ParseNode(model, model.nodes[node.children[i]], modelMeshes));
	}

	return nextNode;

}

std::unique_ptr<Node> ModelLoader::ParseNode(const rapidobj::Result& model, const std::vector<std::unique_ptr<Mesh>>& modelMeshes)
{
	std::vector<Mesh*> currentMeshes;
	for (auto& mesh : modelMeshes)
	{
		currentMeshes.push_back(mesh.get());
	}
	
	std::unique_ptr<Node> root = std::make_unique<Node>(0, "TestName", std::move(currentMeshes), DX::XMMatrixIdentity());

	return root;
}
