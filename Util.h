// Contains declarations of OS-independent classes, functions.

#ifndef OT_UTIL_H__
#define OT_UTIL_H__

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <assert.h>

namespace OTInterviewExercise1
{
    // Our exception object
    struct CException
    {
        enum class ErrorCode
        {
            NoError,
            Error
        };
        CException(const char* functionName = nullptr,
            unsigned int lineNo = 0,
            ErrorCode code = ErrorCode::NoError,
            unsigned int internalErrorCode = 0,
            const wchar_t* errorDescription = nullptr) :
            mFunctionName(functionName ? functionName : ""),
            mLineNo(lineNo),
            mCode(code),
            mInternalErrorCode(internalErrorCode),
            mErrorDescription(errorDescription ? errorDescription : L"")
        {}
        // Can be Error or NoError. The 1st one is used for errors, the 2nd one
        // is used when we need to throw an exception that isn't an error.
        ErrorCode mCode;
        // Placeholder for custom data.
        unsigned int mInternalErrorCode;
        // Line number where exception was thrown
        unsigned int mLineNo;
        // Name of function where exception was thrown
        std::string mFunctionName;
        // Error message string
        std::wstring mErrorDescription;
    };

    // Performs OS-specific (un)initialization
    class COsInitialization
    {
    public:
        COsInitialization() noexcept;
        ~COsInitialization();
        bool IsOk(std::wstring& o_errMsg) const noexcept;

        COsInitialization(const COsInitialization&) = delete;
        COsInitialization(COsInitialization&&) = delete;
        COsInitialization& operator=(const COsInitialization&) = delete;
        COsInitialization& operator=(COsInitialization&&) = delete;
    private:
        class COsInitializationImpl;
        std::unique_ptr<COsInitializationImpl> mImpl;
    };

    // Retrieve contents of text file (file can be ASCII or UTF8)
    // Returned data is wchar_t string
    class CTextFileReader
    {
    public:
        CTextFileReader(const wchar_t* filePathName) noexcept;
        ~CTextFileReader() noexcept;
        // Returns boolean to indicate if file was found.
        // o_sErrorMsg contains error message if false was returned.
        bool Exists(std::wstring& o_sErrorMsg) const noexcept;
        // Returns boolean to indicate success or failure.
        // o_fileData contains file contents converted to wchar_t string.
        // o_sErrorMsg contains error message if false was returned.
        bool GetContents(std::wstring& o_fileData, std::wstring& o_sErrorMsg) const noexcept;
    private:
        enum class Status
        {
            NotFound,
            Found,
            FindError,
            ValidContents,
            NoContents,
            ReadContentsError
        };
        class CTextFileReaderImpl;
        std::unique_ptr<CTextFileReaderImpl> mImpl;
    };

    // Takes a closure (e.g. lambda) as parameter. That closure is executed
    // when object goes out of scope. Mostly useful for cleanup of resources
    // that aren't smart pointers.
    template<typename T> class CRAIICleanup
    {
    public:
        CRAIICleanup(T deferredAction) :
            mDeferredAction(deferredAction)
        {}
        ~CRAIICleanup()
        {
            try
            {
                mDeferredAction();
            }
            catch (...)
            {
                assert(false);
            }
        }
    private:
        T mDeferredAction;
    };

    // Create CRAIICleanup object with a closure that's passed as param.
    template<typename T> CRAIICleanup<T> MakeRAIICleanup(T closure)
    {
        return CRAIICleanup<T>(closure);
    }

    // Logs error/warning/etc. messages (current implementation logs to debug console only).
    class CLogger
    {
    public:
        enum class LogLevel
        {
            Critical,
            Error,
            Warning,
            Info,
            All
        };
        CLogger(LogLevel logLevel = LogLevel::Error);
        ~CLogger();

        CLogger(const CLogger&) = delete;
        CLogger& operator=(const CLogger&) = delete;
        CLogger(CLogger&&) = delete;
        CLogger& operator=(CLogger&&) = delete;

        void SetLogLevel(LogLevel logLevel);
        LogLevel GetLogLevel() const;
        void Log(LogLevel logLevel, const char* functionName, unsigned int lineNo, const std::wstring& sMessage);
    private:
        LogLevel mCurrentLogLevel;
        mutable std::mutex mMutex;
        class CLoggerImpl;
        std::unique_ptr<CLoggerImpl> mImpl;
    };
    // Log critical error message using default logger
    void LogCritical(const char* functionName, unsigned int lineNo, const std::wstring& sError);
    // Log error message using default logger
    void LogError(const char* functionName, unsigned int lineNo, const std::wstring& sError);
    // Log warning message using default logger
    void LogWarn(const char* functionName, unsigned int lineNo, const std::wstring& sError);
    // Log info message using default logger
    void LogInfo(const char* functionName, unsigned int lineNo, const std::wstring& sError);
    // Log all message using default logger
    void LogAll(const char* functionName, unsigned int lineNo, const std::wstring& sError);
}

// Used by other macros. Shouldn't be called directly by client code.
#define THROW_INTERNAL(type,code,descr)		\
    throw OTInterviewExercise1::CException( \
        __FUNCTION__,                       \
        __LINE__,                           \
        type,                               \
        code,								\
        descr);

// Throw error exception using custom code
#define THROW_ERROR_CODE(code,descr) THROW_INTERNAL(OTInterviewExercise1::CException::ErrorCode::Error,code,descr)
// Throw non-error exception using custom code
#define THROW_NORMAL_CODE(code,descr) THROW_INTERNAL(OTInterviewExercise1::CException::ErrorCode::NoError,code,descr)
// Throw error exception
#define THROW_ERROR(descr) THROW_ERROR_CODE(0,descr)
// Throw non-error exception
#define THROW_NORMAL(descr) THROW_NORMAL_CODE(0,descr)

#endif // OT_UTIL_H__
