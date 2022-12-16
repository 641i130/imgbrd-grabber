#ifndef FILENAME_CONDITION_VISITOR_H
#define FILENAME_CONDITION_VISITOR_H

#include <QMap>
#include <QString>
#include <QStringList>
#include "filename/visitors/filename-visitor-javascript.h"


struct FilenameNodeCondition;
class QSettings;
class Token;

/**
 * Visitor to check if a condition is true or false given a set of tokens.
 * Used internally for conditionals, but can also be used to convert filenames to booleans.
 */
class FilenameConditionVisitor : public FilenameVisitorJavaScript
{
	public:
		explicit FilenameConditionVisitor(const QMap<QString, Token> &tokens, QSettings *settings);
		bool run(const FilenameNodeCondition &node);

		void visit(const FilenameNodeConditionInvert &node) override;
		void visit(const FilenameNodeConditionJavaScript &node) override;
		void visit(const FilenameNodeConditionOp &node) override;
		void visit(const FilenameNodeConditionTag &node) override;
		void visit(const FilenameNodeConditionToken &node) override;

	private:
		const QMap<QString, Token> &m_tokens;
		QStringList m_tags;

		bool m_result = false;
};

#endif // FILENAME_CONDITION_VISITOR_H
