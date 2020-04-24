# SR_DeviceInterface
SR设备操作接口动态库，包括设备登录和验证，播放音频文件，对讲，SD卡操作，IO时间消息，串口透传等。

#v0.1.3
2020-01-15
扩展设备登录认证方式，兼容设备v5.1.3之前的登录方式（id是数值，auth1=md5(password@username),auth2=md5(password)）

#v0.2.1
2020-04-06
modify:使用DeviceGet，DeviceGetByAddr替代DeviceFind和DeviceFindByAddr，Get函数在获得设备D的结构指针后，立即对D进行加锁，使用完成后需要调用DeviceRelease解锁。
modify:TcpServerThread不再使用指向设备D的指针，而是使用Ip地址，每次使用设备D前都通过DeviceGetByAddr获取。
modify:原来使用设备D指针的函数修改为使用UserID序号，在需要操作设备D时，通过DeviceGet获取。
modify:读SD卡容量信息函数，最大延时修改为8000ms
add:SR_USER_LOGIN_INFO结构体添加uTimeOut成员，取值[10,3600]秒，设备通讯超时后调用回调函数。添加MSGTYPE_TIMEOUT，MSGTYPE_RECONNECT枚举,用于描述设备超时事件和再连接事件。
    添加DeviceTick函数，在TcpServerThread线程定时调用，用于判断超时事件。
[注]：不要在gfExceptionCallBack回调调用本动态库的函数，在回调函数中不应进行耗时的工作，不要在回调中更新用户界面UI。
