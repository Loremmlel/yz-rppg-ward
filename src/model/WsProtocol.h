#pragma once
#include <QString>

/**
 * @brief WebSocket 通信协议常量
 *
 * 集中管理客户端与服务器之间 JSON 消息的字段名，
 * 避免协议字符串散落在各处，方便后期联调修改。
 *
 * 上行（客户端 → 服务器）：二进制帧，JPEG 编码的人脸 ROI 图像
 * 下行（服务器 → 客户端）：文本帧，UTF-8 JSON
 *
 * 下行 JSON 示例：
 * {
 *   "heart_rate": 72,
 *   "spo2": 98,
 *   "respiration_rate": 16
 * }
 */
namespace WsProtocol {

/// 下行体征数据字段名
constexpr auto KEY_HEART_RATE      = "heart_rate";
constexpr auto KEY_SPO2            = "spo2";
constexpr auto KEY_RESPIRATION_RATE = "respiration_rate";

} // namespace WsProtocol

