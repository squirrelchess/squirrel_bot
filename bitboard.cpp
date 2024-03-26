
#include "bitboard.h"

int score_kingshield(Square ksq, Bitboard occ, Color c);

void init_magics(PieceType pt, int* base, Bitboard* masks, Bitboard* attacks, Bitboard* xray);

void Bitboards::init() {

  for (Square s1 = H1; s1 <= A8; s1++)
    for (Square s2 = H1; s2 <= A8; s2++)
      square_dist[s1][s2] = std::max(file_distance(s1, s2), rank_distance(s1, s2));

  init_magics(BISHOP, bishop_hash, bishop_masks, bishop_atk, bishopxray);
  init_magics(ROOK,   rook_hash,   rook_masks,   rook_atk,   rookxray  );

#define mdiag(s) (square_bb(s) | bishop_attacks(s, 0) & (mask(s, NORTH_WEST) | mask(s, SOUTH_EAST)))
#define adiag(s) (square_bb(s) | bishop_attacks(s, 0) & (mask(s, NORTH_EAST) | mask(s, SOUTH_WEST)))

#define md(a, b) (rank_distance(a, b) + file_distance(a, b))

  for (Square s1 = H1; s1 <= A8; s1++) {

    main_diagonal[s1] = mdiag(s1);
    anti_diagonal[s1] = adiag(s1);

    file[s1] = file_of(s1);
      
    center_dist[s1] = std::min({ md(s1, E4), md(s1, E5), md(s1, D4), md(s1, D5) });

    for (Square s2 = H1; s2 <= A8; s2++) {
      pinmask[s1][s2] =
        mdiag  (s1) & mdiag  (s2) | adiag  (s1) & adiag  (s2)
      | rank_of(s1) & rank_of(s2) | file_of(s1) & file_of(s2);
    }

    for (Square ksq = H1; ksq <= A8; ksq++) {
      for (Direction d : { NORTH_EAST, SOUTH_EAST,
                           SOUTH_WEST, NORTH_WEST }) {
        Bitboard bishop_ray = bishop_attacks(ksq, square_bb(s1)) & mask(ksq, d);
        if (bishop_ray & square_bb(s1))
          checkray[ksq][s1] = bishop_ray;
      }
      for (Direction d : { NORTH, EAST, SOUTH, WEST }) {
        Bitboard rook_ray = rook_attacks(ksq, square_bb(s1)) & mask(ksq, d);
        if (rook_ray & square_bb(s1))
          checkray[ksq][s1] = rook_ray;
      }
    }

    for (Direction d : { NORTH, NORTH_EAST, EAST, SOUTH_EAST,
                         SOUTH, SOUTH_WEST, WEST, NORTH_WEST })
      king_atk[s1] |= safe_step(s1, d);

    for (Direction d : { NORTHNORTH+EAST, NORTH_EAST+EAST, SOUTH_EAST+EAST, SOUTHSOUTH+EAST,
                         SOUTHSOUTH+WEST, SOUTH_WEST+WEST, NORTH_WEST+WEST, NORTHNORTH+WEST })
      knight_atk[s1] |= safe_step(s1, d);

    Square sq = 8 * (s1 / 8) + 1;

    white_kingshield[s1] =
      ((rank_of(sq + NORTH) | rank_of(sq + NORTHNORTH)) & ~(mask(sq + WEST, WEST))) << std::min(5, std::max(0, (s1 % 8) - 1));

    black_kingshield[s1] =
      ((rank_of(sq + SOUTH) | rank_of(sq + SOUTHSOUTH)) & ~(mask(sq + WEST, WEST))) << std::min(5, std::max(0, (s1 % 8) - 1));

    doublecheck[s1] = king_atk[s1] | knight_atk[s1];

    pawn_atk[WHITE][s1] = pawn_attacks<WHITE>(square_bb(s1));
    pawn_atk[BLACK][s1] = pawn_attacks<BLACK>(square_bb(s1));
  }

#undef fdiag
#undef bdiag
#undef md

  uint8_t clearK = 0b0111;
  uint8_t clearQ = 0b1011;
  uint8_t cleark = 0b1101;
  uint8_t clearq = 0b1110;

  Bitboard msk = square_bb(A1, E1, H1, A8, E8, H8);

  for (int p = 0; p < 1 << popcount(msk); p++) {

    Bitboard occ = generate_occupancy(msk, p);
    uint8_t rights_mask = 0b1111;

    if (!(occ & square_bb(H1))) rights_mask &= clearK;
    if (!(occ & square_bb(E1))) rights_mask &= clearK & clearQ;
    if (!(occ & square_bb(A1))) rights_mask &= clearQ;
    if (!(occ & square_bb(H8))) rights_mask &= cleark;
    if (!(occ & square_bb(E8))) rights_mask &= cleark & clearq;
    if (!(occ & square_bb(A8))) rights_mask &= clearq;

    castle_pext[pext(occ, msk)] = rights_mask;
  }

  for (Square sq = H1; sq <= A8; sq++) {
    for (int i = 0; i < 1 << popcount(white_kingshield[sq]); i++) {
      Bitboard occ = generate_occupancy(white_kingshield[sq], i);
      white_kingshield_scores[sq][pext(occ, white_kingshield[sq])] = score_kingshield(sq, occ, WHITE);
    }
    for (int i = 0; i < 1 << popcount(black_kingshield[sq]); i++) {
      Bitboard occ = generate_occupancy(black_kingshield[sq], i);
      black_kingshield_scores[sq][pext(occ, black_kingshield[sq])] = score_kingshield(sq, occ, BLACK);
    }
  }

}

