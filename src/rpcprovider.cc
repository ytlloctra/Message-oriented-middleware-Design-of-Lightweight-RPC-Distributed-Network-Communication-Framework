#include "rpcprovider.h"
#include "rpcheader.pb.h"

/*
service_name   =>   service描述
                            =》  service* 记录服务对象
                            method_name   =》 method方法对象

json 键值对 携带额外数据 文本存储  protobuf  二进制存储

protobuf除了序列化和反序列化   还提供了service类 method类  servicerpc方法的描述  从抽象的层面描述服务对象和服务方法   
*/
//  这里是框架给外部使用的， 可以发布rpc方法的函数接口
void RpcProvider::NotifyService(google::protobuf::Service *service) {

    ServiceInfo service_info;
    //获取了服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();

    // 获取服务的名字
    std::string service_name = pserviceDesc->name();

    //获取服务对象service的方法的数量
    int methodCnt = pserviceDesc->method_count();

    //std::cout << "service_name: " << service_name << std::endl;
    LOG_INFO("service_name:%s", service_name.c_str());

    for (int i = 0; i < methodCnt; ++i) {
        //获取服务对象指定下标的服务方法的描述   UserService Login
        const google::protobuf::MethodDescriptor* pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});

        LOG_INFO("method_name:%s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点，开始提供rpc远程网络调用服务
void RpcProvider::Run() {

    // 读取配置文件rpcserver的信息
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    //创建Tcpserver对象    111   3333
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    //绑定连接回调和消息读写回调方法（有没有新的用户连接和用户的读写事件）  很好的分离了网络代码和业务代码   4444
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));  //设置连接回调
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    //设置muduo库的线程数量  设置为多线程时  一个线程专门做IO线程  其他都作为工作线程
    server.setThreadNum(4);  // 设置多线程为4  一个IO线程  三个worker线程   Reactor网络模型   555

    // 把当前的rpc节点上要发布的服务全部注册到zk上面 让rpc client可以从zk上发现服务
    // session timeout  30s  心跳时间  zkclient API提供心跳机制    网络I/O线程  1/3 timeout时间发送ping消息   告诉znode节点依然存在
    ZkClient zkCli;
    zkCli.Start();

    //service_name为永久性节点  method_name为临时性节点
    for (auto &sp : m_serviceMap) {
        //service_name    /UserServiceRpc
        std::string service_path = "/" + sp.first;   // 创建节点 FriendServiceRpc节点   /FriendServiceRpc 永久性节点  或者 UserServiceRpc
        zkCli.Create(service_path.c_str(), nullptr, 0);

        for (auto &mp : sp.second.m_methodMap) {
            //   service_name/method_name    /UserServiceRpc/Login   存储当前这个rpc服务节点主机的ip和port   
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port); // ip : port
            // ZOO_EPHEMERAL表示znode是一个临时节点   zkserver断开即销毁
            zkCli.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);  
        }
    }
    

    std::cout << "RpcProvider start service at ip: " << ip << " port: " << port << std::endl;  //等待远程连接

    //启动网络服务
    server.start();
    m_eventLoop.loop(); //相当于 epoll wait 以阻塞方式等待连接
}

//新的socket的连接回调
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &coon) {
    if (!coon->connected()) {
        coon->shutdown();
    }
}

/*
在框架内部， RpcProvider 和 RpcConsumer协商好通信用的protobuf数据类型
service_name method_name args_size  定义proto的message类型 ， 进行数据头的序列化和反序列化
UserService Login zhang san 123456  不行!!!!

！！！recv_buf = header_size （前4个字节存储了header_str的长度为header_size） 
               +   header_str(需要反序列化的str  长度为header_size  包括了 service_name method_name args_size)  
               +  args_str     可能会发生tcp粘包问题   
std::string   insert和copy方法

*/

// 已建立连接用户的读写事件回调  如果远程有一个rpc服务的请求，那么OnMessage方法就会响应
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &coon, muduo::net::Buffer *buffer, muduo::Timestamp) {
    //网络上接收的远程rpc调用请求的字符流   Login  args
    std::string recv_buf = buffer->retrieveAllAsString();
    
    //从字符流中读取前四个字节的内容
    uint32_t header_size = 0;
    recv_buf.copy((char*)&header_size, 4, 0);  //把recv_buf里的前四个字节拷贝给header_size

    //根据header_size读取数据头的原始字符流   反序列化数据  得到rpc请求的详细信息
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;


    if (rpcHeader.ParseFromString(rpc_header_str)) {
        //数据头反序列化成功
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    } else {
        //数据头反序列化失败
        std::cout << "rpc_header_str: " << rpc_header_str << "parse error!" << std::endl;
        return;
    }

    //获取rpc方法参数的字符流数据
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    //打印调试信息

    std::cout << "=========================================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl; 
    std::cout << "=========================================================" << std::endl;    

    //获取service对象和method对象
    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end()) {
        std::cout << service_name  << "is not exist!!!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);

    if (mit == it->second.m_methodMap.end()) {
        std::cout << service_name << ":" << method_name << "is not exist!!!" << std::endl;
        return;
    }

    google::protobuf::Service *service = it->second.m_service; //获取service对象  new UserService
    const google::protobuf::MethodDescriptor *method = mit->second;  //获取method对象 Login

    // 生成rpc方法调用的请求request和response参数
    google::protobuf::Message *request = service->GetRequestPrototype(method).New();

    if (!request->ParseFromString(args_str)) {
        std::cout << "request parse error, content: " << args_str << std::endl;
        return;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    //给下面的method方法的调用 ， 绑定一个Closure的回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr&,
                                                                    google::protobuf::Message*>
                                                                    (this,
                                                                    &RpcProvider::SendRpcResponse,
                                                                    coon, response);
    // 在框架上根据远端rpc请求，调用当前rpc节点上发布的方法
    // new UserService().Login(controller, request, response, done)
    service->CallMethod(method, nullptr, request, response, done);
}


//Closuer的回调操作， 用于序列化rpc的响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& coon, google::protobuf::Message *response) {
    std::string response_str;

    if (response->SerializeToString(&response_str)) {   // response进行序列化
        //序列化成功以后 通过网络把rpc方法执行的结果发送回rpc调用方
        coon->send(response_str);
    } else {
        std::cout << "serialize response_str error" << std::endl;
    }
    coon->shutdown(); //模拟http的短链接服务，由rpcprovider主动断开连接
}

