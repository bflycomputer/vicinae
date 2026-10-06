#pragma once
#include <QtQml/qqmlregistration.h>
#include "builtins/clipboard/history/clipboard-history-model.hpp"
#include "ui/views/bridge-view.hpp"
#include "ui/quick/completion-model.hpp"
#include "ui/views/section-list-model.hpp"
#include "services/clipboard/clipboard-db.hpp"
#include "ui/views/view-utils.hpp"

class ClipboardHistoryController;
class ClipboardService;

class ClipboardHistoryViewHost : public ViewHostBase {
  Q_OBJECT
  QML_NAMED_ELEMENT(ClipboardHistoryViewHost)
  QML_UNCREATABLE("")
  Q_PROPERTY(CompletionModel *kindFilterModel READ kindFilterModel CONSTANT)
  Q_PROPERTY(int currentKindFilter READ currentKindFilter NOTIFY currentKindFilterChanged)
  Q_PROPERTY(bool hasDetail READ hasDetail NOTIFY detailChanged)
  Q_PROPERTY(bool hasDetailError READ hasDetailError NOTIFY detailChanged)
  Q_PROPERTY(QString detailType READ detailType NOTIFY detailChanged)
  Q_PROPERTY(QString detailTitle READ detailTitle NOTIFY detailChanged)
  Q_PROPERTY(bool detailIsFileIcon READ detailIsFileIcon NOTIFY detailChanged)
  Q_PROPERTY(QString detailTextContent READ detailTextContent NOTIFY detailChanged)
  Q_PROPERTY(QString detailImageSource READ detailImageSource NOTIFY detailChanged)
  Q_PROPERTY(QString detailCopiedAt READ detailCopiedAt NOTIFY detailChanged)
  Q_PROPERTY(QString detailEncryptionIcon READ detailEncryptionIcon NOTIFY detailChanged)
  Q_PROPERTY(QString detailErrorTitle READ detailErrorTitle NOTIFY detailChanged)
  Q_PROPERTY(QString detailErrorDescription READ detailErrorDescription NOTIFY detailChanged)
  Q_PROPERTY(QStringList searchTerms READ searchTerms NOTIFY searchTermsChanged)

public:
  explicit ClipboardHistoryViewHost();
  ~ClipboardHistoryViewHost() override;

  QUrl qmlComponentUrl() const override;
  QUrl qmlSearchAccessoryUrl() const override;
  QVariantMap qmlProperties() override;
  void loadInitialData() override;
  void textChanged(const QString &text) override;
  void initialize() override;
  void onReactivated() override;
  void beforePop() override;
  bool needsGlobalStatusBar() const override { return false; }
  bool showBackButton() const override { return false; }

  Q_INVOKABLE void toggleMonitoring();
  Q_INVOKABLE void setKindFilter(int kind);

  SectionListModel *listModel() const override { return const_cast<SectionListModel *>(&m_model); }
  CompletionModel *kindFilterModel() { return &m_kindFilterModel; }
  int currentKindFilter() const { return m_currentKindFilter; }
  bool hasDetail() const { return m_hasDetail; }
  bool hasDetailError() const { return m_hasDetailError; }
  QString detailType() const { return m_detailType; }
  QString detailTitle() const { return m_detailTitle; }
  bool detailIsFileIcon() const { return m_detailIsFileIcon; }
  QString detailTextContent() const { return m_detailTextContent; }
  QString detailImageSource() const { return m_detailImageSource; }
  QString detailCopiedAt() const { return m_detailCopiedAt; }
  QString detailEncryptionIcon() const { return m_detailEncryptionIcon; }
  QString detailErrorTitle() const { return m_detailErrorTitle; }
  QString detailErrorDescription() const { return m_detailErrorDescription; }
  QStringList searchTerms() const { return m_searchTerms; }

signals:
  void currentKindFilterChanged();
  void detailChanged();
  void searchTermsChanged();

private:
  void updateSearchTerms(const QString &text);
  void setEntries(const std::vector<ClipboardHistoryEntry> &entries);
  void loadDetail(const ClipboardHistoryEntry &entry);
  void clearDetail();

  SectionListModel m_model{this};
  CompletionModel m_kindFilterModel{this};
  std::vector<std::unique_ptr<ClipboardHistorySection>> m_sections;
  ClipboardHistorySection::DefaultAction m_defaultAction = ClipboardHistorySection::DefaultAction::Copy;
  ClipboardHistoryController *m_controller = nullptr;
  ClipboardService *m_clipman = nullptr;
  QMimeDatabase m_mimeDb;

  int m_currentKindFilter = 0;

  bool m_hasDetail = false;
  bool m_hasDetailError = false;
  QString m_detailType;
  QString m_detailTitle;
  bool m_detailIsFileIcon = false;
  QString m_detailTextContent;
  QString m_detailImageSource;
  QString m_detailCopiedAt;
  QString m_detailEncryptionIcon;
  QString m_detailErrorTitle;
  QString m_detailErrorDescription;
  QStringList m_searchTerms;
};
