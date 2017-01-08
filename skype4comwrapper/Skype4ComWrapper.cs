using System;
using System.IO;
using System.Collections.Generic;
using System.Data;
using System.Text;

using SKYPE4COMLib;

namespace SafeSkype
{
    public class Skype4ComWrapper
    {
        #region Class Data

        private string _logText = "";

        // skype attachment
        private Skype _objSkype;
        private SkypeClass _clsSkype;

        // skype status
        private bool _blnAttached;
        private bool _blnIsOnline;
        private bool _blnWasAttached;
        private bool _blnPendingSilentModeStartup;


        private string[] _strShortCutNums;
        private int _intIncomingCallId;
        private int _intOutgoingCallId;
        private SortedList<string, string> _colSkypeInMappings;

        private string _strMobileUser;

        // current skype user handler
        public string UserName { get { return _objSkype.CurrentUserHandle; } }

        public List<string> FriendNames
        {
            get
            {
                List<string> result = new List<string>();
                foreach (IUser f in _objSkype.Friends)
                {
                    result.Add(f.Handle.ToString());
                }
                return result;
            }
        }


        private bool _encrypted = false; // need encryption or not
        private Dictionary<string, string> _tokens = new Dictionary<string, string>(); // key=friend name, value=password

        public string GetToken(string peer)
        {
            if (!_encrypted) return null;
            if (!_tokens.ContainsKey(peer)) return null;

            return _tokens[peer];
        }


        private string _chatOriginator = null;


        #endregion

        #region Startup

        public Skype4ComWrapper()
        {
            // Start up our application
            WriteToLog("Starting SafeSkype plugin v1.0");
            WriteToLog();

            // Attach to Skype
            WriteToLog("Attaching to Skype");
            this._objSkype = new Skype();

            // Set up the event handlers
            WriteToLog("Attaching to Skype events");
            _clsSkype = new SkypeClass();
            _clsSkype._ISkypeEvents_Event_AttachmentStatus += new _ISkypeEvents_AttachmentStatusEventHandler(Skype_AttachmentStatus);
            _clsSkype._ISkypeEvents_Event_ConnectionStatus += new _ISkypeEvents_ConnectionStatusEventHandler(Skype_ConnectionStatus);

            _objSkype.MessageStatus += new _ISkypeEvents_MessageStatusEventHandler(Skype_MessageStatus);
            _objSkype.CallStatus += new _ISkypeEvents_CallStatusEventHandler(Skype_CallStatus);
            _objSkype.FileTransferStatusChanged += new _ISkypeEvents_FileTransferStatusChangedEventHandler(Skype_FileTransferStatusChanged);

            //_objSkype.Client.CreateMenuItem("menu1", TPluginContext.pluginContextChat, "Example menu", "Example menu hint", "", true, TPluginContactType.pluginContactTypeAll, false);
            //_objSkype.Client.CreateEvent("event1", "example plugin is running", "click here to stop the example");

            try
            {
                // Attach to Skype4COM
                _clsSkype.Attach(7, false);
            }
            catch (Exception)
            {
                // All Skype Logic uses TRY for safety
            }

            try
            {
                if (!_objSkype.Client.IsRunning)
                {
                    _objSkype.Client.Start(false, true);
                }
            }
            catch (Exception)
            {
                // All Skype Logic uses TRY for safety
            }
        }


        public void Skype_AttachmentStatus(TAttachmentStatus status)
        {
            _blnAttached = false;

            // DEBUG: Write Attachment Status to Window
            //WriteToLog("Attachment Status: " + cSkype.Convert.AttachmentStatusToText(status));
            //WriteToLog(" - " + status.ToString() + Environment.NewLine);

            if (status == TAttachmentStatus.apiAttachAvailable)
            {
                try
                {
                    // This attaches to the Skype4COM class statement
                    _clsSkype.Attach(7, true);
                }
                catch (Exception)
                {
                    // All Skype Logic uses TRY for safety
                }
            }
            else
                if (status == TAttachmentStatus.apiAttachSuccess)
                {
                    try
                    {
                        System.Windows.Forms.Application.DoEvents();
                        _objSkype.Attach(7, false);
                    }
                    catch (Exception)
                    {
                        // All Skype Logic uses TRY for safety
                    }

                    _blnAttached = true;
                    _blnWasAttached = true;

                    // If we have a queued Silent Mode request, We are attached, process it now
                    if (_blnPendingSilentModeStartup)
                    {
                        _blnPendingSilentModeStartup = false;
                        try
                        {
                            if (!_objSkype.SilentMode) _objSkype.SilentMode = true;
                        }
                        catch (Exception)
                        {
                            // All Skype Logic uses TRY for safety
                        }
                    }
                }
        }

