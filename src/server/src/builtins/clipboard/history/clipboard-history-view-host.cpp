#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QUrl>
#include <ranges>

#include "builtins/clipboard/history/clipboard-history-view-host.hpp"
#include "builtins/clipboard/history/clipboard-history-controller.hpp"
#include "ui/views/view-utils.hpp"
#include "service-registry.hpp"
#include "services/clipboard/clipboard-service.hpp"
#include "utils/utils.hpp"
#include "vicinae.hpp"

static QString kindLabel(ClipboardOfferKind kind) {
  switch (kind) {
  case ClipboardOfferKind::Text:
    return QCoreApplication::translate("clipboard-history-view-host", "Text");
  case ClipboardOfferKind::Link:
    return QCoreApplication::translate("clipboard-history-view-host", "Link");
  case ClipboardOfferKind::Image:
    return QCoreApplication::translate("clipboard-history-view-host", "Image");
  case ClipboardOfferKind::File:
    return QCoreApplication::translate("clipboard-history-view-host", "File");
  case ClipboardOfferKind::Unknown:
  case ClipboardOfferKind::Count:
    break;
  }
  return QCoreApplication::translate("clipboard-history-view-host", "Unknown");
}

static std::optional<ClipboardOfferKind> kindFromFilterIndex(int index) {
  switch (index) {
  case 1:
    return ClipboardOfferKind::Text;
  case 2:
    return ClipboardOfferKind::Image;
  case 3:
    return ClipboardOfferKind::Link;
  case 4:
    return ClipboardOfferKind::File;
  default:
    return std::nullopt;
  }
}

ClipboardHistoryViewHost::ClipboardHistoryViewHost() : ViewHostBase() {
  auto item = [](const QString &id, const QString &label, QStringView icon) {
    return qml::makeDropdownItem(id, label, qml::imageSourceFor(ImageURL::builtinByName(icon)));
  };
  m_kindFilterModel.setItems({
      item("0", tr("All types"), u"clipboard-all"),
      item("1", tr("Text only"), u"clipboard-text"),
      item("2", tr("Image only"), u"clipboard-image"),
      item("4", tr("Files only"), u"clipboard-file"),
      item("3", tr("Links only"), u"clipboard-link"),
  });
  m_sections.reserve(1);
  m_sections.emplace_back(
      std::make_unique<ClipboardHistorySection>(QString{}, std::span<const ClipboardHistoryEntry>{}));
  m_model.addSource(m_sections.back().get());
  connect(&m_model, &SectionListModel::selectionCleared, this, &ClipboardHistoryViewHost::clearDetail);
}

ClipboardHistoryViewHost::~ClipboardHistoryViewHost() {
  if (m_clipman) m_clipman->resumeEviction();
}

QUrl ClipboardHistoryViewHost::qmlComponentUrl() const { return qml::componentUrl(u"ClipboardHistoryView"); }

QUrl ClipboardHistoryViewHost::qmlSearchAccessoryUrl() const {
  return qml::componentUrl(u"ClipboardFilterAccessory");
}

QVariantMap ClipboardHistoryViewHost::qmlProperties() {
  return {{QStringLiteral("host"), QVariant::fromValue(this)}};
}

void ClipboardHistoryViewHost::initialize() {
  BaseView::initialize();

  m_clipman = context()->services->clipman();
  m_clipman->pauseEviction();
  m_model.setScope(ViewScope(context(), this));

  m_controller = new ClipboardHistoryController(m_clipman, this);

  auto preferences = command()->preferenceValues();
  auto defaultActionStr = preferences.value("defaultAction").toString();
  m_defaultAction = defaultActionStr == "paste" ? ClipboardHistorySection::DefaultAction::Paste
                                                : ClipboardHistorySection::DefaultAction::Copy;

  setSearchPlaceholderText(tr("Search clipboard"));

  connect(m_clipman, &ClipboardService::monitoringChanged, &m_model, &SectionListModel::refreshActionPanel,
          Qt::QueuedConnection);

  connect(m_controller, &ClipboardHistoryController::dataRetrieved, this,
          [this](const PaginatedResponse<ClipboardHistoryEntry> &page) {
            bool const incremental = !m_model.selectFirstOnReset();
            setEntries(page.data);
            m_model.setSelectFirstOnReset(false);
            if (incremental) m_model.refreshActionPanel();
          });

  connect(m_controller, &ClipboardHistoryController::dataLoadingChanged, this, &BaseView::setLoading);
}

