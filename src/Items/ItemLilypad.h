
#pragma once

#include "ItemHandler.h"
#include "../Entities/Player.h"
#include "../LineBlockTracer.h"





class cItemLilypadHandler:
	public cItemHandler
{
	using Super = cItemHandler;

public:

	cItemLilypadHandler(int a_ItemType):
		Super(a_ItemType)
	{

	}





	virtual bool IsPlaceable(void) override
	{
		return false;  // Set as not placeable so OnItemUse is called
	}





	virtual bool OnItemUse(
		cWorld * a_World,
		cPlayer * a_Player,
		cBlockPluginInterface & a_PluginInterface,
		const cItem & a_HeldItem,
		const Vector3i a_ClickedBlockPos,
		eBlockFace a_ClickedBlockFace
	) override
	{
		if (a_ClickedBlockFace > BLOCK_FACE_NONE)
		{
			// Clicked on a face of a submerged block; vanilla allows placement, so should we
			auto PlacePos = AddFaceDirection(a_ClickedBlockPos, a_ClickedBlockFace);

			// Lilypad should not replace non air and non water blocks
			auto BlockToReplace = a_World->GetBlock(PlacePos);
			if ((BlockToReplace != E_BLOCK_AIR) &&
				(BlockToReplace != E_BLOCK_WATER) &&
				(BlockToReplace != E_BLOCK_STATIONARY_WATER))
			{
				return false;
			}

			// Lilypad should be placed only if there is a water block below
			auto BlockBelow = a_World->GetBlock(PlacePos - Vector3i(0, 1, 0));
			if ((BlockBelow != E_BLOCK_WATER) &&
				(BlockBelow != E_BLOCK_STATIONARY_WATER))
			{
				return false;
			}

			a_World->SetBlock(PlacePos, E_BLOCK_LILY_PAD, 0);
			if (!a_Player->IsGameModeCreative())
			{
				a_Player->GetInventory().RemoveOneEquippedItem();
			}
			return true;
		}

		class cCallbacks:
			public cBlockTracer::cCallbacks
		{
		public:

			cCallbacks():
				m_HasHitFluid(false)
			{
			}

			virtual bool OnNextBlock(Vector3i a_CBBlockPos, BLOCKTYPE a_CBBlockType, NIBBLETYPE a_CBBlockMeta, eBlockFace a_CBEntryFace) override
			{
				if (IsBlockWater(a_CBBlockType))
				{
					if ((a_CBBlockMeta != 0) || (a_CBEntryFace == BLOCK_FACE_NONE))  // The hit block should be a source. The FACE_NONE check is clicking whilst submerged
					{
						return false;
					}
					a_CBBlockPos = AddFaceDirection(a_CBBlockPos, BLOCK_FACE_YP);  // Always place pad at top of water block
					m_HasHitFluid = true;
					m_Pos = a_CBBlockPos;
					return true;
				}
				return false;
			}

			Vector3i m_Pos;
			bool m_HasHitFluid;

		} Callbacks;
		auto Start = a_Player->GetEyePosition() + a_Player->GetLookVector();
		auto End =   a_Player->GetEyePosition() + a_Player->GetLookVector() * 5;
		cLineBlockTracer::Trace(*a_Player->GetWorld(), Callbacks, Start, End);

		if (Callbacks.m_HasHitFluid)
		{
			// Lilypad should not replace non air blocks
			auto BlockToReplace = a_World->GetBlock(Callbacks.m_Pos);
			if (BlockToReplace != E_BLOCK_AIR)
			{
				return false;
			}

			a_World->SetBlock(Callbacks.m_Pos, E_BLOCK_LILY_PAD, 0);
			if (!a_Player->IsGameModeCreative())
			{
				a_Player->GetInventory().RemoveOneEquippedItem();
			}
			return true;
		}

		return false;
	}
};