        public void Skype_ConnectionStatus(TConnectionStatus status)
        {
            _blnIsOnline = false;

            // DEBUG: Write Connection Status to Window
            //WriteToLog("Connection Status: " + cSkype.Convert.ConnectionStatusToText(status));
            //WriteToLog(" - " + status.ToString() + Environment.NewLine);

            if (status == TConnectionStatus.conOnline)
            {
                _blnIsOnline = true;
                WriteToLog("Connected to Skype user: " + _objSkype.CurrentUserHandle);
                string names = "";
                foreach (string f in this.FriendNames)
                {
                    names += f + ",";
                }
                WriteToLog("Friends: " + names.Trim(','));

                if (this._tokens.Count == 0)
                {
                    // set default password
                    foreach (string f in this.FriendNames)
                    {
                        this._tokens[f] = f + "'s token";
                    }

                    this._tokens[this.UserName] = this.UserName + "'s token";  // it is me
                }
                WriteToLog("set default tokens");

                WriteToLog("Now listening for events...");
                WriteToLog();
            }
        }

        #endregion

        #region Call Status

        private void Skype_CallStatus(Call aCall, TCallStatus aStatus)
        {
            switch (aStatus)
            {
                case TCallStatus.clsRinging:
                    // A call is ringing, see if it is us calling or
                    // someone calling us
                    if (aCall.Type == TCallType.cltIncomingP2P || aCall.Type == TCallType.cltIncomingPSTN)
                    {
                        // Make sure this isn't a SkypeOut call request
                        if (aCall.PartnerHandle.Equals(_strMobileUser))
                        {
                            WriteToLog("SkypeOut call from " + _strMobileUser);
                            WriteToLog("Allowing call to be forwarded by Skype");
                            return;
                        }
                        // Incoming call, we need to initiate forwarding
                        WriteToLog("Answering call from " + aCall.PartnerHandle);
                        aCall.Answer();
                        this._intIncomingCallId = aCall.Id;
                        aCall.set_InputDevice(TCallIoDeviceType.callIoDeviceTypeSoundcard, "default");
                        aCall.set_InputDevice(TCallIoDeviceType.callIoDeviceTypePort, "1");
                        string strWavPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location) + "\\PleaseWait.wav";
                        aCall.set_InputDevice(TCallIoDeviceType.callIoDeviceTypeFile, strWavPath);
                        System.Threading.Thread.Sleep(8000);
                        aCall.set_InputDevice(TCallIoDeviceType.callIoDeviceTypeFile, "");
                        WriteToLog("Placing call on hold");
                        aCall.Hold();

                        // Check to see if we need to foward this to
                        // a configured SkypeIn mapping
                        string strTargetNumber;
                        if (this._colSkypeInMappings.ContainsKey(aCall.TargetIdentity))
                        {
                            // Yes set the target to the configured value
                            strTargetNumber = this._colSkypeInMappings[aCall.TargetIdentity];
                            WriteToLog("Calling SkypeIn forwarding number " + strTargetNumber);
                        }
                        else
                        {
                            // No, use the mobile user
                            strTargetNumber = this._strMobileUser;
                            WriteToLog("Calling mobile user");
                        }

                        System.Threading.Thread.Sleep(500);
                        try
                        {
                            Call oCall = _objSkype.PlaceCall(strTargetNumber, null, null, null);
                            this._intOutgoingCallId = oCall.Id;
                        }
                        catch (Exception ex)
                        {
                            WriteToLog("Error trying to call target: " + ex.Message);
                        }
                    }
                    break;
                case TCallStatus.clsInProgress:
                    // We have a new call opened. Make sure it's our outgoing call
                    if (aCall.Id == this._intOutgoingCallId)
                    {
                        // Yes, the target user has answered.
                        WriteToLog("Target user has answered, attempting to join calls");
                        foreach (Call objCall in _objSkype.ActiveCalls)
                        {
                            if (objCall.Id == this._intIncomingCallId)
                            {
                                WriteToLog("Joining the calls...");
                                objCall.Join(aCall.Id);
                                WriteToLog("Taking incoming call off hold");
                                objCall.Resume();
                            }
                        }
                    }
                    break;
                case TCallStatus.clsFinished:
                    // Someone has hung up, end the call
                    WriteToLog("Someone has hung up. Attempting to end the conference");
                    foreach (Conference objConf in _objSkype.Conferences)
                    {
                        foreach (Call objCall in objConf.Calls)
                        {
                            if (objCall.Id == this._intIncomingCallId || objCall.Id == this._intOutgoingCallId)
                            {
                                System.Threading.Thread.Sleep(500);
                                try
                                {
                                    objCall.Finish();
                                }
                                catch (Exception) { }
                                try
                                {
                                    objConf.Finish();
                                }
                                catch (Exception) { }
                            }
                        }
                    }
                    break;
                default:
                    // Something else?
                    if ((aCall.Type == TCallType.cltOutgoingP2P || aCall.Type == TCallType.cltOutgoingPSTN) && (
                            aCall.Status == TCallStatus.clsCancelled ||
                            aCall.Status == TCallStatus.clsFailed ||
                            aCall.Status == TCallStatus.clsMissed ||
                            aCall.Status == TCallStatus.clsRefused ||
                            aCall.Status == TCallStatus.clsVoicemailPlayingGreeting ||
                            aCall.Status == TCallStatus.clsVoicemailRecording
                        )
                       )
                    {
                        WriteToLog("Error calling target user: " + _objSkype.Convert.CallStatusToText(aCall.Status));
                        WriteToLog("Redirecting to voicemail");
                        // End the other call
                        foreach (Call objCall in _objSkype.ActiveCalls)
                        {
                            if (objCall.Id == this._intOutgoingCallId)
                            {
                                try
                                {
                                    objCall.Finish();
                                }
                                catch (Exception ex)
                                {
                                    WriteToLog("Error trying to end voicemail call: " + ex.Message);
                                }
                            }
                        }
                        // Now redirect the incoming call
                        foreach (Call objCall in _objSkype.ActiveCalls)
                        {
                            if (objCall.Id == this._intIncomingCallId)
                            {
                                System.Threading.Thread.Sleep(500);
                                try
                                {
                                    //objCall.Resume();
                                    objCall.RedirectToVoicemail();
                                    objCall.Finish();
                                    //objCall.Status = TCallStatus.clsFinished;
                                }
                                catch (Exception ex)
                                {
                                    WriteToLog("Error trying to divert to voicemail: " + ex.Message);
                                    objCall.Finish();
                                }
                            }
                        }
                    }
                    break;
            }
        }

