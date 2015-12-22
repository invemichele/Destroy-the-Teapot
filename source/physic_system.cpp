// File: physic_system.cpp

#include <iostream>
#include "physic_system.h"
#include <cmath>

physic_system::physic_system(std::vector<physic_object*> &all_obj)
{
  all_objects=all_obj;
  ext_field_on=false;
  
  //initializations...
  sys_size=all_objects.size();
  
  forces_table.resize(sys_size);
  for (unsigned int i=0; i<sys_size; i++)
    forces_table[i].resize(sys_size);
  
  for (unsigned int i=0; i<5; i++)
    rk_coeff[i].resize(2*sys_size);
}

physic_system::physic_system(std::string file_name)
{
 //open the file
  std::ifstream level_file;
  level_file.open(file_name.c_str());
  while (!level_file.is_open())
  {
    std::cerr<<"\t ----- ERROR: could not open "<<file_name<<" -----"<<std::endl;
    std::cerr<<"\t    Please insert a new file name: ";
    std::cin>>file_name;
    level_file.open(file_name.c_str());
  }
  
 //read it
  std::string line;
  while (!level_file.eof())
  {
    std::getline(level_file, line);
    if (line.length()==1 && !(line[0]=='#'))
      std::stringstream(line)>>world_type;
    if (line.length()<MIN_FIELD_LENGTH || line[0]=='#')
      continue;
    std::stringstream ss(line);
    ss>>gravity_field>>electric_field>>magnetic_field;
    break;
  }
  if (gravity_field.norm()!=0 || electric_field.norm()!=0 || magnetic_field.norm()!=0)
    ext_field_on=true;
  else
    ext_field_on=false;
 
  while (!level_file.eof())
  {
    std::getline(level_file, line);
    if (line[0]=='#' || line.length()<MIN_OBJ_LENGTH)
      continue;
    physic_object *new_object;
    int kind;
    std::stringstream(line)>>kind;
    switch (kind)
    {
      case KIND_TEAPOT:
        new_object=new obj_teapot(line);
        break;
      case KIND_CANNON:
        new_object=new obj_cannon(line);
        break;
      case KIND_UFO:
        new_object=new obj_ufo(line);
        break;
      case KIND_WALL:
        new_object=new obj_wall(line);
        break;
      default:
        new_object=new obj_ball(line);
    }
    all_objects.push_back(new_object);
  }
  level_file.close();

 //initializations...
  sys_size=all_objects.size();
  
  forces_table.resize(sys_size);
  for (unsigned int i=0; i<sys_size; i++)
    forces_table[i].resize(sys_size);
  
  for (unsigned int i=0; i<5; i++)
    rk_coeff[i].resize(2*sys_size);
}

physic_system::~physic_system()
{
  for (unsigned int i=0; i<all_objects.size(); i++)
     delete (all_objects[i]);
}


void physic_system::add_object(physic_object* new_object)
{
  all_objects.push_back(new_object);
  
  sys_size=all_objects.size();
  forces_table.resize(sys_size);
  for (unsigned int i=0; i<sys_size; i++)
    forces_table[i].resize(sys_size);
  for (unsigned int i=0; i<5; i++)
    rk_coeff[i].resize(2*sys_size);
}

my3Dvector physic_system::force(const physic_object* generating,const physic_object* feeling,const my3Dvector& gen,const my3Dvector& feel)
{
  my3Dvector r=feeling->position+feel-generating->position-gen;
  return r*((G_CONST*feeling->get_mass()*generating->get_mass()+K_CONST*feeling->charge*generating->charge)/pow(r.norm(), 3));
}

void physic_system::rungekutta_step(std::vector<my3Dvector>& previous, std::vector<my3Dvector>& next, double h)
{
  //1.calculate the forces table
  for (unsigned int i=0; i<sys_size; i++)
  {
    for(unsigned int j=i+1; j<sys_size; j++) //i<j
    {
      forces_table[i][j]=force(all_objects[i], all_objects[j], previous[i]*h, previous[j]*h);
      forces_table[j][i]=(-1)*forces_table[i][j];
    }
  }
  
  //2.calculate the net_force
  std::vector<my3Dvector> net_force(sys_size);
  for (unsigned int i=0; i<sys_size; i++)
  {
    if (all_objects[i]->fixed) //fixed objects don't move
      continue;
    for (unsigned int j=0; j<sys_size; j++)
      net_force[i]+=forces_table[j][i];
    if (ext_field_on)
      net_force[i]+=(gravity_field*all_objects[i]->get_mass())+(electric_field*all_objects[i]->charge)+all_objects[i]->charge*((all_objects[i]->velocity+previous[sys_size+i]*h)^magnetic_field);
   }

  //3.calculate the next runge-kutta coefficients
  for (unsigned int i=0; i<sys_size; i++)
  {
   next[sys_size+i]=net_force[i]/all_objects[i]->get_mass();
   next[i]=all_objects[i]->velocity+previous[sys_size+i]*h;
  }
}

