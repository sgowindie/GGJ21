//
//  PlayerController.hpp
//  GGJ21
//
//  Created by Stephen Gowen on 1/29/21.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#include "EntityController.hpp"

#include <string>

class InputState;
struct Rektangle;

enum PlayerDirection
{
    PDIR_UP      = 0,
    PDIR_DOWN    = 1,
    PDIR_LEFT    = 2,
    PDIR_RIGHT   = 3
};

class PlayerController : public EntityController
{
    friend class PlayerNetworkController;
    
    DECL_RTTI;
    DECL_EntityController_create;
    
public:
    PlayerController(Entity* e);
    virtual ~PlayerController() {}
    
    virtual void update();
    virtual void processInput(InputState* inputState, bool isLive);
    
    void setUsername(std::string value);
    std::string getUsername() const;
    void setUserAddress(std::string value);
    std::string getUserAddress() const;
    void setPlayerID(uint8_t value);
    uint8_t getPlayerID() const;
    PlayerDirection getPlayerDirection();
    uint16_t getHealth();
    
protected:
    enum State
    {
        STAT_IDLE = 0,
        STAT_MOVING = 1
    };
    
    enum ReadStateFlag
    {
        RSTF_PLAYER_INFO = 1 << 2,
        RSTF_STATS =       1 << 3
    };
    
    struct PlayerInfo
    {
        std::string _username;
        std::string _userAddress;
        uint8_t _playerID;
        
        PlayerInfo()
        {
            _username = "Unknown";
            _userAddress = "1.3.3.7";
            _playerID = 0;
        }
        
        friend bool operator==(PlayerInfo& a, PlayerInfo& b)
        {
            return
            a._username    == b._username &&
            a._userAddress == b._userAddress &&
            a._playerID    == b._playerID;
        }
        
        friend bool operator!=(PlayerInfo& a, PlayerInfo& b)
        {
            return !(a == b);
        }
    };
    PlayerInfo _playerInfo;
    PlayerInfo _playerInfoCache;
    
    struct Stats
    {
        uint16_t _health;
        uint8_t _dir;
        
        Stats()
        {
            _health = 3;
            _dir = PDIR_UP;
        }
        
        friend bool operator==(Stats& a, Stats& b)
        {
            return
            a._health == b._health &&
            a._dir == b._dir;
        }
        
        friend bool operator!=(Stats& a, Stats& b)
        {
            return !(a == b);
        }
    };
    Stats _stats;
    Stats _statsCache;
};

#include "EntityNetworkController.hpp"

class PlayerNetworkController : public EntityNetworkController
{
public:
    DECL_EntityNetworkController_create;
    
    PlayerNetworkController(Entity* e, bool isServer) : EntityNetworkController(e, isServer) {}
    virtual ~PlayerNetworkController() {}
    
    virtual void read(InputMemoryBitStream& imbs);
    virtual uint8_t write(OutputMemoryBitStream& ombs, uint8_t dirtyState);
    virtual void recallCache();
    virtual uint8_t refreshDirtyState();
};
