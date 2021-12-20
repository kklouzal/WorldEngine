#pragma once
#include <VHACD.h>

//
//	Structure containing objects created during composition
struct DecompResults {
	ndShapeCompound* CompoundShape = nullptr;
	std::vector<ndShapeConvex*> m_convexShapes = {};
	//btAlignedObjectArray<btTriangleMesh*> m_trimeshes = {};
};

//
//	FBXObject contains vectors of Vertices and Indices
DecompResults* Decomp(GLTFInfo* Infos) {
	//
	//	Setup Indices
	const size_t nTriangles = Infos->Indices.size();
	printf("[DECOMP] Index Count: %zu (Triangles: %zu)", nTriangles, nTriangles / 3);
	//
	//	Setup Points (3 Points is 1 Vertex)
	const size_t nPoints = Infos->Vertices.size();
	printf("[DECOMP] Raw Verticies %zu", nPoints);
	std::vector<float> Points;
	for (size_t i = 0; i < nPoints; i++) {
		Points.emplace_back(Infos->Vertices[i].pos.x);
		Points.emplace_back(Infos->Vertices[i].pos.y);
		Points.emplace_back(Infos->Vertices[i].pos.z);
	}
	printf(" (Points: %zu)\n", Points.size());
	//
	//	Setup VHACD Parameters and create its interface
	VHACD::IVHACD::Parameters params;
	VHACD::IVHACD* interfaceVHACD = VHACD::CreateVHACD();
	VHACD::IVHACD::ConvexHull Hull;
	params.m_resolution = 100000;
	params.m_concavity = 0.0025;
	params.m_planeDownsampling = 4;
	params.m_convexhullDownsampling = 4;
	params.m_alpha = 0.05;
	params.m_beta = 0.05;
	params.m_pca = 0;
	params.m_maxNumVerticesPerCH = 64;
	params.m_minVolumePerCH = 0.0001;
	//printf("alpha %f\n", params.m_alpha);
	//printf("beta %f\n", params.m_beta);
	//printf("concavity %f\n", params.m_concavity);
	//printf("approx %i\n", params.m_convexhullApproximation);
	//printf("downsample %i\n", params.m_convexhullDownsampling);
	//printf("max hulls %i\n", params.m_maxConvexHulls);
	//printf("max verts %i\n", params.m_maxNumVerticesPerCH);
	//printf("min volume %f\n", params.m_minVolumePerCH);
	//printf("mode %i\n", params.m_mode);
	//printf("accel %i\n", params.m_oclAcceleration);
	//printf("pca %i\n", params.m_pca);
	//printf("downsample %i\n", params.m_planeDownsampling);
	//printf("hull verts %i\n", params.m_projectHullVertices);
	//printf("res %i\n", params.m_resolution);
	//
	//	Compute approximate convex decomposition
	//printf("Compute V-HACD: Points %i Triangles %i\n", Points.size(), Triangles.size());
	//const bool res = interfaceVHACD->Compute(Points.data(), (uint32_t)(Points.size() / 3),
	//	Triangles.data(), (uint32_t)(Triangles.size() / 3), params);
	const bool res = interfaceVHACD->Compute(Points.data(), (uint32_t)nPoints/3, Infos->Indices.data(), (uint32_t)nTriangles/3, params);
	//
	//	Get the number of convex hulls
	const uint32_t nConvexHulls = interfaceVHACD->GetNConvexHulls();
	//printf("V-HACD Done: Hull Count %i\n", nConvexHulls);
	//
	//	Create a new DecompResults structure
	DecompResults* Results = new DecompResults;
	//
	//	Create a new Compound Shape for this decomposition
	Results->CompoundShape = new ndShapeCompound();
	printf("[DECOMP] Num Hulls: %i\n", nConvexHulls);
	//
	//	Iterate through each convex hull and fill results
	for (uint32_t h = 0; h < nConvexHulls; ++h)
	{
		//
		//	Fill 'Hull' for each individual convex hull
		interfaceVHACD->GetConvexHull(h, Hull);
		printf("\tHull: %i\n", h);
		printf("\t\tVerts: %i\n", Hull.m_nPoints);
		printf("\t\tTris: %i\n", Hull.m_nTriangles);
		//
		//	Iterate through this hulls triangles
		std::vector<ndVector> Verts;
		for (uint32_t i = 0; i < Hull.m_nTriangles; i++) {
			//
			//	Calculate indices
			const uint32_t index0 = Hull.m_triangles[i * 3];
			const uint32_t index1 = Hull.m_triangles[i * 3 + 1];
			const uint32_t index2 = Hull.m_triangles[i * 3 + 2];
			//
			//	Calculate vertices
			Verts.emplace_back(Hull.m_points[index0 * 3], Hull.m_points[index0 * 3 + 1], Hull.m_points[index0 * 3 + 2], 0);
			Verts.emplace_back(Hull.m_points[index1 * 3], Hull.m_points[index1 * 3 + 1], Hull.m_points[index1 * 3 + 2], 0);
			Verts.emplace_back(Hull.m_points[index2 * 3], Hull.m_points[index2 * 3 + 1], Hull.m_points[index2 * 3 + 2], 0);
		}
		//
		//	Create a new ConvexShape from this hulls Triangle Mesh
		ndShapeConvexHull* convexShape = new ndShapeConvexHull((ndInt32)Verts.size(), sizeof(ndVector), 0.0f, &Verts[0].m_x);
		Results->m_convexShapes.push_back(convexShape);
		//
		//	Grab the hulls center position
		ndMatrix matrix(dGetIdentityMatrix());
		matrix.m_posit = ndVector(Hull.m_center[0], Hull.m_center[1], Hull.m_center[2], 0);
		//
		//	Add this ConvexShape to our CompoundShape
		Results->CompoundShape->AddCollision(new ndShapeInstance(convexShape));
	}
	//
	// release memory
	interfaceVHACD->Clean();
	interfaceVHACD->Release();
	//
	//	Return our DecompResults containing the CompoundShape full of Convexically Decomposed Convex Shapes
	return Results;
}