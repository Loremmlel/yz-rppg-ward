#ifndef WARD_IMAGEHELPER_H
#define WARD_IMAGEHELPER_H
#include <qimage.h>
#include  <opencv2/opencv.hpp>


class ImageHelper {
public:
    static cv::Mat QImage2CvMat(const QImage &image);
};

#endif //WARD_IMAGEHELPER_H