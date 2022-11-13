# 轻量级RPC分布式网络通信框架设计

## 实现功能

- 采用了muduo网络库中的主从Reactor反应堆模型， 借用epoll技术实现多路IO复用, 增加并行服务的数量。
- 针对客户端与服务器端的RPC请求和响应，使用Protobuf作为数据序列化和反序列化的私有通信协议。
- 以Zookeeper作为消息中间件， 建立了分布式服务配置中心，所有服务器端的RPC节点均需要向配置中心注册服。客户端在配置中心可以查询到要调用的对应服务的ip地址、端口号等信息，进行远程调用。
- 日志系统采用了单例设计模式，提供了线程安全的异步日志缓冲队列写入操作。

## 框架设计

### 01 整体架构

![img](https://image-1312312327.cos.ap-shanghai.myqcloud.com/efa86ddd01f5e87a7f46cddfb2452712.png)

### 02 RPC调用过程

远程调用需传递**服务对象、函数方法、函数参数**，经序列化成字节流后传给提供服务的服务器，服务器接收到数据后反序列化成**服务对象、函数方法、函数参数**，并发起本地调用，将响应结果序列化成字节流，发送给调用方，调用方接收到后反序列化得到结果，并传给本地调用。

![image-20220814221421663](https://image-1312312327.cos.ap-shanghai.myqcloud.com/867ce487ebd17e5efd23718cca36fb81.png)

### 03 序列化和反序列化

- 序列化：对象转为字节序列称为对象的序列化
- 反序列化：字节序列转为对象称为对象的反序列化

![image-20220814235216125](https://image-1312312327.cos.ap-shanghai.myqcloud.com/20ceda4b9b31c0904d6dd569128adc08.png)

常见序列化和反序列化协议有XML、JSON、PB，相比于其他PB更有优势：跨平台语言支持，序列化和反序列化效率高速度快，且序列化后体积比XML和JSON都小很多，适合网络传输。

| XML      | JSON               | PB                 |                             |
| -------- | ------------------ | ------------------ | --------------------------- |
| 保存方式 | 文本               | 文本               | 二进制                      |
| 可读性   | 较好               | 较好               | 不可读                      |
| 解析效率 | 慢                 | 一般               | 快                          |
| 语言支持 | 所有语言           | 所有语言           | C++/Java/Python及第三方支持 |
| 适用范围 | 文件存储、数据交互 | 文件存储、数据交互 | 文件存储、数据交互          |

注意：序列化和反序列化可能对系统的消耗较大，因此原则是：远程调用函数传入参数和返回值对象要尽量简单，具体来说应避免：

远程调用函数传入参数和返回值对象体积较大，如传入参数是List或Map，序列化后字节长度较长，对网络负担较大
远程调用函数传入参数和返回值对象有复杂关系，传入参数和返回值对象有复杂的嵌套、包含、聚合关系等，性能开销大
远程调用函数传入参数和返回值对象继承关系复杂，性能开销大

### 04 数据传输格式

![image-20220815120520672](https://image-1312312327.cos.ap-shanghai.myqcloud.com/4d779b864509b9b8071f666676b7c97f.png)

### 05 代码逻辑

#### 基本配置

![image-20221103192839064](https://image-1312312327.cos.ap-shanghai.myqcloud.com/image-20221103192839064.png)

#### 整体调用流程

![image-20220815133628010](https://image-1312312327.cos.ap-shanghai.myqcloud.com/495f015a7fa66bf1390f9b00401a37e2.png)

![image-20220815184439376](https://image-1312312327.cos.ap-shanghai.myqcloud.com/46f80f074429cfdc14d4b0baae666603.png)

## 运行

### 运行环境

ubuntu18.04  +  cmake  + protobuf + muduo + zookeeper

### 项目文件说明

![image-20220923200052638](https://image-1312312327.cos.ap-shanghai.myqcloud.com/image-20220923200052638.png)

### 运行脚本

```shell
./autobuild.sh
```

## 运行结果

- rpc服务器端

![image-20221018143315477](https://image-1312312327.cos.ap-shanghai.myqcloud.com/image-20221018143315477.png)

- rpc客户端

![image-20221018143412539](https://image-1312312327.cos.ap-shanghai.myqcloud.com/image-20221018143412539.png)

- zookeeper查看节点信息  可查看到注册成功的节点

![image-20221018143500910](https://image-1312312327.cos.ap-shanghai.myqcloud.com/image-20221018143500910.png)

- zk查看节点的值

![image-20221018143813834](https://image-1312312327.cos.ap-shanghai.myqcloud.com/image-20221018143813834.png)

- tcpdump抓包分析

![image-20221018144320699](https://image-1312312327.cos.ap-shanghai.myqcloud.com/image-20221018144320699.png)