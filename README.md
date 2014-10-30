
###说明    
src下包含两个程序，tickets和parse

* [tickets][1]    
* [parse][2]    

[1]: https://github.com/3null/the_double_chromosphere/blob/master/src/tickets.c "tickets.c"    
[2]: https://github.com/3null/the_double_chromosphere/blob/master/src/parse.c "parse.c"    


tickets随机产生福彩双色球彩票，使用方式如下：    
>Usage:    
>        ./tickets [All tickets] [Choice ticket]
    
随机产生[All tickets]张满足条件的彩票，并且从中随机选择[Choice ticket]张彩票，打印到屏幕。    
测试结果如下：    
![tickets](https://github.com/3null/the_double_chromosphere/blob/master/other/tickets_out.png)    


parse统计多期头奖中奖号码出现的次数，该程序从百度乐彩双色球中奖页面提取中奖号码，再统计各个号码出现的次数，打印到屏幕。    
该程序会调用wget，如果系统没有安装wget应用则无法使用。    
测试结果如下：    
![parse](https://github.com/3null/the_double_chromosphere/blob/master/other/parse_out.png)    

两个程序都是写着好玩的


