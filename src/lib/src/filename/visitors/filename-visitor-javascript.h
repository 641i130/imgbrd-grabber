#ifndef FILENAME_VISITOR_JAVASCRIPT_H
#define FILENAME_VISITOR_JAVASCRIPT_H

#include <QMap>
#include <QString>
#include "filename/ast/filename-visitor-base.h"


class QJSEngine;
class QJSValue;
class QSettings;
class Token;

/**
 * Base class for a visitor to be able to execute JavaScript code.
 */
class FilenameVisitorJavaScript : public FilenameVisitorBase
{
	public:
		explicit FilenameVisitorJavaScript(QSettings *settings);

	protected:
		void setJavaScriptVariables(QJSEngine &engine, const QMap<QString, Token> &tokens, QJSValue obj) const;
		QString separator(const QString &key) const;

	private:
		QSettings *m_settings;
};

#endif // FILENAME_VISITOR_JAVASCRIPT_H
