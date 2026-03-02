/**
 * SPDX-FileComment: Header-only library for generating SVG and WebP QR codes.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file qr_generator.hpp
 * @brief Header-only library for generating SVG and WebP QR codes.
 * @version 0.3.0
 * @date 2026-03-02
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
#include <optional> // Hinzugefügt für die Rückgabe des SVG-Strings

namespace qr {

struct Color {
  uint8_t r{0};   
  uint8_t g{0};   
  uint8_t b{0};   
  uint8_t a{255}; 

  std::string to_hex() const {
    return std::format("#{:02x}{:02x}{:02x}", r, g, b);
  }

  double opacity() const { return a / 255.0; }
};

namespace detail {

inline std::string to_lowercase(std::string s) {
  std::ranges::transform(s, s.begin(),
                         [](unsigned char c) { return std::tolower(c); });
  return s;
}

/**
 * @brief Erzeugt den reinen SVG-Code als String.
 */
inline std::string to_svg_string(QRcode *q, int scale, Color fg, Color bg) {
  const int module_count = q->width;
  const int quiet = 4;
  const int size = (module_count + 2 * quiet) * scale;

  std::string out;
  out += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  out += std::format("<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"{}\" "
                     "height=\"{}\" viewBox=\"0 0 {} {}\">\n",
                     size, size, size, size);

  out += std::format("<rect width=\"100%\" height=\"100%\" fill=\"{}\" "
                     "fill-opacity=\"{:.2f}\"/>\n",
                     bg.to_hex(), bg.opacity());

  out += std::format("<g fill=\"{}\" fill-opacity=\"{:.2f}\">\n", fg.to_hex(),
                     fg.opacity());

  const unsigned char *data = q->data;
  for (int y = 0; y < module_count; ++y) {
    for (int x = 0; x < module_count; ++x) {
      int idx = y * module_count + x;
      bool dark = (data[idx] & 0x01) != 0;
      if (dark) {
        int rx = (x + quiet) * scale;
        int ry = (y + quiet) * scale;
        out += std::format(
            "<rect x=\"{}\" y=\"{}\" width=\"{}\" height=\"{}\"/>\n", rx, ry,
            scale, scale);
      }
    }
  }

  out += "</g>\n</svg>\n";
  return out;
}

inline bool save_as_svg(QRcode *q, const std::filesystem::path &out_path,
                        int scale, Color fg, Color bg) {
  std::string svg_data = to_svg_string(q, scale, fg, bg);
  std::ofstream out(out_path);
  if (!out) {
    std::println(stderr, "Error: Cannot open output file: {}",
                 out_path.string());
    return false;
  }
  out << svg_data;
  return true;
}

inline bool save_as_webp(QRcode *q, const std::filesystem::path &out_path,
                         int scale, Color fg, Color bg) {
  const int modules = q->width;
  const int quiet = 4;
  const int img_size = (modules + 2 * quiet) * scale;
  const int stride = img_size * 4;

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
 * @brief Generates a QR Code and returns it directly as an SVG string.
 */
inline std::optional<std::string> generate_svg_string(
    const std::string &text, int scale = 8,
    Color fg = {0, 0, 0, 255}, Color bg = {255, 255, 255, 255},
    QRecLevel ec_level = QR_ECLEVEL_M) {
    
  QRcode *q = QRcode_encodeString(text.c_str(), 0, ec_level, QR_MODE_8, 1);
  if (!q) {
    std::println(stderr, "Error: QR Code Encoding failed.");
    return std::nullopt;
  }

  std::string svg_out = detail::to_svg_string(q, scale, fg, bg);
  QRcode_free(q);
  
  return svg_out;
}

inline bool generate(const std::string &text,
                     const std::filesystem::path &out_path, int scale = 8,
                     Color fg = {0, 0, 0, 255},       
                     Color bg = {255, 255, 255, 255}, 
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