#pragma once

//
//	Vertex
struct Vertex {
	glm::vec4 pos{ 0, 0, 0, 0 };
	glm::vec4 color{ 0, 0, 0, 0 };
	glm::vec2 texCoord{ 0, 0 };
	glm::vec3 normal{ 0, 0, 0 };
	glm::vec4 Tangents{ 0, 0, 0, 0 };

	glm::vec4 Bones{ 0, 0, 0, 0 };
	glm::vec4 Weights{ 0, 0, 0, 0 };

	//
	//	SHADOW
	static std::vector<VkVertexInputBindingDescription> getBindingDescription_Shadow() {
		std::vector<VkVertexInputBindingDescription> bindingDescription = {};
		bindingDescription.emplace_back();
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions_Shadow() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

		attributeDescriptions.emplace_back();
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Position
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		return attributeDescriptions;
	}

	//
	//	SIMPLE
	static std::vector<VkVertexInputBindingDescription> getBindingDescription_Simple() {
		std::vector<VkVertexInputBindingDescription> bindingDescription = {};
		bindingDescription.emplace_back();
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions_Simple() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

		attributeDescriptions.emplace_back();
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Position
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions.emplace_back();
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;			//	Color
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions.emplace_back();
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;			//	UV
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	//
	//	INSTANCED
	static std::vector<VkVertexInputBindingDescription> getBindingDescription() {
		std::vector<VkVertexInputBindingDescription> bindingDescription = {};
		bindingDescription.emplace_back();
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		bindingDescription.emplace_back();
		bindingDescription[1].binding = 1;
		bindingDescription[1].stride = sizeof(glm::mat4);
		bindingDescription[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions_Static() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

		attributeDescriptions.emplace_back();
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Position
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions.emplace_back();
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;			//	Color
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions.emplace_back();
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;			//	UV
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions.emplace_back();
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Normal
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		attributeDescriptions.emplace_back();
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Tangent
		attributeDescriptions[4].offset = offsetof(Vertex, Tangents);

		attributeDescriptions.emplace_back();
		attributeDescriptions[5].binding = 1;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix X
		attributeDescriptions[5].offset = sizeof(glm::vec4) * 0;

		attributeDescriptions.emplace_back();
		attributeDescriptions[6].binding = 1;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix Y
		attributeDescriptions[6].offset = sizeof(glm::vec4) * 1;

		attributeDescriptions.emplace_back();
		attributeDescriptions[7].binding = 1;
		attributeDescriptions[7].location = 7;
		attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix Z
		attributeDescriptions[7].offset = sizeof(glm::vec4) * 2;

		attributeDescriptions.emplace_back();
		attributeDescriptions[8].binding = 1;
		attributeDescriptions[8].location = 8;
		attributeDescriptions[8].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix W
		attributeDescriptions[8].offset = sizeof(glm::vec4) * 3;

		return attributeDescriptions;
	}
	//VK_FORMAT_A2B10G10R10_UNORM_PACK32
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions_Animated() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

		attributeDescriptions.emplace_back();
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Position
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions.emplace_back();
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;			//	Color
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions.emplace_back();
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;			//	UV
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions.emplace_back();
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Normal
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		attributeDescriptions.emplace_back();
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Tangent
		attributeDescriptions[4].offset = offsetof(Vertex, Tangents);

		attributeDescriptions.emplace_back();
		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Joint
		attributeDescriptions[5].offset = offsetof(Vertex, Bones);

		attributeDescriptions.emplace_back();
		attributeDescriptions[6].binding = 0;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Weight
		attributeDescriptions[6].offset = offsetof(Vertex, Weights);

		attributeDescriptions.emplace_back();
		attributeDescriptions[7].binding = 1;
		attributeDescriptions[7].location = 7;
		attributeDescriptions[7].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix X
		attributeDescriptions[7].offset = sizeof(glm::vec4) * 0;

		attributeDescriptions.emplace_back();
		attributeDescriptions[8].binding = 1;
		attributeDescriptions[8].location = 8;
		attributeDescriptions[8].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix Y
		attributeDescriptions[8].offset = sizeof(glm::vec4) * 1;

		attributeDescriptions.emplace_back();
		attributeDescriptions[9].binding = 1;
		attributeDescriptions[9].location = 9;
		attributeDescriptions[9].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix Z
		attributeDescriptions[9].offset = sizeof(glm::vec4) * 2;

		attributeDescriptions.emplace_back();
		attributeDescriptions[10].binding = 1;
		attributeDescriptions[10].location = 10;
		attributeDescriptions[10].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Instance Model Matrix W
		attributeDescriptions[10].offset = sizeof(glm::vec4) * 3;

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}