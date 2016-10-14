/*
 * Note: the GST_PLUGIN_TYPE macro must be defined before including this file.
 * It is used as a name of the structure that represents GStreamer element type.
 * The defined name have to be unique among plugins installed in the system.
 *
 * Example:
 * #define GST_PLUGIN_TYPE GstMyPluginName
 * #include <gstPluginWrap.h>
 */

#ifndef GSTPLUGINWRAP_H_
#define GSTPLUGINWRAP_H_

#include <glib.h>
#include <gst/gst.h>

#include "gstPluginFrameHandler.h"
#include "gstPluginPropertyHolder.h"

namespace gstPluginWrap {

namespace details {
extern const char* name;
extern const char* longName;
extern const char* classification;
extern const char* description;
extern const char* originUrl;
extern const char* author;
extern const char* license;
extern const char* version;
} // namespace details

extern const char* allowedCaps;

typedef void (*ParametersGetter)(ParamIdSpecMap& parameters);

class Customizer;

extern void configure(Customizer*& customizer);

/**
 * Basic customization class. Should be used as a singleton.
 */
class Customizer {
public:
    static Customizer* getInstance() {
        if (instance != NULL)
            return instance;

        configure(instance);

        if (instance == NULL)
            instance = new Customizer(); // create default customizer if not provided by user code
        return instance;
    }

    static void deleteInstance() {
        delete instance;
        instance = NULL;
    }

    Customizer() {
        getParameters = NULL;
    }

    virtual ~Customizer() {}

    virtual PropertyHolder* createPropertyHolder() {
        return NULL;
    }

    virtual PureFrameHandler* createFrameHandler(PropertyHolder* propertyHolder) {
        return NULL;
    }

    virtual void deleteFrameHandler(PureFrameHandler*& frameHandler, PropertyHolder* propertyHolder) {
        delete frameHandler;
        frameHandler = NULL;
    }

    ParametersGetter getParameters;

private:
    static Customizer* instance;
};

Customizer* Customizer::instance = NULL;

/**
 * Plugin with frame handling.
 */
template<class HandlerT>
class HandlerCustomizer: public Customizer {
public:
    virtual ~HandlerCustomizer() {}

    virtual PureFrameHandler* createFrameHandler(PropertyHolder* propertyHolder) {
        return new HandlerT();
    }
};

/**
 * Plugin with frame handling and properties.
 */
template<class HandlerT, class PropT>
class HandlerAndPropCustomizer: public HandlerCustomizer<HandlerT> {
public:
    virtual ~HandlerAndPropCustomizer() {}

    virtual PropertyHolder* createPropertyHolder() {
        return new PropT();
    }

    virtual PureFrameHandler* createFrameHandler(PropertyHolder* propertyHolder) {
        HandlerT* frameHandler = new HandlerT();
        PropT* properties = static_cast<PropT*>(propertyHolder);

        frameHandler->setProperties(properties);
        return frameHandler;
    }
};

/**
 * Plugin with frame handling and properties bounded by listener.
 */
template<class HandlerT, class PropT>
class HandlerAndPropWithListenerCustomizer: public HandlerCustomizer<HandlerT> {
public:
    virtual ~HandlerAndPropWithListenerCustomizer() {}

    virtual PropertyHolder* createPropertyHolder() {
        return new PropT();
    }

    virtual PureFrameHandler* createFrameHandler(PropertyHolder* propertyHolder) {
        HandlerT* frameHandler = new HandlerT();
        PropT* properties = static_cast<PropT*>(propertyHolder);

        frameHandler->setProperties(properties);
        properties->setListener(frameHandler);
        return frameHandler;
    }

    virtual void deleteFrameHandler(PureFrameHandler*& frameHandler, PropertyHolder* propertyHolder) {
        PropT* properties = static_cast<PropT*>(propertyHolder);
        properties->setListener(NULL);

        Customizer::deleteFrameHandler(frameHandler, propertyHolder);
    }
};

} // namespace gstPluginWrap

G_BEGIN_DECLS // begin C code

#ifndef GST_PLUGIN_TYPE
#error "GST_PLUGIN_TYPE is not defined."
#endif

#define TYPE_JOIN(a, b) a ## b
#define MAKE_CLASS_NAME(base, postfix) TYPE_JOIN(base, postfix)

#define GST_PLUGIN_TYPE_CLASS MAKE_CLASS_NAME(GST_PLUGIN_TYPE, Class)

struct _GstThisPlugin {
    GstElement element;
    GstPad* sinkPad;
    GstPad* srcPad;

    gchar* mime;

    gstPluginWrap::PureFrameHandler* frameHandler;
    gstPluginWrap::PropertyHolder* propertyHolder;
};

typedef struct _GstThisPlugin GST_PLUGIN_TYPE;

struct _GstThisPluginClass {
    GstElementClass parent_class;
};

typedef struct _GstThisPluginClass GST_PLUGIN_TYPE_CLASS;

GType gst_thisplugin_get_type();

#define GST_TYPE_THISPLUGIN \
	gst_thisplugin_get_type()

