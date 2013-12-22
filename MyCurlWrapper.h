#pragma once


class MyCurlWrapper
{
public:
	MyCurlWrapper();
	~MyCurlWrapper();
	bool StandardOpt(const char *szURL);
	bool SetPost();
	void AddPostField(const char *szKey, const char *szValue);
	bool Execute();
	int GetErrorCode();
	CString GetString();
	char *GetBuffer();

	static int CurlWriter(char *pData, size_t size, size_t nmemb, char *pBuffer);

private:
	CURL *m_conn;
	char m_szBuffer[55000];
	char m_szPostFields[5000];
};
