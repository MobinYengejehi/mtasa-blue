/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        mods/deathmatch/logic/CLuaBlock.cpp
 *
 *  Multi Theft Auto is available from https://multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CMemBlock.h"

CMemBlock::CMemBlock(CElement* pParentEntity, const size_t& uiSize, const CMemBlockCastType& ucType) :
    CElement(pParentEntity)
{
    SetTypeName("memblock");
    m_iType = CElement::MEMBLOCK;

    size_t typeSize = GetTypeSize(ucType);

    m_uiAllocatedSize = uiSize*typeSize;
    m_ucType = ucType;

    m_uiStartIndex = 1;

    m_pBlocks = (CMemBlockByte*)realloc(NULL, m_uiAllocatedSize);
}

CMemBlock::~CMemBlock()
{
    Unlink();
}

void CMemBlock::Unlink() {
    if (m_pBlocks)
    {
        free(m_pBlocks);
    }

    m_pBlocks = NULL;
}

size_t CMemBlock::GetAllocatedSize() const
{
    return m_uiAllocatedSize;
}

size_t CMemBlock::GetArrayIndex() const
{
    return m_uiStartIndex;
}

CMemBlockCastType CMemBlock::GetCastType() const
{
    return m_ucType;
}

CMemBlockByte* CMemBlock::GetBlocks()
{
    return m_pBlocks;
}

void CMemBlock::SetArrayIndex(const size_t& uiIndex)
{
    size_t length = GetArrayLength();

    if (uiIndex < 1)
    {
        m_uiStartIndex = 1;
    }
    else if (uiIndex > length)
    {
        m_uiStartIndex = length;
    }
    else
    {
        m_uiStartIndex = uiIndex;
    }
}

bool CMemBlock::SetCastType(const CMemBlockCastType& ucType)
{
    size_t typeSize = GetTypeSize(ucType);
    if (m_uiAllocatedSize < typeSize)
    {
        return false;
    }

    m_ucType = ucType;
    SetArrayIndex(m_uiStartIndex);
    return true;
}

size_t CMemBlock::GetArrayLength()
{
    size_t typeSize = GetTypeSize(m_ucType);
    if (typeSize)
    {
        return m_uiAllocatedSize / typeSize;
    }
    return 0;
}

bool CMemBlock::GetBlock(double* pValue, const size_t& uiIndex)
{
    if (!pValue) {
        return false;
    }

    size_t index = 0;
    size_t length = GetArrayLength();

    if (uiIndex < 1)
    {
        index = 1;
    }
    else if (uiIndex > length)
    {
        index = length;
    }
    else
    {
        index = uiIndex;
    }

    index--;

    if (m_ucType == CMemBlockCastType::Char)
    {
        *pValue = m_pBlocks[index];
    }
    else if (m_ucType == CMemBlockCastType::UnsignedChar)
    {
        *pValue = reinterpret_cast<unsigned char*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::Int)
    {
        *pValue = reinterpret_cast<int*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::UnsignedInt)
    {
        *pValue = reinterpret_cast<unsigned int*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::Float)
    {
        *pValue = reinterpret_cast<float*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::Double)
    {
        *pValue = reinterpret_cast<double*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::Short)
    {
        *pValue = reinterpret_cast<short*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::UnsignedShort)
    {
        *pValue = reinterpret_cast<unsigned short*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::Long)
    {
        *pValue = reinterpret_cast<long*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::UnsingedLong)
    {
        *pValue = reinterpret_cast<unsigned long*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::LongLong)
    {
        *pValue = reinterpret_cast<long long*>(m_pBlocks)[index];
    }
    else if (m_ucType == CMemBlockCastType::UnsignedLongLong)
    {
        *pValue = reinterpret_cast<unsigned long long*>(m_pBlocks)[index];
    }

    return true;
}

size_t CMemBlock::SetBlock(const double& dValue)
{
    return SetBlocks(dValue, 1);
}

