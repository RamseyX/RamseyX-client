/***************************************************************************
 *
 * RamseyX Client: client program of distributed computing project RamseyX
 *
 * Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>, et al.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/
#include "ramseyxcurlwrapper.h"
#include "ramseyxutils.h"

RamseyXCurlWrapper::RamseyXCurlWrapper() :
    conn(curl_easy_init())
{
    std::memset(rawBuffer, 0, sizeof (rawBuffer));
}

RamseyXCurlWrapper::~RamseyXCurlWrapper()
{
    if (conn)
        curl_easy_cleanup(conn);
}

void RamseyXCurlWrapper::init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

bool RamseyXCurlWrapper::standardOpt(const std::string &url)
{
    posting = false;

    return curl_easy_setopt(conn, CURLOPT_URL, url.c_str()) == CURLE_OK &&
            curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1) == CURLE_OK &&
            curl_easy_setopt(conn, CURLOPT_WRITEDATA, rawBuffer) == CURLE_OK &&
            curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, CurlWriter) == CURLE_OK;
}

int RamseyXCurlWrapper::CurlWriter(char *pData, std::size_t size, std::size_t nmemb, char *pBuffer)
{
    if (!pBuffer)
        return 0;
    std::strcat(pBuffer, pData);

    return size * nmemb;
}

int RamseyXCurlWrapper::getErrorCode() const
{
    return std::strtol(rawBuffer, nullptr, 10);
}

bool RamseyXCurlWrapper::execute()
{
    if (posting &&
            !(curl_easy_setopt(conn, CURLOPT_POST, 1) == CURLE_OK &&
              curl_easy_setopt(conn, CURLOPT_POSTFIELDS, postFields.c_str()) == CURLE_OK))
            return false;
    std::memset(rawBuffer, 0, sizeof (rawBuffer));
    if (curl_easy_perform(conn) != CURLE_OK)
        return false;
    return true;
}

std::string RamseyXCurlWrapper::getString() const
{
    std::string s(RamseyXUtils::Utf8ToUnicode(rawBuffer).get());

    return s.substr(s.find(' ') + 1);
}

const char *RamseyXCurlWrapper::getBuffer() const
{
    return rawBuffer;
}

bool RamseyXCurlWrapper::setTimeOut(long maxSecs)
{
    return curl_easy_setopt(conn, CURLOPT_TIMEOUT, maxSecs) == CURLE_OK;
}

void RamseyXCurlWrapper::setPost()
{
    posting = true;
    postFields.clear();
}

void RamseyXCurlWrapper::addPostField(const std::string &key, const std::string &val)
{
    if (!postFields.empty())
        postFields += '&';
    postFields += key;
    postFields += '=';
    postFields += val;
}

#ifdef RX_QT
QString RamseyXCurlWrapper::getQString() const
{
    QString result(rawBuffer);

    return result.section(' ', 1);
}
#endif
