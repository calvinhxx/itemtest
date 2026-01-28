#ifndef THEMECOLORS_H
#define THEMECOLORS_H

#include <QColor>
#include <QString>
#include <vector>

/**
 * @brief ThemeColors - Windows UI Kit 语义化颜色系统
 * 完全对齐 Figma "Windows UI Kit — Community" 中的 Fill Color, Stroke Color, Text Color 规范
 */
namespace ThemeColors {

    /**
     * @brief Light 主题颜色 (标准语义化 Token)
     */
    namespace Light {
        // --- Fill Colors (填充色) ---
        namespace Fill {
            // Accent (强调色，如按钮背景)
            const QColor AccentDefault = QColor("#005FB8");
            const QColor AccentSecondary = QColor("#005FB8").lighter(110); // 90% opacity 效果
            const QColor AccentTertiary = QColor("#005FB8").lighter(120);  // 80% opacity 效果
            const QColor AccentDisabled = QColor(0, 0, 0, 54);           // 21% opacity black

            // Control (控件背景，如输入框、次要按钮)
            const QColor ControlDefault = QColor("#FFFFFF");
            const QColor ControlSecondary = QColor("#F9F9F9");
            const QColor ControlTertiary = QColor("#F3F3F3");
            const QColor ControlDisabled = QColor("#F3F3F3");
            
            // Control Alt (用于交替背景)
            const QColor ControlAltSecondary = QColor(0, 0, 0, 5);      // 2% opacity black
            const QColor ControlAltTertiary = QColor(0, 0, 0, 15);      // 6% opacity black
            
            // Subtle (透明/悬停态)
            const QColor SubtleTransparent = QColor(0, 0, 0, 0);
            const QColor SubtleSecondary = QColor(0, 0, 0, 9);         // Hover
            const QColor SubtleTertiary = QColor(0, 0, 0, 15);         // Pressed
        }

        // --- Stroke Colors (描边/边框) ---
        namespace Stroke {
            const QColor ControlDefault = QColor(0, 0, 0, 15);         // 6% black
            const QColor ControlSecondary = QColor(0, 0, 0, 41);       // 16% black
            const QColor ControlStrong = QColor(0, 0, 0, 112);        // 44% black
            const QColor CardDefault = QColor(0, 0, 0, 13);            // 5% black
            const QColor DividerDefault = QColor(0, 0, 0, 20);         // 8% black
            const QColor SurfaceDefault = QColor("#757575");
        }

        // --- Text Colors (文本) ---
        namespace Text {
            const QColor Primary = QColor(0, 0, 0, 230);               // 90% black
            const QColor Secondary = QColor(0, 0, 0, 154);             // 60% black
            const QColor Tertiary = QColor(0, 0, 0, 112);              // 44% black
            const QColor Disabled = QColor(0, 0, 0, 92);               // 36% black
            const QColor OnAccentPrimary = QColor("#FFFFFF");          // 强调色上的文本
        }

        // --- Backgrounds (页面背景) ---
        const QColor BackgroundCanvas = QColor("#F3F3F3");
        const QColor BackgroundLayer = QColor("#FFFFFF");
        const QColor BackgroundSolid = QColor("#EEEEEE");
        
        // --- 中性色阶 (Neutral / Grey) ---
        const QColor Grey10 = QColor("#FAF9F8");
        const QColor Grey20 = QColor("#F3F2F1");
        const QColor Grey30 = QColor("#EDEBE9");
        const QColor Grey40 = QColor("#E1DFDD");
        const QColor Grey50 = QColor("#D2D0CE");
        const QColor Grey60 = QColor("#C8C6C4");
        const QColor Grey90 = QColor("#A19F9D");
        const QColor Grey130 = QColor("#605E5C");
        const QColor Grey160 = QColor("#323130");
        const QColor Grey190 = QColor("#201F1E");

