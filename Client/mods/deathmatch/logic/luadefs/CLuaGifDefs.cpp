/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include "lua/CLuaFunctionParser.h"
#include "CLuaGifDefs.h"
#include "CScriptFile.h"

#ifndef DEFAULT_MAX_FILESIZE
    #define DEFAULT_MAX_FILESIZE 52428800
#endif

void CLuaGifDefs::LoadFunctions(){
    constexpr static const std::pair<const char*, lua_CFunction> functions[]{
        {"gifCreate", GifCreate},
        {"gifPlay", GifPlay},
        {"gifStop", GifStop},
        {"gifNavigateToThumbnail", GifNavigateToThumbnail},
        {"gifSetFrameDelay", GifSetFrameDelay},
        {"gifSetLoadingDelay", GifSetLoadingDelay},
        {"gifGetLoadedFrameCount", GifGetLoadedFrameCount},
        {"gifGetFrameCount", GifGetFrameCount},
        {"gifGetFrameDelay", GifGetFrameDelay},
        {"gifGetShowingFrame", GifGetShowingFrame},
        {"gifGetFrameDefaultDelay", GifGetFrameDefaultDelay},
        {"gifGetFormat", GifGetFormat},
        {"gifGetTick", GifGetTick},
        {"gifGetLoadingDelay", GifGetLoadingDelay},
        {"isGifPlaying", IsGifPlaying},
        {"isGifLoading", IsGifLoading},
        {"isGifLoaded", IsGifLoaded}
    };
    for (const auto& [name, func] : functions)
        CLuaCFunctions::AddFunction(name, func);
}

void CLuaGifDefs::AddClass(lua_State* luaVM){
    lua_newclass(luaVM);
    lua_classfunction(luaVM, "play", "gifPlay");
    lua_classfunction(luaVM, "stop", "gifStop");
    lua_classfunction(luaVM, "navigateToThumbnail", "gifNavigateToThumbnail");
    lua_classfunction(luaVM, "setFrameDelay", "gifSetFrameDelay");
    lua_classfunction(luaVM, "setLoadingDelay", "gifSetFrameLoadingDelay");
    lua_classfunction(luaVM, "getLoadedFrameCount", "gifGetLoadedFrameCount");
    lua_classfunction(luaVM, "getFrameCount", "gifGetFrameCount");
    lua_classfunction(luaVM, "getFrameDelay", "gifGetFrameDelay");
    lua_classfunction(luaVM, "getShowingFrame", "gifGetShowingFrame");
    lua_classfunction(luaVM, "getFrameDefaultDelay", "gifGetFrameDefaultDelay");
    lua_classfunction(luaVM, "getFormat", "gifGetFormat");
    lua_classfunction(luaVM, "getTick", "gifGetTick");
    lua_classfunction(luaVM, "getLoadingDelay", "gifGetLoadingDelay");
    lua_classfunction(luaVM, "isPlaying", "isGifPlaying");
    lua_classfunction(luaVM, "isLoading", "isGifLoading");
    lua_classfunction(luaVM, "isLoaded", "isGifLoaded");
    lua_registerclass(luaVM, "Gif");
}

