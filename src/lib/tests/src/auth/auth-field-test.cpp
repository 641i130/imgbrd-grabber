#include <QSettings>
#include "auth/auth-const-field.h"
#include "auth/auth-field.h"
#include "auth/auth-hash-field.h"
#include "mixed-settings.h"
#include "catch.h"


MixedSettings *makeSettings(const QString &key, const QString &value)
{
	auto *settings = new QSettings("tests/resources/settings.ini", QSettings::IniFormat);
	settings->setValue(key, value);

	return new MixedSettings(QList<QSettings*>() << settings);
}


TEST_CASE("AuthField")
{
	SECTION("Basic field")
	{
		AuthField field("id", "key", AuthField::FieldType::Text);

		REQUIRE(field.id() == QString("id"));
		REQUIRE(field.key() == QString("key"));
		REQUIRE(field.type() == AuthField::FieldType::Text);

		MixedSettings *settings = makeSettings("auth/id", "user");
		REQUIRE(field.value(settings) == QString("user"));
		delete settings;
	}

	SECTION("Basic field with default value")
	{
		AuthField field("id", "key", AuthField::FieldType::Text, "default");

		REQUIRE(field.id() == QString("id"));
		REQUIRE(field.key() == QString("key"));
		REQUIRE(field.type() == AuthField::FieldType::Text);
		REQUIRE(field.def() == QString("default"));

		MixedSettings *settings = makeSettings("auth/id", "");
		REQUIRE(field.value(settings) == QString("default"));
		delete settings;
	}


	SECTION("Const field")
	{
		AuthConstField field("key", "val");

		REQUIRE(field.key() == QString("key"));
		REQUIRE(field.type() == AuthField::FieldType::Const);

		auto *settings = new MixedSettings(QList<QSettings*>());
		REQUIRE(field.value(settings) == QString("val"));
		delete settings;
	}

	SECTION("Hash field")
	{
		AuthHashField field("key", QCryptographicHash::Algorithm::Md5, "test-%pseudo%");

		REQUIRE(field.key() == QString("key"));
		REQUIRE(field.type() == AuthField::FieldType::Hash);
		REQUIRE(field.salt() == QString("test-%pseudo%"));

		auto *settings = makeSettings("auth/pseudo", "user");
		REQUIRE(field.value(settings) == QString("42b27efc1480b4fe6d7eaa5eec47424d")); // md5("test-user")
		delete settings;
	}

	SECTION("Empty hash field")
	{
		AuthHashField field("key", QCryptographicHash::Algorithm::Md5, "test-%pseudo%");

		REQUIRE(field.key() == QString("key"));
		REQUIRE(field.type() == AuthField::FieldType::Hash);
		REQUIRE(field.salt() == QString("test-%pseudo%"));

		auto *settings = new MixedSettings(QList<QSettings*>());
		REQUIRE(field.value(settings) == QString());
		delete settings;
	}
}
