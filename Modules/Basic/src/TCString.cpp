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
//  $Id: TCString.cpp,v 2d2c7b9f2f6c 2014/09/22 18:55:33 thomas $
//*******************************************************************************

#include "TCString.h"

#include "TCStringImp.h"
#include "TCStringTraits.h"
#include "TCSharedPtr.h"
#include "TCUtil.h"

#include <cctype>

#include "TCNewEnable.h"

namespace tc
{
   typedef StringImpTmpl<std::string, StringTraits> StringImp;

   std::string string::ToString(char val)
   {
      const char string[2] = {val, 0};

      return string;
   }

   std::string string::ToString(bool val)
   {
      return val ? "1" : "0";
   }

   std::string string::ToString(uint16_t val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%hu", val);

      return string;
   }

   std::string string::ToString(int16_t val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%hd", val);

      return string;
   }

   std::string string::ToString(uint32_t val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%u", val);

      return string;
   }

   std::string string::ToString(int32_t val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%d", val);

      return string;
   }

   std::string string::ToString(uint64_t val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%" TC_UINT64_FORMAT, val);

      return string;
   }

   std::string string::ToString(int64_t val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%" TC_SINT64_FORMAT, val);

      return string;
   }

   std::string string::ToString(float val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%f", val);

      return string;
   }

   std::string string::ToString(double val)
   {
      char string[32];
      Snprintf(string, util::ArraySize(string), "%lf", val);

      return string;
   }

   std::string string::ToString(const char* val)
   {
      return val ? val : "";
   }

   bool string::IsIntValue(const std::string& val)
   {
      int32_t tmp;
      return std::sscanf(val.c_str(), "%d", &tmp) == 1;
   }

   bool string::IsFltValue(const std::string& val)
   {
      double tmp;
      return std::sscanf(val.c_str(), "%lf", &tmp) == 1;
   }

   bool string::IsValue(const std::string& val)
   {
      return (IsIntValue(val) || IsFltValue(val));
   }

   double string::ToDouble(const std::string& val)
   {
      double v;
      if (std::sscanf(val.c_str(), "%lf", &v) != 1)
      {
         return 0;
      }

      return v;
   }

   float string::ToFloat(const std::string& val)
   {
      float v;
      if (std::sscanf(val.c_str(), "%f", &v) != 1)
      {
         return 0;
      }

      return v;
   }

   int8_t string::ToSint8(const std::string& val)
   {
      return static_cast<int8_t>(ToSint16(val));
   }

   uint8_t string::ToUint8(const std::string& val)
   {
      return static_cast<uint8_t>(ToUint16(val));
   }

   int16_t string::ToSint16(const std::string& val)
   {
      int16_t v;
      if (std::sscanf(val.c_str(), "%hd", &v) != 1)
      {
         return 0;
      }

      return v;
   }

   uint16_t string::ToUint16(const std::string& val)
   {
      uint16_t v;
      if (std::sscanf(val.c_str(), "%hu", &v) != 1)
      {
         return 0;
      }

      return v;
   }

   int32_t string::ToSint32(const std::string& val)
   {
      int32_t v;
      if (std::sscanf(val.c_str(), "%d", &v) != 1)
      {
         return 0;
      }

      return v;
   }

   uint32_t string::ToUint32(const std::string& val)
   {
      uint32_t v;
      if (std::sscanf(val.c_str(), "%u", &v) != 1)
      {
         return 0;
      }

      return v;
   }

   int64_t string::ToSint64(const std::string& val)
   {
      int64_t v;
      if (std::sscanf(val.c_str(), "%" TC_SINT64_FORMAT, &v) != 1)
      {
         return 0;
      }

      return v;
   }

   uint64_t string::ToUint64(const std::string& val)
   {
      uint64_t v;
      if (std::sscanf(val.c_str(), "%" TC_UINT64_FORMAT, &v) != 1)
      {
         return 0;
      }

      return v;
   }

   bool string::ToBool(const std::string& val)
   {
      char v;
      if (std::sscanf(val.c_str(), "%c", &v) != 1)
      {
         return false;
      }

      return v != 0;
   }

   std::string string::ToUpper(const std::string& text_in)
   {
      return StringImp::ToUpper(text_in);
   }

   std::string string::ToLower(const std::string& text_in)
   {
      return StringImp::ToLower(text_in);
   }

   bool string::Split(const std::string& text, const std::string& split,
      std::string& part1, std::string& part2)
   {
      return StringImp::Split(text, split, part1, part2);
   }

   bool string::Split(const std::string& text, std::string::size_type pos,
      std::string& part1, std::string& part2)
   {
      return StringImp::Split(text, pos, part1, part2);
   }

   std::vector< std::string > string::Split(const std::string& text, const std::string& limiter)
   {
      return StringImp::Split(text, limiter);
   }

   std::string string::Join(const std::vector<std::string>& texts, const std::string& limiter)
   {
     return StringImp::Join(texts, limiter);
   }

   std::string string::TrimmSpaces(const std::string& text)
   {
      return StringImp::TrimmSpaces(text);
   }

   std::string string::TrimmLeadingSpaces(const std::string& text)
   {
      return StringImp::TrimmLeadingSpaces(text);
   }

   std::string string::TrimmTrailingSpaces(const std::string& text)
   {
      return StringImp::TrimmTrailingSpaces(text);
   }

   std::string string::DeleteBlanksAndTabs(const std::string& text_in)
   {
      const std::string text = Replace(text_in, " ", "");
      return Replace(text, "\t", "");
   }

   std::string string::Replace(const std::string& text_in, char r1, char r2)
   {
      return StringImp::Replace(text_in, r1, r2);
   }

   std::string string::Replace(const std::string& text_in,
      const std::string& r1, const std::string& r2)
   {
      return StringImp::Replace(text_in, r1, r2);
   }

   int32_t string::Snprintf(char* buf, uint32_t size_of_buf, const char* fmt, ...)
   {
      va_list arguments;
      va_start(arguments, fmt);
      const int32_t len = StringTraits::VsnPrintf(buf, size_of_buf, fmt, arguments);
      va_end(arguments);

      return len;
   }

   int32_t string::VSnprintf(char* buf, uint32_t size_of_buf, const char *fmt, va_list arguments)
   {
      return StringTraits::VsnPrintf(buf, size_of_buf, fmt, arguments);
   }

   int32_t StringICompare(const char* s1, const char* s2)
   {
       return StringImp::StringICompare(s1, s2);
   }
   
   int32_t StringICompare(const char* s1, const char* s2, uint32_t len_to_compare)
   {
       return StringImp::StringICompare(s1, s2, len_to_compare);
   }

   std::string string::Print(const char *fmt, ...)
   {
      va_list arguments;
      va_start(arguments, fmt);
      std::string buf = StringImp::VPrint(fmt , arguments);
      va_end(arguments);

      return buf;
   }

   std::string string::VPrint(const char *fmt, va_list arguments)
   {
      return StringImp::VPrint(fmt , arguments);
   }

   uint32_t string::HexToUint32(const std::string& text)
   {
#ifdef TCOS_WINDOWS
      int64_t val = 0;
      if (std::sscanf(text.c_str(),"%I64x",&val) != 1)
#else
       int64_t val = 0;
      if (std::sscanf(text.c_str(), "%llx", &val) != 1)
#endif
      {
         return 0;
      }

      if (val > std::numeric_limits<uint32_t>::max() ||
         val < std::numeric_limits<uint32_t>::min())
      {
         return 0;
      }

      return static_cast<uint32_t>(val);
   }
}
