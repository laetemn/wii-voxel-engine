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

#ifndef _SPRITESTAGEMANAGER_H_
#define _SPRITESTAGEMANAGER_H_

#include "../core/grrlib.h"
#include "../textures/ISprite.h"
#include <unordered_map>
#include <map>
#include <vector>
#include <string>

class SpriteStageManager {

private:

    std::vector<const ISprite*> m_spriteRenderCash;
    bool m_spriteCashDirty = true;
    std::unordered_map<std::string, ISprite*> m_spriteAtlas;

public:
    SpriteStageManager() {}
    ~SpriteStageManager() {}   

    void Clear();    
    bool FindSprite(const std::string& name) const;

    ISprite* Add(ISprite* sprite);
    bool Remove(const ISprite& sprite);

    const ISprite* GetSprite(const std::string& key) const;

    std::vector<const ISprite*>& GetSpriteRenderList();

    uint32_t SpriteCount() const
    {
        return m_spriteAtlas.size();
    }

    void SetSpriteCashDirty(bool value)
    {
        m_spriteCashDirty = value;
    }   
};

#endif /* _SPRITESTAGEMANAGER_H_ */
