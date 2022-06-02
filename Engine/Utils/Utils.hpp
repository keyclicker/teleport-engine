#pragma once

// Vertical line
template <typename T = double>
struct Segment {
  T begin, end;
};

// Screen plane struct
struct Plain {
  Segment<> hSeg;
  Segment<> lSeg, rSeg;
};