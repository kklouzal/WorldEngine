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

	void updateUniformBuffer(uint32_t currentImage) {
		_Mesh->updateUniformBuffer(currentImage, false);
	}

	void drawFrame(VkCommandBuffer primaryCommandBuffer) {
		if (!Valid) {
			_Mesh->drawFrame(primaryCommandBuffer);
		}
	}
};

//
//	SceneGraph Create Function
TriangleMeshSceneNode* SceneGraph::createTriangleMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint16_t> indices) {

	TriangleMesh* Mesh = new TriangleMesh(_Driver, this, vertices, indices);
	TriangleMeshSceneNode* MeshNode = new TriangleMeshSceneNode(Mesh);
	SceneNodes.push_back(MeshNode);
	this->invalidate();
	return MeshNode;
}