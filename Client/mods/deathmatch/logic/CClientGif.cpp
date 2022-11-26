/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *
 *  Multi Theft Auto is available from https://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CClientGif.h"

CClientGif::CClientGif(CClientManager* pManager, ElementID ID, CGifItem* p_GifItem,unsigned char* thumbnail)
    : ClassInit(this), CClientTexture(pManager, ID, p_GifItem)
{
    SetTypeName("gif");
    m_pManager = pManager;
    stride = GetRenderItem()->m_uiSizeX * 4;
    frames.push_back(thumbnail);
    m_pGifDisplay = std::make_unique<CClientGifDisplay>(m_pManager->GetDisplayManager(), this);
    loader = std::unique_ptr<CClientGifLoader>(nullptr);
}

CClientGif::~CClientGif(){
    Unlink();
}

void CClientGif::Next(){
    if (!loaded) {
        if (loader->IsValid()) {
            unsigned char* frame = loader->GetFrame();
            if (frame) {
                frames.push_back(frame);
                int delay = loader->GetDelay();
                defaultDelays.push_back(delay);
                delays.push_back(delay);
            }else {
                loader->Close();
                CClientGifLoader* gifLoader = loader.release();
                if (gifLoader) {
                    delete gifLoader;
                }
                loaded = true;
            }
        }
    }
    if (showing >= GetImageCount()) {
        showing = 1;
    }else{
        showing++;
    }
}

void CClientGif::Unlink(){
    if (!loaded){
        if (loader){
            loader->Close();
            CClientGifLoader* gifLoader = loader.release();
            if (gifLoader){
                delete gifLoader;
            }
        }
    }
    for (unsigned char* frame : frames) {
        if (frame) {
            delete[] frame;
        }
    }
    frames.clear();
    delays.clear();
    defaultDelays.clear();
    m_bIsDestoryed = true;
    CClientRenderElement::Unlink();
}

int GetByteInt(const char& c){
    std::byte byt = std::byte(c);
    return std::to_integer<int>(byt);
}

std::map<int,int> GetStringBytes(const SString& string){
    std::map<int,int> bytes;
    size_t length = string.length();
    for (int i = 1; i <= length; i++){
        bytes[i] = GetByteInt(string[i - 1]);
    }
    return bytes;
}

SString ConcatStringArray(const std::vector<SString>& arr){
    SString result = "";
    for (SString chunk : arr){
        result += chunk;
    }
    return result;
}

CClientGifBuffer::CClientGifBuffer(const SString& data,bool order){
    buffer = data;
    byteOrder = order;
    size = buffer.length();
    terminator = new CClientGifBufferTerminator;
    terminator->neww = terminator;
    terminator->old = terminator;
}

void CClientGifBuffer::AddChain(CClientGifBufferTerminator* object) {
    if (object && !HasError()) {
        CClientGifBufferTerminator* old = terminator->old;
        object->neww = terminator;
        object->old = old;
        old->neww = object;
        terminator->old = object;
    }
}

void CClientGifBuffer::RemoveChain(CClientGifBufferTerminator* object) {
    if (object && !HasError()) {
        CClientGifBufferTerminator* neww = object->neww;
        CClientGifBufferTerminator* old = object->old;
        old->neww = neww;
        neww->old = old;
    }
}

SString CClientGifBuffer::GetBlock(const uint& index) {
    SString result = "";
    if (!HasError()) {
        CClientGifBufferTerminator* object = blocks[index];
        if (!object) {
            if (cached < 3) {
                cached++;
            }else{
                CClientGifBufferTerminator* chain = terminator->neww;
                if (chain){
                    RemoveChain(chain);
                    delete chain;
                    blocks.erase(chain->index);
                }
            }
            int offset = index*BYTE32;
            uint length = std::min<uint>(BYTE32,size - offset);
            if (offset >= size) {
                Error("couldn't get block");
                return result;
            }
            SString content = buffer.substr(offset,length);
            if (content.length() != length) {
                Error("couldn't get block");
                return result;
            }
            object = new CClientGifBufferTerminator;
            object->data = content;
            object->index = index;
            AddChain(object);
            blocks[index] = object;
        }else if (object->neww != terminator){
            RemoveChain(object);
            AddChain(object);
        }
        result = object->data;
    }
    return result;
}

