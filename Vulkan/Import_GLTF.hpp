#pragma once

struct GLTFInfo
{
    size_t vertexCount = 0;

    std::vector<Vertex> Vertices = {};
    std::vector<uint32_t> Indices = {};
    std::vector<glm::mat4> InverseBindMatrices = {};
    const char* TexDiffuse = "";
    TextureObject* DiffuseTex = nullptr;
    TextureObject* NormalTex = nullptr;
    //  Joint Name --> GLTF Index
    std::map<std::string, uint16_t> JointMap_ = {};
    std::map<uint16_t, uint16_t> JointMap_OZZ = {};
    //  uint16_t (gltf id) --> uint16_t (ozz id)
};

class ImportGLTF
{
    std::unordered_map<const char*, GLTFInfo*> Model_Cache;
public:

    void getChildren(tinygltf::Model model, int indexNode, std::map<uint16_t, uint16_t>& Joints, int& accumulator)
    {
        Joints.emplace(indexNode-3, accumulator++);
        //printf("GLTF->OZZ MAP %u -> %u\n", indexNode-3, model.nodes[indexNode].name.c_str());
        for (int child : model.nodes[indexNode].children)
        {
            getChildren(model, child, Joints, accumulator);
        }
    };

    GLTFInfo* loadModel(const char* filename, PipelineObject* Pipe)
    {
        
        if (Model_Cache.contains(filename))
        {
            return Model_Cache[filename];
        }
        else
        {
            tinygltf::Model model;
            tinygltf::TinyGLTF loader;
            std::string err;
            std::string warn;

            bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
            if (!warn.empty()) {
                printf("WARN: %s\n", warn.c_str());
            }

            if (!err.empty()) {
                printf("ERR: %s\n", err.c_str());
            }

            if (!res)
                printf("Failed to load glTF: %s", filename);
            else
                printf("Loaded glTF: %s\n", filename);

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
                                printf("[GLTF]: ERROR: Primitive Mode != TRIANGLES\n");
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
                            //  Handle Tangents
                            if (_Primitive.attributes.find("TANGENT") != _Primitive.attributes.end())
                            {
                                const tinygltf::Accessor& accessor = model.accessors[_Primitive.attributes.find("TANGENT")->second];
                                const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                                tangentBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
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
                            //  Hadle Texture Coordinates
                            if (_Primitive.attributes.find("TEXCOORD_0") != _Primitive.attributes.end())
                            {
                                const tinygltf::Accessor& accessor = model.accessors[_Primitive.attributes.find("TEXCOORD_0")->second];
                                const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
                                texcoordBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
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
                                Vert.Tangents   = tangentBuffer ? glm::vec4(glm::make_vec3(&tangentBuffer[VertexID * 3]), 1.0f) : glm::vec4(0.0);
                                Vert.normal     = glm::normalize(glm::vec3(normalBuffer ? glm::make_vec3(&normalBuffer[VertexID * 3]) : glm::vec3(0.0f)));
                                Vert.texCoord   = texcoordBuffer ? glm::make_vec2(&texcoordBuffer[VertexID * 2]) : glm::vec3(0.0f);
                                Vert.Bones      = hasSkin ? glm::vec4(glm::make_vec4(&jointBuffer[VertexID * 4])) : glm::vec4(0.0);
                                Vert.Weights    = hasSkin ? glm::make_vec4(&weightBuffer[VertexID * 4]) : glm::vec4(0.0);
                                //
                                //  Color
                                Vert.color.r = 155;
                                Vert.color.g = 155;
                                Vert.color.b = 155;
                                Vert.color.a = 255;
                                Vert.color.r = Vert.Bones.r * 10;
                                Vert.color.g = Vert.Bones.g * 10;
                                Vert.color.b = Vert.Bones.b * 10;
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

                    //
                    //  Load Textures
                    //
                    //  TODO: Implement default/missing textures

                    if (model.materials.size() <= 0)
                    {
                        Infos->DiffuseTex = Pipe->createTextureImage("NA");
                        Infos->NormalTex = Pipe->createTextureImage("media/missingnormal.png");
                    }
                    else
                    {
                        const tinygltf::Material& _Material = model.materials[_Primitive.material];
                        int ColorTex = _Material.pbrMetallicRoughness.baseColorTexture.index;
                        int NormalTex = _Material.normalTexture.index;

                        if (ColorTex >= 0)
                        {
                            printf("MODEL.IMAGES COLOR NAME %s\n", model.images[ColorTex].name.c_str());
                            Infos->DiffuseTex = Pipe->createTextureImage2(model.images[ColorTex]);
                        }
                        if (NormalTex >= 0)
                        {
                            printf("MODEL.IMAGES NORMAL NAME %s\n", model.images[NormalTex].name.c_str());
                            Infos->NormalTex = Pipe->createTextureImage2(model.images[NormalTex]);
                        }
                        else {
                            printf("MODEL.IMAGES !!HAS NO NORMAL!!\n");
                            Infos->NormalTex = Pipe->createTextureImage("media/missingnormal.png");
                        }
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

                int accumulator = 0;
                getChildren(model, _Skin.skeleton, Infos->JointMap_OZZ, accumulator);
                
                for (auto& JointID : _Skin.joints)
                {
                    std::string NodeName = model.nodes[JointID].name;
                    Infos->JointMap_[NodeName] = JointID - _Skin.skeleton;
                }

                if (_Skin.inverseBindMatrices > -1)
                {
                    const tinygltf::Accessor& acc = model.accessors[_Skin.inverseBindMatrices];
                    const tinygltf::BufferView& bufview = model.bufferViews[acc.bufferView];
                    const tinygltf::Buffer& buf = model.buffers[bufview.buffer];
                    Infos->InverseBindMatrices.resize(acc.count);
                    memcpy(Infos->InverseBindMatrices.data(), &buf.data[acc.byteOffset + bufview.byteOffset], acc.count * sizeof(glm::mat4));

                    break;
                }
            }
            //
            //  Add this info into the model cache
            Model_Cache[filename] = Infos;
            //
            //  Return the info
            return Infos;
        }
    }
};