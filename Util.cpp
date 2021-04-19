// Contains implementations of OS-independent classes, functions.
#include "Util.h"
#ifdef _WIN32
#include "win/WinUtil.h"
#else
// todo: add support for other oses: Mac, Linux, etc.
#endif
#include <assert.h>
#include <sstream>
#include <functional>
#include <algorithm>

namespace OTInterviewExercise1
{
    COsInitialization::COsInitialization() noexcept :
        mImpl(std::make_unique<COsInitializationImpl>())
    {}
    COsInitialization::~COsInitialization()
    {}
    bool COsInitialization::IsOk(std::wstring& o_errorMsg) const noexcept
    {
        return mImpl->IsOk(o_errorMsg);
    }

    CTextFileReader::CTextFileReader(const wchar_t* filePathName) noexcept
    {
        mImpl = std::make_unique<CTextFileReaderImpl>(filePathName);
    }

    CTextFileReader::~CTextFileReader() noexcept
    {}

    bool CTextFileReader::Exists(std::wstring& o_sErrorMsg) const noexcept
    {
        Status status = mImpl->GetStatus(o_sErrorMsg);
        switch (status)
        {
        case Status::NotFound:
        case Status::NoContents:
        case Status::FindError:
            return false;
        }
        return true;
    }

    bool CTextFileReader::GetContents(std::wstring& o_fileData, std::wstring& o_sErrorMsg) const noexcept
    {
        std::string functionName;
        unsigned int lineNo = 0;
        try
        {
            o_fileData.clear();
            o_sErrorMsg.clear();
            std::vector<BYTE> contents;
            if (mImpl->GetContents(contents) && !contents.empty())
            {
                std::vector<wchar_t> buf(contents.size() + 1, 0);
                size_t numConverted = 0;

                mbstowcs_s(&numConverted, buf.data(), buf.size(),
                    reinterpret_cast<char*>(contents.data()), contents.size());
                assert(numConverted);
                if (numConverted != contents.size() + 1)
                {
                    THROW_ERROR(L"Failed to convert UTF8 string to wchar_t");
                }
                o_fileData.assign(buf.data(), wcslen(buf.data()));
                return true;
            }
            else
            {
                mImpl->GetStatus(o_sErrorMsg);
            }
            return false;
        }
        catch (const CException& ex)
        {
            std::wostringstream ss;
            ss << L"Exception caught. ";
            if (!ex.mErrorDescription.empty())
            {
                ss << L"System error: " << ex.mErrorDescription;
            }
            o_sErrorMsg = ss.str();
            o_fileData.clear();
            functionName = ex.mFunctionName;
            lineNo = ex.mLineNo;
        }
        catch (const std::bad_alloc& /*ex*/)
        {
            functionName = __FUNCTION__;
            lineNo = __LINE__;
            o_sErrorMsg = L"Memory allocation error";
            o_fileData.clear();
        }
        LogError(functionName.c_str(), lineNo, o_sErrorMsg);
        return false;
    }

    CLogger::CLogger(CLogger::LogLevel logLevel) :
        mCurrentLogLevel(logLevel),
        mImpl(std::make_unique<CLogger::CLoggerImpl>())
    {}

    CLogger::~CLogger()
    {}

    void CLogger::SetLogLevel(CLogger::LogLevel logLevel)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mCurrentLogLevel = logLevel;
    }
    
    CLogger::LogLevel CLogger::GetLogLevel() const
    {
        std::lock_guard<std::mutex> lock(mMutex);
        return mCurrentLogLevel;
    }

    void CLogger::Log(LogLevel logLevel, const char* functionName, unsigned int lineNo, const std::wstring& sError)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        if (mCurrentLogLevel < logLevel)
            return;
        lock.unlock();
        std::wstring functionNameWideStr;
        if (functionName != nullptr)
        {
            std::transform(functionName, functionName + strlen(functionName),
                std::back_inserter(functionNameWideStr),
                [](char c) {return static_cast<wchar_t>(c); });
        }
        std::wostringstream oss;
        switch (logLevel)
        {
        case LogLevel::Critical:
            oss << L"Critical. ";
            break;
        case LogLevel::Error:
            oss << L"Error. ";
            break;
        case LogLevel::Warning:
            oss << L"Warn. ";
            break;
        case LogLevel::Info:
            oss << L"Info. ";
            break;
        case LogLevel::All:
            oss << L"All. ";
            break;
        };

        if (!functionNameWideStr.empty())
            oss << L"Func: " << functionNameWideStr << L", ";
        if (lineNo != 0)
            oss << L"Lineno: " << lineNo << " ";
        if (!sError.empty())
            oss << " Msg: " << sError;

        mImpl->Log(oss.str().c_str());
    }

    static CLogger DefaultLogger;

    void LogCritical(const char* functionName, unsigned int lineNo, const std::wstring& sError)
    {
        DefaultLogger.Log(CLogger::LogLevel::Critical, functionName, lineNo, sError);
    }

    void LogError(const char* functionName, unsigned int lineNo, const std::wstring& sError)
    {
        DefaultLogger.Log(CLogger::LogLevel::Error, functionName, lineNo, sError);
    }

    void LogWarn(const char* functionName, unsigned int lineNo, const std::wstring& sError)
    {
        DefaultLogger.Log(CLogger::LogLevel::Warning, functionName, lineNo, sError);
    }

    void LogInfo(const char* functionName, unsigned int lineNo, const std::wstring& sError)
    {
        DefaultLogger.Log(CLogger::LogLevel::Info, functionName, lineNo, sError);
    }

    void LogAll(const char* functionName, unsigned int lineNo, const std::wstring& sError)
    {
        DefaultLogger.Log(CLogger::LogLevel::All, functionName, lineNo, sError);
    }
};


