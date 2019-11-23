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

class ScratchBuffer {
public:
	ScratchBuffer() : buffer_(NULL), size_(0) {}

	~ScratchBuffer() {
		ozz::memory::default_allocator()->Deallocate(buffer_);
	}

	// Resizes the buffer to the new size and return the memory address.
	void* Resize(size_t _size) {
		if (_size > size_) {
			size_ = _size;
			buffer_ = ozz::memory::default_allocator()->Reallocate(buffer_, _size, 16);
		}
		return buffer_;
	}

private:
	void* buffer_;
	size_t size_;
};

class SkinnedMeshSceneNode : public SceneNode {
	//
	//	If Valid is false, this node will be resubmitted for drawing.
	bool Valid = false;
	ScratchBuffer scratch_buffer_;

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

	int DrawPosture_FillUniforms(const ozz::animation::Skeleton& _skeleton,
		ozz::Range<const ozz::math::Float4x4> _matrices,
		float* _uniforms, int _max_instances) {

		// Prepares computation constants.
		const int num_joints = _skeleton.num_joints();
		const ozz::Range<const int16_t>& parents = _skeleton.joint_parents();

		int instances = 0;
		for (int i = 0; i < num_joints && instances < _max_instances; ++i) {
			// Root isn't rendered.
			const int16_t parent_id = parents[i];
			if (parent_id == ozz::animation::Skeleton::kNoParent) {
				continue;
			}

			// Selects joint matrices.
			const ozz::math::Float4x4& parent = _matrices.begin[parent_id];
			const ozz::math::Float4x4& current = _matrices.begin[i];

			// Copy parent joint's raw matrix, to render a bone between the parent
			// and current matrix.
			float* uniform = _uniforms + instances * 16;
			ozz::math::StorePtr(parent.cols[0], uniform + 0);
			ozz::math::StorePtr(parent.cols[1], uniform + 4);
			ozz::math::StorePtr(parent.cols[2], uniform + 8);
			ozz::math::StorePtr(parent.cols[3], uniform + 12);

			// Set bone direction (bone_dir). The shader expects to find it at index
			// [3,7,11] of the matrix.
			// Index 15 is used to store whether a bone should be rendered,
			// otherwise it's a leaf.
			float bone_dir[4];
			ozz::math::StorePtrU(current.cols[3] - parent.cols[3], bone_dir);
			uniform[3] = bone_dir[0];
			uniform[7] = bone_dir[1];
			uniform[11] = bone_dir[2];
			uniform[15] = 1.f;  // Enables bone rendering.

			// Next instance.
			++instances;
			uniform += 16;

			// Only the joint is rendered for leaves, the bone model isn't.
			if (ozz::animation::IsLeaf(_skeleton, i)) {
				// Copy current joint's raw matrix.
				uniform = _uniforms + instances * 16;
				ozz::math::StorePtr(current.cols[0], uniform + 0);
				ozz::math::StorePtr(current.cols[1], uniform + 4);
				ozz::math::StorePtr(current.cols[2], uniform + 8);
				ozz::math::StorePtr(current.cols[3], uniform + 12);

				// Re-use bone_dir to fix the size of the leaf (same as previous bone).
				// The shader expects to find it at index [3,7,11] of the matrix.
				uniform[3] = bone_dir[0];
				uniform[7] = bone_dir[1];
				uniform[11] = bone_dir[2];
				uniform[15] = 0.f;  // Disables bone rendering.
				++instances;
			}
		}

		return instances;
	}

	void updateUniformBuffer(uint32_t currentImage) {
		_Mesh->updateUniformBuffer(currentImage, false);

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
		//
		//	ToDo: Update bone matrices here
		//	Push Constants?

		// Convert matrices to uniforms.
		const int max_skeleton_pieces = ozz::animation::Skeleton::kMaxJoints * 2;
		const size_t max_uniforms_size = max_skeleton_pieces * 2 * 16 * sizeof(float);
		float* uniforms =
			static_cast<float*>(scratch_buffer_.Resize(max_uniforms_size));

		const int instance_count = DrawPosture_FillUniforms(
			skeleton_, ozz::make_range(models_), uniforms, max_skeleton_pieces);
		assert(instance_count <= max_skeleton_pieces);

		const bool _draw_joints = true;
		// Loops through models and instances.
		for (int i = 0; i < (_draw_joints ? 2 : 1); ++i) {
			/*const Model& model = models_[i];

			// Setup model vertex data.
			GL(BindBuffer(GL_ARRAY_BUFFER, model.vbo));

			// Bind shader
			model.shader->Bind(ozz::math::Float4x4::identity(), camera_->view_proj(), sizeof(VertexPNC), 0,
				sizeof(VertexPNC), 12, sizeof(VertexPNC), 24);

			GL(BindBuffer(GL_ARRAY_BUFFER, 0));

			// Draw loop.
			const GLint joint_uniform = model.shader->joint_uniform();
			for (int j = 0; j < _instance_count; ++j) {
				GL(UniformMatrix4fv(joint_uniform, 1, false, _uniforms + 16 * j));
				GL(DrawArrays(model.mode, 0, model.count));
			}

			model.shader->Unbind();*/
		}
	}

	void drawFrame(VkCommandBuffer primaryCommandBuffer) {
		if (!Valid) {
			_Mesh->drawFrame(primaryCommandBuffer);
		}
	}
};

//
//	SceneGraph Create Function
SkinnedMeshSceneNode* SceneGraph::createSkinnedMeshSceneNode(const std::vector<Vertex> vertices, const std::vector<uint16_t> indices) {

	TriangleMesh* Mesh = new TriangleMesh(_Driver, this, vertices, indices);
	SkinnedMeshSceneNode* MeshNode = new SkinnedMeshSceneNode(Mesh);
	SceneNodes.push_back(MeshNode);
	this->invalidate();
	return MeshNode;
}