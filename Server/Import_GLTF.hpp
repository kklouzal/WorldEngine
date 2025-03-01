#pragma once

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include "TinyGLTF.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_INLINE
#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_SSE2
#include <glm/glm.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct Vertex {
    glm::vec3 pos{ 0, 0, 0 };
    glm::vec4 color{ 0, 0, 0, 0 };
    glm::vec2 texCoord{ 0, 0 };
    glm::vec3 normal{ 0, 0, 0 };

    glm::vec4 Bones{ 0, 0, 0, 0 };
    glm::vec4 Weights{ 0, 0, 0, 0 };
    glm::vec4 Tangents{ 0, 0, 0, 0 };
};

struct GLTFInfo
{
    const float* positionBuffer = nullptr;
    size_t vertexCount = 0;

    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    std::vector<glm::mat4> InverseBindMatrices;
    const char* TexDiffuse;
    //std::map<std::string, uint16_t> JointMap;
};

class ImportGLTF
{
public:

    //struct SKNode
    //{
    //    SKNode*                 parent;
    //    uint32_t                index;
    //    std::vector<SKNode*>    children;
    //};

    //std::vector<SKNode*>        nodes;
    //
    //void LoadNode(const tinygltf::Node inputNode, tinygltf::Model inputModel, SKNode* parent, uint32_t nodeIndex)
    //{
    //    SKNode* _Node = new SKNode{};
    //    _Node->parent = parent;
    //    _Node->index = nodeIndex;

    //    // Load node's children
    //    if (inputNode.children.size() > 0)
    //    {
    //        for (size_t i = 0; i < inputNode.children.size(); i++)
    //        {
    //            LoadNode(inputModel.nodes[inputNode.children[i]], inputModel, _Node, inputNode.children[i]);
    //        }
    //    }

    //    if (parent)
    //    {
    //        parent->children.push_back(_Node);
    //        printf("LOAD NODE %i\n", nodeIndex);
    //    }
    //    else
    //    {
    //        nodes.push_back(_Node);
    //    }
    //}

    GLTFInfo* loadModel(const char* filename)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
        if (!warn.empty()) {
            wxLogMessage("[GLTF] WARN: %s\n", warn.c_str());
        }

        if (!err.empty()) {
            wxLogMessage("[GLTF] ERR: %s\n", err.c_str());
        }

        if (!res)
        {
            wxLogMessage("[GLTF] Failed to load glTF: %s", filename);
        }

        //
        //  Construct an array of BoneIDs per-vertex and BoneWeights per-vertex
        GLTFInfo* Infos = new GLTFInfo;

