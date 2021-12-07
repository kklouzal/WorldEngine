#pragma once

#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/skeleton_utils.h"

#include "ozz/animation/offline/additive_animation_builder.h"
#include "ozz/animation/offline/animation_builder.h"
#include "ozz/animation/offline/animation_optimizer.h"
#include "ozz/animation/offline/raw_animation.h"
#include "ozz/animation/offline/raw_skeleton.h"
#include "ozz/animation/offline/skeleton_builder.h"

#include "ozz/base/maths/box.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/simd_quaternion.h"
#include "ozz/base/maths/soa_transform.h"
#include "ozz/base/maths/vec_float.h"

#include "ozz/base/containers/vector.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"

#include "ozz_utils.h"
#include "ozz_mesh.h"

#include <ozz/options/options.h>


class SkinnedMeshSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	UniformBufferObject ubo = {};

	std::chrono::time_point<std::chrono::steady_clock> startFrame = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::steady_clock> endFrame = std::chrono::high_resolution_clock::now();
	float deltaFrame = 0;

	ozz::sample::PlaybackController controller_;
	// Runtime skeleton.
	ozz::animation::Skeleton skeleton_;
	// Runtime animation.
	ozz::animation::Animation animation_;
	// Buffer of local transforms as sampled from animation_.
	ozz::vector<ozz::math::SoaTransform> locals_;
	// Buffer of model space matrices.
	ozz::vector<ozz::math::Float4x4> models_;
	// Sampling cache.
	ozz::animation::SamplingCache cache_;

public:
	TriangleMesh* _Mesh = nullptr;
public:
	SkinnedMeshSceneNode(TriangleMesh* Mesh) : _Mesh(Mesh) {
		printf("Loading Skeleton\n");
		ozz::io::File file_skel("media/models/skeleton.ozz", "rb");
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
		ozz::io::File file_anim("media/models/Idle.ozz", "rb");
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
		printf("Destroy SkinnedMeshSceneNode\n");
		delete _Mesh;
	}

	void preDelete(btDiscreteDynamicsWorld* _BulletWorld)
	{
		_BulletWorld->removeRigidBody(_RigidBody);
	}

	void updateUniformBuffer(const uint32_t &currentImage) {
		endFrame = std::chrono::high_resolution_clock::now();
		deltaFrame = std::chrono::duration<double, std::milli>(endFrame - startFrame).count();
		startFrame = endFrame;

		ubo.model = Model;

		controller_.Update(animation_, deltaFrame);

		//	Samples optimized animation at t = animation_time_
		ozz::animation::SamplingJob sampling_job;
		sampling_job.animation = &animation_;
		sampling_job.cache = &cache_;
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

		auto joints = skeleton_.num_joints();
		//printf("UBO Joints %i\n", joints);
		for (int i = 0; i < joints; i++) {

			//	glm_row* == Final Bone Matrix Sent To GPU
			//	ozz_row* == Bone Matrix After Animation

			glm::vec4* glm_row1 = &ubo.bones[i][0];
			ozz::math::SimdFloat4 ozz_row1 = models_[i].cols[0];
			glm_row1->x = ozz_row1.m128_f32[0];
			glm_row1->y = ozz_row1.m128_f32[1];
			glm_row1->z = ozz_row1.m128_f32[2];
			glm_row1->w = ozz_row1.m128_f32[3];

			glm::vec4* glm_row2 = &ubo.bones[i][1];
			ozz::math::SimdFloat4 ozz_row2 = models_[i].cols[1];
			glm_row2->x = ozz_row2.m128_f32[0];
			glm_row2->y = ozz_row2.m128_f32[1];
			glm_row2->z = ozz_row2.m128_f32[2];
			glm_row2->w = ozz_row2.m128_f32[3];

			glm::vec4* glm_row3 = &ubo.bones[i][2];
			ozz::math::SimdFloat4 ozz_row3 = models_[i].cols[2];
			glm_row3->x = ozz_row3.m128_f32[0];
			glm_row3->y = ozz_row3.m128_f32[1];
			glm_row3->z = ozz_row3.m128_f32[2];
			glm_row3->w = ozz_row3.m128_f32[3];

			glm::vec4* glm_row4 = &ubo.bones[i][3];
			ozz::math::SimdFloat4 ozz_row4 = models_[i].cols[3];
			glm_row4->x = ozz_row4.m128_f32[0];
			glm_row4->y = ozz_row4.m128_f32[1];
			glm_row4->z = ozz_row4.m128_f32[2];
			glm_row4->w = ozz_row4.m128_f32[3];

			ubo.bones[i] = glm::transpose(ubo.bones[i]);

		}
		//	Send updated bone matrices to GPU
		_Mesh->updateUniformBuffer(currentImage, ubo);
	}

	void drawFrame(const VkCommandBuffer& CommandBuffer, const uint32_t& CurFrame) {
		if (!Valid) {
			_Mesh->draw(CommandBuffer, CurFrame);
		}
	}
};
