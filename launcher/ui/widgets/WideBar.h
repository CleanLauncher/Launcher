#pragma once

#include <QAction>
#include <QMap>
#include <QMenu>
#include <QToolBar>

#include <memory>

class WideBar : public QToolBar {
    Q_OBJECT

    Q_PROPERTY(bool useDefaultAction MEMBER m_use_default_action)

   public:
    explicit WideBar(const QString& title, QWidget* parent = nullptr);
    explicit WideBar(QWidget* parent = nullptr);
    ~WideBar() override = default;

    void addAction(QAction* action);
    void addSeparator();

    void insertSpacer(QAction* action);
    void insertSeparator(QAction* before);
    void insertActionBefore(QAction* before, QAction* action);
    void insertActionAfter(QAction* after, QAction* action);
    void insertWidgetBefore(QAction* before, QWidget* widget);

    QMenu* createContextMenu(QWidget* parent = nullptr, const QString& title = QString());
    void showVisibilityMenu(const QPoint&);

    void addContextMenuAction(QAction* action);

    QByteArray getVisibilityState() const;
    void setVisibilityState(QByteArray&&);

    void removeAction(QAction* action);

   private:
    struct BarEntry {
        enum class Type { None, Action, Separator, Spacer } type = Type::None;
        QAction* bar_action = nullptr;
        QAction* menu_action = nullptr;
    };

    auto getMatching(QAction* act) -> QList<BarEntry>::iterator;

    QByteArray getHash() const;
    bool checkHash(QByteArray const&) const;

   private:
    QList<BarEntry> m_entries;

    QList<QAction*> m_context_menu_actions;

    bool m_use_default_action = false;

    std::unique_ptr<QMenu> m_bar_menu = nullptr;
    enum class MenuState { Fresh, Dirty } m_menu_state = MenuState::Dirty;
};
