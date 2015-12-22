/*
FILE: main.cpp
DESCRIPTION:	This game creates a physics system in 3D where the total energy is always conserved.
		You control a cannon and your task is to destroy the teapot.
		The game has three play modality: single player mode, two players on the same computer and two players connected via 			local lan.
LIBRARIES:  In addition to the ordinary C++ libraries, we used OpenGl for the graphics and Boost::asio for the network setup
	(packages: freeglut, libasio, libboost-system)
NB:  The network multiplayer mode needs administrator privileges: use 'sudo'. 
     It uses a script, 'myIPtoplay.sh', to find the IP address. If it doesn't work you can insert the IP manually.
*/

#include <iostream>
#include <string>

//our headers
#include "graphics.hpp"
#include "game_handler.hpp"


int main(int argc, char*argv[])
{
  if (argc>1)
  {
    if (argv[1][0]=='-' && argv[1][1]=='c')
    {
      cheats=true;
      std::cout<<"\t--- Cheat mode enabled";
      if (argc==3)
      {
        std::stringstream(argv[2])>>level;
        std::cout<<": level "<<level<<" loaded";
      }
      std::cout<<" ---"<<std::endl;
    }
  }
  std::cout<<"\nWelcome to \"Destroy the Teapot\" (beta prototype)."<<std::endl;
  std::cout<<"\tAre you playing alone [1] or with a friend [2]?  ";  
  std::string ans;
  std::getline(std::cin,ans);
  while (ans.compare("1")!=0 && ans.compare("2")!=0)
  {
    std::cout<<"It's easy: you should write either 1 or 2, not \""<<ans<<"\" or whatever else."<<std::endl;
    std::cout<<"\n\tAre you playing alone [1] or with a friend [2]? ";
    std::getline(std::cin,ans);
  }
  
  if (ans.compare("1")==0)
  {
    P1=new player(LOCAL);
    single_player=true;
    levels_addr+="/1Player/1LOC_level";
  }
  else
  {
    std::cout<<"\tOn the same computer [local] or on different ones [net]?  ";  
    std::getline(std::cin,ans);
    while (ans.compare("local")!=0 && ans.compare("net")!=0)
    {
      std::cout<<"Guess what... you should write either \"local\" or \"net\", not \""<<ans<<"\" or whatever else."<<std::endl;
      std::cout<<"\n\tAre you playing on the same computer [local] or on different ones [net]?  ";
      std::getline(std::cin,ans);
    }
   
    if(ans.compare("local")==0)
    {
      local_multiplayer=true;
      levels_addr+="/2Player/2LOC_level";
      std::cout<<"Player 1 ([arrows] to move, [return] to fire):"<<std::endl;
      P1=new player(LOCAL);
      std::cout<<"Player 2 ([w],[s],[a],[d] to move, [q] to fire):"<<std::endl;
      P2=new player(LOCAL);
      if (P1->name.compare(P2->name)==0)
      {
        std::cerr<<"\t -- "<<P1->name<<": next time try with some imagination... "<<std::endl;
        P1->name+="_P1";
        P2->name+="_P2";
      }
    }

    if(ans.compare("net")==0)
    {
      std::string name;
      P1=new player(NET1);
      
      //checking that the connection works: player1 speaks first, while player2 speaks after him/her
      if (P1->get_priority())
        P1->tell(P1->name);
      name=P1->listen();
      std::cout<<"Your opponent's name is \""<<name<<"\"."<<std::endl;
      if (!P1->get_priority())
        P1->tell(P1->name);
      
      //choosing who is P1 and who is P2
      if (!P1->get_priority())
      {
        P2=P1;
        P1=new player(NET2);
        P1->name=name;
        std::cout<<"You are Player 2, use [w] [s] [a] [d] to move."<<std::endl;
      }
      else
      {
        P2=new player(NET2);
        P2->name=name;
        std::cout<<"You are Player 1, use [arrows] to move."<<std::endl;
      }
      network_multiplayer=true;
      levels_addr+="/Network/2NET_level";
    }
  }
  std::cout<<"\n-----Good luck!-----"<<std::endl;
  
//initializing the physic_system...
  level_init();
  pause_frame=false;

//Initializing needed Glut and GL stuff...
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
  glutInitWindowSize (640, 480);
  glutInitWindowPosition (100,100);
  glutCreateWindow ("Destroy the teapot! Powered by: M.I., M.B., A.M.");
  
  glEnable(GL_NORMALIZE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_LIGHTING);
  glDisable(GL_DITHER);
  glEnable(GL_CULL_FACE);
  glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  glDepthRange(0.0,1.0);
  glClearDepth(1.0);
  glShadeModel(GL_SMOOTH);
  light();
  
  zprInit();
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special_keys);
	
  glutDisplayFunc(display);
  glutTimerFunc(FPS,animate,0);

  glutMainLoop();
  
  return 0;
}