        #endregion

        #region Chat Messages

        private void Skype_MessageStatus(ChatMessage pMessage, TChatMessageStatus aStatus)
        {
            try
            {
                WriteToLog(pMessage.Sender.Handle.ToString() + " " + aStatus.ToString() + " " + pMessage.Body);
                string peer = pMessage.Chat.DialogPartner;
                string password = GetToken(peer);

                if (aStatus == TChatMessageStatus.cmsSending)
                {
                    if (!string.IsNullOrEmpty(password))
                    {
                        // encrypt
                        byte[] msgbytes = SkypeEncryptor.Encrypt(System.Text.Encoding.Unicode.GetBytes(pMessage.Body), password);
                        pMessage.Body = Convert.ToBase64String(msgbytes); // alter the message body
                        WriteToLog("Message encrypted");
                    }
                }
                else if (aStatus == TChatMessageStatus.cmsReceived || aStatus == TChatMessageStatus.cmsRead)
                {
                    if (!string.IsNullOrEmpty(password))
                    {
                        // decrypt
                        byte[] msgbytes = Convert.FromBase64String(pMessage.Body);
                        byte[] dec = SkypeEncryptor.Decrypt(msgbytes, password);
                        string msg = System.Text.Encoding.Unicode.GetString(dec);
                        WriteToLog("Message decrypted: " + msg);
                        //pMessage.Body = msg; // which is not editable
                    }

                    if (false) // (pMessage.Type == TChatMessageType.cmeSaid)
                    {
                        // Make sure the request came from the mobile account
                        if (pMessage.Sender.Handle.Equals(this._strMobileUser))
                        {
                            // OK, they want to make a change request or send an SMS
                            try
                            {
                                if (pMessage.Body.Length > 3 && pMessage.Body.ToLower().Substring(0, 3).Equals("sms"))
                                {
                                    // This is an SMS request
                                    WriteToLog("SMS request received from " + this._strMobileUser);

                                    // Get the number to SMS to
                                    string[] strBits = pMessage.Body.Split(" ".ToCharArray());
                                    // The number to call is the second argument
                                    // (string is in the format "sms [number] [message]"
                                    string strSmsTarget = strBits[1];
                                    // See if it is a quickswitch number
                                    int intQuickSwitch;
                                    if (int.TryParse(strSmsTarget, out intQuickSwitch) && (intQuickSwitch > 0 && intQuickSwitch < _strShortCutNums.Length))
                                    {
                                        // Yes, this is a quickswitch number. Get the number for it
                                        strSmsTarget = this._strShortCutNums[intQuickSwitch];
                                    }
                                    WriteToLog("Sending SMS to " + strSmsTarget);

                                    // Get the message
                                    string strMessage = pMessage.Body.Substring(pMessage.Body.IndexOf(" ", 4) + 1);
                                    WriteToLog("Message is: " + strMessage);

                                    // Send the SMS
                                    _objSkype.SendSms(strSmsTarget, strMessage, null);
                                    WriteToLog("SMS sent");
                                    pMessage.Chat.SendMessage("SMS sent to " + strSmsTarget);
                                    WriteToLog();
                                }
                                else
                                {
                                    // This is a SkypeOut change request
                                    WriteToLog("Forwarding change request received from " + this._strMobileUser);
                                    string strNewNum = "";
                                    switch (pMessage.Body.ToLower())
                                    {
                                        case "off":
                                            // Switch off forwarding
                                            _objSkype.CurrentUserProfile.CallApplyCF = false;
                                            _objSkype.CurrentUserProfile.CallForwardRules = "";
                                            break;
                                        case "1":
                                        case "2":
                                        case "3":
                                        case "4":
                                        case "5":
                                            // Quick switch number
                                            strNewNum = this._strShortCutNums[int.Parse(pMessage.Body)];
                                            break;
                                        case "contacts":
                                            // List all the contact numbers
                                            string strContacts = "Quick switch numbers:" + Environment.NewLine;
                                            for (int i = 1; i <= 5; i++)
                                            {
                                                strContacts += i.ToString() + ": " + this._strShortCutNums[i] + Environment.NewLine;
                                            }
                                            WriteToLog("Sending user the list of quick switch numbers");
                                            pMessage.Chat.SendMessage(strContacts);
                                            return; // Exit out of the function completely
                                        default:
                                            strNewNum = pMessage.Body;
                                            break;
                                    }

                                    if (string.IsNullOrEmpty(strNewNum))
                                    {
                                        pMessage.Chat.SendMessage("Switched off call forwarding");
                                        WriteToLog("Switched off call forwarding");
                                    }
                                    else
                                    {
                                        _objSkype.CurrentUserProfile.CallApplyCF = true;
                                        _objSkype.CurrentUserProfile.CallForwardRules = "0,60," + strNewNum;
                                        _objSkype.CurrentUserProfile.CallNoAnswerTimeout = 5;
                                        pMessage.Chat.SendMessage("Reset call forwarding to " + strNewNum);
                                        WriteToLog("Changed SkypeOut forwarding to " + strNewNum);
                                    }
                                }
                            }
                            catch (Exception ex)
                            {
                                pMessage.Chat.SendMessage("Error:  " + ex.Message);
                            }
                        }
                        else
                        {
                            // Someone else? Log and ignore
                            WriteToLog("Chat message received from " + pMessage.Sender.Handle + " was ignored");
                            WriteToLog("Message was: " + pMessage.Body);
                        }
                    }
                }
            }
            catch (Exception)
            {
            }
        }

