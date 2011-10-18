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

#ifndef PPSOBJECT_H
#define PPSOBJECT_H

#include <QList>
#include <QVariant>

QT_BEGIN_NAMESPACE

namespace BlackBerry
{

class PPSObject
{
public:
    enum ObjectStatus {
        Undefined,  // Indicates that no special status is implied. This is normal when the attributes have changed.
        Incomplete, // The object or attribute line is incomplete.
        Created,    // PPS created the object or client connected
        Deleted,    // PPS deleted the object or client disconnected.
        Truncated,  // PPS truncated the object, all attributes were removed.
        Purged      // The object has lost a critical publisher. All non-persistent attributes have been deleted.
    };

    enum Encoding {
        NullEncoding,       // Same as text encoding.
        TextEncoding,       // Simple text terminated by a linefeed.
        CStyleEncoding,     // C language escape sequences, such as "\t" and "\n"
        Base64Encoding,     // Base 64 encoding
        JSONEncoding        // JSON encoding (@see http://www.json.org/)
    };

public:
    /**
     * Create a PPS object from the specified variant map. The map MAY have a single item.
     * See name() for naming conventions.
     */
    PPSObject(QString const& name, QVariantHash const& attributes);

    /**
     * Serialize a QObject's properties to a PPS object. The entire object will be serialized.
     * See name() for naming conventions.
     */
    PPSObject(QString const& name, QObject const& object);

    /**
     * Create a PPS object from the specified variant map. The map MAY have a single item.
     * See name() for naming conventions.
     */
    PPSObject(QString const& name, QString const& clientId, QVariantHash const& attributes);

    /**
     * Serialize a QObject's properties to a PPS object. The entire object will be serialized.
     * See name() for naming conventions.
     */
    PPSObject(QString const& name, QString const& clientId, QObject const& object);

    /**
     * Construct an empty PPS Object from the pps object name. If it has a client id, they will
     * automatically be split.
     */
    PPSObject(QString const& rawName);

    virtual ~PPSObject();

    /**
     * The full name of this object. This is generally the same as the name of the file. The name is
     * optional unless this is a server object and you will be directing the message at a specific
     * client.
     *
     * If this is a server object the follwing will apply.
     *  - The name() will contain the identifier of the client machine in the format @nnn.ii where
     *    nnn represents the full name of the object and ii represents the identifier of the client.
     *  - Writing an object with the name @nnn.ii will send a message to the client with identifier 'ii'.
     *  - Writing an object without the client identifier will send the message to all clients.
     */
    QString name() const;

    /**
     * The object name only (e.g. @object) with the client id stripped (if it exists)
     */
    QString objectName() const { return mObjectName; }

    /**
     * The client id with object name stripped.
     */
    QString clientId() const { return mClientId; }

    /**
     * The status of the object. See the ObjectStatus structure for more information.
     */
    ObjectStatus objectStatus() const { return mObjectStatus; }

    /**
     * A hash map of all the attributes for this pps object. Use the toHash() function
     * to convert to the hash map.
     *
     * Notes:
     *  - An array is stored as a QList of QVariant objects.
     *  - An object (json data) will be stored in a QVariantHash.
     *  - If an attribute is deleted, its value will be an invalid QVariant.
     *  - If an attribute is empty (NULL), its value will be an empty string.
     */
    QVariant const& attributes() const { return mAttributes; }

    /**
     * Serializes this object into a PPS object data string. If the json flag is specified,
     * the entire object will be json encoded. Otherwise, the object is PPS encoded.
     * This function is called by the PPSFile class, and may be written manually to a PPS
     * device file.
     */
    QByteArray serializeToPps(bool json) const;

    /**
     * parse
     *
     * Parse a QByteArray which contains pps object data. This will return a list of
     * PPSObject objects, one for each object that was recognized in the file. Unless you
     * are using the delta mode this list will be of length 1. See the qnx pps documentation
     * for more information.
     */
    static QList<PPSObject> parse(QByteArray& data);

private:
    QVariant mAttributes;
    QString mClientId;
    QString mObjectName;
    ObjectStatus mObjectStatus;
};

} // namespace BlackBerry

QT_END_NAMESPACE


#endif //PPSOBJECT_H
