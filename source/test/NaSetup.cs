using System;
using System.Text;
using System.Runtime.InteropServices;


    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi, Pack = 1)]

public struct _Param
{
    //设备类别
    public byte DeviceType;

    //未定义
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public byte[] Unuse0;

    //用户名
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
    public byte[] User;

    //密码
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
    public byte[] Password;

    //需要访问的串口号
    public byte ComName;

    //未定义
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
    public byte[] Unuse1;

    //未定义
    [MarshalAs(UnmanagedType.ByValArray, SizeConst = 24)]
    public byte[] Unuse2;
}

    public struct _DeviceInfo
    {
        public byte DeviceType;     //  设备类别

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public byte[] Unuse0;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] Version;      //  固件版本（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] DeviceName;   //	设备名称（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
        public byte[] Mac;          //	设备MAC地址

        public UInt16 LocalPort;    //	设备监听端口

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] LocalIP;      //	设备IP地址

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] PeerIP;       //	服务器IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] NetMask;      //  IP地址掩码

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] GatewayIP;    //	网关地址

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] DNSServerIP0; //	dns服务器IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] DNSServerIP1; //	备用dns服务器IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 24)]
        public byte[] IP;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] ServerIp;     //  第二服务器IP(备用)

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] DeviceTypeName;   //设备型号名称

        public UInt16 Unuse1;
        public byte UartNumber;     //串口
        public byte Unuse2;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] MultiGroup;   //  第一分组（1~4）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] MultiGroup2;  //  第二分组（5~8）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] NetInterface; //  接收数据帧的网卡IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] ServerIp1;    //  云服务器IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] DeviceNumber; //  设备编号（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 180)]
        public byte[] Unuse;
    }

    public struct _Port
    {
        public UInt16 LocalPort;    //	设备监听端口

        public UInt16 PeerPort;     //	服务器端口

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] PeerIP;       //	服务器IP

        public byte Protocol;       //	通信协议：		0(UDP)、1(TCP)，2(Real-com)其他无效

        public byte Workstyle;      //	0--CLIENT（长连接） 1--SERVER 2--CLIENT（短连接）

        public byte URLEnable;      //	使用URL，为1表示使用网址作代替服务器IP地址。为0该选项无效。

        public byte KeepLive;       //  保活定时器时间，单位为分钟。[0~255]，0表示不需要保活定时器

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] URL;          //	目标URL地址。(char)

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] Unuse5;

        public UInt16 Baud;         //	串口波特率：串口波特率的百位数，如值为96表示波特率为9600bps

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] Unuse6;

        public byte DataBit;        //	字节位长度		7、8，		其他无效

        public byte Parity;         //	串口校验方式：	0 ~ 5(见define)，其他无效

        public byte StopBit;        //  停止位长度		1(1bit).2(2bit)。

        public byte FlowCtrl;       //  流控制，为0表示无，为1表示硬件RTS/CTS流控

        public UInt16 MaxPackLen;   //	每帧串口数据包的最大长度	1 ~ 1536，其他无效

        public byte LeastTIme;      //	每帧串口数据包的最小间隔时间	0 ~ 255ms

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 9)]
        public byte[] Unuse7;
    }

    public struct _DeviceEx
    {
        public byte Header;                                     //  帧头部

        public byte DeviceType;                                 //  设备类型

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
        public byte[] Mac;                                      //	设备MAC地址

        public byte UartNumber;                                 //  设备总的串口数量

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
        public byte[] Unuse0;                                   //	未定义

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] DeviceTypeName;                           //  设备类型字符串（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] Version;                                  //	设备的固件版本号，读参数有用，写参数时不考虑

        public UInt16 Unuse1;                                   //  未定义

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] Unuse2;                                   //	未定义

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] UserName;                                 //	用户名，以'\0'字符结束。（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Password;                                 //	用户密码，以'\0'字符结束。（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public Char[] DeviceName;                               //	设备名称，以'\0'字符结束。

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] MultiGroup;                               //  第一分组（1~4）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] LocalIP;                                  //	设备IP地址

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] NetMask;                                  //	子网掩码

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] GatewayIP;                                //	网关IP地址

        public byte DHCP;                                       //	0为禁止,1为允许。

        public byte AutoDNS;                                    //	是否自动配置DNS，为1时，如果启用DHCP，从DHCP服务器获取DNS地址。为0用户需要手动设置DNS服务器IP

        public byte TmpGroup;

        public byte TalkDialinCount;                            // 对讲自动接通呼叫振铃次数，等于时间秒数

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] ServerIP;                                 //	服务器IP地址

        public UInt16 ServerPort;                               //	服务器端口

        public sbyte ComVolume;                                 //音量偏量。-100-100

        public byte TalkVolume;                                 //	对讲时本机的音量大小。

        public byte DefaultSample;                              //	录播时，默认的采样频率0 = 8K；1=16K；2 = 24K；3 = 32K

        public byte DefaultAudioInput;                          //	对讲和录播时，默认的音频输入端口 0=mic，1=linein

        public byte ButtonMode;                                 //	按键模式，0=触发模式，1=保持模式

        public byte RecordVolume;                               //	录播时，向远端发送数据的音量，默认值。

        public UInt16 PADelayTime;                               //	功放延时关闭参数，单位秒

        public byte MicGain;                                    //	默认mic的增益大小0～100

        public byte MicGainLimit;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] AssistantIp;                              //	对讲呼叫转移设备ip地址，不用呼叫转移应填0.0.0.0

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp1;                         //	CustomServiceIp[0]是第一客服IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp2;                         //	CustomServiceIp[0]是第二客服IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp3;                         //	CustomServiceIp[0]是第三客服IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp4;                          //	CustomServiceIp[0]是第四客服IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 7)]
        public _Port[] Port;                                    //	与端口对应的串口相关参数。

        public byte UseAssistant;                               //自动呼叫转移或是手动呼叫转移

        public byte Unuse4;

        public byte TalkMode;                                   //对讲模式，为0表示全双工工作模式，非零表示半双工模式，

        public byte CodecType;                                  //对讲编码模式，0=pcm，1=adpcm

        public byte AECEnable;                                  //是否启用aec(回声抑制)

        public byte DefaultPlayMode;                            //默认播放模式

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse5;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] MultiGroup2;                               //   第二分组（5~8）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 72)]
        public byte[] Unuse;
    }

    public struct _DeviceEx2
    {
        public byte Header;                                     //  帧头部

        public byte DeviceType;                                 //  设备类别

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
        public byte[] Mac;                                      //	设备MAC地址

        public Int32 FunBits;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] DeviceTypeName;                           //  设备类型名称（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] Version;                                  //	设备的软件版本号，读参数有用，写参数时不考虑（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] Unuse2;                                   //	未定义

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] UserName;                                 //	用户名，以'\0'字符结束。（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Password;                                 //	用户密码，以'\0'字符结束。（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] DeviceName;                               //	设备名称（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] AliasName;                                //	设备别名(设备编号)

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] LocalIP;                                  //	设备IP地址

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] NetMask;                                  //	子网掩码

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] GatewayIP;                                //	网关IP地址

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] DNS_SERVER_0;                             //  DNS服务器1

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] DNS_SERVER_1;                             //  DNS服务器2

        public byte DHCP;                                       //	0为禁止,1为允许。

        public byte AutoDNS;                                    //	是否自动配置DNS，为1时，如果启用DHCP，从DHCP服务器获取DNS地址。为0用户需要手动设置DNS服务器IP

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] MultiGroup;                               //  第一分组（1~4）

        public byte TmpGroup;

        public byte TalkDialinCount;                            // 对讲自动接通呼叫振铃次数（等于时间秒数），呼入时间超过这个时间后自动应答。

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] ServerIP;                                 //	服务器IP地址

        public UInt16 ServerPort;                               //	服务器端口

        public byte ComVolume;                                  //	串口设置的音量大小偏移量，

        public byte TalkVolume;                                 //	对讲时本机的音量大小。

        public byte DefaultSample;                              //	录播时，默认的采样频率0 = 8K；1=16K；2 = 24K；3 = 32K

        public byte DefaultAudioInput;                          //	对讲和录播时，默认的音频输入端口 0=mic，1=linein，在对讲和录播模式下有效。

        public byte ButtonMode;                                 //	按键模式，0=触发模式，1=保持模式

        public byte RecordVolume;                               //	录播时，向远端发送数据的音量，默认值。

        public UInt16 PADelayTime;                              //	功放延时关闭时间，单位秒

        public byte MicGain;                                    //	默认mic的增益大小0～100

        public byte _MicDelayTime;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] AssistantIp;                              //	对讲呼叫转移设备ip地址，不用呼叫转移应填0.0.0.0

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp1;                         //	客服一IP（暂不用），应使用Btn01CustomService1字段获取客服信息
        
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp2;                         //	客服二IP（暂不用），应使用Btn01CustomService2字段获取客服信息

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp3;                         //	客服三IP（暂不用），应使用Btn01CustomService3字段获取客服信息

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] CustomServicelp4;                         //	客服四IP（暂不用），应使用Btn01CustomService4字段获取客服信息
        
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public _Port[] Port;                                    //	与端口对应的串口相关参数

        public byte Btn01Fun;                                   //  按键功能，0=对讲，1=采播，2=其他

        public byte Btn01SubMode;                               //  子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话; 采播时0=广播，1=组播，2=单播

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse101;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn01CustomService1;                      //  按键1，第一客服

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn01CustomService2;                      //  按键1，第二客服

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn01CustomService3;                      //  按键1，第三客服

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn01CustomService4;                      //  按键1，第四客服（char）

        public byte Btn01DivertEnable;                          //  =1运行呼叫转移

        public byte Btn01DivertSubMode;                         //  子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse102;
        
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn01Divert;          //按键1呼叫转移目标（char）


        public byte Btn02Fun;               //按键功能，0=对讲，1=采播，2=其他

        public byte Btn02SubMode;           //子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话; 采播时0=广播，1=组播，2=单播

        public byte Btn02DivertEnable;      //=1运行呼叫转移

        public byte Btn02DivertSubMode;     //子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn02CustomService;   //按键2，客服

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn02Divert;          //按键2呼叫转移目标（char）

        public byte AECSelect;      //是否回声抑制

        public byte InputMode;      //输入模式，bit变量，如果对应位=0表示输入，1表示输出

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse003;

        public UInt32 unuse004;     //定义设备具有的功能

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] unuse201;


        public byte Btn03Fun;       //按键功能，0=对讲，1=采播，2=其他

        public byte Btn03SubMode;   //子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话; 采播时0=广播，1=组播，2=单播

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse301;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn03CustomService;   //按键3，客服（char）

        public byte Btn03DivertEnable;      //=1运行呼叫转移

        public byte Btn03DivertSubMode;     //子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse302;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn03Divert;          //按键2呼叫转移目标（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] SIPPassword;          //SIP密码（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 12)]
        public byte[] unuse303;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] URL_SERVER_0;     //第一服务器网址（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] URL_SERVER_1;     //第二服务器网址（char）

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] _ServerIP2;


        public byte Btn04Fun;       //按键功能，0=对讲，1=采播，2=其他

        public byte Btn04SubMode;   //子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话; 采播时0=广播，1=组播，2=单播

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse401;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn04CustomService;  //按键3，客服（char）

        public byte Btn04DivertEnable;  //=1运行呼叫转移

        public byte Btn04DivertSubMode; //子功能，在对讲是描述呼叫目标类型，0=ip地址，1=设备编号，2=sip电话;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
        public byte[] unuse402;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Btn04Divert;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] SIPProxyIp;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] SIPNumber;

        public byte UseAssistant;           //自动呼叫转移或是手动呼叫转移

        public byte Input4RecordPriority;   //输入4的默认优先级

        public byte TalkMode;               //对讲模式，为0表示全双工工作模式，非零表示半双工模式，

        public byte TalkCodecType;          //对讲编码模式，0=pcm，1=adpcm

        public byte AECEnable;              //是否启用aec(回声抑制)

        public byte DefaultPlayMode;        //默认播放模式

        public byte UartVersion;            //备用

        public byte RecordAudioInput;       //采播模式下的输入源，0=mic，1=input；v2.469

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] MultiGroup2;                               //   组号

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] EventServerIp;        //时间服务器地址
               
        public byte VolAjustStepEnable;     //播放声音渐进      允许=0，禁止=其他允许

        public byte ButtonInTalkEnable;     //在对讲是禁止按键结束通话=0禁止 其他允许

        public byte DingDongInRecordEnable; //设定在录播时是否启动叮咚音。=0禁止，其他允许

        public byte Input1RecordPriority;   //默认的录播优先级

        public byte DefaultCallTime;        //默认主动呼叫时间10~100s

        public byte BusyPlayTime;           //远端忙后，本地播放忙音的时间

        public byte VolumeSupress;          //本地有输入有，对本地音量压制的百分比，0~100

        public byte DelayTimeForResume;     //本地输入恢复空闲后，延时多少时间恢复正常音量。

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] _TargetIpForBtn2;     //本地组播按键按下后，向那个设备广播，填入单播，组播，广播ip；

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public byte[] _TargetIpForBtn4;     //本地紧急按键按下后，向那些设备广播，填入单播，组播，广播ip；

        public byte Input2RecordPriority;   //本地组播优先级

        public byte _VolumeForBtn2;

        public byte _InputForBtn2;

        public byte _SamplerateForBtn2;

        public byte _GainForBtn2;

        public byte Input3RecordPriority;//本地组播优先级

        public byte _VolumeForBtn4;

        public byte _InputForBtn4;

        public byte _SamplerateForBtn4;

        public byte _GainForBtn4;

        public byte TalkRingVolume;     //对讲叮咚音量

        public byte RecordRingVolume;   //录播叮咚音量

        public byte TalkendRingCount;   //对讲结束播放嘟嘟音次数

        public byte RecordMicGain;

        public UInt16 UnlinkReset;

        public UInt16 MicDelayTime;

        public UInt16 CloudServerPort;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] CloudServerAddress;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 576)]
        public byte[] unuse403;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public byte[] Code;
    }