int score_kingshield(Square ksq, Bitboard occ, Color c) {

  constexpr int MAX_SCORE =  50;
  constexpr int MIN_SCORE = -50;

  Bitboard home_rank = (c == WHITE) ? RANK_1 : RANK_8;

  if (!(square_bb(ksq) & home_rank) || (popcount(occ) < 2))
    return MIN_SCORE;
  if (square_bb(ksq) & (FILE_E | FILE_D))
    return 10;

  constexpr int pawn_weights[8][6] = 
  { // [H1..A1][LSB..MSB]
    {10, 20, 15, 5, 10, 5},{5,  25, 15, 0, 0,  5},
    {20, 15, 10, 0, 0,  0},{0,  0,  0,  0, 0,  0},
    {0,  0,  0,  0, 0,  0},{10, 15, 20, 0, 0,  0},
    {15, 25, 5,  5, 0,  0},{15, 20, 10, 5, 10, 5}
  };

  constexpr int file_weights[8][3] =
  { // [H1..A1][right, middle, left]
    {40, 50, 45},{40, 50, 45},
    {50, 45, 40},{0,  0,   0},
    {0,  0,   0},{40, 45, 50},
    {45, 50, 40},{45, 50, 40}
  };

  Bitboard shield_mask = (c == WHITE)
    ? white_kingshield[ksq]
    : black_kingshield[ksq];

  Bitboard file_right = file_of(lsb(shield_mask));
  Bitboard file_mid   = file_right << 1;
  Bitboard file_left  = file_right << 2;

  int score = 0;
  if (c == BLACK) ksq -= 56;
  if ((occ & file_right) == 0) score -= file_weights[ksq][0];
  if ((occ & file_mid)   == 0) score -= file_weights[ksq][1];
  if ((occ & file_left)  == 0) score -= file_weights[ksq][2];

  for (int i = 0; shield_mask; i++) {
    int index = (c == WHITE) ? i : ((i < 3) ? (i + 3) : (i - 3));
    Square sq = lsb(shield_mask);
    if (occ & square_bb(sq))
      score += pawn_weights[ksq][index];
    pop_lsb(shield_mask);
  }
  return std::max(MIN_SCORE, std::min(MAX_SCORE, score));
}

Bitboard sliding_attacks(PieceType pt, Square sq, Bitboard occupied) {

  Direction rook_dir  [4] = { NORTH,EAST,SOUTH,WEST };
  Direction bishop_dir[4] = { NORTH_EAST,SOUTH_EAST,
                              SOUTH_WEST,NORTH_WEST };
  Bitboard atk = 0;
  for (Direction d : (pt == ROOK) ? rook_dir : bishop_dir) {
    Square s = sq;
    while (safe_step(s, d) && !(square_bb(s) & occupied))
      atk |= square_bb(s += d);
  }
  return atk;

}

void init_magics(PieceType pt, int* base, Bitboard* masks, Bitboard* attacks, Bitboard* xray)
{
  int permutations, all_permutations = 0;

  for (Square sq = H1; sq <= A8; sq++) {

    base[sq] = all_permutations;

    Bitboard edges = (FILE_A | FILE_H) & ~file_of(sq) | (RANK_1 | RANK_8) & ~rank_of(sq);
    Bitboard mask = sliding_attacks(pt, sq, 0) & ~edges;
    masks[sq] = mask;

    all_permutations += permutations = 1 << popcount(mask);

    for (int p = 0; p < permutations; p++) {
      Bitboard occupied = generate_occupancy(mask, p);
      Bitboard attack_mask = sliding_attacks(pt, sq, occupied);
      int hash = pext(occupied, mask);
      attacks[base[sq] + hash] = attack_mask;
      xray[base[sq] + hash] = sliding_attacks(pt, sq, occupied ^ (attack_mask & occupied));
    }
  }
}
