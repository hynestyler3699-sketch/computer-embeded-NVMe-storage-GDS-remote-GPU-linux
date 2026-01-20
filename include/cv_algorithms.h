#ifndef CV_ALGORITHMS_H
#define CV_ALGORITHMS_H

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Image structure for grayscale images
 */
typedef struct {
    uint32_t width;      /**< Image width in pixels */
    uint32_t height;     /**< Image height in pixels */
    uint8_t* data;       /**< Pixel data (row-major, single channel) */
} Image;

/**
 * @brief Corner keypoint structure
 */
typedef struct {
    float x;             /**< X coordinate */
    float y;             /**< Y coordinate */
    float strength;      /**< Corner response strength */
} Corner;

/* ============================================================================
 * Image I/O
 * ============================================================================ */

/**
 * @brief Create a new image with specified dimensions
 * @param width Image width
 * @param height Image height
 * @return Pointer to new Image, or NULL on failure
 */
Image* cv_image_create(uint32_t width, uint32_t height);

/**
 * @brief Free memory allocated for an image
 * @param img Pointer to image to free
 */
void cv_image_free(Image* img);

/**
 * @brief Load an image from a raw file
 * @param path Path to the raw image file
 * @param width Expected image width
 * @param height Expected image height
 * @return Pointer to loaded Image, or NULL on failure
 */
Image* cv_image_load_raw(const char* path, uint32_t width, uint32_t height);

/**
 * @brief Save an image to a raw file
 * @param img Pointer to the image
 * @param path Output path
 * @return 0 on success, -1 on failure
 */
int cv_image_save_raw(const Image* img, const char* path);

/* ============================================================================
 * Edge Detection
 * ============================================================================ */

/**
 * @brief Perform Sobel edge detection
 * @param input Input image
 * @return New image containing edge magnitudes, or NULL on failure
 * @note Caller must free the returned image with cv_image_free()
 */
Image* cv_sobel_edge_detection(Image* input);

/**
 * @brief Perform Sobel edge detection with direction
 * @param input Input image
 * @param magnitude Output magnitude image (allocated by caller)
 * @param direction Output direction image in radians (allocated by caller, can be NULL)
 * @return 0 on success, -1 on failure
 */
int cv_sobel_edge_detection_full(Image* input, Image* magnitude, float* direction);

/* ============================================================================
 * Feature Detection
 * ============================================================================ */

/**
 * @brief Detect corner keypoints using FAST algorithm
 * @param input Input image
 * @param num_corners Output: number of detected corners
 * @return Array of Corner structures, or NULL on failure
 * @note Caller must free the returned array with free()
 */
Corner* cv_fast_corners(Image* input, int* num_corners);

/**
 * @brief Detect corner keypoints with custom threshold
 * @param input Input image
 * @param threshold Intensity difference threshold (default: 20)
 * @param non_max_suppression Enable non-maximum suppression
 * @param num_corners Output: number of detected corners
 * @return Array of Corner structures, or NULL on failure
 */
Corner* cv_fast_corners_ex(Image* input, int threshold, int non_max_suppression, int* num_corners);

/* ============================================================================
 * Image Processing
 * ============================================================================ */

/**
 * @brief Normalize image pixel values to a range
 * @param img Image to normalize (modified in-place)
 * @param min_val Target minimum value (0.0 - 255.0)
 * @param max_val Target maximum value (0.0 - 255.0)
 */
void cv_normalize_image(Image* img, float min_val, float max_val);

/**
 * @brief Apply Gaussian blur to an image
 * @param input Input image
 * @param kernel_size Blur kernel size (must be odd)
 * @param sigma Gaussian sigma value
 * @return New blurred image, or NULL on failure
 */
Image* cv_gaussian_blur(Image* input, int kernel_size, float sigma);

/**
 * @brief Resize an image using bilinear interpolation
 * @param input Input image
 * @param new_width Target width
 * @param new_height Target height
 * @return New resized image, or NULL on failure
 */
Image* cv_resize(Image* input, uint32_t new_width, uint32_t new_height);

#ifdef __cplusplus
}
#endif

#endif /* CV_ALGORITHMS_H */
