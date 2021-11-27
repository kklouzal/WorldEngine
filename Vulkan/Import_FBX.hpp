#pragma once

#define FBXSDK_SHARED
#include <fbxsdk.h>

struct FBXMesh {
	std::vector<Vertex> Vertices = {};
	std::vector<uint32_t> Indices = {};

	const char* Texture_Diffuse = "";

	FbxDouble3 translation;
	FbxDouble3 rotation;
	FbxDouble3 scaling;

	~FBXMesh() {
		printf("Destroy FBX Mesh\n");
		Vertices.clear();
		Indices.clear();
	}
};

struct FBXObject {
	std::vector<FBXMesh*> Meshes = {};

	FBXObject() {}

	~FBXObject() {
		printf("Destroy FBX Object\n");
		for (auto M : Meshes) {
			delete M;
		}
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

			//
			//	Find Root Node and traverse all children
			FbxNode* RootNode = Scene->GetRootNode();
			if (RootNode) {
				std::vector<fbxsdk::FbxNode*> Nodes;

				for (unsigned int i = 0; i < RootNode->GetChildCount(); i++) {
					fbxsdk::FbxNode* Child = RootNode->GetChild(i);
					const fbxsdk::FbxNodeAttribute* Attribute = Child->GetNodeAttribute();
					if (Attribute == NULL) { continue; }

					const FbxNodeAttribute::EType AttributeType = Attribute->GetAttributeType();
					if (AttributeType != FbxNodeAttribute::eMesh) { continue; }
					
					Nodes.push_back(Child);
				}

				printf("\t%i Nodes - %zi Usable\n", RootNode->GetChildCount(true), Nodes.size());
				FBXObject* NewFBX = _FbxObjects.emplace(File, new FBXObject).first->second;

				for (auto Node : Nodes) {
					FBXMesh* NewMesh = new FBXMesh;
					NewFBX->Meshes.push_back(NewMesh);

					//
					//	Compute Global Translation Matrix
					FbxAMatrix matrixGeo;
					matrixGeo.SetIdentity();
					if (Node->GetNodeAttribute())
					{
						const fbxsdk::FbxVector4 lT = Node->GetGeometricTranslation(FbxNode::eSourcePivot);
						const fbxsdk::FbxVector4 lR = Node->GetGeometricRotation(FbxNode::eSourcePivot);
						const fbxsdk::FbxVector4 lS = Node->GetGeometricScaling(FbxNode::eSourcePivot);
						matrixGeo.SetT(lT);
						matrixGeo.SetR(lR);
						matrixGeo.SetS(lS);
					}
					FbxAMatrix globalMatrix = Node->EvaluateGlobalTransform();

					FbxAMatrix matrix = globalMatrix * matrixGeo;

					const char* nodeName = Node->GetName();
					NewMesh->translation = Node->LclTranslation.Get();
					NewMesh->rotation = Node->LclRotation.Get();
					NewMesh->scaling = Node->LclScaling.Get();

					// Print the contents of the node.
					printf("<node '%s'\n\ttranslation=(%f, %f, %f) \n\trotation=(%f, %f, %f) \n\tscale=(%f, %f, %f)>\n",
						nodeName,
						NewMesh->translation[0], NewMesh->translation[1], NewMesh->translation[2],
						NewMesh->rotation[0], NewMesh->rotation[1], NewMesh->rotation[2],
						NewMesh->scaling[0], NewMesh->scaling[1], NewMesh->scaling[2]
					);
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
								NewFBX->Meshes.back()->Texture_Diffuse = texture->GetRelativeFileName();
								printf("\tRelative File %s\n", NewFBX->Meshes.back()->Texture_Diffuse);
							}
							else { printf("\tTexture Invalid\n"); }
						}
						else { printf("\tNo Diffuse\n"); }
					}
					else { printf("\tMaterial 1 invalid\n"); }
					//
					//
					const FbxMesh* Mesh = FbxCast<const FbxMesh>(Node->GetNodeAttribute());
					fbxsdk::FbxVector4* Vertices = Mesh->GetControlPoints();
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
					unsigned int lPolyIndexCounter = 0;
					uint32_t IndexCount = 0;
					for (unsigned int j = 0; j < Mesh->GetPolygonCount(); j++) {
						const unsigned int NumVerts = Mesh->GetPolygonSize(j);
						if (NumVerts != 3) { continue; }

						for (unsigned int k = 0; k < NumVerts; k++) {
							Vertex* NewVertex = new Vertex;
							//
							//	Pull Normap Mapping Data
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
							fbxsdk::FbxVector4 lNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);

							NewVertex->normal.x = lNormal[0];
							NewVertex->normal.y = lNormal[1];
							NewVertex->normal.z = lNormal[2];
							//
							//	Pull Texture UV Mapping Data
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
							//	Pull Vertex & Index data
							//	ToDo: Remove duplicate vertices using indices
							const unsigned int Index = Mesh->GetPolygonVertex(j, k);
							//NewVertex->pos.x = (float)(Vertices[Index].mData[0]);
							//NewVertex->pos.y = (float)(Vertices[Index].mData[1]);
							//NewVertex->pos.z = (float)(Vertices[Index].mData[2]);
							fbxsdk::FbxVector4 result = matrix.MultT(Vertices[Index]);
							NewVertex->pos.x = (float)result.mData[0];
							NewVertex->pos.y = (float)result.mData[1];
							NewVertex->pos.z = (float)result.mData[2];

							NewFBX->Meshes.back()->Indices.push_back(IndexCount++);
							NewFBX->Meshes.back()->Vertices.push_back(*NewVertex);
						}
					}
					printf("\tNew Mesh Vertex Count: %zi\n", NewFBX->Meshes.back()->Vertices.size());
				}
				printf("\tOut Mesh Count: %zi\n", NewFBX->Meshes.size());
				return NewFBX;
			}
		}
	}
};