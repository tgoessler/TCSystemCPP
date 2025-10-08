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
//  $Id: TCAsciiCodec.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#include "TCAsciiCodec.h"

#include "TCAssert.h"
#include "TCStream.h"
#include "TCString.h"
#include "TCUtil.h"
#include "TCWString.h"

#include <cctype>
#include <cstring>
#include <locale>

#include "TCNewEnable.h"

namespace tc
{
   namespace imp 
   {

      AsciiCodec::AsciiCodec()
         :Codec()
      {
      }

      AsciiCodec::~AsciiCodec()
      {
      }

      static bool iseol(char c)
      {
         return c == '\n' || c == '\r';
      }

      static uint64_t ReadNextValueString(Stream& stream, std::string& val)
      {
         const std::locale l;
         uint64_t len = 0;
         char c;
         // skipp spaces and end of line
         while (stream.ReadBytes(1, &c) == 1)
         {
            len++;
            if (std::isspace(c, l) || iseol(c))
            {
               continue;
            }
            else
            {
               val = c;
               break;
            }
         }

         while (stream.ReadBytes(1, &c) == 1)
         {
            len++;
            if (std::isspace(c, l) || iseol(c))
            {
               break;
            }
            val += c;
         }

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, int8_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToSint8(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, uint8_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToUint8(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, int16_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToSint16(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, uint16_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToUint16(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, int32_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToSint32(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, uint32_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToUint32(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, int64_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToSint64(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, uint64_t& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToUint64(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, float& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToFloat(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, double& val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = string::ToDouble(strval);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, std::string& val)
      {
         val.erase();
         uint64_t len = 0;
         char c;
         // skipp end of lines
         while (stream.ReadBytes(1, &c) == 1)
         {
            len++;
            if (iseol(c))
            {
               continue;
            }
            else
            {
               val = c;
               break;
            }
         }

         while (stream.ReadBytes(1, &c) == 1)
         {
            len++;
            if (iseol(c))
            {
               break;
            }

            val += c;
         }
         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, std::wstring& val)
      {
         std::string tmp;
         const uint64_t len = Decode(stream, tmp);
         val = wstring::ToString(tmp);

         return len;
      }

      uint64_t AsciiCodec::Decode(Stream& stream, char &val)
      {
         return stream.ReadBytes(1, &val);
      }

      uint64_t AsciiCodec::Decode(Stream& stream, bool &val)
      {
         std::string strval;
         const uint64_t len = ReadNextValueString(stream, strval);
         val = strval == "1";
         return len;
      }



      uint64_t AsciiCodec::Encode(int8_t val, Stream& stream)
      {
         return Encode(static_cast<int16_t>(val), stream);
      }

      uint64_t AsciiCodec::Encode(uint8_t val, Stream& stream)
      {
         return Encode(static_cast<uint16_t>(val), stream);
      }

      uint64_t AsciiCodec::Encode(int16_t val, Stream& stream)
      {
         char buffer[16];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%hd", val);

         return stream.WriteBytes(len, buffer);
      }
      uint64_t AsciiCodec::Encode(uint16_t val, Stream& stream)
      {
         char buffer[16];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%hu", val);

         return stream.WriteBytes(len, buffer);
      }

      uint64_t AsciiCodec::Encode(int32_t val, Stream& stream)
      {
         char buffer[32];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%d", val);

         return stream.WriteBytes(len, buffer);
      }

      uint64_t AsciiCodec::Encode(uint32_t val, Stream& stream)
      {
         char buffer[32];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%u", val);

         return stream.WriteBytes(len, buffer);
      }

      uint64_t AsciiCodec::Encode(int64_t val, Stream& stream)
      {
         char buffer[64];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%" TC_SINT64_FORMAT, val);

         return stream.WriteBytes(len, buffer);
      }
      uint64_t AsciiCodec::Encode(uint64_t val, Stream& stream)
      {
         char buffer[64];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%" TC_UINT64_FORMAT, val);

         return stream.WriteBytes(len, buffer);
      }

      uint64_t AsciiCodec::Encode(float val, Stream& stream)
      {
         char buffer[32];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%g", val);

         return stream.WriteBytes(len, buffer);
      }
      uint64_t AsciiCodec::Encode(double val, Stream& stream)
      {
         char buffer[64];
         const uint64_t len = string::Snprintf(buffer, util::ArraySize(buffer), "%lg", val);

         return stream.WriteBytes(len, buffer);
      }

      uint64_t AsciiCodec::Encode(const std::string& val, Stream& stream)
      {
         return stream.WriteBytes(val.length(), val.c_str());
      }

      uint64_t AsciiCodec::Encode(const std::wstring& val, Stream& stream)
      {
         const std::string tmp = wstring::ToString(val);
         return Encode(tmp, stream);
      }

      uint64_t AsciiCodec::Encode(const char *val, Stream& stream)
      {
         return stream.WriteBytes(std::strlen(val), val);
      }

      uint64_t AsciiCodec::Encode(char val, Stream& stream)
      {
         return stream.WriteBytes(1, &val);
      }

      uint64_t AsciiCodec::Encode(bool val, Stream& stream)
      {
         return stream.WriteBytes(1, val ? "1" : "0");
      }

      uint64_t AsciiCodec::EncodeEndOfLine(Stream& stream)
      {
         return stream.WriteBytes(1, "\n");
      }

      uint64_t AsciiCodec::EncodeSpace(Stream& stream)
      {
         return stream.WriteBytes(1, " ");
      }

      CodecPtr AsciiCodec::Clone()
      {
         return CodecPtr(new AsciiCodec);
      }

   }
}
