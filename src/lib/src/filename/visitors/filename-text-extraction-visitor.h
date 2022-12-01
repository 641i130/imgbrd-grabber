#ifndef FILENAME_TEXT_EXTRACTION_VISITOR_H
#define FILENAME_TEXT_EXTRACTION_VISITOR_H

#include <QList>
#include <QString>
#include "filename/ast/filename-visitor-base.h"


struct FilenameNodeRoot;
struct FilenameNodeText;

/**
 * Visitor to extract all raw text in a given filename.
 * Useful to detect invalid characters in a filename.
 */
class FilenameTextExtractionVisitor : public FilenameVisitorBase
{
	public:
		QStringList run(const FilenameNodeRoot &node);

		void visit(const FilenameNodeText &node) override;

	private:
		QStringList m_results;
};

#endif // FILENAME_TEXT_EXTRACTION_VISITOR_H
