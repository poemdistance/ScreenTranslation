# 一. 程序功能
## (一). 目前功能
1. 屏幕取词翻译 -- 英译中
   
   ![img](./gif_pic/iconEntry.png)<br><br>
   ![img](./gif_pic/GUI.png)

## (二). 后期预计完成功能
1. 中英互译
2. 单词收藏
3. 离线翻译
 
 <br> 

# 二. 程序编译安装
1. 先将源码克隆到本地<br><br>
2. cd到src目录<br><br>
3. 安装依赖  Xlib X11 Xtst Xext gtk3开发环境(gtk.h, gtkwindow.h)
 
             # For system base on Debian 
          $  sudo apt-get install build-essential gnome-devel libx11-dev libxtst-dev

             # For Arch Linux
          $  sudo pacman -S gtk3  libxtst libx11      


5. 终端执行命令 
   
          sudo sh prepare.sh && make
          sudo cp stran /usr/bin/stran -v
           
<br> 

# 三. 程序运行与停止
1. 运行**先决条件** <br>
     * **安装这个项目 https://github.com/poemdistance/google-translate 上的在线翻译程序**，其安装使用等相关事项见项目根目录的README. 确保此翻译程序能正常运行。<br><br>
     * **NOTE**: **电脑如果安装了wayland，需要禁用**，不然终端中无法正常使用xdotool,方法如下：<br>
        *  打开/etc/gdm/custom.conf
           添加如下两行(完成后需要重启)
           ```
           [daemon]
           WaylandEnable=false
           ``` 
     * **相关依赖**： 
         1. C语言库: Xlib X11 Xtst Xext gtk3开发环境 (不知道有没有漏掉什么...,等我到其他机器测试完发现有遗漏再来补充)
         2. 终端命令行工具: xdotool, ps, awk, tail 
   
     * **使程序在终端正常运行请完成如下步骤:**<br>
       * 终端执行 ps -p \`xdotool getwindowfocus getwindowpid\` |awk '{print $NF}' | tail -n 1 

          将得到的终端应用名添加到**forDetectMouse.c**的**termName数组**中并拓展数组容量，否则在监测终端复制文字的时候发送的是Ctrl-C，而不是真正的复制快捷键Ctrl-Shift-C。 ( terminator, gnome-terminal以及konsol已经默认添加进了数组). <br><br>
          ![img](./gif_pic/termName.png) 
     
     * **与截图软件共存的问题**<br>
         * 截图软件在进行区域选择的时候，同样会被程序捕捉到，这个时候如果发送Ctrl-C可能导致截图软件异常退出，程序在进行剪贴板内容获取的时候也会发生段错误，此时打印信息中已经包含了截图软件名称，可以将此名称添加到forDetectMouse的screenShotApp数组中，将程序重新编译安装运行即可(即二中的步骤4，但是可以不再运行`prepare.sh`)

          数组如下(已经默认添加flameshot截图软件):<br><br> 
          ![img](./gif_pic/screenShotappArray.png)
           

2. **如果终端使用了Smart copy，在没有选中任何文字的时候，可能会使模拟发送的Ctrl-Shift-C被终端视为Ctrl-C而导致运行中的程序意外结束，这不是本程序的Bug，可以将Smart Copy关闭防止此类危险情况发生**

3. 直接运行编译后生成的可执行文件stran 
 
        $  stran 
    
      这种情况会有很多输出信息，一般是拿来作为调试信息的<br><br>

        $  stran > /dev/null &
  
     放置在后台执行，重定向输出到/dev/null <br> <br>

     停止运行:   

        $ kill `ps -aux | grep stran | head -n 1 |awk '{print $2}'|xargs` 


<br> 

# 四. 运行效果图示 
* [x] 终端演示，用双击以及区域选择进行取词翻译<br><br>
![gif](./gif_pic/1.gif) <br><br>

* [x] 浏览器演示，用三击取段，单击取词以及区域选择取句翻译

![gif](./gif_pic/2.gif) 

<br>

# 五. 程序目前已知问题

- [ ] 由于线程是轮流执行的，进行双击时可能文本已经通过管道传递给Python并完成翻译写入了共享内存，但由于在进入到鼠标动作判断之前，鼠标的双击动作已经由于鼠标移动被刷新掉了，导致入口图标未被创建无法进行翻译界面显示<br><br>
- [ ] 三击动作前为双击，双击选中的文本也会被复制通过管道传递给Python进行翻译，导致先取得的翻译结果是双击的，而非三击取得的一整段或一整句。<br><br> 
- [ ] 程序虽然排除了部分选中为空的情况进行不显示入口图标操作，但是并未排除窗口拖拽导致的被识别为区域选择事件进行入口图标创建的操作，中途其他操作都有一定几率被误识别。不过程序有超时自动销毁图标以及创建图标后进行图标外单击销毁图标的功能(这个问题触发几率比较大)。 

- [ ] 程序创建的共享内存在退出后还没有被处理，后期会解决。
- [ ] Python翻译程序如果异常退出，C语言程序会继续运行但不会得到任何翻译结果，可以重启程序回到正轨
- [ ] **程序只支持Xorg的桌面，目前只在Arch Linux上的 Gnome + Xorg 桌面测试通过，并且系统必须禁用Wayland。** 

<br> 

## 其实以上问题除了最后一个都想到了解决办法，之后会一一处理，这里提出是为了让有兴趣自己独立完善程序的人有方向的进行。  

<br> 

# 六. 程序运行异常问题

1. 如果程序提取到用鼠标获取的结果一直是同一个或者为空，但是用键盘Ctrl-c操作能够获取到对应新的文本,换而言之就是鼠标取词不起作用，这说明打开的键盘设备文件是错的，模拟发送的Ctrl-c不会被捕捉到，此时可以用如下命令得到真正的键盘设备: <br> 
    
         cat  /var/log/Xorg.xx.log | grep keyboard | grep event | tail -n 1  

    (请将Xorg.xx.log替代为你当前系统的实际日志文件)，其输出类似:<br> 

        [275.556] (II) event3  - AT Translated Set 2 keyboard: device removed 

    可以看到，当前我系统使用的键盘设备文件是`/dev/input/event3`<br> 
    当然，我们也可以用其他方法来查明。

    **最后找到DetectMouse.c中/dev/input/eventX这条语句，将eventX修改成你实际得到的结果重新编译运行即可**
  
   
2. 如果**运行报错failed to open mice的问题**，这个是因为没有权限打开文件进行读写导致的，可以有如下解决办法：    
    * **方法一: 添加当前用户到/dev/input/mice的用户组中**：<br>
        * a. 先查明此文件设备所在用户组,使用命令:<br> 
  
                ls -l /dev/input/mic

           b. 得到结果类似如下:<br>  

                crw-rw---- 1 root input 13, 63 Jul 27 09:09 /dev/input/mice

           c. 其中的input即是其所在用户组，得到后使用如下命令**添加用户到input用户组并重启系统**:<br> 

                sudo usermod -aG input userName
            
            **Note**: **如果设备文件没有用户组，请先手动设置规则添加，网上很多相关内容，这里不赘述.** <br><br>

    * **方法二: 使用sudo执行此程序**: <br> 
     
           sudo ./main 

         中途测试过程中以root用户或者说sudo执行时Xdisplay发生过 No protocol specified的错误，所以此方法不一定奏效，但也可能是当时系统忘记关闭Wayland导致的。

<br>

# 七. 关于UI不太赏心悦目的问题 
* 由于欠缺一丢丢美工的能力，界面，呃... 

<br>
