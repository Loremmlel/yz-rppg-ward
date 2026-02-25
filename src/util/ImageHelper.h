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
    static cv::Mat QImage2CvMat(const QImage &image);
};
