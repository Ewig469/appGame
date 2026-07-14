#pragma once

#include "move.h"
#include <optional>

namespace preset {

/// A reference to a PlayerGuiAccess is allowed to be given to the human player via its constructor.
/// Your GUI should inherit from this class so a HUMAN player can request a move from the GUI.
class PlayerGuiAccess {
public:
  virtual std::optional<Move> request_move_from_current_human_player() = 0;
};

} //namespace

