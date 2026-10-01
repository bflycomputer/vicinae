#include <QCoreApplication>
#include <QDateTime>
#include <QFileInfo>
#include <qlogging.h>
#include <qnamespace.h>

#include "builtins/clipboard/history/clipboard-history-model.hpp"
#include "actions/app-actions.hpp"
#include "common/context.hpp"
#include "builtins/clipboard/history/clipboard-history-actions.hpp"
#include "internal/keyboard/keybind.hpp"
#include "navigation-controller.hpp"
#include "services/clipboard/clipboard-db.hpp"
#include "ui/settings/settings-controller.hpp"
#include "service-registry.hpp"
#include "services/clipboard/clipboard-service.hpp"
#include "services/paste/paste-service.hpp"
#include "ui/action-panel/action.hpp"
#include "utils/utils.hpp"

ClipboardHistorySection::ClipboardHistorySection(QString name, std::span<const ClipboardHistoryEntry> entries,
                                                 ClipboardService *clipboard)
    : m_name(std::move(name)) {
  m_entries.reserve(entries.size());
  for (const auto &entry : entries) {
    auto &item = m_entries.emplace_back(entry);
    if (item.kind == ClipboardOfferKind::Link && item.urlHost) item.textPreview = *item.urlHost;
    if (item.kind != ClipboardOfferKind::File || !clipboard) continue;
    if (auto data = clipboard->getMainOfferData(item.id)) {
      QStringList names;
      for (const auto &line : QString::fromUtf8(*data).split('\n', Qt::SkipEmptyParts)) {
        const QUrl url(line.trimmed());
        if (url.isLocalFile()) names.append(QFileInfo(url.toLocalFile()).fileName());
      }
      if (!names.empty()) item.textPreview = names.join(", ");
    }
  }
}

QString ClipboardHistorySection::itemId(int i) const { return m_entries[i].id; }

QString ClipboardHistorySection::itemTitle(int i) const { return m_entries[i].textPreview; }

QString ClipboardHistorySection::itemSubtitle(int i) const {
  auto dt = QDateTime::fromSecsSinceEpoch(m_entries[i].updatedAt);
  return getRelativeTimeString(dt);
}

std::optional<ImageURL> ClipboardHistorySection::itemIcon(int i) const { return iconForEntry(m_entries[i]); }

ImageURL ClipboardHistorySection::iconForEntry(const ClipboardHistoryEntry &entry) const {
  switch (entry.kind) {
  case ClipboardOfferKind::Image:
    return ImageURL::builtinByName(u"clipboard-image");
  case ClipboardOfferKind::Link:
    return ImageURL::builtinByName(u"clipboard-link");
  case ClipboardOfferKind::Text:
    return ImageURL::builtinByName(u"clipboard-text");
  case ClipboardOfferKind::File:
    return ImageURL::builtinByName(u"clipboard-file");
  default:
    return ImageURL::builtin(BuiltinIcon::QuestionMarkCircle);
  }
}

bool ClipboardHistorySection::isDraggable(int idx) const { return true; }

std::unique_ptr<QMimeData> ClipboardHistorySection::dragMimeData(int idx) const {
  auto clipman = scope().services()->clipman();
  auto selection = clipman->retrieveSelectionById(m_entries[idx].id);

  if (!selection) return nullptr;

  return clipman->dragMimeDataForSelection(*std::move(selection));
}

std::unique_ptr<ActionPanelState> ClipboardHistorySection::actionPanel(int i) const {
  const auto &entry = m_entries[i];
  auto panel = std::make_unique<ListActionPanelState>();
  auto clipman = scope().services()->clipman();
  auto mainSection = panel->createSection();
  auto appDb = scope().services()->appDb();
  bool const isCopyable = entry.encryption == ClipboardEncryptionType::None || clipman->isEncryptionReady();

  if (!isCopyable) {
    mainSection->addAction(
        new StaticAction(QCoreApplication::translate("ClipboardHistorySection", "Open Settings"),
                         BuiltinIcon::Cog, [](ApplicationContext *ctx) {
                           ctx->settings->openTab(QStringLiteral("advanced"));
                           ctx->navigation->closeWindow();
                         }));
  }

  auto pasteService = scope().services()->pasteService();
  auto pin = new PinClipboardAction(entry.id, !entry.pinnedAt);
  auto editKeywords = new EditClipboardKeywordsAction(entry.id);
  auto remove = new RemoveSelectionAction(entry.id);
  auto removeAll = new RemoveAllSelectionsAction();

  editKeywords->setShortcut(Keybind::EditAction);
  remove->setStyle(AbstractAction::Style::Danger);
  remove->setShortcut(Keybind::RemoveAction);
  removeAll->setShortcut(Keybind::DangerousRemoveAction);
  pin->setShortcut(Keybind::PinAction);

  if (isCopyable) {
    auto copy = new CopyClipboardSelection(entry.id);
    copy->addShortcut(Keybind::CopyAction);

    if (pasteService->supportsPaste()) {
      auto paste = new PasteClipboardSelection(entry.id);
      paste->addShortcut(Keybind::PasteAction);
      if (m_defaultAction == DefaultAction::Copy) {
        mainSection->addAction(copy);
        mainSection->addAction(paste);
      } else {
        mainSection->addAction(paste);
        mainSection->addAction(copy);
      }
    } else {
      mainSection->addAction(copy);
    }
  }

  {
    if (entry.kind == ClipboardOfferKind::File) {
      if (const auto data = clipman->getMainOfferData(entry.id)) {
        auto urls = QString{*data}.split("\r\n", Qt::SkipEmptyParts);
        if (urls.size() == 1) {
          if (QUrl url{urls.front()}; QFile::exists(url.path())) {
            if (auto app = appDb->findDefaultOpener(url.path())) {
              auto open = new OpenAppAction(app, "Open", {url.path()});
              open->setShortcut(Keybind::OpenAction);
              mainSection->addAction(open);
            }

            auto openWith = new OpenWithAction(url.path());
            openWith->setShortcut(Keyboard::Shortcut(Qt::Key_O, Qt::ControlModifier).shifted());
            mainSection->addAction(openWith);
          }
        }
      }
    }

    else if (entry.kind == ClipboardOfferKind::Link) {
      if (const auto data = clipman->getMainOfferData(entry.id)) {
        if (auto app = appDb->findDefaultOpener(*data)) {
          auto open = new OpenAppAction(app, "Open", {*data});
          open->setShortcut(Keybind::OpenAction);
          mainSection->addAction(open);
        }

        auto openWith = new OpenWithAction(*data);
        openWith->setShortcut(Keyboard::Shortcut(Qt::Key_O, Qt::ControlModifier).shifted());
        mainSection->addAction(openWith);
      }
    }
  }

  auto toolsSection = panel->createSection();
  auto dangerSection = panel->createSection();
  toolsSection->addAction(pin);
  toolsSection->addAction(editKeywords);
  if (clipman->supportsMonitoring() && m_onToggleMonitoring) {
    const bool monitoring = clipman->monitoring();
    auto *toggle = new StaticAction(
        monitoring ? QCoreApplication::translate("ClipboardHistorySection", "Pause clipboard")
                   : QCoreApplication::translate("ClipboardHistorySection", "Resume clipboard"),
        ImageURL::builtin(monitoring ? BuiltinIcon::PauseFilled : BuiltinIcon::PlayFilled),
        m_onToggleMonitoring);
    toggle->setAutoClose(false);
    toolsSection->addAction(toggle);
  }
  dangerSection->addAction(remove);
  dangerSection->addAction(removeAll);

  return panel;
}
