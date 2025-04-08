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
//  $Id: TCFileWin32.cpp,v 19a9fe061077 2014/07/01 18:05:17 thomas $
//*******************************************************************************
#include "TCDefines.h"

#if TCOS_WINDOWS
#include "TCWFile.h"

#include "TCWFileName.h"
#include "TCOutput.h"
#include "TCSystem.h"
#include "TCUtil.h"
#include "TCWString.h"

#include <Windows.h>
#include <WinBase.h>
#include <AccCtrl.h>
#include <AclAPI.h>
#if TC_WINDOWS_UWP
#include <fileapifromapp.h>
#endif

#undef CreateDir
#undef GetUserName
#undef CreateFile
#undef GetClassName

namespace tc::wfile
{
   bool ChangeDirectory(const std::wstring& directory)
   {
      if (directory.empty())
      {
         return false;
      }

      return ::SetCurrentDirectoryW(directory.c_str()) == TRUE;
   }

   std::wstring GetDirectory()
   {
      wchar_t buffer[2048] = L"";
      if (!::GetCurrentDirectoryW(static_cast<DWORD>(util::ArraySize(buffer)), buffer))
      {
         return L"";
      }

      return buffer;
   }

#undef GetFileAttributes

   static WIN32_FILE_ATTRIBUTE_DATA GetFileAttributeData(const std::wstring& file)
   {
      const auto levels = GetFileExInfoStandard;
      WIN32_FILE_ATTRIBUTE_DATA attribute_data;
      ZeroMemory(&attribute_data, sizeof(attribute_data));
      attribute_data.dwFileAttributes = INVALID_FILE_ATTRIBUTES;
#if TC_WINDOWS_UWP
      if (::GetFileAttributesExFromAppW(file.c_str(), levels, &attribute_data))
      {
         return attribute_data;
      }
#else
      if (::GetFileAttributesExW(file.c_str(), levels, &attribute_data))
      {
         return attribute_data;
      }
#endif
      return attribute_data;
   }

   static DWORD GetFileAttributes(const std::wstring& file)
   {
      return GetFileAttributeData(file).dwFileAttributes;
   }


   bool Exists(const std::wstring & file)
   {
      return GetFileAttributes(file) != INVALID_FILE_ATTRIBUTES;
   }

   // Check whether its a directory
   bool IsDirectory(const std::wstring & file)
   {
      const auto atts = GetFileAttributes(file);
      return (atts != INVALID_FILE_ATTRIBUTES) && (atts & FILE_ATTRIBUTE_DIRECTORY);
   }

   // Check whether its a file
   bool IsFile(const std::wstring & file)
   {
      const auto atts = GetFileAttributes(file);
      return (atts != INVALID_FILE_ATTRIBUTES) && !(atts & FILE_ATTRIBUTE_DIRECTORY);
   }

   // Return 1 if file is readable
   bool IsReadable(const std::wstring &file)
   {
      const auto atts = GetFileAttributes(file);
      return atts != INVALID_FILE_ATTRIBUTES;
   }

   // Return 1 if file is writeable
   bool IsWriteable(const std::wstring &file)
   {
      const auto atts = GetFileAttributes(file);
      return (atts != INVALID_FILE_ATTRIBUTES) && !(atts & FILE_ATTRIBUTE_READONLY);
   }

   // Return 1 if file is executable
   bool IsExecutable(const std::wstring &file)
   {
      const auto atts = GetFileAttributes(file);
      return (atts != INVALID_FILE_ATTRIBUTES) && (atts & FILE_EXECUTE);
   }

   // Change the mode flags for this file
   bool SetFileAttr(const std::wstring &file, uint32_t attr)
   {
      attr &= (static_cast<uint32_t>(FileAttributes::READONLY) | 
         static_cast<uint32_t>(FileAttributes::ARCHIVE) | 
         static_cast<uint32_t>(FileAttributes::SYSTEM) |
         static_cast<uint32_t>(FileAttributes::HIDDEN));
#if TC_WINDOWS_UWP
      return ::SetFileAttributesFromAppW(file.c_str(), attr) == TRUE;
#else
      return SetFileAttributesW(file.c_str(), attr) == TRUE;
#endif
   }

