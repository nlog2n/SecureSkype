using System;
using System.Collections.Generic;
//using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Diagnostics;

namespace ClickDN
{
    public partial class GlobalHook
    {
        public event MouseEventHandler OnMouseActivity;
        public event KeyEventHandler OnKeyDown;
        public event KeyPressEventHandler OnKeyPress;
        public event KeyEventHandler OnKeyUp;

        public string targetWindowName = null;

        #region params for SetWindowsHookEx
        int hMouseHook = 0; //Declare mouse hook handle as int.
        int hKeyboardHook = 0; //Declare keyboard hook handle as int.
        int hMessageHook = 0; //Declare callwndproc hook handle as int.

        public delegate int HookProc(int nCode, int wParam, IntPtr lParam);
        HookProc MouseHookProcedure; //Declare MouseHookProcedure as HookProc type.
        HookProc KeyboardHookProcedure; //Declare KeyboardHookProcedure as HookProc type.
        HookProc MessageHookProcedure; //Declare MessageHookProcedure as HookProc type.
        #endregion

        public GlobalHook(string appName)
        {
            targetWindowName = appName;

            //Start();
        }

        ~GlobalHook()
        {
            Stop();
        }

        // determine whether the current process window is the target or not
        // wndName: example "Untitled - Notepad"
        public bool IsTargetWindow()
        {
            IntPtr wndHandle = FindWindow(null, targetWindowName);
            if (wndHandle != IntPtr.Zero)
            {
                // is active?
                IntPtr curWndHandle = GetForegroundWindow();
                if ( curWndHandle == wndHandle )
                    return true;
            }

            return false;
        }


        public void SimulateKeyboardInput(string kbInput)
        {

            //释放按键的常量
            // int KEYEVENTF_KEYUP = 2; // used by keybd_event()

            // set target application to the foreground
            //SetForegroundWindow(wndHandle);

            // fake input password ?
            //byte VK_Q = (byte)Keys.NumPad7;
            //keybd_event(VK_Q, 0, 0, 0);//按下小键盘7
            //keybd_event(VK_Q, 0, KEYEVENTF_KEYUP, 0); //松开小键盘7
        }


        public void Start()
        {
            // get current process, which is this application
            // this is for global hook
            //hInstanceWnd = Marshal.GetHINSTANCE(Assembly.GetExecutingAssembly().GetModules()[0]);    // from ClickDN example
            // or
            IntPtr hInstanceWnd = GetModuleHandle(Process.GetCurrentProcess().MainModule.ModuleName);

            // turn to hook to a specific process
            // Note: the following code does not work for 32-bit process running on 64-bit or vice versa.
            //   someone says GetModuleFileName can also do the work, but i did not figure out
            //Process prs = Process.GetProcessesByName("NotePad")[0];
            //hInstanceWnd = prs.MainModule.BaseAddress; // or hInstanceWnd = GetModuleHandle(prs.MainModule.ModuleName);


            // install Mouse hook 
            if (hMouseHook == 0)
            {
                // Create an instance of HookProc for mouse
                // Make sure we keep a reference to this delegate!
                // If not, GC randomly collects it, and a NullReference exception is thrown
                MouseHookProcedure = new HookProc(MouseHookProc);

                try
                {
                    hMouseHook = SetWindowsHookEx(
                        HookTypes.WH_MOUSE_LL, // WH_MOUSE_LL,
                        MouseHookProcedure, 
                        hInstanceWnd, // Marshal.GetHINSTANCE(Assembly.GetExecutingAssembly().GetModules()[0]),
                        0);
                }
                catch (Exception)
                { }

                //If SetWindowsHookEx fails.
                if (hMouseHook == 0)
                {
                    throw new Exception("mouse hook failed.");
                }
            }

            // install Keyboard hook 
            if (hKeyboardHook == 0)
            {
                // Create an instance of HookProc for keyboard
                KeyboardHookProcedure = new HookProc(KeyboardHookProc);

                try
                {
                    hKeyboardHook = SetWindowsHookEx(
                        HookTypes.WH_KEYBOARD_LL, // WH_KEYBOARD_LL,
                        KeyboardHookProcedure, 
                        hInstanceWnd, //Marshal.GetHINSTANCE(Assembly.GetExecutingAssembly().GetModules()[0]),
                        0);
                }
                catch (Exception)
                { }

                //If SetWindowsHookEx fails.
                if (hKeyboardHook == 0)
                {
                    throw new Exception("keyboard hook failed.");
                }
            }

            // install Message hook 
            if (hMessageHook == 0)
            {
                // Create an instance of HookProc for message
                MessageHookProcedure = new HookProc(MessageHookProc);

                try
                {
                    hMessageHook = SetWindowsHookEx(
                        HookTypes.WH_CALLWNDPROC,
                        MessageHookProcedure,
                        hInstanceWnd, //Marshal.GetHINSTANCE(Assembly.GetExecutingAssembly().GetModules()[0]),
                        0);
                }
                catch (Exception)
                { }

                //If SetWindowsHookEx fails.
                if (hMessageHook == 0)
                {
                    //throw new Exception("message hook failed.");
                    Console.WriteLine("warning: message hook failed.");
                }
            }
        }

