#ifndef __SS_HOOK_H__
#define __SS_HOOK_H__


#ifdef SKYPE_HOOK_EXPORTS
#define SKYPE_HOOK_API __declspec(dllexport)
#else
#define SKYPE_HOOK_API __declspec(dllimport)
#endif




///////////////////// SkypeHook API, defined by fanghui

SKYPE_HOOK_API bool ss_init(); // initializationfor safe skype hook
SKYPE_HOOK_API bool ss_start();  // start the hook
SKYPE_HOOK_API void ss_get_token(unsigned char * outkey); // get current password for this hook
SKYPE_HOOK_API void ss_set_token( unsigned char * pwd, int len ); // set current token for this hook
SKYPE_HOOK_API void ss_stop(); // stop the hook
SKYPE_HOOK_API void ss_close(); // clear and exit hook

SKYPE_HOOK_API void ss_get_local_user(char *name);
SKYPE_HOOK_API void ss_get_remote_user(char *name);

///////////////////// SkypeHook API, end


#endif