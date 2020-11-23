#ifndef __COMMANDER_H__
#define __COMMANDER_H__

class SimpleM2MClient;

class Commander {

private:
  SimpleM2MClient *_client;

  void listen();

public:
  Commander();

  ~Commander();

  void init(SimpleM2MClient &client);
};
#endif /* __COMMANDER_H__ */