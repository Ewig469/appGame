#pragma once

#include "board_config.h"
#include "logger.h"
#include "optional_features.h"
#include "player_type.h"
#include <string>
#include <vector>

namespace preset {

/// Struct storing the game's settings. Settings are given to the game by `preset::ArgumentParser`.
struct Settings {
  /// The maximum log level the program should use when printing information/errors.
  LogLevel log_level = LogLevel::INFO;
  /// A list of optional features implemented in this project.
  std::vector<OptionalFeature> implementedOptionalFeatures = {};
  /// Names of each of player.
  std::vector<std::string> player_names = {"Player 1", "Player 2"};
  /// List of player colors as hexadecimal RGB values.
  std::vector<std::string> player_colors = {"#ff0000", "#0000ff"};
  /// List of player types corrosponding to each player.
  std::vector<PlayerType> player_types = {PlayerType::HUMAN, PlayerType::HUMAN};
  /// Delay after a computer player has made its move.
  /// Value in milliseconds.
  long delay = 100;
  /// Whether to show GUI when no human players are playing.
  bool show_GUI = false;
  /// Amount of tournament rounds to be played. 0 means that this is a normal game.
  /// This only applies if `OptionalFeature::TOURNAMENTS` is implemented.
  int num_tournament_round = 0;
  /// Whether the game should wait for user interaction before starting the next tournament round.
  /// This only applies if `OptionalFeature::TOURNAMENTS` is implemented.
  bool wait_after_tournament_round = true;
  /// The volume control for sound effects.
  /// This only applies if `OptionalFeature::SOUNDEFFECTS` is implemented.
  int volume = 0;
  /// Board settings (width, height, rotation)
  BoardConfig board_config;
  /// Whether to read and load a save file from `save_path`.
  /// This only applies if `OptionalFeature::CHRONOLOGY` is implemented.
  bool load_save = false;
  /// The file from which to load a save.
  /// This only applies if `OptionalFeature::CHRONOLOGY` is implemented.
  std::string save_path;
  /// Whether to read and load a replay file from `replay_path`.
  /// This only applies if `OptionalFeature::CHRONOLOGY` is implemented.
  bool load_replay = false;
  /// The file from which to load a game replay (also know as a demo).
  /// This only applies if `OptionalFeature::CHRONOLOGY` is implemented.
  std::string replay_path;
};

// Convert an optional feature to a string
std::string feature_string(OptionalFeature feature);

/// Class used to parse console arguments and create a settings object based on them. Throws `invalid_argument` and `out_of_range` for incorrect command line arguments.
class ArgumentParser {
public:
  /// The Constructor requires `argc` and `argv` from `main`.
  ArgumentParser(int argc, char* argv[], std::string project_name,
                std::string project_version,
                std::vector<std::string> project_authors,
                std::vector<OptionalFeature> implemented_opt_features)
    : project_name_(std::move(project_name)),
    version_(std::move(project_version)),
    authors_(std::move(project_authors))
    {
    settings_ = {};
    settings_.implementedOptionalFeatures = implemented_opt_features;
    arg_buffer_.reserve(argc);
    for (int i = 0; i < argc; i++){
      arg_buffer_.push_back(argv[i]);
    }
    parse();
    Logger::set_max_level(settings_.log_level);
  }
  /// Get the settings object created by the parser.
  const Settings& get_settings(){
    return settings_;
  }

  const std::string_view get_project_name(){
    return project_name_;
  }
  const std::string_view get_version(){
    return version_;
  }
  const std::vector<std::string>& get_authors(){
    return authors_;
  }
  
private:
  Settings settings_;
  std::string project_name_;
  std::string version_;
  std::vector<std::string> authors_;

  std::vector<std::string> arg_buffer_;
  int loc = 0;
  /// Return whether there are still args to consume.
  bool has_next();
  /// Consume and return next arg.
  const std::string_view next();
  /// View next arg.
  const std::string_view ahead();
  bool arg_ahead();
  /// Prints the help text.
  void print_help();
  /// Parse the arguments from the argument buffer.
  void parse();
};

} //namespace