#define GST_THISPLUGIN(obj) \
	G_TYPE_CHECK_INSTANCE_CAST(obj, GST_TYPE_THISPLUGIN, GST_PLUGIN_TYPE)

#define GST_IS_THISPLUGIN(obj) \
	G_TYPE_CHECK_INSTANCE_TYPE(obj, GST_TYPE_THISPLUGIN)

#define GST_THISPLUGIN_CLASS(probClass) \
	G_TYPE_CHECK_CLASS_CAST(probClass, GST_TYPE_THISPLUGIN, GST_PLUGIN_TYPE_CLASS)

#define GST_IS_THISPLUGIN_CLASS(probClass) \
	G_TYPE_CHECK_CLASS_TYPE(probClass, GST_TYPE_THISPLUGIN)

G_END_DECLS // end C code

/*
 * The _finalize() function is called when the plugin is destroyed.
 */
static void gst_thisplugin_finalize(GObject* pluginObj);

/*
 * The _set_property() function will be notified if an application changes the value of a property,
 * and can then take action required for that property to change value internally.
 */
static void gst_thisplugin_set_property(GObject* pluginObj, guint id, const GValue* val,
                                        GParamSpec* paramSpec);

/*
 * The _get_property() function will be notified
 * if an application requests the value of a property,
 * and can then fill in the value.
 */
static void gst_thisplugin_get_property(GObject* pluginObj, guint id, GValue* val,
                                        GParamSpec* paramSpec);

/*
 * The _change_state() is called when state is changed.
 */
static GstStateChangeReturn gst_thisplugin_change_state(GstElement* element,
        GstStateChange transition);

/*
 * The _setcaps() function is called during caps negotiation.
 * This is the process where the linked pads decide on the stream type
 * that will transfer between them.
 */
static gboolean gst_thisplugin_setcaps(GstPad* pad, GstCaps* caps);

/*
 * The _chain() function is the function in which all data processing takes place.
 */
static GstFlowReturn gst_thisplugin_chain(GstPad* pad, GstBuffer* buffer);

// sink pad (input)
static GstStaticPadTemplate sinkTemplate = GST_STATIC_PAD_TEMPLATE(
            "sink",
            GST_PAD_SINK,
            GST_PAD_ALWAYS,
            GST_STATIC_CAPS(gstPluginWrap::allowedCaps)
        );

// source pad (output)
static GstStaticPadTemplate srcTemplate = GST_STATIC_PAD_TEMPLATE(
            "src",
            GST_PAD_SRC,
            GST_PAD_ALWAYS,
            GST_STATIC_CAPS(gstPluginWrap::allowedCaps)
        );

/*
 * The plugin_init() function is called as soon as the plugin is loaded.
 * It should return TRUE or FALSE depending on whether it is loaded correctly or not.
 * In this function, any supported element type in the plugin should be registered.
 */
static gboolean plugin_init(GstPlugin* gstPlugin) {
    using namespace gstPluginWrap;

    return gst_element_register(gstPlugin, details::name, GST_RANK_NONE, GST_TYPE_THISPLUGIN);
}

#ifndef PACKAGE
#define PACKAGE ""
#endif

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    gstPluginWrap::details::name,
    gstPluginWrap::details::description,
    plugin_init,
    gstPluginWrap::details::version,
    gstPluginWrap::details::license,
    PACKAGE,
    gstPluginWrap::details::originUrl
)

GST_BOILERPLATE(GST_PLUGIN_TYPE, gst_thisplugin, GstElement, GST_TYPE_ELEMENT)

/*
 * The _base_init() function is meant to initialize class and child class properties
 * during each new child class creation.
 */
static void gst_thisplugin_base_init(gpointer typeClass) {
    using namespace gstPluginWrap;

    GstElementClass* elemClass = GST_ELEMENT_CLASS(typeClass);
    gst_element_class_set_details_simple(elemClass,
                                         details::longName,
                                         details::classification,
                                         details::description,
                                         details::author);
    gst_element_class_add_pad_template(elemClass, gst_static_pad_template_get(&srcTemplate));
    gst_element_class_add_pad_template(elemClass, gst_static_pad_template_get(&sinkTemplate));
}

/*
 * The _class_init() function is used to initialize the class.
 * It is called only once and must specify what signals,
 * arguments and virtual functions the class has.
 */
static void gst_thisplugin_class_init(GST_PLUGIN_TYPE_CLASS* pluginClass) {
    using namespace gstPluginWrap;

    parent_class = (GstElementClass*)g_type_class_ref(GST_TYPE_ELEMENT);

    GObjectClass* gobjClass = G_OBJECT_CLASS(pluginClass);
    gobjClass->finalize = gst_thisplugin_finalize;
    gobjClass->set_property = gst_thisplugin_set_property;
    gobjClass->get_property = gst_thisplugin_get_property;

    GstElementClass* elemClass = GST_ELEMENT_CLASS(pluginClass);
    elemClass->change_state = gst_thisplugin_change_state;

    ParametersGetter paramGetter = Customizer::getInstance()->getParameters;
    if (paramGetter != NULL) {
        ParamIdSpecMap params;
        (*paramGetter)(params);
        for (ParamIdSpecMap::iterator it = params.begin(), end = params.end(); it != end; ++it) {
            guint id = it->first;
            GParamSpec* spec = it->second;
            g_object_class_install_property(gobjClass, id, spec);
        }
    }
}

