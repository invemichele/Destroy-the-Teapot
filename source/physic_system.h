// File: physic_system.h

#ifndef PHYSIC_SYSTEM_H
#define PHYSIC_SYSTEM_H

////////////////////////////////////////////////////////////////////////////
// class that takes care of all the existing objects, and makes them move //
////////////////////////////////////////////////////////////////////////////

//#define G_CONST 0  //CAUTION!!! zero gravity!
#define G_CONST -6.67428e-11
#define K_CONST 8.98755e9

#define V_BIG 100 //reference speed for anelastic collisions
#define MIN_FIELD_LENGTH 11 //a line in the data file containig the external fields cannot have less than MIN_FIELD_LENGTH characters

enum {WORLD_EARTH, WORLD_MOON, WORLD_SPACE, WORLD_LAB};

#include <vector>
#include <string>
#include <fstream>
#include <sstream> //std::stringstream()
#include <stdlib.h> //srand(), RAND_MAX

#include <GL/glut.h>

#include "my3Dvector.h"
#include "physic_object.h"


class physic_system
{
  public:
    physic_system(std::vector<physic_object*>&);
    physic_system(std::string); //insert the name of the file containing the game level
    ~physic_system();
    
    void evolve(double);
    void add_object(physic_object*);
    
    std::vector<physic_object*> all_objects;
    unsigned int size() const {return sys_size;};
    int get_world() const {return world_type;};
    
    bool ext_field_on;
    my3Dvector gravity_field;
    my3Dvector electric_field;
    my3Dvector magnetic_field;
    
    //drawing stuff (OpenGl)
    void material(int, GLfloat *, GLfloat *, GLfloat);
    void draw();
    
  private:
    int world_type;
    unsigned int sys_size;

    void collision(physic_object*, physic_object*); 
    void collision(spherical_object*, spherical_object*);
    void collision(spherical_object*, obj_wall*);
    void collision(obj_wall* wa, spherical_object* sph) {collision(sph, wa);};

    //methods to implement a 4th order runge-kutta
    std::vector< std::vector<my3Dvector> > forces_table;
    void rungekutta_step(std::vector<my3Dvector>&, std::vector<my3Dvector>&, double);
    my3Dvector force(const physic_object*, const physic_object*, const my3Dvector&, const my3Dvector&);
    std::vector<my3Dvector> rk_coeff[5];
};

#endif

