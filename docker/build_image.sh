#!/bin/bash

ROOT_DIR=$(realpath "$(dirname "$0")/..")

docker build \
  -t gitlab.com/token-team/engineering/external/trice:latest \
  -f "${ROOT_DIR}/docker/Dockerfile" \
  "${ROOT_DIR}"
