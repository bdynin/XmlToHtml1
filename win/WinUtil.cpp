// Contains implementations of OS-specific (Windows) classes, functions.

#include "..\Util.h"
#include "WinUtil.h"
#include <assert.h>
#include <sstream>
#include <functional>
#include <algorithm>

namespace OTInterviewExercise1
{
    bool COsInitialization::COsInitializationImpl::IsOk(std::wstring& o_errorMsg) const noexcept
    {
        return mCoInitialize.IsOk(o_errorMsg);
    }

    CCoInitialize::CCoInitialize(CCoInitialize::ComThreadModel comThreadModel) noexcept:
        mComThreadModel(comThreadModel),
        mComInitialized(false)
    {
        std::string functionName;
        unsigned int lineNo = 0;
        try
        {
            Init();
            mComInitialized = true;
            return;
        }
        catch (const CException& ex)
        {
            std::wostringstream ss;
            ss << L"Exception caught. ";
            if (!ex.mErrorDescription.empty())
            {
                ss << L"System error: " << ex.mErrorDescription;
            }
            mSError = ss.str();
            functionName = ex.mFunctionName;
            lineNo = ex.mLineNo;
        }
        catch (const std::bad_alloc& /*ex*/)
        {
            mSError = L"Memory allocation error.";
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        catch (const std::exception& ex)
        {
            std::wostringstream ss;
            ss << L"C++ exception caught. ";
            if (ex.what())
            {
                std::vector<wchar_t> sWhat(strlen(ex.what()) + 1, 0);
                size_t numConverted = 0;
                mbstowcs_s(&numConverted, &sWhat[0], sWhat.size() + 1, ex.what(), sWhat.size());
                assert(numConverted);
                ss.write(static_cast<wchar_t*>(sWhat.data()), numConverted);
            }
            mSError = ss.str();
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        catch (...)
        {
            mSError = L"Unknown exception caught.";
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        LogError(functionName.c_str(), lineNo, mSError);;
    }

    CCoInitialize::~CCoInitialize()
    {
        if (mComInitialized)
            ::CoUninitialize();
    }
    
    bool CCoInitialize::IsOk(std::wstring& o_sError) const noexcept
    {
        o_sError.clear();
        bool status = mComInitialized;
        if (!status)
            o_sError = mSError;
        return status;
    }

    void CCoInitialize::Init()
    {
        assert(!mComInitialized);
        mSError.clear();
        DWORD dwCoInit = 0;
        switch (mComThreadModel)
        {
        case CCoInitialize::ComThreadModel::Apartment:
            dwCoInit = COINIT_APARTMENTTHREADED;
            break;
        case CCoInitialize::ComThreadModel::MultiThreaded:
            dwCoInit = COINIT_MULTITHREADED;
            break;
        default:
            THROW_ERROR(L"Unrecognized thread-model-type. Please choose Apartment or MultiThreaded.");
        }
        HRESULT hr = ::CoInitializeEx(nullptr, dwCoInit);
        if (FAILED(hr))
        {
            std::wostringstream ss;
            ss << L"CoInitializeEx() failed with error code: " << std::hex << hr;
            mSError = ss.str();
            THROW_ERROR(mSError.c_str());
        }
    }

    CTextFileReader::CTextFileReaderImpl::CTextFileReaderImpl(const wchar_t* filePathName) :
        mStatus(Status::NotFound)
    {
        std::string functionName;
        unsigned int lineNo = 0;
        try
        {
            HANDLE hFileFind = INVALID_HANDLE_VALUE;
            HANDLE hFile = INVALID_HANDLE_VALUE;

            // Cleanup resources before function returning
            auto cleanup = MakeRAIICleanup([&hFileFind, &hFile]() {
                if (INVALID_HANDLE_VALUE != hFileFind)
                {
                    ::FindClose(hFileFind);
                    hFileFind = INVALID_HANDLE_VALUE;
                }
                if (INVALID_HANDLE_VALUE != hFile)
                {
                    ::CloseHandle(hFile);
                    hFile = INVALID_HANDLE_VALUE;
                }
                });

            WIN32_FIND_DATAW data = { 0 };
            hFileFind = ::FindFirstFile(filePathName, &data);
            if (INVALID_HANDLE_VALUE == hFileFind)
            {
                DWORD lastErr = ::GetLastError();
                if (ERROR_FILE_NOT_FOUND == lastErr)
                {
                    THROW_ERROR_CODE(static_cast<int>(Status::NotFound), L"File not found");
                }
                else
                {
                    std::wostringstream ss;
                    ss << L"FindFirstFile failed. Error code: " << std::hex << lastErr;
                    THROW_ERROR_CODE(static_cast<int>(Status::FindError), ss.str().c_str());
                }
            }
            mStatus = Status::Found;

            hFile = ::CreateFile(
                filePathName,
                GENERIC_READ,
                0,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr
            );
            if (INVALID_HANDLE_VALUE == hFile)
            {
                DWORD lastErr = ::GetLastError();
                std::wostringstream ss;
                ss << L"CreateFile failed. Error code: " << std::hex << lastErr;
                THROW_ERROR_CODE(static_cast<int>(Status::ReadContentsError), ss.str().c_str());
            }
            LARGE_INTEGER liSize = { 0 };
            if (!GetFileSizeEx(hFile, &liSize))
            {
                DWORD lastErr = ::GetLastError();
                std::wostringstream ss;
                ss << L"GetFileSizeEx failed. Error code: " << std::hex << lastErr;
                THROW_ERROR_CODE(static_cast<int>(Status::ReadContentsError), ss.str().c_str());
            }
            else if (liSize.HighPart != 0)
            {
                THROW_ERROR_CODE(static_cast<int>(Status::ReadContentsError), L"File is too large");
            }
            else if (liSize.LowPart == 0)
            {
                THROW_ERROR_CODE(static_cast<int>(Status::NoContents), L"File is empty");
            }
            mStatus = Status::NoContents;
            BYTE buf[INTERNAL_BUF_SIZE] = { 0 };
            for (bool inLoop = true; inLoop;)
            {
                DWORD numRead = 0;
                if (!::ReadFile(hFile, buf, sizeof(buf), &numRead, nullptr))
                {
                    mFileContents.clear();
                    DWORD lastErr = ::GetLastError();
                    std::wostringstream ss;
                    ss << L"ReadFile failed. Error code: " << std::hex << lastErr;
                    THROW_ERROR_CODE(static_cast<int>(Status::ReadContentsError), ss.str().c_str());
                }
                else if (numRead == 0)
                {
                    mStatus = Status::ValidContents;
                    mErrMsg.clear();
                    inLoop = false;
                }
                else
                {
                    mFileContents.insert(end(mFileContents), buf, buf + numRead);
                }
            }
            return;
        }
        catch (const CException& ex)
        {
            mStatus = static_cast<Status>(ex.mInternalErrorCode);
            mErrMsg = ex.mErrorDescription;
            // If it's not an error - don't log it - just return
            if (ex.mCode == CException::ErrorCode::NoError)
                return;
            functionName = ex.mFunctionName;
            lineNo = ex.mLineNo;
        }
        catch (const std::bad_alloc& /*ex*/)
        {
            mErrMsg = L"Memory allocation error.";
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        catch (const std::exception& ex)
        {
            std::wostringstream ss;
            ss << L"C++ exception caught. ";
            if (ex.what())
            {
                std::vector<wchar_t> sWhat(strlen(ex.what()) + 1, 0);
                size_t numConverted = 0;
                mbstowcs_s(&numConverted, &sWhat[0], sWhat.size() + 1, ex.what(), sWhat.size());
                assert(numConverted);
                ss.write(sWhat.data(), wcslen(sWhat.data()));
            }
            mErrMsg = ss.str();
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        catch (...)
        {
            mErrMsg = L"Unknown exception caught.";
            functionName = __FUNCTION__;
            lineNo = __LINE__;
        }
        LogError(functionName.c_str(), lineNo, mErrMsg);
    }

    CTextFileReader::Status CTextFileReader::CTextFileReaderImpl::GetStatus(std::wstring& o_sErrorMsg) const noexcept
    {
        o_sErrorMsg = mErrMsg;
        return mStatus;
    }

    bool CTextFileReader::CTextFileReaderImpl::GetContents(std::vector<unsigned char>& o_fileData) const noexcept
    {
        if (Status::ValidContents == mStatus)
        {
            o_fileData = mFileContents;
            return true;
        }
        return false;
    }

    void CLogger::CLoggerImpl::Log(const wchar_t* message)
    {
        if (message != nullptr)
            ::OutputDebugStringW(message);
    }
}