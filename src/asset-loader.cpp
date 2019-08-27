#include "asset-loader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static void initStaticMesh(IndexedModel& newModel, const aiMesh* mesh);

bool AssetLoader::loadAssets(const std::string& fileName,
		std::vector<IndexedModel>& models) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(fileName.c_str(),
			aiProcess_Triangulate | aiProcess_GenSmoothNormals
			| aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene) {
		DEBUG_LOG("Asset Loader", LOG_ERROR,
				"Failed to load assets from %s", fileName.c_str());
		return false;
	}

	for (uint32 i = 0; i < scene->mNumMeshes; ++i) {
		const aiMesh* mesh = scene->mMeshes[i];
		IndexedModel newModel;

		initStaticMesh(newModel, mesh);

		for (uint32 j = 0; j < mesh->mNumFaces; ++j) {
			const aiFace& face = mesh->mFaces[j];

			newModel.addIndices3i(face.mIndices[0],
					face.mIndices[1], face.mIndices[2]);
		}

		models.push_back(newModel);
	}

	return true;
}

static void initStaticMesh(IndexedModel& newModel, const aiMesh* mesh) {
	newModel.initStaticMesh();

	const aiVector3D aiZeroVector(0.f, 0.f, 0.f);

	for (uint32 i = 0; i < mesh->mNumVertices; ++i) {
		const aiVector3D pos = mesh->mVertices[i];
		const aiVector3D normal = mesh->mNormals[i];
		const aiVector3D texCoord = mesh->HasTextureCoords(0)
				? mesh->mTextureCoords[0][i] : aiZeroVector;
		const aiVector3D tangent = mesh->mTangents[i];

		newModel.addElement3f(0, pos.x, pos.y, pos.z);
		newModel.addElement2f(1, texCoord.x, texCoord.y);
		newModel.addElement3f(2, normal.x, normal.y, normal.z);
		newModel.addElement3f(3, tangent.x, tangent.y, tangent.z);
	}
}
