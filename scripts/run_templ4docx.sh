#!/usr/bin/env bash
set -euo pipefail

# Simple helper to compile & run the templ4docx performance test.
# Place required JARs under tests/performance-tests/libs/ before running.

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TEST_DIR="$ROOT_DIR/tests/performance-tests"
LIB_DIR="$TEST_DIR/libs"
JAVA_FILE="$TEST_DIR/templ4docxPerfTest.java"

if [ ! -f "$JAVA_FILE" ]; then
  echo "Missing $JAVA_FILE" >&2; exit 1
fi
if [ ! -d "$LIB_DIR" ]; then
  echo "Missing libs directory: $LIB_DIR" >&2; exit 1
fi

CP="$LIB_DIR/*:$TEST_DIR"
echo "Compiling..."
javac -cp "$CP" "$JAVA_FILE"
echo "Running..."
java -cp "$CP" templ4docxPerfTest "$TEST_DIR/testdoc.docx" "$TEST_DIR/testdoc-filled.docx"
echo "Done. Output: $TEST_DIR/testdoc-filled.docx"