namespace NASetupDLL
{
    public unsafe class NaSetup
    {
        public const Int32 NP_SUCCESS = 1;		//	函数操作成功，正确返回
        public const Int32 NP_NET_INTERFACE_ERROR = -1;		//	网络操所错误
        public const Int32 NP_DATA_ERROR = -2;		//	返回不期望的数据，操作不成功
        public const Int32 NP_DEVICE_NOT_EXIST = -3;		//	无数据返回，无法联系设备
        public const Int32 NP_WP_ERROR = -4;		//	参数保护，写错误
        public const Int32 NP_PARAM_ERROR = -5;		//	入口参数错误
        public const Int32 NP_PASSWORD_ERROR = -6;		//	密码错误
        public static string IpToStr(byte[] ip)
        {
            int i;
            string strIp = "";
            for (i = 0; i < 3; i++)
            {
                strIp += ip[i].ToString() + '.';
            }
            strIp += ip[3].ToString();
            return strIp;
        }

        //将一个字符串转换成指定长度的byte数组
        public static byte[] CodeBytes(string str, int len)
        {
            if (string.IsNullOrEmpty(str))
            {
                str = string.Empty;
            }

            byte[] result = new byte[len];
            byte[] strBytes = Encoding.Default.GetBytes(str);

            for (int i = 0; i < len; i++)
            {
                
                if (i < strBytes.Length)
                    result[i] = strBytes[i];
                else
                    result[i] = 0;
            }

            return result;
        }

