#import <icCanvasManagerObjC.h>
#include <icCanvasManager.hpp>

@implementation ICMCanvasView {
    icCanvasManager::RefPtr<icCanvasManager::CanvasView> _wrapped;
}

- (id)init {
    self = [super init];
    
    if (self != nil) {
        self->_wrapped = new icCanvasManager::CanvasView();
    }
    
    return self;
};

- (id)initFromWrappedObject:(void*)optr {
    self = [super init];
    
    if (self != nil) {
        self->_wrapped = (icCanvasManager::CanvasView*)optr;
    }
    
    return self;
};

- (void)attachDrawing:(ICMDrawing*)drawing {
    icCanvasManager::RefPtr<icCanvasManager::Drawing> cppdr = (icCanvasManager::Drawing*)[drawing getWrappedObject];
    
    self->_wrapped->attach_drawing(cppdr);
};

- (void)drawWithContext:(cairo_t*)context inDirtyRect:(cairo_rectangle_t) dirtyArea {
    self->_wrapped->draw(context, dirtyArea);
};

- (void)setSizeWidth:(double)width andHeight:(double)height {
    self->_wrapped->set_size(width, height);
};

- (void)mouseDownWithX:(int)x andY:(int)y andDeltaX:(int)deltaX andDeltaY:(int)deltaY {
    self->_wrapped->mouse_down(x, y, deltaX, deltaY);
};

- (void)mouseDragWithX:(int)x andY:(int)y andDeltaX:(int)deltaX andDeltaY:(int)deltaY {
    self->_wrapped->mouse_drag(x, y, deltaX, deltaY);
};

- (void)mouseUpWithX:(int)x andY:(int)y andDeltaX:(int)deltaX andDeltaY:(int)deltaY {
    self->_wrapped->mouse_up(x, y, deltaX, deltaY);
};

- (void*)getWrappedObject {
    return (void*)self->_wrapped;
};

@end