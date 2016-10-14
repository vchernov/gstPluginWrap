#ifndef GSTPLUGINPROPERTYHOLDER_H_
#define GSTPLUGINPROPERTYHOLDER_H_

#include <map>
#include <set>

#include <gst/gst.h>

namespace gstPluginWrap {

/**
 * Parameter id-specification map.
 * @details The key is parameter id, the value is parameter specification.
 */
typedef std::map<guint, GParamSpec*> ParamIdSpecMap;

/**
 * Class for notification of property changes.
 */
class PropertyListener {
public:
    virtual ~PropertyListener() {}

    virtual void propertyChanged(guint id) {}
    virtual void mediaInfoChanged() {}
};

/**
 * Holder of properties.
 */
class PropertyHolder {
public:
    PropertyHolder() {
        listener = NULL;
    }

    virtual ~PropertyHolder() {}

    void setListener(PropertyListener* listener) {
        this->listener = listener;
        if (listener != NULL) {
            for (std::set<guint>::iterator it = initialParamIds.begin(), end = initialParamIds.end();
                    it != end; ++it) {
                guint id = *it;
                listener->propertyChanged(id);
            }
        }
    }

    PropertyListener* getListener() const {
        return listener;
    }

    virtual void setMediaInfo(gchar* mime, GstStructure* params) {
        if (listener != NULL)
            listener->mediaInfoChanged();
    }

    virtual bool set(guint id, const GValue* val) {
        if (listener != NULL)
            listener->propertyChanged(id);
        return true;
    }

    virtual bool get(guint id, GValue* val) {
        return true;
    }

protected:
    std::set<guint> initialParamIds; // parameters that will be notified right after frame handler creation

private:
    PropertyListener* listener;
};

/**
 * Holder of image frame properties.
 */
class ImagePropertyHolder: public PropertyHolder {
public:
    ImagePropertyHolder() {
        bufferWidth = 0;
        bufferHeight = 0;
    }

    virtual ~ImagePropertyHolder() {}

    virtual void setMediaInfo(gchar* mime, GstStructure* params) {
        gst_structure_get_int(params, "width", &bufferWidth);
        gst_structure_get_int(params, "height", &bufferHeight);

        PropertyHolder::setMediaInfo(mime, params);
    }

    int bufferWidth;
    int bufferHeight;
};

} // namespace gstPluginWrap

#endif // GSTPLUGINPROPERTYHOLDER_H_
