rem 指定存放文件的目录
set curdir=%cd%
for /R %curdir% %%s in (*) do (
	@echo %%s
)
@pause