SString CClientGifBuffer::Read(uint length) {
    SString result = "";
    if (!HasError()) {
        if (length < 0) {
            Error("negative string length");
            return result;
        }
        if (!(userOffset >= 0 && userOffset + length <= size)) {
            Error("attempt to read beyond the file boundary");
            return result;
        }
        SString str = "";
        std::vector<SString> arr;
        while (length > 0) {
            int offset = userOffset%BYTE32;
            int partSize = std::min<int>(BYTE32 - offset,length);
            int index = std::floor(userOffset / BYTE32);
            SString part = GetBlock(index).substr(offset,partSize);
            userOffset += partSize;
            length -= partSize;
            if (!arr.empty()) {
                arr.push_back(part);
            }else if (!str.empty()) {
                str += part;
            }else if (length > BYTE32) {
                arr.push_back(part);
            }else{
                str = part;
            }
        }
        if (!arr.empty()) {
            result = ConcatStringArray(arr);
        }else{
            result = str;
        }
    }
    return result;
}

std::map<int,int> CClientGifBuffer::ReadNumbersOfBytes(const int& elements,const int& k) {
    std::map<int,int> numbers;
    if (!HasError()) {
        if (k == 1 && elements <= 100) {
            SString chunk = Read(elements);
            numbers = GetStringBytes(chunk);
            if (HasError()) {
                numbers.clear();
            }
        }else{
            int maxNumbersInString = std::floor(100/k);
            for (int index = 1; index <= elements; index += maxNumbersInString){
                int numbersInPart = std::min<int>(elements - index + 1,maxNumbersInString);
                SString part = Read(numbersInPart * k);
                if (k == 1) {
                    for (int i = 1; i <= numbersInPart; i++) {
                        numbers[index + i - 1] = GetByteInt(part[i - 1]);
                    }
                }else{
                    for (int i = 0; i < numbersInPart; i++){
                        int number = 0;
                        for (int byt = 1; byt <= k; byt++) {
                            int pos = i*k + (byteOrder ? k + 1 - byt : byt);
                            int lastnumber = number;
                            number = number * 256 + GetByteInt(part[pos - 1]);
                        }
                        numbers[index + i] = number;
                    }
                }
                if (HasError()) {
                    numbers.clear();
                    break;
                }
            }
        }
    }
    return numbers;
}

int CClientGifBuffer::ReadWord(){
    std::map<int,int> words = ReadWords(1);
    return words[1];
}

std::map<int,int> CClientGifBuffer::ReadBytes(const int& length) {
    return GetStringBytes(Read(length));
}

int CClientGifBuffer::ReadByte() {
    std::map<int,int> bytes = ReadBytes(1);
    return bytes[1];
}

void CClientGifBuffer::Jump(const uint& offset) {
    userOffset = offset;
}

void CClientGifBuffer::Skip(const uint& amount) {
    userOffset += amount;
}

void CClientGifBuffer::Close() {
    for (auto& [index,block] : blocks){
        if (block && block != terminator) {
            delete block;
        }
    }
    if (terminator){
        delete terminator;
    }
    blocks.clear();
    buffer.clear();
    errorMessage.clear();
}

CClientGifLoader::CClientGifLoader(const SString& buffer) {
    input = CClientGifBuffer(buffer, true);
    if (input.size > 0){
        format = input.Read(6);
        if (format != GIF87 && format != GIF89) {
            Error("wrong file format");
            return;
        }
        width = input.ReadWord();
        height = input.ReadWord();
        size = width*height*4;
        stride = width * 4;
        CheckError();
        if (HasError()) {
            return;
        }
        if (width == 0 || height == 0) {
            Error("gif must be 1x1 at least");
            return;
        }
        flags = input.ReadByte();
        input.Skip(2);
        if (flags >= 0x80) {
            int length = std::pow(2, (flags % 8) + 1);
            for (int index = 0; index < length; index++) {
                std::map<int, int> color = input.ReadBytes(3);
                Global[index] = (0xff << 24) | (color[1] << 16) | (color[2] << 8) | color[3];
            }
        }
        firstFrameOffset = input.GetOffset();
        loadedFrameMatrix = new unsigned char[size];
        backgroundMatrixAfterLoadedFrame = new unsigned char[size];
        ClearLoadedFrame();
        ClearBackgroundFrame();
    }
}

