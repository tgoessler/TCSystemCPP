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
//  $Id: TCStringImp.h,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************
#ifndef _TC_STRING_IMPL_H_
#define _TC_STRING_IMPL_H_

#include "TCScopedArray.h"

#include <string>
#include <cstdarg>
#include <cctype>
#include <cstdio>

#ifndef va_copy
#define va_copy(a, b) (a=b)
#endif

namespace tc
{
   template <class STRING_CLASS, class TRAITS>
   class StringImpTmpl
   {
      typedef typename STRING_CLASS::value_type value_type;
      typedef typename STRING_CLASS::size_type size_type;
   public:
      static STRING_CLASS ToUpper(const STRING_CLASS& text_in)
      {
         STRING_CLASS text(text_in);
         for (size_type i=0; i<text.length(); i++)
         {
            text[i] = static_cast<value_type>(TRAITS::ToUpper(text[i]));
         }

         return text;
      }

      static STRING_CLASS ToLower(const STRING_CLASS& text_in)
      {
         STRING_CLASS text(text_in);
         for (size_type i=0; i<text.length(); i++)
         {
            text[i] = static_cast<value_type>(TRAITS::ToLower(text[i]));
         }

         return text;
      }

      static bool Split(const STRING_CLASS& text, const STRING_CLASS& split,
         STRING_CLASS& part1, STRING_CLASS& part2)
      {
         size_type pos = text.find_first_of(split);
         return Split(text, pos, part1, part2);
      }

      static bool Split(const STRING_CLASS& text, size_type pos,
         STRING_CLASS& part1, STRING_CLASS& part2)
      {
         if (pos == STRING_CLASS::npos)
         {
            part1 = STRING_CLASS();
            part2 = STRING_CLASS();
            return false;
         }

         part1 = text.substr(0, pos);
         part2 = text.substr(pos+1);
         return true;
      }

      static std::vector<STRING_CLASS> Split(const STRING_CLASS& text, const STRING_CLASS& limiter)
      {
         std::vector<STRING_CLASS> data;

         size_type pos1 = text.find_first_not_of(limiter);
         while (pos1 != STRING_CLASS::npos)
         {
            size_type pos2 = text.find_first_of(limiter, pos1);
            if(pos2 == STRING_CLASS::npos)
            {
               pos2 = text.size();
            }
            data.push_back(text.substr(pos1, pos2 - pos1));
            pos1 = text.find_first_not_of(limiter, pos2);
         }

         return data;
      }

      static STRING_CLASS Join(const std::vector<STRING_CLASS>& texts, const STRING_CLASS& limiter)
      {
         STRING_CLASS text;
         for (auto it=texts.begin(); it!=texts.end();)
         {
            text += *it;
            if (++it != texts.end())
            {
               text += limiter;
            }
         }

         return text;
      }

      static STRING_CLASS DeleteBlanksAndTabsAtBorder(const STRING_CLASS& text_in)
      {
         size_type pos1 = text_in.find_first_not_of(" \t");
         // not found return empty string
         if (pos1 == STRING_CLASS::npos)
         {
            return "";
         }

         size_type pos2 = text_in.find_last_not_of(" \t");

         // get the substring
         return text_in.substr(pos1, pos2 - pos1 + 1);
      }

      static STRING_CLASS DeleteBlanksAndTabs(const STRING_CLASS& text_in)
      {
         STRING_CLASS text = Replace(text_in, " ", "");
         return Replace(text, "\t", "");
      }

      static STRING_CLASS Replace(const STRING_CLASS& text_in, value_type r1, value_type r2)
      {
         STRING_CLASS text(text_in);
         for (size_type i=0; i<text.length(); i++)
         {
            if (text[i] == r1)
            {
               text[i] = r2;
            }
         }

         return text;
      }

      static STRING_CLASS Replace(const STRING_CLASS& text_in,
         const STRING_CLASS& r1, const STRING_CLASS& r2)
      {
         STRING_CLASS text;

         size_type pos1 = 0;
         size_type pos2 = STRING_CLASS::npos;
         bool found = false;
         do
         {
            pos2 = text_in.find(r1, pos1);
            if (pos2 == STRING_CLASS::npos)
            {
               text += text_in.substr(pos1);
               found = false;
            }
            else
            {
               text += text_in.substr(pos1, pos2 - pos1);
               text += r2;

               pos1 = pos2 + r1.length();

               found = true;
            }
         }while(found);

         return text;
      }

