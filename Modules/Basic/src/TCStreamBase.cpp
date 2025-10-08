//*******************************************************************************
//
// *******   ***   ***               *
//    *     *     *                  *
//    *    *      *                *****
//    *    *       ***  *   *   **   *    **    ***
//    *    *          *  * *   *     *   ****  * * *
//    *     *         *   *      *   * * *     * * *
//    *      ***   ***    *     **   **   **   *   *
//                        *
//*******************************************************************************
// see https://bitbucket.org/the____tiger/tcsystem for details.
// Copyright (C) 2003 - 2021 Thomas Goessler. All Rights Reserved. 
//*******************************************************************************
//
// TCSystem is the legal property of its developers.
// Please refer to the COPYRIGHT file distributed with this source distribution.
// 
// This library is free software; you can redistribute it and/or             
// modify it under the terms of the GNU Lesser General Public                
// License as published by the Free Software Foundation; either              
// version 2.1 of the License, or (at your option) any later version.        
//                                                                           
// This library is distributed in the hope that it will be useful,           
// but WITHOUT ANY WARRANTY; without even the implied warranty of            
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         
// Lesser General Public License for more details.                           
//                                                                           
// You should have received a copy of the GNU Lesser General Public          
// License along with this library; if not, write to the Free Software       
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
//*******************************************************************************
//  $Id: TCStreamBase.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#include "TCStreamBase.h"

#include "TCOutput.h"
#include "TCSystem.h"

#include "TCNewEnable.h"

namespace tc
{
namespace imp
{

StreamBase::StreamBase(CodecPtr codec)
:m_codec(codec)
{
   ResetStatus();
   m_display_error_messages = true;

   SetStreamDirection(Direction::DEAD);
}

StreamBase::~StreamBase()
{
}

void StreamBase::CloseStream()
{
   SetStreamDirection(Direction::DEAD);
}

void StreamBase::EnableDisplayErrorMessages(bool displ)
{
   m_display_error_messages = displ;
}

// -----------------------------------------------------------------
// status methodes
// -----------------------------------------------------------------
void StreamBase::SetStatus(Error err) const
{
   m_status = err;

   // if we shoud we display an error message
   if (m_display_error_messages)
   {
      DisplayErrorMessage();
	   if (GetStatus() != Error::NONE && system::GetLastError() != 0)
      {
         TCERRORS("TCBASE", static_cast<int>(err) << ", " << system::GetLastErrorMessage().c_str());
      }
   }
}

void StreamBase::ResetStatus()
{
	m_status = Error::NONE;
}

void StreamBase::DisplayErrorMessage() const
{
   switch (GetStatus())
   {
   case Error::STREAM_OPEN:
         TCERRORS("TCBASE", "Error opening stream");
       break;

   case Error::STREAM_CLOSE:
         TCERRORS("TCBASE", "Error closing stream");
       break;

   case Error::STREAM_DIRECTION:
         TCERRORS("TCBASE", "Error wrong stream direction");
       break;

   case Error::READ_FROM_STREAM:
         TCERRORS("TCBASE", "Error during read from stream");
         break;

   case Error::WRITE_TO_STREAM:
         TCERRORS("TCBASE", "Error during write to stream");
         break;

   case Error::END_OF_STREAM:
         break;

   case Error::SET_STREAM_POSITION:
         TCERRORS("TCBASE", "Set or get position of stream not supported");
         break;

   default:
      break;
   }
}

}
}
