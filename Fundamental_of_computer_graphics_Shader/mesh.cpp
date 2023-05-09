#include <stdlib.h>
#include <stdio.h>
#include "mesh.h"
#include "algebra.h"

float rnd() {
	return 2.0f * float(rand()) / float(RAND_MAX) - 1.0f;
}

void insertModel(Mesh **list, int nv, float * vArr, int nt, int * tArr, float scale, Vector translation, Vector rotation, Vector scaling, int id, int LAB) {
	Mesh * mesh = (Mesh *) malloc(sizeof(Mesh));
	mesh->nv = nv;
	mesh->nt = nt;	
	mesh->vertices = (Vector *) malloc(nv * sizeof(Vector));
	mesh->vnorms = (Vector*)calloc(sizeof(Vector),nv);
	mesh->tnorms = (Vector*)calloc(sizeof(Vector),nt);
	mesh->position = translation;
	mesh->orientation = rotation;
	mesh->size = scaling;
	mesh->ID = id;
	mesh->triangles = (Triangle *) malloc(nt * sizeof(Triangle));
	
	// set mesh vertices
	for (int i = 0; i < nv; i++) {
		mesh->vertices[i].x = vArr[i*3] * scale;
		mesh->vertices[i].y = vArr[i*3+1] * scale;
		mesh->vertices[i].z = vArr[i*3+2] * scale;
	}
	// set mesh triangles
	///*
	for (int i = 0; i < nt; i++) {
		mesh->triangles[i].vInds[0] = tArr[i*3];
		mesh->triangles[i].vInds[1] = tArr[i*3+1];
		mesh->triangles[i].vInds[2] = tArr[i*3+2];
	}
	//*/
	///*
	for (int i = 0; i < nt; i++) {
		const int idxA = tArr[i * 3],
			idxB = tArr[i * 3 + 1],
			idxC = tArr[i * 3 + 2];

		const Vector sideAB = Subtract(mesh->vertices[idxB], mesh->vertices[idxA]);
		const Vector sideAC = Subtract(mesh->vertices[idxC], mesh->vertices[idxA]);

		const Vector faceNormal = Normalize(CrossProduct(sideAB, sideAC));
		mesh->vnorms[idxA] = Add(mesh->vnorms[idxA], faceNormal);
		mesh->vnorms[idxB] = Add(mesh->vnorms[idxB], faceNormal);
		mesh->vnorms[idxC] = Add(mesh->vnorms[idxC], faceNormal);

		mesh->tnorms[i] = faceNormal;

		mesh->triangles[i].vInds[0] = idxA;
		mesh->triangles[i].vInds[1] = idxB;
		mesh->triangles[i].vInds[2] = idxC;
	}
	//*/
	// Assignment 1: 
	// Calculate and store suitable vertex normals for the mesh here.
	// Replace the code below that simply sets some arbitrary normal values
	//calculate triangle normals
	/*
	Vector temp1;
	Vector temp2;
	for (int i = 0; i < nt; i++)
	{
		temp1 = Subtract( mesh->vertices[mesh->triangles[i].vInds[1]], mesh->vertices[mesh->triangles[i].vInds[0]]); //Subtract one triangle vertex from another to get vector
		temp2 = Subtract( mesh->vertices[mesh->triangles[i].vInds[2]], mesh->vertices[mesh->triangles[i].vInds[0]]); //Same as above only with different vertex to subtract from
		mesh->tnorms[i] = CrossProduct(temp1, temp2); //Use crospoint multiplication to get perpendicular vector against triangle surrface
		mesh->tnorms[i] = Normalize(mesh->tnorms[i]); //Normalize vector which is perpendicuar to triangle surface get length one for vector
	}
	*/
	//srand(0xBADB0LL);
	//calculate vertex normals
	
	for (int i = 0; i < nv; i++) {
		/*
		for (int j = 0; j < nt; j++)
				//Check if triangle does not point to NULL otherwize gives warning. Next checks if some of the triangles vertex have same index as current vertex normal have
				if (mesh->triangles != NULL && (mesh->triangles[j].vInds[0] == i || mesh->triangles[j].vInds[1] == i || mesh->triangles[j].vInds[2] == i))
					mesh->vnorms[i] = Add(mesh->vnorms[i], mesh->tnorms[j]);//In case indeces are same the triangle normal will be added to vertex normal 
		*/
		mesh->vnorms[i] = Normalize(mesh->vnorms[i]);//Normalize vertex normals created from added triangle normals
		if(LAB == 1 || LAB == 3)
		{ 
			mesh->vnorms[i].x = 0.80078125f;
			mesh->vnorms[i].y = 0.34765625f;
			mesh->vnorms[i].z = 0.1796875f;
		}
	}
	
	mesh->next = *list;
	*list = mesh;	
}
