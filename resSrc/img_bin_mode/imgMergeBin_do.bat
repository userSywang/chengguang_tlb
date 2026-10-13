
:: boll ESP32 图片资源转換(输出为资源文件形式) 
:: img_merge  listFile    prefix
:: Note: 这里 prefix 是指输出的 .h 的每一图片号宏定义的前缀, 建议用模块名的缩写. 
::     输出在 output 文件夹 
::     清单文件里每行指定相应的图片文件(相对路径) (这些图片是否分子目录不做要求) 
@pause

IF not exist  .\\pngquant.exe ( copy  ..\\tools\\pngquant.exe  . 
) 

IF exist .\\img_merge.exe    ( set MY_TOOL_PATH=.
) ELSE ( set MY_TOOL_PATH=%BOLL_TLB_TOOL_PATH%)

call %MY_TOOL_PATH%\\img_merge_bin_group.exe  img_a_tinglib.lst  img_a_tlb.bin

@pause

