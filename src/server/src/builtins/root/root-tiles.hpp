#pragma once
#include <QFileSystemWatcher>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <optional>
#include <vector>

/**
 * Tiles other programs put on the empty root screen, by dropping a JSON file in a `root-tiles`
 * data directory:
 *
 * {"title": "report.pdf", "subtitle": "Move to Documents", "icon": "folder",
 *  "activate": ["mytool", "accept", "42"], "dismiss": ["mytool", "dismiss", "42"]}
 *
 * `activate` and `dismiss` are argument vectors (never run through a shell), `dismiss` is optional.
 */
class RootTiles : public QObject {
  Q_OBJECT

signals:
  void changed();

public:
  struct Tile {
    QString id;
    QString title;
    QString subtitle;
    QString icon;
    QStringList activate;
    std::optional<QStringList> dismiss;
  };

  explicit RootTiles(QObject *parent = nullptr);

  std::optional<Tile> first() const;
  void activate(const QString &id) const;
  void dismiss(const QString &id) const;

private:
  void watch();
  void reload();

  QFileSystemWatcher m_watcher;
  QTimer m_debounce;
  std::vector<Tile> m_tiles;
};
