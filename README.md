# SR_DeviceInterface
SR设备操作接口动态库，包括设备登录和验证，播放音频文件，对讲，SD卡操作，IO时间消息，串口透传等。

v0.1.3
2020-01-15
扩展设备登录认证方式，兼容设备v5.1.3之前的登录方式（id是数值，auth1=md5(password@username),auth2=md5(password)）
