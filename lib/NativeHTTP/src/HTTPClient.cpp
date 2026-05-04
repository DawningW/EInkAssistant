#include "HTTPClient.h"
#undef boolean
#undef INPUT
#include <curl/curl.h>

HTTPClient::HTTPClient() {
    _curl = curl_easy_init();
    _headers_list = nullptr;
    _timeout = 10000;
    _clientStream = &_internalStream;
    if (_clientStream) _clientStream->buffer = &_responseCache;
}

HTTPClient::~HTTPClient() {
    end();
}

bool HTTPClient::begin(const String& url) {
    if (!_curl) {
        _curl = curl_easy_init();
    }
    if (_headers_list) {
        curl_slist_free_all(_headers_list);
        _headers_list = nullptr;
    }
    _url = url;
    _clientStream = &_internalStream;
    if (_clientStream) _clientStream->buffer = &_responseCache;
    return true;
}

bool HTTPClient::begin(WiFiClientSecure& client, const String& url) {
    if (!_curl) {
        _curl = curl_easy_init();
    }
    if (_headers_list) {
        curl_slist_free_all(_headers_list);
        _headers_list = nullptr;
    }
    _url = url;
    _clientStream = &client;
    if (_clientStream) _clientStream->buffer = &_responseCache;
    return true;
}

void HTTPClient::addHeader(const String& name, const String& value) {
    String headerStr = name + ": " + value;
    _headers_list = curl_slist_append(_headers_list, headerStr.c_str());
}

void HTTPClient::collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {
    _collectHeadersList.clear();
    for (size_t i = 0; i < headerKeysCount; i++) {
        String key = headerKeys[i];
        key.toLowerCase();
        _collectHeadersList.push_back(key);
    }
}

String HTTPClient::header(const String& name) {
    String key = name;
    key.toLowerCase();
    if (_collectedHeaders.find(key) != _collectedHeaders.end()) {
        return _collectedHeaders[key];
    }
    return "";
}

size_t HTTPClient::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    HTTPClient *client = (HTTPClient *)userp;

    const uint8_t* ptr = (const uint8_t*)contents;
    client->_responseCache.insert(client->_responseCache.end(), ptr, ptr + realsize);

    return realsize;
}

size_t HTTPClient::HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata) {
    size_t realsize = size * nitems;
    HTTPClient *client = (HTTPClient *)userdata;

    String headerStr;
    for (size_t i = 0; i < realsize; i++) {
        headerStr += buffer[i];
    }

    int separatorIdx = headerStr.indexOf(':');
    if (separatorIdx != -1) {
        String key = headerStr.substring(0, separatorIdx);
        key.trim();
        key.toLowerCase();

        String value = headerStr.substring(separatorIdx + 1);
        value.trim();

        for (const String& collectKey : client->_collectHeadersList) {
            if (key == collectKey) {
                client->_collectedHeaders[key] = value;
                break;
            }
        }
    }
    return realsize;
}

int HTTPClient::GET() {
    if (!_curl) return -1;

    _responseCache.clear();
    _collectedHeaders.clear();
    if (_clientStream) {
        _clientStream->buffer = &_responseCache;
        _clientStream->position = 0;
    }

    curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
    curl_easy_setopt(_curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)this);
    curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(_curl, CURLOPT_HEADERDATA, (void *)this);

    if (_headers_list) {
        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers_list);
    }

    curl_easy_setopt(_curl, CURLOPT_TIMEOUT_MS, _timeout);

    // Ignore SSL verification for emulation purpose
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(_curl);

    if (res != CURLE_OK) {
        return -res;
    }

    long http_code = 0;
    curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    _httpCode = http_code;
    _responseSize = _responseCache.size();

    return _httpCode;
}

int HTTPClient::POST(const uint8_t * payload, size_t size) {
    if (!_curl) return -1;

    _responseCache.clear();
    _collectedHeaders.clear();
    if (_clientStream) {
        _clientStream->buffer = &_responseCache;
        _clientStream->position = 0;
    }

    curl_easy_setopt(_curl, CURLOPT_URL, _url.c_str());
    curl_easy_setopt(_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, (void*)payload);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, (long)size);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)this);
    curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(_curl, CURLOPT_HEADERDATA, (void *)this);

    if (_headers_list) {
        curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers_list);
    }

    curl_easy_setopt(_curl, CURLOPT_TIMEOUT_MS, _timeout);

    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);

    CURLcode res = curl_easy_perform(_curl);

    if (res != CURLE_OK) {
        return -res;
    }

    long http_code = 0;
    curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &http_code);
    _httpCode = http_code;
    _responseSize = _responseCache.size();

    return _httpCode;
}

int HTTPClient::POST(const String& payload) {
    return POST((const uint8_t*)payload.c_str(), payload.length());
}

int HTTPClient::getSize() {
    return _responseSize;
}

Stream& HTTPClient::getStream() {
    return *_clientStream;
}

String HTTPClient::getString() {
    String ret;
    ret.reserve(_responseCache.size());
    for (size_t i = 0; i < _responseCache.size(); i++) {
        ret += (char)_responseCache[i];
    }
    return ret;
}

void HTTPClient::end() {
    if (_headers_list) {
        curl_slist_free_all(_headers_list);
        _headers_list = nullptr;
    }
    if (_curl) {
        curl_easy_cleanup(_curl);
        _curl = nullptr;
    }
}

String HTTPClient::errorToString(int error) {
    if (error < 0) {
        return String(curl_easy_strerror((CURLcode)(-error)));
    }
    return String("Unknown error");
}

void HTTPClient::setTimeout(uint16_t timeout) {
    _timeout = timeout;
}
