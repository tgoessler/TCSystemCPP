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
//  $Id: TCMemoryStream.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************
#include "TCMemoryStream.h"

#include "TCUtil.h"

#include <cstring>
#include <limits>

#include "TCNewEnable.h"

namespace tc
{
   namespace imp
   {

      MemoryStream::MemoryStream(CodecPtr codec, ByteVector& memory)
         :StreamBase(codec),
         m_memory(memory),
         m_memory_position(0)
      {
         SetStreamDirection(Direction::READ_WRITE);
      }

      MemoryStream::~MemoryStream()
      {
      }

      uint64_t MemoryStream::ReadBytes(uint64_t num_bytes, void* bytes)
      {
         // check for an error
         if (HasError())
         {
            return 0;
         }

         // check mode
         if (!IsReading())
         {
            SetStatus(Error::STREAM_DIRECTION);
            return 0;
         }

         if (num_bytes > std::numeric_limits<ByteVector::size_type>::max())
         {
             return 0;
         }

         if (m_memory_position + num_bytes <= m_memory.size())
         {
            const ByteVector::pointer copy_data = static_cast<ByteVector::pointer>(bytes);
            std::copy(m_memory.begin()+m_memory_position, 
                      m_memory.begin()+static_cast<ByteVector::size_type>(m_memory_position + num_bytes), copy_data);
            m_memory_position += static_cast<ByteVector::size_type>(num_bytes);

            return num_bytes;
         }

         return 0;
      }

      uint64_t MemoryStream::WriteBytes(uint64_t num_bytes, const void* bytes)
      {
         // check for an error
         if (HasError())
         {
            return 0;
         }

         // check mode
         if (!IsWriting())
         {
            SetStatus(Error::STREAM_DIRECTION);
            return 0;
         }

         if (num_bytes > std::numeric_limits<ByteVector::size_type>::max())
         {
             return 0;
         }

         const ByteVector::const_pointer data_to_write = static_cast<ByteVector::const_pointer>(bytes);
         if (m_memory.size() == m_memory_position)
         {
            m_memory.insert(m_memory.end(), data_to_write, data_to_write + num_bytes);
         }
         else if (m_memory.size() < m_memory_position+num_bytes)
         {
             m_memory.resize(static_cast<ByteVector::size_type>(m_memory_position + num_bytes));
             std::copy(data_to_write, data_to_write+num_bytes, m_memory.begin()+m_memory_position);
         }
         else
         {
             std::copy(data_to_write, data_to_write+num_bytes, m_memory.begin()+m_memory_position);
         }

         m_memory_position += static_cast<ByteVector::size_type>(num_bytes);

         return num_bytes;
      }

      bool MemoryStream::SetPosition(int64_t pos, Position pos_mode)
      {
         if (static_cast<uint64_t>(util::Abs(pos)) > std::numeric_limits<ByteVector::size_type>::max())
         {
             return false;
         }

         switch(pos_mode)
         {
         case Position::SET:
            m_memory_position = static_cast<ByteVector::size_type>(pos);
            break;

         case Position::CURRENT:
            if (pos > 0)
            {
               m_memory_position += static_cast<ByteVector::size_type>(pos);
            }
            else
            {
               m_memory_position -= static_cast<ByteVector::size_type>(util::Abs(pos));
            }
            break;

         case Position::END:
            if (pos > 0)
            {
               m_memory_position = m_memory.size() + static_cast<ByteVector::size_type>(pos);
            }
            else
            {
               m_memory_position = m_memory.size() - static_cast<ByteVector::size_type>(util::Abs(pos));
            }
            break;
         }

         // check if vector is big enought
         if (m_memory.size() < m_memory_position)
         {
             m_memory.resize(m_memory_position, 0);
         }

         return true;
      }

      uint64_t MemoryStream::GetPosition() const
      {
         return m_memory_position;
      }

      StreamPtr MemoryStream::Clone()
      {
         return StreamPtr(new MemoryStream(GetCodec()->Clone(), m_memory));
      }

   }
}

