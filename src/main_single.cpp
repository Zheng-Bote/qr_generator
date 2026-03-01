/**
 * SPDX-FileComment: Main entry point for the QR Code Generator
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file main.cpp
 * @brief Main entry point for the QR Code Generator
 * @version 0.1.0
 * @date 2026-03-01
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#include <qrencode.h>
#include <webp/encode.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

/**
 * @brief Converts a string to lowercase.
 *
 * @param s The input string to be converted.
 * @return std::string The resulting lowercase string.
 */
std::string to_lowercase(std::string s) {
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
 * @return true If the file was successfully saved.
 * @return false If an error occurred during saving.
 */
bool save_as_svg(QRcode *q, const fs::path &out_path, int scale) {
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
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
         "width=\""
      << size << "\" height=\"" << size << "\" viewBox=\"0 0 " << size << " "
      << size << "\">\n";
  out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
  out << "<g fill=\"black\">\n";

  const unsigned char *data = q->data;
  for (int y = 0; y < module_count; ++y) {
    for (int x = 0; x < module_count; ++x) {
      int idx = y * module_count + x;
      bool dark = (data[idx] & 0x01) != 0;
      if (dark) {
        int rx = (x + quiet) * scale;
        int ry = (y + quiet) * scale;
        out << "<rect x=\"" << rx << "\" y=\"" << ry << "\" width=\"" << scale
            << "\" height=\"" << scale << "\"/>\n";
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
 * @return true If the file was successfully saved.
 * @return false If an error occurred during saving or encoding.
 */
bool save_as_webp(QRcode *q, const fs::path &out_path, int scale) {
  const int modules = q->width;
  const int quiet = 4;
  const int img_size = (modules + 2 * quiet) * scale;
  const int stride = img_size * 4;

  std::vector<uint8_t> rgba(static_cast<size_t>(img_size) * img_size * 4, 255u);

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
          row[0] = 0;   // R
          row[1] = 0;   // G
          row[2] = 0;   // B
          row[3] = 255; // A
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

  if (written != output_size) {
    std::println(stderr, "Error writing WebP data.");
    return false;
  }

  return true;
}

/**
 * @brief Main function of the application.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return int Program exit status.
 */
int main(int argc, char **argv) {
  if (argc < 3) {
    std::println(
        "Usage: {} \"<text-or-url>\" <path/to/output.[svg|webp]> [scale]",
        argv[0]);
    std::println("Example:   {} \"https://example.com\" ./qr_code.svg 8",
                 argv[0]);
    return 1;
  }

  std::string text = argv[1];
  fs::path out_path = argv[2];
  int scale = (argc >= 4) ? std::max(1, std::atoi(argv[3])) : 8;

  // Read extension and convert to lowercase
  std::string ext = to_lowercase(out_path.extension().string());
  if (ext != ".svg" && ext != ".webp") {
    std::println(
        stderr,
        "Error: Unsupported file extension ({}). Please use .svg or .webp.",
        ext);
    return 1;
  }

  // Encode QR Code
  QRcode *q = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_M, QR_MODE_8, 1);
  if (!q) {
    std::println(stderr, "Error: QR Code Encoding failed.");
    return 2;
  }

  bool success = false;
  if (ext == ".svg") {
    success = save_as_svg(q, out_path, scale);
  } else if (ext == ".webp") {
    success = save_as_webp(q, out_path, scale);
  }

  QRcode_free(q);

  if (success) {
    std::println("Successfully saved to: {} (Scale: {})", out_path.string(),
                 scale);
    return 0;
  }

  return 3;
}