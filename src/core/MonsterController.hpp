//
//  MonsterController.hpp
//  GGJ21
//
//  Created by Stephen Gowen on 1/30/21.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include <GowEngine/GowEngine.hpp>

class MonsterController : public EntityController
{
    friend class MonsterNetworkController;
    friend class MonsterRenderController;
    
    DECL_RTTI;
    DECL_EntityController_create(EntityController);
    
public:
    MonsterController(Entity* e);
    virtual ~MonsterController();
    
    virtual void update() override;
    virtual void onCollision(Entity* e) override;
    
    bool isInEncounter();
    Entity* battleAvatar();
    
private:
    enum State
    {
        STAT_IDLE   = 0,
        STAT_MOVING = 1
    };
    Entity* _battleAvatar;
};
