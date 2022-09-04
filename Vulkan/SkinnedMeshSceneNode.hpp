#pragma once



#include "PlaybackController.hpp"

class SkinnedMeshSceneNode : public SceneNode
{
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};

	std::chrono::time_point<std::chrono::steady_clock> startFrame = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> endFrame = std::chrono::high_resolution_clock::now();
	float deltaFrame = 0;

	PlaybackController controller_;
	// Runtime skeleton.
	ozz::animation::Skeleton skeleton_;
	// Runtime animation.
	ozz::animation::Animation animation_;
	// Buffer of local transforms as sampled from animation_.
	ozz::vector<ozz::math::SoaTransform> locals_;
	// Buffer of model space matrices.
	ozz::vector<ozz::math::Float4x4> models_;
	// Sampling cache.
	ozz::animation::SamplingJob::Context context_;


	std::vector<glm::mat4> InverseBindMatrices_;
	std::map<std::string, uint16_t> _JointMap;
	std::map<std::string, uint16_t> _OZZJointMap;

public:
	TriangleMesh* _Mesh = nullptr;
public:
	SkinnedMeshSceneNode(TriangleMesh* Mesh, std::vector<glm::mat4> Inverse, std::map<std::string, uint16_t> Joints)
		: _Mesh(Mesh), InverseBindMatrices_(Inverse), _JointMap(Joints), SceneNode() {
		printf("Create SkinnedMeshSceneNode\n");
		Name = "SkinnedMeshSceneNode";
		printf("Loading Skeleton\n");
		ozz::io::File file_skel("media/models/cesium_man_skeleton.ozz", "rb");
		if (!file_skel.opened()) {
			printf("Skeleton Not Opened\n");
			return;
		}
		ozz::io::IArchive archive_skel(&file_skel);
		if (!archive_skel.TestTag<ozz::animation::Skeleton>()) {
			printf("Skeleton Archive Failed\n");
			return;
		}
		// Once the tag is validated, reading cannot fail.
		archive_skel >> skeleton_;

		printf("Loading Animation\n");
		ozz::io::File file_anim("media/models/cesium_man_animation_0.ozz", "rb");
		if (!file_anim.opened()) {
			printf("Animation Not Opened\n");
			return;
		}
		ozz::io::IArchive archive_anim(&file_anim);
		if (!archive_anim.TestTag<ozz::animation::Animation>()) {
			printf("Animation Archive Failed\n");
			return;
		}
		// Once the tag is validated, reading cannot fail.
		archive_anim >> animation_;

		// Skeleton and animation needs to match.
		if (skeleton_.num_joints() != animation_.num_tracks()) {
			printf("Skeleton and Animation Dont Match!\n");
			return;
		}

		const int num_soa_joints = skeleton_.num_soa_joints();
		locals_.resize(num_soa_joints);
		const int num_joints = skeleton_.num_joints();
		models_.resize(num_joints);
		// Allocates a cache that matches animation requirements.
		context_.Resize(num_joints);

		controller_.set_playback_speed(0.001f);

		printf("%i Joints - %i Joints\n", num_soa_joints, num_joints);

		for (size_t i = 0; i < skeleton_.joint_names().size(); i++)
		{
			const char* const JointName = skeleton_.joint_names()[i];
			_OZZJointMap[JointName] = i;
		}
		for (auto& jm : _OZZJointMap)
		{
			printf("OZZ JOINT MAPPED %s -> %u\n", jm.first.c_str(), jm.second);
		}
	}

	~SkinnedMeshSceneNode() {
		printf("Destroy SkinnedMeshSceneNode\n");
		delete _Mesh;
	}

	//
	//	TODO: Check for bNeedsUpdate
	void updateUniformBuffer(const uint32_t &currentImage) {
		endFrame = std::chrono::high_resolution_clock::now();
		deltaFrame = std::chrono::duration<double, std::milli>(endFrame - startFrame).count();
		startFrame = endFrame;

		ubo.model = Model;
		ubo.Animated = true;
		controller_.Update(animation_, deltaFrame);
		//	Samples optimized animation at t = animation_time_
		ozz::animation::SamplingJob sampling_job;
		sampling_job.animation = &animation_;
		sampling_job.context = &context_;
		sampling_job.ratio = controller_.time_ratio();
		sampling_job.output = make_span(locals_);
		if (!sampling_job.Run()) {
			printf("Sampling Job Failed\n");
			return;
		}

		//	Converts from local space to model space matrices
		ozz::animation::LocalToModelJob ltm_job;
		ltm_job.skeleton = &skeleton_;
		ltm_job.input = make_span(locals_);
		ltm_job.output = make_span(models_);
		if (!ltm_job.Run()) {
			printf("LocalToModel Job Failed\n");
			return;
		}

		//
		//	Iterate through OZZ LocalToModel job
		//	and build a vector of joint matrices
		auto joints = skeleton_.num_joints();
		std::vector<glm::mat4> Jnts;
		for (int i = 0; i < joints; i++)
		{
			std::string OZZ_JointName = skeleton_.joint_names()[i];
			uint16_t GLFW_JointIndex = _OZZJointMap[OZZ_JointName];

			glm::mat4 OZZ_Matrix = to_mat4(models_[i]);
			glm::mat4 GLFW_Matrix = InverseBindMatrices_[GLFW_JointIndex];

			Jnts.push_back(OZZ_Matrix * GLFW_Matrix);
			//Jnts.push_back(to_mat4(models_[i]) * InverseBindMatrices_[i]);
		}
		//
		//	Send updated uniform buffer to GPU
		_Mesh->updateUniformBuffer(currentImage, ubo);
		//
		//	Send updated bone matrices to GPU
		if (Jnts.size() > 0)
		{
			_Mesh->updateSSBuffer(currentImage, Jnts.data(), Jnts.size() * sizeof(glm::mat4));
		}
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
		}
	}
};