   bool Remove(const std::wstring & file)
   {
      if (!Exists(file))
      {
         return false;
      }

      if (IsDirectory(file))
      {
#if TC_WINDOWS_UWP
         return ::RemoveDirectoryFromAppW(file.c_str()) == TRUE;
#else
         return ::RemoveDirectoryW(file.c_str()) == TRUE;
#endif
      }
      else
      {
#if TC_WINDOWS_UWP
         return ::DeleteFileFromAppW(file.c_str()) == TRUE;
#else
         return ::DeleteFileW(file.c_str()) == TRUE;
#endif
      }
   }

   DWORD WINAPI CopyProgress(
      LARGE_INTEGER TotalFileSize,
      LARGE_INTEGER TotalBytesTransferred,
      LARGE_INTEGER /*StreamSize*/,
      LARGE_INTEGER /*StreamBytesTransferred*/,
      DWORD /*dwStreamNumber*/,
      DWORD /*dwCallbackReason*/,
      HANDLE /*hSourceFile*/,
      HANDLE /*hDestinationFile*/,
      LPVOID lpData
      )
   {
      auto progress(*static_cast<SharedPtr<Progress>*>(lpData));
      const auto val = static_cast<double>(TotalFileSize.QuadPart) / TotalBytesTransferred.QuadPart * 100.0;

      progress->OnCurrentStatus(static_cast<uint32_t>(val));

      return 0;
   }

   bool Copy(const std::wstring& source, const std::wstring& dest, SharedPtr<Progress> progress)
   {
      if (!IsFile(source))
      {
         return false;
      }

#if TC_WINDOWS_UWP
      return ::CopyFileFromAppW(source.c_str(), dest.c_str(), FALSE) == TRUE;
#else
      if (progress)
      {
         if (::CopyFileExW(source.c_str(), dest.c_str(), &CopyProgress, &progress, 0, 0) != 0)
         {
            progress->OnCurrentStatus(100);
            return true;
         }
         else
         {
            return false;
         }

      }
      else
      {
         return ::CopyFileExW(source.c_str(), dest.c_str(), 0, 0, 0, 0) != 0;
      }
#endif
   }

   bool Move(const std::wstring& source, const std::wstring& dest)
   {
      if (!IsFile(source))
      {
         return false;
      }

#if TC_WINDOWS_UWP
      return ::MoveFileFromAppW(source.c_str(), dest.c_str());
#else
      return ::MoveFileExW(source.c_str(), dest.c_str(),
         MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH) == TRUE;
#endif
   }

   bool CreateDir(const std::wstring& path)
   {
      if (path.empty())
      {
         return false;
      }

#if TC_WINDOWS_UWP
      return ::CreateDirectoryFromAppW(path.c_str(), nullptr) == TRUE;
#else
      return ::CreateDirectoryW(path.c_str(), nullptr) == TRUE;
#endif
   }

   bool RemoveDir(const std::wstring& path)
   {
      if (path.empty())
      {
         return false;
      }

#if TC_WINDOWS_UWP
      return ::RemoveDirectoryFromAppW(path.c_str()) == TRUE;
#else
      return ::RemoveDirectoryW(path.c_str()) == TRUE;
#endif
   }

