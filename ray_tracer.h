#ifndef __RAY_TRACER_H__
#define __RAY_TRACER_H__

#include <math.h>
#include <vector>
#include <iostream>
#include <limits>

typedef unsigned int Pixel;

#define PI 3.14159265358979323846264338327950288419
#define FLT_MAX numeric_limits<float>::max()
//--------------------------------------------------------------------------------
// declarations
//--------------------------------------------------------------------------------
class Object;
class Box;
class Sphere;
class Plane;
class Render_World;
void Initialize_World(Render_World& world,const int,const int,const int test_number);
//--------------------------------------------------------------------------------
// utility functions
//--------------------------------------------------------------------------------
double sqr(const double x);
//--------------------------------------------------------------------------------
// Vector_2D
//--------------------------------------------------------------------------------
template<class T>
class Vector_2D
{
public:
    T x,y;

    Vector_2D()
        :x(0),y(0)
    {}

    Vector_2D(T x_input,T y_input)
        :x(x_input),y(y_input)
    {}

    static T Dot_Product(const Vector_2D& vector1,const Vector_2D& vector2)
    {
        return vector1.x*vector2.x+vector1.y*vector2.y;
    }
    
    double Length() const
    {
        T length_squared=Dot_Product(*this,*this);
        return sqrt((double)length_squared);
    }

    void Normalize()
    {
        double one_over_length=(double)1/Length();
        x=(T)(x*one_over_length);
        y=(T)(y*one_over_length);
    }

};
//--------------------------------------------------------------------------------
// Vector_3D
//--------------------------------------------------------------------------------
template<class T>
class Vector_3D
{
public:
    T x,y,z;
    
    Vector_3D()
        :x(0),y(0),z(0)
    {}

    Vector_3D(T x_input,T y_input,T z_input)
        :x(x_input),y(y_input),z(z_input)
    {}

    Vector_3D(const Vector_3D& vector)
        :x(vector.x),y(vector.y),z(vector.z)
    {}

    static T Dot_Product(const Vector_3D& vector1,const Vector_3D& vector2)
    {
        return vector1.x*vector2.x+vector1.y*vector2.y+vector1.z*vector2.z;
    }
    
    double Length_Squared() const
    {
        return Dot_Product(*this,*this);
    }

    double Length() const
    {
        return sqrt(Length_Squared());
    }

    void Normalize()
    {
        double one_over_length=(double)1/Length();
        x=(T)(x*one_over_length);
        y=(T)(y*one_over_length);
        z=(T)(z*one_over_length);
    }

    Vector_3D Normalized() const
    {
        Vector_3D result(*this);
        result.Normalize();
        return result;
    }

    static Vector_3D Cross_Product(const Vector_3D& v1,const Vector_3D& v2) // 6 mults, 3 adds
    {
        return Vector_3D(v1.y*v2.z-v1.z*v2.y,v1.z*v2.x-v1.x*v2.z,v1.x*v2.y-v1.y*v2.x);
    }


    Vector_3D operator-(const Vector_3D& vector) const
    {
        Vector_3D result;
        result.x=x-vector.x;
        result.y=y-vector.y;
        result.z=z-vector.z;
        return result;
    }

    Vector_3D operator+(const Vector_3D& vector) const
    {
        Vector_3D result;
        result.x=x+vector.x;
        result.y=y+vector.y;
        result.z=z+vector.z;
        return result;
    }

    Vector_3D operator*(double s) const
    {
        Vector_3D result;
        result.x=s*x;
        result.y=s*y;
        result.z=s*z;
        return result;
    }

    Vector_3D operator*(const Vector_3D& vector) const
    {
        Vector_3D result;
        result.x=vector.x*x;
        result.y=vector.y*y;
        result.z=vector.z*z;
        return result;
    }

    Vector_3D& operator+=(const Vector_3D& vector)
    {
        x+=vector.x;
        y+=vector.y;
        z+=vector.z;
        return *this;
    }


};

template<class T>
std::ostream& operator<<(std::ostream& os,const Vector_3D<T>& vector)
{
    os<<"("<<vector.x<<","<<vector.y<<","<<vector.z<<")";
    return os;
}

//--------------------------------------------------------------------------------
// Grid_2D
//--------------------------------------------------------------------------------
class Grid_2D
{
public:
    int m,n; // # of points: x and y direction
    double xmin,xmax,ymin,ymax; // left and right wall, bottom and top wall
    double dx,dy;

    Grid_2D() 
    {
        Initialize(0,0,0,0,0,0);
    }

    Grid_2D(int m_input,int n_input,double xmin_input,double xmax_input,double ymin_input,double ymax_input)
    {
        Initialize(m_input,n_input,xmin_input,xmax_input,ymin_input,ymax_input);
    }

    void Initialize(const int m_input,const int n_input,
        const double xmin_input,const double xmax_input,
        const double ymin_input,const double ymax_input)
    {
        m=m_input;
        n=n_input;
        xmin=xmin_input;
        xmax=xmax_input;
        ymin=ymin_input;
        ymax=ymax_input;
        if(m) dx=(xmax-xmin)/m; else dx=0;
        if(n) dy=(ymax-ymin)/n; else dy=0;
    }

