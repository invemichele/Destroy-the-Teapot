//File: player.h

#ifndef PLAYER_H
#define PLAYER_H

//////////////////////////////////////////////////////
// class that manages everything needed by a player //
//////////////////////////////////////////////////////

enum {LOCAL, NET1, NET2}; //NET1 is the physical player, NET2 is the remote player
enum {MOVE_UP, MOVE_DOWN, MOVE_RIGHT, MOVE_LEFT};
#define INITIAL_LIVES 3

#include <boost/asio.hpp>
#include <sstream>

#include "my3Dvector.h"
#include "physic_object.h"
#include "physic_system.h"

class player
{
  public:
    player(int type);
    ~player() {};
    
  //things that every player needs:
    std::string name;
    int lives;
    float shot_time;
    obj_cannon *cannon;
    obj_ball *ball;
    obj_ufo *ufo;
    bool winner;	
    int get_type() const {return player_type;};
    std::vector<my3Dvector> traces; //to draw the trajectory of the cannonballs
    my3Dvector ball_init_vel;

    void draw_lives();
    void draw_trajectory(float); //must access level_time
    void move(int);
    void reset_ball_init();
	
  //network stuff:
    bool get_priority() const {return priority; };
    std::string listen();
    bool tell(std::string);
    bool communicate_move(player*, int);
		
  private:
    int player_type;
    void draw_ghost_ball(obj_ball*, float);
    
    bool priority; //to regulate telling/listening between the two online players		
    std::string opponent_IP;
};

#endif