        //将一个0.9.245.1.2.3格式的字符串转换为{0,9,245,1,2,3}的数组，如果字符串格式错误，返回false
        public static bool strMacToArray(string Mac, ref byte[] a)
        {
            char[] delimit = new char[] {'.'};
            bool flag = true;

            string[] words = Mac.Split(delimit);
            if (words.Length == 6)
            {
                for (int i = 0; i < 6; i++)
                {
                    try
                    {
                        a[i] = Convert.ToByte(words[i]);
                    }
                    catch
                    {
                        flag = false;
                    }
                }          
            }
            return flag;

        }

        public static UInt16 Swap(UInt16 num)
        {
            byte[] revrNum = BitConverter.GetBytes(num);
            Array.Reverse(revrNum);

            return BitConverter.ToUInt16(revrNum, 0);
        }

        //对np_search_all进行封装
        public static int NpSearchAll(ref _DeviceInfo[] DeviceInfoArray, int Size)
        {
            int result = 0;
            int typesize = Marshal.SizeOf(typeof(_DeviceInfo));
            byte* pInfo = stackalloc byte[Size * typesize];
            int Number = Size;
            int res = NASetupDLL.NaSetup.np_search_all(pInfo, &Number);
            if (res == NP_SUCCESS)
            {
                for (int i = 0; i < Number; i++)
                {
                    _DeviceInfo DeviceInfo = (_DeviceInfo)Marshal.PtrToStructure(new IntPtr(pInfo + i * typesize), typeof(_DeviceInfo));
                    DeviceInfoArray[i] = DeviceInfo;
                }
                result = Number;
            }
            else
            {
                result = 0;
            }
            return result;
        }

