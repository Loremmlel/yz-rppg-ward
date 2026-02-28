#pragma once

/**
 * @brief 一维卡尔曼滤波器（仅跟踪标量位置）
 *
 *  - 状态：标量位置（无速度分量）
 *  - 过程噪声随时间间隔平方缩放：Q = processNoise × (dt / refInterval)²
 *  - 跳帧时用上一次原始观测值做 update（hold + smooth），避免漂移
 *
 * 用于平滑人脸检测框的 x / y / w / h 四个分量。
 */
class KalmanFilter1D {
public:
    /**
     * @param processNoise     过程噪声基准方差
     * @param measureNoise     观测噪声方差
     * @param initialState     初始估计值
     * @param initialError     初始估计误差
     * @param referenceInterval 基准时间间隔（秒），默认 1/30
     */
    explicit KalmanFilter1D(double processNoise = 0.01,
                            double measureNoise = 0.5,
                            double initialState = 0.0,
                            double initialError = 1.0,
                            double referenceInterval = 1.0 / 30.0)
        : m_q(processNoise),
          m_r(measureNoise),
          m_estimate(initialState),
          m_estimateError(initialError),
          m_refInterval(referenceInterval) {}

    /**
     * @brief 用观测值更新滤波器
     * @param measurement 观测值
     * @param dt 距上次更新的时间间隔（秒），默认使用基准间隔
     * @return 滤波后的估计值
     */
    double update(double measurement, double dt = -1.0) {
        if (dt < 0.0) dt = m_refInterval;

        // 过程噪声随时间间隔平方缩放
        const double timeScale = dt / m_refInterval;
        const double adjustedQ = m_q * (timeScale * timeScale);

        // 预测步：状态不变（无速度模型），误差累加过程噪声
        const double predError = m_estimateError + adjustedQ;

        // 卡尔曼增益
        const double K = predError / (predError + m_r);

        // 更新步
        m_estimate = m_estimate + K * (measurement - m_estimate);
        m_estimateError = (1.0 - K) * predError;

        return m_estimate;
    }

    /** 重置滤波器状态 */
    void reset(double state, double error = 1.0) {
        m_estimate = state;
        m_estimateError = error;
    }

    [[nodiscard]] double estimate() const { return m_estimate; }

private:
    double m_q;             ///< 过程噪声基准方差
    double m_r;             ///< 观测噪声方差
    double m_estimate;      ///< 当前估计值
    double m_estimateError; ///< 当前估计误差（协方差）
    double m_refInterval;   ///< 基准时间间隔
};


