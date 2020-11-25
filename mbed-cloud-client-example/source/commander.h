#ifndef __COMMANDER_H__
#define __COMMANDER_H__

#include <mqueue.h>

class SimpleM2MClient;

class Commander {

private:
  SimpleM2MClient *_client;
  mqd_t qd_cmd, qd_resp; // queue descriptors

public:
  Commander(SimpleM2MClient *client);

  void sendMsg(const char *cmd, const char *params, const char *data);
  void listen();
};

#endif /* __COMMANDER_H__ */