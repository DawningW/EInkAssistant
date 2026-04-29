#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <vector>
#include <map>
#include <Arduino.h>

class WiFiClient : public Stream {
public:
    std::vector<uint8_t>* buffer = nullptr;
    size_t position = 0;

    int available() override { return buffer ? buffer->size() - position : 0; }
    int read() override {
        if (buffer && position < buffer->size()) return (*buffer)[position++];
        return -1;
    }
    int peek() override {
        if (buffer && position < buffer->size()) return (*buffer)[position];
        return -1;
    }
    size_t write(uint8_t val) override { return 0; }
    virtual void flush() override {}
};

class WiFiClientSecure : public WiFiClient {
public:
    void setInsecure() {}
};

#define HTTP_CODE_OK 200

#define HTTPC_ERROR_CONNECTION_REFUSED (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED      (-4)
#define HTTPC_ERROR_CONNECTION_LOST    (-5)
#define HTTPC_ERROR_NO_STREAM          (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER     (-7)
#define HTTPC_ERROR_TOO_LESS_RAM       (-8)
#define HTTPC_ERROR_ENCODING           (-9)
#define HTTPC_ERROR_STREAM_WRITE       (-10)
#define HTTPC_ERROR_READ_TIMEOUT       (-11)

class HTTPClient {
public:
    HTTPClient();
    ~HTTPClient();

    bool begin(const String& url);
    bool begin(WiFiClientSecure& client, const String& url);

    void addHeader(const String& name, const String& value);
    void collectHeaders(const char* headerKeys[], const size_t headerKeysCount);
    String header(const String& name);

    int GET();
    int POST(const uint8_t * payload, size_t size);
    int POST(const String& payload);

    int getSize();
    Stream& getStream();
    String getString();

    void end();
    static String errorToString(int error);
    void setTimeout(uint16_t timeout);

private:
    void* _curl;
    struct curl_slist* _headers_list;
    String _url;
    std::vector<String> _collectHeadersList;
    std::map<String, String> _collectedHeaders;

    std::vector<uint8_t> _responseCache;
    int _responseSize;
    int _httpCode;
    uint16_t _timeout;

    WiFiClient* _clientStream = nullptr;
    WiFiClient _internalStream;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static size_t HeaderCallback(char *buffer, size_t size, size_t nitems, void *userdata);
};

#endif
