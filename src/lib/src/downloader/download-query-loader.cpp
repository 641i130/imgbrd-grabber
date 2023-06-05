#include "downloader/download-query-loader.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "downloader/download-query-group.h"
#include "downloader/download-query-image.h"
#include "logger.h"
#include "utils/file-utils.h"


bool DownloadQueryLoader::load(const QString &path, QList<DownloadQueryImage> &uniques, QList<DownloadQueryGroup> &groups, Profile *profile)
{
	QFile f(path);
	if (!f.open(QFile::ReadOnly)) {
		return false;
	}

	// Get the file's header to get the version
	QString header = f.readLine().trimmed();

	// Version 1 and 2 are plain text
	if (header.startsWith("[IGL ")) {
		log(QStringLiteral("Text-based IGL files are not supported"), Logger::Warning);
		return false;
	}

	// Other versions are JSON-based
	f.reset();

	const QByteArray data = f.readAll();
	f.close();
	QJsonDocument loadDoc = QJsonDocument::fromJson(data);
	QJsonObject object = loadDoc.object();

	const int version = object["version"].toInt();
	switch (version)
	{
		case 3:
		{
			QJsonArray groupsJson = object["batchs"].toArray();
			for (auto groupJson : groupsJson) {
				DownloadQueryGroup batch;
				if (batch.read(groupJson.toObject(), profile)) {
					groups.append(batch);
				}
			}

			QJsonArray uniquesJson = object["uniques"].toArray();
			for (auto uniqueJson : uniquesJson) {
				DownloadQueryImage unique;
				if (unique.read(uniqueJson.toObject(), profile)) {
					uniques.append(unique);
				}
			}
			return true;
		}

		default:
			log(QStringLiteral("Unknown IGL file version: %1").arg(version), Logger::Warning);
			return false;
	}
}

bool DownloadQueryLoader::save(const QString &path, const QList<DownloadQueryImage> &uniques, const QList<DownloadQueryGroup> &groups, bool saveProgress)
{
	// Batch downloads
	QJsonArray groupsJson;
	for (const auto &b : groups) {
		QJsonObject batch;
		b.write(batch, saveProgress);
		groupsJson.append(batch);
	}

	// Unique images
	QJsonArray uniquesJson;
	for (const auto &u : uniques) {
		QJsonObject unique;
		u.write(unique);
		uniquesJson.append(unique);
	}

	// Generate result
	QJsonObject full;
	full["version"] = 3;
	full["batchs"] = groupsJson;
	full["uniques"] = uniquesJson;

	// Write result
	QJsonDocument saveDoc(full);
	return safeWriteFile(path, saveDoc.toJson());
}
