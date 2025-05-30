#include <not_implemented.h>
#include <httplib.h>
#include "../include/server_logger.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

server_logger::~server_logger() noexcept
{
    std::string pid = std::to_string(inner_getpid());
    auto res = _client->Get("/destroy?pid=" + pid);
}

logger &server_logger::log(
        const std::string &text,
        logger::severity severity) &
{
    std::string pid = std::to_string(inner_getpid());

    // Сначала применяем make_format
    std::string formatted_message = make_format(text, severity);

    // Затем кодируем для URL только formatted_message
    std::string encoded_message;
    for (char c : formatted_message) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == ' ') {
            encoded_message += c;
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", static_cast<unsigned char>(c));
            encoded_message += buf;
        }
    }

    // Формируем URL с закодированным сообщением
    std::string url = "/log?pid=" + pid +
                      "&sev=" + severity_to_string(severity) +
                      "&message=" + encoded_message;
    auto res = _client->Get(url);

    return *this;
}

server_logger::flag server_logger::char_to_flag(char c) const noexcept
{
    switch (c)
    {
        case 'd': return flag::DATE;
        case 't': return flag::TIME;
        case 's': return flag::SEVERITY;
        case 'm': return flag::MESSAGE;
        default: return flag::NO_FLAG;
    }
}

std::string server_logger::make_format(const std::string &message, severity sev) const
{
    std::stringstream formatted_message;

    for (size_t i = 0; i < _format.length(); ++i)
    {
        if (_format[i] == '%' && i + 1 < _format.length())
        {
            switch (char_to_flag(_format[i + 1]))
            {
                case flag::DATE:
                    formatted_message << current_date_to_string();
                    break;
                case flag::TIME:
                    formatted_message << current_time_to_string();
                    break;
                case flag::SEVERITY:
                    formatted_message << severity_to_string(sev);
                    break;
                case flag::MESSAGE:
                    formatted_message << message;
                    break;
                default:
                    formatted_message << '%' << _format[i + 1];
            }
            ++i;
        }
        else
        {
            formatted_message << _format[i];
        }
    }
    return formatted_message.str();
}

server_logger::server_logger(const std::string &dest,
                             const std::unordered_map<logger::severity, std::pair<std::string, bool> > &
                             streams, const std::string &format) :  _client(std::make_unique<httplib::Client>(dest)), _streams(streams), _format(format)
{
    std::string pid = std::to_string(inner_getpid());
    for (const auto &[sev, stream_info]: streams)
    {
        std::string url = "/init?pid=" + pid + "&sev=" + severity_to_string(sev) + "&path="
                          + stream_info.first + "&console=" + std::to_string(+stream_info.second);
        auto res = _client->Get(url);
    }
}

int server_logger::inner_getpid()
{
#ifdef _WIN32
    return ::_getpid();
#else
    return getpid();
#endif
}

server_logger::server_logger(server_logger &&other) noexcept : _client(std::move(other._client)),
                                                               _format(std::move(other._format)) {}

server_logger &server_logger::operator=(server_logger &&other) noexcept
{
    if (&other != this)
    {
        _client = std::move(other._client);
        _format = std::move(other._format);
    }
    return *this;
}