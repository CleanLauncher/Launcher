
#pragma once

#include <QWidget>
#include <memory>

class Task;
class QProgressBar;
class QLabel;

class ProgressWidget : public QWidget {
    Q_OBJECT
   public:
    explicit ProgressWidget(QWidget* parent = nullptr, bool show_label = true);

    void hideIfInactive(bool hide) { m_hide_if_inactive = hide; }

    void reset();

    void progressFormat(QString);

   public slots:

    void watch(Task* task);

    void start(Task* task);

    bool exec(std::shared_ptr<Task> task);

    void show();

    void hide();

   private slots:
    void handleTaskFinish();
    void handleTaskStatus(const QString& status);
    void handleTaskProgress(qint64 current, qint64 total);
    void taskDestroyed();

   private:
    QLabel* m_label = nullptr;
    QProgressBar* m_bar = nullptr;
    Task* m_task = nullptr;

    bool m_hide_if_inactive = false;
};