        //对动态库np_login的封装
        public static int NpLogIn(string strIp, string strMac, ref _Param Param)
        {
            int ParamSize = Marshal.SizeOf(typeof(_Param));
            byte* pParam = stackalloc byte[ParamSize];
            byte* ip = stackalloc byte[16];
            byte* mac = stackalloc byte[6];
            byte[] bys = System.Text.Encoding.Default.GetBytes(strIp);
            if (bys.Length < 16)
            {
                Marshal.Copy(bys, 0, (IntPtr)ip, bys.Length);
                ip[bys.Length] = 0;
            }

            byte[] bys2 = new byte[6];
            if (strMacToArray(strMac, ref bys2))
            {
                Marshal.Copy(bys2, 0, (IntPtr)mac, 6);
            }
            Marshal.StructureToPtr(Param, (IntPtr)pParam, true);

            int res = np_login(ip, mac, pParam);

            return res;
        }

        //对动态库np_logout的封装
        public static int NpLogOut(string strIp, string strMac, ref _Param Param)
        {
            int ParamSize = Marshal.SizeOf(typeof(_Param));
            byte* pParam = stackalloc byte[ParamSize];
            byte* ip = stackalloc byte[16];
            byte* mac = stackalloc byte[6];
            byte[] bys = System.Text.Encoding.Default.GetBytes(strIp);
            if (bys.Length < 16)
            {
                Marshal.Copy(bys, 0, (IntPtr)ip, bys.Length);
                ip[bys.Length] = 0;
            }

            byte[] bys2 = new byte[6];
            if (strMacToArray(strMac, ref bys2))
            {
                Marshal.Copy(bys2, 0, (IntPtr)mac, 6);
            }
            Marshal.StructureToPtr(Param, (IntPtr)pParam, true);

            int res = np_logout(ip, mac, pParam);

            return res;
        }

