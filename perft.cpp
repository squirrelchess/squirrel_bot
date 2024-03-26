
#include "perft.h"
#include "position.h"
#include "debug.h"
#include "ui.h"
#include "util.h"
#include "gamestate.h"
#include "movegen.h"
#include "moveordering.h"

#include <thread>
#include <chrono>

uint64_t leaves;

template<Color ToMove>
void expand(int depth) {

  if (depth == 0) {
    leaves++;
    return;
  }

  if constexpr (ToMove == WHITE) {
    MoveList<WHITE> ml(false);
    ml.sort(NULLMOVE, 0);
    for (int i = 0; i < ml.length(); i++) {
      Piece captured = piece_on(to_sq(ml[i]));
      uint8_t c_rights = GameState::castling_rights;
      do_move<WHITE>(ml[i]);
      expand<BLACK>(depth - 1);
      undo_move<WHITE>(ml[i], captured);
      GameState::castling_rights = c_rights;
    }
  }
  else {
    MoveList<BLACK> ml(false);
    ml.sort(NULLMOVE, 0);
    for (int i = 0; i < ml.length(); i++) {
      Piece captured = piece_on(to_sq(ml[i]));
      uint8_t c_rights = GameState::castling_rights;
      do_move<BLACK>(ml[i]);
      expand<WHITE>(depth - 1);
      undo_move<BLACK>(ml[i], captured);
      GameState::castling_rights = c_rights;
    }
  }

}

void Perft::go(int depth) {

  if (GameState::white_to_move) {
    for (int plies_to_search = 1; plies_to_search <= depth; plies_to_search++) {
      leaves = 0;
      auto start = curr_time_millis();
      MoveList<WHITE> movelist(true);
      for (Move m : movelist) {
        Piece captured = piece_on(to_sq(m));
        uint8_t c_rights = GameState::castling_rights;
        do_move<WHITE>(m);
        expand<BLACK>(plies_to_search - 1);
        undo_move<WHITE>(m, captured);
        GameState::castling_rights = c_rights;
      }
      auto delta = curr_time_millis() - start;
      std::cout << "\nDepth " << plies_to_search << ": " << leaves << " (" << delta << " ms)\n";
    }
  }
  else {
    for (int plies_to_search = 1; plies_to_search <= depth; plies_to_search++) {
      leaves = 0;
      auto start = curr_time_millis();
      MoveList<BLACK> movelist(true);
      for (Move m : movelist) {
        Piece captured = piece_on(to_sq(m));
        uint8_t c_rights = GameState::castling_rights;
        do_move<BLACK>(m);
        expand<WHITE>(plies_to_search - 1);
        undo_move<BLACK>(m, captured);
        GameState::castling_rights = c_rights;
      }
      auto delta = curr_time_millis() - start;
      std::cout << "\nDepth " << plies_to_search << ": " << leaves << " (" << delta << " ms)\n";
    }
  }

}