    Vector_2D<double> X(const Vector_2D<int>& index) const
    {
        return X(index.x,index.y);
    }

    Vector_2D<double> X(const int i,const int j) const
    {
        return Vector_2D<double>(xmin+(i+.5)*dx,ymin+(j+.5)*dy);
    }

};

//--------------------------------------------------------------------------------
// Ray
//--------------------------------------------------------------------------------
class Ray
{
public:
    Vector_3D<double> endpoint; // endpoint of the ray where t=0
    Vector_3D<double> direction; // direction the ray sweeps out - unit vector
    bool semi_infinite;  // indicates whether the ray is semi_infinite or should stop at t_max
    double t_max; // maximum value of t allowed for the ray
    const Object* current_object; // used to store a pointer to the object of intersection
    int recursion_depth;

    Ray()
        :endpoint(0,0,0),direction(0,0,1),semi_infinite(true),t_max(0),current_object(0),recursion_depth(0)
    {}

    Ray(const Vector_3D<double>& endpoint_input,const Vector_3D<double>& direction_input)
        :endpoint(endpoint_input),direction(direction_input.Normalized()),semi_infinite(true),t_max(0),current_object(0),recursion_depth(0)
    {}

    Vector_3D<double> Point(const double t) const
    {
        return endpoint+direction*t;
    }
};

//--------------------------------------------------------------------------------
// Light
//--------------------------------------------------------------------------------
class Light
{
public:
    Vector_3D<double> position;
    Vector_3D<double> color; // RGB color components
    double brightness;

    Light()
        :position(),color(1,1,1),brightness(1)
    {}

    Light(const Vector_3D<double>& position,const Vector_3D<double>& color,const double brightness)
        :position(position),color(color),brightness(brightness)
    {}

    virtual Vector_3D<double> Emitted_Light(const Ray& ray) const=0;
};
//--------------------------------------------------------------------------------
// Point_Light
//--------------------------------------------------------------------------------
class Point_Light : public Light
{
public:
    Point_Light(const Vector_3D<double>& position,const Vector_3D<double>& color,const double brightness)
        :Light(position,color,brightness)
    {}

    Vector_3D<double> Emitted_Light(const Ray& ray) const
    {
        return color*brightness;
    }
};

//--------------------------------------------------------------------------------
// Shader
//--------------------------------------------------------------------------------
class Shader
{
public:
    Render_World& world;

    Shader(Render_World& world_input)
        :world(world_input)
    {}

    virtual Vector_3D<double> Shade_Surface(const Ray& ray,const Object& intersection_object,const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const=0;

};

//--------------------------------------------------------------------------------
// Phong_Shader
//--------------------------------------------------------------------------------
class Phong_Shader : public Shader
{
public:
    Vector_3D<double> color_ambient,color_diffuse,color_specular;
    double specular_power;

    Phong_Shader(Render_World& world_input,
        const Vector_3D<double>& color_ambient,const Vector_3D<double>& color_diffuse,
        const Vector_3D<double>& color_specular=Vector_3D<double>(1,1,1),const double specular_power=50)
        :Shader(world_input),color_ambient(color_ambient),color_diffuse(color_diffuse),color_specular(color_specular),specular_power(specular_power)
    {}

    Vector_3D<double> Shade_Surface(const Ray& ray,const Object& intersection_object,const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const;
};

//--------------------------------------------------------------------------------
// Flat_Shader
//--------------------------------------------------------------------------------
class Flat_Shader : public Shader
{
public:
    Vector_3D<double> color;

    Flat_Shader(Render_World& world_input,const Vector_3D<double>& color=Vector_3D<double>(1,1,1))
        :Shader(world_input),color(color)
    {}

    Vector_3D<double> Shade_Surface(const Ray& ray,const Object& intersection_object,const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const;
};

//--------------------------------------------------------------------------------
// Reflective_Shader
//--------------------------------------------------------------------------------
class Reflective_Shader : public Phong_Shader
{
public:
    double reflectivity;

    Reflective_Shader(Render_World& world_input,
        const Vector_3D<double>& color_ambient,
        const Vector_3D<double>& color_diffuse,
        const Vector_3D<double>& color_specular=Vector_3D<double>(1,1,1),const double specular_power=50,
        const double reflectivity=1)
        :Phong_Shader(world_input,color_ambient,color_diffuse,color_specular,specular_power),reflectivity(std::min(1.0,reflectivity))
    {}

     Vector_3D<double> Shade_Surface(const Ray& ray,const Object& intersection_object,const Vector_3D<double>& intersection_point,const Vector_3D<double>& same_side_normal) const;
};

//--------------------------------------------------------------------------------
// Objects
//--------------------------------------------------------------------------------
class Object
{
public:
    Shader* material_shader;
    static const double small_t; // t has to be bigger than small_t to register an intersection with a ray

    Object()
        :material_shader(0)
    {
    }

