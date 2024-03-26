
#include "evaluation.h"
#include "util.h"
#include "position.h"
#include "gamestate.h"

ForceInline int midgame();
int endgame();
ForceInline int pawn_advancement();
ForceInline int king_safety();
int mopup();
ForceInline int material_count();
ForceInline int piece_placement();

int static_eval() {
  return GameState::endgame ? endgame() : midgame();
}

ForceInline int midgame() {
  return material_count() + king_safety() + pawn_advancement() + piece_placement();
}

ForceInline int piece_placement() {
  int score = 0;
  for (PieceType pt : {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {

    for (Bitboard b = bitboards[pt]; b; pop_lsb(b))
      score += square_score<WHITE>(pt, lsb(b));

    for (Bitboard b = bitboards[pt + 8]; b; pop_lsb(b))
      score -= square_score<BLACK>(pt, lsb(b));
  }
  return score;
}

ForceInline int pawn_advancement() {

  constexpr Bitboard RANK_567 = RANK_5 | RANK_6 | RANK_7;
  constexpr Bitboard RANK_234 = RANK_2 | RANK_3 | RANK_4;

  return 4 * (popcount(bb(W_PAWN) & RANK_567) 
           -  popcount(bb(B_PAWN) & RANK_234));
}

ForceInline int king_safety() {

  Square wksq = lsb(bb(W_KING));
  Square bksq = lsb(bb(B_KING));

  return king_safety<WHITE>(wksq, bb(W_PAWN))
       - king_safety<BLACK>(bksq, bb(B_PAWN));
}

int endgame() {

  if (GameState::mopup) return mopup();
                
  int score = material_count();

  score += end_king_squares[lsb(bb(W_KING))];
  score -= end_king_squares[lsb(bb(B_KING))];
  score += 10 * popcount(bb(W_PAWN) & RANK_4);
  score += 20 * popcount(bb(W_PAWN) & RANK_5);
  score += 50 * popcount(bb(W_PAWN) & RANK_6);
  score += 90 * popcount(bb(W_PAWN) & RANK_7);
  score -= 10 * popcount(bb(B_PAWN) & RANK_5);
  score -= 20 * popcount(bb(B_PAWN) & RANK_4);
  score -= 50 * popcount(bb(B_PAWN) & RANK_3);
  score -= 90 * popcount(bb(B_PAWN) & RANK_2);

  return score;

}

int mopup() {

  int score = 0;
  if (GameState::white_computer) {
    score += distance_from_center(lsb(bb(B_KING))) * 10;
    score += (14 - square_distance(lsb(bb(W_KING)),lsb(bb(B_KING)))) * 4;
    return score + material_count();
  }
  score -= distance_from_center(lsb(bb(W_KING))) * 10;
  score -= (14 - square_distance(lsb(bb(W_KING)),lsb(bb(B_KING)))) * 4;
  return score + material_count();

}

ForceInline int material_count() {

  int score = 100 * (popcount(bb(W_PAWN))                  - popcount(bb(B_PAWN)));
  score    += 300 * (popcount(bb(W_KNIGHT) | bb(W_BISHOP)) - popcount(bb(B_KNIGHT) | bb(B_BISHOP)));
  score    += 500 * (popcount(bb(W_ROOK))                  - popcount(bb(B_ROOK)));
  score    += 900 * (popcount(bb(W_QUEEN))                 - popcount(bb(B_QUEEN)));
  return score;

}
