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


#include <sstream>
#include "Chunk.h"
#include "../PerlinNoise.h"
#include "../../utils/MathHelper.h"
#include "../../renderer/MasterRenderer.h"
#include "../../renderer/BlockRenderer.h"
#include "../../utils/Debug.h"

Chunk::Chunk(class GameWorld& gameWorld) : m_bIsDirty(false)
{
	m_pWorldManager = &gameWorld;
}

Chunk::~Chunk()
{
    Clear();

    for (int x = 0; x < CHUNK_SIZE_X; ++x)
    {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y)
        {
            delete [] m_blocks[x][y];
        }

        delete [] m_blocks[x];
    }
    delete [] m_blocks;
}

void Chunk::Init()
{
    m_blocks = new BlockType**[CHUNK_SIZE_X];
	for (uint32_t x = 0; x < CHUNK_SIZE_X; x++)
	{
        m_blocks[x] = new BlockType*[CHUNK_SIZE_Y];

		for (uint32_t y = 0; y < CHUNK_SIZE_Y; y++)
        {
            m_blocks[x][y] = new BlockType[CHUNK_SIZE_Z];
		}
	}
}

void Chunk::Build()
{
    PerlinNoise pn = m_pWorldManager->GetNoise();

    for ( uint32_t x = 0; x < CHUNK_SIZE_X; x++)
    {
        for ( uint32_t z = 0; z < CHUNK_SIZE_Z; z++)
        {
            for ( uint32_t y = 0; y < CHUNK_SIZE_Y; y++)
            {
                m_blocks[x][y][z] = BlockType::AIR;
            }

            double xWorld, zWorld;
            xWorld = (m_centerPosition.GetX() - (CHUNK_BLOCK_SIZE_X / 2) + (x * BLOCK_SIZE));
            zWorld = (m_centerPosition.GetZ() - (CHUNK_BLOCK_SIZE_Z / 2) + (z * BLOCK_SIZE));

            double noise = pn.GetHeight(xWorld, zWorld);
            uint32_t height = (uint32_t) MathHelper::Clamp((CHUNK_SIZE_Y * noise) + CHUNK_MIN_GROUND, CHUNK_MIN_GROUND, CHUNK_SIZE_Y);

            for (uint32_t y = 0; y < height; y++)
            {
                if ( y == height - 1 )
                {
                    m_blocks[x][y][z] = BlockType::GRASS;
                }
                else if (y <= STONE_LEVEL )
                {
                    m_blocks[x][y][z] = BlockType::STONE;
                }
                else
                {
                    m_blocks[x][y][z] = BlockType::DIRT;
                }
            }
        }
    }

    CreateTrees();
}



void Chunk::Clear()
{
    DeleteDisplayList();
    ClearBlockRenderList();
}

void Chunk::CreateTrees()
{
    //srand (time(NULL));
    uint32_t x = 6;//2 + (rand() % (CHUNK_SIZE_X - 4)); // value range 2 - 14
    //srand (time(NULL));
    uint32_t z = 8;// + (rand() % (CHUNK_SIZE_Z - 4));

    double xWorld = (m_centerPosition.GetX() - (CHUNK_BLOCK_SIZE_X / 2) + (x * BLOCK_SIZE));
    double zWorld = (m_centerPosition.GetZ() - (CHUNK_BLOCK_SIZE_Z / 2) + (z * BLOCK_SIZE));

    double noise = m_pWorldManager->GetNoise().GetHeight(xWorld, zWorld);

    uint32_t y = (uint32_t) MathHelper::Clamp((CHUNK_SIZE_Y * noise) + CHUNK_MIN_GROUND, CHUNK_MIN_GROUND, CHUNK_SIZE_Y);

    if ( y < CHUNK_SIZE_Y - TREE_HIGHT)
    {
        if ( m_blocks[x][y-1][z] == BlockType::GRASS)
        {

            for (int8_t i = -2; i <= 2; i++)
            {
                for (int8_t j = -2; j <= 2; j++)
                {
                    m_blocks[x+i][y+3][z+j] = BlockType::LEAF;
                    m_blocks[x+i][y+4][z+j] = BlockType::LEAF;
                }
            }

            for (int8_t i = -1; i <= 1; i++)
            {
                for (int8_t j = -1; j <= 1; j++)
                {
                    m_blocks[x+i][y+5][z+j] = BlockType::LEAF;
                    m_blocks[x+i][y+6][z+j] = ((i % 2) == 0) || ((j % 2) == 0) ? (BlockType::LEAF) : (BlockType::AIR);
                }
            }


            for (uint32_t i = 0; i < 3; i++)
            {
                m_blocks[x][y+i][z] = BlockType::WOOD;
            }
        }
    }
}

