#pragma once
#define ENABLE_VHACD_IMPLEMENTATION 1
#include "VHACD.h"

//
//	Structure containing objects created during composition
struct DecompResults {
	btCompoundShape* CompoundShape = nullptr;
	btAlignedObjectArray<btConvexShape*> m_convexShapes = {};
	btAlignedObjectArray<btTriangleMesh*> m_trimeshes = {};
};

//
//	FBXObject contains vectors of Vertices and Indices
DecompResults* Decomp(GLTFInfo* Infos) {
	//
	//	Setup VHACD Parameters and create its interface
	VHACD::IVHACD::Parameters params;
	params.m_maxConvexHulls = 100;						//	1 - 100,000
	params.m_resolution = 100000;						//	10,000 - 10,000,000
	params.m_minimumVolumePercentErrorAllowed = 5;		//	0 - 50
	params.m_maxRecursionDepth = 8;						//	2 - 64
	params.m_shrinkWrap = true;							//	true/false
	params.m_logger = nullptr;
	params.m_callback = nullptr;
	params.m_fillMode = VHACD::FillMode::FLOOD_FILL;	//	FLOOD/RAYCAST/SURFACE
	params.m_maxNumVerticesPerCH = 128;					//	8 - 2048
	params.m_asyncACD = false;							//	true/false
	params.m_findBestPlane = true;						//	true/false
	params.m_minEdgeLength = 1;							//	1 - 32

	VHACD::IVHACD* interfaceVHACD = VHACD::CreateVHACD();
	//
	//	Setup Indices
	double* points = new double[Infos->Vertices.size() * 3];
	for (uint32_t i = 0; i < Infos->Vertices.size(); i++)
	{
		points[i * 3] = Infos->Vertices[i].pos.x;
		points[(i * 3) + 1] = Infos->Vertices[i].pos.y;
		points[(i * 3) + 2] = Infos->Vertices[i].pos.z;
	}
	const bool res = interfaceVHACD->Compute(points, static_cast<uint32_t>(Infos->Vertices.size()), Infos->Indices.data(), static_cast<uint32_t>(Infos->Indices.size() / 3), params);

	VHACD::IVHACD::ConvexHull Hull;
	//
	//	Compute approximate convex decomposition
	//printf("Compute V-HACD: Points %i Triangles %i\n", Points.size(), Triangles.size());
	//const bool res = interfaceVHACD->Compute(Points.data(), (uint32_t)(Points.size() / 3),
	//	Triangles.data(), (uint32_t)(Triangles.size() / 3), params);
	//
	//	Get the number of convex hulls
	const uint32_t nConvexHulls = interfaceVHACD->GetNConvexHulls();
	//printf("V-HACD Done: Hull Count %i\n", nConvexHulls);
	//
	//	Create a new DecompResults structure
	DecompResults* Results = new DecompResults;
	//
	//	Create a new Compound Shape for this decomposition
	Results->CompoundShape = new btCompoundShape();
	//printf("[DECOMP] Num Hulls: %i\n", nConvexHulls);
	//
	//	Iterate through each convex hull and fill results
	for (uint32_t h = 0; h < nConvexHulls; ++h)
	{
		//
		//	Fill 'Hull' for each individual convex hull
		interfaceVHACD->GetConvexHull(h, Hull);
		//printf("\tHull: %i\n", h);
		//printf("\t\tVerts: %i\n", Hull.m_nPoints);
		//printf("\t\tTris: %i\n", Hull.m_nTriangles);
		//
		//	Create a new ConvexShape from this hulls Triangle Mesh
		btConvexHullShape* convexShape = new btConvexHullShape();
		//
		//	Iterate through this hulls triangles
		for (uint32_t i = 0; i < Hull.m_triangles.size(); i++) {
			//
			//	Calculate indices
			VHACD::Triangle T = Hull.m_triangles[i];
			//
			//	Calculate vertices
			const btVector3 vertex0(static_cast<btScalar>(Hull.m_points[T.mI0].mX), static_cast<btScalar>(Hull.m_points[T.mI0].mY), static_cast<btScalar>(Hull.m_points[T.mI0].mZ));
			const btVector3 vertex1(static_cast<btScalar>(Hull.m_points[T.mI1].mX), static_cast<btScalar>(Hull.m_points[T.mI1].mY), static_cast<btScalar>(Hull.m_points[T.mI1].mZ));
			const btVector3 vertex2(static_cast<btScalar>(Hull.m_points[T.mI2].mX), static_cast<btScalar>(Hull.m_points[T.mI2].mY), static_cast<btScalar>(Hull.m_points[T.mI2].mZ));
			//
			//	Add this triangle into our Triangle Mesh
			convexShape->addPoint(vertex0);
			convexShape->addPoint(vertex1);
			convexShape->addPoint(vertex2);
		}
		Results->m_convexShapes.push_back(convexShape);
		//
		//	Grab the hulls center position
		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(static_cast<btScalar>(Hull.m_center[0]), static_cast<btScalar>(Hull.m_center[1]), static_cast<btScalar>(Hull.m_center[2])));
		//
		//	Add this ConvexShape to our CompoundShape
		Results->CompoundShape->addChildShape(trans, convexShape);
	}
	//
	// release memory
	interfaceVHACD->Clean();
	interfaceVHACD->Release();
	//
	//	Return our DecompResults containing the CompoundShape full of Convexically Decomposed Convex Shapes
	return Results;
}