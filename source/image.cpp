#include "image.h"
#include "functions.h"



Image::Image(QMap<QString, QString> details, int timezonedecay, Page* parent)
{
	m_site = parent->website();
	m_url = details.contains("file_url") ? (details["file_url"].startsWith("/") ? "http://"+m_site+details["file_url"] : details["file_url"]) : "";
	m_md5 = details.contains("md5") ? details["md5"] : "";
	m_author = details.contains("author") ? details["author"] : "";
	m_status = details.contains("status") ? details["status"] : "";
	m_rating = details.contains("rating") ? details["rating"] : "";
	QStringMap assoc;
		assoc["s"] = tr("Safe");
		assoc["q"] = tr("Questionable");
		assoc["e"] = tr("Explicit");
	if (assoc.contains(m_rating))
	{ m_rating = assoc[m_rating]; }
	m_source = details.contains("source") ? details["source"] : "";
	m_tags = QList<Tag*>();
	if (details.contains("tags"))
	{
		QStringList t = details["tags"].split(" ");
		for (int i = 0; i < t.count(); i++)
		{
			QString tg = t.at(i);
			tg.replace("&amp;", "&");
			m_tags.append(new Tag(tg));
		}
	}
	m_id = details.contains("id") ? details["id"].toInt() : 0;
	m_score = details.contains("score") ? details["score"].toInt() : 0;
	m_hasScore = details.contains("score");
	m_parentId = details.contains("parent_id") ? details["parent_id"].toInt() : 0;
	m_fileSize = details.contains("file_size") ? details["file_size"].toInt() : 0;
	m_authorId = details.contains("creator_id") ? details["creator_id"].toInt() : 0;
	m_createdAt = QDateTime();
	if (details.contains("created_at"))
	{
		if (details["created_at"].toInt() != 0)
		{ m_createdAt.setTime_t(details["created_at"].toInt()); }
		else
		{ m_createdAt = qDateTimeFromString(details["created_at"], timezonedecay); }
	}
	m_hasChildren = details.contains("has_children") ? details["has_children"] == "true" : false;
	m_hasNote = details.contains("has_note") ? details["has_note"] == "true" : false;
	m_hasComments = details.contains("has_comments") ? details["has_comments"] == "true" : false;
	m_pageUrl = details.contains("page_url") ? QUrl(details["page_url"]) : QUrl();
	m_fileUrl = details.contains("file_url") ? QUrl(details["file_url"].startsWith("/") ? "http://"+m_site+details["file_url"] : details["file_url"]) : QUrl();
	m_sampleUrl = details.contains("sample_url") ? QUrl(details["sample_url"].startsWith("/") ? "http://"+m_site+details["sample_url"] : details["sample_url"]) : QUrl();
	m_previewUrl = details.contains("preview_url") ? QUrl(details["preview_url"].startsWith("/") ? "http://"+m_site+details["preview_url"] : details["preview_url"]) : QUrl();
	m_size = QSize(details.contains("width") ? details["width"].toInt() : 0, details.contains("height") ? details["height"].toInt() : 0);
	m_parent = parent;

	m_loadPreviewExists = false;
	m_loadTagsExists = false;
}
Image::~Image()
{ delete &m_imagePreview; }

void Image::loadPreview()
{
	m_loadPreviewExists = true;
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(parsePreview(QNetworkReply*)));
	QNetworkRequest r(m_previewUrl);
		r.setRawHeader("Referer", m_previewUrl.toString().toAscii());
	m_loadPreview = manager->get(r);
}
void Image::abortPreview()
{
	if (m_loadPreviewExists)
	{
		if (m_loadPreview->isRunning())
		{ m_loadPreview->abort(); }
	}
}
void Image::parsePreview(QNetworkReply* r)
{
	// Check redirection
	QUrl redir = r->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if (!redir.isEmpty())
	{
		m_previewUrl = redir;
		loadPreview();
		return;
	}

	// Load preview from raw result
	m_imagePreview.loadFromData(r->readAll());
	if (m_imagePreview.isNull())
	{
		log(tr("<b>Attention :</b> %1").arg(tr("une des vignettes est vide (<a href=\"%1\">%1</a>). Nouvel essai...").arg(m_previewUrl.toString())));
		loadPreview();
	}
	else
	{ emit finishedLoadingPreview(this); }
}

