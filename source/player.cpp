//File: player.cpp

#include <iostream>
#include "player.h"
#include <iomanip> //setprecision

player::player(int type)
{
  player_type=type;
  lives=INITIAL_LIVES;
  winner=false;
  priority=false;
  ufo=NULL;
	
  if (player_type!=NET2)
  {
    std::cout<<"\tPlease enter your name: ";
    std::getline(std::cin,name);
  }

  if (player_type==NET1)
  {
    FILE * script;
    char address [20];
    std::string my_IP;
    script = popen ("./myIPtoplay.sh" , "r");
    if (script == NULL) 
    {
      std::cerr<<"\t--- ERROR: 'popen' failed ---";
      std::cout<<"Please insert your IP address: ";
      std::cin>>my_IP;
    }
    else
    {
      fgets (address , 20 , script);
      pclose (script);   
      my_IP=address;
      my_IP.erase(my_IP.end()-1); //throw the final '\n'
    }
    std::cout<<"This is your IP address: "<<my_IP<<"\nWrite here your opponent's IP (or [manual]): ";
    std::cin>>opponent_IP;
    if (opponent_IP.compare("manual")==0) //in case 'myIPtoplay.sh' doesn't work properly
    {
      std::cout<<"\t--- MANUAL MODE ---\nType here your IP: ";
      std::cin>>my_IP;
      std::cout<<"Type here your opponent's IP: ";
      std::cin>>opponent_IP;
    }
    if(my_IP.compare(opponent_IP)>0)
      priority=true;
    else
      priority=false;
    if(my_IP.compare(opponent_IP)==0)
    {
      std::cout<<"\tCAUTION: Same IP! \nTo play one of you has to write '0' (P2) and the other '1' (P1): ";
      std::cin>>priority;
    }
  }
}


void player::draw_lives()
{
  double xtr, ytr, ztr;
  for (int i=0; i<lives; i++)
  {
    glLoadIdentity();
    xtr=cannon->position.x+ball->get_radius()*2.1*i;
    ytr=cannon->position.y-cannon->get_radius()+ball->get_radius();
    ztr=cannon->position.z+cannon->get_radius();
    glTranslatef(xtr, ytr, ztr);
    draw_ghost_ball(ball, 1);
  }
}


void player::draw_trajectory(float run_time)
{
  if (run_time/0.1>traces.size()+1)
    traces.push_back(ball->position);
  for (unsigned int l=0; l<traces.size(); l++)
  {
    glLoadIdentity();
    glTranslated(traces[l].x, traces[l].y, traces[l].z);
    draw_ghost_ball(ball, 0.3);
  }
}


void player::draw_ghost_ball(obj_ball* real_ball, float scale_factor)
{
  glTranslatef(0, GROUND_LEVEL, BACK_STEP);
  glNewList(0, GL_COMPILE);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, real_ball->color);
   glMaterialfv(GL_FRONT, GL_SPECULAR, real_ball->color);
   glMaterialf(GL_FRONT, GL_SHININESS, 0.5);
  glEndList();
  glutSolidSphere(real_ball->get_radius()*scale_factor,10.0,10.0);
}


void player::reset_ball_init()
{
  ball->position=0;
  ball->position.set_norm(cannon->get_radius()+ball->get_radius()+ball->get_radius()/10.);
  ball->velocity=ball_init_vel;
  ball->velocity.rotate(cannon->barrel_elevation, 0, 0, 1); 
  ball->velocity.rotate(cannon->barrel_azimuth, 0, 1, 0);
  ball->position.rotate(cannon->barrel_elevation, 0, 0, 1);
  ball->position.rotate(cannon->barrel_azimuth, 0, 1, 0);
  ball->position+=cannon->position;
}


