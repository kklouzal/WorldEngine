#pragma once

class SceneNode {
protected:
	uintmax_t NodeID;
	bool NeedsDelete;
	//
	btVector3 Position;
	btRigidBody* _RigidBody = nullptr;
	btCollisionShape* _CollisionShape = nullptr;
public:
	std::string Name = "N/A";
	bool isFrozen = false;
	bool canPhys = true;

public:
	SceneNode(btVector3 Position = btVector3(0.0f, 0.0f, 0.0f)) :
		NodeID(0), NeedsDelete(false), Position(Position) {}

	virtual ~SceneNode()
	{
		WorldEngine::dynamicsWorld->removeRigidBody(_RigidBody);
		delete _RigidBody->getMotionState();
		delete _RigidBody;
		wxLogMessage("[SceneNode] Destroy");
	}

	virtual void Tick(std::chrono::time_point<std::chrono::steady_clock> CurTime)
	{

	}

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

	const bool GetNeedsDelete()
	{
		return NeedsDelete;
	}

	void NetUpdate(btVector3 Origin, btVector3 Rotation, btVector3 LinearVelocity, btVector3 AngularVelocity)
	{
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setOrigin(Origin);
		Trans.setRotation(btQuaternion(Rotation.x(), Rotation.y(), Rotation.z()));
		_RigidBody->setWorldTransform(Trans);
		_RigidBody->setLinearVelocity(LinearVelocity);
		_RigidBody->setAngularVelocity(AngularVelocity);
	}
};

//
//	Bullet Motion State
class SceneNodeMotionState : public btMotionState {
	SceneNode* _SceneNode;
	btTransform _btPos;

public:
	SceneNodeMotionState(SceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos) {}

	//
	//	Sets our initial spawn position
	virtual void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
	}

	//
	//	Called whenever the physics representation of this SceneNode is finished moving
	virtual void setWorldTransform(const btTransform& worldTrans) {
	}
};