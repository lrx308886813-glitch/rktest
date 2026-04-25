#pragma once

#include "core/Config.hpp"
#include "core/StateMachine.hpp"
#include "core/Storage.hpp"
#include "services/AudioService.hpp"
#include "services/CallService.hpp"
#include "services/CameraService.hpp"
#include "services/ContactService.hpp"
#include "services/DeviceService.hpp"
#include "services/MessageService.hpp"
#include "services/ReminderService.hpp"
#include "web/HttpServer.hpp"

#include <memory>

namespace ect {

class App {
public:
    App();
    ~App();

    bool initialize();
    bool start();
    void stop();
    int run();

private:
    HttpResponse route(const HttpRequest& request);
    std::string webRoot() const;
    std::string dataDir() const;

    Config config_;
    StateMachine stateMachine_;
    std::unique_ptr<Storage> storage_;
    std::shared_ptr<IAudio> audioHardware_;
    std::shared_ptr<ICamera> cameraHardware_;
    std::unique_ptr<AudioService> audioService_;
    std::unique_ptr<CameraService> cameraService_;
    std::unique_ptr<ContactService> contactService_;
    std::unique_ptr<MessageService> messageService_;
    std::unique_ptr<ReminderService> reminderService_;
    std::unique_ptr<CallService> callService_;
    std::unique_ptr<DeviceService> deviceService_;
    std::unique_ptr<HttpServer> httpServer_;
};

} // namespace ect
