#include "mprpcconfig.h"

//负责解析加载配置文件
void MprpcConfig::LoadConfigFile(const char *config_file) {
    FILE* pf = fopen(config_file, "r");  // 只读 打开文件
    if (pf == nullptr) {
        std::cout << config_file << " is not exist!!!" << std::endl;
        exit(EXIT_FAILURE);
    }

    //1.注释 2.正确的配置项 = 3.去掉开头多余的空格   循环读取
    while (!feof(pf)) {
        char buffer[512] = {0};
        fgets(buffer, 512, pf); // 读取一行

        //去掉字符串前面多余的空格
        std::string read_buf(buffer);  // 转为string src_buf
        Trim(read_buf);

        //判断#的注释  以及  空行
        if (read_buf[0] == '#' || read_buf.empty()) {
            continue;
        }

        //解析配置项
        int idx = read_buf.find('=');

        if (idx == -1) {
            //配置不合法
            continue;
        }

        std::string key;
        std::string value;
        key = read_buf.substr(0, idx); //0 ~ idx - 1  起始下标  长度
        Trim(key);      //  去除key前后多余的空格

        //127.0.0.1          \n
        int end_idx = read_buf.find('\n', idx);
        value = read_buf.substr(idx + 1, end_idx - idx - 1); // idx + 1 ~ 最后
        Trim(value); // 去除value前后多余的空格

        m_configMap.insert({key, value});
    }
}
//查询配置项信息
std::string MprpcConfig::Load(const std::string &key) {
    auto it = m_configMap.find(key);
    if (it == m_configMap.end()) {
        return "";
    }
    return it->second;
}

//去掉字符串前后的空格
void MprpcConfig::Trim(std::string &src_buf) {
    int idx = src_buf.find_first_not_of(' ');  // 返回第一个非空格字符的下标

    if (idx != -1) {
        //说明字符串里有空格
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }

    //去掉字符串后面多余的空格
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1) {
        src_buf = src_buf.substr(0, idx + 1);
    }
}