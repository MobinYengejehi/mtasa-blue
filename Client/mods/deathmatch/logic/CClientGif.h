/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include "CClientGifDisplay.h"

#define GIF87 "GIF87a"
#define GIF89 "GIF89a"
#define NETSCAPE20 "NETSCAPE2.0"

constexpr int BYTE32 = 32*1024;

enum class CGifFrameAction {
    Combine,
    Erase,
    Undo,
    Null
};

class CClientGifBufferTerminator {
public:
    CClientGifBufferTerminator* neww = nullptr;
    CClientGifBufferTerminator* old = nullptr;
    SString                     data;
    uint                        index;
    ~CClientGifBufferTerminator() { data.clear(); };
};

class CClientGifBuffer {
public:
    uint size = 0;
    CClientGifBuffer() = default;
    CClientGifBuffer(const SString&, bool order);
    SString            Read(uint);
    std::map<int, int> ReadWords(const int& length) { return ReadNumbersOfBytes(length, 2); }
    int                ReadWord();
    std::map<int, int> ReadBytes(const int&);
    int                ReadByte();
    void               Jump(const uint&);
    void               Skip(const uint&);
    uint               GetOffset() { return userOffset; }
    bool               HasError() { return !errorMessage.empty(); }
    SString            GetError() { return errorMessage; }
    void               Close();

private:
    std::map<int, CClientGifBufferTerminator*> blocks;
    SString                                    buffer;
    CClientGifBufferTerminator*                terminator = nullptr;
    uint                                       userOffset = 0;
    uint                                       cached = 0;
    SString                                    errorMessage = "";
    bool                                       byteOrder = true;
    void                                       AddChain(CClientGifBufferTerminator*);
    void                                       RemoveChain(CClientGifBufferTerminator*);
    SString                                    GetBlock(const uint&);
    std::map<int, int>                         ReadNumbersOfBytes(const int&, const int&);
    void                                       Error(const SString& message) { errorMessage = message; }
};

class CClientGifRect {
public:
    uint left = 0;
    uint top = 0;
    uint width = 0;
    uint height = 0;
    bool valid = false;
    CClientGifRect() = default;
    CClientGifRect(const uint& l, const uint& t, const uint& w, const uint& h) : left(l), top(t), width(w), height(h) { valid = true; }
};

class CClientGifLoader {
public:
    CClientGifLoader(const SString&);
    ~CClientGifLoader() = default;
    unsigned char* GetFrame();
    uint           GetWidth() { return width; }
    uint           GetHeight() { return height; }
    int            GetFrameLength() { return frameLength; }
    uint           GetDelay() { return delay; }
    SString&       GetFormat() { return format; }
    bool           HasError() { return !errorMessage.empty(); }
    SString        GetError() { return errorMessage; }
    bool           IsValid() { return input.size > 0; }
    void           Close();
    unsigned char* GetLoaded() { return loadedFrameMatrix; }

private:
    CClientGifBuffer                                 input;
    uint                                             size;
    uint                                             stride;
    SString                                          format;
    uint                                             width;
    uint                                             height;
    uint                                             delay = -1;
    std::map<int, uint32_t>                          Global;
    int                                              flags;
    int                                              firstFrameOffset;
    SString                                          frameComment;
    bool                                             frameLoop;
    int                                              frameLength = 0;
    int                                              lastFrameProcessOffset = 0;
    bool                                             hasParams = false;
    int                                              loadedFrameNumber = 0;
    CGifFrameAction                                  loadedFrameActionOnBackground = CGifFrameAction::Null;
    int                                              loadedFrameTransparentColor = -1;
    unsigned char*                                   loadedFrameMatrix = nullptr;
    unsigned char*                                   backgroundMatrixAfterLoadedFrame = nullptr;
    CClientGifRect                                   backgroundRectangleToErase;
    SString                                          errorMessage = "";
    CClientGifRect                                   coords;
    bool                                             interlaced;
    std::map<int, uint32_t>                          palette;
    int                                              bitsInColor = 0;
    int                                              bytesInCurrentPart = 0;
    int                                              CLEAR_VOC = 0;
    int                                              END_OF_STREAM = 0;
    std::map<int, std::map<int, int>>                LZW_VOC;
    int                                              bitsInCode = 0;
    int                                              nextPower = 0;
    int                                              firstUndefinedCode = -1;
    bool                                             needCompletion = false;
    int                                              streamBitBuffer = 0;
    int                                              bitsInBuffer = 0;
    int                                              pixelsRemained = 0;
    int                                              px = 0;
    int                                              py = 0;
    bool                                             PassFirstFrame();
    void                                             SkipToEndOfBlock();
    void                                             Skip2C();
    SString                                          Process2C();
    void                                             Process21_F9();
    SString                                          ProcessBlocks(const bool&, const bool&);
    bool                                             LoadNextFrame();
    int                                              ReadByteFromStream();
    int                                              ReadCodeFromStream();
    void                                             ClearLZWVoc();
    void                                             PixelFromStream(const int&);
    void                                             ClearBackgroundFrame();
    void                                             ClearLoadedFrame();
    void                                             CheckError();
    void                                             Error(const SString& message) { errorMessage = message; }
};

