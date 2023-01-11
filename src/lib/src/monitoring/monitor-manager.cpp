#include "monitoring/monitor-manager.h"
#include <QByteArray>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <utility>
#include "monitoring/monitor.h"


MonitorManager::MonitorManager(QString file, Profile *profile)
	: m_file(std::move(file)), m_profile(profile)
{
	load();
}

void MonitorManager::load()
{
	QFile file(m_file);
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		return;
	}

	const QByteArray data = file.readAll();
	QJsonDocument loadDoc = QJsonDocument::fromJson(data);
	QJsonObject object = loadDoc.object();

	QJsonArray monitors = object["monitors"].toArray();
	for (const auto &monitorJson : monitors) {
		m_monitors.append(Monitor::fromJson(monitorJson.toObject(), m_profile));
	}
}

void MonitorManager::save() const
{
	QFile file(m_file);
	if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate)) {
		return;
	}

	// Generate JSON array
	QJsonArray monitorsJson;
	for (const Monitor &monitor : m_monitors) {
		QJsonObject unique;
		monitor.toJson(unique);
		monitorsJson.append(unique);
	}

	// Generate result
	QJsonObject full;
	full["version"] = 1;
	full["monitors"] = monitorsJson;

	// Write result
	QJsonDocument saveDoc(full);
	file.write(saveDoc.toJson());
	file.close();
}

void MonitorManager::add(const Monitor &monitor, int index)
{
	if (index < 0) {
		index = m_monitors.count();
	}

	emit inserted(index);
	m_monitors.insert(index, monitor);
	save();
}

int MonitorManager::remove(const Monitor &monitor)
{
	int index = m_monitors.indexOf(monitor);
	if (index != -1) {
		emit removed(index);
		m_monitors.removeAt(index);
		save();
	}

	return index;
}

QList<Monitor> &MonitorManager::monitors()
{
	return m_monitors;
}
