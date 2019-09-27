	DLL Demo的说明和使用
	
    Demo程序使用8877端口调用SR_Init，固定在此端口监听，接受设备的连接请求。
    所有的设备都必须先进行注册，（对于没有注册的设备，DLL会拒绝连接请求。）在register按钮左边的文本框填入设备IP地址，然后点击register，成功注册的设备会现在程序左边的列表框中。状态灯显示为绿色。

    	AudioDemo_Play_DLL
    普通播放，是通过TCP进行音频数据的传输，当前DLL仅支持MP3文件播放，选择UseID和一个本地MP3文件后，点击《start》开始播放。设备端可以支持多种编码格式的数据，这需要DLL未来的升级才能支持。

    紧急播放：与普通播放操作一样，唯一区别是仅支持编码格式为0x0001，单声道，16K采样，每采样16bits的wav文件。如需要支持其他格式，需要应用程序进行对音频进行解码。
    普遍播放和紧急播放放在一个Demo中，用户可以测试在普通播放时，启动紧急播放可以打断普通播放的节目。

	AudioDemo_Intercom_DLL
    会话Demo是启动计算机的声卡，与设备进行实时，双向的语音传输。声卡操作封装在SoundCardDll动态库，默认16K采样率，当前对讲的参数大多已经在DLL中固定，未来DLL版本使用动态库的lpInputParam参数，将会话所使用的参数传给DLL内部。
   	
	AudioDemo_SDCard_DLL
    SR设备支持Class 10 TFT卡，在8G，32G都可以正常使用。只能操作SD卡的user目录下文件。支持长文件名，但当前设备仅支持英文数字组成的文件，暂不支持中文。
    get disk info 获取SD卡容量信息和user目录下文明信息。
    upload file，将一个本地文件上传到设备的sd卡目录中。
    delete file  删除sd卡上的文件。
    start 播放sd卡的mp3和wav文件。