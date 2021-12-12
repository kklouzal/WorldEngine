#pragma once

/*class RayCast_cb : public btTriangleRaycastCallback {
public:
	RayCast_cb(const btVector3& from, const btVector3& to) :
		btTriangleRaycastCallback(from, to, kF_None) {
		printf("RayCastCallback_::RayCastCallback_\n");
	};

	~RayCast_cb(void) {};

	btScalar reportHit(const btVector3& hitNormalLocal, btScalar hitFraction, int partId, int triangleIndex) {
		printf("hitFraction = " << hitFraction << "  triangleIndex = " << triangleIndex);
		return  (hitFraction < m_hitFraction ? hitFraction : m_hitFraction);
	};
};*/

class Weapon {
public:
	Weapon() {}
	~Weapon() {}

	void Primary(ndRayCastClosestHitCallback& CB) {

		if (CB.m_contact.m_body0) {
			printf("Hit1\n");
			SceneNode* Node = (SceneNode*)(CB.m_contact.m_body0->GetNotifyCallback()->GetUserData());
			printf("Node Name: %s\n", Node->Name.c_str());
			/*const btCollisionObject* HitObj = Ray.m_collisionObject;
			SceneNode* HitNode = (SceneNode*)HitObj->getUserPointer();
			std::string HitName = HitNode->Name;
			printf("\t%s\n", HitName.c_str());
			HitNode->_RigidBody->activate(true);
			HitNode->_RigidBody->applyCentralForce(Ray.m_hitNormalWorld * -500.0f);*/
		}
		else {
			printf("No Hit1\n");
		}
		if (CB.m_contact.m_body1) {
			printf("Hit2\n");
			SceneNode* Node = (SceneNode*)(CB.m_contact.m_body0->GetNotifyCallback()->GetUserData());
			printf("Node Name: %s\n", Node->Name.c_str());
			/*const btCollisionObject* HitObj = Ray.m_collisionObject;
			SceneNode* HitNode = (SceneNode*)HitObj->getUserPointer();
			std::string HitName = HitNode->Name;
			printf("\t%s\n", HitName.c_str());
			HitNode->_RigidBody->activate(true);
			HitNode->_RigidBody->applyCentralForce(Ray.m_hitNormalWorld * -500.0f);*/
		}
		else {
			printf("No Hit2\n");
		}
	}

	void Secondary() {

	}

	void ShiftPrimary() {

	}

	void ShiftSecondary() {

	}

	void CtrlPrimary() {

	}

	void CtrlSecondary() {

	}

	void AltPrimary() {

	}

	void AltSecondary() {

	}
};