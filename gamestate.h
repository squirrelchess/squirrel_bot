
#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "defs.h"
#include "repetitiontable.h"
#include "types.h"
#include "bitboard.h"

#include <vector>
#include <cstdint>
#include <string>

//                            epindx
//  00000000000000000000000000000000 <- boardstate
//                              KQkq
//                          00000000 <- uint8_t castling_rights

namespace GameState {

  extern uint8_t castling_rights;

  template<Color Perspective>
  uint8_t kingside_rights() {
    constexpr uint8_t msk = Perspective == WHITE ? 0b1000 : 0b0010;  
    return castling_rights & msk;
  }

  template<Color Perspective>
  uint8_t queenside_rights() {
    constexpr uint8_t msk = Perspective == WHITE ? 0b0100 : 0b0001;
    return castling_rights & msk;
  }

  inline RepetitionTable repetition_table;
  inline int boardstate;
  inline int boardstate_save;
  inline int halfmove_clock;
  inline bool endgame;
  inline bool mopup;
  inline bool white_to_move;
  inline bool white_computer;
  inline bool white_human;

  inline uint64_t current_ep_square() { return 1ull << (boardstate & 0x3f); }
  inline int rights_K() { return castling_rights & 0b1000; }
  inline int rights_Q() { return castling_rights & 0b0100; }
  inline int rights_k() { return castling_rights & 0b0010; }
  inline int rights_q() { return castling_rights & 0b0001; }

  void parse_fen(std::string fen);
  inline void init(std::string fen) { parse_fen(fen); }
  void update_gamephase();
  void diagnostic();
  void update_enpassant(int move);
  void update(int move);
  void restore();
  void checkmate(bool white_just_moved);

} // namespace GameState

#endif
