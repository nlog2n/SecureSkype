using System;

namespace ClickDN
{
    /// <summary>
    /// 钩子类型
    /// using low-level mouse and keyboard constants for global hook. 
    /// used by SetWindowsHookEx. values from Winuser.h in Microsoft SDK.
    /// </summary>
    public enum HookTypes : int
    {
        // Note: Global hooks are not supported in the .NET Framework 
        // except for the WH_KEYBOARD_LL low-level hook and the WH_MOUSE_LL low-level hook

        // We can use SetWindowsHookEx method to set a WH_CALLWNDPROC hook. 
        // By setting a hook Windows will automatically Loads our hook dll to all processes. 
        // But the problem with this method is, we don’t know the target process is .NET or not. 
        // So we have to write a native dll for this hook operation. Because if the target process 
        // is not .NET this means there is no CLR attached to that process and our .NET dll won’t 
        // be worked within this process. When we call SetWindowsHookEx function systems attach the dll, 
        // which contains the hook method, to all processes. At this point; you have to know that 
        // for each process, system creates a new instance of our dll and attach it to every process. 

        WH_JOURNALRECORD = 0,//对寄送至消息队列的输入消息进行记录
        WH_JOURNALPLAYBACK = 1,//对此前由WH_JOURNALRECORD 挂钩处理过程纪录的消息进行寄送
        WH_GETMESSAGE = 3,//对寄送至消息队列的消息进行监视
        WH_CALLWNDPROC = 4,//在系统将消息发送至目标窗口处理过程之前，对该消息进行监视
        WH_CBT = 5,//接受对CBT应用程序有用的消息
        WH_SYSMSGFILTER = 6,//监视由对话框,消息框,菜单条,滚动条中的输入事件引发的消息
        WH_HARDWARE = 8,
        WH_DEBUG = 9,//对其他钩子处理过程进行调试
        WH_SHELL = 10,//接受对外壳应用程序有用的通知
        WH_FOREGROUNDIDLE = 11,//当应用程序的前台线程即将进入空闲状态时被调用,它有助于在空闲时间内执行低优先级的任务
        WH_CALLWNDPROCRET = 12,
        WH_MSGFILTER = -1,//监视由对话框、消息框、菜单条、或滚动条中的输入事件引发的消息

        #region 鼠标和键盘事件
        /// <summary>
        /// 对击键消息进行监视
        /// </summary>
        WH_KEYBOARD = 2,
        /// <summary>
        /// 只能在windows NT中被安装,用来对底层的键盘输入事件进行监视
        /// </summary>
        WH_KEYBOARD_LL = 13,
        /// <summary>
        /// 对鼠标消息进行监视
        /// </summary>
        WH_MOUSE = 7,
        /// <summary>
        /// 只能在windows NT中被安装,用来对底层鼠标事件进行监视
        /// </summary>
        WH_MOUSE_LL = 14,
        #endregion
    }
}
