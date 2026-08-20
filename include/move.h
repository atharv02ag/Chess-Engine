#pragma once

#include "globals.h"

#include <type_traits>

class move {
  public:
    // Packed move layout (least-significant bit first):
    //
    //   bits  0.. 5  source square (0 = a1, 63 = h8)
    //   bits  6..11  destination square (0 = a1, 63 = h8)
    //   bits 12..14  PIECE value
    //   bits 15..16  COLOUR value
    //   bits 17..24  MOVE_FLAGS bit mask
    //   bit      25  source/destination fields are present
    //   bits 26..31  reserved; currently zero for constructed moves
    //
    // The coordinate-present bit lets the default/invalid move retain zero
    // bitboards even though square zero (a1) is itself a valid square.
    constexpr move() = default;
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const PIECE &prom_piece);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap,
         const PIECE &prom_piece);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap);
    move(const PIECE &p, const COLOUR &c, const u64 &start, const u64 &end, const bool &p_cap, const bool &ep);
    move(const PIECE &p, const COLOUR &c, const bool &castle_k, const bool &castle_q);

    constexpr u32 packed() const { return data_; }
    static constexpr move unpack(u32 packed_move) {
        return move(packed_move, packed_tag{});
    }

    constexpr PIECE piece() const {
        return static_cast<PIECE>((data_ >> PIECE_SHIFT) & PIECE_MASK);
    }
    constexpr COLOUR colour() const {
        return static_cast<COLOUR>((data_ >> COLOUR_SHIFT) & COLOUR_MASK);
    }
    constexpr u32 flags() const { return (data_ >> FLAGS_SHIFT) & FLAGS_MASK; }
    constexpr bool has_coordinates() const { return (data_ & COORDINATES_PRESENT_MASK) != 0; }
    constexpr int start_square() const {
        return static_cast<int>((data_ >> START_SHIFT) & SQUARE_MASK);
    }
    constexpr int end_square() const {
        return static_cast<int>((data_ >> END_SHIFT) & SQUARE_MASK);
    }
    constexpr u64 start_location() const {
        return has_coordinates() ? (1ULL << start_square()) : 0ULL;
    }
    constexpr u64 end_location() const {
        return has_coordinates() ? (1ULL << end_square()) : 0ULL;
    }

    bool piece_captured() const;
    bool enpass() const;
    bool castle_kside() const;
    bool castle_qside() const;
    PIECE promotion_piece() const;
    constexpr bool operator==(const move &other) const { return data_ == other.data_; }
    void print_move() const;

  private:
    static constexpr int START_SHIFT = 0;
    static constexpr int END_SHIFT = 6;
    static constexpr int PIECE_SHIFT = 12;
    static constexpr int COLOUR_SHIFT = 15;
    static constexpr int FLAGS_SHIFT = 17;
    static constexpr int COORDINATES_PRESENT_SHIFT = 25;

    static constexpr u32 SQUARE_MASK = 0x3fU;
    static constexpr u32 PIECE_MASK = 0x07U;
    static constexpr u32 COLOUR_MASK = 0x03U;
    static constexpr u32 FLAGS_MASK = 0xffU;
    static constexpr u32 COORDINATES_PRESENT_MASK = 1U << COORDINATES_PRESENT_SHIFT;

    struct packed_tag {};

    constexpr move(u32 packed_move, packed_tag) : data_(packed_move) {}
    static u32 encode(PIECE piece, COLOUR colour, u64 start, u64 end, u32 flags,
                      bool coordinates_present = true);
    static u32 promotion_flag(PIECE promotion_piece);

    u32 data_ = static_cast<u32>(COLOUR::NONE) << COLOUR_SHIFT;
};

static_assert(sizeof(move) == sizeof(u32), "move must occupy exactly 32 bits");
static_assert(std::is_trivially_copyable_v<move>, "packed moves should be cheap to copy");
