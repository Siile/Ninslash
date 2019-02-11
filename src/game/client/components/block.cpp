#include <base/math.h>
#include <engine/shared/config.h>

#include <game/generated/client_data.h>
#include <game/client/render.h>

#include <game/client/components/camera.h>
#include <game/client/components/effects.h>

#include <game/gamecore.h>
#include "block.h"

	
CBlocks::CBlocks()
{
	m_pBlocks = NULL;
	m_pBlockSyncTick = NULL;
	m_Width = 0;
	m_Height = 0;
}

CBlocks::~CBlocks()
{
	if (m_pBlocks)
		delete m_pBlocks;
	
	if (m_pBlockSyncTick)
		delete m_pBlockSyncTick;
	
	m_pBlocks = NULL;
	m_pBlockSyncTick = NULL;
}

void CBlocks::OnInit()
{
	ResetBlocks();
}

void CBlocks::OnReset()
{
	ResetBlocks();
}

void CBlocks::OnMapLoad()
{
	ResetBlocks();
}

void CBlocks::ResetBlocks()
{
	if (m_pBlocks)
		delete m_pBlocks;
	
	m_Width = Collision()->GetWidth();
	m_Height = Collision()->GetHeight();
	
	m_pBlocks = new int[m_Width*m_Height];
	m_pBlockSyncTick = new int[m_Width*m_Height];
	for (int i = 0; i < m_Width*m_Height; i++)
	{
		m_pBlocks[i] = 0;
		m_pBlockSyncTick[i] = 0;
	}
}
	
	
void CBlocks::SetBlock(ivec2 Pos, int Block)
{
	if (!m_pBlocks)
		ResetBlocks();
	
	int Nx = clamp(Pos.x/32, 0, m_Width-1);
	int Ny = clamp(Pos.y/32, 0, m_Height-1);
	
	m_pBlocks[Ny*m_Width+Nx] = Block;
	m_pBlockSyncTick[Ny*m_Width+Nx] = Client()->GameTick();
	
	Collision()->SetBlock(Pos, Block);
}


