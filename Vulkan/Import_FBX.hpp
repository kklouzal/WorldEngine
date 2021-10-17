#pragma once

#define FBXSDK_SHARED
#include <fbxsdk.h>

struct FBXObject {
	std::vector<Vertex> Vertices = {};
	std::vector<uint32_t> Indices = {};

	std::vector<FbxAMatrix> bindPoses = {};

	const char* Texture_Diffuse = "";

	FBXObject() {}

	~FBXObject() {
		printf("Destroy FBX Object\n");
		Vertices.clear();
		Indices.clear();
		bindPoses.clear();
	}
};

class ImportFBX {
	std::unordered_map<const char*, FBXObject*> _FbxObjects;
public:
	FbxManager* _FbxManager = nullptr;

	ImportFBX() {
		_FbxManager = FbxManager::Create();
		FbxIOSettings* _FbxSettings = FbxIOSettings::Create(_FbxManager, IOSROOT);
		_FbxManager->SetIOSettings(_FbxSettings);
	}

	~ImportFBX() {
		_FbxManager->Destroy();
	}

	void EmptyCache() {
		for (auto FBX : _FbxObjects) {
			delete FBX.second;
		}
		_FbxObjects.clear();
	}

	//	For now, only return mesh nodes.
	//	ToDo: Implement lights and possibly others
	void SearchNodes(const fbxsdk::FbxNode* Node, std::vector<const fbxsdk::FbxNode*>& Nodes) {
		for (unsigned int i = 0; i < Node->GetChildCount(); i++) {
			const fbxsdk::FbxNode* Child = Node->GetChild(i);
			const fbxsdk::FbxNodeAttribute* Attribute = Child->GetNodeAttribute();
			if (Attribute == NULL) {
				SearchNodes(Child, Nodes);
			}
			else {
				const FbxNodeAttribute::EType AttributeType = Attribute->GetAttributeType();
				if (AttributeType != FbxNodeAttribute::eMesh) {
					SearchNodes(Child, Nodes);
				}
				else {
					Nodes.push_back(Child);
					SearchNodes(Child, Nodes);
				}
			}
		}
	}

	struct VertexBoneInfo {
		std::vector<unsigned int> Bones = {};
		std::vector<float> Weights = {};
	};

