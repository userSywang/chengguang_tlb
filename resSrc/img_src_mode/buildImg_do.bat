:: 主界面图标(src 模式)生成 
@echo will combine image src ...
@pause

IF not exist  .\pngquant.exe ( copy  ..\tools\pngquant.exe  . 
) 

IF exist .\img_merge.exe    ( set MY_TOOL_PATH=.
) ELSE ( set MY_TOOL_PATH=%BOLL_TLB_TOOL_PATH%)


call %BOLL_TLB_TOOL_PATH%\img_merge.exe mmnu_img.cfg  MMN

@pause
