#pragma once

struct GLTFInfo
{
    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
    std::vector<ozz::math::Float4x4> InverseBindMatrices;
    const char* TexDiffuse;
    TextureObject* DiffuseTex = nullptr;
    TextureObject* NormalTex = nullptr;
};

class ImportGLTF
{
    std::unordered_map<const char*, GLTFInfo*> Model_Cache;
public:

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
                const tinygltf::Mesh& _Mesh = model.meshes[i];
                
                for (size_t j = 0; j < _Mesh.primitives.size(); j++)
                {
                    const tinygltf::Primitive& _Primitive = _Mesh.primitives[j];
                    for (auto& _Attribute : _Primitive.attributes)
                    {
                        if (_Primitive.mode != TINYGLTF_MODE_TRIANGLES)
                        {
                            printf("[GLTF]: ERROR: Primitive Mode != TRIANGLES\n");
                            continue;
                        }
                        //
                        int AccessorID = _Attribute.second;
                        const tinygltf::Accessor Accessor = model.accessors[AccessorID];
                        const tinygltf::BufferView BufferView = model.bufferViews[Accessor.bufferView];
                        int BufferNum = BufferView.buffer;
                        std::vector<unsigned char> Buffer = model.buffers[BufferNum].data;
                        size_t DataStart = BufferView.byteOffset + Accessor.byteOffset;
                        int DataStride = Accessor.ByteStride(BufferView);
                        //
                        if (Accessor.sparse.isSparse)
                        {
                            printf("[GLTF]: ERROR: Accessor Is Sparse!");
                            continue;
                        }
                        //
                        //  Handle our attributes
                        if (_Attribute.first == "POSITION")
                        {

                            #ifdef _DEBUG
                            printf("[GLTF]: POSITION\n");
                            printf("\tAccessor ID %i\n", AccessorID);
                            printf("\tBufferView ID %i\n", Accessor.bufferView);
                            printf("\tBuffer ID %i\n", BufferNum);
                            printf("\tData Start Position %zu\n", DataStart);
                            printf("\tData Stride %i\n", DataStride);
                            printf("\t\tAccessor Type %i\n", Accessor.type);
                            printf("\t\tAccessor Count %zu\n", Accessor.count);
                            printf("\t\tAccessor Component Type %i\n", Accessor.componentType);
                            #endif
                            if (Accessor.type == TINYGLTF_TYPE_VEC3)
                            {
                                if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                                {
                                    size_t DataPos = DataStart;

                                    for (size_t VertexID = 0; VertexID < Accessor.count; VertexID++)
                                    {
                                        if (Infos->Vertices.size() == VertexID)
                                        {
                                            Infos->Vertices.emplace_back();
                                        }
                                        Vertex& Vert = Infos->Vertices[VertexID];
                                        //
                                        // Parse buffer returning Vector3_Float's
                                        // Vec3(float, float, float)
                                        Vert.pos.x = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(const float);
                                        Vert.pos.y = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(const float);
                                        Vert.pos.z = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(const float);// +DataStride;
                                        //printf("\tPosition: Vec3(%f, %f, %f)\n", positions[VertexID], positions[VertexID+1], positions[VertexID+2]);
                                        Infos->Vertices.push_back(Vert);
                                    }
                                }
                            }
                        }
                        else if (_Attribute.first == "TANGENT")
                        {

#ifdef _DEBUG
                            printf("[GLTF]: TANGENT\n");
                            printf("\tAccessor ID %i\n", AccessorID);
                            printf("\tBufferView ID %i\n", Accessor.bufferView);
                            printf("\tBuffer ID %i\n", BufferNum);
                            printf("\tData Start Position %zu\n", DataStart);
                            printf("\tData Stride %i\n", DataStride);
                            printf("\t\tAccessor Type %i\n", Accessor.type);
                            printf("\t\tAccessor Count %zu\n", Accessor.count);
                            printf("\t\tAccessor Component Type %i\n", Accessor.componentType);
#endif
                            
                            if (Accessor.type == TINYGLTF_TYPE_VEC4)
                            {
                                if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                                {
                                    size_t DataPos = DataStart;

                                    for (size_t VertexID = 0; VertexID < Accessor.count; VertexID++)
                                    {
                                        if (Infos->Vertices.size() == VertexID)
                                        {
                                            Infos->Vertices.emplace_back();
                                        }
                                        Vertex& Vert = Infos->Vertices[VertexID];
                                        //
                                        // Parse buffer returning Vector3_Float's
                                        // Vec3(float, float, float)
                                        Vert.Tangents.x = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(const float);
                                        Vert.Tangents.y = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(const float);
                                        Vert.Tangents.z = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(const float);
                                        Vert.Tangents.w = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(const float);// +DataStride;
                                        //printf("\tPosition: Vec3(%f, %f, %f)\n", positions[VertexID], positions[VertexID+1], positions[VertexID+2]);
                                    }
                                }
                            }
                        }
                        else if (_Attribute.first == "JOINTS_0")
                        {

                            #ifdef _DEBUG
                            printf("[GLTF]: JOINTS_0\n");
                            printf("\tAccessor ID %i\n", AccessorID);
                            printf("\tBufferView ID %i\n", Accessor.bufferView);
                            printf("\tBuffer ID %i\n", BufferNum);
                            printf("\tData Start Position %zu\n", DataStart);
                            printf("\tData Stride %i\n", DataStride);
                            printf("\t\tAccessor Type %i\n", Accessor.type);
                            printf("\t\tAccessor Count %zu\n", Accessor.count);
                            printf("\t\tAccessor Component Type %i\n", Accessor.componentType);
                            #endif

                            size_t DataPos = DataStart;

                            for (size_t VertexID = 0; VertexID < Accessor.count; VertexID++)
                            {
                                if (Infos->Vertices.size() == VertexID)
                                {
                                    Infos->Vertices.emplace_back();
                                }
                                Vertex& Vert = Infos->Vertices[VertexID];
                                if (Accessor.type == TINYGLTF_TYPE_VEC4)
                                {
                                    if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                                    {
                                        //
                                        // Parse buffer returning Vector4_Int's
                                        // Vec4(int, int, int, int)
                                        Vert.Bones.x = *reinterpret_cast<const unsigned char*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(unsigned char);
                                        Vert.Bones.y = *reinterpret_cast<const unsigned char*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(unsigned char);
                                        Vert.Bones.z = *reinterpret_cast<const unsigned char*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(unsigned char);
                                        Vert.Bones.w = *reinterpret_cast<const unsigned char*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(unsigned char);// +DataStride;
                                        //printf("\tBones: Vec4(%i, %i, %i, %i)\n", BoneID1, BoneID2, BoneID3, BoneID4);
                                    }
                                }
                            }
                        }
                        else if (_Attribute.first == "WEIGHTS_0")
                        {

                            #ifdef _DEBUG
                            printf("[GLTF]: WEIGHTS_0\n");
                            printf("\tAccessor ID %i\n", AccessorID);
                            printf("\tBufferView ID %i\n", Accessor.bufferView);
                            printf("\tBuffer ID %i\n", BufferNum);
                            printf("\tData Start Position %zu\n", DataStart);
                            printf("\tData Stride %i\n", DataStride);
                            printf("\t\tAccessor Type %i\n", Accessor.type);
                            printf("\t\tAccessor Count %zu\n", Accessor.count);
                            printf("\t\tAccessor Component Type %i\n", Accessor.componentType);
                            #endif

                            size_t DataPos = DataStart;

                            for (size_t VertexID = 0; VertexID < Accessor.count; VertexID++)
                            {
                                if (Infos->Vertices.size() == VertexID)
                                {
                                    Infos->Vertices.emplace_back();
                                }
                                Vertex& Vert = Infos->Vertices[VertexID];
                                if (Accessor.type == TINYGLTF_TYPE_VEC4)
                                {
                                    if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                                    {
                                        //
                                        // Parse buffer returning Vector4_Int's
                                        // Vec4(int, int, int, int)
                                        Vert.Weights.x = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);
                                        Vert.Weights.y = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);
                                        Vert.Weights.z = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);
                                        Vert.Weights.w = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);// +DataStride;
                                        //printf("\tWeights: Vec4(%f, %f, %f, %f)\n", BoneWeight1, BoneWeight2, BoneWeight3, BoneWeight4);
                                    }
                                }
                            }
                        }
                        else if (_Attribute.first == "TEXCOORD_0")
                        {

                            #ifdef _DEBUG
                            printf("[GLTF]: TEXCOORD_0\n");
                            printf("\tAccessor ID %i\n", AccessorID);
                            printf("\tBufferView ID %i\n", Accessor.bufferView);
                            printf("\tBuffer ID %i\n", BufferNum);
                            printf("\tData Start Position %zu\n", DataStart);
                            printf("\tData Stride %i\n", DataStride);
                            printf("\t\tAccessor Type %i\n", Accessor.type);
                            printf("\t\tAccessor Count %zu\n", Accessor.count);
                            printf("\t\tAccessor Component Type %i\n", Accessor.componentType);
                            #endif

                            size_t DataPos = DataStart;

                            for (size_t VertexID = 0; VertexID < Accessor.count; VertexID++)
                            {
                                if (Infos->Vertices.size() == VertexID)
                                {
                                    Infos->Vertices.emplace_back();
                                }
                                Vertex& Vert = Infos->Vertices[VertexID];
                                if (Accessor.type == TINYGLTF_TYPE_VEC2)
                                {
                                    if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                                    {
                                        //
                                        // Parse buffer returning Vector4_Int's
                                        // Vec4(int, int, int, int)
                                        Vert.texCoord.x = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);
                                        Vert.texCoord.y = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);// +DataStride;
                                        //printf("\tWeights: Vec4(%f, %f, %f, %f)\n", BoneWeight1, BoneWeight2, BoneWeight3, BoneWeight4);
                                    }
                                }
                            }
                        }
                        else if (_Attribute.first == "NORMAL")
                        {

                            #ifdef _DEBUG
                            printf("[GLTF]: NORMAL\n");
                            printf("\tAccessor ID %i\n", AccessorID);
                            printf("\tBufferView ID %i\n", Accessor.bufferView);
                            printf("\tBuffer ID %i\n", BufferNum);
                            printf("\tData Start Position %zu\n", DataStart);
                            printf("\tData Stride %i\n", DataStride);
                            printf("\t\tAccessor Type %i\n", Accessor.type);
                            printf("\t\tAccessor Count %zu\n", Accessor.count);
                            printf("\t\tAccessor Component Type %i\n", Accessor.componentType);
                            #endif

                            size_t DataPos = DataStart;

                            for (size_t VertexID = 0; VertexID < Accessor.count; VertexID++)
                            {
                                if (Infos->Vertices.size() == VertexID)
                                {
                                    Infos->Vertices.emplace_back();
                                }
                                Vertex& Vert = Infos->Vertices[VertexID];
                                if (Accessor.type == TINYGLTF_TYPE_VEC3)
                                {
                                    if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                                    {
                                        //
                                        // Parse buffer returning Vector4_Int's
                                        // Vec4(int, int, int, int)
                                        Vert.normal.x = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);
                                        Vert.normal.y = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);
                                        Vert.normal.z = *reinterpret_cast<const float*>(&Buffer.data()[DataPos]);
                                        DataPos += sizeof(float);// +DataStride;
                                        //printf("\tWeights: Vec4(%f, %f, %f, %f)\n", BoneWeight1, BoneWeight2, BoneWeight3, BoneWeight4);
                                    }
                                }
                            }
                        }
                        else {
                            printf("[GLTF]: Unhandled Attribute: %s\n", _Attribute.first.c_str());
                        }
                        //
                        for (size_t VertexID = 0; VertexID < Accessor.count; VertexID++)
                        {
                            Vertex& Vert = Infos->Vertices[VertexID];
                            //
                            Vert.color.r = 155;
                            Vert.color.g = 155;
                            Vert.color.b = 155;
                            Vert.color.a = 255;
                        }
                    }
                    int AccessorID = _Primitive.indices;
                    const tinygltf::Accessor Accessor = model.accessors[AccessorID];
                    const tinygltf::BufferView BufferView = model.bufferViews[Accessor.bufferView];
                    int BufferNum = BufferView.buffer;
                    std::vector<unsigned char> Buffer = model.buffers[BufferNum].data;
                    size_t DataStart = BufferView.byteOffset + Accessor.byteOffset;
                    int DataStride = Accessor.ByteStride(BufferView);

                    #ifdef _DEBUG
                    printf("[GLTF]: INDEX\n");
                    printf("\tAccessor ID %i\n", AccessorID);
                    printf("\tBufferView ID %i\n", Accessor.bufferView);
                    printf("\tBuffer ID %i\n", BufferNum);
                    printf("\tData Start Position %zu\n", DataStart);
                    printf("\tData Stride %i\n", DataStride);
                    printf("\t\tAccessor Type %i\n", Accessor.type);
                    printf("\t\tAccessor Count %zu\n", Accessor.count);
                    printf("\t\tAccessor Component Type %i\n", Accessor.componentType);
                    #endif

                    size_t DataPos = DataStart;

                    if (Accessor.type == TINYGLTF_TYPE_SCALAR)
                    {
                        if (Accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            //
                            //  Finally iterate for our data!
                            for (size_t IndexID = 0; IndexID < Accessor.count; IndexID++)
                            {
                                //unsigned short Index = IndexBuffer[IndexDataStart + (IndexID * sizeof(unsigned short))];
                                //printf("\tPosition: Vec3(%f, %f, %f)\n", VertexPositionX, VertexPositionY, VertexPositionZ);
                                //Infos->Indices.push_back(Index);
                                Infos->Indices.push_back(*reinterpret_cast<const unsigned short*>(&Buffer.data()[DataPos]));
                                DataPos += sizeof(unsigned short);// +DataStride;
                            }
                        }
                    }
                    //
                    //  Load Textures
                    //
                    //  TODO: Implement default/missing textures

                    if (model.materials.size() <= 0)
                    {
                        Infos->DiffuseTex = Pipe->createTextureImage("NA");
                        Infos->NormalTex = Pipe->createTextureImage("NA");
                    }
                    else
                    {
                        const tinygltf::Material& _Material = model.materials[_Primitive.material];
                        int ColorTex = _Material.pbrMetallicRoughness.baseColorTexture.index;
                        int NormalTex = _Material.normalTexture.index;

                        printf("MODEL.IMAGES COLOR NAME %s\n", model.images[ColorTex].name.c_str());
                        Infos->DiffuseTex = Pipe->createTextureImage2(model.images[ColorTex]);
                        if (NormalTex >= 0)
                        {
                            printf("MODEL.IMAGES NORMAL NAME %s\n", model.images[NormalTex].name.c_str());
                            Infos->NormalTex = Pipe->createTextureImage2(model.images[NormalTex]);
                        }
                    }
                }
            }
            //
            // Load SKIN
            //  TODO: This doesn't take into account that model with multiple meshes has a unique SKIN per MESH and should be handled accordingly.
            //  TODO: This assumes only a single MESH and SKIN.
            for (size_t i = 0; i < model.skins.size(); i++)
            {
                tinygltf::Skin _Skin = model.skins[i];

                if (_Skin.inverseBindMatrices > -1)
                {
                    const tinygltf::Accessor& acc = model.accessors[_Skin.inverseBindMatrices];
                    const tinygltf::BufferView& bufview = model.bufferViews[acc.bufferView];
                    const tinygltf::Buffer& buf = model.buffers[bufview.buffer];
                    Infos->InverseBindMatrices.resize(acc.count);

                    memcpy(Infos->InverseBindMatrices.data(), &buf.data[acc.byteOffset + bufview.byteOffset], acc.count * sizeof(ozz::math::Float4x4));
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