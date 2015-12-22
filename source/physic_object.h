// File: physic_object.h

#ifndef PHYSIC_OBJECT_H
#define PHYSIC_OBJECT_H

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// abstract class that identifies any existing object, and classes for some kinds of different objects. //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {KIND_BALL, KIND_TEAPOT, KIND_CANNON, KIND_UFO, MAX_SPHERE=9, KIND_WALL=10}; //conventionally (kind<MAX_SPHERE) => spherical_object

#define MIN_OBJ_LENGTH 29 //a regular line in the data file containig an object cannot have less than MIN_OBJ_LENGTH characters
#define GROUND_LEVEL -3   //when drawn objects have to be translated to the ground level
#define BACK_STEP -10     //when drawn objects have to be translated back

#include <string>
#include <sstream> //std::stringstream()

#include <GL/glut.h>

#include "my3Dvector.h"


class physic_object
{
  public:
    physic_object(double m, my3Dvector& pos);
    physic_object(std::string&);
    virtual ~physic_object() {};
    
    int get_kind() const {return kind;};
    double get_mass() const {return mass;};
    
    my3Dvector position;
    my3Dvector velocity;
    float charge;
    bool anelastic_collision, fixed, ghost; //fixed objects don't feel forces, ghost objects don't collide with anything
    
    //drawing stuff (OpenGl)
    float color[3];
    void draw_object_init(const my3Dvector&, const float*) const;
    virtual void draw() const =0;
    
  private:
    int kind;
    double mass;
};

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//abstract class of (at first order) spherically symmetric objects
class spherical_object : public physic_object
{
  public:
    spherical_object (std::string& info);
    virtual ~spherical_object() {};
    
    double get_radius() const {return radius;};
    virtual void draw() const =0;
    
  private:
    double radius;
};

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//0 = KIND_BALL
class obj_ball : public spherical_object
{
  public:
    obj_ball (std::string& info) : spherical_object(info) {};
    ~obj_ball() {};
    
    void draw() const;
};

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//1 = KIND_TEAPOT
class obj_teapot : public spherical_object
{
  public:
    obj_teapot (std::string& info) : spherical_object(info) {};
    ~obj_teapot() {};
    
    void draw() const;
};

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//2 = KIND_CANNON
class obj_cannon : public spherical_object
{
  public:
    obj_cannon (std::string& info);
    ~obj_cannon() {};
    
    double barrel_azimuth;
    double barrel_elevation;    
    
    void draw() const;
};

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//3 = KIND_UFO

class obj_ufo : public spherical_object
{
   public:
    obj_ufo (std::string& info) : spherical_object(info) {};
    ~obj_ufo() {};
    
    void draw() const;
};

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//4 = KIND_WALL a wall is a fixed 2D rectangle
class obj_wall : public physic_object
{
  public:
    obj_wall (std::string& info);
    ~obj_wall() {};
    
    my3Dvector position1; //caution! the 4 points have to be the vertexes of a rectangle.
    my3Dvector position2;
    my3Dvector position3;    
    my3Dvector get_normal_versor() const;
    
    void draw() const;
};

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-

//some useful external functions
bool in_touch(const physic_object*, const physic_object*);
bool in_touch(const spherical_object*, const spherical_object*);
bool in_touch(const spherical_object*, const obj_wall*);
bool in_touch(const obj_wall* wa, const spherical_object* sph);

#endif

