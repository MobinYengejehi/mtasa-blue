/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include "CLuaDefs.h"
#include "CClientGif.h"

class CLuaGifDefs : public CLuaDefs
{
public:
    static void LoadFunctions();
    static void AddClass(lua_State*);
    static int  GifCreate(lua_State*);
    static int  GifPlay(lua_State*);
    static int  GifStop(lua_State*);
    static int  GifNavigateToThumbnail(lua_State*);
    static int  GifSetFrameDelay(lua_State*);
    static int  GifGetFrameCount(lua_State*);
    static int  GifGetFrameDelay(lua_State*);
    static int  GifGetShowingFrame(lua_State*);
    static int  GifGetFrameDefaultDelay(lua_State*);
    static int  GifGetFormat(lua_State*);
    static int  GifGetTick(lua_State*);
    static int  IsGifPlaying(lua_State*);
};