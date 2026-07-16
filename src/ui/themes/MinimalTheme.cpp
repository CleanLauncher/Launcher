#include "MinimalTheme.h"

QString MinimalTheme::qtTheme()
{
    // The "Windows" style is completely flat and provides the best base for a custom minimal look.
    return "Windows";
}

QString MinimalTheme::appStyleSheet()
{
    return R"(
        QWidget {
            font-family: "Segoe UI", "Helvetica Neue", Helvetica, Arial, sans-serif;
            font-size: 10pt;
        }
        QPushButton {
            padding: 5px 15px;
            border: 1px solid palette(mid);
            border-radius: 4px;
            background: palette(button);
        }
        QPushButton:hover {
            background: palette(midlight);
        }
        QPushButton:pressed {
            background: palette(mid);
        }
        QLineEdit, QSpinBox, QComboBox {
            padding: 4px 6px;
            border: 1px solid palette(mid);
            border-radius: 4px;
            background: palette(base);
        }
        QScrollBar:vertical {
            background: transparent;
            width: 10px;
        }
        QScrollBar::handle:vertical {
            background: palette(mid);
            border-radius: 5px;
            min-height: 20px;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";
}