      static STRING_CLASS VPrint(const value_type *fmt, va_list arguments_in)
      {
         STRING_CLASS return_string;
         uint32_t size_of_buf = 64;
         int32_t len;
         do 
         {
            size_of_buf *= 2;
            ScopedArray<value_type> buf_ptr = ScopedArray<value_type>(new value_type[size_of_buf]);

            va_list arguments;
            va_copy(arguments, arguments_in);
            len = TRAITS::VsnPrintf(buf_ptr, size_of_buf-1, fmt, arguments);
            va_end(arguments);

            if (len != -1)
            {
               return_string = buf_ptr;
            }
         } while(len == -1);

         return return_string;
      }

      static void ReplaceString(STRING_CLASS& data,
         const value_type* token, uint32_t token_length,
         const value_type* replace, uint32_t replace_length)
      {
         std::string::size_type pos = data.find(token, 0, token_length);

         while(pos != std::string::npos)
         {
            data.replace(pos, token_length, replace, replace_length);
            pos += replace_length;

            pos = data.find(token, pos);
         }
      }

      static int32_t StringICompare(const value_type* s1, const value_type* s2)
      {
         value_type c1, c2;

         for (;;)
         {
            c1 = *s1++;
            c2 = *s2++;
            // check for end of string
            if (c1 == 0 || c2 == 0)
            {
               break;
            }
            // check same case sensitive char
            if (c1 == c2)
            {
               continue;
            }

            // check lower case characters
            if ((c1 = TRAITS::ToLower(c1)) != (c2 = TRAITS::ToLower(c2)))
            {
               break;
            }
         }
         return static_cast<int32_t>(c1) - static_cast<int32_t>(c2);
      }


      static int32_t StringICompare(const value_type* s1, const value_type* s2, uint32_t len_to_compare)
      {
         value_type c1=0;
         value_type c2=0;

         for (uint32_t i=0; i<len_to_compare; i++)
         {
            c1 = *s1++;
            c2 = *s2++;
            // check for end of string
            if (c1 == 0 || c2 == 0)
            {
               break;
            }
            // check same case sensitive char
            if (c1 == c2)
            {
               continue;
            }

            // check lower case characters
            if ((c1 = TRAITS::ToLower(c1)) != (c2 = TRAITS::ToLower(c2)))
            {
               break;
            }
         }
         return static_cast<int32_t>(c1) - static_cast<int32_t>(c2);
      }
      static STRING_CLASS TrimmSpaces(const STRING_CLASS& text_in)
      {
         if (text_in.length() == 0) return STRING_CLASS();

         int32_t i;
         STRING_CLASS text(text_in);
         // skip spaces at the end
         for (i = static_cast<int32_t>(text.length()) - 1; i >= 0; i--)
         {
            if (TRAITS::IsSpace(text.at(i) ) )
            {
               text.at(i) = 0;
            }
            else
            {
               break;
            }
         }

         const value_type *dum = text.c_str();

         // skip leading spaces
         while (TRAITS::IsSpace(*dum) )
         {
            dum++;
         }

         return dum;
      }

      static STRING_CLASS TrimmLeadingSpaces(const STRING_CLASS& text)
      {
         if (text.length() == 0) return STRING_CLASS();

         const value_type *dum = text.c_str();

         // skip leading spaces
         while (TRAITS::IsSpace(*dum) )
         {
            dum++;
         }

         return dum;
      }

      static STRING_CLASS TrimmTrailingSpaces(const STRING_CLASS& text_in)
      {
         if (text_in.length() == 0) return STRING_CLASS();

         int32_t i;
         STRING_CLASS text(text_in);
        // skip spaces at the end
         for (i = static_cast<int32_t>(text.length()) - 1; i >= 0; i--)
         {
            if (TRAITS::IsSpace(text.at(i) ) )
            {
               text.at(i) = 0;
            }
            else
            {
               break;
            }
         }

         return text.c_str();
      }
   };

}

#endif // _TC_STRING_IMPL_H_
