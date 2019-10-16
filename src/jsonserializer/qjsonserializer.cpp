#include "qjsonserializer.h"
#include "qjsonserializer_p.h"

#include <QtCore/QBuffer>

QJsonSerializer::QJsonSerializer(QObject *parent) :
	  QJsonSerializerBase{*new QJsonSerializerPrivate{}, parent}
{}

QJsonValue QJsonSerializer::serialize(const QVariant &data) const
{
	return serializeVariant(data.userType(), data).toJsonValue();
}

void QJsonSerializer::serializeTo(QIODevice *device, const QVariant &data, QJsonDocument::JsonFormat format) const
{
	QJsonDocument doc;
	const auto jData = serialize(data);
	if (jData.isArray())
		doc = QJsonDocument{jData.toArray()};
	else if (jData.isObject())
		doc = QJsonDocument{jData.toObject()};
	else
		throw QJsonSerializationException("Only objects or arrays can be written to a device!");
	device->write(doc.toJson(format));
}

QByteArray QJsonSerializer::serializeTo(const QVariant &data, QJsonDocument::JsonFormat format) const
{
	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	serializeTo(&buffer, data, format);
	buffer.close();
	return buffer.data();
}

QVariant QJsonSerializer::deserialize(const QJsonValue &json, int metaTypeId, QObject *parent) const
{
	return deserializeVariant(metaTypeId, QCborValue::fromJsonValue(json), parent);
}

QVariant QJsonSerializer::deserializeFrom(QIODevice *device, int metaTypeId, QObject *parent) const
{
	QJsonParseError error;
	auto doc = QJsonDocument::fromJson(device->readAll(), &error);
	if (error.error != QJsonParseError::NoError)
		throw QJsonDeserializationException{"Failed to read file as JSON with error: " + error.errorString().toUtf8()};
	if (doc.isArray())
		return deserialize(doc.array(), metaTypeId, parent);
	else
		return deserialize(doc.object(), metaTypeId, parent);
}

QVariant QJsonSerializer::deserializeFrom(const QByteArray &data, int metaTypeId, QObject *parent) const
{
	QBuffer buffer(const_cast<QByteArray*>(&data));
	buffer.open(QIODevice::ReadOnly);
	auto res = deserializeFrom(&buffer, metaTypeId, parent);
	buffer.close();
	return res;
}

bool QJsonSerializer::validateBase64() const
{
	Q_D(const QJsonSerializer);
	return d->validateBase64;
}

void QJsonSerializer::setValidateBase64(bool validateBase64)
{
	Q_D(QJsonSerializer);
	if(d->validateBase64 == validateBase64)
		return;

	d->validateBase64 = validateBase64;
	emit validateBase64Changed(d->validateBase64, {});
}

bool QJsonSerializer::jsonMode() const
{
	return true;
}

QCborTag QJsonSerializer::typeTag(int metaTypeId) const
{
	Q_UNUSED(metaTypeId)
	return QJsonTypeConverter::NoTag;
}

QList<int> QJsonSerializer::typesForTag(QCborTag tag) const
{
	Q_UNUSED(tag)
	return {};
}
