#!/usr/bin/env bash
set -euo pipefail

# Multi-mode helper: perf (original harness) or report (radgus-report export style)

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TEST_DIR="$ROOT_DIR/tests/performance-tests"
LIB_DIR="$TEST_DIR/libs"
PERF_FILE="$TEST_DIR/templ4docxPerfTest.java"
REPORT_FILE="$TEST_DIR/templ4docxReportExport.java"

[ -f "$PERF_FILE" ] || { echo "Missing $PERF_FILE" >&2; exit 1; }
[ -f "$REPORT_FILE" ] || echo "(Optional) Missing $REPORT_FILE" >&2
[ -d "$LIB_DIR" ] || { echo "Missing libs directory: $LIB_DIR" >&2; exit 1; }

CP="$LIB_DIR/*:$TEST_DIR"
echo "Compiling harnesses..."
javac -cp "$CP" "$PERF_FILE" $([ -f "$REPORT_FILE" ] && echo "$REPORT_FILE")

MODE=${1:-perf}; shift || true
TEMPLATE="$TEST_DIR/testdoc-java.docx"
OUTPUT="$TEST_DIR/testdoc-java-filled.docx"

case "$MODE" in
  perf)
    echo "Running perf harness (templ4docxPerfTest)...";
    java -cp "$CP" templ4docxPerfTest "$TEMPLATE" "$OUTPUT" "$@";
    ;;
  report)
    if [ ! -f "$REPORT_FILE" ]; then echo "report mode requested but $REPORT_FILE missing" >&2; exit 2; fi
    echo "Running report export harness (templ4docxReportExport)...";
    java -cp "$CP" templ4docxReportExport "$TEMPLATE" "$OUTPUT" "$@";
    ;;
  *)
    echo "Unknown mode: $MODE (use perf | report)" >&2; exit 3;
    ;;
esac

echo "Done. Output: $OUTPUT"