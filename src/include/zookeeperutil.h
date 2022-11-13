#pragma once

#include <semaphore.h>
#include <zookeeper/zookeeper.h>
#include <string>

//封装的zk类
class ZkClient {
public:
    ZkClient();
    ~ZkClient();
    void Start();   // 启动zkserver
    void Create(const char *path, const char *data, int datalen, int state = 0);  // 在zkserver上根据指定的path创建znode节点  state = 0代表永久性节点
    std::string GetData(const char *path); //根据参数指定的znode节点路径 获得znode节点的值

private:
    zhandle_t *m_zhandle;  //zk的客户端句柄   操作zkserver
};