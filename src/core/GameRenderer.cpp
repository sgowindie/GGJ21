//
//  GameRenderer.cpp
//  GGJ21
//
//  Created by Stephen Gowen on 1/27/21.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#include "GameRenderer.hpp"

#include "Entity.hpp"

#include "GameEngineState.hpp"
#include "OpenGLWrapper.hpp"
#include "Macros.hpp"
#include "Color.hpp"
#include "MainConfig.hpp"
#include "InputManager.hpp"
#include "GameInputManager.hpp"
#include "StringUtil.hpp"
#include "FPSUtil.hpp"
#include "Constants.hpp"
#include "Assets.hpp"
#include "World.hpp"
#include "Network.hpp"
#include "NetworkManagerClient.hpp"
#include "PlayerController.hpp"

#include <sstream>
#include <ctime>
#include <string>
#include <assert.h>

GameRenderer::GameRenderer() :
_fontRenderer(128, 0, 0, 16, 64, 75, 1024, 1024),
_framebuffer(CFG_MAIN._framebufferWidth, CFG_MAIN._framebufferHeight),
_polygonBatcher(1, true),
_screenRenderer(),
_shaderManager(),
_spriteBatcher(4096),
_textureManager(),
_textViews{
    TextView(TEXA_CENTER, "Joining Server..., [ESC] to exit", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "Lost Jackie again...", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "She needs to connect", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "a controller or", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "join me via IP.", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "", TEXV_HIDDEN),
    TextView(TEXA_CENTER, "", TEXV_HIDDEN)
}
{
    _fontRenderer.setMatrixSize(CFG_MAIN._camWidth, CFG_MAIN._camHeight);
    _fontRenderer.configure(_textViews[0], 0.5f, 0.12f, 0.02f);
    _fontRenderer.configure(_textViews[1], 0.25f, 0.7f, 0.018f);
    _fontRenderer.configure(_textViews[2], 0.25f, 0.6f, 0.018f);
    _fontRenderer.configure(_textViews[3], 0.25f, 0.56f, 0.018f);
    _fontRenderer.configure(_textViews[4], 0.25f, 0.52f, 0.018f);
    _fontRenderer.configure(_textViews[5], 0.24f, 0.08f, 0.02f);
    _fontRenderer.configure(_textViews[6], 0.76f, 0.08f, 0.02f);
}

void GameRenderer::createDeviceDependentResources()
{
    _fontRenderer.createDeviceDependentResources();
    
    OGL.loadFramebuffer(_framebuffer);
    
    _polygonBatcher.createDeviceDependentResources();
    _screenRenderer.createDeviceDependentResources();
    _shaderManager.createDeviceDependentResources();
    _spriteBatcher.createDeviceDependentResources();
    _textureManager.createDeviceDependentResources();
}

void GameRenderer::onWindowSizeChanged(int screenWidth, int screenHeight)
{
    _screenRenderer.onWindowSizeChanged(screenWidth, screenHeight);
}

void GameRenderer::releaseDeviceDependentResources()
{
    _fontRenderer.releaseDeviceDependentResources();
    
    OGL.unloadFramebuffer(_framebuffer);
    
    _polygonBatcher.releaseDeviceDependentResources();
    _screenRenderer.releaseDeviceDependentResources();
    _shaderManager.releaseDeviceDependentResources();
    _spriteBatcher.releaseDeviceDependentResources();
    _textureManager.releaseDeviceDependentResources();
}

void GameRenderer::render()
{
    updateMatrix(0, CFG_MAIN._camWidth, 0, CFG_MAIN._camHeight);

    OGL.bindFramebuffer(_framebuffer);
    OGL.enableBlending(true);
    
    renderWorld();
    renderSplitScreen();
    renderUI();

    _screenRenderer.renderToScreen(_shaderManager.shader("framebuffer"), _framebuffer);
}

void GameRenderer::updateMatrix(float l, float r, float b, float t)
{
    mat4_identity(_matrix);
    mat4_ortho(_matrix, l, r, b, t, -1, 1);
    float w = r - l;
    float h = t - b;
    INPUT_MGR.setMatrixSize(w, h);
}

void GameRenderer::addSpritesToBatcher(std::vector<Entity*>& entities)
{
    for (Entity* e : entities)
    {
        TextureRegion tr = ASSETS.findTextureRegion(e->getTextureMapping(), e->getStateTime());
        
        _spriteBatcher.addSprite(tr, e->getPosition().x, e->getPosition().y, e->getWidth(), e->getHeight(), e->getAngle(), e->isFacingLeft());
    }
}