void Chunk::SetChunkNeighbors()
{
    Vector3 leftChunkPos(m_centerPosition.GetX() - CHUNK_BLOCK_SIZE_X, m_centerPosition.GetY(), m_centerPosition.GetZ());
    Vector3 rightChunkPos(m_centerPosition.GetX() + CHUNK_BLOCK_SIZE_X, m_centerPosition.GetY(), m_centerPosition.GetZ());
    Vector3 frontChunkPos(m_centerPosition.GetX(), m_centerPosition.GetY(), m_centerPosition.GetZ() + CHUNK_BLOCK_SIZE_Z);
    Vector3 backChunkPos(m_centerPosition.GetX(), m_centerPosition.GetY(), m_centerPosition.GetZ() - CHUNK_BLOCK_SIZE_X);

    m_pChunkLeft  =  m_pWorldManager->GetCashedChunkAt(leftChunkPos);
    m_pChunkRight =  m_pWorldManager->GetCashedChunkAt(rightChunkPos);
    m_pChunkFront =  m_pWorldManager->GetCashedChunkAt(frontChunkPos);
    m_pChunkBack  =  m_pWorldManager->GetCashedChunkAt(backChunkPos);
}

bool Chunk::NeighborsLoaded()
{
    if(m_pChunkLeft && m_pChunkLeft->IsLoaded())
        return true;

    if(m_pChunkRight && m_pChunkRight->IsLoaded())
        return true;

    if(m_pChunkFront && m_pChunkFront->IsLoaded())
        return true;

    if(m_pChunkBack && m_pChunkBack->IsLoaded())
        return true;

    return false;
}

void Chunk::Render()
{
    if ( m_displayListSize > 0 )
	{
        GX_CallDispList(m_pDispList, m_displayListSize);
    }
}

bool Chunk::AddBlockToRenderList(BlockType type, const BlockRenderVO& blockRenderVO)
{
	bool bSuccessful = false;
	auto blockRenderListIt = m_mBlockRenderList.find(type);
	if (blockRenderListIt != m_mBlockRenderList.end())
	{
        blockRenderListIt->second.emplace_back(blockRenderVO);
		bSuccessful = true;
	}
	else
	{
        std::vector<BlockRenderVO> blockList;
        blockList.emplace_back(blockRenderVO);
        m_mBlockRenderList.insert(std::pair<BlockType, std::vector<BlockRenderVO> >(type, blockList ));
		bSuccessful = true;
	}

	return bSuccessful;
}

