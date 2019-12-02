#pragma once

#include "Weapon.hpp"

class CharacterSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};
public:
	TriangleMesh* _Mesh = nullptr;
	Camera* _Camera = nullptr;
	Weapon _Weapon;
public:
	CharacterSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {}

	~CharacterSceneNode() {
		printf("Destroy CharacterSceneNode\n");
		delete _Mesh;
	}

	void moveForward(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		btVector3 Forward = Trans(btVector3(1 * Speed, 0, 0));
		Trans.setOrigin(Forward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveBackward(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		btVector3 Forward = Trans(btVector3(-1 * Speed, 0, 0));
		Trans.setOrigin(Forward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveLeft(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		btVector3 Forward = Trans(btVector3(0, 0, -1 * Speed));
		Trans.setOrigin(Forward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void moveRight(const btScalar& Speed) {
		btTransform Trans = _RigidBody->getWorldTransform();
		btVector3 Forward = Trans(btVector3(0, 0, 1 * Speed));
		Trans.setOrigin(Forward);
		_RigidBody->activate(true);
		_RigidBody->setWorldTransform(Trans);
	}

	void doJump(const btScalar& Speed) {
		glm::vec3 up = glm::vec3(Model[0][1], Model[1][1], Model[2][1]);
	}

	void setPosition(btVector3 NewPosition) {
		_RigidBody->activate(true);
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setOrigin(btVector3(NewPosition));
		_RigidBody->setWorldTransform(Trans);
	}

	void setYaw(float Yaw) {
		_RigidBody->activate(true);
		btTransform Trans = _RigidBody->getWorldTransform();
		Trans.setRotation(btQuaternion(glm::radians(-Yaw), 0, 0));
		_RigidBody->setWorldTransform(Trans);
	}

	void preDelete(btDiscreteDynamicsWorld* _BulletWorld) {
		_BulletWorld->removeRigidBody(_RigidBody);
	}

	void updateUniformBuffer(const uint32_t& currentImage) {
		ubo.model = Model;

		_Mesh->updateUniformBuffer(currentImage, ubo);
	}

	void drawFrame(const VkCommandBuffer& primaryCommandBuffer) {
		if (!Valid) {
			_Mesh->drawFrame(primaryCommandBuffer);
		}
	}
};

class CharacterSceneNodeMotionState : public btMotionState {
	CharacterSceneNode* _SceneNode;
	glm::f32* ModelPtr;
	btTransform _btPos;

public:
	CharacterSceneNodeMotionState(CharacterSceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	virtual void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		_btPos.getOpenGLMatrix(ModelPtr);
	}

	//Bullet only calls the update of worldtransform for active objects
	virtual void setWorldTransform(const btTransform& worldTrans) {
		_btPos = worldTrans;
		_btPos.getOpenGLMatrix(ModelPtr);
		if (_SceneNode->_Camera) {
			const btVector3 cmt = _SceneNode->_RigidBody->getCenterOfMassPosition();
			//printf("CMT %f %f %f\n", cmt.x(), cmt.y(), cmt.z());
			glm::vec3 NewCamPos(cmt.x(), cmt.y(), cmt.z());
			_SceneNode->_Camera->SetPosition(NewCamPos + _SceneNode->_Camera->getOffset());
		}
	}
};

//
//	SceneGraph Create Function
CharacterSceneNode* SceneGraph::createCharacterSceneNode(const char* FileFBX, btVector3 Position) {
	Pipeline::Default* Pipe = _Driver->_MaterialCache->GetPipe_Default();

	FBXObject* FBX = _ImportFBX->Import(FileFBX);
	std::string DiffuseFile("media/");
	DiffuseFile += FBX->Texture_Diffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile);
	if (DiffuseTex == nullptr) {
		return nullptr;
	}
	else {
		TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, FBX, DiffuseTex);
		btCollisionShape* ColShape;
		if (_CollisionShapes.count(FileFBX) == 0) {
			DecompResults* Results = Decomp(FBX);
			ColShape = Results->CompoundShape;
			_CollisionShapes[FileFBX] = ColShape;
			for (unsigned int i = 0; i < Results->m_convexShapes.size(); i++) {
				_ConvexShapes.push_back(Results->m_convexShapes[i]);
			}
			for (unsigned int i = 0; i < Results->m_trimeshes.size(); i++) {
				_TriangleMeshes.push_back(Results->m_trimeshes[i]);
			}
			delete Results;
		}
		else {
			ColShape = _CollisionShapes[FileFBX];
		}

		CharacterSceneNode* MeshNode = new CharacterSceneNode(Mesh);
		MeshNode->Name = "Character Scene Node";

		//
		//	Bullet Physics
		MeshNode->_CollisionShape = ColShape;
		btTransform Transform;
		Transform.setIdentity();
		Transform.setOrigin(Position);
		//Transform.setRotation(btQuaternion(btVector3(1, 0, 0), glm::radians(-90.0f)));

		btScalar Mass = 1.0f;
		bool isDynamic = (Mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic) {
			MeshNode->_CollisionShape->calculateLocalInertia(Mass, localInertia);
		}

		CharacterSceneNodeMotionState* MotionState = new CharacterSceneNodeMotionState(MeshNode, Transform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(Mass, MotionState, MeshNode->_CollisionShape, localInertia);
		MeshNode->_RigidBody = new btRigidBody(rbInfo);
		MeshNode->_RigidBody->setUserPointer(MeshNode);
		MeshNode->_RigidBody->setAngularFactor(btVector3(0.0f, 0.0f, 0.0f));
		dynamicsWorld->addRigidBody(MeshNode->_RigidBody);

		//
		//	Push new SceneNode into the SceneGraph
		SceneNodes.push_back(MeshNode);
		this->invalidate();
		return MeshNode;
	}
}