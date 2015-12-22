//File: game_handler.hpp

#ifndef GAME_HANDLER_HPP
#define GAME_HANDLER_HPP

///////////////////////////////////////////////////////////
/// contains all the functions that handle the gameplay ///
///////////////////////////////////////////////////////////

#define LAP_RATE 500 //cycles of the physical_system betwen each new redrawing
#define FPS 33 //Milliseconds between two frames, increase it if game is too fast, decrease if too slow
#define ESC_KEY 27
#define RETURN_KEY 13
#define SPACEBAR_KEY 32
#define LAST_LEVEL 5 //there are at least that many levels for each play modality

#include "graphics.hpp"
#include "my3Dvector.h"
#include "physic_object.h"
#include "physic_system.h"
#include "player.h"

// Needed overall variables and functions: 
/*--time--*/
float step=0.0001;  //sets the precision of the simulation
float level_time=0; //keeps track of the global time of a level
float max_time=20; //sets the max duration of a shot
float max_distance=20; //sets the max distance the cannonball can go
int teapot_exp_duration=100; //sets duration of the explosion
int cannonball_exp_duration=30;

/*--switches--*/
bool cheats=false;
bool cover_view=true; //true to view the cover screen at the beginning
bool single_player=false;
bool local_multiplayer=false;
bool network_multiplayer=false; 
bool pause_game_system=false; //to stop the system evolution
bool pause_frame=false;//to stop both, drawing and evolution
bool reverse_time=false; //true to reverse time flow
bool new_shot=true;
bool new_shot2=true;

/*--levels--*/
int level=0;
std::string levels_addr("./media/Levels");
std::string levels_ext(".dat");
std::string level_name;

/*--game system--*/
physic_system *game_system;
obj_teapot *theTeapot; //FIXME what about using more than one teapot?
player *P1, *P2;
void level_init();
void end_turn_singleplayer();
void end_turn_multiplayer();
void end_game();


/*--OpenGl related--*/ //game_handler is not a class because of these, OpenGl requires functions, and not methods.
void explosion (const spherical_object*, int);
void display();
void animate(int);
void keyboard(unsigned char, int, int);
void special_keys(int, int, int);


// Function definitions:

////////////////////////////////////////////////////
/////////////////    GAME_SYSTEM   /////////////////
////////////////////////////////////////////////////
//Game_system: create it, delete it, and manage the end of each turn (with explosions!)
void level_init()
{
  level_name=levels_addr;
  if (level<LAST_LEVEL)
    level_name+=('0'+level); //convertion from int to char: works only for less than 10!
  else
  {
    std::cerr<<"\tCAUTION: Level "<<level<<" doesn't exist yet!"<<std::endl;
    end_game();
  }
  level_name+=levels_ext;
  
  game_system=new physic_system(level_name);
  int cannon_counter=1;
  int ball_counter=1; //the first 2 balls in the data file have to be the cannonballs! possible fix: add a kind_cannonball
	
  for(unsigned int i=0; i<game_system->size(); i++)
  {
    switch (game_system->all_objects[i]->get_kind())
    {
      case KIND_TEAPOT:
        theTeapot = static_cast<obj_teapot*>(game_system->all_objects[i]);
        break;
      case KIND_CANNON:
        if (cannon_counter==1)
          P1->cannon = static_cast<obj_cannon*>(game_system->all_objects[i]);
        if (cannon_counter==2 && !single_player)
          P2->cannon = static_cast<obj_cannon*>(game_system->all_objects[i]);
        cannon_counter++;
        break;
      case KIND_BALL:
        if (ball_counter==1)
          P1->ball = static_cast<obj_ball*>(game_system->all_objects[i]);
        if (ball_counter==2 && !single_player)
          P2->ball = static_cast<obj_ball*>(game_system->all_objects[i]);
        ball_counter++;
        break;
      case KIND_UFO:
        if (network_multiplayer)
        {
          P1->ufo = static_cast<obj_ufo*>(game_system->all_objects[i]);
          P2->ufo=P1->ufo;
        }
        break;
      default:;
    }
  }
  
  P1->ball_init_vel=P1->ball->velocity;
  P1->reset_ball_init();
  if(!single_player)
  {
    P2->ball_init_vel=P2->ball->velocity;
    P2->reset_ball_init();
    P2->shot_time=0;
    new_shot2=true;
  }
  P1->shot_time=0;
  new_shot=true;
  pause_frame=true;
  pause_game_system=true;
  if (local_multiplayer) //we want the game in local_multiplayer mode to be continuous
  {
    cannonball_exp_duration=0; 
    P1->lives++;
    P2->lives++;
    P1->ball->fixed=true;
    P2->ball->fixed=true;
    P1->cannon->fixed=true;
    P2->cannon->fixed=true;
    pause_game_system=false;
  }
}


