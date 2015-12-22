// File: physic_object.cpp

#include <iostream>
#include "physic_object.h"

physic_object::physic_object (double m, my3Dvector &pos)
{
  mass=m;
  position=pos;

  //default values
  velocity=0;
  charge=0;
  anelastic_collision=false;
  fixed=false; 
  ghost=false;
  color[0]=0;
  color[2]=0;
  color[1]=0;
  kind=0;
}

physic_object::physic_object (std::string& info)
{
  if (info[0]=='#' || info.length()<MIN_OBJ_LENGTH)
  {
    std::cerr<<"\t--- ERROR LOADING OBJECT: "<<info<<" ---"<<std::endl;
    mass=1e-7;
    position=0;
    velocity=0;
    charge=0;
    anelastic_collision=false;
    fixed=true;
    ghost=true;
    for(int i=0; i<3;i++)
      color[i]=0;
    kind=0;
  }
  else
  {
   std::stringstream (info)>>kind>>mass>>position>>velocity>>charge>>anelastic_collision>>fixed>>ghost>>color[0]>>color[1]>>color[2];
   for(int i=0; i<3;i++)
    color[i]/=255.;
  }				
}

void physic_object::draw_object_init(const my3Dvector& pos, const float* col) const
{
  glLoadIdentity();
  glTranslatef(pos.x, pos.y+GROUND_LEVEL, pos.z+BACK_STEP);
  glNewList(0, GL_COMPILE);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
   glMaterialfv(GL_FRONT, GL_SPECULAR, col);
   glMaterialf(GL_FRONT, GL_SHININESS, 0.5);
  glEndList();
}

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//external functions:
bool in_touch(const physic_object* a, const physic_object* b)
{
  if (a->get_kind()<MAX_SPHERE && b->get_kind()<MAX_SPHERE)
  {
    const spherical_object * sph_a = static_cast<const spherical_object*>(a);
    const spherical_object * sph_b = static_cast<const spherical_object*>(b);
    return in_touch(sph_a, sph_b);
  }
  if (a->get_kind()==KIND_WALL && b->get_kind()<MAX_SPHERE)
  {  
    const obj_wall * wa = static_cast<const obj_wall*>(a);
    const spherical_object * sph = static_cast<const spherical_object*>(b);
    return (in_touch(sph, wa));
  }
  if (b->get_kind()==KIND_WALL && a->get_kind()<MAX_SPHERE)
  {
    const spherical_object * sph = static_cast<const spherical_object*>(a);
    const obj_wall * wa = static_cast<const obj_wall*>(b);
    return (in_touch(sph, wa));
  }
  if (a->get_kind()==KIND_WALL && b->get_kind()==KIND_WALL) //walls don't collide among them, as if they never touch.
    return false;
  
  std::cerr<<"\t----  WARNING!! something wrong with \"in_touch\" function! ----"<<std::endl;
  return false;
}

bool in_touch(const spherical_object* a, const spherical_object* b)
{
  if((a->position-b->position).norm()<a->get_radius()+b->get_radius())
    return true;
  else
    return false;
}

bool in_touch(const obj_wall* wa, const spherical_object* sph) {return in_touch (sph, wa);}

bool in_touch(const spherical_object* point, const obj_wall* plane)
{
  bool check_on_normal=std::abs((point->position-plane->position)*plane->get_normal_versor())<(point->get_radius());
  //CAUTION: what follows works well only for rectangles...
  bool check_in_rectangle1=((point->position-plane->position)*(plane->position3-plane->position))>0;
  bool check_in_rectangle2=((point->position-plane->position)*(plane->position1-plane->position))>0;
  bool check_in_rectangle3=((point->position-plane->position2)*(plane->position3-plane->position2))>0;
  bool check_in_rectangle4=((point->position-plane->position2)*(plane->position1-plane->position2))>0;
  
  if(check_on_normal && check_in_rectangle1 && check_in_rectangle2 && check_in_rectangle3 && check_in_rectangle4)
    return true;
  else
    return false;
}

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-

