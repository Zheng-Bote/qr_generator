/**
 * SPDX-FileComment: QR code to WebP generation utility
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file qr_webp.cpp
 * @brief QR code to WebP generation utility
 * @version 0.1.0
 * @date 2026-03-01
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#include <cstdio>
#include <cstring>
#include <iostream>
#include <print>
#include <qrencode.h>
#include <string>
#include <vector>
#include <webp/encode.h>

/**
 * @brief Main function for the WebP generation utility.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return int Program exit status.
 */
int main(int argc, char **argv) {
  if (argc < 2) {
    std::println(stderr, "Usage: {} \"<text-or-url>\" [output.webp] [scale]",
                 argv[0]);
    return 1;
  }

  std::string text = argv[1];
  const char *out_path = (argc >= 3) ? argv[2] : "qr.webp";
  int scale = (argc >= 4) ? std::max(1, std::atoi(argv[3])) : 8;

  // Encode QR (automatic version, error level M, 8-bit mode)
  QRcode *q = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_M, QR_MODE_8, 1);
  if (!q) {
    std::println(stderr, "QR encoding failed");
    return 2;
  }

  const int modules = q->width;
  const int quiet = 4; // quiet zone in modules
  const int img_size = (modules + 2 * quiet) * scale;
  const int stride = img_size * 4; // RGBA

  // RGBA buffer, default white background (255,255,255,255)
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

  // Encode lossless WebP (RGBA)
  uint8_t *output = nullptr;
  size_t output_size =
      WebPEncodeLosslessRGBA(rgba.data(), img_size, img_size, stride, &output);
  if (output_size == 0 || output == nullptr) {
    std::println(stderr, "WebP encoding failed");
    QRcode_free(q);
    return 3;
  }

  // Write to file
  FILE *f = std::fopen(out_path, "wb");
  if (!f) {
    std::println(stderr, "Cannot open output file: {}", out_path);
    WebPFree(output);
    QRcode_free(q);
    return 4;
  }
  size_t written = std::fwrite(output, 1, output_size, f);
  std::fclose(f);
  WebPFree(output);
  QRcode_free(q);

  if (written != output_size) {
    std::println(stderr, "Write error");
    return 5;
  }

  std::println("Saved {} ({} bytes)", out_path, output_size);
  return 0;
}