#pragma once

#define FBXSDK_SHARED
#include <fbxsdk.h>

struct FBXObject {
	std::vector<Vertex> Vertices = {};
	std::vector<uint32_t> Indices = {};
};

class ImportFBX {
	fbxsdk::FbxGeometryConverter* GeometryConverter;
public:
	/**
	 * Print an attribute.
	 */
	FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
		switch (type) {
		case FbxNodeAttribute::eUnknown: return "unidentified";
		case FbxNodeAttribute::eNull: return "null";
		case FbxNodeAttribute::eMarker: return "marker";
		case FbxNodeAttribute::eSkeleton: return "skeleton";
		case FbxNodeAttribute::eMesh: return "mesh";
		case FbxNodeAttribute::eNurbs: return "nurbs";
		case FbxNodeAttribute::ePatch: return "patch";
		case FbxNodeAttribute::eCamera: return "camera";
		case FbxNodeAttribute::eCameraStereo: return "stereo";
		case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
		case FbxNodeAttribute::eLight: return "light";
		case FbxNodeAttribute::eOpticalReference: return "optical reference";
		case FbxNodeAttribute::eOpticalMarker: return "marker";
		case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
		case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
		case FbxNodeAttribute::eBoundary: return "boundary";
		case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
		case FbxNodeAttribute::eShape: return "shape";
		case FbxNodeAttribute::eLODGroup: return "lodgroup";
		case FbxNodeAttribute::eSubDiv: return "subdiv";
		default: return "unknown";
		}
	}
	/**
	 * Print an attribute.
	 */
	void PrintAttribute(FbxNodeAttribute* pAttribute) {
		if (!pAttribute) return;

		FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
		FbxString attrName = pAttribute->GetName();
		// Note: to retrieve the character array of a FbxString, use its Buffer() method.
		printf("\t<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
	}

	FbxManager* _FbxManager = nullptr;

	ImportFBX() {
		_FbxManager = FbxManager::Create();
		FbxIOSettings* _FbxSettings = FbxIOSettings::Create(_FbxManager, IOSROOT);
		_FbxManager->SetIOSettings(_FbxSettings);
		GeometryConverter = new fbxsdk::FbxGeometryConverter(_FbxManager);
	}

	~ImportFBX() {
		delete GeometryConverter;
		_FbxManager->Destroy();

	}

	//	For now, only return mesh nodes.
	//	ToDo: Implement lights and possibly others
	void SearchNodes(fbxsdk::FbxNode* Node, std::vector<fbxsdk::FbxNode*>& Nodes) {
		for (int i = 0; i < Node->GetChildCount(); i++) {
			fbxsdk::FbxNode* Child = Node->GetChild(i);
			fbxsdk::FbxNodeAttribute* Attribute = Child->GetNodeAttribute();
			if (Attribute == NULL) {
				SearchNodes(Child, Nodes);
			}
			else {
				FbxNodeAttribute::EType AttributeType = Attribute->GetAttributeType();
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

	FBXObject* Import(const char* File) {
		FbxImporter* Importer = FbxImporter::Create(_FbxManager, "");
		if (!Importer->Initialize(File, -1, _FbxManager->GetIOSettings())) {
			printf("FBX Import Initialize Failed: %s", Importer->GetStatus().GetErrorString());
			return nullptr;
		}


		FbxScene* Scene = FbxScene::Create(_FbxManager, "NewScene");
		Importer->Import(Scene);
		Importer->Destroy();
		GeometryConverter->Triangulate(Scene, true);

		FbxNode* RootNode = Scene->GetRootNode();
		if (RootNode) {
			std::vector<fbxsdk::FbxNode*> Nodes;
			SearchNodes(RootNode, Nodes);
			printf("Nodes Size: %i (%i)\n", RootNode->GetChildCount(true), Nodes.size());

			std::vector<Vertex> OutVertices = {};
			std::vector<uint32_t> OutIndices = {};

			uint32_t IndexCount = 0;
			for (auto Node : Nodes) {
				FbxMesh* Mesh = (FbxMesh*)Node->GetNodeAttribute();

				FbxVector4* Vertices = Mesh->GetControlPoints();

				for (int j = 0; j < Mesh->GetPolygonCount(); j++) {
					int NumVerts = Mesh->GetPolygonSize(j);
					//printf("NumVerts: %i\n", NumVerts);
					if (NumVerts != 3 ) { continue; }
					for (int k = 0; k < NumVerts; k++) {
						int VertID = Mesh->GetPolygonVertex(j, k);
						Vertex NewVertex{};
						NewVertex.pos.x = (float)Vertices[VertID].mData[0];
						NewVertex.pos.y = (float)Vertices[VertID].mData[1];
						NewVertex.pos.z = (float)Vertices[VertID].mData[2];
						OutVertices.push_back(NewVertex);
						OutIndices.push_back(++IndexCount);
					}
				}
			}
			FBXObject* NewFBX = new FBXObject;
			NewFBX->Vertices.swap(OutVertices);
			NewFBX->Indices.swap(OutIndices);
			printf("Out Vertex Count: %i\n", OutVertices.size());
			return NewFBX;
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