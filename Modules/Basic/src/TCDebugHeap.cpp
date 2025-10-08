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
//  $Id: TCDebugHeap.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#include "TCDebugHeap.h"

#include "TCHeapMutex.h"
#include "TCSystem.h"
#include "TCString.h"

#include <iostream>
#include <cstring>

#include "TCNewDisable.h"

namespace tc
{
   class DebugHeap::AllocList
   {
   public:
      class Entry
      {
      public:
         void Init(uint32_t size, const char* file, uint32_t line)
         {
            const char* name;
            name = std::strrchr(file, '\\');
            if (!name)
            {
               name = std::strrchr(file, '/');
            }
            if (!name)
            {
               name = file;
            }
            else
            {
               ++name;
            }
            std::strncpy(m_file, name, sizeof(m_file) - 1);
            m_file[sizeof(m_file) - 1] = '\0';
            m_line   = line;
            m_user   = this + 1;
            m_size   = size;
            m_thread = system::GetCurrentThreadID();
         }

         void*        m_user;
         Entry*       m_prev;
         Entry*       m_next;
         uint32_t       m_line;
         uint32_t       m_size;
         uint32_t       m_thread;
         char         m_file[32];
      };

      AllocList()
         :m_first(0),
         m_last(0)
      {
      }

      uint32_t GetNumEntries() const
      {
         m_mutex.Lock();

         uint32_t num_blocks = 0;
         Entry* block = m_first;
         while (block)
         {
            num_blocks++;
            block = block->m_next;
         }
         m_mutex.UnLock();

         return num_blocks;
      }

      Entry* GetFirstEntry() {return m_first;}

      void Insert(Entry* entry_in, bool sorted)
      {
         m_mutex.Lock();

         Entry* entry_to_insert = static_cast<Entry*>(entry_in);
         if (!m_first)
         {
            m_first = entry_to_insert;
            m_last  = entry_to_insert;
            entry_to_insert->m_prev = 0;
            entry_to_insert->m_next = 0;
            m_mutex.UnLock();
            return;
         }

         if (sorted)
         {
            Entry* block = FindBlockWhereToInsert(entry_to_insert);
            if (block)
            {
               InsertAfter(block, entry_to_insert);
               m_mutex.UnLock();
               return;
            }
         }

         InsertAsFirst(entry_to_insert);

         m_mutex.UnLock();
      }

      void Remove(Entry* entry_in)
      {
         m_mutex.Lock();

         const Entry* block = static_cast<Entry*>(entry_in);
         if (block->m_prev)
         {
            block->m_prev->m_next = block->m_next;
         }
         else
         {
            m_first = block->m_next;
         }

         if (block->m_next)
         {
            block->m_next->m_prev = block->m_prev;
         }
         else
         {
            m_last = block->m_prev;
         }

         m_mutex.UnLock();
      }

      void Sort()
      {
         m_mutex.Lock();
         Sort(&m_first, &m_last);
         m_mutex.UnLock();
      }

   private:
      void InsertAfter(Entry* block, Entry* entry_to_insert)
      {
         entry_to_insert->m_next = block->m_next;
         entry_to_insert->m_prev = block;
         if (block->m_next)
         {
            block->m_next->m_prev = entry_to_insert;
         }
         else
         {
            m_last = entry_to_insert;
         }
         block->m_next = entry_to_insert;

      }
      void InsertAsFirst(Entry* entry_to_insert)
      {
         entry_to_insert->m_next = m_first;
         entry_to_insert->m_prev = 0;

         m_first->m_prev = entry_to_insert;
         m_first = entry_to_insert;

      }

      static uint32_t IsLargerThan(const Entry* entry1, const Entry* entry2)
      {
         const int32_t str_diff  = std::strcmp(entry1->m_file, entry2->m_file);
         if (str_diff > 0)
         {
            return 1;
         }
         else if (str_diff == 0)
         {
            const int32_t line_diff = static_cast<int32_t>(entry1->m_line) - static_cast<int32_t>(entry2->m_line);
            if (line_diff > 0)
            {
               return 2;
            }
            else if (line_diff == 0)
            {
               const int32_t thread_diff = static_cast<int32_t>(entry1->m_thread) - static_cast<int32_t>(entry2->m_thread);
               if (thread_diff >= 0)
               {
                  return 3;
               }
            }
         }
         return 0;
      }

      Entry* FindBlockWhereToInsert(Entry* entry_to_insert)
      {
         if (IsLargerThan(m_last, entry_to_insert))
         {
            Entry* entry = m_first;
            while (entry)
            {
               if (IsLargerThan(entry, entry_to_insert) > 0)
               {
                  return entry->m_prev;
               }
               entry = entry->m_next;
            }
         }

         return m_last;
      }

