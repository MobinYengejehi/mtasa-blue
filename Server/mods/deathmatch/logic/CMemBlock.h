/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        mods/deathmatch/logic/CLuaBlock.h
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#pragma once

#include "CElement.h"

typedef char CMemBlockByte;

enum class CMemBlockCastType
{
    Unknown,
    Char,
    UnsignedChar,
    Int,
    UnsignedInt,
    Float,
    Double,
    Short,
    UnsignedShort,
    Long,
    UnsingedLong,
    LongLong,
    UnsignedLongLong
};

class CMemBlock : public CElement {
public:
    CMemBlock(CElement* pParentEntity, const size_t& uiSize, const CMemBlockCastType& ucType);
    ~CMemBlock();

    virtual void Unlink();
    virtual bool IsEntity() { return true; }
    bool         ReadSpecialData(const int iLine) override { return false; }

    size_t             GetAllocatedSize() const;
    size_t             GetArrayIndex() const;
    CMemBlockCastType  GetCastType() const;

    CMemBlockByte* GetBlocks();

    void SetArrayIndex(const size_t& uiIndex);
    bool SetCastType(const CMemBlockCastType& ucType);

    size_t GetArrayLength();

    bool GetBlock(double* pValue, const size_t& uiIndex);

    size_t SetBlock(const double& dValue);
    size_t SetBlocks(const double& dValue, const size_t& uiLength);

    size_t CopyBlocks(CMemBlock* pMemBlock, const size_t& uiLength);

    size_t ReadString(const std::string& sBuffer);

    void GetBlocksArray(lua_State* luaVM, const size_t& uiLength);

    static size_t GetTypeSize(const CMemBlockCastType& ucType);

private:
    size_t            m_uiAllocatedSize;
    size_t            m_uiStartIndex;
    CMemBlockCastType m_ucType;

    CMemBlockByte* m_pBlocks;
};
