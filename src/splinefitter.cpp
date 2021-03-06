#include <icCanvasManager.hpp>

#include <cmath>
#include <iostream>
#include <algorithm>

icCanvasManager::SplineFitter::SplineFitter() : beizer_4_coeff(4,4) {
    this->beizer_4_coeff << -1.0,  3.0, -3.0, 1.0,
                             3.0, -6.0,  3.0, 0.0,
                            -3.0,  3.0,  0.0, 0.0,
                             1.0,  0.0,  0.0, 0.0;

    Eigen::Matrix4f bmat_identity = Eigen::Matrix4f::Identity();
    this->beizer_4_invcoeff = this->beizer_4_coeff.colPivHouseholderQr().solve(bmat_identity);
};
icCanvasManager::SplineFitter::~SplineFitter() {};

void icCanvasManager::SplineFitter::begin_fitting(icCanvasManager::RefPtr<icCanvasManager::BrushStroke> storage, int error_threshold) {
    if (this->unfitted_points.size() > 0) this->unfitted_points.clear();
    if (this->distances.size() > 0) this->distances.clear();
    this->target_curve = storage;
    this->unfitted_id = 0;
    this->error_threshold = error_threshold;
};

void icCanvasManager::SplineFitter::fit_curve(int max_pts) {
    auto ptsize = std::min((decltype(this->unfitted_points.size()))max_pts, this->unfitted_points.size());

    if (ptsize <= 0) return;

    int newTotalDist = this->distances.at(ptsize - 1);
    
    Eigen::Matrix<float, Eigen::Dynamic, 4> b_indexes(ptsize, 4);
    Eigen::Matrix<float, Eigen::Dynamic, 1> xposVec(ptsize, 1), yposVec(ptsize, 1), pressureVec(ptsize, 1), tiltVec(ptsize, 1), angleVec(ptsize, 1), xdeltaVec(ptsize, 1), ydeltaVec(ptsize, 1);
    
    auto i = this->distances.begin();
    auto j = this->unfitted_points.begin();

    assert(this->distances.size() == this->unfitted_points.size() && this->distances.size() == ptsize);

    for (int k = 0;
         i != this->distances.end() &&
         j != this->unfitted_points.end() &&
         k != ptsize;
         i++, j++, k++) {
        float tval = (float)(*i) / (float)newTotalDist;
        
        b_indexes(k, 0) = tval * tval * tval;
        b_indexes(k, 1) = tval * tval;
        b_indexes(k, 2) = tval;
        b_indexes(k, 3) = 1.0f;

        xposVec(k, 0) = j->x;
        yposVec(k, 0) = j->y;
        pressureVec(k, 0) = j->pressure;
        tiltVec(k, 0) = j->tilt;
        angleVec(k, 0) = j->angle;
        xdeltaVec(k, 0) = j->dx;
        ydeltaVec(k, 0) = j->dy;
    }
    
    Eigen::Matrix<float, 4, Eigen::Dynamic> b_indexes_transpose = b_indexes.transpose();
    Eigen::Matrix4f b_indexes_matrix = b_indexes_transpose * b_indexes;
    Eigen::Matrix4f bmat_identity = Eigen::MatrixXf::Identity(4,4);
    Eigen::MatrixXf b_matrix_inverse = b_indexes_matrix.llt().solve(bmat_identity);

    Eigen::Vector4f curve_xpos = this->beizer_4_invcoeff * b_matrix_inverse * b_indexes_transpose * xposVec;
    Eigen::Vector4f curve_ypos = this->beizer_4_invcoeff * b_matrix_inverse * b_indexes_transpose * yposVec;
    Eigen::Vector4f curve_pressure = this->beizer_4_invcoeff * b_matrix_inverse * b_indexes_transpose * pressureVec;
    Eigen::Vector4f curve_tilt = this->beizer_4_invcoeff * b_matrix_inverse * b_indexes_transpose * tiltVec;
    Eigen::Vector4f curve_angle = this->beizer_4_invcoeff * b_matrix_inverse * b_indexes_transpose * angleVec;
    Eigen::Vector4f curve_xdelta = this->beizer_4_invcoeff * b_matrix_inverse * b_indexes_transpose * xdeltaVec;
    Eigen::Vector4f curve_ydelta = this->beizer_4_invcoeff * b_matrix_inverse * b_indexes_transpose * ydeltaVec;

    this->target_curve->pen_begin(curve_xpos(0,0), curve_ypos(0,0));
    this->target_curve->pen_begin_pressure(curve_pressure(0,0));
    this->target_curve->pen_begin_tilt(curve_tilt(0,0), curve_angle(0,0));
    this->target_curve->pen_begin_velocity(curve_xdelta(0,0), curve_ydelta(0,0));
    
    this->target_curve->pen_to(curve_xpos(1,0), curve_ypos(1,0), curve_xpos(2,0), curve_ypos(2,0), curve_xpos(3,0), curve_ypos(3,0));
    this->target_curve->pen_to_pressure(curve_pressure(1,0), curve_pressure(2,0), curve_pressure(3,0));
    this->target_curve->pen_to_tilt(curve_tilt(1,0), curve_angle(1,0), curve_tilt(2,0), curve_angle(2,0), curve_tilt(3,0), curve_angle(3,0));
    this->target_curve->pen_to_velocity(curve_xdelta(1,0), curve_ydelta(1,0), curve_xdelta(2,0), curve_ydelta(2,0), curve_xdelta(3,0), curve_ydelta(3,0));

    this->curve_xpos = curve_xpos;
    this->curve_ypos = curve_ypos;
};

