#include "builtins/root/root-tiles.hpp"
#include "ui/image/url.hpp"
#include "vicinae.hpp"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <algorithm>
#include <chrono>

static constexpr auto TILES_DIR = "root-tiles";

static std::optional<QStringList> argv(const QJsonValue &value) {
  if (!value.isArray()) return std::nullopt;
  QStringList args;
  for (const auto &arg : value.toArray()) {
    if (!arg.isString()) return std::nullopt;
    args.emplace_back(arg.toString());
  }
  if (args.isEmpty()) return std::nullopt;
  return args;
}

static void run(const QStringList &command) { QProcess::startDetached(command.first(), command.mid(1)); }

RootTiles::RootTiles(QObject *parent) : QObject(parent) {
  using namespace std::chrono_literals;

  const auto dirs = Omnicast::dataSearchPaths(TILES_DIR);
  QDir().mkpath(QString::fromStdString(dirs.front().string()));

  m_debounce.setSingleShot(true);
  m_debounce.setInterval(100ms);
  connect(&m_debounce, &QTimer::timeout, this, [this]() {
    reload();
    emit changed();
  });
  const auto schedule = [this]() {
    watch();
    m_debounce.start();
  };
  connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, schedule);
  connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, schedule);

  watch();
  reload();
}

void RootTiles::watch() {
  for (const auto &dir : Omnicast::dataSearchPaths(TILES_DIR)) {
    const auto path = QString::fromStdString(dir.string());
    if (!QDir(path).exists()) continue;
    if (!m_watcher.directories().contains(path)) m_watcher.addPath(path);
    for (const auto &file : QDir(path).entryInfoList({"*.json"}, QDir::Files)) {
      if (!m_watcher.files().contains(file.filePath())) m_watcher.addPath(file.filePath());
    }
  }
}

void RootTiles::reload() {
  m_tiles.clear();

  for (const auto &dir : Omnicast::dataSearchPaths(TILES_DIR)) {
    const QDir qdir(QString::fromStdString(dir.string()));
    for (const auto &info : qdir.entryInfoList({"*.json"}, QDir::Files, QDir::Name)) {
      QFile file(info.filePath());
      if (!file.open(QIODevice::ReadOnly)) continue;

      const auto doc = QJsonDocument::fromJson(file.readAll());
      if (!doc.isObject()) continue;

      const auto obj = doc.object();
      const auto activate = argv(obj.value("activate"));
      const auto title = obj.value("title").toString();
      if (!activate || title.isEmpty()) continue;

      const auto icon = ImageURL::builtinByName(obj.value("icon").toString());
      m_tiles.emplace_back(Tile{.id = info.completeBaseName(),
                                .title = title,
                                .subtitle = obj.value("subtitle").toString(),
                                .icon = icon.toString(),
                                .activate = *activate,
                                .dismiss = argv(obj.value("dismiss"))});
    }
  }
}

std::optional<RootTiles::Tile> RootTiles::first() const {
  if (m_tiles.empty()) return std::nullopt;
  return m_tiles.front();
}

void RootTiles::activate(const QString &id) const {
  const auto it = std::ranges::find(m_tiles, id, &Tile::id);
  if (it != m_tiles.end()) run(it->activate);
}

void RootTiles::dismiss(const QString &id) const {
  const auto it = std::ranges::find(m_tiles, id, &Tile::id);
  if (it != m_tiles.end() && it->dismiss) run(*it->dismiss);
}
