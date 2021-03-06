/***
 *
 * Copyright (C) 2016 DaeFennek
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
***/

#ifndef _BLOCK_H_
#define _BLOCK_H_

#include <vector>
#include <map>
#include "../../textures/Texture.h"

enum EBlockFaces
{
    Left,
    Right,
    Front,
    Back,
    Top,
    Bottom
};

class Block {
public:
    Block();
    Block( float size, std::map<const Texture*, std::vector<EBlockFaces>> textureMap);
    virtual ~Block();
	float GetSize() const;
    const std::map<const Texture*, std::vector<EBlockFaces>>& GetTextures() const;

protected:
	float m_size; // the size from the middle point to each axis   
    std::map<const Texture*, std::vector<EBlockFaces>> m_textureMap;

};

#endif /* _BLOCK_H_ */