size_t CMemBlock::SetBlocks(const double& dValue, const size_t& uiLength)
{
    size_t length = 0;
    size_t arrayLength = GetArrayLength();
    if (uiLength < 1)
    {
        length = 1;
    }
    else if (uiLength > arrayLength - m_uiStartIndex + 1)
    {
        length = arrayLength - m_uiStartIndex + 1;
    }
    else
    {
        length = uiLength;
    }

    size_t bytes = 0;

    #define SETBLOCKS(type) \
        type* blocks = reinterpret_cast<type*>(m_pBlocks); \
        for (size_t i = 0; i < length; blocks[i + m_uiStartIndex - 1] = (type)dValue, i++, bytes++); \
        bytes *= GetTypeSize(m_ucType);

    if (m_ucType == CMemBlockCastType::Char)
    {
        for (size_t i = 0; i < length; m_pBlocks[i + m_uiStartIndex - 1] = (char)dValue, i++, bytes++)
            ;
    }
    else if (m_ucType == CMemBlockCastType::UnsignedChar)
    {
        SETBLOCKS(unsigned char);
    }
    else if (m_ucType == CMemBlockCastType::Int)
    {
        SETBLOCKS(int);
    }
    else if (m_ucType == CMemBlockCastType::UnsignedInt)
    {
        SETBLOCKS(unsigned int);
    }
    else if (m_ucType == CMemBlockCastType::Float)
    {
        SETBLOCKS(float);
    }
    else if (m_ucType == CMemBlockCastType::Double)
    {
        SETBLOCKS(double);
    }
    else if (m_ucType == CMemBlockCastType::Short)
    {
        SETBLOCKS(short);
    }
    else if (m_ucType == CMemBlockCastType::UnsignedShort)
    {
        SETBLOCKS(unsigned short);
    }
    else if (m_ucType == CMemBlockCastType::Long)
    {
        SETBLOCKS(long);
    }
    else if (m_ucType == CMemBlockCastType::UnsingedLong)
    {
        SETBLOCKS(unsigned long);
    }
    else if (m_ucType == CMemBlockCastType::LongLong)
    {
        SETBLOCKS(long long);
    }
    else if (m_ucType == CMemBlockCastType::UnsignedLongLong)
    {
        SETBLOCKS(unsigned long long);
    }
    
    #undef SETBLOCKS

    return bytes;
}

size_t CMemBlock::CopyBlocks(CMemBlock* pMemBlock, const size_t& uiLength)
{
    if (!pMemBlock)
    {
        return 0;
    }

    size_t start = m_uiStartIndex;
    size_t typeSize = GetTypeSize(CMemBlockCastType::Char);

    if (start > m_uiAllocatedSize)
    {
        start = m_uiAllocatedSize;
    }

    size_t length = 0;
    if (uiLength < 1)
    {
        length = 1;
    }
    else if (uiLength > m_uiAllocatedSize - start + 1)
    {
        length = m_uiAllocatedSize - start + 1;
    }
    else
    {
        length = uiLength;
    }

    if (start == m_uiAllocatedSize)
    {
        length = 1;
    }

    CMemBlockByte* targetBlocks = pMemBlock->GetBlocks();
    size_t         targetOccupiedSize = pMemBlock->GetAllocatedSize();
    size_t         targetStartIndex = pMemBlock->GetArrayIndex();

    if (targetStartIndex > targetOccupiedSize)
    {
        targetStartIndex = targetOccupiedSize;
    }

    if (length > targetOccupiedSize - targetStartIndex + 1)
    {
        length = targetOccupiedSize - targetStartIndex + 1;
    }

    if (targetStartIndex == targetOccupiedSize)
    {
        length = 1;
    }

    size_t bytes = 0;

    for (size_t i = 0; i < length; targetBlocks[i + targetStartIndex - 1] = m_pBlocks[i + start - 1], i++, bytes++)
        ;

    return bytes;
}

