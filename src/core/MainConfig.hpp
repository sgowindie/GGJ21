//
//  MainConfig.hpp
//  GGJ21
//
//  Created by Stephen Gowen on 1/27/21.
//  Copyright © 2021 Stephen Gowen. All rights reserved.
//

#pragma once

#define CFG_MAIN MainConfig::getInstance()

class MainConfig
{
public:
    static MainConfig& getInstance();
    
    int _framebufferWidth;
    int _framebufferHeight;
    int _camWidth;
    int _camHeight;
    int _maxTextInputLength;
    
    void init();
    
private:
    MainConfig() {}
    ~MainConfig() {}
    MainConfig(const MainConfig&);
    MainConfig& operator=(const MainConfig&);
};
