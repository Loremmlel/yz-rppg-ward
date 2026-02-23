#ifndef WARD_NETWORKSERVICE_H
#define WARD_NETWORKSERVICE_H
#include <QObject>


class NetworkService : public QObject {
    Q_OBJECT
public:
    explicit NetworkService(QObject *parent = nullptr);

public slots:
    /**
     * @brief 接收裁剪好的人脸区域并发送给后端
     */
    void sendFaceRoiStream(const QImage& faceRoi);
};


#endif //WARD_NETWORKSERVICE_H