void player::move(int how)
{
 if (get_type()!=NET2)
 {
  //rotation are necessary to have the cannonball follow perfectly the cannon barrel. The method set_spherical of my3Dvector isn't appropriate
  //reset to initial conditions
  ball->position-=cannon->position; 
  ball->velocity.rotate(cannon->barrel_azimuth, 0, -1, 0);
  ball->velocity.rotate(cannon->barrel_elevation, 0, 0, -1);
  ball->position.rotate(cannon->barrel_azimuth, 0, -1, 0);
  ball->position.rotate(cannon->barrel_elevation, 0, 0, -1);
  
  switch (how)
  {
    case MOVE_UP:
      cannon->barrel_elevation+=M_PI/100.;
      break;
    case MOVE_DOWN:
      cannon->barrel_elevation-=M_PI/100.;
      break;
    case MOVE_RIGHT:
      cannon->barrel_azimuth-=M_PI/100.;
      break;
    case MOVE_LEFT:
      cannon->barrel_azimuth+=M_PI/100.;
      break;
    default:;
  }
  
  //apply changes
  ball->velocity.rotate(cannon->barrel_elevation, 0, 0, 1); 
  ball->velocity.rotate(cannon->barrel_azimuth, 0, 1, 0);
  ball->position.rotate(cannon->barrel_elevation, 0, 0, 1);
  ball->position.rotate(cannon->barrel_azimuth, 0, 1, 0);
  ball->position+=cannon->position;
 }
 else if(ufo!=NULL)
 {
  switch (how)
  {
    case MOVE_UP:
      ufo->velocity.set_azimuth(ufo->velocity.azimuth()+M_PI/100.);
      break;
    case MOVE_DOWN:
      ufo->velocity.set_azimuth(ufo->velocity.azimuth()-M_PI/100.);
      break;
    /*case MOVE_RIGHT:
      ufo->velocity.set_elevation(ufo->velocity.elevation()+M_PI/100.);
      break;
    case MOVE_LEFT:
      ufo->velocity.set_elevation(ufo->velocity.elevation()-M_PI/100.);
      break;*/
    default:;
  }
  /* //FIXME this way doesn't work...
  glPushMatrix();
  glLoadIdentity();
  glLineWidth(2.5); 
  //glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_LINES);
  glVertex3f(ufo->position.x, ufo->position.y, ufo->position.z);
  glVertex3f(ufo->velocity.x, ufo->velocity.y, ufo->velocity.z);
  glEnd();
  glPopMatrix();
  */
 }
}


/////////////////////
////Network stuff////
/////////////////////
bool player::tell(std::string message)
{
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 13);
  boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
  boost::asio::ip::tcp::iostream stream;
  acceptor.accept(*stream.rdbuf());
  if(!acceptor.is_open())
  {
    std::cerr<<"\t---ERROR: Connection lost---"<<std::endl;
    return false;
  }
  stream << message;
  return true; 
}

std::string player::listen()
{
  std::string message;
  while(message.length()==0)
  {
    boost::asio::ip::tcp::iostream r(opponent_IP, "daytime");
    std::getline(r, message);
  }
  return message;
}

bool player::communicate_move(player* other, int my_key)
{
  if (player_type!=NET1 || other->get_type()!=NET2)
  {
    std::cerr<<"\t---ERROR: wrong usage of player::comunicate_move(player*, int)---"<<std::endl;
    return false;
  }
  bool tell_check;
  int opponent_key;
  my3Dvector ufo_vel;
  std::stringstream my_move;
  my_move<<std::setprecision(100); //FIXME 100 is actually the same of 53. another way could be using 'reinterpret_cast'
  my_move<<my_key<<" "<<cannon->barrel_azimuth<<" "<<cannon->barrel_elevation<<" "<<ball->position<<" "<<ball->velocity;
  if (ufo!=NULL)
    my_move<<" "<<ufo->velocity;
  if (priority)
    tell_check=tell(my_move.str());
  std::stringstream opnt_move(listen());
  opnt_move>>opponent_key>>(other->cannon->barrel_azimuth)>>(other->cannon->barrel_elevation)>>(other->ball->position)>>(other->ball->velocity);
  if(ufo!=NULL)
    opnt_move>>ufo_vel;
  if(!priority)
    tell_check=tell(my_move.str());
  if (ufo!=NULL)
    ufo->velocity+=ufo_vel;

  if (tell_check && my_key==opponent_key)
    return true;
  else
  {
    if (my_key!=opponent_key)
      std::cerr<<"\t---WARNING: different communication key!---"<<std::endl;
    return false;
  }
}

