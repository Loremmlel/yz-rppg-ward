#pragma once

/**
 * @brief WebSocket 通信协议字段名常量
 *
 * 将协议耦合点集中到一处，避免字段字符串散落在业务代码中，
 * 联调或协议变更时只需修改此文件。
 *
 * 下行 JSON 示例（服务器 → 客户端）：
 * @code
 * { "heart_rate": 72, "spo2": 98, "respiration_rate": 16 }
 * @endcode
 */
namespace WsProtocol {

constexpr auto KEY_HEART_RATE       = "heart_rate";
constexpr auto KEY_SPO2             = "spo2";
constexpr auto KEY_RESPIRATION_RATE = "respiration_rate";

} // namespace WsProtocol

