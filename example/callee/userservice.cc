#include<iostream>
#include<string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
/*
userservice 原来是一个本地服务, 提供了两个进程内的本地方法， Login 和 GetFriendLists
*/

//服务器端程序

//UserService从UserServiceRpc继承而来   重写Login方法
class UserService : public fixbug::UserServiceRpc{  //使用在RPC服务的发布端（rpc服务提供者）   ctrl点击UserServiceRpc查看重载的虚函数方法
public: 
    bool Login(std::string name, std::string pwd) {
        std::cout << "doing local service: Login" << std::endl;
        std::cout << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }

    //本地业务   部署成 远程rpc服务
    bool Register(uint32_t id, std::string name, std::string pwd) {
        std::cout << "doing local service: Register" << std::endl;
        std::cout << "id: " << id << "name: " << name << " pwd: " << pwd << std::endl;
        return true;
    }
    /*
    1. caller ===> Login(LoginRequest) （写一个proto文件 把方法名字 参数名写进去） => muduo => callee
    2. callee ===> Login(LoginRequest)  => 交到下面重写的这个Login方法上了
    */
    //重写基类 UserServiceRpc 的虚函数方法   下面这些方法都是框架直接提供的   接收远程发送的rpc请求 到 服务发布  响应后再给框架
    void Login(::google::protobuf::RpcController* controller,
                        const ::fixbug::LoginRequest* request,
                        ::fixbug::LoginResponse* response,
                        ::google::protobuf::Closure* done) {
        // 框架给业务上报了请求参数LoginRequest， 应用获取相应数据做本地业务  request 携带了 反序列化好的数据 name  pwd
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_result = Login(name, pwd);   //做本地业务

        // 把响应写入  包括错误码  错误消息  返回值
        fixbug::ResultCode* code = response->mutable_result();
        code->set_errcode(0);  //设置为1   登陆失败
        code->set_errmsg("");  // Login error

        response->set_success(login_result);

        //执行回调操作  执行响应对象数据的序列化和网络发送（都是由框架完成的）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done) {
        // 第一步
        //框架从网络上接收到请求的数据  并且反序列化 Registerrequest对象   直接获取对应参数
        uint32_t id = request->id();  //直接取数据
        std::string name = request->name();
        std::string pwd = request->pwd();


        //第二步  做本地业务
        bool ret = Register(id, name ,pwd);  // 将上一步得到的三个参数   进行传参传到本地业务中去

        //第三步  响应  数据进行序列化  并通过网络发回客户端
        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        //第四步 执行回调
        done->Run();
    }

};

int main(int argc, char **argv) {

    /*
    UserService us;
    us.Login("zhang san", "123456"); // 这样在一台机器同一个进程进行调用没有问题  在其他机器上如何调用  或者  在同一台机器上不同进程如何调用
    */

    //调用框架的初始化操作   provider -i config.conf
    MprpcApplication::Init(argc, argv);

    //provider是一个rpc网络服务对象    把UserService对象发布到rpc节点上
    RpcProvider provider;
    provider.NotifyService(new UserService());

    //启动一个rpc服务发布节点   run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}