        //读取音频设备的参数
        public static int NpReadSettingEx(string strIp, string strMac, ref _DeviceEx2 DeviceEx, ref _Param Param)
        {
            //_Port Port = new _Port();
            int DeviceExSize = Marshal.SizeOf(DeviceEx);
            int ParamSize = Marshal.SizeOf(Param);

            //int PortSize = Marshal.SizeOf(typeof(_Port));
            //if (PortSize != 68) return 0;

            byte* pDeviceEx = stackalloc byte[2000];
            byte* pParam = stackalloc byte[ParamSize];
            byte* ip = stackalloc byte[16];
            byte* mac = stackalloc byte[6];

            byte[] bys = System.Text.Encoding.Default.GetBytes(strIp);
            if (bys.Length < 16)
            {
                Marshal.Copy(bys,0,(IntPtr)ip,bys.Length);
                ip[bys.Length] = 0;
            }
            
            byte[] bys2 = new byte[6];
            if (strMacToArray(strMac, ref bys2))
            {
                Marshal.Copy(bys2, 0, (IntPtr)mac, 6);
            }

            Marshal.StructureToPtr(DeviceEx, (IntPtr)pDeviceEx,true);

            Marshal.StructureToPtr(Param, (IntPtr)pParam, true);

            int res = np_read_setting2(ip, mac, pDeviceEx, pParam);
            if (res == 1)
            { 
                //读回正确的数据结构
                //DeviceEx = (_DeviceEx2)Marshal.PtrToStructure((IntPtr)pDeviceEx, typeof(_DeviceEx2));
            }
            
            return res;
        }

