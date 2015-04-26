#include "maths.h"

float maths::dot(ObjFileModel::xyz* one, ObjFileModel::xyz* two)
{
	return((one->x * two->x) + (one->y * two->y) + (one->z* two->z));
}

ObjFileModel::xyz maths::cross(ObjFileModel::xyz* v1, ObjFileModel::xyz* v2)
{
	ObjFileModel::xyz v3;

	v3.x = (v1->y * v2->z) - (v1->z * v2->y);
	v3.y = (v1->z * v2->x) - (v1->x * v2->z);
	v3.z = (v1->x * v2->y) - (v1->y * v2->x);


	return v3;
}

ObjFileModel::xyz maths::normal(ObjFileModel::xyz* v1, ObjFileModel::xyz* v2, ObjFileModel::xyz* v3)
{
	ObjFileModel::xyz c1, c2, c3, c4;
	
	c1.x = v2->x - v1->x;
	c1.y = v2->y - v1->y;
	c1.z = v2->z - v1->z;

	c2.x = v3->x - v1->x;
	c2.y = v3->y - v1->y;
	c2.z = v3->z - v1->z;

	c3 = cross(&c1, &c2);

	float length = sqrt((c3.x * c3.x) + (c3.y * c3.y) + (c3.z * c3.z));

	c4.x = c3.x / length;
	c4.y = c3.y / length;
	c4.z = c3.z / length;

	return c4;
}

maths::Plane maths::triPlane(ObjFileModel::xyz* v1, ObjFileModel::xyz* v2, ObjFileModel::xyz* v3)
{
	ObjFileModel::xyz c1, c2;
	float f1;
	Plane result;

	c1 =  normal(v1, v2, v3);
	f1 = dot(v1, &c1);

	result.normal = c1;
	result.d = f1;

	return result;
}

float maths::equPlane(Plane p1, ObjFileModel::xyz v1)
{

	return (v1.x + v1.y + v1.z + p1.d);
}

ObjFileModel::xyz maths::planeIntersection(Plane* p1, ObjFileModel::xyz* v1, ObjFileModel::xyz* v2)
{
	ObjFileModel::xyz ray, p;
	float t;

	ray.x = v1->x - v2->x;
	ray.y = v1->y - v2->y;
	ray.z = v1->z - v2->z;

	t = (-(p1->d) - dot(&p1->normal, v2) / dot(&p1->normal, &ray));

	p.x = v1->x + (ray.x * t);
	p.y = v1->y + (ray.y * t);
	p.z = v1->z + (ray.z * t);

	return p;
}

bool maths::in_triangle(ObjFileModel::xyz* triangle0, ObjFileModel::xyz* triangle1, ObjFileModel::xyz* triangle2, ObjFileModel::xyz* point)
{
	ObjFileModel::xyz ap, ab, bp, bc, cp, ca;
	ap = cross(triangle0, point);
	ab = cross(triangle0, triangle1);
	bp = cross(triangle1, point);
	bc = cross(triangle1, triangle2);
	cp = cross(triangle2, point);
	ca = cross(triangle2, triangle0);

	ObjFileModel::xyz c1, c2, c3;

	c1 = cross(&ap, &ab);
	c2 = cross(&bp, &bc);
	c3 = cross(&cp, &ca);

	ObjFileModel::xyz compare;
	compare.x = 0;
	compare.y = 0;
	compare.z = 0;

	if (c1.x > compare.x && c1.y > compare.y && c1.z > compare.z &&
		c2.x > compare.x && c2.y > compare.y && c2.z > compare.z  &&
		c3.x > compare.x&& c3.y > compare.y && c3.z > compare.z)
	{
		return true;
	}

	if (c1.x < compare.x && c1.y < compare.y && c1.z < compare.z &&
		c2.x < compare.x && c2.y < compare.y && c2.z < compare.z  &&
		c3.x < compare.x&& c3.y < compare.y && c3.z < compare.z)
	{
		return false;
	}

}
