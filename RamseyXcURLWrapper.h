#ifndef RX_CURL_H
#define RX_CURL_H


#include <string>
#include <curl/curl.h>
#ifdef RX_QT
#include <QString>
#endif

const int RX_CURLWRAPPER_BUFFER_SIZE = 65536;

#define RX_SVR_API(s) ("www.ramseyx.org/" s)

class RamseyXcURLWrapper
{
public:
	RamseyXcURLWrapper();
	~RamseyXcURLWrapper();
    static void init();
	bool standardOpt(const char *szURL);
	bool setPost();
	void addPostField(const char *szKey, const char *szValue);
	void addPostField(const std::wstring &key, const std::wstring &val);
	bool execute();
	int getErrorCode() const;
	std::wstring getString() const;
	const char *getBuffer() const;

#ifdef RX_QT
    QString getQString() const;
#endif

    static int cURLWriter(char *pData, std::size_t size, std::size_t nmemb, char *pBuffer);

private:
    CURL *conn = nullptr;
    char rawBuffer[RX_CURLWRAPPER_BUFFER_SIZE] = {};
    std::string postFields;
    bool posting = false;
};

#endif