        //写音频设备的参数
        public static int NpWriteSettingEx(string strIp, string strMac, ref _DeviceEx2 DeviceEx, ref _Param Param)
        {

            int DeviceExSize = Marshal.SizeOf(DeviceEx);
            int ParamSize = Marshal.SizeOf(Param);

            byte* pDeviceEx = stackalloc byte[DeviceExSize];
            byte* pParam = stackalloc byte[ParamSize];
            byte* ip = stackalloc byte[16];
            byte* mac = stackalloc byte[6];

            byte[] bys = System.Text.Encoding.Default.GetBytes(strIp);
            if (bys.Length < 16)
            {
                Marshal.Copy(bys, 0, (IntPtr)ip, bys.Length);
                ip[bys.Length] = 0;
            }

            byte[] bys2 = new byte[6];
            if (strMacToArray(strMac, ref bys2))
            {
                Marshal.Copy(bys2, 0, (IntPtr)mac, 6);
            }

            Marshal.StructureToPtr(DeviceEx, (IntPtr)pDeviceEx, true);

            Marshal.StructureToPtr(Param, (IntPtr)pParam, true);

            int res = np_write_setting2(ip, mac, pDeviceEx, DeviceExSize, pParam);

            return res;

        }

        /***********************************************************************************
        函数名称：	np_search_all
        函数功能：	在本地网内搜索NP系列设备
        输入参数：	struct _DeviceInfo * devs	系统参数结构数组指针，用户在调用这个函数前应为这个结构分配空间。
                                        分配空间不小于Number * sizeof(struct _DeviceInfo);
                                        搜索到的设备参数写入这个结构中。
                    int * number		此指针传入系统开辟的缓存大小，（也就是可以容纳多少个设备的信息），返回搜索到的转换器个数。
        返回值  ：	1：					表示操作成功
                    其它值：			表示操作出错，返回值请参考函数返回值宏定义。
        其他说明：	
        ***********************************************************************************/
        [DllImport("NASetup.dll")]
        public static extern int np_search_all(byte* pDevInfo,int* pNumber);

        /***********************************************************************************
        函数名称：	np_search_one
        函数功能：	搜索指定IP的NP设备，成功则将该设备得系统参数填入dev结构中
        输入参数：	char * ip		目标转换器得IP地址。ip为以'\0'结尾的字符串。
                    struct _DeviceInfo * devs	系统参数结构指针，用户在调用这个函数前应为这个结构分配空间。
                                        搜索到的设备参数写入这个结构中。
        返回值  ：	1：					表示操作成功
                    其它值：			表示操作出错，返回值请参考函数返回值宏定义。
        其他说明：	使用IP地址进行设备搜索时，应该注意，目标转换器的IP地址不属本网段而其物理连接实际在本网段的，使用该命令将搜索不到。
        ***********************************************************************************/
        [DllImport("NASetup.dll")]
        public static extern int np_search_one(byte* ip,byte* pDevInfo);

