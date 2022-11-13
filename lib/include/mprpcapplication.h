#pragma once
#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"


//mprpc框架的初始化基础类
class MprpcApplication {
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig m_config; // Init为静态的成员方法  然后调用了一个普通的成员变量  这是不允许的！！！

    MprpcApplication() {}  //设置为单例模式
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};

/*
设置为单例模式
1.构造函数私有化
2.static MprpcApplication& GetInstance
3.定义static 对象 static MprpcApplication instance
4.return
*/