// 
// Michael Shafae
// mshafae at fullerton.edu
// 
// Utility functions.
//
//

#define _USE_MATH_DEFINES
#include <cmath>

#include "utilities.h"

float deg2rad(float degrees){
  return degrees * (M_PI / 180.0);
}

float rad2deg(float radians){
  return radians * (180.0 / M_PI);
}
