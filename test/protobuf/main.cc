#include "test.pb.h"
#include<iostream>

using namespace fixbug;

//protobuf处理普通的单个对象
int main1() {
    //封装了login请求对象的数据
    LoginRequest req;
    req.set_name("zhang san");
    req.set_pwd("123456");

    // 对象数据序列化 =》 string/char*
    std::string send_str;
    if (req.SerializeToString(&send_str)) {
        std::cout << send_str << std::endl;
    }

    //从send_str反序列化一个login请求对象
    LoginRequest reqB;
    if (reqB.ParseFromString(send_str)) {
        std::cout << reqB.name() << std::endl;
        std::cout << reqB.pwd() << std::endl;
    }

    return 0;
}


//protobuf处理 对象里有对象 以及列表类型
int main() {
    // LoginResponse rsp;
    // ResultCode *rc = rsp.mutable_result();
    // rc->set_errcode(1);
    // rc->set_errmsg("登录处理失败了");

    GetFriendListsResponse rsp;
    ResultCode *rc = rsp.mutable_result();
    rc->set_errcode(0);

    User *user1 = rsp.add_friend_list();
    user1->set_name("zhang san");
    user1->set_age(20);
    user1->set_sex(User::Man);

    User *user2 = rsp.add_friend_list();
    user2->set_name("li si");
    user2->set_age(23);
    user2->set_sex(User::Man);

    std::cout << rsp.friend_list_size() << std::endl;  //列表个数

    

    return 0;
}