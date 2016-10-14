#ifndef GSTPLUGINFRAMEHANDLER_H_
#define GSTPLUGINFRAMEHANDLER_H_

#include <stdint.h>

namespace gstPluginWrap {

/**
 * Abstract class for frame processing inside GStreamer plugin.
 */
class PureFrameHandler {
public:
    virtual ~PureFrameHandler() {}

    virtual void process(uint8_t* buffer) = 0;
};

/**
 * Frame processing handler with GStreamer properties.
 */
template<class PropT>
class FrameHandler: public PureFrameHandler {
public:
    virtual ~FrameHandler() {}

    void setProperties(PropT* properties) {
        this->properties = properties;
    }

protected:
    PropT* properties;
};

} // namespace gstPluginWrap

#endif // GSTPLUGINFRAMEHANDLER_H_
