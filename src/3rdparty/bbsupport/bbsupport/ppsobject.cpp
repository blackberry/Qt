/****************************************************************************
**
** Copyright (C) 2011 Research In Motion Limited
**
** This file is part of the plugins of the Qt Toolkit.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
****************************************************************************/

// #define PPSOBJECT_DEBUG

#include "ppsobject.h"

#include <QDebug>
#include <QStringList>
#include <QMetaProperty>

extern "C" {
#include <sys/pps.h>
}

QT_BEGIN_NAMESPACE
namespace BlackBerry
{
QVariant qObjectToQVariant(QObject const& object);
QVariant readArray(pps_decoder_t &decoder);
QVariant readValue(pps_decoder_t &decoder);
QVariant readObject(pps_decoder_t &decoder);
void encodeObject(pps_encoder_t &encoder, QString const& name, QVariantHash const& object, bool root);
void encodeVariant(pps_encoder_t &encoder, QString const& name, QVariant const& value);
void encodeArray(pps_encoder_t &encoder, QString const& name, QVariantList const& object);
void encoderAddString(pps_encoder_t &encoder, QString const& name, QVariant const& object);

const QString ObjectTag = QString::fromAscii("@");

PPSObject::PPSObject(QString const& name, QVariantHash const& attributes) :
    mAttributes(attributes),
    mObjectName(name),
    mObjectStatus(Undefined)
{
    if (mObjectName.isEmpty())
        mObjectName = ObjectTag;
}

PPSObject::PPSObject(QString const& name, QObject const& object) :
    mObjectName(name),
    mObjectStatus(Undefined)
{
    if (mObjectName.isEmpty())
        mObjectName = ObjectTag;

    // Read the object meta-data and turn into a QVariant.
    mAttributes = qObjectToQVariant(object);
}

PPSObject::PPSObject(QString const& name, QString const& clientId, QVariantHash const& attributes) :
    mAttributes(attributes),
    mClientId(clientId),
    mObjectName(name),
    mObjectStatus(Undefined)
{
    if (mObjectName.isEmpty())
        mObjectName = ObjectTag;
}

PPSObject::PPSObject(QString const& name, QString const& clientId, QObject const& object) :
    mClientId(clientId),
    mObjectName(name),
    mObjectStatus(Undefined)
{
    if (mObjectName.isEmpty())
        mObjectName = ObjectTag;

    // Read the object meta-data and turn into a QVariant.
    mAttributes = qObjectToQVariant(object);
}

PPSObject::PPSObject(QString const& name) :
    mObjectStatus(Undefined)
{
    if (name.isEmpty()) {
        mObjectName = ObjectTag;
        return;
    }

    QStringList items = name.split(QChar::fromAscii('.'));

    if (items.length()>2)
        qFatal("Unexpected/unsupported pps object name!");

    mObjectName = items[0];

    if (items.length() > 1)
        mClientId = items[1];
}

PPSObject::~PPSObject()
{
}

QString PPSObject::name() const
{
    if (mClientId.isEmpty())
        return mObjectName;
    else {
        return mObjectName + QString::fromAscii(".") + mClientId;
    }
}

QByteArray PPSObject::serializeToPps(bool json) const
{
#ifdef PPSOBJECT_DEBUG
    qDebug() << __FILE__ << ":" << __FUNCTION__
             << "json=" <<json;
#endif

    QVariantHash items = mAttributes.toHash();

    pps_encoder_t encoder;
    pps_encoder_initialize(&encoder, json);

    // The base object must at least be "@"?
    encodeObject(encoder, name(), items, true);

    QByteArray ret(pps_encoder_buffer(&encoder), pps_encoder_length(&encoder));
    pps_encoder_cleanup(&encoder);

    return ret;
}

QList<PPSObject> PPSObject::parse(QByteArray& data)
{
    QList<PPSObject> ret;
    pps_decoder_error_t err;
    pps_decoder_t decoder;

    if (!data.endsWith('\0'))
        qFatal("PPSObject: Byte array is not a valid string!");

    err = pps_decoder_initialize(&decoder, data.data());
    if (err != PPS_DECODER_OK) {
        qWarning() << "PPSObject: Could not intitialize the pps decoder. Err=" << err;
        return ret;
    }

    // Loop over all of the objects, determine if they are readable or not, then read the object.
    while(1) {

        // If it is not an object, then it is the end of the data.
        if( pps_decoder_type(&decoder, 0) != PPS_TYPE_OBJECT) {
            break;
        }

        // Create a new pps object with the correct name.
        PPSObject obj(QString::fromUtf8(pps_decoder_name(&decoder)));
        int flags = pps_decoder_flags(&decoder, 0);

        // If there are flags set on the object, then this object will not have
        // any attributes.
        if (flags) {
            if (flags & PPS_DELETED) {
                obj.mObjectStatus = Deleted;
            } else if (flags & PPS_CREATED) {
                obj.mObjectStatus = Created;
            } else if (flags & PPS_TRUNCATED) {
                obj.mObjectStatus = Truncated;
            } else if (flags & PPS_INCOMPLETE) {
                qWarning() << "PPSObject: PPS Object was incomplete. Incomplete objects are currentl unsupported.";
                obj.mObjectStatus = Incomplete;
            } else if (flags & PPS_PURGED) {
                obj.mObjectStatus = Purged;
            }

            // Move to the next object.
            pps_decoder_next(&decoder);
        } else {
            obj.mAttributes = readObject(decoder);
        }

        ret.push_back(obj);
     }

    pps_decoder_cleanup(&decoder);

    return ret;
}

/* ====================================================== */
/* Encoding functions.                                    */
/* ====================================================== */
QVariant qObjectToQVariant(QObject const& object)
{
    // Does not support nested QObjects (yet?)
    QVariantHash result;
    const QMetaObject* metaObject = object.metaObject();

    if (!metaObject)
        return result;

    for (int i = 0; i < metaObject->propertyCount(); i++) {
        QMetaProperty metaProperty = metaObject->property(i);

        if (!metaProperty.isReadable())
            continue;

        result[QString::fromLatin1(metaProperty.name())] = metaProperty.read(&object);
    }

    return result;
}

// Converts the variant to a string regardless of contents.
void encoderAddString(pps_encoder_t &encoder, QString const& name, QVariant const& object)
{
#ifdef PPSOBJECT_DEBUG
    qDebug() << __FILE__ << ":" << __FUNCTION__
             << "name=" << name
             << "object=" <<object;
#endif

    QByteArray str = object.toString().toUtf8();
    QByteArray utf8name = name.toUtf8();
    const char* pName = utf8name.length() ? utf8name.data() : 0;

    pps_encoder_add_string(&encoder, pName, str.data());
}

void encodeArray(pps_encoder_t &encoder, QString const& name, QVariantList const& object)
{
#ifdef PPSOBJECT_DEBUG
    qDebug() << __FILE__ << ":" << __FUNCTION__
             << "name=" << name
             << "object=" <<object;
#endif

    QByteArray utf8Name = name.toUtf8();
    const char* pName = utf8Name.length()?utf8Name.data():0;

    pps_encoder_start_array(&encoder, pName);

    QVariantList::const_iterator it;
    for (it = object.begin(); it != object.end(); it++) {
        encodeVariant(encoder, QString(), *it);
    }

    pps_encoder_end_array(&encoder);
}

void encodeVariant(pps_encoder_t &encoder, QString const& name, QVariant const& value)
{
#ifdef PPSOBJECT_DEBUG
    qDebug() << __FILE__ << ":" << __FUNCTION__
             << "name=" << name
             << "value=" <<value;
#endif

    QByteArray utf8Name = name.toUtf8();
    const char* pName = utf8Name.length()?utf8Name.data():0;

    if (!value.isValid()) {
        pps_encoder_delete_attribute(&encoder, pName);
        return;
    }

    switch(value.type()) {
    case QVariant::Bool:
        pps_encoder_add_bool(&encoder, pName, value.toBool());
        break;

    case QVariant::String:
        encoderAddString(encoder, name, value);
        break;

    case QVariant::Int:
        pps_encoder_add_int(&encoder, pName, value.toInt());
        break;

    case QVariant::UInt:
    case QVariant::ULongLong:
    case QVariant::LongLong:
        qWarning() << "PPSObject: encoder: Converting unsupported " << value.typeName() << " to double.";
        pps_encoder_add_double(&encoder, pName, value.toDouble());
        break;

    case QVariant::Double:
        pps_encoder_add_double(&encoder, pName, value.toDouble());
        break;

    case QVariant::Hash:
        encodeObject(encoder, name, value.toHash(), false);
        break;

    case QVariant::StringList:
    case QVariant::List:
        encodeArray(encoder, name, value.toList());
        break;

    default:
        if (value.canConvert(QVariant::String)) {
            qWarning() << "PPSObject: encoder: Converting unsupported " << value.typeName() << " to string.";
            encoderAddString(encoder, name, value);
        } else {
            qWarning() << "PPSObject: encoder: Cannot convert unsupported " << value.typeName() << " to string. Writing a NULL object to PPS";
            pps_encoder_add_null(&encoder, pName);
        }
        break;
    }
}

void encodeObject(pps_encoder_t &encoder, QString const& name, QVariantHash const& object, bool root)
{
#ifdef PPSOBJECT_DEBUG
    qDebug() << __FILE__ << ":" << __FUNCTION__
             << "name=" << name
             << "object=" <<object;
#endif

    QByteArray utf8name = name.toUtf8();
    const char* pName = utf8name.length() ? utf8name.data() : NULL;

    // We don't necessarily need to start the object if we are at the root. Only
    // start one if there was a name set.
    if (pName || !root)
        pps_encoder_start_object(&encoder, pName);

    QVariantHash::const_iterator it;
    for (it = object.begin(); it !=object.end(); it++) {
        encodeVariant(encoder, it.key(), it.value());
    }

    if (pName || !root)
        pps_encoder_end_object(&encoder);
}


/* ====================================================== */
/* Decoding functions.                                    */
/* ====================================================== */

QVariant readArray(pps_decoder_t &decoder)
{
    QVariantList array;

    pps_decoder_push(&decoder, 0);
    int len = pps_decoder_length(&decoder);

    for (int i=0; i<len; i++) {
        array.push_back(readValue(decoder));
    }

    pps_decoder_pop(&decoder);
    return array;
}

QVariant readValue(pps_decoder_t &decoder)
{
    QVariant value;
    bool bValue = false;
    double dValue = 0.0;
    const char* cValue = 0;

    switch ( pps_decoder_type(&decoder, 0)) {
    case PPS_TYPE_NULL:
        pps_decoder_next(&decoder);
        break;
    case PPS_TYPE_BOOL:
        pps_decoder_get_bool(&decoder, 0, &bValue);
        value = QVariant::fromValue(bValue);
        break;
    case PPS_TYPE_NUMBER:
        pps_decoder_get_double(&decoder, 0, &dValue);
        value = QVariant::fromValue(dValue);
        break;
    case PPS_TYPE_STRING:
        pps_decoder_get_string(&decoder, 0, &cValue);
        value = QVariant::fromValue(QString::fromUtf8(cValue));
        break;
    case PPS_TYPE_ARRAY:
        value = readArray(decoder);
        break;
    case PPS_TYPE_OBJECT:
        value = readObject(decoder);
        break;
    case PPS_TYPE_UNKNOWN:
        qWarning() << "PPSObject: Unknown type in pps object. Reading from pps as string.";
        pps_decoder_get_string(&decoder, 0, &cValue);
        value = QVariant::fromValue(QString::fromUtf8(cValue));
        break;
    default:
        qWarning() << "PPSObject: Unsupported type in pps object. Skipping";
        pps_decoder_next(&decoder);
        break;
    }

    return value;
}

QVariant readObject(pps_decoder_t &decoder)
{
    QVariantHash object;

    pps_decoder_push(&decoder, 0);

    while ( pps_decoder_type(&decoder, 0) != PPS_TYPE_NONE ) {
        QString name = QString::fromUtf8(pps_decoder_name(&decoder));
        int flags = pps_decoder_flags(&decoder, 0);

        if (name.isEmpty()) {
            qWarning() << "PPSObject: PPS object had nameless attribute.";
        }

        if (flags & PPS_DELETED) {
            object[name] = QVariant();
            pps_decoder_next(&decoder);
        } else {
            object[name] = readValue(decoder);
        }
    }

    pps_decoder_pop(&decoder);
    return object;
}


} // namespace BlackBerry
QT_END_NAMESPACE