        for (size_t i = 0; i < model.meshes.size(); i++)
        {
            const tinygltf::Mesh _Mesh = model.meshes[i];
                
            for (size_t j = 0; j < _Mesh.primitives.size(); j++)
            {
                const tinygltf::Primitive _Primitive = _Mesh.primitives[j];

                for (auto& _Attribute : _Primitive.attributes)
                {
                    bool hasSkin = false;
                    //
                    //  Verticies
                    {
                        const float* positionBuffer = nullptr;
                        const float* tangentBuffer = nullptr;
                        const float* normalBuffer = nullptr;
                        const float* texcoordBuffer = nullptr;
                        const uint16_t* jointBuffer = nullptr;
                        const float* weightBuffer = nullptr;
                        size_t vertexCount = 0;

                        if (_Primitive.mode != TINYGLTF_MODE_TRIANGLES)
                        {
                            wxLogMessage("[GLTF] ERROR: Primitive Mode != TRIANGLES\n");
                            continue;
                        }

                        //
                        //  Handle Positions
                        if (_Primitive.attributes.find("POSITION") != _Primitive.attributes.end())
                        {
                            const tinygltf::Accessor& accessor = model.accessors[_Primitive.attributes.find("POSITION")->second];
                            const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                            positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                            vertexCount = accessor.count;
                        }

                        //
                        //  Hadle Normals
                        if (_Primitive.attributes.find("NORMAL") != _Primitive.attributes.end())
                        {
                            const tinygltf::Accessor& accessor = model.accessors[_Primitive.attributes.find("NORMAL")->second];
                            const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                            normalBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        }

                        //
                        //  Hadle Joints
                        if (_Primitive.attributes.find("JOINTS_0") != _Primitive.attributes.end())
                        {
                            const tinygltf::Accessor& accessor = model.accessors[_Primitive.attributes.find("JOINTS_0")->second];
                            const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                            jointBuffer = reinterpret_cast<const uint16_t*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        }

                        //
                        //  Hadle Weights
                        if (_Primitive.attributes.find("WEIGHTS_0") != _Primitive.attributes.end())
                        {
                            const tinygltf::Accessor& accessor = model.accessors[_Primitive.attributes.find("WEIGHTS_0")->second];
                            const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                            weightBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                        }

                        hasSkin = (jointBuffer && weightBuffer);

                        //
                        //  Store our loaded data into the individual vertices
                        for (size_t VertexID = 0; VertexID < vertexCount; VertexID++)
                        {
                            if (Infos->Vertices.size() == VertexID)
                            {
                                Infos->Vertices.emplace_back();
                            }
                            Vertex& Vert = Infos->Vertices[VertexID];

                            Vert.pos        = glm::vec4(glm::make_vec3(&positionBuffer[VertexID * 3]), 1.0f);
                            Vert.normal     = glm::normalize(glm::vec3(normalBuffer ? glm::make_vec3(&normalBuffer[VertexID * 3]) : glm::vec3(0.0f)));
                            Vert.Bones      = hasSkin ? glm::vec4(glm::make_vec4(&jointBuffer[VertexID * 4])) : glm::vec4(0.0);
                            Vert.Weights    = hasSkin ? glm::make_vec4(&weightBuffer[VertexID * 4]) : glm::vec4(0.0);
                        }
                    }
                }

                //
                //  Handle Indicies
                const tinygltf::Accessor& accessor = model.accessors[_Primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                //
                // glTF supports different component types of indices
                switch (accessor.componentType)
                {
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                    const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        Infos->Indices.push_back(buf[index] + Infos->vertexCount*3);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                    const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        Infos->Indices.push_back(buf[index] + Infos->vertexCount*3);
                    }
                    break;
                }
                case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                    const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t index = 0; index < accessor.count; index++)
                    {
                        Infos->Indices.push_back(buf[index] + Infos->vertexCount*3);
                    }
                    break;
                }
                default:
                    std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                    //return;
                }
            }
        }
            
        //
        // Load SKIN
        //  TODO: This doesn't take into account that model with multiple meshes has a unique SKIN per MESH and should be handled accordingly.
        //  TODO: This assumes only a single MESH and SKIN.
        //printf("Skins Count %i\n", model.skins.size());

        for (size_t i = 0; i < model.skins.size(); i++)
        {
            tinygltf::Skin _Skin = model.skins[i];

            if (_Skin.inverseBindMatrices > -1)
            {
                for (size_t i = 0; i < model.nodes.size(); i++)
                {
                    const tinygltf::Node& node = model.nodes[i];
                    //Infos->JointMap[node.name] = i;
                    //printf("%zi -> NODE NAME: %s\n", i, node.name.c_str());
                }

                const tinygltf::Accessor& acc = model.accessors[_Skin.inverseBindMatrices];
                const tinygltf::BufferView& bufview = model.bufferViews[acc.bufferView];
                const tinygltf::Buffer& buf = model.buffers[bufview.buffer];
                Infos->InverseBindMatrices.resize(acc.count);
                for (auto J : _Skin.joints)
                {
                    //printf("GLTF JOINTS %i\n", J);
                }
                memcpy(Infos->InverseBindMatrices.data(), &buf.data[acc.byteOffset + bufview.byteOffset], acc.count * sizeof(glm::mat4));

                /*for (size_t i = 0; i < model.scenes[0].nodes.size(); i++)
                {
                    const tinygltf::Node node = model.nodes[i];
                    LoadNode(node, model, nullptr, model.scenes[0].nodes[i]);
                }
                std::vector<size_t> DepthOrder;
                for (size_t i = 0; i < nodes.size(); i++)
                {
                    DepthOrder.push_back(nodes[i]->index);
                }
                for (size_t i = 0; i < DepthOrder.size(); i++)
                {
                    printf("DEPTH: %i\n", DepthOrder[i]);
                }*/
                break;
            }
        }
        //
        //  Return the info
        return Infos;
    }
};

#include "ConvexDecomposition.hpp"