#pragma once

struct ab_link {
  unsigned a;
  unsigned b;
};

template<unsigned I>
unsigned &get(ab_link &link) {
  static_assert(I < 2);
  if constexpr (I == 0) {
    return link.a;
  }
  if constexpr (I == 1) {
    return link.b;
  }
}

template<unsigned I>
const unsigned &get(const ab_link &link) {
  static_assert(I < 2);
  if constexpr (I == 0) {
    return link.a;
  }
  if constexpr (I == 1) {
    return link.b;
  }
}

// struct ab_key {
//   bool is_a() const { return (m_key) & 1; }
//
//   void set_a() { m_key |= 1; }
//
//   void set_b() { m_key &= ~(1); }
//
//   void flip_is_a() { m_key ^= (1); }
//
//   constexpr operator unsigned() const {
//     unsigned return_key = m_key >> 1;
//     return return_key;
//   }
//
//   constexpr ab_key() {};
//
//   constexpr ab_key(unsigned k) { m_key = k << 1; }
//
//   constexpr ab_key(unsigned k, bool b) {
//     m_key = k << 1;
//     m_key |= b;
//   }
//
// private:
//   unsigned m_key = 0;
// };
//
