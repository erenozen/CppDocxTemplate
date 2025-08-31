#!/usr/bin/env bash
set -euo pipefail
# Parity runner for C++ harnesses mirroring scripts/run_templ4docx.sh
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
TEST_DIR="$ROOT_DIR/tests/performance-tests"
PERF_BIN="$BUILD_DIR/tests/QtDocxTemplatePerfTest"
REPORT_BIN="$BUILD_DIR/tests/QtDocxTemplateReportExport"
MODE=${1:-perf}; shift || true
TEMPLATE_CPP="$TEST_DIR/testdoc-cpp.docx"
OUTPUT_CPP="$TEST_DIR/testdoc-cpp-filled.docx"
# Configure & build if binaries missing
if [ ! -x "$PERF_BIN" ] || [ ! -x "$REPORT_BIN" ]; then
  echo "Building QtDocxTemplate harnesses..."
  cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DQTDOCTXTEMPLATE_BUILD_TESTS=ON
  cmake --build "$BUILD_DIR" -j
fi
case "$MODE" in
  perf)
    echo "Running C++ perf harness..."
    "$PERF_BIN" "$TEMPLATE_CPP" "$OUTPUT_CPP" "$@";
    ;;
  report)
    echo "Running C++ report export harness..."
    "$REPORT_BIN" "$TEMPLATE_CPP" "$OUTPUT_CPP" "$@";
    ;;
  *)
    echo "Unknown mode: $MODE (use perf | report)" >&2; exit 3;
    ;;
 esac
 echo "Done. Output: $OUTPUT_CPP"
