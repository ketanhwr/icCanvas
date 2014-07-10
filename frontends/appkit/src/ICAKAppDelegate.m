#import <icCanvasAppKit.h>
#import <icCanvasManagerObjC.h>

@implementation ICAKAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    NSRect r = {{100,100}, {250, 250}};
    self.window = [[NSWindow alloc] initWithContentRect:r styleMask:(NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask) backing:NSBackingStoreBuffered defer:TRUE];
    [self.window makeKeyAndOrderFront:nil];
    [self.window setCollectionBehavior:(self.window.collectionBehavior|NSWindowCollectionBehaviorFullScreenPrimary)];
    
    ICMDrawing* drawing = [[ICMDrawing alloc] init];
    
    ICAKCanvasView* cv = [[ICAKCanvasView alloc] initWithDrawing: drawing];
    [self.window setContentView:cv];
}

@end