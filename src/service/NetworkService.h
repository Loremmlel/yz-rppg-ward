#ifndef WARD_NETWORKSERVICE_H
#define WARD_NETWORKSERVICE_H

#include <QObject>
#include <QElapsedTimer>
#include "../model/AppConfig.h"

class WebSocketClient;

/**
 * @brief 网络发送服务
 *
 * 职责：
 *  - 接收 VideoService 裁剪好的人脸 ROI 图像
 *  - 将 QImage 编码为 JPEG 字节流
 *  - 通过 WebSocketClient 推送至服务器
 *  - 内置帧率节流，避免发送队列堆积
 *
 * 依赖 WebSocketClient 处理底层连接细节，自身不关心重连逻辑。
 */
class NetworkService : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param wsClient 由外部（AppController）注入的 WebSocketClient，生命周期由外部管理
     * @param parent   Qt 父对象
     */
    explicit NetworkService(WebSocketClient *wsClient, QObject *parent = nullptr);

public slots:
    /**
     * @brief 接收裁剪好的人脸区域，编码后通过 WebSocket 发送
     *        内置 30 fps 节流：超出帧率的帧将被丢弃。
     */
    void sendFaceRoiStream(const QImage &faceRoi);

private:
    WebSocketClient *m_wsClient {nullptr}; ///< 不拥有，仅持有引用

    QElapsedTimer m_fpsTimer;
    static constexpr int TARGET_FPS     = 30;
    static constexpr int FRAME_INTERVAL_MS = 1000 / TARGET_FPS; // ~33 ms

    int  m_jpegQuality {75}; ///< JPEG 压缩质量 (1-100)
};

#endif // WARD_NETWORKSERVICE_H