void physic_system::evolve (double h)
{
  rungekutta_step(rk_coeff[0], rk_coeff[1], 0);
  rungekutta_step(rk_coeff[1], rk_coeff[2], h/2.);
  rungekutta_step(rk_coeff[2], rk_coeff[3], h/2.);
  rungekutta_step(rk_coeff[3], rk_coeff[4], h);
  
  for (unsigned int i=0; i<sys_size; i++) 
  { 
    if (all_objects[i]->fixed) //fixed objects don't move
      continue;
    all_objects[i]->velocity+=h/6*(rk_coeff[1][sys_size+i]+rk_coeff[2][sys_size+i]*2.+rk_coeff[3][sys_size+i]*2+rk_coeff[4][sys_size+i]);
    all_objects[i]->position+=h/6*(rk_coeff[1][i]+rk_coeff[2][i]*2.+rk_coeff[3][i]*2+rk_coeff[4][i]);
  }
  
  //check and make collisions happen
  for (unsigned int i=0; i<sys_size; i++)
  {
    for(unsigned int j=i+1; j<sys_size; j++) //i<j
    {
      if (in_touch(all_objects[i], all_objects[j]))
        collision(all_objects[i], all_objects[j]);
    } 
  }
}

void physic_system::collision(physic_object* a, physic_object* b)
{
  if (a->ghost || b->ghost) //ghost objects don't collide
    return;
  
  if (a->get_kind()<MAX_SPHERE && b->get_kind()<MAX_SPHERE)
  {
    spherical_object * sph_a = static_cast<spherical_object*>(a);
    spherical_object * sph_b = static_cast<spherical_object*>(b);
    collision(sph_a, sph_b);
    return;
  }
  if (a->get_kind()==KIND_WALL && b->get_kind()<MAX_SPHERE)
  {  
    obj_wall * wa = static_cast<obj_wall*>(a);
    spherical_object * sph = static_cast<spherical_object*>(b);
    (collision(sph, wa));
    return;
  }
  if (b->get_kind()==KIND_WALL && a->get_kind()<MAX_SPHERE)
  {
    spherical_object * sph = static_cast<spherical_object*>(a);
    obj_wall * wa = static_cast<obj_wall*>(b);
    (collision(sph, wa));
    return;
  }
  else
    std::cerr<<"\t----  WARNING!! something wrong with \"collision\" function! ----"<<std::endl;
}

void physic_system::collision(spherical_object* a, spherical_object* b)
{
  if (a->ghost || b->ghost) //ghost objects don't collide
    return;
  
  my3Dvector Va, Vb, direction;
  direction=(a->position-b->position)/(a->position-b->position).norm(); //this is meant expecially for the collision of two spheres
  Va=(a->velocity*direction)*direction;
  Vb=(b->velocity*direction)*direction;

  if (a->anelastic_collision || b->anelastic_collision)
  {
    a->velocity=(a->velocity-Va)+(a->get_mass()*Va+b->get_mass()*Vb)/(a->get_mass()+b->get_mass());
    b->velocity=(b->velocity-Vb)+(a->get_mass()*Va+b->get_mass()*Vb)/(a->get_mass()+b->get_mass());
    if (a->anelastic_collision && b->anelastic_collision) //just to avoid complete merging of the two objects
    {
      if (a->get_radius()<=b->get_radius())
        b->anelastic_collision=false;
      else
        a->anelastic_collision=false;
    }
    if (a->anelastic_collision)
      a->position+=a->get_radius()/V_BIG*(Va-Vb);
    else
      b->position+=b->get_radius()/V_BIG*(Vb-Va);
    
    a->anelastic_collision=false; //each spherical_object can collide anelastically only once !!
    b->anelastic_collision=false;
  }
  else
  {
    a->velocity=(a->velocity-Va)+(Va*(a->get_mass()-b->get_mass())+2*b->get_mass()*Vb)/(a->get_mass()+b->get_mass());
    b->velocity=(b->velocity-Vb)+(Vb*(b->get_mass()-a->get_mass())+2*a->get_mass()*Va)/(a->get_mass()+b->get_mass());
  }
}

void physic_system::collision(spherical_object* point, obj_wall* plane)
{
  if (point->ghost || plane->ghost) //ghost objects don't collide
    return;
  
  if (point->anelastic_collision || plane->anelastic_collision)
  {
    if (!point->fixed) //a sphere that collides anelastically with a wall gets stuck in it
    {
      point->position+=point->get_radius()/V_BIG*(point->velocity);
      point->velocity=0;
      point->fixed=true;
    }
  }
  else
  {
    point->velocity-=2*(point->velocity*plane->get_normal_versor())*plane->get_normal_versor();
  }
}  

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//drawing stuff

