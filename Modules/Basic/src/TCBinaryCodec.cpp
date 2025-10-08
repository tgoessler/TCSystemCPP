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
//  $Id: TCBinaryCodec.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#include "TCBinaryCodec.h"

#include "TCStream.h"
#include "TCValue.h"
#include "TCWString.h"

#include <cstring>

#include "TCNewEnable.h"

namespace tc
{
   namespace imp 
   {
      template < template<class T> class VALUE_CONV >
      class BinaryCodec: public Codec
      {
      public:
         BinaryCodec()
         {
         }

         ~BinaryCodec()
         {
         }

         CodecPtr Clone()
         {
            return CodecPtr(new BinaryCodec);
         }

         uint64_t Decode(Stream& stream, int8_t& val)
         {
             return stream.ReadBytes(1, &val);
         }

         uint64_t Decode(Stream& stream, uint8_t& val)
         {
             return stream.ReadBytes(1, &val);
         }

         uint64_t Decode(Stream& stream, int16_t &val)
         {
            VALUE_CONV< int16_t > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, uint16_t &val)
         {
            VALUE_CONV< uint16_t > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, int32_t &val)
         {
            VALUE_CONV< int32_t > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, uint32_t &val)
         {
            VALUE_CONV< uint32_t > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, int64_t &val)
         {
            VALUE_CONV< int64_t > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, uint64_t &val)
         {
            VALUE_CONV< uint64_t > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, float &val)
         {
            VALUE_CONV< float > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, double &val)
         {
            VALUE_CONV< double > bval;
            if (stream.ReadBytes(bval.GetNumBytes(), bval.GetBytes()) != bval.GetNumBytes())
            {
               return 0;
            }
            val = bval;
            return bval.GetNumBytes();
         }

         uint64_t Decode(Stream& stream, std::string& val)
         {
            // for char we have to read the len first
            uint32_t nBytes;
            const uint64_t len = Decode(stream, nBytes);

            char* data = new char[static_cast<std::size_t>(nBytes)+1];
            if (stream.ReadBytes(nBytes, data) != nBytes)
            {
               delete []data;
               return 0;
            }
            data[nBytes] = 0;
            val = data;

            delete []data;
            return len + nBytes;
         }

         uint64_t Decode(Stream& stream, std::wstring& val)
         {
            std::string tmp;
            const uint64_t len = Decode(stream, tmp);
            val = wstring::ToString(tmp);

            return len;
         }

         uint64_t Decode(Stream& stream, char *val)
         {
            // for char we have to read the len first
            uint32_t nBytes;
            const uint64_t len = Decode(stream, nBytes);

            if (stream.ReadBytes(nBytes, val) != nBytes)
            {
               return 0;
            }
            val[nBytes] = 0;
            return len + nBytes;
         }

         uint64_t Decode(Stream& stream, char &val)
         {
            return stream.ReadBytes(1, &val);
         }

         uint64_t Decode(Stream& stream, bool &val)
         {
            uint8_t cval;
            const uint64_t len = stream.ReadBytes(1, &cval);
            val = cval == 1;
            return len;
         }

         uint64_t Encode(int8_t val, Stream& stream)
         {
             return stream.WriteBytes(1, &val);
         }

         uint64_t Encode(uint8_t val, Stream& stream)
         {
             return stream.WriteBytes(1, &val);
         }

         uint64_t Encode(int16_t val, Stream& stream)
         {
            VALUE_CONV< int16_t > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(uint16_t val, Stream& stream)
         {
            VALUE_CONV< uint16_t > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(int32_t val, Stream& stream)
         {
            VALUE_CONV< int32_t > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(uint32_t val, Stream& stream)
         {
            VALUE_CONV< uint32_t > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(int64_t val, Stream& stream)
         {
            VALUE_CONV< int64_t > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(uint64_t val, Stream& stream)
         {
            VALUE_CONV< uint64_t > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(float val, Stream& stream)
         {
            VALUE_CONV< float > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(double val, Stream& stream)
         {
            VALUE_CONV< double > bval(val);
            return stream.WriteBytes(bval.GetNumBytes(), bval.GetBytes());
         }

         uint64_t Encode(const std::string& val, Stream& stream)
         {
            const uint32_t nBytes = static_cast<uint32_t>(val.length());
            const uint64_t len = Encode(nBytes, stream);

            return stream.WriteBytes(nBytes, val.c_str()) + len;
         }

         uint64_t Encode(const std::wstring& val, Stream& stream)
         {
            const std::string tmp = wstring::ToString(val);
            return Encode(tmp, stream);
         }

         uint64_t Encode(const char *val, Stream& stream)
         {
            const uint32_t nBytes = static_cast<uint32_t>(std::strlen(val));
            const uint64_t len = Encode(nBytes, stream);

            return stream.WriteBytes(nBytes, val) + len;
         }

         uint64_t Encode(char val, Stream& stream)
         {
            return stream.WriteBytes(1, &val);
         }

         uint64_t Encode(bool val, Stream& stream)
         {
            const uint8_t cval = val ? 1 : 0;
            return stream.WriteBytes(1, &cval);
         }

         uint64_t EncodeEndOfLine(Stream& /*stream*/)
         {
            return 0;
         }

         uint64_t EncodeSpace(Stream& /*stream*/)
         {
            return 0;
         }

      };

      TC_DLL_LOCAL CodecPtr CreateBinaryCodec()
      {
         return CodecPtr(new BinaryCodec<Value>);
      }

      TC_DLL_LOCAL CodecPtr CreateLittleEndianBinaryCodec()
      {
         return CodecPtr(new BinaryCodec<LittleEndianValue>);
      }

      TC_DLL_LOCAL CodecPtr CreateBigEndianBinaryCodec()
      {
         return CodecPtr(new BinaryCodec<BigEndianValue>);
      }
   }
}