bool Chunk::IsBlockVisible(uint32_t iX, uint32_t iY, uint32_t iZ, BlockRenderVO& blockRenderVO)
{
    bool bIsAir = m_blocks[iX][iY][iZ] == BlockType::AIR;

	if ( iX == 0 )
	{
		if (bIsAir)
		{
            if ( !m_bNeighbourUpdate && m_pChunkLeft && m_pChunkLeft->m_blocks[CHUNK_SIZE_X -1][iY][iZ] != BlockType::AIR)
			{
				m_pChunkLeft->SetDirty(true); //rebuild neighbour
				m_pChunkLeft->m_bNeighbourUpdate = true;
			}
		}
        else if ( /*!m_pChunkLeft || */(m_pChunkLeft && m_pChunkLeft->m_blocks[CHUNK_SIZE_X -1][iY][iZ] == BlockType::AIR))
		{
            blockRenderVO.FaceMask |= LEFT_FACE;
            blockRenderVO.Faces++;
		}
	}

	if ( iX == CHUNK_SIZE_X -1 )
	{
		if (bIsAir)
		{
            if ( !m_bNeighbourUpdate && m_pChunkRight && m_pChunkRight->m_blocks[0][iY][iZ] != BlockType::AIR)
			{
				m_pChunkRight->SetDirty(true); //rebuild neighbour
				m_pChunkRight->m_bNeighbourUpdate = true;
			}
		}
        else if ( /*!m_pChunkRight || */(m_pChunkRight && m_pChunkRight->m_blocks[0][iY][iZ] == BlockType::AIR))
		{
            blockRenderVO.FaceMask |= RIGHT_FACE;
            blockRenderVO.Faces++;
		}
	}

	if ( iZ == 0 )
	{
		if (bIsAir)
		{
            if ( !m_bNeighbourUpdate && m_pChunkBack && m_pChunkBack->m_blocks[iX][iY][CHUNK_SIZE_Z -1] != BlockType::AIR)
			{
				m_pChunkBack->SetDirty(true); //rebuild neighbour
				m_pChunkBack->m_bNeighbourUpdate = true;
			}
		}
        else if (/*!m_pChunkBack || */(m_pChunkBack && m_pChunkBack->m_blocks[iX][iY][CHUNK_SIZE_Z -1] == BlockType::AIR))
		{
            blockRenderVO.FaceMask |= BACK_FACE;
            blockRenderVO.Faces++;
		}
	}

	if (iZ == CHUNK_SIZE_Z -1 )
	{
		if (bIsAir)
		{
            if ( !m_bNeighbourUpdate && m_pChunkFront && m_pChunkFront->m_blocks[iX][iY][0] != BlockType::AIR)
			{
				m_pChunkFront->SetDirty(true); //rebuild neighbour
				m_pChunkFront->m_bNeighbourUpdate = true;
			}
		}
        else if ( /*!m_pChunkFront || */(m_pChunkFront && m_pChunkFront->m_blocks[iX][iY][0] == BlockType::AIR))
		{
            blockRenderVO.FaceMask |= FRONT_FACE;
            blockRenderVO.Faces++;
		}
	}

	if ( !bIsAir )
	{
		if ( iY == CHUNK_SIZE_Y -1 )
		{
            blockRenderVO.FaceMask |= TOP_FACE;
            blockRenderVO.Faces++;
		}

		 // Check all 6 block faces if neighbor is air
        if ( iX + 1 <= CHUNK_SIZE_X -1 && m_blocks[iX + 1][iY][iZ] == BlockType::AIR)
		{
            blockRenderVO.FaceMask |= RIGHT_FACE;
            blockRenderVO.Faces++;
		}

        if ( iX > 0 && m_blocks[iX - 1][iY][iZ] == BlockType::AIR)
		{
            blockRenderVO.FaceMask |= LEFT_FACE;
            blockRenderVO.Faces++;
		}

        if ( iY + 1 <= CHUNK_SIZE_Y -1 && m_blocks[iX][iY + 1][iZ] == BlockType::AIR)
		{
            blockRenderVO.FaceMask |= TOP_FACE;
            blockRenderVO.Faces++;
		}


        if (iY > 0 && m_blocks[iX][iY - 1][iZ] == BlockType::AIR)
		{
            blockRenderVO.FaceMask |= BOTTOM_FACE;
            blockRenderVO.Faces++;
		}

        if ( iZ + 1 <= CHUNK_SIZE_Z -1 && m_blocks[iX][iY][iZ + 1] == BlockType::AIR)
		{
            blockRenderVO.FaceMask |= FRONT_FACE;
            blockRenderVO.Faces++;
		}

        if ( iZ > 0 && m_blocks[iX][iY][iZ - 1] == BlockType::AIR)
		{
            blockRenderVO.FaceMask |= BACK_FACE;
            blockRenderVO.Faces++;
        }
	}

    if (blockRenderVO.Faces > 0)
    {
        const Vector3& worldBlockPos = LocalPositionToGlobalPosition(Vec3i{ iX, iY, iZ });
        blockRenderVO.BlockPosition = Vector3(worldBlockPos.GetX(), worldBlockPos.GetY(), worldBlockPos.GetZ());
        return true;
    }

    return false;
}

