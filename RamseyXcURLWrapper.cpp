#include "RamseyXcURLWrapper.h"
#include "RamseyXUtils.h"

bool RamseyXcURLWrapper::firstInstance = true;

RamseyXcURLWrapper::RamseyXcURLWrapper()
{
	if (firstInstance)
	{
		firstInstance = false;
		::curl_global_init(CURL_GLOBAL_DEFAULT);
	}
	m_conn = ::curl_easy_init();
}

RamseyXcURLWrapper::~RamseyXcURLWrapper()
{
	if (m_conn)
		::curl_easy_cleanup(m_conn);
}

bool RamseyXcURLWrapper::standardOpt(const char *szURL)
{
	if (!szURL)
		return false;
	
	if (::curl_easy_setopt(m_conn, CURLOPT_URL, szURL) != CURLE_OK ||
		::curl_easy_setopt(m_conn, CURLOPT_FOLLOWLOCATION, 1) != CURLE_OK ||
		::curl_easy_setopt(m_conn, CURLOPT_WRITEDATA, m_szBuffer) != CURLE_OK ||
		::curl_easy_setopt(m_conn, CURLOPT_WRITEFUNCTION, cURLWriter) != CURLE_OK)
		return false;

	return true;
}

int RamseyXcURLWrapper::cURLWriter(char *pData, size_t size, size_t nmemb, char *pBuffer)
{
	if (!pBuffer)
		return 0;
	std::strcat(pBuffer, pData);
	
	return size * nmemb;
}

int RamseyXcURLWrapper::getErrorCode() const
{
	return std::strtol(m_szBuffer, nullptr, 10);
}

bool RamseyXcURLWrapper::execute()
{
	std::memset(m_szBuffer, 0, sizeof (m_szBuffer));
	if (::curl_easy_perform(m_conn) != CURLE_OK)
		return false;
    std::strcpy(m_szBuffer, m_szBuffer);
	return true;
}

std::wstring RamseyXcURLWrapper::getString() const
{
    std::string s(RamseyXUtils::UTF8ToUnicode(m_szBuffer).get());
	std::wstring ws(RamseyXUtils::to_wstring(s));

	return ws.substr(ws.find(L' ') + 1);
}

const char *RamseyXcURLWrapper::getBuffer() const
{
	return m_szBuffer;
}

bool RamseyXcURLWrapper::setPost()
{
	std::memset(m_szPostFields, 0, sizeof (m_szPostFields));
	return ::curl_easy_setopt(m_conn, CURLOPT_POST, 1) == CURLE_OK && ::curl_easy_setopt(m_conn, CURLOPT_POSTFIELDS, m_szPostFields) == CURLE_OK;
}

void RamseyXcURLWrapper::addPostField(const char *szKey, const char *szValue)
{
	if (std::strlen(m_szPostFields) > 0)
		std::strcat(m_szPostFields, "&");
	std::strcat(m_szPostFields, szKey);
	std::strcat(m_szPostFields, "=");
	std::strcat(m_szPostFields, szValue);
}

void RamseyXcURLWrapper::addPostField(const std::wstring &key, const std::wstring &val)
{
	if (std::strlen(m_szPostFields) > 0)
		std::strcat(m_szPostFields, "&");

    std::strcat(m_szPostFields, RamseyXUtils::to_string(key).c_str());
	std::strcat(m_szPostFields, "=");
	std::strcat(m_szPostFields, RamseyXUtils::to_string(val).c_str());
}

#ifdef QT_VERSION
QString RamseyXcURLWrapper::getQString() const
{
    QString result(m_szBuffer);

    return result.mid(result.indexOf(' ') + 1);
}
#endif