void ClipboardHistoryViewHost::loadInitialData() {
  m_controller->setFilter(searchText());
  updateSearchTerms(searchText());
}

void ClipboardHistoryViewHost::textChanged(const QString &text) {
  m_model.setSelectFirstOnReset(true);
  m_controller->setFilter(text);
  updateSearchTerms(text);
}

void ClipboardHistoryViewHost::updateSearchTerms(const QString &text) {
  auto terms = ClipboardDatabase::searchTerms(text);
  if (terms == m_searchTerms) return;
  m_searchTerms = std::move(terms);
  emit searchTermsChanged();
}

void ClipboardHistoryViewHost::onReactivated() { m_model.refreshActionPanel(); }

void ClipboardHistoryViewHost::beforePop() { m_model.beforePop(); }

void ClipboardHistoryViewHost::toggleMonitoring() {
  QJsonObject patch;
  if (m_clipman->monitoring()) {
    patch["monitoring"] = false;
  } else {
    patch["monitoring"] = true;
  }
  command()->setPreferenceValues(patch);
}

void ClipboardHistoryViewHost::setKindFilter(int kind) {
  if (m_currentKindFilter == kind) return;
  m_currentKindFilter = kind;
  emit currentKindFilterChanged();

  auto offerKind = kindFromFilterIndex(kind);
  m_model.setSelectFirstOnReset(true);
  m_controller->setKindFilter(offerKind);
}

void ClipboardHistoryViewHost::setEntries(const std::vector<ClipboardHistoryEntry> &entries) {
  auto dateFor = [](const ClipboardHistoryEntry &entry) -> std::optional<QDate> {
    if (entry.pinnedAt != 0) return std::nullopt;
    return QDateTime::fromSecsSinceEpoch(entry.updatedAt).date();
  };
  const auto today = QDate::currentDate();
  m_model.clearSources();
  m_sections.clear();
  m_sections.reserve(entries.size());
  for (auto group : entries | std::views::chunk_by(
                                  [&](const auto &a, const auto &b) { return dateFor(a) == dateFor(b); })) {
    const auto date = dateFor(group.front());
    QString name;
    if (!date) {
      name = tr("Pinned");
    } else if (*date == today) {
      name = tr("Today");
    } else if (*date == today.addDays(-1)) {
      name = tr("Yesterday");
    } else {
      name = QLocale().toString(*date, QLocale::LongFormat);
    }
    auto &section = m_sections.emplace_back(
        std::make_unique<ClipboardHistorySection>(std::move(name), std::span(group), m_clipman));
    section->setDefaultAction(m_defaultAction);
    section->setOnEntrySelected([this](const ClipboardHistoryEntry &entry) { loadDetail(entry); });
    section->setOnToggleMonitoring([this]() { toggleMonitoring(); });
    m_model.addSource(section.get());
  }
  if (m_sections.empty()) {
    m_sections.emplace_back(
        std::make_unique<ClipboardHistorySection>(QString{}, std::span<const ClipboardHistoryEntry>{}));
    m_model.addSource(m_sections.back().get());
  }
  m_model.rebuild();
}