void Image::loadTags()
{
	m_loadPreviewExists = true;
	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseTags(QNetworkReply*)));
	QNetworkRequest r(m_pageUrl);
		r.setRawHeader("Referer", m_pageUrl.toString().toAscii());
	m_loadTags = manager->get(r);
}
void Image::abortTags()
{
	if (m_loadTags->isRunning())
	{ m_loadTags->abort(); }
}
void Image::parseTags(QNetworkReply* r)
{
	// Check redirection
	QUrl redir = r->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if (!redir.isEmpty())
	{
		m_pageUrl = redir;
		loadTags();
		return;
	}

	if (m_parent->site().contains("Regex/Tags"))
	{
		QString source = r->readAll();
		QRegExp rx(m_parent->site().value("Regex/Tags"));
		rx.setMinimal(true);
		int pos = 0;
		QList<Tag*> tgs;
		while ((pos = rx.indexIn(source, pos)) != -1)
		{
			pos += rx.matchedLength();
			QString type = rx.cap(1), tag = rx.cap(2).replace(" ", "_").replace("&amp;", "&");
			int count = rx.cap(3).toInt();
			tgs.append(new Tag(tag, type, count));
		}
		if (!tgs.isEmpty())
		{ m_tags = tgs; }
	}
	emit finishedLoadingTags(this);
}

QString Image::filter(QStringList filters)
{
	QStringList types = QStringList() << "rating" << "source" << "id" << "width" << "height" << "score" << "mpixels";
	bool invert;
	QString filter, type;
	for (int i = 0; i < filters.count(); i++)
	{
		invert = false;
		filter = filters.at(i);
		if (filter.startsWith('-'))
		{
			filter = filter.right(filter.length()-1);
			invert = true;
		}
		if (filter.contains(":"))
		{
			type = filter.section(':', 0, 0).toLower();
			filter = filter.section(':', 1).toLower();
			if (!types.contains(type))
			{ return QObject::tr("unknown type \"%1\" (available types: \"%2\")").arg(filter, types.join("\", \"")); }
			if (type == "rating")
			{
				bool cond = m_rating.toLower().startsWith(filter.left(1));
				if (!cond && !invert)
				{ return QObject::tr("image is not \"%1\"").arg(filter); }
				if (cond && invert)
				{ return QObject::tr("image is \"%1\"").arg(filter); }
			}
			else if (type == "source")
			{
				QRegExp rx = QRegExp(filter+"*", Qt::CaseInsensitive, QRegExp::Wildcard);
				bool cond = rx.exactMatch(m_source);
				if (!cond && !invert)
				{ return QObject::tr("image's source does not starts with \"%1\"").arg(filter); }
				if (cond && invert)
				{ return QObject::tr("image's source starts with \"%1\"").arg(filter); }
			}
			else if (type == "id" || type == "width" || type == "height" || type == "score" || type == "mpixels")
			{
				int input = 0;
				if (type == "id")		{ input = m_id;								}
				if (type == "width")	{ input = m_size.width();					}
				if (type == "height")	{ input = m_size.height();					}
				if (type == "score")	{ input = m_score;							}
				if (type == "mpixels")	{ input = m_size.width()*m_size.height();	}

				bool cond;
				if (filter.startsWith("..") || filter.startsWith("<="))
				{ cond = input <= filter.right(filter.size()-2).toInt(); }
				else if (filter.endsWith  ("..") || filter.startsWith(">="))
				{ cond = input >= filter.right(filter.size()-2).toInt(); }
				else if (filter.startsWith("<"))
				{ cond = input < filter.right(filter.size()-1).toInt(); }
				else if (filter.startsWith(">"))
				{ cond = input > filter.right(filter.size()-1).toInt(); }
				else if (filter.contains(".."))
				{ cond = input >= filter.left(filter.indexOf("..")).toInt() && input <= filter.right(filter.size()-filter.indexOf("..")-2).toInt();	}
				else
				{ cond = input == filter.toInt(); }

				if (!cond && !invert)
				{ return QObject::tr("image's %1 does not match").arg(type); }
				if (cond && invert)
				{ return QObject::tr("image's %1 match").arg(type); }
			}
		}
		else if (!filter.isEmpty())
		{
			bool cond = false;
			for (int t = 0; t < m_tags.count(); t++)
			{
				if (m_tags.at(t)->text().toLower() == filter.toLower())
				{ cond = true; break; }
			}
			if (!cond && !invert)
			{ return QObject::tr("image does not contains \"%1\"").arg(filter); }
			if (cond && invert)
			{ return QObject::tr("image contains \"%1\"").arg(filter); }
		}
	}
	return QString();
}