class CClientGif final : public CClientTexture {
    DECLARE_CLASS(CClientGif, CClientTexture);

public:
    CClientGif(CClientManager*, ElementID, CGifItem*,unsigned char*);
    ~CClientGif();
    virtual void                 Unlink() override;
    CResource*                   GetResource() { return m_pResource; }
    void                         SetResource(CResource* resource) { m_pResource = resource; }
    eClientEntityType            GetType() const { return CCLIENTGIF; }
    CGifItem*                    GetRenderItem() const { return static_cast<CGifItem*>(m_pRenderItem); }
    CClientGifDisplay*           GetDisplay() const { return m_pGifDisplay.get(); }
    std::vector<unsigned char*>& GetFrames() { return frames; }
    uint                         GetStride() { return stride; }
    uint                         GetShowingFrame() { return showing; }
    uint                         GetFrameDelay(const uint& id) { return delays[(id < 1 ? showing : (id > GetImageCount() ? showing : id)) - 1]; }
    uint                         GetFrameDefaultDelay(const uint& id) { return defaultDelays[(id < 1 ? showing : (id > GetImageCount() ? showing : id)) - 1]; }
    int                          GetImageCount() { return frames.size(); }
    double&                      GetTick() { return tick; }
    unsigned char*               GetFrame() { return showing <= GetImageCount() ? frames[showing - 1] : nullptr; }
    SString&                     GetFormat() { return format; }
    uint                         GetLoadingDelay() { return loadingDelay; }
    uint                         GetFrameCount() { return frameCount; }
    void                         SetFrameDelay(const uint& id, const uint32_t& delay) { delays[(id < 1 ? showing : (id > GetImageCount() ? (showing) : id)) - 1] = delay; }
    void                         SetFormat(const SString& fmt) { if (!fmt.empty()) format = fmt; }
    void                         SetLoader(CClientGifLoader* gifLoader) { if (gifLoader) {loader.reset(gifLoader);delays.push_back(loader->GetDelay());defaultDelays.push_back(loader->GetDelay());} }
    void                         SetLoadingDelay(const uint& dly) { loadingDelay = std::max<uint>(0,dly); }
    void                         SetFrameCount(const uint& count) { frameCount = count; }
    void                         UpdateTick() { tick = (double)GetTickCount64_(); }
    void                         Play() { playing = true; }
    void                         Stop() { playing = false; }
    void                         Next();
    void                         NavigateToThumbnail() { showing = 1; }
    bool                         IsPlaying() { return playing; }
    bool                         IsLoading() { return playing && !loaded; }
    bool                         IsLoaded() { return loaded; }
    bool                         IsDestoryed() const { return m_bIsDestoryed; }
    bool                         HasErrors() { return !error.empty(); }
    std::string                  GetError() const { return error; }

private:
    CResource*                                 m_pResource = nullptr;
    CClientManager*                            m_pManager;
    std::unique_ptr<CClientGifLoader>          loader;
    std::unique_ptr<CClientGifDisplay>         m_pGifDisplay;
    SString                                    format = "Undefined";
    std::vector<unsigned char*>                frames;
    std::vector<int>                           defaultDelays;
    std::vector<int>                           delays;
    uint                                       frameCount = 0;
    uint                                       stride;
    uint                                       showing = 1;
    uint                                       loadingDelay = 500;
    double                                     tick = (double)GetTickCount64_();
    bool                                       playing = false;
    bool                                       loading = false;
    bool                                       loaded = false;
    bool                                       m_bIsDestoryed = false;
    std::string                                error = "";
};
