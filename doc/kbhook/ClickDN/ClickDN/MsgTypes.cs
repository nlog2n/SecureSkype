using System;

namespace ClickDN
{
    /// <summary>
    /// 消息类型
    /// 作为SendMessage和PostMessage的参数
    /// </summary>
    public enum MsgTypes : int
    {
        #region constants for mouse events
        WM_MOUSEMOVE = 0x200,
        WM_LBUTTONDOWN = 0x201,
        WM_RBUTTONDOWN = 0x204,
        WM_MBUTTONDOWN = 0x207,
        WM_LBUTTONUP = 0x202,
        WM_RBUTTONUP = 0x205,
        WM_MBUTTONUP = 0x208,
        WM_LBUTTONDBLCLK = 0x203,
        WM_RBUTTONDBLCLK = 0x206,
        WM_MBUTTONDBLCLK = 0x209,
        #endregion

        #region constants for keyboard events
        WM_KEYDOWN = 0x100,
        WM_KEYUP = 0x101,
        WM_SYSKEYDOWN = 0x104,
        WM_SYSKEYUP = 0x105,
        #endregion

        #region windows messages
        WM_CREATE = 0x01, // 创建一个窗口
        WM_DESTROY = 0x02, // 当一个窗口被破坏时发送
        WM_MOVE = 0x03, // 移动一个窗口
        WM_SIZE = 0x05, // 改变一个窗口的大小
        WM_ACTIVATE = 0x06, // 一个窗口被激活或失去激活状态
        WM_SETFOCUS = 0x07, // 一个窗口获得焦点
        WM_KILLFOCUS = 0x08, // 一个窗口失去焦点
        WM_SETTEXT = 0x0C, // 应用程序发送此消息来设置一个窗口的文本
        WM_GETTEXT = 0x0D, // 应用程序发送此消息来复制对应窗口的文本到缓冲区
        WM_GETTEXTLENGTH = 0x0E, // 得到与一个窗口有关的文本的长度（不包含空字符）
        WM_PAINT = 0x0F, // 要求一个窗口重画自己
        WM_CLOSE = 0x10, // 当一个窗口或应用程序要关闭时发送一个信号
        WM_QUERYENDSESSION = 0x11, // 当用户选择结束对话框或程序自己调用ExitWindows函数
        WM_QUIT = 0x12, // 用来结束程序运行
        #endregion

        // more please refer to
        // http://www.cnblogs.com/Johness/archive/2012/12/28/2837977.html
    }
}
