
[Tinglib_xsz 项目基本说明]:

.源码默认使用 utf-8 编码;

.该产品选用的 MCU 是 rv1106,  128M RAM;  linux 系统;

.屏的分辨率: 320宽、480高 (见 SCREEN_WIDTH、SCREEN_HEIGHT);

.产品共 8 个键:
  LV_KEY_ESC、 LV_KEY_EX_VOLUME_INC、LV_KEY_EX_VOLUME_DEC、LV_KEY_EX_POWER ... 

.我们的仿真工程必须要运行在 x86 模式下，不然传真的效果受较大影响; 

.申请内存, 一般情况下建议用  heap_caps_malloc_dflt()来代替 malloc(), 用 heap_caps_realloc_dflt() 来代替 realloc();
  (LVGL 库内部仍保持用 lv_mem_alloc()、lv_mem_realloc()、lv_mem_free());

.LVGL库的三种界面布局(layout):
   缺省布局(这里建议各组件用 align 方式)、grid 布局、 flex 柔性布局(或称流式布局); 

[常用组件]
.obj (可作为容器，或作为单个组件，其他组件都继承自它);
.label (文本框);
.img  (按钮、图片);
.btn (多用于作文字按钮);
.bar (进度指示条);
.slide (滑动条);
.checkbox (复选框);
.switch (开关);
.dropdown (像 win32 那样的组合框);
.roller  (滑动选择器);
.btnmatrix  (按钮阵列, 每行按钮可为任意个);
.spangroup (简单的富文本, 每项可单独指定是否有下划数、文字颜色 等); 
.msgbox (消息框);
.list (列表框);
.table (表格，多用于两列以上的);
.animimg (动态图);
   Note: 不推荐用 imgbtn;  不推荐 menu;

.组件的属性(style、prop)是组件特性的描述, 版面创建后轻易不变(比如 CHECKED 状态下用什么背景色/背景图, FOCUS_KEY 状态该用什么 BORDER_OPA .... 等等); 
 组件的标记(flag)也是相对稳定的(比如 LV_OBJ_FLAG_CLICKABLE、LV_OBJ_FLAG_CHECKABLE、LV_OBJ_FLAG_SCROLLABLE、LV_OBJ_FLAG_HIDDEN); 
 而组件的状态(state) 往往是易变的(比如 LV_STATE_CHECKED、LV_STATE_FOCUSED)，我们版面上的变化 实质是反应的组件内部 state 的变化; 


.区分是否仿真工程 用 C_PC_SIM (PC 仿真里 该宏是定义了的, 该宏多用于发音、录音、网络等);

.调用时 lv_obj_set_style_XXX() 时，  (第3个)参数 selector 最多只能包含一个 LV_PART_XXX ; 

.文件系统操作可用 zb_fopen()、zb_fread()、zb_fwrite() ... , 对于 zb_fopen() 现在支持的路径为根路径为 /userdata/ (linux风格，无盘符), 在仿真工程里对应的路径为 diskData/A/, 仿真工程里 diskData/A/ 文件夹对应TF卡内容;
.该平台文件路径里各级文件夹之间的分隔符推荐是 / (正斜杠);

.打日志: 对于 LVGL 库内部一般用 LV_LOG_WARN()、LV_LOG_ERROR()、LV_LOG_USER()， 它们会自动带上 时刻、函数名、文件名、行号 等信息; 
  对于其他地方(比如应用), log 可用 ESP_LOGI(tag, fmt, ...)、ESP_LOGW(tag, fmt, ...)、ESP_LOGE(tag, fmt, ...)； 该方式带 tag  (没有了 LV_LOG_XXX 里的 文件名、函数名、行号); 日志字串尾不需要有换行符\n; 


.一般来说, demo、example、test_demo 文件夹下面的文件是样例代码，最终出货工程里 那里的代码有时就会被删除了(以节省 ROM 占用), 所以不应该调那里面的代码 (需要的话 一般会复制到其他文件夹下的源文件里);

.现在工程里, 源文件只有两个大文件夹，一个 components (LVGL 源码)、LvR9/main (我们项目的源码);
  (resSrc 是存放用于生成项目所需要(图片)资源的工作文件夹)