int CLuaGifDefs::GifCreate(lua_State* luaVM) {
    SString buffer;
    CScriptArgReader argStream(luaVM);
    argStream.ReadString(buffer);
    if (!argStream.HasErrors()) {
        CLuaMain& luaMain = lua_getownercluamain(luaVM);
        CResource* parentResource = luaMain.GetResource();
        SString path, metaPath,lastPath = buffer;
        std::unique_ptr<CClientGifLoader> loader(nullptr);
        SString format = buffer.substr(0, 6);
        if (format == GIF87 || format == GIF89) {
            loader = std::make_unique<CClientGifLoader>(buffer);
        }else if (CResourceManager::ParseResourcePathInput(buffer,parentResource,&path,&metaPath)){
            if (FileExists(path)) {
                eAccessType  access = buffer[0] == '@' ? eAccessType::ACCESS_PRIVATE : eAccessType::ACCESS_PUBLIC;
                CScriptFile* file = new CScriptFile(parentResource->GetScriptID(), metaPath, DEFAULT_MAX_FILESIZE,access);
                if (file->Load(parentResource, CScriptFile::MODE_READ)) {
                    file->SetLuaDebugInfo(g_pClientGame->GetScriptDebugging()->GetLuaDebugInfo(luaVM));
                    long size = file->GetSize();
                    SString fileBuffer;
                    long bytesRead = file->Read(size,fileBuffer);
                    if (bytesRead >= 0){
                        loader = std::make_unique<CClientGifLoader>(fileBuffer);
                    }else if (bytesRead == -2){ 
                        m_pScriptDebugging->LogWarning(luaVM, "out of memory");
                    }else{
                        m_pScriptDebugging->LogError(luaVM, SString("error while reading file [%s]", lastPath.c_str()).c_str());
                    }
                    file->Unload();
                    m_pElementDeleter->Delete(file);
                }else{
                    delete file;
                    m_pScriptDebugging->LogError(luaVM, SString("couldn't load file [%s]",lastPath.c_str()).c_str());
                    lua_pushboolean(luaVM, false);
                    return 1;
                }
            }else{
                m_pScriptDebugging->LogError(luaVM, SString("file [%s] doesn't exists!",lastPath.c_str()).c_str());
                lua_pushboolean(luaVM, false);
                return 1;
            }
        }else{
            loader = std::make_unique<CClientGifLoader>(buffer);
        }
        if (loader->HasError()) {
            m_pScriptDebugging->LogError(luaVM, loader->GetError().c_str());
            loader->Close();
            lua_pushboolean(luaVM, false);
            return 1;
        }
        if (loader->IsValid()) {
            unsigned char* thumbnail = loader->GetFrame();
            if (loader->HasError()){
                m_pScriptDebugging->LogError(luaVM,loader->GetError().c_str());
                delete[] thumbnail;
                loader->Close();
                lua_pushboolean(luaVM, false);
                return 1;
            }
            CClientGif* gif = g_pClientGame->GetManager()->GetRenderElementManager()->CreateGif(loader->GetWidth(),loader->GetHeight(),thumbnail);
            if (!gif) {
                m_pScriptDebugging->LogError(luaVM,"couldn't create the texture");
                if (thumbnail){
                    delete[] thumbnail;
                }
                loader->Close();
                lua_pushboolean(luaVM,false);
                return 1;
            }
            gif->SetFormat(loader->GetFormat());
            gif->SetFrameCount(loader->GetFrameLength());
            CClientGifLoader* gifLoader = loader.release();
            gif->SetLoader(gifLoader);
            gif->SetParent(parentResource->GetResourceDynamicEntity());
            gif->SetResource(parentResource);
            lua_pushelement(luaVM, gif);
            return 1;
        }else{
            m_pScriptDebugging->LogError(luaVM, "couldn't read gif buffer");
        }
        loader->Close();
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM, false);
    return 1;
}

int CLuaGifDefs::GifPlay(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM,argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM, false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        gif->Play();
        lua_pushboolean(luaVM,true);
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifStop(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        gif->Stop();
        lua_pushboolean(luaVM, true);
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM,argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifNavigateToThumbnail(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        gif->NavigateToThumbnail();
        lua_pushboolean(luaVM, true);
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifSetFrameDelay(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    uint index = 0;
    uint delay = 0;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    argStream.ReadNumber(index,-1);
    argStream.ReadNumber(delay);
    if (!argStream.HasErrors()) {
        gif->SetFrameDelay(index, delay);
        lua_pushboolean(luaVM,true);
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifSetLoadingDelay(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    uint delay = 0;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    argStream.ReadNumber(delay);
    if (!argStream.HasErrors()){
        gif->SetLoadingDelay(delay);
        lua_pushboolean(luaVM,true);
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetLoadedFrameCount(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushnumber(luaVM, gif->GetImageCount());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetFrameCount(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushnumber(luaVM, gif->GetFrameCount());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetFrameDelay(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    uint index = 0;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    argStream.ReadNumber(index,-1);
    if (!argStream.HasErrors()) {
        lua_pushnumber(luaVM, gif->GetFrameDelay(index));
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetShowingFrame(lua_State* luaVM){
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushnumber(luaVM, gif->GetShowingFrame());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetFrameDefaultDelay(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    uint index = 0;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    argStream.ReadNumber(index,-1);
    if (!argStream.HasErrors()) {
        lua_pushnumber(luaVM, gif->GetFrameDefaultDelay(index));
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetFormat(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM,argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushstring(luaVM, gif->GetFormat().c_str());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetTick(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif){
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM, false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushnumber(luaVM, gif->GetTick());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::GifGetLoadingDelay(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif){
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushnumber(luaVM, gif->GetLoadingDelay());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::IsGifPlaying(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM,argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushboolean(luaVM, gif->IsPlaying());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::IsGifLoading(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushboolean(luaVM, gif->IsLoading());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}

int CLuaGifDefs::IsGifLoaded(lua_State* luaVM) {
    CClientGif* gif = nullptr;
    CScriptArgReader argStream(luaVM);
    argStream.ReadUserData(gif);
    if (!gif) {
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
        lua_pushboolean(luaVM,false);
        return 1;
    }
    if (!argStream.HasErrors()) {
        lua_pushboolean(luaVM, gif->IsLoaded());
        return 1;
    }else{
        m_pScriptDebugging->LogCustom(luaVM, argStream.GetFullErrorMessage());
    }
    lua_pushboolean(luaVM,false);
    return 1;
}
