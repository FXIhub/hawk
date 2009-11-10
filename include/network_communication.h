#ifdef NETWORK_SUPPORT
#ifndef _NETWORK_COMMUNICATION_H_
#define _NETWORK_COMMUNICATION_H_ 1


#ifdef __cplusplus
extern "C"{
#endif
  typedef struct RPCInfo RPCInfo;

  RPCInfo * attempt_connection(char * server, int server_port);
  int start_event_loop();
  void init_qt(int argc, char ** argv);
  void cleanup_and_free_qt();
  void setup_signals_and_slots(RPCInfo * rpcInfo);
#ifdef __cplusplus
}
#endif



#endif /* _NETWORK_COMMUNICATION_H_ */
#endif /* NETWORK_SUPPORT */