.我们现在的 LVGL, 按键事件如下:
  .LV_EVENT_KEY(刚按下) --- LV_EVENT_KEY_STAGE.KEY_LONG、LV_EVENT_KEY_STAGE.KEY_REPEAT(持续按下时每一定间隔时间发一次)  --- LV_EVENT_KEY_STAGE.KEY_SHORT(抬起时);
  .每一个键的按下抬起过程中, 其中 LV_EVENT_KEY_STAGE.KEY_LONG 和 .KEY_SHORT 两事件只会发出其一;
  .对于要对 短按、长按键 做不同响应处理的版面, 对相应键 请不要响应 LV_EVENT_KEY 事件(因为分不清具体是 长按还是短按), 应该响应 LV_EVENT_KEY_STAGE.LONG、LV_EVENT_KEY_STAGE.SHORT; 
   Note: 响应 LV_EVENT_KEY 和 LV_EVENT_KEY_STAGE.SHORT 两种处理的对比, 响应 LV_EVENT_KEY 事件时响应较快(键刚按下时就响应), 只是此响应动作里切换版面的话相对容易出现 bug ;
     (EVENT_KEY 送给了前一版面, 而 LV_EVENT_KEY_STAGE 送给了后一版面, 为避免此问题 可调用 lv_indev_keypad_wait_release()); 
     响应 LV_EVENT_KEY_STAGE.KEY_SHORT 的话不易出问题,不足之处是反应略慢(是在按键抬起时才响应的);


.LVGL 库 跟 win32 GUI 的不同之一:
  焦点管理是用输入组(group)来管理的，只有加入到输入组里的组件，才能获得焦点; 有的组件(比如 btn, 一般是有交互的)缺省会被加入组，有的不会(比如 label、img、imgbtn); 设置焦点用 lv_group_focus_obj(obj);
  组件没有静态的 id(只有句柄);
  按键消息是直接发给焦点组件的, 不经过主窗体;  为了满足我们自己产品需求, 加了主窗体拦截的功能 (sys_setCurMainWinObj());
  (不管有多少层版面)只有一级消息循环;
  对于组件, 只要子组件大于父组件，子组件默认就带了 scrollBar;


. LV_STATE_CHECKED、LV_STATE_FOCUS_KEY 两状态位 是最常用到的表示选中的状态位(LV_STATE_FOCUS_KEY 和 LV_STATE_FOCUSED 一般是同步加上或去掉的)。
他们的主要区别是, 一个输入组里 一堆组件(通常是兄弟组件)里，最多只有一个组件是 FOCUS_KEY 状态; 按ok键时, (没有主窗体拦截时)当前焦点组件会收到 LV_EVENT_CLICKED 事件;
  按 TAB 键的缺省处理, 是切换 FOCUSED 组件;  
   
 而 LV_STATE_CHECKED 没有此限制; 当然应用层可以自行实现一个组件加上 LV_STATE_CHECKED 状态位时，去除原有的 STATE_CHECKED 的兄弟组件 STATE_CHECKED 状态位.

-------------------------
-------------------------

.图标(图片)资源现有两种方式，一种是以 .c 源码方式(合成工具为 resSrc/img_src_mode)，一种图片资源文件方式(合成工具在 resSrc/img_bin_mode )   (这里不算 比如 听说 等专有格式数据里的图片); 
  其中源码方式的图片，会占用 RAM 空间, 所以图片量使用比较大的场景, 图片应使用 放入图片资源文件 的方式 来使用.

-------------------------
Note: 
.独立定义的 lv_style_t (非本地 style)一般应为 static 或全局变量，而不能为自动变量; 因为 obj 里是用指针引用的它(自动变量很快就被销毁了);

.图片合成工具, 图片合成工具 是放置到主工程之外的; 每人的电脑都需要设置一个 环境变量 BOLL_TLB_TOOL_PATH , 其值为图片合成工具的路径(即 img_merge.exe、img_merge_bin_group.exe 所在的文件夹的路径), 该路径末尾不需要反斜杠(\)字符;
.(对于小机模式编译)每人的电脑都需要一个 ADF_PATH 环境变量，指向 ADF 的路径;


.各 .c 里建议都定义名为 TAG 的宏(其值应为文件名的缩写), .h 里不可定义 TAG 宏(否则容易引起编译错误);



[建议]:
.版面设计时，一般尽量不要用大的尺寸的立即数, 以便程序在不同size屏的设备间移植。 用命名的宏常量是可以的, 比如 SCREEN_WIDTH; 大的长或宽 一般可用权重 或按比例来间接算得; 比如10、20 这样小值立即数是可以的;

.grid layou 里 pad 不起作用时, 检查调用 lv_obj_set_grid_cell()时, grid_algin 参数换成 LV_GRID_ALIGN_STRETCH 试试;

