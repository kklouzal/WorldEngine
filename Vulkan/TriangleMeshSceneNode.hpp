#pragma once

class TriangleMeshSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
public:
	TriangleMesh* _Mesh = nullptr;
public:
	TriangleMeshSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~TriangleMeshSceneNode() {
#ifdef _DEBUG
		std::cout << "Destroy TriangleMeshSceneNode" << std::endl;
#endif
		delete _Mesh;
	}

	void updateUniformBuffer(const uint32_t &currentImage) {

		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo = {};
		ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		_Mesh->updateUniformBuffer(currentImage, ubo);
	}

	void drawFrame(const VkCommandBuffer &primaryCommandBuffer) {
		if (!Valid) {
			_Mesh->drawFrame(primaryCommandBuffer);
		}
	}
};

//
//	SceneGraph Create Function
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const char* FileFBX) {

	FBXObject* FBX = _ImportFBX->Import(FileFBX);

	TriangleMesh* Mesh = new TriangleMesh(_Driver, FBX->Vertices, FBX->Indices);
	TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
	SceneNodes.push_back(MeshNode);
	delete FBX;
	this->invalidate();
	return MeshNode;
}
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const std::vector<Vertex> Vertices, const std::vector<uint32_t> Indices) {

	TriangleMesh* Mesh = new TriangleMesh(_Driver, Vertices, Indices);
	TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
	SceneNodes.push_back(MeshNode);
	this->invalidate();
	return MeshNode;
}