#ifndef RX_CURL_H
#define RX_CURL_H

#include <QString>
#include <string>
#include "curl/curl.h"

#define RX_CURLWRAPPER_BUFFER_SIZE 50000

#define RX_SVR_API(s) ("localhost/" s)

class RamseyXcURLWrapper
{
public:
	RamseyXcURLWrapper();
	~RamseyXcURLWrapper();
	bool standardOpt(const char *szURL);
	bool setPost();
	void addPostField(const char *szKey, const char *szValue);
	void addPostField(const std::wstring &key, const std::wstring &val);
	bool execute();
	int getErrorCode() const;
	std::wstring getString() const;
	const char *getBuffer() const;

#ifdef QT_VERSION
    QString getQString() const;
#endif

	static int cURLWriter(char *pData, size_t size, size_t nmemb, char *pBuffer);

private:
	static bool firstInstance;
	CURL *m_conn;
    char m_szBuffer[RX_CURLWRAPPER_BUFFER_SIZE];
    char m_szPostFields[RX_CURLWRAPPER_BUFFER_SIZE];
};

#endif
