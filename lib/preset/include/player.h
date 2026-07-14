#pragma once

#include <optional>
#include <string_view>
#include "move.h"
#include "board_config.h"

namespace preset {

/// Docs and requirments of the this class can be found in `ANFORDERUNGEN.md`
class Player {
public:
  virtual ~Player() = default;
  virtual std::string_view get_name() = 0;

  virtual void init(const BoardConfig& board_config, int player_id) = 0;

  virtual std::optional<Move> request() = 0;
  virtual void update(Move opponent_move) = 0;
};

} //namespace