    virtual bool Intersection(Ray& ray) const=0; // check for an intersection in the range [0,t_max] or [0,inf] if
                                         // ray.semi_inifite==true.  Update t_max if an intersection is found, and
                                         // update semi_infinite variable.  set ray.current_object to this if found
    virtual Vector_3D<double> Normal(const Vector_3D<double>& point) const=0;
};

//--------------------------------------------------------------------------------
// Sphere
//--------------------------------------------------------------------------------
class Sphere : public Object
{
    Vector_3D<double> center;
    double radius;

public:
    Sphere(const Vector_3D<double>& center_input,const double radius_input)
        :center(center_input),radius(radius_input)
    {}

    bool Intersection(Ray& ray) const;
    Vector_3D<double> Normal(const Vector_3D<double>& point) const;
};

//--------------------------------------------------------------------------------
// Plane
//--------------------------------------------------------------------------------
class Plane : public Object
{
public:
    typedef double T;
    typedef Vector_3D<T> TV;

    TV x1;
    TV normal;

    Plane(const Vector_3D<double>& point,const Vector_3D<double>& normal)
        :x1(point),normal(normal.Normalized())
    {}

    bool Intersection(Ray& ray) const;
    Vector_3D<double> Normal(const Vector_3D<double>& point) const;
};
//--------------------------------------------------------------------------------
// Film
//--------------------------------------------------------------------------------
class Film
{
public:
    Pixel* colors;
    double width,height;
    Grid_2D pixel_grid;

    Film()
        :colors(0),width(0),height(0),pixel_grid()
    {
    }

    ~Film()
    {
        delete[] colors;
    }

    void Set_Resolution(const int pixels_width,const int pixels_height)
    {
        if(colors) delete[] colors;
        colors=new Pixel[pixels_width*pixels_height];
        pixel_grid.Initialize(pixels_width,pixels_height,-width*.5,width*.5,-height*.5,height*.5);
    }

    void Set_Pixel(const Vector_2D<int>& pixel_index,const Pixel& color)
    {
        int i=pixel_index.x;
        int j=pixel_index.y;
        colors[j*pixel_grid.m+i]=color;
    }

};
//--------------------------------------------------------------------------------
// Camera
//--------------------------------------------------------------------------------
class Camera
{
public:
    typedef Vector_3D<double> TV;

    TV position; // camera position 
    TV focal_point; // where the image plane is located
    TV look_vector; // points from the position to the focal point - normalized
    TV vertical_vector; // point up in the image plane - normalized
    TV horizontal_vector; // points to the right on the omage plane - normalized
    Film film;

    Camera()
        :position(0,0,-1),focal_point(0,0,0),vertical_vector(0,1,0)
    {}

    void Position_And_Aim_Camera(const TV& position_input,const TV& look_at_point,const TV& pseudo_up_vector)
    {
        position=position_input;
        look_vector=(look_at_point-position).Normalized();
        horizontal_vector=TV::Cross_Product(look_vector,pseudo_up_vector).Normalized();
        vertical_vector=TV::Cross_Product(horizontal_vector,look_vector).Normalized();
    }

    void Focus_Camera(const double focal_distance,const double aspect_ratio,const double field_of_view)
    {
        focal_point=position+look_vector*focal_distance;
        film.width=(double)2*focal_distance*tan((double).5*field_of_view);
        film.height=film.width/aspect_ratio;
    }

    Vector_3D<double> World_Position(const Vector_2D<int>& pixel_index);
};
//--------------------------------------------------------------------------------
// Render_World
//--------------------------------------------------------------------------------
class Render_World
{
public:
    Shader *background_shader;
    std::vector<Object*> objects;
    std::vector<Light*> lights;
    Camera camera;
    bool enable_shadows;
    int recursion_depth_limit;

    Render_World()
        :background_shader(new Flat_Shader(*this,Vector_3D<double>())),enable_shadows(true),recursion_depth_limit(3)
    {}

    ~Render_World()
    {
        delete background_shader;
        for(unsigned int i=0;i<objects.size();i++) delete objects[i];
        for(unsigned int i=0;i<lights.size();i++) delete lights[i];
    }

    void Render_Pixel(const Vector_2D<int>& pixel_index);
    Vector_3D<double> Cast_Ray(Ray& ray,const Ray& parent_ray);
    const Object* Closest_Intersection(Ray& ray);
};
//--------------------------------------------------------------------------------
// Driver
//--------------------------------------------------------------------------------
class Driver
{
public:
    Render_World& world;
    int state_j; // current rendering row

    Driver(Render_World& world_input)
        :world(world_input),state_j(0)
    {}

    int Pixel_Width() const
    {
        return world.camera.film.pixel_grid.m;
    }

    int Pixel_Height() const
    {
        return world.camera.film.pixel_grid.n;
    }

    void Render_More()
    {
        if(state_j>=Pixel_Height()) return;

        for(int i=0;i<Pixel_Width();i++){
            Vector_2D<int> pixel_index(i,state_j);
            world.Render_Pixel(pixel_index);}

        state_j++;
    }
};
//--------------------------------------------------------------------------------
#endif
