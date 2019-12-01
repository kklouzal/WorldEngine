#pragma once
#include <VHACD.h>

struct DecompResults {
	btCompoundShape* CompoundShape = nullptr;
	btAlignedObjectArray<btConvexShape*> m_convexShapes = {};
	btAlignedObjectArray<btTriangleMesh*> m_trimeshes = {};
};

//
//	FBXObject contains vectors of Vertices and Indices
DecompResults* Decomp(FBXObject* FBX) {
	//
	//	Setup Indices
	const uint32_t nTriangles = FBX->Indices.size();
	std::vector<uint32_t> Triangles;
	for (uint32_t i = 0; i < nTriangles; i++) {
		Triangles.push_back(FBX->Indices[i]);
	}
	//
	//	Setup Points (3 Points is 1 Vertex)
	const uint32_t nPoints = FBX->Vertices.size();
	std::vector<float> Points;
	for (uint32_t i = 0; i < nPoints; i++) {
		Points.push_back(FBX->Vertices[i].pos.x);
		Points.push_back(FBX->Vertices[i].pos.y);
		Points.push_back(FBX->Vertices[i].pos.z);
	}
	//
	//	Setup VHACD Parameters and create its interface
	VHACD::IVHACD::Parameters params;
	VHACD::IVHACD* interfaceVHACD = VHACD::CreateVHACD();
	//
	//	Compute approximate convex decomposition
	//printf("Compute V-HACD: Points %i Triangles %i\n", Points.size(), Triangles.size());
	bool res = interfaceVHACD->Compute(Points.data(), (uint32_t)(Points.size() / 3),
		Triangles.data(), (uint32_t)(Triangles.size() / 3), params);
	//
	//	Get the number of convex hulls
	unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls();
	//printf("V-HACD Done: Hull Count %i\n", nConvexHulls);
	//
	//	Iterate through each convex hull
	DecompResults* Results = new DecompResults;
	btAlignedObjectArray<btVector3> m_convexCentroids;
	VHACD::IVHACD::ConvexHull Hull;
	for (unsigned int h = 0; h < nConvexHulls; ++h)
	{
		//printf("\tHull: %i\n", h);
		//	Fill Hull for each individual convex hull
		interfaceVHACD->GetConvexHull(h, Hull);

		btAlignedObjectArray<btVector3> vertices;

		btTriangleMesh* trimesh = new btTriangleMesh();
		Results->m_trimeshes.push_back(trimesh);
		btVector3 centroid = btVector3(0, 0, 0);
		printf("Hull Center %f %f %f\n", Hull.m_center[0], Hull.m_center[1], Hull.m_center[2]);
		//
		//	Calculate centroid and fill vertices
		for (unsigned int i = 0; i < Hull.m_nTriangles; i++) {
			const unsigned int index0 = Hull.m_triangles[i * 3];
			const unsigned int index1 = Hull.m_triangles[i * 3 + 1];
			const unsigned int index2 = Hull.m_triangles[i * 3 + 2];

			btVector3 vertex0(Hull.m_points[index0 * 3], Hull.m_points[index0 * 3 + 1], Hull.m_points[index0 * 3 + 2]);
			btVector3 vertex1(Hull.m_points[index1 * 3], Hull.m_points[index1 * 3 + 1], Hull.m_points[index1 * 3 + 2]);
			btVector3 vertex2(Hull.m_points[index2 * 3], Hull.m_points[index2 * 3 + 1], Hull.m_points[index2 * 3 + 2]);

			centroid += vertex0;
			centroid += vertex1;
			centroid += vertex2;

			vertices.push_back(vertex0);
			vertices.push_back(vertex1);
			vertices.push_back(vertex2);
		}
		//printf("\t\tPoints: %i\n", Hull.m_points);
		//printf("\t\tTriangles: %i\n", Hull.m_triangles);
		//printf("\t\tVertices: %i\n", vertices.size());
		//
		//	Offset vertices by centroid and add them to the trimesh
		for (unsigned int i = 0; i < vertices.size()/3; i++) {

			trimesh->addTriangle(vertices[i*3] - centroid, vertices[i*3 + 1] - centroid, vertices[i*3 + 2] - centroid);
		}
		//
		//	Create a new ConvexShape from our Hull
		btConvexShape* convexShape = new btConvexTriangleMeshShape(trimesh);
		Results->m_convexShapes.push_back(convexShape);
		m_convexCentroids.push_back(centroid);
	}
	//
	//	Create a new Compound Shape
	Results->CompoundShape = new btCompoundShape();
	//
	btTransform trans;
	trans.setIdentity();
	//
	//	Add each individual Convex Shape into our Compound Shape offset by its centroid
	for (unsigned int i = 0; i < nConvexHulls; i++)
	{
		//printf("\tConvex Shape: %i\n", i);
		btVector3 centroid = m_convexCentroids[i];
		//printf("\t\tCentroid: %f %f %f\n", centroid.x(), centroid.y(), centroid.z());
		trans.setOrigin(centroid);
		Results->CompoundShape->addChildShape(trans, Results->m_convexShapes[i]);
	}
	//
	// release memory
	interfaceVHACD->Clean();
	interfaceVHACD->Release();

	//	Return our Compound Shape full of Convexically Decomposed Convex Shapes
	return Results;
}