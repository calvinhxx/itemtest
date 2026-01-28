#ifndef TYPOGRAPHY_H
#define TYPOGRAPHY_H

#include <QFont>
#include <QString>

/**
 * @brief Typography - Windows UI Kit 字体排版系统
 * 基于 Segoe UI 字体家族的标准排版规范
 */
namespace Typography {
    /**
     * @brief 字体家族
     */
    namespace FontFamily {
        const QString SegoeUI = "Segoe UI";
        const QString SegoeUIVariable = "Segoe UI Variable";
        const QString SegoeFluentIcons = "Segoe Fluent Icons";
    }

    /**
     * @brief 字体大小
     */
    namespace FontSize {
        const int Caption = 12;      // 说明文字
        const int Body = 14;         // 正文
        const int BodyStrong = 14;   // 强调正文
        const int Subtitle = 16;     // 副标题
        const int Title = 18;        // 标题
        const int TitleLarge = 20;   // 大标题
        const int Display = 24;      // 展示文字
        const int DisplayLarge = 28;  // 大展示文字
    }

    /**
     * @brief 字体粗细
     */
    namespace FontWeight {
        const int Regular = QFont::Normal;      // 400
        const int Medium = QFont::Medium;       // 500
        const int SemiBold = QFont::DemiBold;   // 600
        const int Bold = QFont::Bold;           // 700
    }

    /**
     * @brief 行高（Line Height）
     */
    namespace LineHeight {
        const double Caption = 1.2;      // 说明文字行高
        const double Body = 1.5;         // 正文行高
        const double Subtitle = 1.4;     // 副标题行高
        const double Title = 1.3;        // 标题行高
        const double Display = 1.25;     // 展示文字行高
    }

    /**
     * @brief 预定义的字体样式
     */
    struct FontStyle {
        QString family;
        int size;
        int weight;
        double lineHeight;

        /**
         * @brief 创建 QFont 对象
         */
        QFont toQFont() const {
            QFont font(family, size, weight);
            font.setPixelSize(size);
            return font;
        }

        /**
         * @brief 获取样式表字符串
         */
        QString toStyleSheet() const {
            QString weightStr;
            if (weight == QFont::Bold) weightStr = "bold";
            else if (weight == QFont::DemiBold) weightStr = "600";
            else if (weight == QFont::Medium) weightStr = "500";
            else weightStr = "normal";

            return QString("font-family: %1; font-size: %2px; font-weight: %3;")
                .arg(family)
                .arg(size)
                .arg(weightStr);
        }
    };

    /**
     * @brief 预定义样式
     */
    namespace Styles {
        const FontStyle Caption = {
            FontFamily::SegoeUI,
            FontSize::Caption,
            FontWeight::Regular,
            LineHeight::Caption
        };

        const FontStyle Body = {
            FontFamily::SegoeUI,
            FontSize::Body,
            FontWeight::Regular,
            LineHeight::Body
        };

        const FontStyle BodyStrong = {
            FontFamily::SegoeUI,
            FontSize::BodyStrong,
            FontWeight::SemiBold,
            LineHeight::Body
        };

        const FontStyle Subtitle = {
            FontFamily::SegoeUI,
            FontSize::Subtitle,
            FontWeight::Medium,
            LineHeight::Subtitle
        };

        const FontStyle Title = {
            FontFamily::SegoeUI,
            FontSize::Title,
            FontWeight::SemiBold,
            LineHeight::Title
        };

        const FontStyle TitleLarge = {
            FontFamily::SegoeUI,
            FontSize::TitleLarge,
            FontWeight::Bold,
            LineHeight::Title
        };

        const FontStyle Display = {
            FontFamily::SegoeUI,
            FontSize::Display,
            FontWeight::Bold,
            LineHeight::Display
        };
    }
}

#endif // TYPOGRAPHY_H
