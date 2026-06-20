#pragma once

namespace preset {

/// The different types of players.
enum PlayerType{
  /// Human player controlled by GUI.
  HUMAN,
  /// AI player playing random moves. 
  RANDOM_AI,
  /// AI player following rules and heuristics to *ideally* be
  /// better than RANDOM_AI
  SIMPLE_AI,
  /// AI player using advanced strategies, like predicting the
  /// future using Monte-Carlo tree search.
  ADVANCED_AI
};

} //namespace

