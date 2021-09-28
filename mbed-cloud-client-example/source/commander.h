#ifndef __COMMANDER_H__
#define __COMMANDER_H__

#include <mqueue.h>

class MbedCloudClient;
class Blinky;

class Commander {

private:
  MbedCloudClient &_client;
  Blinky &_blinky;
  mqd_t qd_cmd, qd_resp, qd_fota; // mqueue descriptors

public:
  Commander(MbedCloudClient &client, Blinky &blinky);

  void sendMsg(const char *cmd, const char *params, const char *data);
  void listen();
};

#endif /* __COMMANDER_H__ */