        /***********************************************************************************
        函数名称：	np_login
        函数功能：	在目标设备上进行登录操作，只有登录操作后，才能使用np_read_setting和np_write_setting这两个命令。
        输入参数：	char* ip			目标转换器的ip地址。形如"192.168.0.2",以'\0'结尾的字符串。
                    char *mac			目标转换器的MAC地址。6个byte长度的数组指针。
                    struct _Param* Param	调用函数前，用户应填充Param各成员。
        返回值  ：	1：					表示操作成功
                    其它值：			表示操作出错，返回值请参考函数返回值宏定义。
        其他说明：	函数登录指定ip地址的设备，如果ip地址为空，则登录在本网段符合指定mac地址的设备。ip或mac应至少有一个参数是有效的。
        ***********************************************************************************/
        [DllImport("nasetup.dll")]
        public static extern int np_login(byte* ip, byte* mac, byte* param);

        /***********************************************************************************
        函数名称：	np_logout
        函数功能：	在目标设备上进行登出操作，使用np_login命令并操作完成后，应进行登出操作。
        输入参数：	char* ip			目标转换器的ip地址。形如"192.168.0.2",以'\0'结尾的字符串。
                    char *mac			目标转换器的MAC地址。6个byte长度的数组指针。
                    struct _Param* Param	调用函数前，用户应填充Param的DeviceType。
        返回值  ：	1：					表示操作成功
                    其它值：			表示操作出错，返回值请参考函数返回值宏定义。
        其他说明：	函数登出指定ip地址的设备，如果ip地址为空，则登出在本网段符合指定mac地址的设备。ip或mac应至少有一个参数是有效的。
        ***********************************************************************************/
        [DllImport("nasetup.dll")]
        public static extern int np_logout(byte* ip, byte* mac, byte* param);

        /***********************************************************************************
        函数名称：	np_read_setting
        函数功能：	读取指定IP或Mac地址的NP设备系统参数
        输入参数：	char* ip			目标转换器的ip地址。形如"192.168.0.2",以'\0'结尾的字符串。
                    char *mac			目标转换器的MAC地址。6个byte长度的数组指针。
                    void* dev			系统参数缓存指针，用户在调用这个函数前应为这个缓存分配空间，缓存不小于sizeof(struct _DeviceEx);
                                        读取的设备参数写入到这个缓冲中。
                                        函数详细说明及调用实例请参考动态库使用说明。

                    struct _Param	Param	搜索参数选项，调用函数前，用户应填充Param各成员。
        返回值  ：	1：					表示操作成功。
                    其它值：			表示操作出错，返回值请参考函数返回值宏定义。
        其他说明：	函数读取指定ip地址的设备参数，如果ip地址为空，函数在本网段读取符合指定mac地址设备的参数。ip或mac应至少有一个参数是有效的。
        ***********************************************************************************/
        [DllImport("nasetup.dll")]
        public static extern int np_read_setting(byte* ip,byte* mac,byte* sys,byte* param);

        [DllImport("nasetup.dll")]
        public static extern int np_read_setting2(byte* ip, byte* mac, byte* sys, byte* param);

        /***********************************************************************************
        函数名称：	np_write_setting
        函数功能：	设置指定指定IP或Mac地址的NP设备系统参数
        输入参数：	char* ip			目标转换器的ip地址。以'\0'结尾的字符串。
                    char *mac			目标转换器的MAC地址。6个byte长度的数组指针。
                    void* dev			系统参数缓存，用户在调用这个函数前应为这个结构分配空间，读取的设备参数填入这个缓存中。
                                        函数详细说明及调用实例请参考动态库使用说明。
                    struct _Param*	Param	搜索参数选项，调用函数前，用户应填充Param各成员。
        返回值  ：	1：					表示操作成功
                    其它值：			表示操作出错，返回值请参考函数返回值宏定义。
        其他说明：	函数设置指定ip地址的设备参数，如果ip地址为空，函数在本网段设置符合指定mac地址设备的参数。ip或mac应至少有一个参数是有效的。
        ***********************************************************************************/
        [DllImport("nasetup.dll")]
        public static extern int np_write_setting(byte* ip, byte* mac, byte* sys, int length, byte* param);

        [DllImport("nasetup.dll")]
        public static extern int np_write_setting2(byte* ip, byte* mac, byte* sys, int length, byte* param);
    }

}