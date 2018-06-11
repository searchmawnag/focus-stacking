#pragma once

#include <stdint.h>
#include <iostream>
#include "matrix.h"
#include "filter_kernel.h"

template <typename TInput, typename TOutput>
class ImageFilter {
    private:
        FilterKernel& kernel;

        void convolution2D(Matrix<TInput>& image, Matrix<TOutput>& filtered_image);
        void convolution1D(Matrix<TInput>& image, Matrix<TOutput>& filtered_image);

    public:
        ImageFilter(FilterKernel& kernel);

        Matrix<TOutput> convolution(Matrix<TInput>& image);
};

template <typename TInput, typename TOutput>
ImageFilter<TInput, TOutput>::ImageFilter(FilterKernel& kernel) :
        kernel(kernel)
{ }

template <typename TInput, typename TOutput>
Matrix<TOutput> ImageFilter<TInput, TOutput>::convolution(Matrix<TInput>& image) {
    Shape result_image_shape = image.getShape();
    Matrix<TOutput> filtered_image(result_image_shape);

    if (kernel.isSeparableInto1D()) {
        convolution1D(image, filtered_image);
    }
    else {
        convolution2D(image, filtered_image);
    }

    return filtered_image;
}

template <typename TInput, typename TOutput>
void ImageFilter<TInput, TOutput>::convolution2D(Matrix<TInput>& image, Matrix<TOutput>& filtered_image) {
    for (int col = 0; col < image.getShape().x; col++) {
        for (int row = 0; row < image.getShape().y; row++) {
            for (int channel = 0; channel < image.getShape().z; channel++) {

                float convolution_step_value = 0;
                for (int kernel_col = 0; kernel_col < kernel.getSize(); kernel_col++) {
                    for (int kernel_row = 0; kernel_row < kernel.getSize(); kernel_row++) {
                        int image_col_idx = col - (kernel.getSize() - 1) / 2 + kernel_col;
                        int image_row_idx = row - (kernel.getSize() - 1) / 2 + kernel_row;

                        uint8_t image_value = 0;
                        if (image_col_idx >= 0 && image_col_idx < image.getShape().x &&
                            image_row_idx >= 0 && image_row_idx < image.getShape().y) {
                            image_value = image.at(image_col_idx, image_row_idx, channel);
                        }

                        convolution_step_value += kernel.at(kernel_col, kernel_row) * image_value;
                    }
                }
                filtered_image.at(col, row, channel) = convolution_step_value;

            }
        }
    }
}

template <typename TInput, typename TOutput>
void ImageFilter<TInput, TOutput>::convolution1D(Matrix<TInput>& image, Matrix<TOutput>& filtered_image) {
    Matrix<TOutput> after_first_pass_image(image.getShape());

    for (int kernel_direction = 0; kernel_direction < 2; kernel_direction++) {
        for (int row = 0; row < image.getShape().y; row++) {
            for (int col = 0; col < image.getShape().x; col++) {
                for (int channel = 0; channel < image.getShape().z; channel++) {

                    float convolution_step_value = 0;
                    for (int kernel_idx = 0; kernel_idx < kernel.getSize(); kernel_idx++) {
                        int image_col_idx = kernel_direction == 0 ? col - (kernel.getSize() - 1) / 2 + kernel_idx : col;
                        int image_row_idx = kernel_direction == 1 ? row - (kernel.getSize() - 1) / 2 + kernel_idx : row;

                        double image_value = 0.0;
                        if (image_col_idx >= 0 && image_col_idx < image.getShape().x &&
                            image_row_idx >= 0 && image_row_idx < image.getShape().y) {
                            image_value = kernel_direction == 0 ? 
                                          image.at(image_col_idx, image_row_idx, channel) : 
                                          after_first_pass_image.at(image_col_idx, image_row_idx, channel);
                        }

                        convolution_step_value += kernel.at1D(kernel_idx) * image_value;
                    }
                    filtered_image.at(col, row, channel) = convolution_step_value;

                    // TODO: magic number
                    if (kernel_direction == 0) {
                        after_first_pass_image.at(col, row, channel) = convolution_step_value;
                    }
                }
            }
        }
    }
}