//////////////////////////////////////////
////////////    END TURN   ///////////////
//////////////////////////////////////////
//single player version
void end_turn_singleplayer()
{
  if (P1->winner)
  {
    explosion(theTeapot, teapot_exp_duration);
    output(1200,2300,"Level Cleared!");
    std::cout<< P1->name <<" wins level "<<level<<std::endl;
    if(level!=(LAST_LEVEL))
    	output(1000,2000,"press [spacebar] to continue");
    glutSwapBuffers();
    glFlush();    
    
    P1->traces.clear();
    P1->winner=false;
    P1->lives++;
    level++;
    level_time=0;
    
    delete (game_system);
    level_init();
    std::cout<<"\nGreat shot!\n\tWelcome to level "<<level<<".\tPress [p] to begin"<<std::endl;
    std::cout<<"\tUse [up arrow] and [down arrow] to set the angle, [return] to start."<<std::endl; 
  }
	
  else if ((P1->shot_time>max_time || (P1->ball->position).norm()>max_distance))
  {
    explosion(P1->ball, cannonball_exp_duration);
    P1->lives--;
    if(P1->lives>=0)
    {
      std::cout<< "Don't worry, "<< P1->name<< " you have still "<< P1->lives <<" lives left. Try again!"<<std::endl;
      std::cout<< "-----Press  [spacebar] to continue----- "<< std::endl;
      output(1000,2000,"press [spacebar] to continue");
      glutSwapBuffers();
      glFlush();
      delete (game_system);
      level_init();
    }
    if(P1->lives<0)
      end_game();
  }
}

//multiplayer version
void end_turn_multiplayer()
{
  if (local_multiplayer)
    level_time=0; //small expedient: this way no traces will be drawn

  if(P1->winner || P2->winner)
  {
    explosion(theTeapot, teapot_exp_duration);
    output(1200,2300,"We have a winner!!");
    if (P1->winner)
    {
      output(1400,2000,P1->name.c_str());
      std::cout<< P1->name <<" wins the level!"<<std::endl;
      P1->lives++;
    }
    if (P2->winner)
    {
      output(1400,2000,P2->name.c_str());
      std::cout<< P2->name <<" wins the level!"<<std::endl;
      P2->lives++;
    }
    output(1000,1800,"press [spacebar] to continue");
    glutSwapBuffers();
    glFlush();
		
    level++;
    delete (game_system);
    level_init();
    
  //we have to initialize the time and cancel the traces when we load a new level
    level_time=0;
    P1->traces.clear();
    P2->traces.clear();
    P1->winner=false;
    P2->winner=false;
  }	
  else
  {
    if (P1->shot_time>max_time || (local_multiplayer && P1->ball->position.norm()>max_distance) || (network_multiplayer && P1->ball->position.norm()>max_distance && P2->ball->position.norm()>max_distance))
    {
      explosion(P1->ball, cannonball_exp_duration);
      P1->lives--;
      new_shot=true;
      P1->shot_time=0;
      P1->reset_ball_init();
      P1->ball->fixed=true;
      std::stringstream next1, next2;
      next1<< "Don't worry, "<< P1->name<< " you have still "<< P1->lives <<" lives left.";
      if (network_multiplayer)
      {
        P2->lives--;
        if (P2->lives<0 || P1->lives<0)
          end_game();
        next2<< "Don't worry, "<< P2->name<< " you have still "<< P2->lives <<" lives left.";
        output(800,1800,next1.str().c_str());
        output(800,1700,next2.str().c_str());
        output(1000,2000,"press [spacebar] to continue");
        glutSwapBuffers();
        glFlush();
        delete (game_system);
        level_init();
      }
      if (P1->lives<0)
        end_game();
      std::cout<<next1.str()<<std::endl;
      std::cout<<next2.str()<<std::endl;
    }
    if (P2->shot_time>max_time || (local_multiplayer && P2->ball->position.norm()>max_distance) || (network_multiplayer && P1->ball->position.norm()>max_distance && P2->ball->position.norm()>max_distance))
    {
      explosion(P2->ball, cannonball_exp_duration);
      P2->lives--;
      new_shot2=true;
      P2->shot_time=0;
      P2->reset_ball_init();
      P2->ball->fixed=true; 
      std::stringstream next1, next2;
      next2<< "Don't worry, "<< P2->name<< " you have still "<< P2->lives <<" lives left.";
      if (network_multiplayer)
      {
        P1->lives--;
        if (P1->lives<0 || P2->lives<0)
          end_game();
        next1<< "Don't worry, "<< P1->name<< " you have still "<< P1->lives <<" lives left.";
        output(800,1800,next1.str().c_str());
        output(800,1700,next2.str().c_str());
        output(1000,2000,"press [spacebar] to continue");
        glutSwapBuffers();
        glFlush();
        delete (game_system);
        level_init();
      }
      if (P2->lives<0)
        end_game();   
      std::cout<<next1.str()<<std::endl;
      std::cout<<next2.str()<<std::endl;
    }
  }
}

