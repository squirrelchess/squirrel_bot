
#include "position.h"
#include "util.h"

#include <cstring>
#include <random>
#include <sstream>

Bitboard bitboards[B_KING + 1];
Piece board[SQUARE_NB];

uint64_t Zobrist::hash[B_KING + 1][SQUARE_NB];
uint64_t Zobrist::key;

void Position::init() {
  
  std::mt19937_64 rng(221564671644);

  for (Piece pc : { W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
                    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING })
  {
    for (Square sq = H1; sq <= A8; sq++)
      Zobrist::hash[pc][sq] = rng();
  }

}

void Position::set(const std::string& fen) {

  memset(bitboards, 0ull, 16 * sizeof(Bitboard));
  memset(board, NO_PIECE, 64 * sizeof(Piece   ));

  uint8_t token;
  Square sq = A8;
  size_t piece;

  std::istringstream ss(fen);

  while (ss >> token) {
    if (std::isdigit(token))
      sq -= token - '0';
    else if (std::isspace(token))
      break;
    else if ((piece = piece_to_char.find(token)) != std::string::npos) {;
      board[sq] = piece;
      bitboards[piece] ^= square_bb(sq);
      bitboards[color_of(piece)] ^= square_bb(sq);
      sq--;
    }
  }

  std::string color, castling;
  ss >> color >> castling;

  GameState::side_to_move = std::string("wb").find(color);

  st.castling_rights = 0;
  for (char c : castling)
    st.castling_rights ^= 1 << std::string("qkQk").find(c);

  st.key = GameState::side_to_move == WHITE ? 0 : Zobrist::Side;

  for (Square sq = H1; sq <= A8; sq++)
    st.key ^= Zobrist::hash[piece_on(sq)][sq];

  set_gamephase();

}

void set_gamephase() {

  int enemy_material = 0;
  int friendly_material = 0;

  if (GameState::us == WHITE) {
    enemy_material += 3 * popcount(bb(B_BISHOP) | bb(B_KNIGHT));
    enemy_material += 5 * popcount(bb(B_ROOK));
    enemy_material += 9 * popcount(bb(B_QUEEN));
    friendly_material += 3 * popcount(bb(W_BISHOP) | bb(W_KNIGHT));
    friendly_material += 5 * popcount(bb(W_ROOK));
    friendly_material += 9 * popcount(bb(W_QUEEN));

    GameState::mopup = (enemy_material < 5) && (friendly_material >= 5);
    GameState::endgame = (enemy_material < 10) || (enemy_material < 17 && bb(B_QUEEN) == 0);
  }
  else {
    enemy_material += 3 * popcount(bb(W_BISHOP) | bb(W_KNIGHT));
    enemy_material += 5 * popcount(bb(W_ROOK));
    enemy_material += 9 * popcount(bb(W_QUEEN));
    friendly_material += 3 * popcount(bb(B_BISHOP) | bb(B_KNIGHT));
    friendly_material += 5 * popcount(bb(B_ROOK));
    friendly_material += 9 * popcount(bb(B_QUEEN));

    GameState::mopup = (enemy_material < 5) && (friendly_material >= 5);
    GameState::endgame = (enemy_material < 10) || (enemy_material < 17 && bb(W_QUEEN) == 0);
  }
}