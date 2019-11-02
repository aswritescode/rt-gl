#include "mesh.h"
#include "util.h"
#include "plane.h"

#include <fstream>
#include <string>
#include <limits>

// Consider a triangle to intersect a ray if the ray intersects the plane of the
// triangle with barycentric weights in [-weight_tolerance, 1+weight_tolerance]
static const double weight_tolerance = 1e-4;

// Read in a mesh from an obj file.  Populates the bounding box and registers
// one part per triangle (by setting number_parts).
void Mesh::Read_Obj(const char* file)
{
    std::ifstream fin(file);
    if(!fin)
    {
        exit(EXIT_FAILURE);
    }
    std::string line;
    ivec3 e;
    vec3 v;
    box.Make_Empty();
    while(fin)
    {
        getline(fin,line);

        if(sscanf(line.c_str(), "v %lg %lg %lg", &v[0], &v[1], &v[2]) == 3)
        {
            vertices.push_back(v);
            box.Include_Point(v);
        }

        if(sscanf(line.c_str(), "f %d %d %d", &e[0], &e[1], &e[2]) == 3)
        {
            for(int i=0;i<3;i++) e[i]--;
            triangles.push_back(e);
        }
    }
    number_parts=triangles.size();
}

// Check for an intersection against the ray.  See the base class for details.
Hit Mesh::Intersection(const Ray& ray, int part) const
{
    //TODO;
    
    Hit intersection = NO_INTERSECTION; // Initialize the hit to our standardized representation of a miss. This will get overwritten by and collision.

    double distance = -1; // Intialize the distance to a nonsense value. it will be replaced with the Intersection sub-routine.

    if (Intersect_Triangle(ray, part, distance))
        intersection = Hit(this, distance, part);

    return intersection;
}

// Compute the normal direction for the triangle with index part.
vec3 Mesh::Normal(const vec3& point, int part) const
{
    assert(part>=0);

    return cross(vertices[triangles[part][1]] - vertices[triangles[part][0]],  vertices[triangles[part][2]] - vertices[triangles[part][0]]).normalized();
}

// This is a helper routine whose purpose is to simplify the implementation
// of the Intersection routine.  It should test for an intersection between
// the ray and the triangle with index tri.  If an intersection exists,
// record the distance and return true.  Otherwise, return false.
// This intersection should be computed by determining the intersection of
// the ray and the plane of the triangle.  From this, determine (1) where
// along the ray the intersection point occurs (dist) and (2) the barycentric
// coordinates within the triangle where the intersection occurs.  The
// triangle intersects the ray if dist>small_t and the barycentric weights are
// larger than -weight_tolerance.  The use of small_t avoid the self-shadowing
// bug, and the use of weight_tolerance prevents rays from passing in between
// two triangles.
bool Mesh::Intersect_Triangle(const Ray& ray, int tri, double& dist) const
{
    // Switched from an internet-found method to one using the baryocentric coordinate method described in class: https://www.cs.ucr.edu/~craigs/courses/2019-fall-cs-130/lectures/barycentric-coordinates.pdf

    assert(tri>=0);

    vec3 vert0, vert1, vert2, intersection_point;

    // Store the verticies of the triangle for later use (storing them simplfies future calculations)
    vert0 = vertices[triangles[tri][0]];
    vert1 = vertices[triangles[tri][1]];
    vert2 = vertices[triangles[tri][2]];

    Plane triangle_plane(vert0, Normal(vert0, tri)); // Calculate the plane that the triangle lies in

    Hit plane_intersection = triangle_plane.Intersection(ray, tri); // Try and find an intersection between the ray and the plane

    if (plane_intersection.object == nullptr)
        return false; // Since the hit doesn't contain an object, it must have not collided. Return false.
    
    intersection_point = ray.endpoint + (ray.direction * dist); // Get the point of intersection

    // Get the vectors between the points of the traingle (the sides)
    vec3 v1v0 = vert1 - vert0;
    vec3 v2v0 = vert2 - vert0;

    vec3 t = intersection_point - vert0; // Line from the intersected point to v0. This allows us to check its direction and distance against the other two vectors and see if it makes sense
    
    double denom = dot(cross(ray.direction, v1v0), v2v0); // Can be re-used later, so store it for now. Calculate the inverse to avoid dividing multiple times in the future.
    

    if (denom == 0)
        return false; // Avoids devide-by-zero errors when calculating the inverse. 

    double inv_denom = 1 / denom; // Divides are bad, but this one is cool.

    // Find the Baryocentric weights for the three points
    double v1v0_weight = dot(
                   cross(ray.direction, v1v0), 
                   t) * inv_denom;

    double v2v0_weight  = dot(
                   cross(v2v0, ray.direction), 
                   t) * inv_denom;

    double m_weight = 1 - (v1v0_weight + v2v0_weight);

    // compare against weight_tol to prevent ray going between triangles
    if ( m_weight > (01.0) * weight_tol && v1v0_weight > (01.0) * weight_tol && v2v0_weight > (-1.0) * weight_tol ) {
        dist = plane_intersection.dist; // Update the distance to make the upper function aware of it!!! Very important
        return true;
    }
    
    return false; // If we haven't returned true by now, something went wrong.Return false
}

// Compute the bounding box.  Return the bounding box of only the triangle whose
// index is part.
Box Mesh::Bounding_Box(int part) const
{
    Box b;
    b.Make_Empty();

    for (int i = 0; i < 3; i++)
        b.Include_Point(vertices[triangles[part][i]]);
    
    return b;
}
