// Copyright (c) 2015 Blowmorph Team

#include "client/utils.h"

#include <cmath>

#include <SFML/Graphics.hpp>

namespace bm {

sf::Vector2f Round(const sf::Vector2f& vector) {
  return sf::Vector2f(floor(vector.x), floor(vector.y));
}

}  // namespace bm
