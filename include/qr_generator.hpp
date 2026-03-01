/**
 * SPDX-FileComment: Header-only library for generating SVG and WebP QR codes.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file qr_generator.hpp
 * @brief Header-only library for generating SVG and WebP QR codes.
 * @version 0.1.0
 * @date 2026-03-01
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#pragma once

#include <qrencode.h>
#include <webp/encode.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <vector>

namespace qr {

/**
 * @brief Represents an RGBA color.
 */
struct Color {
  uint8_t r{0};   ///< Red component
  uint8_t g{0};   ///< Green component
  uint8_t b{0};   ///< Blue component
  uint8_t a{255}; ///< Alpha component

  /**
   * @brief Helper for SVG: Returns the hex code (e.g., #FF0000).
   *
   * @return std::string Hexadecimal color representation.
   */
  std::string to_hex() const {
    return std::format("#{:02x}{:02x}{:02x}", r, g, b);
  }

  /**
   * @brief Helper for SVG: Returns the opacity (0.0 to 1.0).
   *
   * @return double Opacity ranging from 0.0 to 1.0.
   */
  double opacity() const { return a / 255.0; }
};

namespace detail {

/**
 * @brief Converts a string to lowercase.
 *
 * @param s String to evaluate
 * @return std::string Lowercase copy of the string
 */
inline std::string to_lowercase(std::string s) {
  std::ranges::transform(s, s.begin(),
                         [](unsigned char c) { return std::tolower(c); });
  return s;
}

/**
 * @brief Generates and saves a QR code as an SVG file.
 *
 * @param q Pointer to the generated QRcode object.
 * @param out_path The filesystem path where the SVG should be saved.
 * @param scale The scaling factor for the QR code.
 * @param fg The foreground color of the QR code.
 * @param bg The background color of the QR code.
 * @return true If the file was successfully saved.
 * @return false If an error occurred during saving.
 */
inline bool save_as_svg(QRcode *q, const std::filesystem::path &out_path,
                        int scale, Color fg, Color bg) {
  const int module_count = q->width;
  const int quiet = 4;
  const int size = (module_count + 2 * quiet) * scale;

  std::ofstream out(out_path);
  if (!out) {
    std::println(stderr, "Error: Cannot open output file: {}",
                 out_path.string());
    return false;
  }

  out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  out << std::format("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"{}\" "
                     "height=\"{}\" viewBox=\"0 0 {} {}\">\n",
                     size, size, size, size);

  // Draw background
  out << std::format("<rect width=\"100%\" height=\"100%\" fill=\"{}\" "
                     "fill-opacity=\"{:.2f}\"/>\n",
                     bg.to_hex(), bg.opacity());

  // Open foreground group
  out << std::format("<g fill=\"{}\" fill-opacity=\"{:.2f}\">\n", fg.to_hex(),
                     fg.opacity());

  const unsigned char *data = q->data;
  for (int y = 0; y < module_count; ++y) {
    for (int x = 0; x < module_count; ++x) {
      int idx = y * module_count + x;
      bool dark = (data[idx] & 0x01) != 0;
      if (dark) {
        int rx = (x + quiet) * scale;
        int ry = (y + quiet) * scale;
        out << std::format(
            "<rect x=\"{}\" y=\"{}\" width=\"{}\" height=\"{}\"/>\n", rx, ry,
            scale, scale);
      }
    }
  }

  out << "</g>\n</svg>\n";
  return true;
}

/**
 * @brief Generates and saves a QR code as a WebP file.
 *
 * @param q Pointer to the generated QRcode object.
 * @param out_path The filesystem path where the WebP should be saved.
 * @param scale The scaling factor for the QR code.
 * @param fg The foreground color of the QR code.
 * @param bg The background color of the QR code.
 * @return true If the file was successfully saved.
 * @return false If an error occurred during saving or encoding.
 */
inline bool save_as_webp(QRcode *q, const std::filesystem::path &out_path,
                         int scale, Color fg, Color bg) {
  const int modules = q->width;
  const int quiet = 4;
  const int img_size = (modules + 2 * quiet) * scale;
  const int stride = img_size * 4;

  // Fill buffer with background color
  std::vector<uint8_t> rgba(static_cast<size_t>(img_size) * img_size * 4);
  for (size_t i = 0; i < rgba.size(); i += 4) {
    rgba[i] = bg.r;
    rgba[i + 1] = bg.g;
    rgba[i + 2] = bg.b;
    rgba[i + 3] = bg.a;
  }

  const unsigned char *data = q->data;
  for (int y = 0; y < modules; ++y) {
    for (int x = 0; x < modules; ++x) {
      int idx = y * modules + x;
      bool dark = (data[idx] & 0x01) != 0;
      if (!dark)
        continue;

      int start_x = (x + quiet) * scale;
      int start_y = (y + quiet) * scale;
      for (int yy = 0; yy < scale; ++yy) {
        int py = start_y + yy;
        uint8_t *row = &rgba[(py * img_size + start_x) * 4];
        for (int xx = 0; xx < scale; ++xx) {
          row[0] = fg.r;
          row[1] = fg.g;
          row[2] = fg.b;
          row[3] = fg.a;
          row += 4;
        }
      }
    }
  }

  uint8_t *output = nullptr;
  size_t output_size =
      WebPEncodeLosslessRGBA(rgba.data(), img_size, img_size, stride, &output);

  if (output_size == 0 || output == nullptr) {
    std::println(stderr, "Error: WebP Encoding failed.");
    return false;
  }

  FILE *f = std::fopen(out_path.string().c_str(), "wb");
  if (!f) {
    std::println(stderr, "Error: Cannot open output file: {}",
                 out_path.string());
    WebPFree(output);
    return false;
  }

  size_t written = std::fwrite(output, 1, output_size, f);
  std::fclose(f);
  WebPFree(output);

  return written == output_size;
}

} // namespace detail

/**
 * @brief Main wrapper function to generate and save QR code with optional color
 * parameters.
 *
 * @param text The data to be encoded as a QR code.
 * @param out_path Output filesystem path. Must end with .svg or .webp.
 * @param scale Scaling factor. Default is 8.
 * @param fg Foreground color. Default is black.
 * @param bg Background color. Default is white.
 * @param ec_level Error correction level. Default is medium (QR_ECLEVEL_M).
 * @return true if successful, false otherwise.
 */
inline bool generate(const std::string &text,
                     const std::filesystem::path &out_path, int scale = 8,
                     Color fg = {0, 0, 0, 255},       // Default: Black
                     Color bg = {255, 255, 255, 255}, // Default: White
                     QRecLevel ec_level = QR_ECLEVEL_M) {
  std::string ext = detail::to_lowercase(out_path.extension().string());
  if (ext != ".svg" && ext != ".webp") {
    std::println(stderr, "Error: Unsupported file extension ({}).", ext);
    return false;
  }

  QRcode *q = QRcode_encodeString(text.c_str(), 0, ec_level, QR_MODE_8, 1);
  if (!q) {
    std::println(stderr, "Error: QR Code Encoding failed.");
    return false;
  }

  bool success = false;
  if (ext == ".svg") {
    success = detail::save_as_svg(q, out_path, scale, fg, bg);
  } else if (ext == ".webp") {
    success = detail::save_as_webp(q, out_path, scale, fg, bg);
  }

  QRcode_free(q);
  return success;
}

} // namespace qr