GLfloat white_light[] = {1.0, 1.0, 1.0, 1.0};
GLfloat blu_light[] = {0.0,0.1,1.0, 0.3};
GLfloat green_light[] = {0.0, 1.0, 0.0, 0.3};
GLfloat yellow_light[] = {1.0, 1.0, 0.0, 1.0};
GLfloat red_light[] = {1.0, 0.0, 0.0, 1.0};
GLfloat black_light[] = {0.0, 0.0, 0.0, 1.0};
GLfloat blackb_light[] = {0.15, 0.15, 0.15, 1.0};
GLfloat grey_light[]={50./255.,50./255.,50./255.};
GLfloat lab_light[]={0./255.,76./255.,153./255.};

void physic_system::material(int dlist, GLfloat * diffuse, GLfloat * specular, GLfloat shininess)
{
  glNewList(dlist, GL_COMPILE);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
   glMaterialf(GL_FRONT, GL_SHININESS, shininess);
  glEndList();
  glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
}

void physic_system::draw() 
{
  GLfloat dim=100.0; //dimension of the ground/sky rectangle
  
  switch (world_type)
  {
    case WORLD_EARTH:
    {  
      //sky
      glLoadIdentity();
      glTranslatef(0,0,-30);
      glRotatef(180, 1, 0, 0);
      glDisable(GL_CULL_FACE);
      material(0,blu_light,blu_light,1);
      glRectf(-dim,-dim,dim,dim);

      //ground
      glLoadIdentity();
      glTranslatef(0,GROUND_LEVEL,0);
      glRotatef(90, 1, 0, 0);
      material(0,green_light,green_light,0.5);
      glDisable(GL_CULL_FACE);
      glRectf(-dim,-dim,dim,dim); 
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);

      break;
    }
    case WORLD_MOON:
    {
      //ground
      glLoadIdentity();
      glTranslatef(0,GROUND_LEVEL,BACK_STEP+5);
      glRotatef(90, 1, 0, 0);
      material(0,blackb_light,blackb_light,0.5);
      glCullFace(GL_FRONT);
      glRectf(-dim,-dim,dim,dim); 
      glCullFace(GL_BACK);
      //break; //without the break, so it goes ahead and draws also the stars
    }
    case WORLD_SPACE:
    {
      //stars
      int number_stars=1000;
      srand(100);
      for(int i=0; i<number_stars; i++)
      { 
        glLoadIdentity();
        material(0,white_light,white_light,0.5);
        double phi=(rand()*dim*2/RAND_MAX);
        double theta=(rand()*dim*2/RAND_MAX);
        glTranslatef(2*dim*sin(theta)*cos(phi), 2*dim*sin(phi)*sin(theta),2*dim*cos(theta));
        glutSolidSphere((rand()%1000)/5000.,10,10);
      }
      break;
    }
    case WORLD_LAB: //CAUTION: the lateral walls have to be physical_objects
    {
      //Floor
      glLoadIdentity();
      glTranslatef(0,GROUND_LEVEL,BACK_STEP+5);
      glRotatef(90, 1, 0, 0);
      glCullFace(GL_FRONT);
      material(0,grey_light,grey_light,1);
      glRectf(-dim,-dim,dim,dim); 
      glCullFace(GL_BACK);
      
      //Back_Wall
      glLoadIdentity();
      glTranslatef(0,0,-20);
      glRotatef(180, 1, 0, 0);
      glDisable(GL_CULL_FACE);
      material(0,lab_light,lab_light,1);
      glRectf(-dim,-dim,dim,dim);
      
      //Blackboard
      glLoadIdentity();
      glTranslatef(0, GROUND_LEVEL+6, -20);
      glRotatef(180, 1, 0, 0);
      material(0,white_light,white_light,0.5);
      GLdouble x2=6.1;
      GLdouble y2=2.6;
      glRectf(-x2,-y2,x2,y2);
      
      //Blackboard edge
      glLoadIdentity();
      glTranslatef(0, GROUND_LEVEL+6,-20);
      glRotatef(180, 1, 0, 0);
      material(0,black_light,black_light,0.5);
      GLdouble x3=6.0;
      GLdouble y3=2.5;
      glRectf(-x3,-y3,x3,y3); 

      break;
    }
    default:
      std::cerr<<"\t---- Error: no matches for world type, check the data file. ----"<<std::endl;
  }
  
  for (unsigned int i=0; i<sys_size; i++)
    all_objects[i]->draw();
}