        public void Stop()
        {
            bool retMouse = true;
            bool retKeyboard = true;
            bool retMessage = true;
            if (hMouseHook != 0)
            {
                retMouse = UnhookWindowsHookEx(hMouseHook);
                hMouseHook = 0;
            }

            if (hKeyboardHook != 0)
            {
                retKeyboard = UnhookWindowsHookEx(hKeyboardHook);
                hKeyboardHook = 0;
            }

            if (hMessageHook != 0)
            {
                retMessage = UnhookWindowsHookEx(hMessageHook);
                hMessageHook = 0;
            }

            //If UnhookWindowsHookEx fails.
            if (!(retMouse && retKeyboard && retMessage))
            {
                throw new Exception("UnhookWindowsHookEx failed.");
            }
        }




        private int MouseHookProc(int nCode, int wParam, IntPtr lParam)
        {
            if (IsTargetWindow())

            // if ok and someone listens to our events
            if ((nCode >= 0) && (OnMouseActivity != null))
            {
                MouseButtons button = MouseButtons.None;
                switch (wParam)
                {
                    case (int)MsgTypes.WM_LBUTTONDOWN:
                        //case WM_LBUTTONUP: 
                        //case WM_LBUTTONDBLCLK: 
                        button = MouseButtons.Left;
                        break;
                    case (int)MsgTypes.WM_RBUTTONDOWN:
                        //case WM_RBUTTONUP: 
                        //case WM_RBUTTONDBLCLK: 
                        button = MouseButtons.Right;
                        break;
                }
                int clickCount = 0;
                if (button != MouseButtons.None)
                    if (wParam == (int)MsgTypes.WM_LBUTTONDBLCLK || wParam == (int)MsgTypes.WM_RBUTTONDBLCLK)
                        clickCount = 2;
                    else
                        clickCount = 1;

                //Marshall the data from callback.
                MouseHookStruct MyMouseHookStruct =
                    (MouseHookStruct)Marshal.PtrToStructure(lParam, typeof(MouseHookStruct));
                MouseEventArgs e = new MouseEventArgs(
                    button,
                    clickCount,
                    MyMouseHookStruct.pt.x,
                    MyMouseHookStruct.pt.y,
                    0);
                OnMouseActivity(this, e);
            }

            return CallNextHookEx(hMouseHook, nCode, wParam, lParam);
        }


        private int KeyboardHookProc(int nCode, int wParam, IntPtr lParam)
        {
            if (IsTargetWindow())

            // it was ok and someone listens to events
            if ((nCode >= 0) && (OnKeyDown != null || OnKeyUp != null || OnKeyPress != null))
            {
                KeyboardHookStruct MyKeyboardHookStruct = (KeyboardHookStruct)Marshal.PtrToStructure(lParam, typeof(KeyboardHookStruct));

                // raise KeyDown
                if (OnKeyDown != null && (wParam == (int)MsgTypes.WM_KEYDOWN || wParam == (int)MsgTypes.WM_SYSKEYDOWN))
                {
                    Keys keyData = (Keys)MyKeyboardHookStruct.vkCode;
                    KeyEventArgs e = new KeyEventArgs(keyData);
                    OnKeyDown(this, e);
                }

                // raise KeyPress
                if (OnKeyPress != null && wParam == (int)MsgTypes.WM_KEYDOWN)
                {
                    byte[] keyState = new byte[256];
                    GetKeyboardState(keyState);

                    byte[] inBuffer = new byte[2];
                    if (ToAscii(MyKeyboardHookStruct.vkCode,
                        MyKeyboardHookStruct.scanCode,
                        keyState,
                        inBuffer,
                        MyKeyboardHookStruct.flags) == 1)
                    {
                        KeyPressEventArgs e = new KeyPressEventArgs((char)inBuffer[0]);
                        OnKeyPress(this, e);
                    }
                }

                // raise KeyUp
                if (OnKeyUp != null && (wParam == (int)MsgTypes.WM_KEYUP || wParam == (int)MsgTypes.WM_SYSKEYUP))
                {
                    Keys keyData = (Keys)MyKeyboardHookStruct.vkCode;
                    KeyEventArgs e = new KeyEventArgs(keyData);
                    OnKeyUp(this, e);
                }
            }

            return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
        }

        private int MessageHookProc(int nCode, int wParam, IntPtr lParam)
        {
            if (IsTargetWindow())

                // it was ok and someone listens to events
                if (nCode >= 0 )  // HC_ACTION = 0
                {
                    try
                    {
                        CWPSTRUCT cwp = (CWPSTRUCT)Marshal.PtrToStructure(lParam, typeof(CWPSTRUCT));

                        IntPtr wndHandle = FindWindow(null, targetWindowName);
                        if (wndHandle != IntPtr.Zero && cwp.hwnd == wndHandle)
                        {
                            Console.WriteLine("captured 1 msg from " + targetWindowName);
                            //OnMessage(this, e);

                            if (cwp.message == (int)MsgTypes.WM_CLOSE)
                            {
                                // the hooked process is about to close
                            }
                        }
                    }
                    catch (Exception)
                    {
                    }
                }

            return CallNextHookEx(hMessageHook, nCode, wParam, lParam);
        }

    }
}
