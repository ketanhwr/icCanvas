#include <icCanvasManager.hpp>

#include <icCanvasManagerC.h>

extern "C" {
    icm_renderscheduler icm_renderscheduler_construct(icm_application app) {
        icCanvasManager::Application *theApp = (icCanvasManager::Application*)app;
        icCanvasManager::RenderScheduler* d = new icCanvasManager::RenderScheduler(theApp);
        d->ref();

        return (icm_renderscheduler)d;
    };

    icm_renderscheduler icm_renderscheduler_reference(icm_renderscheduler w) {
        icCanvasManager::RenderScheduler* d = (icCanvasManager::RenderScheduler*)w;
        d->ref();

        return w;
    };

    int icm_renderscheduler_dereference(icm_renderscheduler w) {
        icCanvasManager::RenderScheduler* d = (icCanvasManager::RenderScheduler*)w;
        int refcount = d->deref();

        if (refcount <= 0) {
            delete d;
        }

        return refcount;
    };

    icm_renderer icm_renderscheduler_renderer(icm_renderscheduler wrap) {
        auto* w = (icCanvasManager::RenderScheduler*)wrap;
        icCanvasManager::Renderer* r = w->renderer();

        return r;
    };

    void icm_renderscheduler_set_renderer(icm_renderscheduler wrap, icm_renderer r) {
        auto* w = (icCanvasManager::RenderScheduler*)wrap;
        auto* r2 = (icCanvasManager::Renderer*)r;

        w->set_renderer(r2);
    };

    void icm_renderscheduler_request_tile(icm_renderscheduler w, icm_drawing d, int x, int y, int size, int time) {
        icCanvasManager::RenderScheduler* theSched = (icCanvasManager::RenderScheduler*)w;
        icCanvasManager::Drawing* theDrawing = (icCanvasManager::Drawing*)d;
        theSched->request_tile(theDrawing, x, y, size, time);
    };

    void icm_renderscheduler_request_tiles(icm_renderscheduler w, icm_drawing d, cairo_rectangle_t *rect, int size, int time) {
        icCanvasManager::RenderScheduler* theSched = (icCanvasManager::RenderScheduler*)w;
        icCanvasManager::Drawing* theDrawing = (icCanvasManager::Drawing*)d;

        cairo_rectangle_t loco_rect = *rect;

        theSched->request_tiles(theDrawing, loco_rect, size, time);
    };

    void icm_renderscheduler_revoke_request_rect(icm_renderscheduler w, icm_drawing d, int x_min, int y_min, int x_max, int y_max, bool is_inverse) {
        icCanvasManager::RenderScheduler* theSched = (icCanvasManager::RenderScheduler*)w;
        icCanvasManager::Drawing* theDrawing = (icCanvasManager::Drawing*)d;
        theSched->revoke_request(theDrawing, x_min, y_min, x_max, y_max, is_inverse);
    };

    void icm_renderscheduler_revoke_request_zoom(icm_renderscheduler w, icm_drawing d, int zoom_min, int zoom_max, bool is_inverse) {
        icCanvasManager::RenderScheduler* theSched = (icCanvasManager::RenderScheduler*)w;
        icCanvasManager::Drawing* theDrawing = (icCanvasManager::Drawing*)d;
        theSched->revoke_request(theDrawing, zoom_min, zoom_max, is_inverse);
    };

    void icm_renderscheduler_background_tick(icm_renderscheduler w) {
        icCanvasManager::RenderScheduler* d = (icCanvasManager::RenderScheduler*)w;
        d->background_tick();
    };

    int icm_renderscheduler_collect_request(icm_renderscheduler w, icm_drawing d, cairo_rectangle_t* out_tile_rect) {
        icCanvasManager::RenderScheduler* theSched = (icCanvasManager::RenderScheduler*)w;
        icCanvasManager::Drawing* theDrawing = (icCanvasManager::Drawing*)d;
        return theSched->collect_request(theDrawing, out_tile_rect);
    };
}
