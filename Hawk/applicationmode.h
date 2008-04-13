#ifndef _APPLICATIONMODE_H_
#define _APPLICATIONMODE_H_ 1
#if defined __cplusplus || defined Q_MOC_RUN

typedef enum{ModeDefault=0,ModeIncludeInMask,ModeExcludeFromMask,ModePickCenter} ApplicationMode;


#else
#error "Someone is including " __FILE__ " from a C file!"
#endif
#endif
