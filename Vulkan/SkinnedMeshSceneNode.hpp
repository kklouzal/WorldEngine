#pragma once

#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/skeleton_utils.h"

#include "ozz/base/maths/box.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/simd_quaternion.h"
#include "ozz/base/maths/soa_transform.h"
#include "ozz/base/maths/vec_float.h"

#include "ozz/base/containers/vector.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"

#include "PlaybackController.hpp"


class SkinnedMeshSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;

	PlaybackController controller_;
	// Runtime skeleton.
	ozz::animation::Skeleton skeleton_;
	// Runtime animation.
	ozz::animation::Animation animation_;
	// Buffer of local transforms as sampled from animation_.
	ozz::Vector<ozz::math::SoaTransform>::Std locals_;
	// Buffer of model space matrices.
	ozz::Vector<ozz::math::Float4x4>::Std models_;
	// Sampling cache.
	ozz::animation::SamplingCache cache_;

public:
	TriangleMesh* _Mesh = nullptr;
public:
	SkinnedMeshSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {

		printf("Loading Skeleton\n");
		ozz::io::File file_skel("media/skeleton.ozz", "rb");
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
		ozz::io::File file_anim("media/animation.ozz", "rb");
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

		const int num_soa_joints = skeleton_.num_soa_joints();
		locals_.resize(num_soa_joints);
		const int num_joints = skeleton_.num_joints();
		models_.resize(num_joints);
		// Allocates a cache that matches animation requirements.
		cache_.Resize(num_joints);

		printf("%i Joints - %i Joints\n", num_soa_joints, num_joints);
	}

	~SkinnedMeshSceneNode() {
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

		controller_.Update(animation_, 0.1f);

		// Samples optimized animation at t = animation_time_.
		ozz::animation::SamplingJob sampling_job;
		sampling_job.animation = &animation_;
		sampling_job.cache = &cache_;
		sampling_job.ratio = controller_.time_ratio();
		sampling_job.output = make_range(locals_);
		if (!sampling_job.Run()) {
			printf("Sampling Job Failed\n");
			return;
		}

		// Converts from local space to model space matrices.
		ozz::animation::LocalToModelJob ltm_job;
		ltm_job.skeleton = &skeleton_;
		ltm_job.input = make_range(locals_);
		ltm_job.output = make_range(models_);
		if (!ltm_job.Run()) {
			printf("LocalToModel Job Failed\n");
			return;
		}

		//	Somehow translate bones into our uniform buffer
		/*auto poses = skeleton_.joint_bind_poses();
		for (int i = 0; i < poses.count(); i++) {
			glm::mat4 BoneMat(poses[i].identity);
			ubo.bones[i] = BoneMat;
		}*/

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
SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(const char* FileFBX) {

	FBXObject* FBX = _ImportFBX->Import(FileFBX);

	TriangleMesh* Mesh = new TriangleMesh(_Driver, FBX->Vertices, FBX->Indices);
	SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh);
	SceneNodes.push_back(MeshNode);
	delete FBX;
	this->invalidate();
	return MeshNode;
}
SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint32_t> indices) {

	TriangleMesh* Mesh = new TriangleMesh(_Driver, vertices, indices);
	SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh);
	SceneNodes.push_back(MeshNode);
	this->invalidate();
	return MeshNode;
}