//
//	Bullet Motion State
class SkinnedMeshSceneNodeMotionState : public btMotionState {
	SkinnedMeshSceneNode* _SceneNode;
	glm::f32* ModelPtr;
	btTransform _btPos;

public:
	SkinnedMeshSceneNodeMotionState(SkinnedMeshSceneNode* Node, const btTransform& initialPos) : _SceneNode(Node), _btPos(initialPos), ModelPtr(glm::value_ptr(_SceneNode->Model)) {}

	virtual void getWorldTransform(btTransform& worldTrans) const {
		worldTrans = _btPos;
		_btPos.getOpenGLMatrix(ModelPtr);
	}

	virtual void setWorldTransform(const btTransform& worldTrans) {
		_btPos = worldTrans;
		_btPos.getOpenGLMatrix(ModelPtr);
		_SceneNode->bNeedsUpdate[0] = true;
		_SceneNode->bNeedsUpdate[1] = true;
		_SceneNode->bNeedsUpdate[2] = true;
		const btVector3 Pos = _btPos.getOrigin();
		if (_SceneNode->_Camera) {
			_SceneNode->_Camera->SetPosition(glm::vec3(Pos.x(), Pos.y(), Pos.z()) + _SceneNode->_Camera->getOffset());
		}
		//
		//	Update server with our new values
		// 
		//KNet::NetPacket_Send* Pkt = WorldEngine::NetCode::_Server->GetFreePacket((uint8_t)WorldEngine::NetCode::OPID::Player_PositionUpdate);
		//if (Pkt)
		//{
		//	Pkt->write<float>(Pos.x());															//	Player Position - X
		//	Pkt->write<float>(Pos.y());															//	Player Position - Y
		//	Pkt->write<float>(Pos.z());															//	Player Position - Z
		//	WorldEngine::NetCode::LocalPoint->SendPacket(Pkt);
		//}
	}
};