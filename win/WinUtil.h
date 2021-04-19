// Contains declarations of OS-specific (Windows) classes, functions.
#ifndef _WINUTIL_H__
#define _WINUTIL_H__

#include <Windows.h>
#include "..\Util.h"
#include <vector>
#include <string>

namespace OTInterviewExercise1
{
    // (Un)initialize COM
    class CCoInitialize
    {
    public:
        enum class ComThreadModel
        {
            Apartment,
            MultiThreaded
        };
        CCoInitialize(ComThreadModel comThreadModel = ComThreadModel::Apartment) noexcept;
        ~CCoInitialize() noexcept;
        bool IsOk(std::wstring& o_sError) const noexcept;
    private:
        void Init();
        CCoInitialize::ComThreadModel mComThreadModel;
        bool mComInitialized;
        std::wstring mSError;
    };

    // Low-level class for Windows-specific (un)initialization
    class COsInitialization::COsInitializationImpl
    {
    public:
        COsInitializationImpl() = default;
        ~COsInitializationImpl() = default;
        bool IsOk(std::wstring& o_errorMsg) const noexcept;
    private:
        // Initialize COM (it will be unitialized by d-tor). This is Windows specific.
        CCoInitialize mCoInitialize;
    };

    // Low-level class for retrieving contents of text files (ASCII or UTF8)
    class CTextFileReader::CTextFileReaderImpl
    {
    public:
        CTextFileReaderImpl(const wchar_t* filePathName);
        ~CTextFileReaderImpl() = default;
        Status GetStatus(std::wstring& o_sErrorMsg) const noexcept;
        bool GetContents(std::vector<unsigned char>& o_fileData) const noexcept;
    private:
        enum
        {
            INTERNAL_BUF_SIZE = 2048
        };
        std::vector<unsigned char> mFileContents;
        Status mStatus;
        std::wstring mErrMsg;
    };

    class CLogger::CLoggerImpl
    {
    public:
        void Log(const wchar_t *message);
    };
}

#endif