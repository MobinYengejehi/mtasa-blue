/*****************************************************************************
 *
 *  PROJECT:     Multi Theft Auto v1.0
 *  LICENSE:     See LICENSE in the top level directory
 *  FILE:        game_sa/CTrailerSA.cpp
 *  PURPOSE:     Trailer vehicle entity
 *
 *  Multi Theft Auto is available from http://www.multitheftauto.com/
 *
 *****************************************************************************/

#include "StdInc.h"
#include "CTrailerSA.h"

CTrailerSA::CTrailerSA(CTrailerSAInterface* trailer) : CAutomobileSA(trailer)
{
    this->m_pInterface = trailer;
}

CTrailerSA::CTrailerSA(eVehicleTypes dwModelID, unsigned char ucVariation, unsigned char ucVariation2) : CAutomobileSA(dwModelID, ucVariation, ucVariation2)
{
}