   // Return time file was last modified
   uint64_t GetModificationTime(const std::wstring &file)
   {
      const auto attribute_data = GetFileAttributeData(file);
      if (attribute_data.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
      {
         return 0;
      }

      ULARGE_INTEGER ltime;
      ltime.LowPart  = attribute_data.ftLastWriteTime.dwLowDateTime;
      ltime.HighPart = attribute_data.ftLastWriteTime.dwHighDateTime;
      return ltime.QuadPart;
   }

   // Return time file was last accessed
   uint64_t GetLastAccessTime(const std::wstring &file)
   {
      const auto attribute_data = GetFileAttributeData(file);
      if (attribute_data.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
      {
         return 0;
      }

      ULARGE_INTEGER ltime;
      ltime.LowPart  = attribute_data.ftLastAccessTime.dwLowDateTime;
      ltime.HighPart = attribute_data.ftLastAccessTime.dwHighDateTime;
      return ltime.QuadPart;
   }

   // Return time when created
   uint64_t GetCreationTime(const std::wstring &file)
   {
      const auto attribute_data = GetFileAttributeData(file);
      if (attribute_data.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
      {
         return 0;
      }

      ULARGE_INTEGER ltime;
      ltime.LowPart  = attribute_data.ftCreationTime.dwLowDateTime;
      ltime.HighPart = attribute_data.ftCreationTime.dwHighDateTime;
      return ltime.QuadPart;
   }

   // Get file size
   uint64_t GetFileSize(const std::wstring &file)
   {
      const auto attribute_data = GetFileAttributeData(file);
      if (attribute_data.dwFileAttributes == INVALID_FILE_ATTRIBUTES)
      {
         return 0;
      }

      ULARGE_INTEGER size;
      size.LowPart = attribute_data.nFileSizeLow;
      size.HighPart = attribute_data.nFileSizeHigh;

      return size.QuadPart;
   }

   static std::wstring Win32FileInformation(const std::wstring &file, SECURITY_INFORMATION info_type)
   {
#if TC_WINDOWS_UWP
      auto file_handle = ::CreateFile2FromAppW(file.c_str(),
                                               FILE_READ_ATTRIBUTES,
                                               FILE_SHARE_READ,
                                               OPEN_EXISTING,
                                               nullptr);

#else
      // Get the handle of the file object.
      const HANDLE file_handle = ::CreateFile2(file.c_str(),
                                               FILE_READ_ATTRIBUTES,
                                               FILE_SHARE_READ,
                                               OPEN_EXISTING,
                                               nullptr);
#endif
      // Check GetLastError for CreateFile error code.
      if (file_handle == INVALID_HANDLE_VALUE)
      {
         TCERRORS("TCBASE", system::GetLastErrorMessage().c_str());
         return L"";
      }

      // Allocate memory for the SID structure.
      auto pSidOwner = static_cast<PSID>(GlobalAlloc(GMEM_FIXED, sizeof(PSID)));

      // Allocate memory for the security descriptor structure.
      auto p_sd = static_cast<PSECURITY_DESCRIPTOR>(::GlobalAlloc(GMEM_FIXED, sizeof(PSECURITY_DESCRIPTOR)));

      // Get the owner SID of the file.
      // Check GetLastError for GetSecurityInfo error condition.
      if (GetSecurityInfo(file_handle, SE_FILE_OBJECT, info_type,
         &pSidOwner, nullptr, nullptr, nullptr, &p_sd) != ERROR_SUCCESS)
      {
         TCERRORS("TCBASE", system::GetLastErrorMessage().c_str());
         return L"";
      }
      ::CloseHandle(file_handle);
      ::GlobalFree(p_sd);

      auto eUse = SidTypeUnknown;
      char AcctName[256];
      char DomainName[256];
      auto dwAcctName = static_cast<DWORD>(util::ArraySize(AcctName));
      auto dwDomainName = static_cast<DWORD>(util::ArraySize(DomainName));
      // call to LookupAccountSid to get the account name.
      if (!LookupAccountSidA(
         nullptr,                          // name of local or remote computer
         pSidOwner,                     // security identifier
         AcctName,                      // account name buffer
         &dwAcctName,                   // size of account name buffer 
         DomainName,                    // domain name
         &dwDomainName,                 // size of domain name buffer
         &eUse))                        // SID type
      {
         TCERRORS("TCBASE", system::GetLastErrorMessage().c_str());
         return L"";
      }
      ::GlobalFree(pSidOwner);

      return wstring::ToString(std::string(DomainName) + "\\" + AcctName);
   }

   // get name of file user
   std::wstring GetFileUser(const std::wstring &file)
   {
      return Win32FileInformation(file, OWNER_SECURITY_INFORMATION);
   }

   // get name of file group
   std::wstring GetFileGroup(const std::wstring &file)
   {
      return Win32FileInformation(file, GROUP_SECURITY_INFORMATION);
   }

   std::vector<std::wstring> GetFileListOfDirectory(const std::wstring & searchDirectory,
                                const std::wstring & searchExtension)
   {
      std::vector<std::wstring> fileList;

      std::wstring search_dir;
      if (searchExtension.empty())
      {
         search_dir = wfile_name::AddFileNameAndPath(L"*", searchDirectory);
      }
      else
      {
         search_dir = wfile_name::AddFileNameAndPath(L"*." + searchExtension, searchDirectory);
      }

      WIN32_FIND_DATAW find_data;
      const FINDEX_INFO_LEVELS info_levels = FindExInfoBasic;
      const FINDEX_SEARCH_OPS search_ops = FindExSearchNameMatch;
      const DWORD additional = FIND_FIRST_EX_CASE_SENSITIVE | FIND_FIRST_EX_LARGE_FETCH;
#if TC_WINDOWS_UWP
      auto find_file = ::FindFirstFileExFromAppW(search_dir.c_str(), info_levels, &find_data,
                                                       search_ops, nullptr, additional);
#else
      const auto find_file = ::FindFirstFileExW(search_dir.c_str(), info_levels, &find_data,
                                                search_ops, nullptr, additional);
#endif
      if (find_file == INVALID_HANDLE_VALUE)
      {
         return fileList;
      }

      do
      {
         std::wstring file = find_data.cFileName;
         if (wfile_name::GetName(file) == L"." || wfile_name::GetName(file) == L"..")
         {
            continue;
         }

         fileList.push_back(file);
      }
      while (::FindNextFileW(find_file, &find_data));

      ::FindClose(find_file);
      return fileList;
   }

   
   void GetFileInfosOfDirectory(std::vector<FileInfo>& file_infos,
      const std::wstring & searchDirectory,
      const std::wstring& searchExtension)
   {
      std::wstring search_dir;
      if (searchExtension.empty())
      {
         search_dir = wfile_name::AddFileNameAndPath(L"*", searchDirectory);
      }
      else
      {
         search_dir = wfile_name::AddFileNameAndPath(L"*." + searchExtension, searchDirectory);
      }
      WIN32_FIND_DATAW find_data;
      const FINDEX_INFO_LEVELS info_levels = FindExInfoBasic;
      const FINDEX_SEARCH_OPS search_ops = FindExSearchNameMatch;
      const DWORD additional = FIND_FIRST_EX_CASE_SENSITIVE | FIND_FIRST_EX_LARGE_FETCH;
#if TC_WINDOWS_UWP
      const auto find_file = ::FindFirstFileExFromAppW(search_dir.c_str(), info_levels, &find_data,
                                                       search_ops, nullptr, additional);
#else
      const auto find_file = ::FindFirstFileExW(search_dir.c_str(), info_levels, &find_data,
                                                search_ops, nullptr, additional);
#endif
      if (find_file == INVALID_HANDLE_VALUE)
      {
         return;
      }

      do
      {
         FileInfo file_info;
         file_info.name = find_data.cFileName;
         if (wfile_name::GetName(file_info.name) == L"." || 
             wfile_name::GetName(file_info.name) == L"..")
         {
            continue;
         }

         LARGE_INTEGER last_write_time;
         last_write_time.LowPart  = find_data.ftLastWriteTime.dwLowDateTime;
         last_write_time.HighPart = find_data.ftLastWriteTime.dwHighDateTime;
         file_info.last_modified  = last_write_time.QuadPart;
         file_info.is_directory   = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY;
         LARGE_INTEGER file_size;
         file_size.LowPart   = find_data.nFileSizeLow;
         file_size.HighPart  = find_data.nFileSizeHigh;
         file_info.file_size = file_size.QuadPart;
         file_infos.push_back(file_info);
      }
      while (::FindNextFileW(find_file, &find_data));

      ::FindClose(find_file);
   }
}
#endif
