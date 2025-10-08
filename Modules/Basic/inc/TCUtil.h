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
//  $Id: TCUtil.h,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#ifndef _TC_UTIL_H_
#define _TC_UTIL_H_

#include "TCTypes.h"

namespace tc
{
   /**
   * @addtogroup TC_BASE
   * @{
   */

   /**
   * @file 
   * This header file provides the definition of the namespace tc::util. 
   * 
   * @author Thomas Goessler 
   */

   /**
   * @brief Defines basic functions like Min, Max, Swap ...
   */
   namespace util
   {
      /** @return the absolute value of the given value */
      template <class T>
      inline T Abs(const T& x)
      {
         return (x < 0) ? -x : x;
      }

      /** @return maximum of two values */
      template <class T>
      inline const T& Max(const T& x, const T& y)
      {
         return (x > y) ? x : y;
      }
      /** @return maximum of three values */
      template <class T>
      inline const T& Max(const T& x, const T& y, const T& z)
      {
         return Max(x, Max(y, z));
      }

      /** @return minimum of two values */
      template <class T>
      inline const T& Min(const T& x, const T& y)
      {
         return (x < y) ? x : y;
      }
      /** @return minimum of three values */
      template <class T>
      inline const T& Min(const T& x, const T& y, const T& z)
      {
         return Min(x, Min(y, z));
      }


      /** @return Swap the two variables */
      template <class T>
      inline void Swap(T& x, T& y)
      {
         T tmp = x;
         x     = y;
         y     = tmp;
      }

      /** @brief Really frees memory of vector */
      template <class T>
      inline void FreeMemoryOfStlContainer(T& container)
      {
         T tmp;
         container.swap(tmp);
      }

      /** @brief Minimizes the memory usage of a vector */
      template <class T>
      inline void MinimizeMemoryOfStlContainer(T& container)
      {
         T tmp(container);
         container.swap(tmp);
      }

      /**
      * Function template for safe memory pointer deletion
      * @param mem memory pointer to delete
      */
      template <class T>
      inline void SafeRelease(T& mem)
      {
         if (mem != 0)
         {
            delete mem;
            mem = 0;
         }
      }

      /**
      * Function template for safe memory pointer deletion of an allocated array
      * @param mem memory pointer to delete
      */
      template <class T>
      inline void SafeReleaseArray(T& mem)
      {
         if (mem != 0)
         {
            delete[] mem;
            mem = 0;
         }
      }

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4100)
#endif
      /** @return The number of elements of an C array */
      template <class T>
      uint32_t ArraySize(const T& array)
      {
         return sizeof(array)/sizeof(array[0]);
      }
#ifdef _MSC_VER
#pragma warning (pop)
#endif

      /** @brief Swaps the bytes of an uint8_t value */
      inline void SwapBytes(uint8_t&)
      {
      }
      /** @brief Swaps the bytes of an uint16_t value */
      inline void SwapBytes(uint16_t& val)
      {
          val = (val >> 8) | (val << 8);
      }
      /** @brief Swaps the bytes of an uint32_t value */
      inline void SwapBytes(uint32_t& val)
      {
          val = (val >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | (val << 24);
      }
      /** @brief Swaps the bytes of an uint64_t value */
      inline void SwapBytes(uint64_t& val)
      {
         uint32_t *val_ptr = (uint32_t *)&val;
         uint32_t tmp1 = val_ptr[0];
         uint32_t tmp2 = val_ptr[1];
         SwapBytes(tmp1);
         SwapBytes(tmp2);
         val_ptr[0] = tmp2;
         val_ptr[1] = tmp1;
      }

      /** @brief Swaps the bytes of an int8_t value */
      inline void SwapBytes(int8_t&)
      {
      }
      /** @brief Swaps the bytes of an int16_t value */
      inline void SwapBytes(int16_t& val)
      {
          SwapBytes((uint16_t&)val);
      }
      /** @brief Swaps the bytes of an int32_t value */
      inline void SwapBytes(int32_t& val)
      {
          SwapBytes((uint32_t&)val);
      }
      /** @brief Swaps the bytes of an int64_t value */
      inline void SwapBytes(int64_t& val)
      {
          SwapBytes((uint64_t&)val);
      }
      /** @brief Swaps the bytes of an float value */
      inline void SwapBytes(float& val)
      {
          SwapBytes((uint32_t&)val);
      }
      /** @brief Swaps the bytes of an double value */
      inline void SwapBytes(double& val)
      {
          SwapBytes((uint64_t&)val);
      }

      /** @brief Swaps the bytes of any type */
      template <class T>
      void SwapBytes(T &val)
      {
         uint8_t *buffer = static_cast<uint8_t*>(&val);
         for (uint32_t i = 0; i<sizeof(val)/2; i++)
         {
            const uint8_t b = buffer[i];

            buffer[i] = buffer[sizeof(val)/2 - i - 1];
            buffer[sizeof(val)/2 - i - 1] = b;
         }
      }
      /** @return if byte ordering is big endian */
      inline bool IsBigEndian()
      {
         union
         {
            int32_t int_val;
            char bytes[4];
         } data;
         data.int_val = 1;

         return (data.bytes[3] == 1);
      }
      /** @return if byte ordering is little endian */
      inline bool IsLittleEndian()
      {
         return !IsBigEndian();
      }

   }

   /**
   * @}
   */
}

#endif // _TC_UTIL_H_
