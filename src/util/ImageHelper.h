#pragma once
#include <QImage>
#include <opencv2/opencv.hpp>

/**
 * @brief Qt 与 OpenCV 图像格式互转工具
 */
class ImageHelper {
public:
    /**
     * @brief 将 QImage 转换为 cv::Mat（BGR 格式）
     *
     * 对常见格式（RGB32、RGB888）走零拷贝路径；未知格式先转为 RGB888 再处理。
     */
    static cv::Mat QImage2CvMat(const QImage &image) {
        cv::Mat mat;
        switch (image.format()) {
            case QImage::Format_RGB32:
            case QImage::Format_ARGB32:
            case QImage::Format_ARGB32_Premultiplied: {
                const cv::Mat map4(image.height(), image.width(), CV_8UC4,
                                   const_cast<uchar *>(image.bits()), image.bytesPerLine());
                cv::cvtColor(map4, mat, cv::COLOR_BGRA2BGR);
                break;
            }
            case QImage::Format_RGB888: {
                const cv::Mat mat3(image.height(), image.width(), CV_8UC3,
                                   const_cast<uchar *>(image.bits()), image.bytesPerLine());
                cv::cvtColor(mat3, mat, cv::COLOR_RGB2BGR);
                break;
            }
            default: {
                // 非常见格式走慢速转换路径，转成 RGB888 后再处理
                QImage convImg = image.convertToFormat(QImage::Format_RGB888);
                const cv::Mat mat3(convImg.height(), convImg.width(), CV_8UC3,
                                   convImg.bits(), convImg.bytesPerLine());
                cv::cvtColor(mat3, mat, cv::COLOR_RGB2BGR);
                break;
            }
        }
        return mat;
    };
};