void end_game()
{
  std::stringstream end;
  end<<"Last level reached: ";
  if ((single_player && P1->lives>=0) || (!single_player && (P1->lives>=0 && P2->lives>=0)))
  {
    if (single_player)
      end<<"YOU ARE THE WINNER!!!";
    if (!single_player)
    {
      if (P1->lives>P2->lives)
        end<<P1->name<<" IS THE WINNER!!!";
      else if (P1->lives<P2->lives)
        end<<P2->name<<" IS THE WINNER!!!";
      else
        end<<"TIE!!!";
    }
    std::cout<<end.str()<< std::endl;
    output(750,1500,end.str().c_str());
    glutSwapBuffers();
    glFlush();
    sleep(4);
    delete P1;
    delete P2;
    exit(0);
  }
  else
  {
    output(1300,2300,"GAME OVER");
    if (single_player && P1->lives<0)
      std::cout<< "......You lost....."<<std::endl;
    if (!single_player)
    {
      if (P2->lives<0 && P1->lives>=0)
      {
        end<<P2->name<< " is dead.\n"<< P1->name<<" is the winner!!";
        std::cout<<end.str()<< std::endl;
        output(1100,2000,end.str().c_str());
      }
      if (P1->lives<0 && P2->lives>=0)
      {
        end<<P1->name<< " is dead.\n"<< P2->name<<" is the winner!!";
        std::cout<<end.str()<< std::endl;
        output(1100,2000,end.str().c_str());
      }
      if (P1->lives<0 && P2->lives<0)
      {
        end<<"......both of you lost.....";
        std::cout<<end.str()<<std::endl;
        output(1100,2000,end.str().c_str());
      }
    }
    glutSwapBuffers();
    glFlush();
    sleep(4);
    delete (game_system);
    delete P1;
    delete P2;
    exit(0);
  }
}

void explosion(const spherical_object* bomb, int duration) //here and not in 'graphics.hpp' because uses physical_object
{
  GLfloat red[] = {1.0, 0.0, 0.0, 1.0};
  GLfloat orange[]={1.0,1.0,0.5,1.0};
  GLfloat yellow[]={1.0, 1.0, 0.0, 1.0};
  
  float dimension=bomb->get_radius()-bomb->get_radius()/10.;
  my3Dvector pos=bomb->position;
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  
  for(int i=0;i<duration;i++)
  {
    glLoadIdentity();
    glTranslatef(pos.x, pos.y+GROUND_LEVEL, pos.z+BACK_STEP);
    draw_cube(dimension, red);
   
    glLoadIdentity();
    glTranslatef(pos.x, pos.y+GROUND_LEVEL, pos.z+BACK_STEP);
    glRotatef(45,1,1,0);
    draw_cube(dimension, yellow);
    
    glLoadIdentity();
    glTranslatef(pos.x, pos.y+GROUND_LEVEL, pos.z+BACK_STEP);
    glRotatef(35,0,0,1);
    glRotatef(45,0,1,0);
    draw_cube(dimension, orange);
    
    dimension+=bomb->get_radius()/10.;
    glutSwapBuffers();
    glFlush();
  }
 glPopMatrix();
}

