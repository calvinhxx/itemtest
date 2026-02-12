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
        /**
         * @brief 默认 UI 字体
         * Fluent UI 标准使用 Segoe UI Variable
         */
        const QString SegoeUI = "Segoe UI Variable";
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
     * @brief 常用图标 Unicode 编码 (Segoe Fluent Icons / MDL2 Assets)
     * 参考 WinUI 3 Gallery: Design -> Iconography
     * 完整参考: https://learn.microsoft.com/en-us/windows/apps/design/style/segoe-fluent-icons-font
     */
    namespace Icons {
        // --- Navigation & Window (E700 - E8FF) ---
        const QString GlobalNav = QString::fromUtf16(u"\uE700");
        const QString ChevronDown = QString::fromUtf16(u"\uE70D");
        const QString ChevronUp = QString::fromUtf16(u"\uE70E");
        const QString ChevronLeft = QString::fromUtf16(u"\uE76B");
        const QString ChevronRight = QString::fromUtf16(u"\uE76C");
        const QString ChevronDownMed = QString::fromUtf16(u"\uE972");
        const QString ChevronUpMed = QString::fromUtf16(u"\uE971");
        const QString ChevronLeftMed = QString::fromUtf16(u"\uE973");
        const QString ChevronRightMed = QString::fromUtf16(u"\uE974");
        const QString Back = QString::fromUtf16(u"\uE72B");
        const QString Forward = QString::fromUtf16(u"\uE72A");
        const QString Home = QString::fromUtf16(u"\uE80F");
        const QString Menu = QString::fromUtf16(u"\uE700");
        const QString Up = QString::fromUtf16(u"\uE74A");
        const QString Down = QString::fromUtf16(u"\uE74B");
        const QString Left = QString::fromUtf16(u"\uE72B");
        const QString Right = QString::fromUtf16(u"\uE72A");
        const QString FullScreen = QString::fromUtf16(u"\uE740");
        const QString BackToWindow = QString::fromUtf16(u"\uE73F");
        const QString More = QString::fromUtf16(u"\uE712");
        const QString AllApps = QString::fromUtf16(u"\uE71D");

        // --- Common Actions & Editing (E700 - E9FF) ---
        const QString Add = QString::fromUtf16(u"\uE710");
        const QString Cancel = QString::fromUtf16(u"\uE711");
        const QString Delete = QString::fromUtf16(u"\uE74D");
        const QString Save = QString::fromUtf16(u"\uE74E");
        const QString SaveAs = QString::fromUtf16(u"\uE792");
        const QString Search = QString::fromUtf16(u"\uE721");
        const QString Settings = QString::fromUtf16(u"\uE713");
        const QString Edit = QString::fromUtf16(u"\uE70F");
        const QString Refresh = QString::fromUtf16(u"\uE72C");
        const QString Share = QString::fromUtf16(u"\uE72D");
        const QString Copy = QString::fromUtf16(u"\uE8C8");
        const QString Cut = QString::fromUtf16(u"\uE8C6");
        const QString Paste = QString::fromUtf16(u"\uE8C7");
        const QString Filter = QString::fromUtf16(u"\uE71C");
        const QString Link = QString::fromUtf16(u"\uE71B");
        const QString FavoriteStar = QString::fromUtf16(u"\uE734");
        const QString FavoriteStarFill = QString::fromUtf16(u"\uE735");
        const QString Pin = QString::fromUtf16(u"\uE718");
        const QString PinFill = QString::fromUtf16(u"\uE746");
        const QString Unpin = QString::fromUtf16(u"\uE77A");
        const QString Flag = QString::fromUtf16(u"\uE7C1");
        const QString Block = QString::fromUtf16(u"\uE733");
        const QString Zoom = QString::fromUtf16(u"\uE71E");
        const QString ZoomIn = QString::fromUtf16(u"\uE8A3");
        const QString ZoomOut = QString::fromUtf16(u"\uE71F");
        const QString Undo = QString::fromUtf16(u"\uE7A7");
        const QString Redo = QString::fromUtf16(u"\uE7A6");
        const QString SelectAll = QString::fromUtf16(u"\uE8B3");

        // --- Media & Sound (E700 - EAFF) ---
        const QString Play = QString::fromUtf16(u"\uE768");
        const QString Pause = QString::fromUtf16(u"\uE769");
        const QString Stop = QString::fromUtf16(u"\uE71A");
        const QString Volume = QString::fromUtf16(u"\uE767");
        const QString Mute = QString::fromUtf16(u"\uE74F");
        const QString Microphone = QString::fromUtf16(u"\uE720");
        const QString Video = QString::fromUtf16(u"\uE714");
        const QString Camera = QString::fromUtf16(u"\uE722");
        const QString Music = QString::fromUtf16(u"\uE8D6");
        const QString Movie = QString::fromUtf16(u"\uE8B2");
        const QString Headphones = QString::fromUtf16(u"\uE7F6");
        const QString Speaker = QString::fromUtf16(u"\uE7F5");
        const QString SkipBack = QString::fromUtf16(u"\uE892");
        const QString SkipForward = QString::fromUtf16(u"\uE893");

        // --- Communication & User (E700 - EBFF) ---
        const QString Mail = QString::fromUtf16(u"\uE715");
        const QString People = QString::fromUtf16(u"\uE716");
        const QString Phone = QString::fromUtf16(u"\uE717");
        const QString Message = QString::fromUtf16(u"\uE891");
        const QString Send = QString::fromUtf16(u"\uE724");
        const QString Contact = QString::fromUtf16(u"\uE77B");
        const QString Group = QString::fromUtf16(u"\uE902");
        const QString Emoji = QString::fromUtf16(u"\uE899");
        const QString World = QString::fromUtf16(u"\uE909");
        const QString ContactInfo = QString::fromUtf16(u"\uE779");
        const QString Accounts = QString::fromUtf16(u"\uE910");

        // --- Files & Folders (E800 - ECFF) ---
        const QString Folder = QString::fromUtf16(u"\uE838");
        const QString File = QString::fromUtf16(u"\uE8A5");
        const QString Document = QString::fromUtf16(u"\uE8A5");
        const QString Cloud = QString::fromUtf16(u"\uE753");
        const QString Download = QString::fromUtf16(u"\uE896");
        const QString Upload = QString::fromUtf16(u"\uE898");
        const QString Sync = QString::fromUtf16(u"\uE895");
        const QString Storage = QString::fromUtf16(u"\uE8B7");
        const QString Calculator = QString::fromUtf16(u"\uE8EF");
        const QString Calendar = QString::fromUtf16(u"\uE787");
        const QString Clock = QString::fromUtf16(u"\uE917");
        const QString History = QString::fromUtf16(u"\uE81C");

        // --- System & Hardware (E700 - EDFF) ---
        const QString Wifi = QString::fromUtf16(u"\uE701");
        const QString Bluetooth = QString::fromUtf16(u"\uE702");
        const QString Battery = QString::fromUtf16(u"\uE83F");
        const QString Print = QString::fromUtf16(u"\uE749");
        const QString Laptop = QString::fromUtf16(u"\uE7F8");
        const QString Mobile = QString::fromUtf16(u"\uE8EA");
        const QString Desktop = QString::fromUtf16(u"\uE9EE");
        const QString Mouse = QString::fromUtf16(u"\uE962");
        const QString Keyboard = QString::fromUtf16(u"\uE92E");
        const QString Controller = QString::fromUtf16(u"\uE7FC");
        const QString Power = QString::fromUtf16(u"\uE7E8");
        const QString Brightness = QString::fromUtf16(u"\uE706");
        const QString Airplane = QString::fromUtf16(u"\uE709");

        // --- Feedback & Status (E700 - EEFF) ---
        const QString Warning = QString::fromUtf16(u"\uE7BA");
        const QString Error = QString::fromUtf16(u"\uE783");
        const QString Info = QString::fromUtf16(u"\uE946");
        const QString Success = QString::fromUtf16(u"\uE73E");
        const QString CheckMark = QString::fromUtf16(u"\uE73E");
        const QString Hyphen = QString::fromUtf16(u"\uE738");
        const QString Shield = QString::fromUtf16(u"\uEA18");
        const QString Lock = QString::fromUtf16(u"\uE72E");
        const QString Unlock = QString::fromUtf16(u"\uE785");
        const QString Heart = QString::fromUtf16(u"\uEB51");
        const QString HeartFill = QString::fromUtf16(u"\uEB52");
        const QString Star = QString::fromUtf16(u"\uE74C");

        // --- Design & Layout (E800 - EFFF) ---
        const QString Brush = QString::fromUtf16(u"\uE790");
        const QString Color = QString::fromUtf16(u"\uE790");
        const QString Font = QString::fromUtf16(u"\uE8D2");
        const QString Grid = QString::fromUtf16(u"\uE80A");
        const QString List = QString::fromUtf16(u"\uEA37");
        const QString AlignLeft = QString::fromUtf16(u"\uE8E4");
        const QString AlignCenter = QString::fromUtf16(u"\uE8E3");
        const QString AlignRight = QString::fromUtf16(u"\uE8E2");
        const QString MapPin = QString::fromUtf16(u"\uE707");

        // --- Weather & Environment (F000+) ---
        const QString Sunny = QString::fromUtf16(u"\uE706");
        const QString CloudWeather = QString::fromUtf16(u"\uE753");
        const QString Rain = QString::fromUtf16(u"\uE9D5");
        const QString Snow = QString::fromUtf16(u"\uE944");
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
