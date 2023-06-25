#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <QVariantMap>
#include "vendor/ganalytics.h"
#include "vendor/qt-google-analytics/qt-google-analytics.h"


class QString;
class QVariant;

class Analytics
{
	public:
		// Singleton pattern
		static Analytics &getInstance()
		{
			static Analytics instance;
			return instance;
		}
		Analytics(Analytics const &) = delete;
		void operator=(Analytics const &) = delete;

		// Setters
		void setTrackingID(const QString& trackingId);
		void setMeasurementID(const QString& measurementId);
		void setEnabled(bool enabled);

		// API
		void startSending();
		void startSession();
		void endSession();
		void sendScreenView(const QString& screenName, const QVariantMap &customValues = {});
		void sendEvent(const QString& category, const QString &action, const QString &label = {}, const QVariant &value = {}, const QVariantMap &customValues = {});

	private:
		Analytics() = default;

		bool m_enabled = false;
		GAnalytics m_googleAnalytics;
		QtGoogleAnalytics m_ga4;
};

#endif // ANALYTICS_H
