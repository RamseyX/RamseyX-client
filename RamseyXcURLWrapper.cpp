#include "RamseyXcURLWrapper.h"
#include "RamseyXUtils.h"

RamseyXcURLWrapper::RamseyXcURLWrapper() :
    conn(curl_easy_init())
{
}

RamseyXcURLWrapper::~RamseyXcURLWrapper()
{
    if (conn)
        curl_easy_cleanup(conn);
}

void RamseyXcURLWrapper::init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

bool RamseyXcURLWrapper::standardOpt(const char *szURL)
{
	if (!szURL)
		return false;
    posting = false;

    return curl_easy_setopt(conn, CURLOPT_URL, szURL) == CURLE_OK &&
            curl_easy_setopt(conn, CURLOPT_FOLLOWLOCATION, 1) == CURLE_OK &&
            curl_easy_setopt(conn, CURLOPT_WRITEDATA, rawBuffer) == CURLE_OK &&
            curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, cURLWriter) == CURLE_OK;
}

int RamseyXcURLWrapper::cURLWriter(char *pData, std::size_t size, std::size_t nmemb, char *pBuffer)
{
	if (!pBuffer)
		return 0;
	std::strcat(pBuffer, pData);
	
	return size * nmemb;
}

int RamseyXcURLWrapper::getErrorCode() const
{
    return std::strtol(rawBuffer, nullptr, 10);
}

bool RamseyXcURLWrapper::execute()
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

std::wstring RamseyXcURLWrapper::getString() const
{
    std::string s(RamseyXUtils::UTF8ToUnicode(rawBuffer).get());
	std::wstring ws(RamseyXUtils::to_wstring(s));

	return ws.substr(ws.find(L' ') + 1);
}

const char *RamseyXcURLWrapper::getBuffer() const
{
    return rawBuffer;
}

bool RamseyXcURLWrapper::setPost()
{
    posting = true;
    postFields.clear();
    return true;
    //return curl_easy_setopt(conn, CURLOPT_POST, 1) == CURLE_OK && curl_easy_setopt(conn, CURLOPT_POSTFIELDS, postFields) == CURLE_OK;
}

void RamseyXcURLWrapper::addPostField(const char *szKey, const char *szValue)
{
    if (!postFields.empty())
        postFields += '&';
    postFields += szKey;
    postFields += '=';
    postFields += szValue;
}

void RamseyXcURLWrapper::addPostField(const std::wstring &key, const std::wstring &val)
{
    if (!postFields.empty())
        postFields += '&';
    postFields += RamseyXUtils::to_string(key);
    postFields += '=';
    postFields += RamseyXUtils::to_string(val);
}

#ifdef RX_QT
QString RamseyXcURLWrapper::getQString() const
{
    QString result(rawBuffer);

    return result.section(' ', 1);
}
#endif