unsigned char* CClientGifLoader::GetFrame(){
    CheckError();
    if (HasError()) {
        Error("couldn't get gif frame");
        return nullptr;
    }
    if (!hasParams) {
        int saved = input.GetOffset();
        ProcessBlocks(false,false);
        input.Jump(saved);
    }
    if (!LoadNextFrame()) {
        return nullptr;
    }
    unsigned char* frame = new unsigned char[size];
    memcpy(frame, loadedFrameMatrix, size);
    return frame;
}

bool CClientGifLoader::PassFirstFrame() {
    CheckError();
    if (!HasError() && !hasParams) {
        int current = input.GetOffset();
        if (current > lastFrameProcessOffset) {
            lastFrameProcessOffset = current;
            return true;
        }
    }
    return false;
}

void CClientGifLoader::SkipToEndOfBlock() {
    CheckError();
    if (HasError()) {
        return;
    }
    int size = -1;
    while (size != 0) {
        size = input.ReadByte();
        input.Skip(size);
    }
}

void CClientGifLoader::Skip2C() {
    CheckError();
    if (HasError()) {
        return;
    }
    input.Skip(8);
    int localFlags = input.ReadByte();
    if (localFlags >= 0x80) {
        input.Skip(3 * std::pow(2, (localFlags % 8) + 1));
    }
    input.Skip(1);
    SkipToEndOfBlock();
}

SString CClientGifLoader::Process2C() {
    CheckError();
    if (HasError()){
        return "ERROR";
    }
    if (backgroundRectangleToErase.valid) {
        CClientGifRect& rect = backgroundRectangleToErase;
        rect.valid = false;
        for (int row = rect.top; row < rect.height; row++) {
            uint32_t* pixels = reinterpret_cast<uint32_t*>(backgroundMatrixAfterLoadedFrame + stride*row);
            for (int column = rect.left; column < rect.width; column++) {
                pixels[column] = 0xff000000;
            }
        }
    }
    if (loadedFrameActionOnBackground == CGifFrameAction::Null) {
        loadedFrameActionOnBackground = CGifFrameAction::Combine;
    }
    std::map<int, int> cc = input.ReadWords(4);
    coords = CClientGifRect(cc[1], cc[2], cc[3], cc[4]);
    if (coords.width == 0 || coords.height == 0 || coords.left + coords.width > width || coords.top + coords.height > height) {
        Error("wrong frame coordinates detected");
        return "ERROR";
    }
    int localFlags = input.ReadByte();
    interlaced = localFlags%0x80 >= 0x40;
    palette.clear();
    palette = Global;
    if (localFlags >= 0x80) {
        palette.clear();
        int length = std::pow(2, (localFlags % 8) + 1);
        for (int color = 0; color < length; color++) {
            std::map<int,int> c = input.ReadBytes(3);
            palette[color] = (0xff << 24) | (c[1] << 16) | (c[2] << 8) | c[3];
        }
    }
    if (palette.empty()) {
        Error("couldn't get frame pixels");
        return "ERROR";
    }
    bitsInColor = input.ReadByte();
    bytesInCurrentPart = 0;
    CLEAR_VOC = std::pow(2, bitsInColor);
    END_OF_STREAM = CLEAR_VOC + 1;
    LZW_VOC.clear();
    bitsInCode = bitsInColor + 1;
    nextPower = std::pow(2, bitsInCode);
    firstUndefinedCode = -1;
    needCompletion = false;
    streamBitBuffer = 0;
    bitsInBuffer = 0;
    if (ReadCodeFromStream() != CLEAR_VOC) {
        Error("couldn't read first pixel");
        return "ERROR";
    }
    ClearLZWVoc();
    memcpy(loadedFrameMatrix, backgroundMatrixAfterLoadedFrame, size);
    pixelsRemained = coords.width * coords.height;
    px = -5;
    py = -5;
    int code = -5;
    while (code != END_OF_STREAM) {
        code = ReadCodeFromStream();
        if (code == CLEAR_VOC) {
            ClearLZWVoc();
        }else if (code != END_OF_STREAM) {
            if (code >= firstUndefinedCode) {
                Error("wrong pixel code");
                return "ERROR";
            }
            std::map<int,int> stack;
            int pos = 1;
            int firstPixel = code;
            while (firstPixel >= CLEAR_VOC) {
                firstPixel = LZW_VOC[firstPixel][1];
                stack[pos] = LZW_VOC[firstPixel][2];
                pos++;
            }
            stack[pos] = firstPixel;
            if (needCompletion) {
                needCompletion = false;
                LZW_VOC[firstUndefinedCode - 1][2] = firstPixel;
                if (code == firstUndefinedCode - 1) {
                    stack[1] = firstPixel;
                }
            }
            for (int p = pos; p >= 1; p--){
                PixelFromStream(stack[p]);
            }
            if (firstUndefinedCode < 0x1000) {
                LZW_VOC[firstUndefinedCode] = {{1,code}};
                needCompletion = true;
                if (firstUndefinedCode == nextPower) {
                    bitsInCode++;
                    nextPower = std::pow(2, bitsInCode);
                }
                firstUndefinedCode++;
            }
        }
    }
    if (pixelsRemained != 0 || streamBitBuffer != 0) {
        Error("buffer calculation mistake (wrong file format)");
        return "ERROR";
    }
    int extraByte = ReadByteFromStream();
    if (extraByte != -5 && extraByte != 0) {
        if (ReadByteFromStream() != -5){
            Error("couldn't pass the frame");
            return "ERROR";
        }
    }
    memcpy(backgroundMatrixAfterLoadedFrame, loadedFrameMatrix, size);
    if (loadedFrameActionOnBackground == CGifFrameAction::Erase){
        backgroundRectangleToErase = CClientGifRect(coords.left, coords.top, coords.width, coords.height);
    }
    loadedFrameNumber++;
    return "OK";
}