/////////////////////////////////////////////////
////////////////    DISPLAY    //////////////////
/////////////////////////////////////////////////
//Display: the function that draws
void display (void)
{
  if(cover_view)
  {
    glEnable(GL_TEXTURE_2D);			        // Enable Texture Mapping
    LoadGLTextures("./media/Destroy_the_teapot.bmp");// Load The Texture(s) 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    
    glLoadIdentity();				// Reset The View
    glTranslatef(0.0f,0.0f,-4.0f);              // move 5 units into the screen.   
    glBindTexture(GL_TEXTURE_2D, texture[0]);   // choose the texture to use.

    glBegin(GL_QUADS);		                // begin drawing a cube
     glCullFace(GL_BACK);
    // Front Face (note that the texture's corners have to match the quad's corners)
     glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.0f, -1.6f,  1.0f);	// Bottom Left Of The Texture and Quad
     glTexCoord2f(1.0f, 0.0f); glVertex3f( 2.0f, -1.6f,  1.0f);	// Bottom Right Of The Texture and Quad
     glTexCoord2f(1.0f, 1.0f); glVertex3f( 2.0f,  1.6f,  1.0f);	// Top Right Of The Texture and Quad
     glTexCoord2f(0.0f, 1.0f); glVertex3f(-2.0f,  1.6f,  1.0f);	// Top Left Of The Texture and Quad
    glEnd();
	
    glutSwapBuffers();
    glFlush();
  	
    cover_view=false; 
    sleep(3);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHT1);
  }
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE); 
  
//Drawing the whole physic system
  game_system->draw();
  
//Drawing Cannonball Traces and lives	
  P1->draw_lives();
  P1->draw_trajectory(level_time);
  if (!single_player)
  {
    P2->draw_lives();
    P2->draw_trajectory(level_time);
  }

//Write on the Screen
  std::stringstream level_N;
  level_N<<"Level "<<level<<": ";
  switch(game_system->get_world())
  {
    case WORLD_EARTH:
      level_N<<"Earth";
      break;
    case WORLD_MOON:
      level_N<<"Moon";
      break;
    case WORLD_SPACE:
      level_N<<"Open Space";
      break;
    case WORLD_LAB:
      level_N<<"Laboratory";
      break;
    default:
      level_N<<"Unknown World";
      break;
  }
  output(80, 2800, level_N.str().c_str());
  output(80, 250, "INSTRUCTIONS:");
  output(80, 30, "Press C to enable Mouse navigation mode");  
  
  if (single_player)
  {
    output(80, 420, P1->name.c_str());
    output(80, 140, "Use UP/DOWN arrows to move your cannon barrel - Press RETURN to fire");
  }
  else
  { 
    output(2700, 420, P1->name.c_str());
    output(80, 420, P2->name.c_str());
    if (local_multiplayer)
      output(80, 140, "P2: W/S keys, Q to fire     ---      P1: UP/DOWN arrows, RETURN to fire");
    else
      output(80, 140, "P2: W/S keys P1: UP/DOWN arrows - Press RETURN to fire");
  }
	
//Draw on the Buffer 	
  glutSwapBuffers();
  glFlush();

//Load a new level if the teapot is hit or lose one life if it isn't  
  if(single_player)
    end_turn_singleplayer();
  else
    end_turn_multiplayer();
}

/////////////////////////////////////////////////
//////////////////   ANIMATE   //////////////////
/////////////////////////////////////////////////
//Manage the animation: frames/play/pause
void animate(int value)
{
// Set up the next timer tick (do this first)
  if(!pause_frame)
  {
    glutTimerFunc(FPS, animate, 0);
    if (!pause_game_system)
    {
      if(reverse_time)
      {
        for (unsigned int i=0; i<LAP_RATE; i++)
          game_system->evolve(-1*step);
      }
      else
      {
        for (unsigned int i=0; i<LAP_RATE; i++) //to make it faster
        {
          game_system->evolve(step);
          if (!new_shot)
          {
            level_time+=step;
            P1->shot_time+=step;
          }
          if (local_multiplayer && !new_shot2)
            P2->shot_time+=step;
          if(in_touch(P1->ball, theTeapot))
            P1->winner=true;
          if (!single_player)
          {
            if(in_touch(P2->ball, theTeapot))
              P2->winner=true;
          }
        }
      }
    }
    glutPostRedisplay();
  }
}


