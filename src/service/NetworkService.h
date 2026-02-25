#ifndef WARD_NETWORKSERVICE_H
#define WARD_NETWORKSERVICE_H
#include <QObject>
#include "../model/AppConfig.h"


class NetworkService : public QObject {
    Q_OBJECT

public:
    explicit NetworkService(QObject *parent = nullptr);

public slots:
    /**
     * @brief 接收裁剪好的人脸区域并发送给后端
     */
    void sendFaceRoiStream(const QImage &faceRoi);

    /**
     * @brief 响应配置变更，更新服务器连接参数
     */
    void onConfigChanged(const AppConfig &config);

private:
    QString m_serverHost;
    quint16 m_serverPort{0};
};


#endif //WARD_NETWORKSERVICE_H
