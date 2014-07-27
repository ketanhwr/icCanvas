#include <icCanvasManager.hpp>

#include <cmath>
#include <algorithm>

icCanvasManager::CanvasView::CanvasView() {
    this->renderer = new icCanvasManager::Renderer();
    this->fitter = new icCanvasManager::SplineFitter();

    this->x_center = 0;
    this->y_center = 0;
    this->zoom = 65535; //TODO: Replace with Drawing-specified zoom
};

icCanvasManager::CanvasView::~CanvasView() {};

void icCanvasManager::CanvasView::attach_drawing(icCanvasManager::RefPtr<icCanvasManager::Drawing> drawing) {
    this->drawing = drawing;
};

void icCanvasManager::CanvasView::draw(cairo_t *ctxt, cairo_rectangle_t dirtyArea) {
    float square_size = std::max(this->width, this->height);
    int lowest_zoom = std::floor(31 - log2(square_size * this->zoom));
    int highest_zoom = std::ceil(31 - log2(icCanvasManager::TileCache::TILE_SIZE * this->zoom));
    auto clear_ptn = cairo_pattern_create_rgb(1.0f, 1.0f, 1.0f);

    //Phase 0: Clear the surface
    cairo_save(ctxt);
    cairo_set_source(ctxt, clear_ptn);
    cairo_paint(ctxt);
    cairo_pattern_destroy(clear_ptn);

    //Phase 1: Look up tiles from cache
    auto renderscheduler = this->drawing->get_scheduler();
    renderscheduler->collect_requests(this->drawing);

    icCanvasManager::TileCache::TileCacheQuery qu1;
    qu1.query_size_gte(lowest_zoom);
    qu1.query_size_lt(highest_zoom);
    auto tilecache = this->drawing->get_tilecache();
    auto tilelist = tilecache->execute(qu1);
    auto begin = tilelist.begin(), end = tilelist.end();

    //Phase 2: Copy/scale tiles onto view
    for (; begin != end; begin++) {
        auto &tile = tilecache->tile_at(*begin);

        cairo_save(ctxt);

        auto target_size = UINT32_MAX >> tile.size;
        auto scale_factor = icCanvasManager::TileCache::TILE_SIZE / target_size;
        int txpos, typos; //yes I know, typos

        this->coordToWindowspace(tile.x, tile.y, &txpos, &typos);

        cairo_scale(ctxt, scale_factor, scale_factor);
        cairo_translate(ctxt, (double)txpos, (double)typos);

        cairo_set_source_surface(ctxt, tile.image, 0, 0);
        cairo_paint(ctxt);

        cairo_restore(ctxt);
    }

    //Phase 3: Request tiles we couldn't find
    auto request_size = UINT32_MAX >> highest_zoom;
    auto request_mask = !request_size;
    auto base_x = this->x_scroll & request_mask, base_y = this->y_scroll & request_mask;
    auto x_tile_count = std::ceil((2 * (this->x_center - this->x_scroll)) / request_size);
    auto y_tile_count = std::ceil((2 * (this->y_center - this->y_scroll)) / request_size);

    for (int i = 0; i <= x_tile_count; i++) {
        for (int j = 0; j <= y_tile_count; j++) {
            renderscheduler->request_tile(this->drawing, base_x + (request_size * i), base_y + (request_size * j), highest_zoom, this->drawing->strokes_count());
        }
    }

    cairo_restore(ctxt);
};

void icCanvasManager::CanvasView::set_size(double width, double height, double ui_scale) {
    this->width = width;
    this->height = height;
    this->ui_scale = ui_scale;

    this->x_size = this->width * this->zoom;
    this->y_size = this->height * this->zoom;
    this->x_scroll = this->x_center - (width * this->zoom / 2);
    this->y_scroll = this->y_center - (height * this->zoom / 2);
};

void icCanvasManager::CanvasView::windowToCoordspace(const int32_t x, const int32_t y, int32_t* out_tx, int32_t* out_ty) {
    if (out_tx) *out_tx = (int)((float)x * this->zoom + this->x_scroll);
    if (out_ty) *out_ty = (int)((float)y * this->zoom + this->y_scroll);
};

void icCanvasManager::CanvasView::coordToWindowspace(const int32_t x, const int32_t y, int32_t* out_tx, int32_t* out_ty) {
    if (out_tx) *out_tx = (int)((float)(x - this->x_scroll) / this->zoom);
    if (out_ty) *out_ty = (int)((float)(y - this->y_scroll) / this->zoom);
};

void icCanvasManager::CanvasView::mouse_down(int x, int y, int deltaX, int deltaY) {
    this->built_stroke = new icCanvasManager::BrushStroke();
    this->fitter->begin_fitting(this->built_stroke, this->width * this->zoom);
};

void icCanvasManager::CanvasView::mouse_drag(int x, int y, int deltaX, int deltaY) {
    int cx, cy, cdx = deltaX * this->zoom, cdy = deltaY * this->zoom;

    this->windowToCoordspace(x, y, &cx, &cy);
    this->fitter->add_fit_point(cx, cy, icCanvasManager::BrushStroke::PRESSURE_MAX, 0, 0, cdx, cdy);
};

void icCanvasManager::CanvasView::mouse_up(int x, int y, int deltaX, int deltaY) {
    this->fitter->finish_fitting();
    this->drawing->append_stroke(this->built_stroke);
    this->built_stroke = NULL;
};