/*
 * The _init() function is used to initialize a specific instance of the plugin type.
 */
static void gst_thisplugin_init(GST_PLUGIN_TYPE* plugin, GST_PLUGIN_TYPE_CLASS* pluginClass) {
    using namespace gstPluginWrap;

    plugin->sinkPad = gst_pad_new_from_static_template(&sinkTemplate, "sink");
    gst_pad_set_setcaps_function(plugin->sinkPad, gst_thisplugin_setcaps);
    gst_pad_set_getcaps_function(plugin->sinkPad, gst_pad_proxy_getcaps);
    gst_pad_set_chain_function(plugin->sinkPad, gst_thisplugin_chain);

    plugin->srcPad = gst_pad_new_from_static_template(&srcTemplate, "src");
    gst_pad_set_getcaps_function(plugin->srcPad, gst_pad_proxy_getcaps);

    gst_element_add_pad(GST_ELEMENT(plugin), plugin->sinkPad);
    gst_element_add_pad(GST_ELEMENT(plugin), plugin->srcPad);

    plugin->frameHandler = NULL;
    plugin->propertyHolder = Customizer::getInstance()->createPropertyHolder();
}

static void gst_thisplugin_finalize(GObject* pluginObj) {
    using namespace gstPluginWrap;

    GST_PLUGIN_TYPE* plugin = GST_THISPLUGIN(pluginObj);

    delete plugin->propertyHolder;
    plugin->propertyHolder = NULL;

    g_free(plugin->mime);
    plugin->mime = NULL;

    Customizer::deleteInstance();

    G_OBJECT_CLASS(parent_class)->finalize(pluginObj);
}

static void gst_thisplugin_set_property(GObject* pluginObj, guint id, const GValue* val,
                                        GParamSpec* paramSpec) {
    GST_PLUGIN_TYPE* plugin = GST_THISPLUGIN(pluginObj);
    if (!plugin->propertyHolder->set(id, val))
        G_OBJECT_WARN_INVALID_PROPERTY_ID(pluginObj, id, paramSpec);
}

static void gst_thisplugin_get_property(GObject* pluginObj, guint id, GValue* val,
                                        GParamSpec* paramSpec) {
    GST_PLUGIN_TYPE* plugin = GST_THISPLUGIN(pluginObj);
    if (!plugin->propertyHolder->get(id, val))
        G_OBJECT_WARN_INVALID_PROPERTY_ID(pluginObj, id, paramSpec);
}

static GstStateChangeReturn gst_thisplugin_change_state(GstElement* element,
        GstStateChange transition) {
    using namespace gstPluginWrap;

    GST_PLUGIN_TYPE* plugin = GST_THISPLUGIN(element);
    GstStateChangeReturn retVal = GST_STATE_CHANGE_SUCCESS;

    if (transition == GST_STATE_CHANGE_NULL_TO_READY) {
        if (plugin->frameHandler == NULL) {
            Customizer* customer = Customizer::getInstance();
            plugin->frameHandler = customer->createFrameHandler(plugin->propertyHolder);
        } else {
            g_printerr("%s\n", "FrameHandler is not clean at null to ready state change");
            retVal = GST_STATE_CHANGE_FAILURE;
        }
    }
    if (retVal == GST_STATE_CHANGE_FAILURE)
        return retVal;

    retVal = GST_ELEMENT_CLASS(parent_class)->change_state(element, transition);
    if (retVal == GST_STATE_CHANGE_FAILURE)
        return retVal;

    if (transition == GST_STATE_CHANGE_READY_TO_NULL)
        Customizer::getInstance()->deleteFrameHandler(plugin->frameHandler, plugin->propertyHolder);
    return retVal;
}

static gboolean gst_thisplugin_setcaps(GstPad* pad, GstCaps* caps) {
    GST_PLUGIN_TYPE* plugin = GST_THISPLUGIN(gst_pad_get_parent(pad));
    GstStructure* props = gst_caps_get_structure(caps, 0);

    const gchar* mime = gst_structure_get_name(props);
    plugin->mime = g_strdup(mime);

    if (plugin->propertyHolder != NULL)
        plugin->propertyHolder->setMediaInfo(plugin->mime, props);

    GstPad* otherPad = plugin->srcPad;
    gst_object_unref(plugin);
    return gst_pad_set_caps(otherPad, caps);
}

static GstFlowReturn gst_thisplugin_chain(GstPad* pad, GstBuffer* buffer) {
    GST_PLUGIN_TYPE* plugin = GST_THISPLUGIN(GST_OBJECT_PARENT(pad));

    if (plugin->frameHandler)
        plugin->frameHandler->process(GST_BUFFER_DATA(buffer));

    return gst_pad_push(plugin->srcPad, buffer);
}

#endif // GSTPLUGINWRAP_H_