void icCanvasManager::SplineFitter::add_fit_point(int x, int y, int pressure, int tilt, int angle, int dx, int dy) {
    std::cout << "Fit pt: " << x << ", " << y << std::endl;
    icCanvasManager::BrushStroke::__ControlPoint cp;

    cp.x = x;
    cp.y = y;
    cp.pressure = pressure;
    cp.tilt = tilt;
    cp.angle = angle;
    cp.dx = dx;
    cp.dy = dy;

    this->target_curve->_fitpts.push_back(cp);

    auto ptsize = this->unfitted_points.size();
    if (ptsize < 1) {
        this->distances.push_back(0);
        this->unfitted_points.push_back(cp);
        return;
    }

    if (ptsize > 4) { //We need a minimum point count to start splitting polybeizers
        icCanvasManager::SplineFitter::__ErrorPoint errorPt = this->measure_fitting_error();
        float max_error = std::max(errorPt.x, std::max(errorPt.y, std::max(errorPt.pressure, std::max(errorPt.tilt, std::max(errorPt.angle, std::max(errorPt.dx, errorPt.dy))))));

        if (max_error > (float)this->error_threshold) {
            //Curve exceeds the desired error, time to split
            auto lastcp = this->unfitted_points.back();
            this->unfitted_points.clear();
            this->unfitted_points.push_back(lastcp);
            this->unfitted_points.push_back(cp);

            int xDelta = cp.x - lastcp.x, yDelta = cp.y - lastcp.y;
            int segDist = (int)sqrt((float)xDelta * (float)xDelta + (float)yDelta * (float)yDelta);

            this->distances.clear();
            this->distances.push_back(0);
            this->distances.push_back(segDist);

            this->target_curve->pen_extend();
            this->unfitted_id++;
            std::cout << "Fit curve: " << this->curve_xpos << ", " << this->curve_ypos << std::endl;

            return;
        }
    }

    auto lastcp = this->unfitted_points.back();
    int lastDist = this->distances.back();
    this->unfitted_points.push_back(cp);
    ptsize++;

    int xDelta = cp.x - lastcp.x, yDelta = cp.y - lastcp.y;
    int segDist = (int)sqrt((float)xDelta * (float)xDelta + (float)yDelta * (float)yDelta);
    int newTotalDist = lastDist + segDist;
    this->distances.push_back(newTotalDist);

    this->fit_curve(ptsize);
};

void icCanvasManager::SplineFitter::finish_fitting() {
    auto ptsize = this->unfitted_points.size();
    std::cout << "Ptsize: " << ptsize << std::endl;

    if (ptsize <= 0 && this->target_curve->count_segments() <= 0) {
        //Do nothing.
    } else if (ptsize <= 3 && this->target_curve->count_segments() > 0) {
        this->target_curve->pen_back();
    } else {
        this->fit_curve(ptsize);
        std::cout << "Fit curve: " << this->curve_xpos << ", " << this->curve_ypos << std::endl;
    }
}

void icCanvasManager::SplineFitter::prepare_for_reuse() {
    if (this->unfitted_points.size() > 0) this->unfitted_points.clear();
    if (this->distances.size() > 0) this->distances.clear();
    this->target_curve = NULL;
    this->unfitted_id = 0;
};

icCanvasManager::SplineFitter::__ErrorPoint icCanvasManager::SplineFitter::measure_fitting_error() {
    icCanvasManager::SplineFitter::__ErrorPoint errorPt = {0,0,0,0,0,0,0};
    int newTotalDist = this->distances.back();

    auto i = this->distances.begin();
    auto j = this->unfitted_points.begin();

    for (;
         i != this->distances.end() && j != this->unfitted_points.end();
         i++, j++) {
        float tval = (float)(*i) / (float)newTotalDist;

        icCanvasManager::BrushStroke::__ControlPoint fitted_point = this->target_curve->_curve.evaluate_for_point(this->unfitted_id + tval, this->unfitted_id);

        errorPt.x += std::pow((float)(*j).x - (float)fitted_point.x, 2.0);
        errorPt.y += std::pow((float)(*j).y - (float)fitted_point.y, 2.0);
        errorPt.pressure += std::pow((float)(*j).pressure - (float)fitted_point.pressure, 2.0);
        errorPt.tilt += std::pow((float)(*j).tilt - (float)fitted_point.tilt, 2.0);
        errorPt.angle += std::pow((float)(*j).angle - (float)fitted_point.angle, 2.0);
        errorPt.dx += std::pow((float)(*j).dx - (float)fitted_point.dx, 2.0);
        errorPt.dy += std::pow((float)(*j).dy - (float)fitted_point.dy, 2.0);
    }

    return errorPt;
};
