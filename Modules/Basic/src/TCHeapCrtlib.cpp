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
//  $Id: TCHeapCrtlib.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#include "TCHeapCrtlib.h"

#include "TCUtil.h"

#include <new>
#include <cstdlib>
#include <cstring>

#define TC_HEAP_CRTLIB_INITIAL_VALUE   0xcd

namespace tc
{
   struct HeapCrtlib::Header
   {
      void*  m_user_visible_pointer;
      uint32_t m_allocation_size;
   };

   HeapCrtlib::HeapCrtlib()
   {
   }


   HeapCrtlib::~HeapCrtlib()
   {
   }

   void* HeapCrtlib::Alloc(uint32_t size)
   {
      const uint32_t allocation_size = size + sizeof(Header);
      Header* p = static_cast<Header*>(std::malloc(allocation_size));
      if (p)
      {
#ifdef DEBUG
         std::memset(p, TC_HEAP_CRTLIB_INITIAL_VALUE, allocation_size);
#endif

         p->m_user_visible_pointer = p + 1;
         p->m_allocation_size = allocation_size;
         return p + 1;
      }
      else
      {
         throw std::bad_alloc();
      }
   }


   void* HeapCrtlib::Alloc(uint32_t size, const char*, uint32_t)
   {
      return Alloc(size);
   }

   void* HeapCrtlib::ReAlloc(void* memory, uint32_t size)
   {
      void* new_memory = Alloc(size);

      if (!new_memory)
      {
         Free(memory);
         throw std::bad_alloc();
      }

      if (!memory)
      {
         return new_memory;
      }

      if (!IsAHeapPointer(memory))
      {
         Free(new_memory);
         throw std::bad_alloc();
      }

      Header* header = static_cast<Header*>(memory);
      --header;

      const uint32_t mem_size = util::Min(size, header->m_allocation_size - static_cast<uint32_t>(sizeof(Header)));
      std::memcpy(new_memory, memory, mem_size);

      Free(memory);

      return new_memory;
   }

   void* HeapCrtlib::ReAlloc(void* memory, uint32_t size, const char*, uint32_t)
   {
      return ReAlloc(memory, size);
   }

   void HeapCrtlib::Free(void* memory)
   {
      if (IsAHeapPointer(memory))
      {
         Header* header = static_cast<Header*>(memory);
         --header;

         std::free(header);
      }
      else
      {
      }
   }

   bool HeapCrtlib::IsAHeapPointer(void* memory) const
   {
      if (!memory)
      {
         return false;
      }

      Header* header = static_cast<Header*>(memory);
      --header;
      return header->m_user_visible_pointer == memory;
   }
}
