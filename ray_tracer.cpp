/**
 * ray_tracer.cpp
 * CS230
 * -------------------------------
 * Implement ray tracer here.
 */

#define SET_RED(P, C)   (P = (((P) & 0x00ffffff) | ((C) << 24)))
#define SET_GREEN(P, C)  (P = (((P) & 0xff00ffff) | ((C) << 16)))
#define SET_BLUE(P, C) (P = (((P) & 0xffff00ff) | ((C) << 8)))

#include "ray_tracer.h"

using namespace std;

const double Object::small_t=1e-6;
//--------------------------------------------------------------------------------
// utility functions
//--------------------------------------------------------------------------------
double sqr(const double x)
{
    return x*x;
}

Pixel Pixel_Color(const Vector_3D<double>& color)
{
    Pixel pixel=0;
    SET_RED(pixel,(unsigned char)(min(color.x,1.0)*255));
    SET_GREEN(pixel,(unsigned char)(min(color.y,1.0)*255));
    SET_BLUE(pixel,(unsigned char)(min(color.z,1.0)*255));
    return pixel;
}
//--------------------------------------------------------------------------------
// Shader
//--------------------------------------------------------------------------------
Vector_3D<double> Phong_Shader::
Shade_Surface(const Ray& ray,const Object& intersection_object,const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const
{
    Vector_3D<double> color;
    
    Vector_3D<double> Call;
    Vector_3D<double> dirLight;
    Vector_3D<double> normal;
    Vector_3D<double> direction_to_camera;
    Vector_3D<double> result;
    Vector_3D<double> Ambient;
    Vector_3D<double> Diffuse;
    Vector_3D<double> Specular;
    double diff_temp;
    double spec_temp;
    
    //color = color_ambient;
    for(unsigned int i = 0; i < world.lights.size();i++)
    {
        
        Ambient = color_ambient* world.lights.at(i)->Emitted_Light(ray);
        color += Ambient;
        
        dirLight = world.lights.at(i)->position - intersection_point;
        normal = same_side_normal;
        direction_to_camera = ray.direction;
        direction_to_camera = direction_to_camera*-1;
        
        dirLight.Normalize();
        normal.Normalize();
        direction_to_camera.Normalize();
        
        diff_temp = max(0.0,Call.Dot_Product(normal,dirLight));
        //diff_temp = Call.Dot_Product(dirLight,normal);
        //cout << "DIFF_TEMP: " << diff_temp << " || ";
        
        result = normal * diff_temp * 2;
        result = result - dirLight;
        
        //result.Normalize();
        
        spec_temp = pow(max(0.0,Call.Dot_Product(result,direction_to_camera)), specular_power);
        //spec_temp = pow(Call.Dot_Product(direction_to_camera,result), specular_power);
        //cout << "SPEC_TEMP: " << spec_temp << endl;
        
        Diffuse = color_diffuse * diff_temp * world.lights.at(i)->Emitted_Light(ray);
        Specular = color_specular * spec_temp * world.lights.at(i)->Emitted_Light(ray);
        
        if(world.enable_shadows)
        {
            Ray Shadow_Ray(intersection_point + same_side_normal * intersection_object.small_t, world.lights.at(i)->position 
            - intersection_point);
            bool shadow_check = false;
            
            for(unsigned int j = 0; j < world.objects.size();j++)
            {
                if(world.objects.at(j)->Intersection(Shadow_Ray))
                {
                    shadow_check = true;
                }
            }
            
            if(!shadow_check)
            {
                color += Diffuse + Specular;
            }
        }
        else
        {
            color += Diffuse + Specular;
        }
        
    }
    

    // TODO: determine the color

    return color;
}

Vector_3D<double> Reflective_Shader::
Shade_Surface(const Ray& ray,const Object& intersection_object,const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const
{
    Vector_3D<double> color;
    Vector_3D<double> Call;
    // TODO: determine the color
    
    color = Phong_Shader::Shade_Surface(ray, intersection_object,intersection_point, same_side_normal);
    
    Vector_3D<double> end = intersection_point;
    Vector_3D<double> dir = ray.direction - same_side_normal * 2 * Call.Dot_Product(same_side_normal,ray.direction);
    
    Ray reflect(end, dir);
    
    reflect.recursion_depth = ray.recursion_depth;
    reflect.recursion_depth++;
    
    if(reflect.recursion_depth != world.recursion_depth_limit)
    {
        return color = color + world.Cast_Ray(reflect,ray) * reflectivity;
    }
    else
    {
        return color;
    }
}

Vector_3D<double> Flat_Shader::
Shade_Surface(const Ray& ray,const Object& intersection_object,const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const
{
    return color;
}

//--------------------------------------------------------------------------------
// Objects
//--------------------------------------------------------------------------------
// determine if the ray intersects with the sphere
// if there is an intersection, set t_max, current_object, and semi_infinite as appropriate and return true
bool Sphere::
Intersection(Ray& ray) const
{
    // TODO
    
    Vector_3D<double> Call, U_Vector, S_Vector;
    
    double Check, Delta;
            
    U_Vector = ray.direction;
    S_Vector = ray.endpoint - center;
    
    Delta = pow(2 * Call.Dot_Product(U_Vector, S_Vector),2) 
    - 4 * Call.Dot_Product(U_Vector, U_Vector) * (Call.Dot_Product(S_Vector, S_Vector) - pow(radius,2));
    
    if(Delta < 0 )
    {
        return false;
    }

    else
    {
        
        double temp1 =(-2 * Call.Dot_Product(U_Vector, S_Vector) + sqrt(Delta) ) / (2.0 * Call.Dot_Product(U_Vector, U_Vector));
        double temp2 = (-2 * Call.Dot_Product(U_Vector, S_Vector) - sqrt(Delta) ) / (2.0 * Call.Dot_Product(U_Vector, U_Vector));
        

        if( temp1 >= 0 && temp2 < 0 ) 
        {
            Check = temp1;
        }

        else if( temp2 >= 0 && temp1 < 0 ) 
        {
            Check = temp2;
        }

        else if( temp1 >= 0 && temp2 >= 0 ) 
        {
            Check = min(temp1, temp2);
        }

        else
        {
            return false;
        }
    
        if( ray.semi_infinite == true && Check > small_t )
        {
            ray.current_object = this;
            ray.t_max = Check;
            ray.semi_infinite = false;

            return true;
        }
        else if(ray.semi_infinite == false && Check < ray.t_max  && Check > small_t)
        {
            ray.current_object = this;
            ray.t_max = Check;
            return true;
        }

        return false;
    }
}

Vector_3D<double> Sphere::
Normal(const Vector_3D<double>& location) const
{
    Vector_3D<double> normal;
    // TODO: set the normal
    normal = location - center;
    return normal.Normalized();
}

// determine if the ray intersects with the sphere
// if there is an intersection, set t_max, current_object, and semi_infinite as appropriate and return true
bool Plane::
Intersection(Ray& ray) const
{
    // TODO
    Vector_3D<double> Call;
    double dot = Call.Dot_Product(normal, ray.direction);
    double temp = 0;
    if(dot < 0)
    {
        temp = dot*-1;
    }
    else
    {
        temp = dot;
    }
    
    if(temp > small_t)
    {
        double t = Call.Dot_Product(x1 - ray.endpoint, normal);
        t /= dot;
        
        if(t >= 0)
        {
            if(ray.semi_infinite)
            {
                ray.current_object = this;
                ray.t_max = t;
                ray.semi_infinite = false;
            }

            return true;
        }
        
    }
    return false;
}

Vector_3D<double> Plane::
Normal(const Vector_3D<double>& location) const
{
    return normal;
}
//--------------------------------------------------------------------------------
// Camera
//--------------------------------------------------------------------------------
// Find the world position of the input pixel
Vector_3D<double> Camera::
World_Position(const Vector_2D<int>& pixel_index)
{
    Vector_3D<double> result;
    Vector_2D<double> increment = film.pixel_grid.X(pixel_index);
    // TODO 
    
    result = focal_point + horizontal_vector*increment.x + vertical_vector*increment.y;
    return result;
}
//--------------------------------------------------------------------------------
// Render_World
//--------------------------------------------------------------------------------
// Find the closest object of intersection and return a pointer to it
//   if the ray intersects with an object, then ray.t_max, ray.current_object, and ray.semi_infinite will be set appropriately
//   if there is no intersection do not modify the ray and return 0
const Object* Render_World::
Closest_Intersection(Ray& ray)
{
    // TODO
    Vector_3D<double> Saved;
    bool check = true;
    int save = -1;
    double max = 1000000;
    
    for(unsigned int i = 0; i < objects.size(); i++)
    {
        if(objects.at(i)->Intersection(ray))
        {
            if(ray.t_max < max)
            {
                max = ray.t_max;
                save = i;
            }
        }
    }
    
    //cout << "POS: " << save << endl;
    
    if(ray.current_object != 0 && save != -1)
    {
        return objects.at(save);
    }
    
    return 0;
}

// set up the initial view ray and call 
void Render_World::
Render_Pixel(const Vector_2D<int>& pixel_index)
{
    // TODO
    Ray ray; // TODO: set up the initial view ray here
    
    //something like this
    ray.endpoint = camera.position;
    ray.direction = camera.World_Position(pixel_index)-camera.position;

    //end TODO
    Ray dummy_root;
    Vector_3D<double> color=Cast_Ray(ray,dummy_root);
    camera.film.Set_Pixel(pixel_index,Pixel_Color(color));
}

// cast ray and return the color of the closest intersected surface point, 
// or the background color if there is no object intersection
Vector_3D<double> Render_World::
Cast_Ray(Ray& ray,const Ray& parent_ray)
{
    // TODO
    Vector_3D<double> color;
    const Object* closest_object;
    
    //~ int count = 0;
    
    closest_object = Closest_Intersection(ray);
    
    if(ray.current_object == 0)
    {
        //cout << "Background" << endl;
        color = background_shader->Shade_Surface(parent_ray, *parent_ray.current_object,
        parent_ray.direction, parent_ray.direction);
    }
    else
    {
        Vector_3D<double> intersect = ray.Point(ray.t_max);
        Vector_3D<double> normal = ray.current_object->Normal(intersect);
        //cout << "Color" << endl;
        color = closest_object->material_shader->Shade_Surface(ray, *ray.current_object,
        intersect, normal);
    }

    // determine the color here

    return color;
}
