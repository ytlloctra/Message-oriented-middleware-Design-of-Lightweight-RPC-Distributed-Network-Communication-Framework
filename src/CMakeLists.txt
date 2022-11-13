# 把当前目录所有源文件放入 SRC_LIST
#aux_source_directory(. SRC_LIST)
set(SRC_LIST mprpcapplication.cc mprpcconfig.cc rpcheader.pb.cc rpcprovider.cc mprpcchannel.cc mprpccontroller.cc logger.cc zookeeperutil.cc)

# 生成静态库
add_library(mprpc ${SRC_LIST})

# muduo_net 要写在前面  muduo_base也依赖 muduo_net   不能写成target_link_directories  zookeeper_mt为多线程版本
target_link_libraries(mprpc muduo_net muduo_base pthread zookeeper_mt)