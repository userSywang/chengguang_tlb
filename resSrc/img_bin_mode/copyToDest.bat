
@echo will copy res and .h to dest ...
@pause
@IF not exist  ..\..\diskData\A\sysData\img ( mkdir ..\..\diskData\A\sysData\img 
)
copy /y .\destin\img_a_tlb.bin  ..\..\diskData\A\sysData\img\
copy /y .\destin\img_a_tlb.h    ..\..\LvR9\main\include\

@pause
