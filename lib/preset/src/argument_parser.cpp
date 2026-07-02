#include "optional_features.h"
#include "player_type.h"
#include <iostream>
#include <stdexcept>
#include "argument_parser.h"
#include "preset_info.h"
#include <algorithm>
namespace preset {


/// Return whether there are still args to consume.
bool ArgumentParser::has_next(){
  return loc + 1 < static_cast<int>(arg_buffer_.size());
}
/// Consume and return next arg.
const std::string_view ArgumentParser::next(){
  if (has_next()){
    return arg_buffer_[++loc];
  }
  throw std::out_of_range("Out of arguments to parse");
  return {};
}
/// View next arg.
const std::string_view ArgumentParser::ahead() {
  if (has_next()){
    return arg_buffer_[loc+1];
  }
  return {};
}
bool ArgumentParser::arg_ahead(){
  auto arg = ahead();
  if (arg.empty()) {
    return false; 
  }
  return arg[0] != '-';
}
/// Prints the help text.
void ArgumentParser::print_help(){
  std::cout << "=========================================================================" << std::endl
            << "    " << project_name_ << " <" << version_ << ">" << std::endl
            << "  " << "Authors: ";

  for (int i = 0; i < static_cast<int>(authors_.size()-1); i++) std::cout << authors_[i] << ", ";
  std::cout << authors_[authors_.size()-1];

  std::cout << std::endl
            << "=========================================================================" << std::endl
            << "Options:" << std::endl
            << " -d,--delay <DELAY>\t\tDelay in milliseconds after an ai has\n\t\t\t\tmade its move." << std::endl
            << " -g,--gui\t\t\tShow the GUI, even if no HUMAN player\n\t\t\t\texists." << std::endl
            << " -s,--size <WIDTH HEIGHT>\tDimensions of the board. (5-96)" << std::endl
            << " -r,--rotation <ROTATION>\tDegrees by which to rotate the board.\n\t\t\t\t(0.0-90.0)" << std::endl
            << " -h,--help\t\t\tPrint this help message and some extra\n\t\t\t\tinformation about this program." << std::endl
            << " -ll,--loglevel <LEVEL>\t\tThe maximum log level. [ERRORS,\n\t\t\t\tWARNINGS, INFO, DEBUG]" << std::endl
            << " -pc,--playerColors <COLOR ...>\tThe colors of the players. [#RRGGBB ...]" << std::endl
            << " -pn,--playerNames <NAME ...>\tThe names of the players." << std::endl
            << " -pt,--playerTypes <TYPE ...>\tThe type of each player. [HUMAN,\n\t\t\t\tRANDOM_AI, ...]" << std::endl;

  // optional feature settings

  if (std::find(settings_.implementedOptionalFeatures.begin(), settings_.implementedOptionalFeatures.end(), OptionalFeature::SOUNDEFFECTS) != settings_.implementedOptionalFeatures.end()){
    std::cout << " -v,--volume <VOLUME>\t\tVolume of the soundeffects. (0-100).\n\t\t\t\t(SOUNDEFFECTS)" << std::endl;
  }
  if (std::find(settings_.implementedOptionalFeatures.begin(), settings_.implementedOptionalFeatures.end(), OptionalFeature::CHRONOLOGY) != settings_.implementedOptionalFeatures.end()){
    std::cout << " -ls,--loadSave <FILE>\t\tThe file from which the save state\n\t\t\t\tshould be read. (CHRONOLOGY)" << std::endl
              << " -lr,--loadReplay <FILE>\tThe file from which a recorded game\n\t\t\t\tshould be played. (CHRONOLOGY)" << std::endl;
  }
  if (std::find(settings_.implementedOptionalFeatures.begin(), settings_.implementedOptionalFeatures.end(), OptionalFeature::TOURNAMENTS) != settings_.implementedOptionalFeatures.end()){
    std::cout << " -t,--tournament <ROUNDS>\tPlay a tournament. (TOURNAMENTS)" << std::endl
              << " -tw,--tournamentWait\t\tWait for user input before starting\n\t\t\t\tthe next round. (TOURNAMENTS)" << std::endl;
  }

  std::cout << "_________________________________________________________________________" << std::endl << std::endl;

  std::cout << "Preset: " << PRESET_VERSION << std::endl
            << "Raylib: " << RAYLIB_VERSION << std::endl
            << "  This project uses Raylib." << std::endl
            << "    Website: https://www.raylib.com/" << std::endl
            << "    License: zlib" << std::endl;
  if (!settings_.implementedOptionalFeatures.empty()){
    std::cout << "Implemented optional features:" << std::endl;
    for (auto feature : settings_.implementedOptionalFeatures){
      std::cout << "  - " << feature_string(feature) << std::endl;
    }
  }
  std::cout << std::endl;
}
/// Parse the arguments from the argument buffer.
void ArgumentParser::parse(){
  while (has_next()){
    if (arg_ahead()){
      throw std::invalid_argument("Got argument, expected option");
    }
    auto opt = next();
    // parse delay
    if ((opt == "-d") || (opt == "--delay")){
      try {
        settings_.delay = std::stoi(next().data());
      }
      catch (std::invalid_argument const& ex) {
        throw std::invalid_argument("Invalid delay parameter");
      }
      if (settings_.delay < 0){
        throw std::invalid_argument("Invalid delay parameter, can not be negative");
      }
      continue;
    }
    // parse show gui
    if ((opt == "-g") || (opt == "--gui")) {
      settings_.show_GUI = true;
      continue;
    }
    // parse help
    if ((opt == "-h") || (opt == "--help")) {
      print_help();
      std::exit(0);
      break;
    }
    // parse log level
    if ((opt == "-ll") || (opt == "--loglevel")) {
      if (!arg_ahead()) throw std::invalid_argument("Expected argument, got an option");
      auto level = next();
      if (level == "ERRORS"){
        settings_.log_level = LogLevel::ERRORS;
        continue;
      }
      if (level == "WARNINGS"){
        settings_.log_level = LogLevel::WARNINGS;
        continue;
      }
      if (level == "DEBUG"){
        settings_.log_level = LogLevel::DEBUG;
        continue;
      }
      if (level == "INFO"){
        settings_.log_level = LogLevel::INFO;
        continue;
      }
      throw std::invalid_argument("Invalid log level");
     
      break;
    }
    // parse player colors
    if ((opt == "-pc") || (opt == "--playerColors")) {
      std::vector<std::string> colors = {};
      while (arg_ahead()){
        colors.push_back(next().data());
      }
      if (colors.size() < 2){
        throw std::invalid_argument("Not enough player colors. (Needs to be at least 2)");
        break;
      } 
      settings_.player_colors = std::move(colors);
      continue;
    }
    // parse player names
    if ((opt == "-pn") || (opt == "--playerNames")) {
      std::vector<std::string> names = {};
      while (arg_ahead()){
        names.push_back(next().data());
      }
      if (names.size() < 2){
        throw std::invalid_argument("Not enough player names. (Needs to be at least 2)");
        break;
      } 
      settings_.player_names = std::move(names);
      continue;
    }
    // parse player types
    if ((opt == "-pt") || (opt == "--playerTypes")) {
      std::vector<PlayerType> types = {};
      while (arg_ahead()){
        auto type = next();
        if (type == "HUMAN"){
          types.push_back(PlayerType::HUMAN);
          settings_.show_GUI = true;
          continue;
        }
        if (type == "RANDOM_AI"){
          types.push_back(PlayerType::RANDOM_AI);
          continue;
        }
        if (type == "SIMPLE_AI"){
          types.push_back(PlayerType::SIMPLE_AI);
          continue;
        }
        if (type == "ADVANCED_AI"){
          types.push_back(PlayerType::ADVANCED_AI);
          continue;
        }

        throw std::invalid_argument("Invalid player type");
        break;
      }

      if (types.size() < 2){
        throw std::invalid_argument("Not enough player types. (Needs to be at least 2)");
        break;
      } 
      
      settings_.player_types = std::move(types);
      continue;
    }
    // parse volume
    if ((opt == "-v") || (opt == "--volume")){
      try {
        settings_.volume = std::stoi(next().data());
      }
      catch (std::invalid_argument const& ex) {
        throw std::invalid_argument("Volume parameter needs to be a number");
        break;
      }
      if (settings_.volume < 0 || settings_.volume > 100){
        throw std::out_of_range("Out of range volume. Allowed range: [0, 100]");
        break;
      }
      continue;
    }
    // parse rotation
    if ((opt == "-r") || (opt == "--rotation")){
      try {
        settings_.board_config.rotation = std::stof(next().data());
      }
      catch (std::invalid_argument const& ex) {
        throw std::invalid_argument("Board rotation parameter needs to be a number");
        break;
      }
      if (settings_.board_config.rotation < 0.0 || settings_.board_config.rotation > 90.0){
        throw std::out_of_range("Out of range board rotation. Allowed range: [0.0, 90.0]");
        break;
      }
      continue;
    }
    // parse size
    if ((opt == "-s") || (opt == "--size")){
      // width
      try {
        settings_.board_config.width = std::stoi(next().data());
      }
      catch (std::invalid_argument const& ex) {
        throw std::invalid_argument("Width parameter needs to be a number");
        break;
      }
      if (settings_.board_config.width < 5 || settings_.board_config.width > 96){
        throw std::out_of_range("Out of range board width. Allowed range: [5, 96]");
        break;
      }
      // height
      try {
        settings_.board_config.height = std::stoi(next().data());
      }
      catch (std::invalid_argument const& ex) {
        throw std::invalid_argument("Height parameter needs to be a number");
        break;
      }
      if (settings_.board_config.height < 5 || settings_.board_config.height > 96){
        throw std::out_of_range("Out of range board height. Allowed range: [5, 96]");
        break;
      }
      
      continue;
    }
    // parse load save
    if ((opt == "-ls") || (opt == "--loadSave")){
      settings_.load_save = true;
      settings_.save_path = next();
      continue;
    }
    // parse play replay
    if ((opt == "-lr") || (opt == "--loadReplay")){
      settings_.load_replay = true;
      settings_.replay_path = next();
      continue;
    }
    // parse tournament
    if ((opt == "-t") || (opt == "--tournament")){
      try {
        settings_.num_tournament_round = std::stoi(next().data());
      }
      catch (std::invalid_argument const& ex) {
        throw std::invalid_argument("Rounds parameter needs to be a number");
        break;
      }
      if (settings_.num_tournament_round < 1){
        throw std::out_of_range("Rounds cannot be less than 1");
        break;
      }
      continue;
    }
    // parse show gui
    if ((opt == "-tw") || (opt == "--tournamentWait")) {
      settings_.wait_after_tournament_round = true;
      continue;
    }

    throw std::invalid_argument("Invalid option");
  }
}

// Convert an optional feature to a string
std::string feature_string(OptionalFeature feature){
  switch (feature) {
    case SIMPLE_AI_PLAYER:
      return "SIMPLE_AI_PLAYER";
    case ADVANCED_AI_PLAYER:
      return "ADVANCED_AI_PLAYER";
    case TOURNAMENTS:
      return "TOURNAMENTS";
    case CHRONOLOGY:
      return "CHRONOLOGY";
    case ANIMATIONS:
      return "ANIMATIONS";
    case BETTERGRAPHICS:
      return "BETTERGRAPHICS";
    case THREE_DIMENSIONS:
      return "THREE_DIMENSIONS";
    case SOUNDEFFECTS:
      return "SOUNDEFFECTS";
    }
  return "";
}

} //namespace