        // --- 图表色 (Charts - Guidance & Charts 页面) ---
        const std::vector<QColor> Charts = {
            QColor("#005FB8"), QColor("#00BCF2"), QColor("#2B88D8"), 
            QColor("#004B50"), QColor("#00AD56"), QColor("#007833"),
            QColor("#881798"), QColor("#B4009E"), QColor("#E3008C"),
            QColor("#D83B01"), QColor("#EA4300"), QColor("#FF8C00")
        };
    }

    /**
     * @brief Dark 主题颜色 (标准语义化 Token)
     */
    namespace Dark {
        // --- Fill Colors (填充色) ---
        namespace Fill {
            const QColor AccentDefault = QColor("#60CDFF");
            const QColor AccentSecondary = QColor("#60CDFF").darker(110);
            const QColor AccentTertiary = QColor("#60CDFF").darker(120);
            const QColor AccentDisabled = QColor(255, 255, 255, 40);

            const QColor ControlDefault = QColor(255, 255, 255, 15);
            const QColor ControlSecondary = QColor(255, 255, 255, 20);
            const QColor ControlTertiary = QColor(255, 255, 255, 10);
            const QColor ControlDisabled = QColor(255, 255, 255, 10);
            
            // Control Alt (用于交替背景)
            const QColor ControlAltSecondary = QColor(255, 255, 255, 15);
            const QColor ControlAltTertiary = QColor(255, 255, 255, 20);
            
            const QColor SubtleTransparent = QColor(255, 255, 255, 0);
            const QColor SubtleSecondary = QColor(255, 255, 255, 15);   // Hover
            const QColor SubtleTertiary = QColor(255, 255, 255, 20);    // Pressed
        }

        // --- Stroke Colors (描边/边框) ---
        namespace Stroke {
            const QColor ControlDefault = QColor(255, 255, 255, 17);    // 7% white
            const QColor ControlSecondary = QColor(255, 255, 255, 23);  // 9% white
            const QColor ControlStrong = QColor(255, 255, 255, 138);   // 54% white
            const QColor CardDefault = QColor(255, 255, 255, 10);      // 4% white
            const QColor DividerDefault = QColor(255, 255, 255, 20);    // 8% white
            const QColor SurfaceDefault = QColor(255, 255, 255, 30);    // 12% white
        }

        // --- Text Colors (文本) ---
        namespace Text {
            const QColor Primary = QColor("#FFFFFF");
            const QColor Secondary = QColor(255, 255, 255, 200);        // 78% white
            const QColor Tertiary = QColor(255, 255, 255, 138);         // 54% white
            const QColor Disabled = QColor(255, 255, 255, 92);          // 36% white
            const QColor OnAccentPrimary = QColor("#000000");
        }

        // --- Backgrounds (页面背景) ---
        const QColor BackgroundCanvas = QColor("#202020");
        const QColor BackgroundLayer = QColor("#2D2D2D");
        const QColor BackgroundSolid = QColor("#1C1C1C");

        // --- 中性色阶 (Neutral / Grey) ---
        const QColor Grey10 = QColor("#201F1E");
        const QColor Grey20 = QColor("#252423");
        const QColor Grey30 = QColor("#292827");
        const QColor Grey40 = QColor("#323130");
        const QColor Grey50 = QColor("#3B3A39");
        const QColor Grey60 = QColor("#484644");
        const QColor Grey90 = QColor("#8A8886");
        const QColor Grey130 = QColor("#C8C6C4");
        const QColor Grey160 = QColor("#F3F2F1");
        const QColor Grey190 = QColor("#FAF9F8");

        // --- 图表色 (Charts - Guidance & Charts 页面) ---
        const std::vector<QColor> Charts = {
            QColor("#60CDFF"), QColor("#00BCF2"), QColor("#2B88D8"),
            QColor("#00AD56"), QColor("#107C10"), QColor("#004B50"),
            QColor("#FF8C00"), QColor("#F7630C"), QColor("#EA4300"),
            QColor("#E3008C"), QColor("#BF0077"), QColor("#C239B3")
        };
    }
}

#endif // THEMECOLORS_H
