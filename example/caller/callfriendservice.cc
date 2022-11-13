#include <iostream>
#include "mprpcapplication.h"
#include "friend.pb.h"


int main(int argc, char **argv) {
    //整个程序启动以后，想使用mprpc框架来享受rpc服务调用，一定需要先调用框架的初始化函数（只初始化一次）
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的rpc方法
    fixbug::FriendServiceRpc_Stub stub(new MprpcChannel());  

    //rpc方法的请求参数
    fixbug::GetFriendsListRequest request;
    request.set_userid(1000);

    //rpc方法的响应参数
    fixbug::GetFriendsListResponse response;

    //发起rpc方法的调用 同步的rpc的调用过程
    MprpcController controller;
    stub.GetFriendsList(&controller, &request, &response, nullptr);  // RpcChannel->RpcChannel::callMethod  集中来做所有rpc方法调用的参数序列化和网络发送   caller的黄色  绿色部分

    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
    } else {
        //  一次rpc调用完成，读调用的结果
        if (0 == response.result().errcode()) {
            std:: cout << " rpc GetFriendsList response success!" << std::endl;
            int size = response.friends_size();  // 在生成的friend.pb.h中查找对应方法

            for (int i = 0; i < size; ++i) {
                std::cout << "index: " << (i + 1) << response.friends(i) << std::endl;
            }
        } else {
            std::cout << " rpc GetFriendsList response error : " << response.result().errmsg() << std::endl;
        }
    }


    return 0;
}