QString analyse(QStringList tokens, QString text, QStringList tags)
{
	QString ret = text;
	QRegExp reg = QRegExp("\\<([^>]+)\\>");
	int pos = 0;
	while ((pos = reg.indexIn(text, pos)) != -1)
	{
		QString cap = reg.cap(1);
		if (!cap.isEmpty())
		{
			cap += QString(">").repeated(cap.count('<')-cap.count('>'));
			ret.replace("<"+cap+">", analyse(tokens, cap, tags));
		}
		pos += reg.matchedLength()+cap.count('<')-cap.count('>');
	}
	QString r = ret;
	for (int i = 0; i < tokens.size(); i++)
	{ r.replace("%"+tokens.at(i)+"%", ""); }
	reg = QRegExp("\"([^\"]+)\"");
	pos = 0;
	while ((pos = reg.indexIn(text, pos)) != -1)
	{
		if (!reg.cap(1).isEmpty() && tags.contains(reg.cap(1)))
		{ ret.replace(reg.cap(0), reg.cap(1)); }
		pos += reg.matchedLength();
	}
	return r.contains("%") || ret.contains("\"") ? "" : ret;
}

QString Image::path(QString fn)
{
	QSettings settings(savePath("settings.ini"), QSettings::IniFormat);
	settings.beginGroup("Save");
	if (fn.isEmpty())
	{ fn = settings.value("filename").toString(); }
	QStringList copyrights;
	QString cop;
	bool found;
	QMap<QString,QStringList> custom = QMap<QString,QStringList>(), scustom = getCustoms();
	QMap<QString,QStringList> details;
	QStringList ignore = loadIgnored();
	for (int i = 0; i < m_tags.size(); i++)
	{
		QString t = m_tags.at(i)->text();
		for (int r = 0; r < scustom.size(); r++)
		{
			if (!custom.contains(scustom.keys().at(r)))
			{ custom.insert(scustom.keys().at(r), QStringList()); }
			if (scustom.values().at(r).contains(t))
			{ custom[scustom.keys().at(r)].append(t); }
		}
		details["allos"].append(t);
		t = t.replace("\\", "_").replace("%", "_").replace("/", "_").replace(":", "_").replace("|", "_").replace("*", "_").replace("?", "_").replace("\"", "_").replace("<", "_").replace(">", "_").replace("__", "_").replace("__", "_").replace("__", "_").trimmed();
		if (!settings.value("replaceblanks", false).toBool())
		{ t.replace("_", " "); }
		details[ignore.contains(m_tags.at(i)->text(), Qt::CaseInsensitive) ? "generals" : m_tags.at(i)->type()+"s"].append(t);
		details["alls"].append(t);
	}
	if (settings.value("copyright_useshorter", true).toBool())
	{
		for (int i = 0; i < details["copyrights"].size(); i++)
		{
			found = false;
			cop = details["copyrights"].at(i);
			for (int r = 0; r < copyrights.size(); r++)
			{
				if (copyrights.at(r).left(cop.size()) == cop.left(copyrights.at(r).size()))
				{
					if (cop.size() < copyrights.at(r).size())
					{ copyrights[r] = cop; }
					found = true;
				}
			}
			if (!found)
			{ copyrights.append(cop); }
		}
	}
	else
	{ copyrights = details["copyrights"]; }
	QStringList search = m_parent->search();
	QString filename = fn, pth = settings.value("path").toString().replace("\\", "/");
	filename.replace("\\", "/");
	if (filename.left(1) == "/")	{ filename = filename.right(filename.length()-1);	}
	if (pth.right(1) == "/")		{ pth = pth.left(pth.length()-1);					}
	int i = 1;
	while (filename.contains("%search_"+QString::number(i)+"%"))
	{
		filename.replace("%search_"+QString::number(i)+"%", (search.size() >= i ? search[i-1] : "").left(259-pth.length()-1-filename.length()));
		i++;
	}

	QString ext = m_url.section('.', -1).left(259-pth.length()-1-filename.length());
	if (ext.length() > 5)
	{ ext = "jpg"; }
	QStringList rem = (filename.contains("%artist%") ? details["artists"] : QStringList()) +
		(filename.contains("%copyright%") ? copyrights : QStringList()) +
		(filename.contains("%character%") ? details["characters"] : QStringList()) +
		(filename.contains("%model%") ? details["models"] : QStringList()) +
		(filename.contains("%general%") ? details["generals"] : QStringList());

	QStringList tokens = QStringList() << "artist" << "general" << "copyright" << "character" << "model" << "model|artist" << "filename" << "rating" << "md5" << "website" << "ext" << "all" << "id" << "search" << "allo" << custom.keys();
	filename = analyse(tokens, filename, details["allos"]);

	for (int i = 0; i < custom.size(); i++)
	{ filename.replace("%"+custom.keys().at(i)+"%", custom.values().at(i).join(settings.value("separator").toString()).left(259-pth.length()-1-filename.length())); }
	filename.replace("%artist%", (details["artists"].isEmpty() ? settings.value("artist_empty").toString() : (settings.value("artist_useall").toBool() || details["artists"].count() == 1 ? details["artists"].join(settings.value("artist_sep").toString()) : settings.value("artist_value").toString())).left(259-pth.length()-1-filename.length()));
	filename.replace("%copyright%", (copyrights.isEmpty() ? settings.value("copyright_empty").toString() : (settings.value("copyright_useall").toBool() || copyrights.count() == 1 ? copyrights.join(settings.value("copyright_sep").toString()) : settings.value("copyright_value").toString())).left(259-pth.length()-1-filename.length()));
	filename.replace("%character%", (details["characters"].isEmpty() ? settings.value("character_empty").toString() : (settings.value("character_useall").toBool() || details["characters"].count() == 1 ? details["characters"].join(settings.value("character_sep").toString()) : settings.value("character_value").toString())).left(259-pth.length()-1-filename.length()));
	filename.replace("%model%", (details["models"].isEmpty() ? settings.value("model_empty").toString() : (settings.value("model_useall").toBool() || details["models"].count() == 1 ? details["models"].join(settings.value("model_sep").toString()) : settings.value("model_value").toString())).left(259-pth.length()-1-filename.length()));
	filename.replace("%model|artist%", (!details["models"].isEmpty() ? (settings.value("model_useall").toBool() || details["models"].count() == 1 ? details["models"].join(settings.value("model_sep").toString()) : settings.value("model_value").toString()) : (details["artists"].isEmpty() ? settings.value("artist_empty").toString() : (settings.value("artist_useall").toBool() || details["artists"].count() == 1 ? details["artists"].join(settings.value("artist_sep").toString()) : settings.value("artist_value").toString()))).left(259-pth.length()-1-filename.length()));
	filename.replace("%search%", search.join(settings.value("separator").toString()).left(259-pth.length()-1-filename.length()));
	filename.replace("%filename%", m_url.section('/', -1).section('.', 0, -2).left(259-pth.length()-1-filename.length()));
	filename.replace("%rating%", m_rating.left(259-pth.length()-1-filename.length()));
	filename.replace("%md5%", m_md5.left(259-pth.length()-1-filename.length()));
	filename.replace("%id%", QString::number(m_id).left(259-pth.length()-1-filename.length()));
	filename.replace("%website%", m_site.left(259-pth.length()-1-filename.length()));
	filename.replace("%height%", QString::number(m_size.height()).left(259-pth.length()-1-filename.length()));
	filename.replace("%width%", QString::number(m_size.width()).left(259-pth.length()-1-filename.length()));
	filename.replace("%ext%", ext);
	filename.replace("%general%", details["generals"].join(settings.value("separator").toString()).left(259-pth.length()-1-filename.length()));
	QStringList l = details["alls"];
	for (int i = 0; i < rem.size(); i++)
	{ l.removeAll(rem.at(i)); }
	filename.replace("%all%", l.join(settings.value("separator").toString()).left(259-pth.length()-1-filename.length()));
	filename.replace("%allo%", details["allos"].join(" "));
	while (filename.indexOf("//") >= 0)
	{ filename.replace("//", "/"); }
	if (filename.length() > settings.value("limit").toInt() && settings.value("limit").toInt() > 0)
	{ filename = filename.left(filename.length()-ext.length()-1).left(settings.value("limit").toInt()-ext.length()-1) + filename.right(ext.length()+1); }
	return QDir::toNativeSeparators(filename);
}

