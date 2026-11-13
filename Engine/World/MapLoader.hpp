#pragma once

#include <string>

#include "../Graphics/Texture.hpp"
#include "Map.hpp"

namespace te {

/**
 * @brief Load a map from JSON
 *
 * Format, all lengths in world units, angles in degrees:
 *
 * ```json
 * {
 *   "styles": {
 *     "wood": { "middle": "wall.gif" },
 *     "door": { "upper": "brick.png", "lower": "brick.png" }
 *   },
 *   "sectors": [{
 *     "name": "hall",
 *     "floor": 0, "ceiling": 112,
 *     "floorTexture": "stone.png", "ceilingTexture": "ceiling.gif",
 *     "flatScale": 0.78, "light": 1,
 *     "style": "wood",
 *     "walls": [
 *       { "from": [400, 400] },
 *       { "from": [400, 0], "style": "door", "portal": ["tunnel", 0] }
 *     ]
 *   }],
 *   "things": [
 *     { "type": "player", "sector": "hall", "pos": [0, 0], "angle": 0 }
 *   ]
 * }
 * ```
 *
 * - Wall `i` runs from its `from` to the next wall's, the last one back
 *   to the first. Either winding works.
 * - Wall textures: `upper`, `middle`, `lower`, and `textureScale` (0
 *   stretches to fit). A wall's own keys beat its style's, which beat
 *   the sector's `style`.
 * - `portal`: partner as [sector name, wall index]. The partner links
 *   back on its own unless it names another portal. Partners must have
 *   equal length.
 * - Sectors must be convex.
 *
 * @param path JSON file
 * @param textures Resolves texture names
 * @throw std::runtime_error Bad file, with the reason and location
 */
Map loadMap(const std::string &path, TextureCache &textures);

} // namespace te