void ClipboardHistoryViewHost::loadDetail(const ClipboardHistoryEntry &entry) {
  m_detailTextContent.clear();
  m_detailImageSource.clear();
  m_hasDetailError = false;
  m_detailErrorTitle.clear();
  m_detailErrorDescription.clear();

  m_detailType = kindLabel(entry.kind);
  m_detailTitle = m_detailType;
  m_detailIsFileIcon = false;
  m_detailCopiedAt = QLocale().toString(QDateTime::fromSecsSinceEpoch(entry.updatedAt),
                                        QStringLiteral("MMM d yyyy, h:mm AP"));

  if (entry.encryption != ClipboardEncryptionType::None) {
    m_detailEncryptionIcon =
        qml::imageSourceFor(ImageURL::builtin(BuiltinIcon::Key).setFill(SemanticColor::Green));
  } else {
    m_detailEncryptionIcon.clear();
  }

  auto data = m_clipman->getMainOfferData(entry.id);
  if (!data) {
    m_hasDetailError = true;
    switch (data.error()) {
    case ClipboardService::OfferDecryptionError::DecryptionFailed:
      m_detailErrorTitle = tr("Decryption failed");
      m_detailErrorDescription =
          tr("Vicinae could not decrypt the data for this selection. It was most likely encrypted "
             "with a different key and cannot be recovered. You can remove this entry from the "
             "history.");
      break;
    case ClipboardService::OfferDecryptionError::DataUnavailable:
      m_detailErrorTitle = tr("Data unavailable");
      m_detailErrorDescription = tr("The data for this selection could not be found on disk.");
      break;
    case ClipboardService::OfferDecryptionError::DecryptionRequired:
      m_detailErrorTitle = tr("Data is encrypted");
      m_detailErrorDescription =
          tr("Data for this selection was previously encrypted but the clipboard is not currently "
             "configured to use encryption. You should be able to fix this by enabling it in the "
             "settings.");
      break;
    }
    m_hasDetail = true;
    emit detailChanged();
    return;
  }

  const auto &rawData = data.value();
  const auto &mime = entry.mimeType;

  if (mime == "text/uri-list") {
    QString const text(rawData);
    auto paths = text.split("\r\n", Qt::SkipEmptyParts);
    if (paths.size() == 1) {
      QUrl const url(paths.at(0));
      if (url.isLocalFile()) {
        m_detailTitle = QFileInfo(url.toLocalFile()).fileName();
        std::error_code ec;
        std::filesystem::path const path = url.toLocalFile().toStdString();
        if (std::filesystem::is_regular_file(path, ec)) {
          auto preview = qml::resolveFilePreview(path, m_mimeDb);
          m_detailImageSource = preview.imageSource;
          m_detailTextContent = preview.textContent;
          m_detailIsFileIcon = !preview.mimeType.startsWith("image/") && !m_detailImageSource.isEmpty();
          if (m_detailImageSource.isEmpty() && m_detailTextContent.isEmpty()) {
            m_detailImageSource = qml::imageSourceFor(ImageURL::fileIcon(path));
            m_detailIsFileIcon = true;
          }
          m_hasDetail = true;
          emit detailChanged();
          return;
        }
      }
    }
  }

  if (mime.startsWith("image/")) {
    m_detailTitle = entry.textPreview;
    auto const cacheDir = QString::fromStdString(Omnicast::cacheDir().string());
    QDir().mkpath(cacheDir);
    QString const path = cacheDir + QStringLiteral("/clipboard-") + entry.md5sum;
    QFile f(path);
    if (!f.exists() && f.open(QIODevice::WriteOnly)) {
      f.write(rawData);
      f.close();
    }
    m_detailImageSource = qml::imageSourceFor(ImageURL::local(path));
    m_hasDetail = true;
    emit detailChanged();
    return;
  }

  if (Utils::isTextMimeType(mime) || mime == "text/uri-list") {
    static constexpr qsizetype MAX_DISPLAY = 10 * 1024;
    m_detailTextContent = QString::fromUtf8(rawData.first(qMin<qsizetype>(rawData.size(), MAX_DISPLAY)));
    m_hasDetail = true;
    emit detailChanged();
    return;
  }

  m_hasDetail = true;
  emit detailChanged();
}

void ClipboardHistoryViewHost::clearDetail() {
  if (!m_hasDetail) return;
  m_hasDetail = false;
  m_hasDetailError = false;
  m_detailTextContent.clear();
  m_detailImageSource.clear();
  m_detailType.clear();
  m_detailTitle.clear();
  m_detailIsFileIcon = false;
  m_detailCopiedAt.clear();
  m_detailEncryptionIcon.clear();
  m_detailErrorTitle.clear();
  m_detailErrorDescription.clear();
  emit detailChanged();
}