/////////////////////////////////////////////////
////////////////    KEYBOARD    /////////////////
/////////////////////////////////////////////////
//Manage keyboard actions
void keyboard(unsigned char key, int x, int y)
{
 //Player 2 controls:
  if (!single_player && !pause_frame && new_shot2)
  {
    switch (key)
    {
      case 'w':
        P2->move(MOVE_UP);
        break;
      case 's':
        P2->move(MOVE_DOWN);
        break;
      case 'd':
        P2->move(MOVE_RIGHT);
        break;
      case 'a':
        P2->move(MOVE_LEFT);
        break;
      default:;
    }
  }
  
 //Game controls:
  switch (key)
  {
    case RETURN_KEY:
      if (new_shot && !pause_frame) 
      {
        if (network_multiplayer)
        {
          std::cerr<<"\t--- network communication: please wait for your opponent's move---"<<std::endl;
          if(P1->get_type()==NET1)
            P1->communicate_move(P2, level);
          else
            P2->communicate_move(P1, level);
          new_shot2=false;
        }
        new_shot=false;
        pause_game_system=false;
        if (local_multiplayer)
          P1->ball->fixed=false;
      }
      break;
    case 'q':
      if (new_shot2 && !pause_frame && local_multiplayer)
      { 
        P2->ball->fixed=false;
        new_shot2=false;
      }
      break;
    case ESC_KEY:
      exit(0);
      break;
    case 'p': case SPACEBAR_KEY:
      if(pause_frame)
      {
        pause_frame=false;
        glutTimerFunc(FPS,animate,0);
        std::cerr<<"\tpause: OFF\n"<<std::endl;
      }
      else
      {
        pause_frame=true;
        std::cerr<<"\tpause: ON\n"<<std::endl;
      }
      break;
    case 'c': //sperimental feature: there are still some issues, see zpr.hpp for details
    if(static_camera)
      {
        static_camera=false;
        zprReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        std::cerr<<"\tstatic_camera: FALSE\n"<<std::endl;
      }
      else
      {
        static_camera=true;
        zprReshape(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
        std::cerr<<"\tstatic_camera: TRUE\n"<<std::endl;
      }
      break;
    default:;
  }
  
  //Special features: execute with -c to enable cheats
  if (cheats)
  {  
    switch (key)
    {
      case 'r':
        if(reverse_time)
          reverse_time=false;
        else
          reverse_time=true;
        break;
      case 'v':
        step=step*2;
        std::cerr<<"\t---step: "<<step<<std::endl;
        break;
      case 'l':
        step=step/2;
        std::cerr<<"\t---step: "<<step<<std::endl;
        break;
      case '+':
        P1->ball->velocity.set_norm(P1->ball->velocity.norm()+0.2);
        std::cerr<<"P1->ball->velocity.norm()="<<P1->ball->velocity.norm()<<std::endl;
        break;
      case '-':
        P1->ball->velocity.set_norm(P1->ball->velocity.norm()-0.2);
        std::cerr<<"P1->ball->velocity.norm()="<<P1->ball->velocity.norm()<<std::endl;
        break;
      case 'k':
        P1->shot_time=max_time+1;
        if (!single_player)
          P2->shot_time=max_time+1;
        std::cerr<<"\t---end shot---"<<std::endl;
        break;
      default:;
    }
  }
}

//Manage special keyboard keys such as arrows
void special_keys(int key, int x, int y)
{
 //Player 1 controls:
  if (!pause_frame && new_shot)
  {
    switch (key)
    {
      case GLUT_KEY_UP:
        P1->move(MOVE_UP);
        break;
      case GLUT_KEY_DOWN:
        P1->move(MOVE_DOWN);
        break;
      case GLUT_KEY_RIGHT:
        P1->move(MOVE_RIGHT);
        break;
      case GLUT_KEY_LEFT:
        P1->move(MOVE_LEFT);
        break;
      default:;
    }
  }
}

#endif


