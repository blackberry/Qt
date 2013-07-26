#ifndef ColoredItemsCascades_HPP_
#define ColoredItemsCascades_HPP_

#include <QObject>

namespace bb { namespace cascades { class Application; }}

class ColoredItemsCascades : public QObject
{
    Q_OBJECT
public:
    ColoredItemsCascades(bb::cascades::Application *app);
    virtual ~ColoredItemsCascades() {}
};

#endif /* ColoredItemsCascades_HPP_ */

