#include "ImageHelper.h"

cv::Mat ImageHelper::QImage2CvMat(const QImage &image) {
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
}
