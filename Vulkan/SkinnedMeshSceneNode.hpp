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
	// Per Bone Bind Pose.
	std::vector<FbxAMatrix> bindPoses = {};

public:
	TriangleMesh* _Mesh = nullptr;
public:
	SkinnedMeshSceneNode(TriangleMesh* Mesh, std::vector<FbxAMatrix>& binds) : _Mesh(Mesh) {
		bindPoses.swap(binds);
		printf("Loading Skeleton\n");
		ozz::io::File file_skel("media/arnaud/skeleton.ozz", "rb");
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
		ozz::io::File file_anim("media/arnaud/take 001.ozz", "rb");
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

		controller_.Update(animation_, 1.0f);

		//	Samples optimized animation at t = animation_time_
		ozz::animation::SamplingJob sampling_job;
		sampling_job.animation = &animation_;
		sampling_job.cache = &cache_;
		sampling_job.ratio = controller_.time_ratio();
		sampling_job.output = make_range(locals_);
		if (!sampling_job.Run()) {
			printf("Sampling Job Failed\n");
			return;
		}

		//	Converts from local space to model space matrices
		ozz::animation::LocalToModelJob ltm_job;
		ltm_job.skeleton = &skeleton_;
		ltm_job.input = make_range(locals_);
		ltm_job.output = make_range(models_);
		if (!ltm_job.Run()) {
			printf("LocalToModel Job Failed\n");
			return;
		}

		auto joints = skeleton_.num_joints();
		//printf("UBO Joints %i\n", joints);
		for (int i = 0; i < joints; i++) {

			if (i >= bindPoses.size()) { break; }

			FbxAMatrix InvBind = bindPoses[i].Inverse();

			//	glm_row* == Final Bone Matrix Sent To GPU
			//	ozz_row* == Bone Matrix After Animation
			//	fbx_row* == Bone Inverse Bind Pose

			glm::vec4* glm_row1 = &ubo.bones[i][0];
			ozz::math::SimdFloat4 ozz_row1 = models_[i].cols[0];
			FbxDouble4 fbx_row1 = InvBind.mData[0];
			glm_row1->x = ozz_row1.m128_f32[0];// *fbx_row1.mData[0];
			glm_row1->y = ozz_row1.m128_f32[1];// *fbx_row1.mData[1];
			glm_row1->z = ozz_row1.m128_f32[2];// *fbx_row1.mData[2];
			glm_row1->w = ozz_row1.m128_f32[3];// *fbx_row1.mData[3];

			glm::vec4* glm_row2 = &ubo.bones[i][1];
			ozz::math::SimdFloat4 ozz_row2 = models_[i].cols[1];
			FbxDouble4 fbx_row2 = InvBind.mData[1];
			glm_row2->x = ozz_row2.m128_f32[0];// *fbx_row2.mData[0];
			glm_row2->y = ozz_row2.m128_f32[1];// *fbx_row2.mData[1];
			glm_row2->z = ozz_row2.m128_f32[2];// *fbx_row2.mData[2];
			glm_row2->w = ozz_row2.m128_f32[3];// *fbx_row2.mData[3];

			glm::vec4* glm_row3 = &ubo.bones[i][2];
			ozz::math::SimdFloat4 ozz_row3 = models_[i].cols[2];
			FbxDouble4 fbx_row3 = InvBind.mData[2];
			glm_row3->x = ozz_row3.m128_f32[0];// *fbx_row3.mData[0];
			glm_row3->y = ozz_row3.m128_f32[1];// *fbx_row3.mData[1];
			glm_row3->z = ozz_row3.m128_f32[2];// *fbx_row3.mData[2];
			glm_row3->w = ozz_row3.m128_f32[3];// *fbx_row3.mData[3];

			glm::vec4* glm_row4 = &ubo.bones[i][3];
			ozz::math::SimdFloat4 ozz_row4 = models_[i].cols[3];
			FbxDouble4 fbx_row4 = InvBind.mData[3];
			glm_row4->x = ozz_row4.m128_f32[0];// *fbx_row4.mData[0];
			glm_row4->y = ozz_row4.m128_f32[1];// *fbx_row4.mData[1];
			glm_row4->z = ozz_row4.m128_f32[2];// *fbx_row4.mData[2];
			glm_row4->w = ozz_row4.m128_f32[3];// *fbx_row4.mData[3];

			//ubo.bones[i] = glm::transpose(ubo.bones[i]);

			glm::mat4 InverseBind = {};
			InverseBind[0].x = fbx_row1.mData[0];
			InverseBind[0].y = fbx_row1.mData[1];
			InverseBind[0].z = fbx_row1.mData[2];
			InverseBind[0].w = fbx_row1.mData[3];

			InverseBind[1].x = fbx_row2.mData[0];
			InverseBind[1].y = fbx_row2.mData[1];
			InverseBind[1].z = fbx_row2.mData[2];
			InverseBind[1].w = fbx_row2.mData[3];

			InverseBind[2].x = fbx_row3.mData[0];
			InverseBind[2].y = fbx_row3.mData[1];
			InverseBind[2].z = fbx_row3.mData[2];
			InverseBind[2].w = fbx_row3.mData[3];

			InverseBind[3].x = fbx_row4.mData[0];
			InverseBind[3].y = fbx_row4.mData[1];
			InverseBind[3].z = fbx_row4.mData[2];
			InverseBind[3].w = fbx_row4.mData[3];

			ubo.bones[i] *= InverseBind;
		}
		//	Send updated bone matrices to GPU
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
	Pipeline::Skinned* Pipe = _Driver->_MaterialCache->GetPipe_Skinned();

	FBXObject* FBX = _ImportFBX->Import(FileFBX);

	std::string DiffuseFile("media/");
	DiffuseFile += FBX->Texture_Diffuse;
	TextureObject* DiffuseTex = Pipe->createTextureImage(DiffuseFile.c_str());

	if (DiffuseTex == nullptr) {
		delete FBX;
		return nullptr;
	}
	else {
		TriangleMesh* Mesh = new TriangleMesh(_Driver, Pipe, FBX->Vertices, FBX->Indices, DiffuseTex);
		SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh, FBX->bindPoses);
		SceneNodes.push_back(MeshNode);
		delete FBX;
		this->invalidate();
		return MeshNode;
	}
}