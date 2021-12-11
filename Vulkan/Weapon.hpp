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

	/*void Primary(btCollisionWorld::ClosestRayResultCallback Ray) {
		if (Ray.hasHit()) {
			printf("Hit\n");
			const btCollisionObject* HitObj = Ray.m_collisionObject;
			SceneNode* HitNode = (SceneNode*)HitObj->getUserPointer();
			std::string HitName = HitNode->Name;
			printf("\t%s\n", HitName.c_str());
			HitNode->_RigidBody->activate(true);
			HitNode->_RigidBody->applyCentralForce(Ray.m_hitNormalWorld * -500.0f);
		}
		else {
			printf("No Hit\n");
		}
	}*/

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