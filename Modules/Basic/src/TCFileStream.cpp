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
//  $Id: TCFileStream.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#include "TCFileStream.h"

#include "TCOutput.h"
#include "TCWString.h"

#include "TCNewEnable.h"

namespace tc
{
   namespace imp
   {

      FileStream::FileStream(std::FILE *stream, Direction direction, CodecPtr codec)
         :StreamBase(codec)
         ,m_stream_pointer(0)
         ,m_file_name()
      {
         SetStream(stream, direction);
      }

      FileStream::FileStream(const std::string &fileName, Direction direction, CodecPtr codec)
         :StreamBase(codec)
         ,m_stream_pointer(0)
         ,m_file_name(fileName)
      {
         SetStream(fileName, direction);
      }

      FileStream::~FileStream()
      {
         CloseStream();
      }

      void FileStream::CloseStream()
      {
         if (!m_file_name.empty() && m_stream_pointer)
         {
            std::fclose(m_stream_pointer);
            m_stream_pointer = 0;
         }

         StreamBase::CloseStream();
      }

      void FileStream::SetStream(std::FILE *stream, Direction direction)
      {
         CloseStream();

         m_stream_pointer = stream;
         m_file_name.clear();

         SetStreamDirection(direction);
      }

      void FileStream::SetStream(const std::string &fileName, Direction direction)
      {
         CloseStream();

         FILE* file = 0;
         if (direction == Direction::READ)
         {
#ifdef TCOS_WINDOWS
            const std::wstring wfileName(wstring::ToString(fileName));
            file = _wfopen(wfileName.c_str(), L"rb");
#else
            file = std::fopen(fileName.c_str(), "rb");
#endif
         }
         else if (direction == Direction::WRITE)
         {
#ifdef TCOS_WINDOWS
            const std::wstring wfileName(wstring::ToString(fileName));
            file = _wfopen(wfileName.c_str(), L"wb");
#else
            file = std::fopen(fileName.c_str(), "wb");
#endif
         }
         else if (direction == Direction::READ_WRITE)
         {
#ifdef TCOS_WINDOWS
            const std::wstring wfileName(wstring::ToString(fileName));
            file = _wfopen(wfileName.c_str(), L"wb+");
#else
            file = std::fopen(fileName.c_str(), "wb+");
#endif
         }

         if (!file)
         {
            TCERRORS("TCBASE", "Error opening file '" << fileName << "'");
            SetStatus(Error::STREAM_OPEN);
         }

         SetStream(file, direction);
         m_file_name = fileName;
      }


      uint64_t FileStream::ReadBytes(uint64_t nBytes, void *bytes)
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

         uint64_t read_bytes = 0;
         while(read_bytes < nBytes)
         {
            const std::size_t num = std::fread(static_cast<uint8_t*>(bytes)+read_bytes, 1, 
                                               static_cast<std::size_t>(nBytes - read_bytes), m_stream_pointer);
            if (num <= 0)
            {
               if (std::feof(m_stream_pointer))
               {
                   SetStatus(Error::END_OF_STREAM);
               }
               else
               {
                   SetStatus(Error::READ_FROM_STREAM);
               }
               break;
            }
            read_bytes += num;
         }

         return read_bytes;
      }

      uint64_t FileStream::WriteBytes(uint64_t nBytes, const void *bytes)
      {
         if (nBytes == 0)
         {
            return 0;
         }

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

         uint64_t wrote_bytes = 0;
         while(wrote_bytes < nBytes)
         {
            const std::size_t num = std::fwrite(static_cast<const uint8_t*>(bytes)+wrote_bytes, 1, 
                                                static_cast<std::size_t>(nBytes - wrote_bytes), m_stream_pointer);
            if (num <= 0)
            {
                SetStatus(Error::WRITE_TO_STREAM);
               break;
            }
            wrote_bytes += num;
         }

         return wrote_bytes;
      }

      void FileStream::Flush()
      {
         // check for an error
         if (HasError())
         {
            return;
         }

         // check correct stream direction
         if (IsWriting())
         {
            std::fflush(m_stream_pointer);
         }
         else
         {
             SetStatus(Error::STREAM_DIRECTION);
         }
      }

      bool FileStream::SetPosition(int64_t pos, Position pos_mode)
      {
         ResetStatus();

         switch(pos_mode)
         {
         case Position::SET:
            return std::fseek(m_stream_pointer, static_cast<long>(pos), SEEK_SET) == 0;

         case Position::CURRENT:
            return std::fseek(m_stream_pointer, static_cast<long>(pos), SEEK_CUR) == 0;

         case Position::END:
            return std::fseek(m_stream_pointer, static_cast<long>(pos), SEEK_END) == 0;
         }

         return false;
      }

      uint64_t FileStream::GetPosition() const
      {
         return std::ftell(m_stream_pointer);
      }

      StreamPtr FileStream::Clone()
      {
         if (m_file_name.empty())
         {
            return StreamPtr(new FileStream(m_stream_pointer, GetStreamDirection(), GetCodec()->Clone()));
         }
         else
         {
            return StreamPtr(new FileStream(m_file_name, GetStreamDirection(), GetCodec()->Clone()));
         }
      }

   }
}
