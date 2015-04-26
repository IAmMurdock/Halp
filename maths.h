#include "objfilemodel.h"
#include <vector>

class maths
{
private:




public:

	struct Plane
	{
		ObjFileModel::xyz normal; /* the normal to the plane */
		float d; /* the 'd' constant in the equation for this plane */
	};

	float dot(ObjFileModel::xyz* one, ObjFileModel::xyz* two);
	ObjFileModel::xyz cross(ObjFileModel::xyz* one, ObjFileModel::xyz* two);
	ObjFileModel::xyz normal(ObjFileModel::xyz* v1, ObjFileModel::xyz* v2, ObjFileModel::xyz* v3);
	Plane triPlane(ObjFileModel::xyz* v1, ObjFileModel::xyz* v2, ObjFileModel::xyz* v3);
	float equPlane(Plane p1, ObjFileModel::xyz v1);
	ObjFileModel::xyz planeIntersection(Plane* p1, ObjFileModel::xyz* v1, ObjFileModel::xyz* v2);
	bool in_triangle(ObjFileModel::xyz* triangle0, ObjFileModel::xyz* triangle1, ObjFileModel::xyz* triangle2, ObjFileModel::xyz* point);
};