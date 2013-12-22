#include "stdafx.h"
#include "MyCurlWrapper.h"

MyCurlWrapper::MyCurlWrapper()
{
	m_conn = ::curl_easy_init();
}

MyCurlWrapper::~MyCurlWrapper()
{
	if (m_conn)
		::curl_easy_cleanup(m_conn);
}

bool MyCurlWrapper::StandardOpt(const char *szURL)
{
	if (!szURL)
		return false;
	
	if (::curl_easy_setopt(m_conn, CURLOPT_URL, szURL) != CURLE_OK ||
		::curl_easy_setopt(m_conn, CURLOPT_FOLLOWLOCATION, 1) != CURLE_OK ||
		::curl_easy_setopt(m_conn, CURLOPT_WRITEDATA, m_szBuffer) != CURLE_OK ||
		::curl_easy_setopt(m_conn, CURLOPT_WRITEFUNCTION, CurlWriter) != CURLE_OK)
		return false;

	return true;
}

int MyCurlWrapper::CurlWriter(char *pData, size_t size, size_t nmemb, char *pBuffer)
{
	if (!pBuffer)
		return 0;
	strcat_s(pBuffer, 55000, pData);
	
	return size * nmemb;
}

int MyCurlWrapper::GetErrorCode()
{
	int nErrorCode = -1;
	sscanf_s(m_szBuffer, "%d", &nErrorCode);
	
	return nErrorCode;
}

bool MyCurlWrapper::Execute()
{
	memset(m_szBuffer, 0, sizeof (m_szBuffer));
	if (::curl_easy_perform(m_conn) != CURLE_OK)
		return false;
	strcpy_s(m_szBuffer, sizeof (m_szBuffer), UTF8ToUnicode(m_szBuffer));
	return true;
}

CString MyCurlWrapper::GetString()
{
	CString str(m_szBuffer);
	return str.Right(str.GetLength() - str.Find(_T(' '), 0) - 1);
}

char *MyCurlWrapper::GetBuffer()
{
	return m_szBuffer;
}

bool MyCurlWrapper::SetPost()
{
	memset(m_szPostFields, 0, sizeof (m_szPostFields));
	return ::curl_easy_setopt(m_conn, CURLOPT_POST, 1) == CURLE_OK && ::curl_easy_setopt(m_conn, CURLOPT_POSTFIELDS, m_szPostFields) == CURLE_OK;
}

void MyCurlWrapper::AddPostField(const char *szKey, const char *szValue)
{
	if (strlen(m_szPostFields) > 0)
		strcat_s(m_szPostFields, sizeof (m_szPostFields), "&");
	strcat_s(m_szPostFields, sizeof (m_szPostFields), szKey);
	strcat_s(m_szPostFields, sizeof (m_szPostFields), "=");
	strcat_s(m_szPostFields, sizeof (m_szPostFields), szValue);
}