size_t CMemBlock::ReadString(const std::string& buffer)
{
    size_t length = buffer.length();
    if (length < 1)
    {
        return 0;
    }
    else if (length > m_uiAllocatedSize - m_uiStartIndex + 1)
    {
        length = m_uiAllocatedSize - m_uiStartIndex + 1;
    }

    if (m_uiAllocatedSize == m_uiStartIndex)
    {
        length = 1;
    }

    size_t      bytes = 0;
    const char* bufferBytes = buffer.data();

    for (size_t i = 0; i < length; m_pBlocks[i + m_uiStartIndex - 1] = bufferBytes[i], i++, bytes++)
        ;

    return bytes;
}

void CMemBlock::GetBlocksArray(lua_State* luaVM, const size_t& uiLength) {
    if (!luaVM)
    {
        lua_pushboolean(luaVM, false);
        return;
    }

    size_t length = 0;
    size_t arrayLength = GetArrayLength();
    if (uiLength < 1)
    {
        length = 1;
    }
    else if (uiLength > arrayLength - m_uiStartIndex + 1)
    {
        length = arrayLength - m_uiStartIndex + 1;
    }
    else
    {
        length = uiLength;
    }

    lua_createtable(luaVM, 0, length);

    #define COPYTOTABLE(type) \
        type* blocks = reinterpret_cast<type*>(m_pBlocks); \
        for (size_t i = 0; i < length; i++) \
        { \
            lua_pushnumber(luaVM, i + 1); \
            lua_pushnumber(luaVM, blocks[i + m_uiStartIndex - 1]); \
            lua_settable(luaVM, -3); \
        }

    if (m_ucType == CMemBlockCastType::Char)
    {
        for (size_t i = 0; i < length; i++)
        {
            lua_pushnumber(luaVM, i + 1);
            lua_pushnumber(luaVM, m_pBlocks[i + m_uiStartIndex - 1]);
            lua_settable(luaVM, -3);
        }
    }
    else if (m_ucType == CMemBlockCastType::UnsignedChar)
    {
        COPYTOTABLE(unsigned char);
    }
    else if (m_ucType == CMemBlockCastType::Int)
    {
        COPYTOTABLE(int);
    }
    else if (m_ucType == CMemBlockCastType::UnsignedInt)
    {
        COPYTOTABLE(unsigned int);
    }
    else if (m_ucType == CMemBlockCastType::Float)
    {
        COPYTOTABLE(float);
    }
    else if (m_ucType == CMemBlockCastType::Double)
    {
        COPYTOTABLE(double);
    }
    else if (m_ucType == CMemBlockCastType::Short)
    {
        COPYTOTABLE(short);
    }
    else if (m_ucType == CMemBlockCastType::UnsignedShort)
    {
        COPYTOTABLE(unsigned short);
    }
    else if (m_ucType == CMemBlockCastType::Long)
    {
        COPYTOTABLE(long);
    }
    else if (m_ucType == CMemBlockCastType::UnsingedLong)
    {
        COPYTOTABLE(unsigned long);
    }
    else if (m_ucType == CMemBlockCastType::LongLong)
    {
        COPYTOTABLE(long long);
    }
    else if (m_ucType == CMemBlockCastType::UnsignedLongLong)
    {
        COPYTOTABLE(unsigned long long);
    }

    #undef COPYTOTABLE
}

size_t CMemBlock::GetTypeSize(const CMemBlockCastType& ucType)
{
    switch (ucType)
    {
        case CMemBlockCastType::Char:
            return sizeof(char);
        case CMemBlockCastType::UnsignedChar:
            return sizeof(unsigned char);
        case CMemBlockCastType::Int:
            return sizeof(int);
        case CMemBlockCastType::UnsignedInt:
            return sizeof(unsigned int);
        case CMemBlockCastType::Float:
            return sizeof(float);
        case CMemBlockCastType::Double:
            return sizeof(double);
        case CMemBlockCastType::Short:
            return sizeof(short);
        case CMemBlockCastType::UnsignedShort:
            return sizeof(unsigned short);
        case CMemBlockCastType::Long:
            return sizeof(long);
        case CMemBlockCastType::UnsingedLong:
            return sizeof(unsigned long);
        case CMemBlockCastType::LongLong:
            return sizeof(long long);
        case CMemBlockCastType::UnsignedLongLong:
            return sizeof(unsigned long long);
        default:
            return 0;
    }
}
