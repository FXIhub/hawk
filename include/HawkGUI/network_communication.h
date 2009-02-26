#ifdef NETWORK_SUPPORT
#ifndef _NETWORK_COMMUNICATION_H_
#define _NETWORK_COMMUNICATION_H_ 1


#ifdef __cplusplus
extern "C"{
#endif
  void * attempt_connection(char * server, int server_port);
  void wait_for_server_instructions(void * _socket);
  void init_qt(int argc, char ** argv);
  void cleanup_qt();
#ifdef __cplusplus
} /* extern "C" */
#endif



#endif /* _NETWORK_COMMUNICATION_H_ */
#endif /* NETWORK_SUPPORT */
