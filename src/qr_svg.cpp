/**
 * SPDX-FileComment: QR code to SVG generation utility
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file qr_svg.cpp
 * @brief QR code to SVG generation utility
 * @version 0.1.0
 * @date 2026-03-01
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */

#include <qrencode.h>

#include <iomanip>
#include <iostream>
#include <print>
#include <sstream>
#include <string>

/**
 * @brief Main function for the SVG generation utility.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return int Program exit status.
 */
int main(int argc, char **argv) {
  if (argc < 2) {
    std::println(stderr, "Usage: {} \"<text-or-url>\"", argv[0]);
    return 1;
  }

  std::string text = argv[1];

  // Encode: version 0 = automatic, error level M (QRECLEVELM), mode 8-bit
  QRcode *q = QRcode_encodeString(text.c_str(), 0, QR_ECLEVEL_M, QR_MODE_8, 1);
  if (!q) {
    std::println(stderr, "QR encoding failed");
    return 2;
  }

  const int module_count = q->width; // number of modules per side
  const int quiet = 4;               // quiet zone in modules
  const int scale = 8;               // pixels per module in SVG units
  const int size = (module_count + 2 * quiet) * scale;

  std::ostringstream svg;
  svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
         "width=\""
      << size << "\" height=\"" << size << "\" viewBox=\"0 0 " << size << " "
      << size << "\">\n";
  svg << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
  svg << "<g fill=\"black\">\n";

  const unsigned char *data = q->data;
  for (int y = 0; y < module_count; ++y) {
    for (int x = 0; x < module_count; ++x) {
      int idx = y * module_count + x;
      bool dark = (data[idx] & 0x01) != 0;
      if (dark) {
        int rx = (x + quiet) * scale;
        int ry = (y + quiet) * scale;
        svg << "<rect x=\"" << rx << "\" y=\"" << ry << "\" width=\"" << scale
            << "\" height=\"" << scale << "\"/>\n";
      }
    }
  }

  svg << "</g>\n</svg>\n";

  // Output SVG to stdout
  std::print("{}", svg.str());

  QRcode_free(q);
  return 0;
}