	//	Only the first UV set is used for each mesh
	FBXObject* Import(const char* File) {
		if (_FbxObjects.count(File) == 1) {
			return _FbxObjects[File];
		}
		else {
			printf("[FBX]: Load %s\n", File);
			FbxImporter* Importer = FbxImporter::Create(_FbxManager, "");
			if (!Importer->Initialize(File, -1, _FbxManager->GetIOSettings())) {
				printf("FBX Import Initialize Failed: %s", Importer->GetStatus().GetErrorString());
				return nullptr;
			}

			FbxScene* Scene = FbxScene::Create(_FbxManager, "NewScene");
			Importer->Import(Scene);
			Importer->Destroy();
			fbxsdk::FbxGeometryConverter(_FbxManager).Triangulate(Scene, true);

			const FbxNode* RootNode = Scene->GetRootNode();
			if (RootNode) {
				std::vector<const fbxsdk::FbxNode*> Nodes;
				SearchNodes(RootNode, Nodes);
				printf("\t%i Nodes - %zi Usable\n", RootNode->GetChildCount(true), Nodes.size());

				FBXObject* NewFBX = _FbxObjects.emplace(File, new FBXObject).first->second;

				uint32_t IndexCount = 0;
				for (auto Node : Nodes) {
					//
					//	Material Information
					const int Materials = Node->GetSrcObjectCount<FbxSurfaceMaterial>();
					printf("\tMaterials %i\n", Materials);
					const FbxSurfaceMaterial* Material = FbxCast<const FbxSurfaceMaterial>(Node->GetSrcObject<FbxSurfaceMaterial>(0));
					if (Material) {
						//
						//	Diffuse Texture
						const FbxProperty pDiffuse = Material->FindProperty(FbxSurfaceMaterial::sDiffuse);
						if (pDiffuse.IsValid()) {
							const unsigned int layered_texture_count = pDiffuse.GetSrcObjectCount<FbxLayeredTexture>();
							const unsigned int textureCount = pDiffuse.GetSrcObjectCount<FbxFileTexture>();
							printf("\tTextures: %i %i\n", layered_texture_count, textureCount);
							const FbxFileTexture* texture = FbxCast<const FbxFileTexture>(pDiffuse.GetSrcObject<FbxFileTexture>(0));
							if (texture) {
								NewFBX->Texture_Diffuse = texture->GetRelativeFileName();
								printf("\tRelative File %s\n", NewFBX->Texture_Diffuse);
							}
							else { printf("\tTexture Invalid\n"); }
						}
						else { printf("\tNo Diffuse\n"); }
					}
					else { printf("\tMaterial 1 invalid\n"); }
					//
					//
					const FbxMesh* Mesh = FbxCast<const FbxMesh>(Node->GetNodeAttribute());
					//
					//	Skinning Information
					std::unordered_map<unsigned int, VertexBoneInfo> BoneData = {};
					const unsigned int Deformers = Mesh->GetDeformerCount();
					printf("\tDeformers %i\n", Deformers);
					const FbxSkin* pSkin = FbxCast<const FbxSkin>(Mesh->GetDeformer(0, FbxDeformer::eSkin));
					if (pSkin) {
						const unsigned int ncBones = pSkin->GetClusterCount();
						printf("\tBones: %i\n", ncBones);
						for (unsigned int boneIndex = 0; boneIndex < ncBones; ++boneIndex)
						{
							const FbxCluster* cluster = pSkin->GetCluster(boneIndex);
							const FbxNode* pBone = cluster->GetLink();

							FbxAMatrix bindPoseMatrix, transformMatrix;
							cluster->GetTransformMatrix(transformMatrix);
							cluster->GetTransformLinkMatrix(bindPoseMatrix);
							NewFBX->bindPoses.push_back(bindPoseMatrix);

							const int* const pVertexIndices = cluster->GetControlPointIndices();
							const double* const pVertexWeights = cluster->GetControlPointWeights();

							// Iterate through all the vertices, which are affected by the bone
							for (unsigned int iBoneVertexIndex = 0; iBoneVertexIndex < cluster->GetControlPointIndicesCount(); iBoneVertexIndex++)
							{
								// vertex
								const unsigned int niVertex = pVertexIndices[iBoneVertexIndex];
								// weight
								const float fWeight = (const float)pVertexWeights[iBoneVertexIndex];

								BoneData[niVertex].Bones.push_back(boneIndex);
								BoneData[niVertex].Weights.push_back(fWeight);
							}
						}
					}
					else {
						printf("\tModel Has No Skin\n");
					}
					//
					//	UV Mapping *only uses first uv set*
					FbxStringList lUVSetNameList;
					Mesh->GetUVSetNames(lUVSetNameList);
					const unsigned int UVSets = lUVSetNameList.GetCount();
					printf("\tUV Sets: %i\n", UVSets);
					const char* lUVSetName = lUVSetNameList.GetStringAt(0);
					const FbxGeometryElementUV* lUVElement = Mesh->GetElementUV(lUVSetName);
					bool lUseIndex;
					int lIndexCount;
					if (lUVElement) {
						//if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
						//printf("\tPolygon Vertex Mapping\n");
						lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
						lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;
					}
					//
					//	Normal Mapping
					const FbxGeometryElementNormal* lNormalElement = Mesh->GetElementNormal();

					const FbxVector4* const Vertices = Mesh->GetControlPoints();
					unsigned int lPolyIndexCounter = 0;
					for (unsigned int j = 0; j < Mesh->GetPolygonCount(); j++) {
						const unsigned int NumVerts = Mesh->GetPolygonSize(j);
						if (NumVerts != 3) { continue; }

						for (unsigned int k = 0; k < NumVerts; k++) {
							Vertex* NewVertex = &NewFBX->Vertices.emplace_back();
							//
							//	Bone Data
							if (BoneData.count(j) == 1) {
								const size_t BoneSize = BoneData[j].Bones.size();
								if (BoneSize > 0) {
									NewVertex->Bones.x = BoneData[j].Bones[0];
								}
								if (BoneSize > 1) {
									NewVertex->Bones.y = BoneData[j].Bones[1];
								}
								if (BoneSize > 2) {
									NewVertex->Bones.z = BoneData[j].Bones[2];
								}
								if (BoneSize > 3) {
									NewVertex->Bones.w = BoneData[j].Bones[3];
								}

								const size_t WeightSize = BoneData[j].Weights.size();
								if (WeightSize > 0) {
									NewVertex->Weights.x = BoneData[j].Weights[0];
								}
								if (WeightSize > 1) {
									NewVertex->Weights.y = BoneData[j].Weights[1];
								}
								if (WeightSize > 2) {
									NewVertex->Weights.z = BoneData[j].Weights[2];
								}
								if (WeightSize > 3) {
									NewVertex->Weights.w = BoneData[j].Weights[3];
								}
							}
							//
							//	Normap Mapping Data
							int lNormalIndex = 0;
							// reference mode is direct, the normal index is same as lIndexByPolygonVertex.
							if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eDirect) {
								lNormalIndex = lPolyIndexCounter;
							}
							// reference mode is index-to-direct, get normals by the index-to-direct
							else if (lNormalElement->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
								lNormalIndex = lNormalElement->GetIndexArray().GetAt(lPolyIndexCounter);
							}

							// Got normals of each polygon-vertex.
							FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);

							NewVertex->normal.x = lNormal[0];
							NewVertex->normal.y = lNormal[1];
							NewVertex->normal.z = lNormal[2];
							//
							//	UV Mapping Data
							if (lUVElement) {
								if (lPolyIndexCounter < lIndexCount)
								{
									//the UV index depends on the reference mode
									int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

									const FbxVector2 lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

									NewVertex->texCoord.x = lUVValue.mData[0];
									NewVertex->texCoord.y = -lUVValue.mData[1];

									lPolyIndexCounter++;
								}
							}
							//
							//	Vertex & Index data
							//	ToDo: Remove duplicate vertices using indices
							const unsigned int VertID = Mesh->GetPolygonVertex(j, k);
							NewVertex->pos.x = (float)Vertices[VertID].mData[0];
							NewVertex->pos.y = (float)Vertices[VertID].mData[1];
							NewVertex->pos.z = (float)Vertices[VertID].mData[2];

							NewFBX->Indices.emplace_back(IndexCount++);
						}
					}
				}
				printf("\tOut Vertex Count: %zi\n", NewFBX->Vertices.size());
				return NewFBX;
			}
		}
	}
};

/*const char* nodeName = Node->GetName();
FbxDouble3 translation = Node->LclTranslation.Get();
FbxDouble3 rotation = Node->LclRotation.Get();
FbxDouble3 scaling = Node->LclScaling.Get();*/

// Print the contents of the node.
/*printf("<node '%s'\n\ttranslation=(%f, %f, %f) \n\trotation=(%f, %f, %f) \n\tscale=(%f, %f, %f)>\n",
	nodeName,
	translation[0], translation[1], translation[2],
	rotation[0], rotation[1], rotation[2],
	scaling[0], scaling[1], scaling[2]
);*/