int Image::value()
{
	int pixels;
	if (!m_size.isEmpty())
	{ pixels = m_size.width()*m_size.height(); }
	else
	{
		pixels = 1200*900;
		QStringList tags;
		for (int t = 0; t < m_tags.size(); t++)
		{ tags.append(m_tags.at(t)->text().toLower()); }
		if (tags.contains("incredibly_absurdres"))	{ pixels = 10000*10000; }
		else if (tags.contains("absurdres"))		{ pixels = 3200*2400; }
		else if (tags.contains("highres"))			{ pixels = 1600*1200; }
		else if (tags.contains("lowres"))			{ pixels = 500*500; }
	}
	return pixels;
}



QString		Image::url()			{ return m_url;				}
QString		Image::md5()			{ return m_md5;				}
QString		Image::author()			{ return m_author;			}
QString		Image::status()			{ return m_status;			}
QString		Image::rating()			{ return m_rating;			}
QString		Image::source()			{ return m_source;			}
QString		Image::site()			{ return m_site;			}
QList<Tag*>	Image::tags()			{ return m_tags;			}
int			Image::id()				{ return m_id;				}
int			Image::score()			{ return m_score;			}
int			Image::parentId()		{ return m_parentId;		}
int			Image::fileSize()		{ return m_fileSize;		}
int			Image::width()			{ return m_size.width();	}
int			Image::height()			{ return m_size.height();	}
int			Image::authorId()		{ return m_authorId;		}
QDateTime	Image::createdAt()		{ return m_createdAt;		}
bool		Image::hasChildren()	{ return m_hasChildren;		}
bool		Image::hasNote()		{ return m_hasNote;			}
bool		Image::hasComments()	{ return m_hasComments;		}
bool		Image::hasScore()		{ return m_hasScore;		}
QUrl		Image::fileUrl()		{ return m_fileUrl;			}
QUrl		Image::sampleUrl()		{ return m_sampleUrl;		}
QUrl		Image::previewUrl()		{ return m_previewUrl;		}
QUrl		Image::pageUrl()		{ return m_pageUrl;			}
QSize		Image::size()			{ return m_size;			}
QPixmap		Image::previewImage()	{ return m_imagePreview;	}
Page		*Image::page()			{ return m_parent;			}

void	Image::setUrl(QString u)	{ m_url = u;	}