int CClientGifLoader::ReadByteFromStream() {
    CheckError();
    if (!HasError()) {
        if (bytesInCurrentPart > 0) {
            bytesInCurrentPart--;
            return input.ReadByte();
        }else{
            bytesInCurrentPart = input.ReadByte() - 1;
            return bytesInCurrentPart >= 0 ? input.ReadByte() : -5;
        }
    }
    return -5;
}

int CClientGifLoader::ReadCodeFromStream(){
    CheckError();
    if (!HasError()){
        while (bitsInBuffer < bitsInCode){
            int byte = ReadByteFromStream();
            if (byte == -5) {
                Error("couldn't read the pixel code");
                return -5;
            }
            streamBitBuffer += byte * std::pow(2, bitsInBuffer);
            bitsInBuffer += 8;
        }
        int code = streamBitBuffer%nextPower;
        streamBitBuffer = (streamBitBuffer - code)/nextPower;
        bitsInBuffer -= bitsInCode;
        return code;
    }
    return -5;
}

void CClientGifLoader::ClearLZWVoc() {
    CheckError();
    if (!HasError()){
        LZW_VOC.clear();
        bitsInCode = bitsInColor + 1;
        nextPower = std::pow(2, bitsInCode);
        firstUndefinedCode = CLEAR_VOC + 2;
        needCompletion = false;
    }
}

void CClientGifLoader::PixelFromStream(const int& color) {
    CheckError();
    if (!HasError()) {
        pixelsRemained--;
        if (pixelsRemained < 0) {
            Error("couldn't calculate frame coordinates");
            return;
        }
        if (px != -5) {
            px++;
            if (px == coords.width) {
                px = 0;
                if (interlaced) {
                    while (py >= coords.height) {
                        if (py % 8 == 0) {
                            py = py < coords.height ? py + 8 : 4;
                        }else if (py % 4 == 0) {
                            py = py < coords.height ? py + 8 : 2;
                        }else if (py % 2 == 0) {
                            py = py < coords.height ? py + 4 : 1;
                        }else{
                            py += 2;
                        }
                    }
                }else{
                    py++;
                }
            }
        }else{
            px = 0;
            py = 0;
        }
        if (color != loadedFrameTransparentColor) {
            if (palette.find(color) == palette.end()) {
                Error("pixel color doesn't exist in frame palette");
                return;
            }
            uint32_t* pixels = reinterpret_cast<uint32_t*>(loadedFrameMatrix + stride * (coords.top + py));
            pixels[coords.left + px] = palette[color];
        }
    }
}

