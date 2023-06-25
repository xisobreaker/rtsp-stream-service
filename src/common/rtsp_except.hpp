#pragma once

#include <exception>
#include <string>

class RTSPException : public std::exception
{
public:
    RTSPException(std::string &&message)
    {
        m_errorMsg = message;
    };

public:
    virtual char const *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT
    {
        return m_errorMsg.c_str();
    }

private:
    std::string m_errorMsg;
};