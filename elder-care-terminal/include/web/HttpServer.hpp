#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <string>
#include <thread>

namespace ect {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    std::map<std::string, std::string> headers;
};

struct HttpResponse {
    int status{200};
    std::string contentType{"application/json; charset=utf-8"};
    std::string body;
};

class HttpServer {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    HttpServer(int port, std::string webRoot);
    ~HttpServer();

    void setHandler(Handler handler);
    bool start();
    void stop();
    bool isRunning() const { return running_; }
    int port() const { return port_; }

    static std::map<std::string, std::string> parseQuery(const std::string& query);

private:
    void loop();
    void handleClient(int clientSocket);
    HttpResponse serveStatic(const HttpRequest& request);

    int port_;
    std::string webRoot_;
    Handler handler_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    int serverSocket_{-1};
};

std::string urlDecode(const std::string& value);
std::string extractJsonString(const std::string& body, const std::string& key);
bool extractJsonBool(const std::string& body, const std::string& key, bool defaultValue);

} // namespace ect
