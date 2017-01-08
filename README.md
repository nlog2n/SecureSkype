# SecureSkype
End-to-end encryption plugin for Skype communications

SecureSkype is an end-to-end encryption plugin to protect Skype communications. Currently it only supports encryption on texting and attached files.

## v2.0
New release fixed the following issues:
-	corrected token setting for local-remote user pair. When started, local user must either set a new token or use default one.
-	added detection for current conversation. Plugin will not start if no active conversation found.


Quick start:
1.	Start Skype program and get logged in;
2.	Choose a conversation window in Skype
3.	Start SafeSkype program, and configure tokens for friends by clicking “Setting” button;
4.	In setting window, type remote user display name and related token. 
5.	Click “OK” and return to SafeSkype window. 
6.	Click “Start” button to enable protection. Or again click “Stop” to disable encryption.


Important notes: 
-	If switching to another conversation in Skype, one must stop the existing plugin and restart.
-	If “Start” button is clicked directly without pre-setting, SafeSkype will use default token ( and save it ) 


## v1.2
merged the key hook for Skype chat messages


## v1.1
The attached is the updated security plug-in v1.1 for Skype:
-	Added token settings for friends
-	Fixed the bug about secure file transfer
-	Finished decryption for receiving


##v1.0
This plug-in implemented encryption on both chat messages and file transfer.


## Version Notes
- (latest)
- safeskypev2_20131119.zip:  纯用keyboard hook的程序
- safeskypev1_2: messaging 用keyboard hook, file transfer 用skype4com的程序
- safeskypev1_1: 纯用skype4com的版本，带token设置界面
- safeskypev1  : 纯用skype4com的版本

- (earlier versions)
- skypehook_20131029.zip:   包含msg hook and file transfer hook. using Skype4Com
- skypehook_20130918.zip:   只有msg hook. using Skype4Com

