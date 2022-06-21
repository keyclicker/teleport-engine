#pragma once

// Vertical line
template <typename T = double>
struct Segment {
  T begin, end;

  [[nodiscard]] T length() const {
    return end - begin;
  }
};

// Screen plane struct
struct Plane {
  Segment<> hSeg;
  Segment<> lSeg, rSeg;
};