spherical_object::spherical_object(std::string& info) : physic_object(info)
{
  std::stringstream ss(info);
  double appo; //simple expedient to skip the alredy used part of the string
  for(int i=0; i<MIN_OBJ_LENGTH/2+1; i++)
    ss>>appo;
  ss >> radius;
}

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//0 = KIND_BALL
void obj_ball::draw() const
{
  if (get_radius()>1e3)//avoid drawing too big balls
    return;
  glPushMatrix();
  draw_object_init(position, color);
  glutSolidSphere(get_radius(),40,40);
  glPopMatrix();
  
}

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//1 = KIND_TEAPOT
void obj_teapot::draw() const
{
  glPushMatrix();
  draw_object_init(position, color);
   glCullFace(GL_FRONT);
  glutSolidTeapot(get_radius());
   glCullFace(GL_BACK);
  glPopMatrix();
}

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//2 = KIND_CANNON
obj_cannon::obj_cannon(std::string& info) : spherical_object(info)
{
  std::stringstream ss(info);
  double appo; //simple expedient to skip the alredy used part of the string
  for(int i=0; i<MIN_OBJ_LENGTH/2+2; i++)
    ss>>appo;
  
  ss >> barrel_azimuth >> barrel_elevation;
  
  barrel_azimuth=M_PI/180.*barrel_azimuth; //angles are in degrees in the input file but in radiants here
  barrel_elevation=M_PI/180.*barrel_elevation;
}

void obj_cannon::draw() const
{
  float grey[]={50./255.,50./255.,50./255.};
  
  glPushMatrix();
  draw_object_init(position, grey);
   glRotatef(barrel_azimuth*180/M_PI, 0,1.,0); //CAUTION: here azimuth and elevation are referred to the window coordinate system,
   glRotatef(barrel_elevation*180/M_PI, 0,0,1);//         so is better to avoid fuctions like set.azimuth() from my3Dvector
   glRotatef(90, 0,1,0);
  glDisable(GL_CULL_FACE);
  glBegin(GL_POLYGON);
   GLUquadricObj *quadObj = gluNewQuadric();
   gluCylinder(quadObj, get_radius()*2./3.,get_radius()/3., get_radius()*2, 30, 30);
  glEnd();
  glEnable(GL_CULL_FACE);
  glPopMatrix();
  
  glPushMatrix();
  draw_object_init(position, color);
  glutSolidSphere(get_radius(),40,40);
  glPopMatrix();
}

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//3 = KIND_UFO
void obj_ufo::draw() const
{
  glPushMatrix();
  draw_object_init(position, color);
  glRotatef(90, 1,0,0);
  glutSolidTorus(get_radius()*0.3, get_radius(), 40, 40);
  glPopMatrix();
  
  float blue[] = {0.0, 0.0, 1.0};
  my3Dvector a_little_upper(0, get_radius()/5. ,0);
  a_little_upper+=position;
  
  glPushMatrix();
  draw_object_init(a_little_upper, blue);
  glutSolidSphere(get_radius()*0.7,40,40);
  glPopMatrix();
}

//^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-^-_-
//4 = KIND_WALL
obj_wall::obj_wall(std::string& info) : physic_object(info)
{
  std::stringstream ss(info);
  double appo; //simple expedient to skip the alredy used part of the string
  for(int i=0; i<MIN_OBJ_LENGTH/2+1; i++)
    ss>>appo;
  
  ss >> position1 >> position2 >> position3;
  fixed=true; //walls cannot move!
  velocity=0;
}

my3Dvector obj_wall::get_normal_versor() const
{
  my3Dvector normal_vector=((position2-position1)^(position1-position3));
  return normal_vector/normal_vector.norm();
}

void obj_wall::draw() const //FIXME: Walls are always black
{
  my3Dvector zero(0,0,0);
  
  glPushMatrix();
  glDisable(GL_CULL_FACE);  
  draw_object_init(zero, color);
  glBegin(GL_QUADS);
   glVertex3f(position.x, position.y, position.z);
   glVertex3f(position1.x, position1.y, position1.z);
   glVertex3f(position2.x, position2.y, position2.z);
   glVertex3f(position3.x, position3.y, position3.z);
  glEnd();
  glEnable(GL_CULL_FACE);
  glPopMatrix();
}

