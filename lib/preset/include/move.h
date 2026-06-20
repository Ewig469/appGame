#pragma once

#include <ostream>
namespace preset {

/// Class used to model a move, which can be passed around between players.
class Move {
public:
  /// Construct a move with desired x and y coordinates and the ID
  // of the player who made the move.
  Move(int x, int y, int player_id) :
    x_(x), y_(y), player_id_(player_id) {};
  /// return the desired x position of this move.
  int get_x(){
    return x_;
  }
  /// Return the desired y position of this move.
  int get_y(){
    return y_;
  }
  /// Return ID of the player who made this move.
  int get_id(){
    return player_id_;
  }

  /// Equality operator.
  bool operator==(const Move& other){
    return (x_ == other.x_) && (y_ == other.y_) && (player_id_ == other.player_id_); 
  }
  friend std::ostream& operator<<(std::ostream& os, const Move& m);

private:
  /// Coordinates of peg to be placed
  int x_, y_;
  /// ID of the player who played this move
  int player_id_;
};
inline std::ostream &operator<<(std::ostream &out, const Move &m){
  out << "Move (Coordinate: (" << m.x_ << ", " << m.y_ << "), Player ID: " << m.player_id_ << ")";
  return out;
}

} //namespace