void CBlocks::RenderBlocks()
{
	if (!m_Width)
		return;
	
	/*
	CUIRect Screen = *UI()->Screen();
	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);
	MapscreenToGroup(0, 0, Layers()->GameGroup());
	Graphics()->BlendNormal();
	*/
	
	vec2 Center = m_pClient->m_pCamera->m_Center / 32;

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BLOCKS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	float ScreenX0, ScreenY0, ScreenX1, ScreenY1;
	Graphics()->GetScreen(&ScreenX0, &ScreenY0, &ScreenX1, &ScreenY1);
	
	//int w = Graphics()->ScreenWidth()/32/2+4;
	//int h = Graphics()->ScreenHeight()/32/2+3;
	int w = int(ScreenX1-ScreenX0)/64+1; //ScreenX1/32-1;
	int h = int(ScreenY1-ScreenY0)/64+1; //ScreenY1/32-1;
	
	int x1 = clamp(int(Center.x) - w, 1, m_Width-2);
	int x2 = clamp(int(Center.x) + w, 1, m_Width-2);
	int y1 = clamp(int(Center.y) - h, 1, m_Height-2);
	int y2 = clamp(int(Center.y) + h, 1, m_Height-2);
	
	const float Size = 32.0f;
	
	for (int x = x1; x <= x2; x++)
	{
		for (int y = y1; y <= y2; y++)
		{
			if (m_pBlocks[y*m_Width+x])
			{
				if (m_pBlockSyncTick[y*m_Width+x] > Client()->GameTick() || m_pBlockSyncTick[y*m_Width+x] < Client()->GameTick() - Client()->GameTickSpeed() * 6.0f)
				{
					m_pBlocks[y*m_Width+x] = 0;
					m_pBlockSyncTick[y*m_Width+x] = 0;
					Collision()->SetBlock(ivec2(x*32, y*32), 0);
					continue;
				}
					
				
				vec2 p = vec2(x, y)*32;
				
				const float PixX = 1/256.0f;
				const float PixY = 1/128.0f;
				
				const float x0 = 0.0f;
				const float x1 = 1.0f/4.0f;
				const float x2 = 2.0f/4.0f;
				const float x3 = 3.0f/4.0f;
				
				const float y0 = 0.0f;
				const float y1 = 1.0f/2.0f;
				const float y2 = 1.0f;
				
				// fill
				{
					// block 1
					if (m_pBlocks[y*m_Width+x] == 1)
						Graphics()->QuadsSetSubsetFree(	x0+PixX, y0+PixY,
														x1-PixX, y0+PixY,
														x0+PixX, y1-PixY,
														x1-PixX, y1-PixY);
					else if (m_pBlocks[y*m_Width+x] == 2)
						Graphics()->QuadsSetSubsetFree(	x1+PixX, y0+PixY,
														x2-PixX, y0+PixY,
														x1+PixX, y1-PixY,
														x2-PixX, y1-PixY);
					else if (m_pBlocks[y*m_Width+x] == 3)
						Graphics()->QuadsSetSubsetFree(	x2+PixX, y0+PixY,
														x3-PixX, y0+PixY,
														x2+PixX, y1-PixY,
														x3-PixX, y1-PixY);
					// block 2
					else if (m_pBlocks[y*m_Width+x] == 4)
						Graphics()->QuadsSetSubsetFree(	x0+PixX, y1+PixY,
														x1-PixX, y1+PixY,
														x0+PixX, y2-PixY,
														x1-PixX, y2-PixY);
					else if (m_pBlocks[y*m_Width+x] == 5)
						Graphics()->QuadsSetSubsetFree(	x1+PixX, y1+PixY,
														x2-PixX, y1+PixY,
														x1+PixX, y2-PixY,
														x2-PixX, y2-PixY);
					else if (m_pBlocks[y*m_Width+x] == 6)
						Graphics()->QuadsSetSubsetFree(	x2+PixX, y1+PixY,
														x3-PixX, y1+PixY,
														x2+PixX, y2-PixY,
														x3-PixX, y2-PixY);
													
													
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y,
						p.x+Size, p.y,
						p.x, p.y+Size,
						p.x+Size, p.y+Size);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (m_pBlocks[y*m_Width+x] > 3)
					continue;
				
				// borders
				int Top = m_pBlocks[(y-1)*m_Width+x];
				int Bot = m_pBlocks[(y+1)*m_Width+x];
				int Left = m_pBlocks[y*m_Width+x-1];
				int Right = m_pBlocks[y*m_Width+x+1];
				
				if (Top > 3) Top = 0;
				if (Bot > 3) Bot = 0;
				if (Left > 3) Left = 0;
				if (Right > 3) Right = 0;
				
				const float bx0 = 6.0f/8.0f;
				const float bx1 = 7.0f/8.0f;
				const float bx2 = 1.0f;
				
				Graphics()->QuadsSetSubsetFree(	bx0+PixX, 0.25f+PixY,
												bx1-PixX, 0.25f+PixY,
												bx0+PixX, 0.5f-PixY,
												bx1-PixX, 0.5f-PixY);

				if (!Left && !Top)
				{
					p = vec2(x, y)*32;
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y,
						p.x, p.y,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (!Right && !Top)
				{
					p = vec2(x, y)*32+vec2(Size/2, 0);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y,
						p.x+Size/2, p.y,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (!Left && !Bot)
				{
					p = vec2(x, y)*32+vec2(0, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y,
						p.x, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (!Right && !Bot)
				{
					p = vec2(x, y)*32+vec2(Size/2, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y,
						p.x+Size/2, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				
				Graphics()->QuadsSetSubsetFree(	bx0+PixX, 0.0f+PixY,
												bx1-PixX, 0.0f+PixY,
												bx0+PixX, 0.25f-PixY,
												bx1-PixX, 0.25f-PixY);
				

				if (Left && !Top)
				{
					p = vec2(x, y)*32;
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y,
						p.x, p.y,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (Right && !Top)
				{
					p = vec2(x, y)*32+vec2(Size/2, 0);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y,
						p.x+Size/2, p.y,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (Left && !Bot)
				{
					p = vec2(x, y)*32+vec2(0, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y,
						p.x, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (Right && !Bot)
				{
					p = vec2(x, y)*32+vec2(Size/2, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y,
						p.x+Size/2, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				
				Graphics()->QuadsSetSubsetFree(	bx1+PixX, 0.0f+PixY,
												bx2-PixX, 0.0f+PixY,
												bx1+PixX, 0.25f-PixY,
												bx2-PixX, 0.25f-PixY);
				
				if (!Left && Top)
				{
					p = vec2(x, y)*32;
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y,
						p.x, p.y,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (!Right && Top)
				{
					p = vec2(x, y)*32+vec2(Size/2, 0);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y,
						p.x+Size/2, p.y,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (!Left && Bot)
				{
					p = vec2(x, y)*32+vec2(0, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y,
						p.x, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (!Right && Bot)
				{
					p = vec2(x, y)*32+vec2(Size/2, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y,
						p.x+Size/2, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				

				Graphics()->QuadsSetSubsetFree(	bx1+PixX, 0.25f+PixY,
												bx2-PixX, 0.25f+PixY,
												bx1+PixX, 0.5f-PixY,
												bx2-PixX, 0.5f-PixY);
												
				if (Left && Top && (!m_pBlocks[(y-1)*m_Width+x-1] || m_pBlocks[(y-1)*m_Width+x-1] > 3))
				{
					p = vec2(x, y)*32;
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y,
						p.x, p.y,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (Right && Top && (!m_pBlocks[(y-1)*m_Width+x+1] || m_pBlocks[(y-1)*m_Width+x+1] > 3))
				{
					p = vec2(x, y)*32+vec2(Size/2, 0);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y,
						p.x+Size/2, p.y,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (Left && Bot && (!m_pBlocks[(y+1)*m_Width+x-1] || m_pBlocks[(y+1)*m_Width+x-1] > 3))
				{
					p = vec2(x, y)*32+vec2(0, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x+Size/2, p.y+Size/2,
						p.x, p.y+Size/2,
						p.x+Size/2, p.y,
						p.x, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
				
				if (Right && Bot && (!m_pBlocks[(y+1)*m_Width+x+1] || m_pBlocks[(y+1)*m_Width+x+1] > 3))
				{
					p = vec2(x, y)*32+vec2(Size/2, Size/2);
					
					IGraphics::CFreeformItem FreeFormItem(
						p.x, p.y+Size/2,
						p.x+Size/2, p.y+Size/2,
						p.x, p.y,
						p.x+Size/2, p.y);
					
					Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				}
			}
		}
	}
	
	
	Graphics()->QuadsEnd();
}
	
void CBlocks::OnRender()
{
	if(Client()->State() < IClient::STATE_ONLINE)
		return;
	
	
	int Num = Client()->SnapNumItems(IClient::SNAP_CURRENT);
	for(int i = 0; i < Num; i++)
	{
		IClient::CSnapItem Item;
		const void *pData = Client()->SnapGetItem(IClient::SNAP_CURRENT, i, &Item);

		if (Item.m_Type == NETOBJTYPE_BLOCK)
		{
			const struct CNetObj_Block *pBlock = (const CNetObj_Block *)pData;
			
			SetBlock(ivec2(pBlock->m_X, pBlock->m_Y), pBlock->m_Type);
			
			// uncomment to visualize block snapping / syncing
			/*
			{
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				Graphics()->SetColor(0.0f, 1.0f, 0.0f, 0.5f);
				
				vec2 p = vec2(pBlock->m_X, pBlock->m_Y);
				
				IGraphics::CFreeformItem FreeFormItem(
					p.x, p.y,
					p.x+32, p.y,
					p.x, p.y+32,
					p.x+32, p.y+32);
										
				Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
				Graphics()->QuadsEnd();
			}
			*/
		}
	}
	
	RenderBlocks();
}