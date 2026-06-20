#pragma once

namespace preset {

/// Optional Features you may want to implement.
enum OptionalFeature{
  /// Implement a simple, rulebased AI that *hopefully* beats the random AI.
  SIMPLE_AI_PLAYER,
  /// Write an advanced, intelligent AI, which could possibly even beat a player.
  /// Possible techniques could be Monte-Carlo tree search or other algorithms.
  ADVANCED_AI_PLAYER,
  /// Implement a tournament mode in which multiple (AI) players play several
  /// matches back to back, until a winner is found. Show statistic when a
  /// winner is found.
  TOURNAMENTS,
  /// Store the playthrough of the game and use them for undos, saving and
  /// replays.
  CHRONOLOGY,
  /// Animate objects and interactions with the GUI.
  ANIMATIONS,
  /// Prettier graphics using sprites, textures and effects.
  BETTERGRAPHICS,
  /// 3D visuals.
  THREE_DIMENSIONS,
  /// Sound effects for interactions.
  SOUNDEFFECTS
};

} //namespace

