#include "mprpcapplication.h"
#include<iostream>
#include<unistd.h>
#include<string>

MprpcConfig MprpcApplication::m_config;

void ShowArgsHelp() {
    std::cout << "format: command -i <configfile>" << std::endl;
}

//类外实现静态方法不用带static
void MprpcApplication::Init(int argc, char **argv) {
    if (argc < 2) {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int c = 0;
    std::string config_file;
    //man getopt 寻找参数i :表示必须有
    while ((c = getopt(argc, argv, "i:")) != -1) {   //注意不要写错！！！！ 
        switch (c) {
            case 'i':
                config_file = optarg;
                break;
            case '?':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            case ':':
                ShowArgsHelp();
                exit(EXIT_FAILURE);
            default:
                break;

        }
    }


    //开始加载配置文件 rpcserver_ip= rpcserver_port=  zookeeper_ip= zookeeper_port=
    m_config.LoadConfigFile(config_file.c_str());

    // std::cout << "rpcserverip: " << m_config.Load("rpcserverip") << std::endl;
    // std::cout << "rpcserverport: " << m_config.Load("rpcserverport") << std::endl;
    // std::cout << "zookeeperip: " << m_config.Load("zookeeperip") << std::endl;
    // std::cout << "zookeeperport: " << m_config.Load("zookeeperport") << std::endl;    
}

MprpcApplication& MprpcApplication::GetInstance() {
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig() {
    return m_config;
}