void CClientGifLoader::Process21_F9() {
    CheckError();
    if (HasError()) {
        return;
    }
    int len = input.ReadByte();
    int localFlags = input.ReadByte();
    int Delay = input.ReadWord();
    int transparent = input.ReadByte();
    int terminator = input.ReadByte();
    if (len != 4 || terminator != 0) {
        Error("couldn't get frame delay");
        return;
    }
    delay = Delay*10;
    if (localFlags % 2 == 1) {
        loadedFrameTransparentColor = transparent;
    }
    int method = ((int)std::floor(localFlags / 4))%8;
    if (method == 2) {
        loadedFrameActionOnBackground = CGifFrameAction::Erase;
    }else if (method == 3){
        loadedFrameActionOnBackground = CGifFrameAction::Undo;
    }
}

SString CClientGifLoader::ProcessBlocks(const bool& p2C, const bool& p21) {
    CheckError();
    if (HasError()) {
        return "";
    }
    SString error = "";
    while (error.empty()) {
        int starter = input.ReadByte();
        if (starter == 0x3B) {
            if (PassFirstFrame()) {
                hasParams = true;
            }
            error = "EOF";
        }else if (starter == 0x2C) {
            if (PassFirstFrame()) {
                frameLength++;
            }
            if (p2C) {
                error = Process2C();
            }else{
                Skip2C();
            }
        }else if (starter == 0x21) {
            int fn = input.ReadByte();
            if (fn == 0xF9) {
                if (p21) {
                    Process21_F9();
                }else{
                    SkipToEndOfBlock();
                }
            }else if (fn == 0xFE && frameComment.empty()) {
                std::vector<SString> comment;
                int size = -1;
                while (size != 0) {
                    size = input.ReadByte();
                    comment.push_back(input.Read(size));
                }
                frameComment = ConcatStringArray(comment);
            }else if (fn == 0xFF && input.Read(input.ReadByte()) == NETSCAPE20){
                frameLoop = true;
                SkipToEndOfBlock();
            }else{
                SkipToEndOfBlock();
            }
        }else{
            Error("unexpected error while process gif blocks");
            break;
        }
    }
    return error;
}

bool CClientGifLoader::LoadNextFrame() {
    CheckError();
    if (HasError()) {
        return false;
    }
    if (loadedFrameNumber == 0) {
        backgroundRectangleToErase = CClientGifRect(0,0,width,height);
        input.Jump(firstFrameOffset);
    }
    delay = -1;
    loadedFrameActionOnBackground = CGifFrameAction::Null;
    loadedFrameTransparentColor = -1;
    SString result = ProcessBlocks(true, true);
    return !result.empty() && result != "EOF";
}

void CClientGifLoader::ClearLoadedFrame() {
    if (loadedFrameMatrix) {
        for (int y = 0; y < height; y++) {
            uint32_t* pixels = reinterpret_cast<uint32_t*>(loadedFrameMatrix + stride*y);
            for (int x = 0; x < width; x++) {
                pixels[x] = 0;
            }
        }
    }
}

void CClientGifLoader::ClearBackgroundFrame(){
    if (backgroundMatrixAfterLoadedFrame){
        for (int y = 0; y < height; y++){
            uint32_t* pixels = reinterpret_cast<uint32_t*>(backgroundMatrixAfterLoadedFrame + stride*y);
            for (int x = 0; x < width; x++) {
                pixels[x] = 0;
            }
        }
    }
}

void CClientGifLoader::Close() {
    if (loadedFrameMatrix) {
        delete[] loadedFrameMatrix;
    }
    if (backgroundMatrixAfterLoadedFrame) {
        delete[] backgroundMatrixAfterLoadedFrame;
    }
    palette.clear();
    Global.clear();
    input.Close();
    errorMessage.clear();
}

void CClientGifLoader::CheckError() {
    if (!HasError() && input.HasError()) {
        Error(input.GetError());
    }
}
