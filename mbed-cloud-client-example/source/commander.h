#ifndef __COMMANDER_H__
#define __COMMANDER_H__

#include <mqueue.h>

class SimpleM2MClient;
class Blinky;

class Commander {

private:
  SimpleM2MClient &_client;
  Blinky &_blinky;
  mqd_t qd_cmd, qd_resp; // mqueue descriptors

public:
  Commander(SimpleM2MClient &client, Blinky &blinky);

  void sendMsg(const char *cmd, const char *params, const char *data);
  void listen();
};

#endif /* __COMMANDER_H__ */