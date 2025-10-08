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
//  $Id: TCTime.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************

#include "TCTime.h"

#include "TCException.h"

#if TCOS_WINDOWS
#   include <windows.h>
#elif TCOS_POSIX
#   include <time.h>
#   include <sys/time.h>
#endif
#include <limits>

#include "TCNewEnable.h"

namespace tc
{
   class UnderflowError: public Exception
   {
   public:
      UnderflowError()
         :Exception("tc::Time::UnderflowError") {}
   };

   Time Time::FromSeconds(uint64_t seconds)
   {
      Time time;
      time.m_secs = seconds;
      time.m_nsecs = 0;
      return time;
   }

   Time Time::FromMilliSeconds(uint64_t milli_seconds)
   {
      Time time;
      time.m_secs = milli_seconds / ONE_SECOND_AS_MILLI_SECONDS;
      time.m_nsecs = (milli_seconds % ONE_SECOND_AS_MILLI_SECONDS) * ONE_MILLI_SECOND_AS_NANO_SECONDS;
      return time;
   }

   Time Time::FromMicroSeconds(uint64_t micro_seconds)
   {
      Time time;
      time.m_secs = micro_seconds / ONE_SECOND_AS_MICRO_SECONDS;
      time.m_nsecs = (micro_seconds % ONE_SECOND_AS_MICRO_SECONDS) * ONE_MICRO_SECOND_AS_NANO_SECONDS;
      return time;
   }

   Time Time::FromNanoSeconds(uint64_t nano_seconds)
   {
      Time time;
      time.m_secs = nano_seconds / ONE_SECOND_AS_NANO_SECONDS;
      time.m_nsecs = nano_seconds % ONE_SECOND_AS_NANO_SECONDS;
      return time;
   }

   Time Time::Now()
   {
      Time time;
#if TCOS_WINDOWS || TCOS_WINCE_40
      FILETIME ft;
      GetSystemTimeAsFileTime(&ft);
      ULARGE_INTEGER uval;
      uval.LowPart = ft.dwLowDateTime;
      uval.HighPart = ft.dwHighDateTime;
      time = Time::FromNanoSeconds(uval.QuadPart * 100);
#else
#ifdef TCOS_FUJITSU
      struct timeval tp;
      gettimeofday(&tp);
#else
      struct timeval tp;
      struct timezone tzp;
      gettimeofday(&tp, &tzp);
#endif
      time.m_secs = tp.tv_sec;
      time.m_nsecs = tp.tv_usec * 1000;
#endif
      return time;
   }

   Time Time::NowMonotonic()
   {
      Time time;
#if TCOS_WINDOWS || TCOS_WINCE_40
      static bool s_init(false);
      static double s_frequence(0);
      if (!s_init)
      {
         LARGE_INTEGER frequence;
         ::QueryPerformanceFrequency(&frequence);

         s_init      = true;
         s_frequence = frequence.QuadPart/static_cast<double>(ONE_SECOND_AS_NANO_SECONDS);
      }

      if (s_frequence > 0)
      {
         LARGE_INTEGER ticks;
         ::QueryPerformanceCounter(&ticks);
         const uint64_t mticks = static_cast<uint64_t>(ticks.QuadPart / s_frequence);
         time = Time::FromNanoSeconds(mticks);
      }
      else
      {
         time = Time::FromMilliSeconds(::GetTickCount64());
      }
#else
      struct timespec ts;
      ::clock_gettime(CLOCK_MONOTONIC, &ts);

      time.m_secs = ts.tv_sec;
      time.m_nsecs = ts.tv_nsec;
#endif
      return time;
   }


   Time::Time(uint64_t secs, uint64_t nsecs)
   {
      const Time time = FromSeconds(secs) + FromNanoSeconds(nsecs);
      m_secs  = time.m_secs;
      m_nsecs = time.m_nsecs;
   }

   Time& Time::operator+=(const Time& time_to_add)
   {
      m_secs  += time_to_add.m_secs;
      m_nsecs += time_to_add.m_nsecs;
      if (m_nsecs >= ONE_SECOND_AS_NANO_SECONDS)
      {
         m_secs++;
         m_nsecs -= ONE_SECOND_AS_NANO_SECONDS;
      }
      return *this;
   }

   Time& Time::operator-=(const Time& time_to_sub)
   {
      uint64_t sub_sec = 0;
      if (m_nsecs < time_to_sub.m_nsecs)
      {
         sub_sec = 1;
         m_nsecs += ONE_SECOND_AS_NANO_SECONDS;
      }
      m_nsecs -= time_to_sub.m_nsecs;

      if (m_secs < (time_to_sub.m_secs + sub_sec))
      {
         throw UnderflowError();
      }

      m_secs -= (time_to_sub.m_secs + sub_sec);
      return *this;
   }

   bool Time::operator<(const Time& a) const
   {
      if (m_secs < a.m_secs)
      {
         return true;
      }
      else if (m_secs == a.m_secs)
      {
         return (m_nsecs < a.m_nsecs);
      }
      return false;
   }

   Time Time::Infinite()
   {
      return Time(std::numeric_limits<uint64_t>::max(), 0);
   }

   Time Time::Zero()
   {
      return Time(0, 0);
   }

   StreamPtr operator>>(StreamPtr stream, Time& time)
   {
      return stream >> time.m_secs >> time.m_nsecs;
   }

   StreamPtr operator<<(StreamPtr stream, const Time& time)
   {
      return stream << time.m_secs << space << time.m_nsecs;
   }
}