        #endregion

        #region File Transfer

        void Skype_FileTransferStatusChanged(IFileTransfer pTransfer, TFileTransferStatus Status)
        {
            try
            {
                WriteToLog(pTransfer.Type.ToString() + " " + pTransfer.Filename + " Status: " + Status.ToString());

                if (pTransfer.Type == TFileTransferType.fileTransferTypeOutgoing)  // sending
                {
                    string peer = pTransfer.PartnerHandle;
                    string password = GetToken(peer);
                    if (string.IsNullOrEmpty(password))
                        return; // no encryption

                    if (peer != this.UserName)
                        return; // tricky: no duplicated encryption

                    string filepath = pTransfer.FilePath; //.Filename;
                    if (Status == TFileTransferStatus.fileTransferStatusNew)
                    {
                        // encrypt the file content
                        byte[] content = File.ReadAllBytes(filepath);
                        if (content == null)
                        {
                            WriteToLog(" -- read 0 bytes...");
                            return;
                        }
                        WriteToLog(" -- read " + content.Length + " bytes...");

                        byte[] content2 = SkypeEncryptor.Encrypt(content, password);
                        if (content2 == null)
                        {
                            WriteToLog(" -- encrypt error!");
                            return;
                        }
                        WriteToLog(" -- encrypt " + content2.Length + " bytes...");

                        // Tricky: at this time the file is still held by Skype,
                        //    so the bytes are actually written into cache (sometime even not before receiver downloaded it)
                        File.WriteAllBytes(filepath, content2);
                        WriteToLog(" -- write " + content2.Length + " bytes...");
                    }
                    else if (Status == TFileTransferStatus.fileTransferStatusCancelled ||
                        Status == TFileTransferStatus.fileTransferStatusCompleted ||
                        Status == TFileTransferStatus.fileTransferStatusFailed)
                    {
                        if (pTransfer.PartnerHandle == this.UserName) // message for myself
                        {
                            // decrypt the file content
                            byte[] content = File.ReadAllBytes(filepath);
                            if (content == null)
                            {
                                WriteToLog(" -- read 0 bytes...");
                                return;
                            }
                            WriteToLog(" -- read " + content.Length + " bytes...");


                            byte[] content2 = SkypeEncryptor.Decrypt(content, password);
                            if (content2 == null)
                            {
                                WriteToLog(" -- decrypt error!");
                                return;
                            }
                            WriteToLog(" -- decrypt " + content2.Length + " bytes...");

                            // Read the tricky thing above
                            File.WriteAllBytes(filepath, content2);
                            WriteToLog(" -- write " + content2.Length + " bytes...");
                        }
                    }
                }
                else  // receiving
                {
                    string peer = pTransfer.PartnerHandle;
                    string password = GetToken(peer);
                    if (string.IsNullOrEmpty(password))
                        return; // no encryption

                    if (Status == TFileTransferStatus.fileTransferStatusCompleted)
                    {
                        string filepath = pTransfer.FilePath; //.Filename;
                        // decrypt the file content
                        byte[] content = File.ReadAllBytes(filepath);
                        if (content == null)
                        {
                            WriteToLog(" -- read 0 bytes...");
                            return;
                        }
                        WriteToLog(" -- read " + content.Length + " bytes...");


                        byte[] content2 = SkypeEncryptor.Decrypt(content, password);
                        if (content2 == null)
                        {
                            WriteToLog(" -- decrypt error!");
                            return;
                        }
                        WriteToLog(" -- decrypt " + content2.Length + " bytes...");

                        // Read the tricky thing above
                        File.WriteAllBytes(filepath, content2);
                        WriteToLog(" -- write " + content2.Length + " bytes...");
                    }
                }
            }
            catch (Exception ex)
            {
                WriteToLog(ex.Message);
            }
        }

        #endregion


        #region Logging

        private void WriteToLog(string pMessage)
        {
            this._logText += DateTime.Now.ToString("dd/MM/yyyy HH:mm:ss:") + " " + pMessage + Environment.NewLine;
            //this.txtLog.ScrollToCaret();
        }

        private void WriteToLog()
        {
            this._logText += Environment.NewLine;
            //this.txtLog.ScrollToCaret();
        }

        #endregion




    }
}