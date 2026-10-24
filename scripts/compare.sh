#!/bin/sh
# Compare associativity on every sample trace.
# Usage: scripts/compare.sh [cache_size_bytes] [block_size_bytes]   (defaults: 64 16)
SIZE=${1:-64}
BLOCK=${2:-16}

echo "Cache: ${SIZE} B, block ${BLOCK} B, LRU"
printf "%-16s %-6s %-6s %-8s %-10s %-9s\n" "trace" "ways" "hits" "misses" "evictions" "hit rate"
for trace in traces/*.txt; do
  for ways in 1 2 4; do
    out=$(./cachesim --trace "$trace" --size "$SIZE" --block "$BLOCK" --ways "$ways") || exit 1
    hits=$(echo "$out" | awk '/^Hits:/ {print $2}')
    misses=$(echo "$out" | awk '/^Misses:/ {print $2}')
    evic=$(echo "$out" | awk '/^Evictions:/ {print $2}')
    rate=$(echo "$out" | awk '/^Hit rate:/ {print $3}')
    printf "%-16s %-6s %-6s %-8s %-10s %-9s\n" "$(basename "$trace" .txt)" "$ways" "$hits" "$misses" "$evic" "$rate"
  done
done
