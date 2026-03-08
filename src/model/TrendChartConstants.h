#pragma once

#include <QColor>

/**
 * @brief 趋势图表页面的集中常量定义
 *
 * 包含：
 *  - 各指标图表的主题色（线色 / 卡片色带）
 *  - 各指标 Y 轴软上限（下限固定为 0；数据全部落在 [0, kYMax_xxx] 内时
 *    Y 轴固定使用该范围，避免因数值稳定而造成视觉上的剧烈抖动；
 *    有数据超出上限时自动退回动态计算）
 */
namespace TrendChartConstants {
    // ════════════════════════════════════════════════════════════════
    // 主题色
    //   原则：同组指标色相相近但明度/饱和度有区分；
    //         跨组之间色相差异明显，保证总体 13 色各不相同。
    // ════════════════════════════════════════════════════════════════

    // ── 基础生命体征 ─────────────────────────────────────────────────
    /// 心率均值   —— 珊瑚红
    inline constexpr QColor kColorHrAvg{0xEF, 0x53, 0x50};
    /// 呼吸率均值 —— 天蓝
    inline constexpr QColor kColorBrAvg{0x29, 0xB6, 0xF6};
    /// 信号质量   —— 中紫
    inline constexpr QColor kColorSqiAvg{0xAB, 0x47, 0xBC};

    // ── HRV 时域 ────────────────────────────────────────────────────
    /// SDNN   —— 青绿
    inline constexpr QColor kColorSdnn{0x26, 0xA6, 0x9A};
    /// RMSSD  —— 翠绿
    inline constexpr QColor kColorRmssd{0x66, 0xBB, 0x6A};
    /// SDSD   —— 草绿
    inline constexpr QColor kColorSdsd{0x9C, 0xCC, 0x65};
    /// pNN50  —— 青蓝
    inline constexpr QColor kColorPnn50{0x26, 0xC6, 0xDA};
    /// pNN20  —— 蓝绿
    inline constexpr QColor kColorPnn20{0x00, 0xAC, 0xC1};

    // ── HRV 频域 ────────────────────────────────────────────────────
    /// LF/HF  —— 琥珀橙
    inline constexpr QColor kColorLfHfRatio{0xFF, 0xA0, 0x26};
    /// HF     —— 金黄
    inline constexpr QColor kColorHfAvg{0xFF, 0xCA, 0x28};
    /// LF     —— 深橙
    inline constexpr QColor kColorLfAvg{0xFF, 0x70, 0x43};
    /// VLF    —— 玫红
    inline constexpr QColor kColorVlfAvg{0xEC, 0x40, 0x7A};
    /// TP     —— 蓝灰（总功率，综合指标用中性色）
    inline constexpr QColor kColorTpAvg{0x78, 0x90, 0x9C};

    // ════════════════════════════════════════════════════════════════
    // Y 轴软上限
    //   含义：正常值上限，用于稳定 Y 轴显示范围。
    //   数据超出时自动切换为动态计算，下限始终钳制到 0。
    // ════════════════════════════════════════════════════════════════

    /// 心率均值上限（bpm）
    inline constexpr double kYMaxHrAvg = 120.0;
    /// 呼吸率均值上限（Hz）
    inline constexpr double kYMaxBrAvg = 0.4;
    /// 信号质量上限（无量纲，0–1）
    inline constexpr double kYMaxSqiAvg = 1.0;

    /// SDNN 上限（ms）
    inline constexpr double kYMaxSdnn = 150.0;
    /// RMSSD 上限（ms）
    inline constexpr double kYMaxRmssd = 100.0;
    /// SDSD 上限（ms）
    inline constexpr double kYMaxSdsd = 100.0;
    /// pNN50 上限（比例，0–1）
    inline constexpr double kYMaxPnn50 = 1.0;
    /// pNN20 上限（比例，0–1）
    inline constexpr double kYMaxPnn20 = 1.0;

    /// LF/HF 比值上限
    inline constexpr double kYMaxLfHfRatio = 4.0;
    /// HF 均值上限（ms²）
    inline constexpr double kYMaxHfAvg = 1200.0;
    /// LF 均值上限（ms²）
    inline constexpr double kYMaxLfAvg = 1700.0;
    /// VLF 均值上限（ms²）
    inline constexpr double kYMaxVlfAvg = 1000.0;
    /// 总功率均值上限（ms²）
    inline constexpr double kYMaxTpAvg = 5000.0;
} // namespace TrendChartConstants