      void Sort(AllocList::Entry** first, AllocList::Entry** last)
      {
         // Apply merge sort to list. This performs sort in O(n*log(n)) 
         // Memory consumption: O(log(n)) for stack. 
         // - best and worst case.
         if (*first == *last)
         {
            return;
         }

         AllocList::Entry* right_first = *first;
         bool do_inc = false;
         for (AllocList::Entry* loop = *first; loop != (*last)->m_next; loop = loop->m_next)
         {
            if (do_inc)
            {
               right_first = right_first->m_next;
            }
            do_inc = !do_inc;
         }
         AllocList::Entry* left_last = right_first->m_prev;

         if (*first != left_last)
         {
            Sort(first, &left_last);
         }

         if (right_first == (*last)->m_next)
         {
            return;
         }

         Sort(&right_first, last);

         AllocList::Entry* loop_l = *first;
         AllocList::Entry* loop_r = right_first;

         // Merge two sections in that elements from right section are moved into
         // left section if necessary
         while (loop_l != loop_r && (loop_r != (*last)->m_next))
         {
            if (IsLargerThan(loop_l, loop_r)) //R1677.16726
            {
               // move element from right section to left section
               AllocList::Entry* next_r = loop_r->m_next;
               AllocList::Entry* prev_r = loop_r->m_prev;
               Remove(loop_r);
               if (loop_r == (*last))
               {
                  (*last) = prev_r;
               }

               if (loop_l->m_prev)
               {
                  InsertAfter(loop_l->m_prev, loop_r);
               }
               else
               {
                  InsertAsFirst(loop_r);
               }

               if (loop_l == (*first))
               {
                  *first = loop_r; 
               }

               loop_r = next_r; 
            }
            else
            {
               loop_l = loop_l->m_next;
            }
         }
      }

   private:
      mutable Mutex m_mutex;
      Entry* m_first;
      Entry* m_last;
   };

   DebugHeap::DebugHeap(Heap& heap)
      :m_heap(heap),
      m_alloc_list(0)
   {
      m_alloc_list = new (heap.Alloc(sizeof(AllocList))) AllocList;
   }


   DebugHeap::~DebugHeap()
   {
      PrintMemoryLeaks(std::cout);
      m_alloc_list->~AllocList();
      m_heap.Free(m_alloc_list);
      m_alloc_list = 0;
   }

   void* DebugHeap::Alloc(uint32_t size)
   {
      return Alloc(size, "unknown alloc", 0);
   }

   void* DebugHeap::Alloc(uint32_t size, const char *file_name, uint32_t line)
   {
      void* pointer = 0;

      AllocList::Entry* p = static_cast<AllocList::Entry*>(m_heap.Alloc(size + sizeof(AllocList::Entry)));
      if (p)
      {
         p->Init(size, file_name, line);
         {
            m_alloc_list->Insert(p, false);
         }

         pointer = p + 1;
      }

      return pointer;
   }

   void* DebugHeap::ReAlloc(void* memory, uint32_t size,
      const char* file_name, uint32_t line)
   {
      void* new_memory = Alloc(size, file_name, line);
      if (!new_memory)
      {
         Free(memory);
         return 0;
      }

      if (!memory)
      {
         return new_memory;
      }

      if (!IsAHeapPointer(memory))
      {
         Free(new_memory);
         return 0;
      }

      AllocList::Entry* header = static_cast<AllocList::Entry*>(memory);
      --header;

      ::memcpy(new_memory, memory, header->m_size);

      Free(memory);

      return new_memory;
   }

   void* DebugHeap::ReAlloc(void* memory, uint32_t size)
   {
      return ReAlloc(memory, size, "unknown realloc", 0);
   }

   void DebugHeap::Free(void* memory)
   {
      if (memory)
      {
         void* pointer;

         AllocList::Entry* p = static_cast<AllocList::Entry*>(memory);
         --p;

         if (p->m_user == p + 1)
         {
            pointer = p;
            m_alloc_list->Remove(p);
         }
         else
         {
            pointer = memory;
         }

         m_heap.Free(pointer);
      }
   }

   bool DebugHeap::PrintMemoryLeaks(std::ostream& stream) const
   {
      // print memory leaks only if more than two allocations
      // this is because the shared pointer of the dump target needs this two allocations
      if (m_alloc_list->GetNumEntries() == 0)
      {
         stream << "No memory leaks found" << std::endl;
         return false;
      }

      // sort the list
      m_alloc_list->Sort();

      AllocList::Entry* current = m_alloc_list->GetFirstEntry();
      const AllocList::Entry* prev = current;
      uint32_t prev_num_bytes = 0;
      uint32_t prev_num_times = 0;
      uint32_t total_bytes    = 0;
      uint32_t total_allocs   = 0;
      while (current)
      {
         if (std::strcmp(current->m_file, prev->m_file) == 0 &&
            current->m_line == prev->m_line &&
            current->m_thread == prev->m_thread)
         {
            prev_num_bytes += current->m_size;
            prev_num_times++;
         }
         else
         {
            stream  << string::Print("%40s, line=%5lu, bytes=%8lu, ntime=%6lu, thread_id=%2lu",
                                     prev->m_file, prev->m_line, prev_num_bytes, prev_num_times, prev->m_thread)
                    << std::endl;

            prev           = current;
            prev_num_bytes = current->m_size;
            prev_num_times = 1;
         }

         total_bytes += current->m_size;
         total_allocs++;
         current = current->m_next;
      }

      if (prev)
      {
         stream  << string::Print("%40s, line=%5lu, bytes=%8lu, ntime=%6lu, thread_id=%2lu",
            prev->m_file, prev->m_line, prev_num_bytes, prev_num_times, prev->m_thread)
            << std::endl;
      }

      stream << "-------------------------------------------------------------------------------------" << std::endl;
      stream << string::Print("%40s              bytes=%8lu, ntime=%6lu\n", "Total leaked",total_bytes, total_allocs) << std::endl;

      return true;
   }

   bool DebugHeap::IsAHeapPointer(void* memory) const
   {
      return m_heap.IsAHeapPointer(memory);
   }
}
