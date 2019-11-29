#pragma once
#include <VHACD.h>

void Decomp(FBXObject* FBX) {

	//	Amount of indices
	const uint32_t nTriangles = FBX->Indices.size();
	//	Array of indices
	std::vector<uint32_t> Triangles;
	for (uint32_t i = 0; i < nTriangles; i++) {
		Triangles.push_back(FBX->Indices[i]);
	}
	//	Amount of points	(point*3=vertex)
	const uint32_t nPoints = FBX->Vertices.size();
	//	Array of vertices	(+0=x, +1=y, +2=z)
	std::vector<float> Points;
	for (uint32_t i = 0; i < nPoints; i++) {
		//	Grab X,Y,Z vertex point
		Points.push_back(FBX->Vertices[i].pos.x);
		Points.push_back(FBX->Vertices[i].pos.y);
		Points.push_back(FBX->Vertices[i].pos.z);
	}

	VHACD::IVHACD::Parameters    params; // V-HACD parameters
	VHACD::IVHACD* interfaceVHACD = VHACD::CreateVHACD(); // create interface

	// compute approximate convex decomposition
	bool res = interfaceVHACD->Compute(Points.data(), (uint32_t)(Points.size() / 3),
		Triangles.data(), (uint32_t)(Triangles.size() / 3), params);

	// read results
	unsigned int nConvexHulls = interfaceVHACD->GetNConvexHulls(); // Get the number of convex-hulls
	VHACD::IVHACD::ConvexHull ch;
	for (unsigned int p = 0; p < nConvexHulls; ++p)
	{
		printf("Convex Hull %i: Center %f %f %f\n", p, ch.m_center[0], ch.m_center[1], ch.m_center[2]);
		interfaceVHACD->GetConvexHull(p, ch); // get the p-th convex-hull information
		for (unsigned int v = 0, idx = 0; v < ch.m_nPoints; ++v, idx += 3)
		{
			printf("\tx=%f, y=%f, z=%f\n", ch.m_points[idx], ch.m_points[idx + 1], ch.m_points[idx + 2]);
		}
		for (unsigned int t = 0, idx = 0; t < ch.m_nTriangles; ++t, idx += 3)
		{
			printf("\ti=%i, j=%i, k=%i\n", ch.m_triangles[idx], ch.m_triangles[idx + 1], ch.m_triangles[idx + 2]);
		}
	}

	// release memory
	interfaceVHACD->Clean();
	interfaceVHACD->Release();
}