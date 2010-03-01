#ifndef _NETWORK_COMMUNICATION_H_
#define _NETWORK_COMMUNICATION_H_ 1

#include <io_utils.h>

#ifdef __cplusplus
extern "C"{
#endif

  typedef struct RPCInfo RPCInfo;

#ifdef NETWORK_SUPPORT

  void attempt_connection(char * server, int server_port,int key);
  int start_event_loop();
  void init_qt(int argc, char ** argv);
  void cleanup_and_free_qt();
  void rpc_send_message(MessageType type, const char * s);
  void rpc_send_log_line(const char * s);
  void rpc_send_image_output(const char * s,const Image * a);
  int is_connected();
#else
#define is_connected() 0
#endif /* NETWORK_SUPPORT */


#ifdef __cplusplus
}
#endif

#endif /* _NETWORK_COMMUNICATION_H_ */