void Chunk::BuildBlockRenderList()
{
    m_amountOfBlocks = 0;
    m_amountOfFaces = 0;

	for ( uint32_t x = 0; x < CHUNK_SIZE_X; x++)
	{
		for ( uint32_t y = 0; y < CHUNK_SIZE_Y; y++ )
		{
			for ( uint32_t z = 0; z < CHUNK_SIZE_Z; z++)
			{                
                BlockRenderVO renderVO;
                if ( IsBlockVisible(x, y, z, renderVO))
				{
                    AddBlockToRenderList(m_blocks[x][y][z], renderVO);
                    m_amountOfBlocks++;
                    m_amountOfFaces += renderVO.Faces;
                }
			}
		}
	}
}

void Chunk::ClearBlockRenderList()
{
	m_mBlockRenderList.clear();
}


void Chunk::CreateDisplayList(size_t sizeOfDisplayList)
{
    m_pDispList = memalign(32, sizeOfDisplayList);
    memset(m_pDispList, 0, sizeOfDisplayList);
    DCInvalidateRange(m_pDispList, sizeOfDisplayList);
    GX_BeginDispList(m_pDispList, sizeOfDisplayList);
}

void Chunk::FinishDisplayList()
{
    m_displayListSize = GX_EndDispList();
    m_bIsDirty = false;
    // Update display list size to the size returned by GX_EndDispList() to save memory
    realloc(m_pDispList, m_displayListSize);
}


bool Chunk::IsDirty() const
{
    return m_bIsDirty;
}

void Chunk::SetDirty(bool dirty)
{
    m_bIsDirty = dirty;
}

uint32_t Chunk::GetDisplayListSize() const
{
    return m_displayListSize;
}

uint64_t Chunk::GetAmountOfBlocks() const
{
    return m_amountOfBlocks;
}

uint64_t Chunk::GetAmountOfFaces() const
{
    return m_amountOfFaces;
}

const Vector3& Chunk::GetCenterPosition() const
{
    return m_centerPosition;
}

void Chunk::DeleteDisplayList()
{
    if ( m_displayListSize > 0 )
	{
        free(m_pDispList);
        m_displayListSize = 0;
        m_pDispList = nullptr;
        m_bIsDirty = true;
	}
}

void Chunk::RebuildDisplayList()
{
	BlockRenderer blockRenderer;

	ClearBlockRenderList();        
	BuildBlockRenderList();    

	DeleteDisplayList();
    CreateDisplayList( MasterRenderer::GetDisplayListSizeForFaces(m_amountOfFaces) );

    for(auto it = m_mBlockRenderList.begin(); it != m_mBlockRenderList.end(); ++it)
	{
        Block* pBlockToRender = m_pWorldManager->GetBlockManager().GetBlockByType(it->first);
        blockRenderer.Prepare( &it->second, *pBlockToRender);
        blockRenderer.Draw();
	}

    blockRenderer.Finish();

	FinishDisplayList();
	ClearBlockRenderList();

	m_bNeighbourUpdate = false;
}

void Chunk::RemoveBlockByWorldPosition(const Vector3& blockPosition)
{
	Vec3i vec = GetLocalBlockPositionByWorldPosition(blockPosition);

    if ( m_blocks[vec.X][vec.Y][vec.Z] != BlockType::AIR)
    {
        m_blocks[vec.X][vec.Y][vec.Z] = BlockType::AIR;
        BlockListUpdated( BlockChangeData { GetFilePath(), BlockType::AIR, vec, m_centerPosition });
    }
}

