#pragma once

class SceneNode {
private:
protected:
	uintmax_t NodeID;
	btRigidBody* _RigidBody = nullptr;
	btCollisionShape* _CollisionShape = nullptr;
public:
	glm::mat4 Model{};
	glm::vec3 Pos{};
	glm::vec3 Rot{};
	std::string Name = "N/A";
	Camera* _Camera = nullptr;
	bool isFrozen = false;
	bool canPhys = true;
	//
	//	per frame check
	bool bNeedsUpdate[3];
public:
	SceneNode()
	{
		bNeedsUpdate[0] = true;
		bNeedsUpdate[1] = true;
		bNeedsUpdate[2] = true;
	}
	virtual ~SceneNode()
	{
		WorldEngine::VulkanDriver::dynamicsWorld->removeRigidBody(_RigidBody);
		delete _RigidBody->getMotionState();
		delete _RigidBody;
		printf("Destroy Base SceneNode\n");
	}
	virtual void updateUniformBuffer(const uint32_t &currentImage) = 0;
	virtual void drawFrame(const VkCommandBuffer &CommandBuffer, const uint32_t &CurFrame) = 0;
	virtual void drawGUI() {}

	void SetNodeID(const uintmax_t ID)
	{
		if (NodeID == 0)
		{
			NodeID = ID;
		}
	}

	const uintmax_t GetNodeID()
	{
		return NodeID;
	}

	void NetUpdate(btVector3 Origin, btVector3 Rotation, btVector3 LinearVelocity, btVector3 AngularVelocity)
	{
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setOrigin(Origin);
		Trans.setRotation(btQuaternion(Rotation.x(), Rotation.y(), Rotation.z()));
		_RigidBody->setWorldTransform(Trans);
		_RigidBody->setLinearVelocity(LinearVelocity);
		_RigidBody->setAngularVelocity(AngularVelocity);

		bNeedsUpdate[0] = true;
		bNeedsUpdate[1] = true;
		bNeedsUpdate[2] = true;
	}

	btVector3 GetLinearVelocity()
	{
		return _RigidBody->getLinearVelocity();
	}

	btVector3 GetAngularVelocity()
	{
		return _RigidBody->getAngularVelocity();
	}

	friend WorldSceneNode* WorldEngine::SceneGraph::createWorldSceneNode(uintmax_t NodeID, const char* File);
	friend CharacterSceneNode* WorldEngine::SceneGraph::createCharacterSceneNode(uintmax_t NodeID, const char* File, const btVector3& Position);
	friend SkinnedMeshSceneNode* WorldEngine::SceneGraph::createSkinnedMeshSceneNode(uintmax_t NodeID, const char* File, const float& Mass, const btVector3& Position);
	friend TriangleMeshSceneNode* WorldEngine::SceneGraph::createTriangleMeshSceneNode(uintmax_t NodeID, const char* File, const float& Mass, const btVector3& Position);
};

//
//	Bullet Motion State
class SceneNodeMotionState : public btMotionState {
	SceneNode* _SceneNode;
	glm::f32* ModelPtr;
	btTransform _btPos;

public:
	SceneNodeMotionState(SceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	//
	//	Sets our initial spawn position
	virtual void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		worldTrans.getOpenGLMatrix(ModelPtr);
	}

	//
	//	Called whenever the physics representation of this SceneNode is finished moving
	virtual void setWorldTransform(const btTransform& worldTrans) {
		worldTrans.getOpenGLMatrix(ModelPtr);
		_SceneNode->bNeedsUpdate[0] = true;
		_SceneNode->bNeedsUpdate[1] = true;
		_SceneNode->bNeedsUpdate[2] = true;
	}
};

//
#include "Item.hpp"
//
#include "WorldSceneNode.hpp"
#include "CharacterSceneNode.hpp"
#include "TriangleMeshSceneNode.hpp"
#include "SkinnedMeshSceneNode.hpp"