void GameRenderer::renderWorld()
{
    World* w = ENGINE_STATE_GAME._world;
    
    _spriteBatcher.begin();
    addSpritesToBatcher(w->getLayers());
    addSpritesToBatcher(w->getStaticEntities());
    _spriteBatcher.end(_shaderManager.shader("texture"), _matrix, _textureManager.texture("background_tiles"));
    
    _spriteBatcher.begin();
    for (Entity* e : w->getDynamicEntities())
    {
        if (!IS_BIT_SET(e->getEntityDef()._bodyFlags, BODF_PLAYER))
        {
            TextureRegion tr = ASSETS.findTextureRegion(e->getTextureMapping(), e->getStateTime());
            _spriteBatcher.addSprite(tr, e->getPosition().x, e->getPosition().y, e->getWidth(), e->getHeight(), e->getAngle(), e->isFacingLeft());
        }
    }
    _spriteBatcher.end(_shaderManager.shader("texture"), _matrix, _textureManager.texture("background_tiles"));
    
    _spriteBatcher.begin();
    for (Entity* e : w->getDynamicEntities())
    {
        if (IS_BIT_SET(e->getEntityDef()._bodyFlags, BODF_PLAYER))
        {
            TextureRegion tr = ASSETS.findTextureRegion(e->getTextureMapping(), e->getStateTime());
            _spriteBatcher.addSprite(tr, e->getPosition().x, e->getPosition().y, e->getWidth(), e->getHeight(), e->getAngle(), e->isFacingLeft());
        }
    }
    _spriteBatcher.end(_shaderManager.shader("texture"), _matrix, _textureManager.texture("overworld_characters"));
    
}

void GameRenderer::renderUI()
{
    if (!CFG_MAIN._showDebug)
    {
        return;
    }
    
    updateMatrix(0, CFG_MAIN._camWidth, 0, CFG_MAIN._camHeight);
    
    _textViews[0]._visibility = TEXV_HIDDEN;
    _textViews[1]._visibility = TEXV_HIDDEN;
    _textViews[2]._visibility = TEXV_HIDDEN;
    _textViews[3]._visibility = TEXV_HIDDEN;
    _textViews[4]._visibility = TEXV_HIDDEN;
    _textViews[5]._visibility = TEXV_HIDDEN;
    _textViews[6]._visibility = TEXV_HIDDEN;
    World* w = ENGINE_STATE_GAME._world;
    for (Entity* e : w->getPlayers())
    {
        PlayerController* pc = static_cast<PlayerController*>(e->getController());
        
        int playerID = pc->getPlayerID();
        _textViews[playerID + 4]._text = StringUtil::format("%i|%s", playerID, pc->getPlayerName().c_str()).c_str();
        _textViews[playerID + 4]._visibility = TEXV_VISIBLE;
    }
    
    if (NW_MGR_CLIENT != NULL &&
        NW_MGR_CLIENT->state() == NWCS_WELCOMED)
    {
        if (w->getPlayers().size() == 1)
        {
            _textViews[1]._visibility = TEXV_VISIBLE;
            _textViews[2]._visibility = TEXV_VISIBLE;
            _textViews[3]._visibility = TEXV_VISIBLE;
            _textViews[4]._visibility = TEXV_VISIBLE;
        }
    }
    else
    {
        _textViews[0]._visibility = TEXV_VISIBLE;
    }
    
    for (int i = 0; i < NUM_TEXT_VIEWS; ++i)
    {
        _fontRenderer.renderText(_shaderManager.shader("texture"), _textureManager.texture("texture_font"), _textViews[i]);
    }
}

void GameRenderer::renderSplitScreen()
{
    _polygonBatcher.begin();
    _polygonBatcher.addRektangle(CFG_MAIN._splitScreenBarX,
                                 CFG_MAIN._splitScreenBarY,
                                 CFG_MAIN._splitScreenBarX + CFG_MAIN._splitScreenBarWidth,
                                 CFG_MAIN._splitScreenBarY + CFG_MAIN._splitScreenBarHeight);
    _polygonBatcher.end(_shaderManager.shader("geometry"), _matrix, Color::BLACK);
}
