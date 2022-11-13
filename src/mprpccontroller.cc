#include "mprpccontroller.h"

MprpcController::MprpcController() {
    m_failed = false;  // 默认是false   不会出错
    m_errText = "";
}

void MprpcController::Reset() {
    m_failed = false;
    m_errText = "";
}

bool MprpcController::Failed() const{
    return m_failed;   // 判断rpc调用成功与否  true即为成功
}

std::string MprpcController::ErrorText() const {
    return m_errText;
}

void MprpcController::SetFailed(const std::string& reason) {
    m_failed = true;   //  真的发生错误了
    m_errText = reason;
}


void MprpcController::StartCancel() {}
bool MprpcController::IsCanceled() const {return false;}
void MprpcController::NotifyOnCancel(google::protobuf::Closure* callback) {}

