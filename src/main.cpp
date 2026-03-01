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

#include "qr_generator.hpp"
#include <charconv>
#include <filesystem>
#include <iostream>
#include <print>
#include <string>

/**
 * @brief Parses a hex string (e.g., "FF0000") into a qr::Color.
 *
 * @param hex The hexadecimal color string (optionally prefixed with '#').
 * @param alpha The alpha (opacity) component to use.
 * @return qr::Color The parsed color (falls back to black on error).
 */
qr::Color parse_hex(std::string_view hex, uint8_t alpha = 255) {
  if (hex.starts_with('#'))
    hex.remove_prefix(1);
  if (hex.length() != 6)
    return {0, 0, 0, alpha}; // Fallback: Black on invalid input

  uint32_t rgb = 0;
  std::from_chars(hex.data(), hex.data() + hex.size(), rgb, 16);

  return {static_cast<uint8_t>((rgb >> 16) & 0xFF),
          static_cast<uint8_t>((rgb >> 8) & 0xFF),
          static_cast<uint8_t>(rgb & 0xFF), alpha};
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
        "Usage: {} \"<text>\" <output.[svg|webp]> [scale] [fg_hex] [bg_hex]",
        argv[0]);
    std::println(
        "Example:   {} \"https://example.com\" qr.webp 10 FF0000 FFFFFF",
        argv[0]);
    return 1;
  }

  std::string text = argv[1];
  std::filesystem::path out_path = argv[2];
  int scale = (argc >= 4) ? std::max(1, std::atoi(argv[3])) : 8;

  // Parse colors (or use defaults)
  qr::Color fg = (argc >= 5) ? parse_hex(argv[4]) : qr::Color{0, 0, 0, 255};
  qr::Color bg =
      (argc >= 6) ? parse_hex(argv[5]) : qr::Color{255, 255, 255, 255};

  if (qr::generate(text, out_path, scale, fg, bg)) {
    std::println("Successfully saved to: {}", out_path.string());
    std::println("Scale: {} | FG: {} | BG: {}", scale, fg.to_hex(),
                 bg.to_hex());
    return 0;
  }

  return 2;
}