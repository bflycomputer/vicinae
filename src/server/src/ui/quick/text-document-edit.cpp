#include <QTextBlockFormat>
#include <QTextCursor>
#include <QTextDocument>

#include "ui/quick/text-document-edit.hpp"

TextDocumentEdit::TextDocumentEdit(QObject *parent) : QObject(parent) {}

void TextDocumentEdit::replace(QQuickTextDocument *document, int start, int end, const QString &text) {
  auto *doc = document ? document->textDocument() : nullptr;
  if (!doc) return;

  QTextCursor cursor(doc);
  cursor.setPosition(start);
  cursor.setPosition(end, QTextCursor::KeepAnchor);
  cursor.insertText(text);
}

void TextDocumentEdit::setLineHeight(QQuickTextDocument *document, qreal height) {
  auto *doc = document ? document->textDocument() : nullptr;
  if (!doc) return;

  QTextBlockFormat format;
  format.setLineHeight(height, height > 0 ? QTextBlockFormat::FixedHeight : QTextBlockFormat::SingleHeight);
  QTextCursor cursor(doc);
  cursor.select(QTextCursor::Document);
  cursor.mergeBlockFormat(format);
}
