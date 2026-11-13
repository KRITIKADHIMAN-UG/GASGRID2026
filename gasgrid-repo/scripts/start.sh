#!/bin/sh
# Linux/macOS helper
cd "$(dirname "$0")/.." || exit 1
[ -f .env ] || cp .env.example .env
set -a; . ./.env; set +a
make -C os || exit 1
[ -f backend/target/gasgrid.jar ] || (cd backend && mvn -q -DskipTests package) || exit 1
cd backend && exec java -jar target/gasgrid.jar
