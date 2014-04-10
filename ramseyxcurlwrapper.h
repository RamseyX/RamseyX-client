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
#ifndef RAMSEYXCURLWRAPPER_H
#define RAMSEYXCURLWRAPPER_H


#include <string>
#include <curl/curl.h>
#ifdef RX_QT
#include <QString>
#endif

const int RX_CURLWRAPPER_BUFFER_SIZE = 65536;

#define RX_SVR_API(s) ("www.ramseyx.org/" s)

class RamseyXCurlWrapper
{
public:
    RamseyXCurlWrapper();
    ~RamseyXCurlWrapper();
    static void init();
    bool standardOpt(const std::string &url);
    bool setTimeOut(long maxSecs);
    void setPost();
    void addPostField(const std::string &key, const std::string &val);
    bool execute();
    int getErrorCode() const;
    std::string getString() const;
    const char *getBuffer() const;

#ifdef RX_QT
    QString getQString() const;
#endif

    static int CurlWriter(char *pData, std::size_t size, std::size_t nmemb, char *pBuffer);

private:
    CURL *conn = nullptr;
    char rawBuffer[RX_CURLWRAPPER_BUFFER_SIZE];
    std::string postFields;
    bool posting = false;
};

#endif // RAMSEYXCURLWRAPPER_H
