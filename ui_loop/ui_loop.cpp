#include <string.h>
#include <iostream>
#include<typeinfo>

/*
client操作命令：
send -t   ：测试一组tcp延迟（一组30个）
send -t 20：测试20组tcp
send -u   ：测试一组udp延迟（一组30个）
shut      ：终止程序

*/
struct Con_data
{
    int if_tcp ;//1为udp，0为tcp，默认为0
    int times ;//测试的组数，默认为1
    int if_send ;//保留
    int shut;//默认为0
};
int Read_once(char*buf) {
    if (buf[0]!='\0')//先清空
    {
        memset(buf, 0, sizeof(buf));
    }
    scanf("%[^\n]",buf);//https://blog.csdn.net/weixin_43534086/article/details/104335865
    //printf("%s\n", buf);
    return(0);

};

int Analysis_txt(char* buf, Con_data* con_data) {
    const char sp[2] = " ";
    char* token;
    token = strtok(buf, sp);//先取一条
    //printf("%s\n", token);
    if (!strcmp(token, "send"))
        (*con_data).if_send = 1;
    else if (!strcmp(token, "shut")) {
        con_data->shut = 1;
        return(0);
    }
    while (token!=NULL)
    {
        
        token = strtok(NULL,sp);
        //printf("%s\n", token);
        if (token == NULL)return(0);
        if (!strcmp(token, "-t"))
            (*con_data).if_tcp = 1; 
        if (!strcmp(token, "-u"))
            (*con_data).if_tcp = 0;

        if (token[0] >= '0' && token[0] <= '9')//判断是否为数字
            (*con_data).times=atoi(token);
        
    }
    return(0);
};

int Do_something() {return(0);}
int main()
{
    
    char read_buf[128];
    Con_data* con_data;
    con_data = (Con_data*)malloc(sizeof(Con_data));//控制参数们的结构体


    *con_data = { 0,1,1,0 };
    Read_once(read_buf);//读取一次输入
    if (strcmp(read_buf, "tcp"))
        (*con_data).if_tcp = 1;
    getchar();

    Do_something();
    while (true){
        
        Read_once(read_buf);//读取一次输入
        //printf("I have read \"%s\"\n", read_buf);
        
        Analysis_txt(read_buf,con_data);//对输入进行分析并修改con_data控制结构体
        printf("if_send is %d,if_tcp is %d,send_times is %d\n", con_data->if_send, con_data->if_tcp, con_data->times);



        if (con_data->shut == 1)break;//输入'shut'后要做的事情
        Do_something();

        getchar();
    }
    


    
    
    free(con_data);
    return(0);
}

