#pragma once
#include "ui/views/bridge-view.hpp"
#include "ui/action-panel/action.hpp"
#include <qtimer.h>

class RootSearchModel;
class SectionListModel;

class RootViewHost : public ViewHostBase {
  Q_OBJECT
  Q_PROPERTY(QVariantList recentApps READ recentApps NOTIFY recentsChanged)
  Q_PROPERTY(int recentIndex MEMBER m_recentIndex NOTIFY recentSelectionChanged)
  Q_PROPERTY(bool queryEmpty MEMBER m_queryEmpty NOTIFY queryChanged)

public:
  QUrl qmlComponentUrl() const override;
  QVariantMap qmlProperties() override;
  QString initialSearchPlaceholderText() const override { return tr("Start typing to search"); }
  bool showBackButton() const override { return false; }

  void initialize() override;
  void textChanged(const QString &text) override;
  void onReactivated() override;
  void beforePop() override;

  SectionListModel *listModel() const override;
  QVariantList recentApps() const;
  Q_INVOKABLE void launchRecent(int index);

signals:
  void recentsChanged();
  void queryChanged();
  void recentSelectionChanged();

protected:
  bool inputFilter(QKeyEvent *) override;
  void beforeActionExecuted(const AbstractAction *action) override;
  bool tryAliasFastTrack();
  void scheduleNextClockTick();

private:
  void selectRecent(int index);

  bool m_queryEmpty = true;
  int m_recentIndex = -1;
  bool m_textChangedByHistory = false;
  std::optional<int> m_historyOffset;
  QTimer *m_clockTimer = new QTimer(this);
  RootSearchModel *m_model = nullptr;
};