void Chunk::AddBlockByWorldPosition(const Vector3& blockPosition, BlockType type)
{
	Vec3i vec = GetLocalBlockPositionByWorldPosition(blockPosition);

    if ( m_blocks[vec.X][vec.Y][vec.Z] == BlockType::AIR)
	{
         m_blocks[vec.X][vec.Y][vec.Z] = type;
         BlockListUpdated( BlockChangeData { GetFilePath(), type, vec, m_centerPosition } );
	}
}

void Chunk::BlockListUpdated(const BlockChangeData& data)
{
    m_bIsDirty = true;
    m_pWorldManager->Serialize(data);
}

void Chunk::SetCenterPosition(const Vector3 &centerPosition)
{
    m_centerPosition = centerPosition;
}

void Chunk::SetLoaded(bool value)
{
    m_mutex.Lock();
    m_bLoadingDone = value;
    m_mutex.Unlock();
}

bool Chunk::IsLoaded()
{
    m_mutex.Lock();
    bool bLoaded = m_bLoadingDone;
    m_mutex.Unlock();
    return bLoaded;
}

Vec3i Chunk::GetLocalBlockPositionByWorldPosition(const Vector3& blockWorldPosition) const
{
    Vec3i vec;
    float blockScale = BLOCK_SIZE;

    vec.X = (uint32_t) (MathHelper::Mod(blockWorldPosition.GetX(), CHUNK_BLOCK_SIZE_X) / blockScale);
    vec.Y = (uint32_t) (MathHelper::Mod(blockWorldPosition.GetY(), CHUNK_BLOCK_SIZE_Y) / blockScale);
    vec.Z = (uint32_t) (MathHelper::Mod(blockWorldPosition.GetZ(), CHUNK_BLOCK_SIZE_Z) / blockScale);

	return vec;
}

Vector3 Chunk::GetBlockPositionByWorldPosition(const Vector3& worldPosition) const
{
	Vec3i vec = GetLocalBlockPositionByWorldPosition(worldPosition);    
	return LocalPositionToGlobalPosition(vec);
}

BlockType Chunk::GetBlockTypeByWorldPosition(const Vector3& worldPosition) const
{
	Vec3i vec = GetLocalBlockPositionByWorldPosition(worldPosition);
    return m_blocks[vec.X][vec.Y][vec.Z];
}

Vector3 Chunk::GetPhysicalPosition(const Vector3& position) const
{
    Vector3 pos = GetBlockPositionByWorldPosition(position);
    double currentPos = CHUNK_BLOCK_SIZE_Y - (BLOCK_SIZE);
	bool bValidated = false;
	do
	{
		pos.SetY(currentPos);
        bValidated = (GetBlockTypeByWorldPosition(pos) != BlockType::AIR) || (currentPos == 0);
        /*  quick physics fix for the tree update
         *  Player shouldn't jump higher than blocksize
         */
        if (bValidated)
        {
            double distance = position.GetY() - currentPos;
            bValidated = distance > (-BLOCK_SIZE);
        }
		currentPos -= BLOCK_SIZE;

	} while(!bValidated);

    return pos;
}

std::string Chunk::GetFilePath() const
{
    std::ostringstream filename;
    filename << WORLD_PATH "/";
    filename << m_centerPosition.GetX();
    filename << '_';
    filename << m_centerPosition.GetY();
    filename << '_';
    filename << m_centerPosition.GetZ();
    filename << ".dat";
    return filename.str();
}

Vector3 Chunk::LocalPositionToGlobalPosition(const Vec3i& localPosition) const
{
    Vector3 vec( (double)(m_centerPosition.GetX() - (CHUNK_BLOCK_SIZE_X / 2) + (double)(localPosition.X * BLOCK_SIZE)),
            (double)(m_centerPosition.GetY() - (CHUNK_BLOCK_SIZE_Y / 2) + (double)(localPosition.Y * BLOCK_SIZE)),
            (double)(m_centerPosition.GetZ() - (CHUNK_BLOCK_SIZE_Z / 2) + (double)(localPosition.Z * BLOCK_SIZE)));

    return vec;
}
