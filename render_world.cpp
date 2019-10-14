#include "render_world.h"
#include "flat_shader.h"
#include "object.h"
#include "light.h"
#include "ray.h"

extern bool disable_hierarchy;

Render_World::Render_World()
    :background_shader(0),ambient_intensity(0),enable_shadows(true),
    recursion_depth_limit(3)
{}

Render_World::~Render_World()
{
    delete background_shader;
    for(size_t i=0;i<objects.size();i++) delete objects[i];
    for(size_t i=0;i<lights.size();i++) delete lights[i];
}

// Find and return the Hit structure for the closest intersection.  Be careful
// to ensure that hit.dist>=small_t.
Hit Render_World::Closest_Intersection(const Ray& ray)
{
    double T_MIN_LIMIT = 0.01; // The minimum allowable distance for the hit to be counted
    double min_t = std::numeric_limits<double>::max(); // Set min_t to the largest possible double value


    Hit closest_hit;

    for (auto& obj : objects) { // For each object in the world

        for (int i = 0; i < obj->number_parts; i++) { // For each part on a given object
            Hit collision = obj -> Intersection(ray, i); // Get a hit
        
            if (collision.dist < min_t && collision.dist > T_MIN_LIMIT) { // Checks if the hit is closer than the previous-closest hit. Ignores if the hit is itself
                min_t = collision.dist;
                closest_hit = collision;

            }
        }
    }

    return closest_hit;
}

// set up the initial view ray and call
void Render_World::Render_Pixel(const ivec2& pixel_index)
{

    vec3 p2 = camera.World_Position(pixel_index);

    vec3 direction_raw;
    
    // Build the direction vector from p1 and p2;
    for (int i = 0; i < 3; i++)
        direction_raw[i] = p2[i] - camera.position[i];
    
    vec3 direction = direction_raw.normalized();

    Ray ray(camera.position, direction); // Initialize the ray using the camera position as the end point and the direction vector calculated earlier.
    vec3 color=Cast_Ray(ray,1);
    camera.Set_Pixel(pixel_index,Pixel_Color(color));
}

void Render_World::Render()
{
    if(!disable_hierarchy)
        Initialize_Hierarchy();

    for(int j=0;j<camera.number_pixels[1];j++)
        for(int i=0;i<camera.number_pixels[0];i++)
            Render_Pixel(ivec2(i,j));
}

// cast ray and return the color of the closest intersected surface point,
// or the background color if there is no object intersection
vec3 Render_World::Cast_Ray(const Ray& ray,int recursion_depth)
{
    vec3 color;
    TODO; // determine the color here
    return color;
}

void Render_World::Initialize_Hierarchy()
{
    TODO; // Fill in hierarchy.entries; there should be one entry for
    // each part of each object.

    hierarchy.Reorder_Entries();
    hierarchy.Build_Tree();
}
